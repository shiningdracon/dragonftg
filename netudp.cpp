/*
Reference:
	Simple Network Library from "Networking for Game Programmers"
	http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/
#include "netudp.h"
#include <stdio.h>

namespace dragonfighting {

Socket::Socket() :
    socket(0)
{
}

Socket::~Socket()
{
    Close();
}

void Socket::Open(unsigned short port)
{
    assert(!IsOpen());

    socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket <=0) {
        socket = 0;
        throw "create socket failed";
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(socket, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) {
        Close();
        throw "bind socket failed";
    }

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
    int nonBlocking = 1;
    if (fcntl(socket, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
        Close();
        throw "set non-blocking failed";
    }
#elif PLATFORM == PLATFORM_WINDOWS
    DWORD nonBlocking = 1;
    if ( ioctlsocket( socket, FIONBIO, &nonBlocking ) != 0 ) {
        Close();
        throw "set non-blocking failed";
    }

#endif

}

void Socket::Close()
{
    if (socket != 0) {
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
        close( socket );
#elif PLATFORM == PLATFORM_WINDOWS
        closesocket( socket );
#endif
        socket = 0;
    }
}

bool Socket::IsOpen() const
{
    return socket != 0;
}

bool Socket::Send(const Address &destination, const void *data, int size) {
    assert(data);
    assert(size > 0);
    assert(socket > 0);
    assert( destination.GetAddress() != 0 );
    assert( destination.GetPort() != 0 );

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl( destination.GetAddress() );
    address.sin_port = htons( destination.GetPort() );

    int sent_bytes = sendto( socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in) );

    return sent_bytes == size;
}

int Socket::Receive(Address &sender, void *data, int size)
{
    assert(data);
    assert(size > 0);
    assert(socket > 0);
#if PLATFORM == PLATFORM_WINDOWS
    typedef int socklen_t;
#endif

    sockaddr_in from;
    socklen_t fromLength = sizeof( from );

    int received_bytes = recvfrom( socket, (char*)data, size, 0, (sockaddr*)&from, &fromLength );

    if ( received_bytes <= 0 )
        return 0;

    unsigned int address = ntohl( from.sin_addr.s_addr );
    unsigned short port = ntohs( from.sin_port );

    sender = Address( address, port );

    return received_bytes;
}

// Connection

Connection::Connection(unsigned int protocolId, float timeout) :
    protocolId(protocolId),
    timeout(timeout),
    running(false)
{
    ClearData();
}

Connection::~Connection()
{
    if (IsRunning()) {
        Stop();
    }
}

bool Connection::Start(int port)
{
    assert(!running);
    try {
        socket.Open(port);
    } catch(const char *) {
        return false;
    }
    running = true;
    OnStart();
    return true;
}

void Connection::Stop()
{
    assert( running );
    bool connected = IsConnected();
    ClearData();
    socket.Close();
    running = false;
    if ( connected ) {
        OnDisconnect();
    }
    OnStop();
}

void Connection::Listen()
{
    bool connected = IsConnected();
    ClearData();
    if (connected) {
        OnDisconnect();
    }
    mode = Server;
    state = Listening;
}

void Connection::Connect(const Address &address)
{
    bool connected = IsConnected();
    ClearData();
    if (connected) {
        OnDisconnect();
    }
    mode = Client;
    state = Connecting;
    this->address = address;
}

void Connection::Update( float deltaTime )
{
    assert(running);
    timeoutAccumulator += deltaTime;
    if ( timeoutAccumulator > timeout )
    {
        if ( state == Connecting )
        {
            printf( "connect timed out\n" );
            ClearData();
            state = ConnectFail;
            OnDisconnect();
        }
        else if ( state == Connected )
        {
            printf( "connection timed out\n" );
            ClearData();
            if ( state == Connecting )
                state = ConnectFail;
            OnDisconnect();
        }
    }
}

bool Connection::SendPacket( const void *data, int size )
{
    assert( running );
    if ( address.GetAddress() == 0 ) {
        return false;
    }
    unsigned char packet[size+4];
    // why?
    packet[0] = (unsigned char) ( protocolId >> 24 );
    packet[1] = (unsigned char) ( ( protocolId >> 16 ) & 0xFF );
    packet[2] = (unsigned char) ( ( protocolId >> 8 ) & 0xFF );
    packet[3] = (unsigned char) ( ( protocolId ) & 0xFF );
    memcpy( &packet[4], data, size );
    return socket.Send( address, packet, size + 4 );
}

int Connection::ReceivePacket( void *data, int size )
{
    assert( running );
    unsigned char packet[size+4];
    Address sender;
    int bytes_read = socket.Receive( sender, packet, size + 4 );
    if ( bytes_read == 0 ) {
        return 0;
    }
    if ( bytes_read <= 4 ) {
        return 0;
    }
    if ( packet[0] != (unsigned char) ( protocolId >> 24 ) || 
            packet[1] != (unsigned char) ( ( protocolId >> 16 ) & 0xFF ) ||
            packet[2] != (unsigned char) ( ( protocolId >> 8 ) & 0xFF ) ||
            packet[3] != (unsigned char) ( protocolId & 0xFF ) ) {
        return 0;
    }
    if ( mode == Server && !IsConnected() )
    {
        printf( "server accepts connection from client %d.%d.%d.%d:%d\n", 
                sender.GetA(), sender.GetB(), sender.GetC(), sender.GetD(), sender.GetPort() );
        state = Connected;
        address = sender;
        OnConnect();
    }
    if ( sender == address )
    {
        if ( mode == Client && state == Connecting )
        {
            printf( "client completes connection with server\n" );
            state = Connected;
            OnConnect();
        }
        timeoutAccumulator = 0.0f;
        memcpy( data, &packet[4], bytes_read - 4 );
        return bytes_read - 4;
    }
    return 0;
}

bool PacketQueue::exists( unsigned int sequence )
{
    for ( iterator itor = begin(); itor != end(); ++itor ) {
        if ( itor->sequence == sequence ) {
            return true;
        }
    }
    return false;
}

void PacketQueue::insert_sorted( const PacketData & p, unsigned int max_sequence )
{
    if ( empty() ) {
        push_back( p );
    } else {
        if ( !sequence_more_recent( p.sequence, front().sequence, max_sequence ) ) {
            push_front( p );
        } else if ( sequence_more_recent( p.sequence, back().sequence, max_sequence ) ) {
            push_back( p );
        } else {
            for ( PacketQueue::iterator itor = begin(); itor != end(); itor++ ) {
                assert( itor->sequence != p.sequence );
                if ( sequence_more_recent( itor->sequence, p.sequence, max_sequence ) ) {
                    insert( itor, p );
                    break;
                }
            }
        }
    }
}

void PacketQueue::verify_sorted( unsigned int max_sequence )
{
    PacketQueue::iterator prev = end();
    for ( PacketQueue::iterator itor = begin(); itor != end(); itor++ ) {
        assert( itor->sequence <= max_sequence );
        if ( prev != end() ) {
            assert( sequence_more_recent( itor->sequence, prev->sequence, max_sequence ) );
            prev = itor;
        }
    }
}



ReliabilitySystem::ReliabilitySystem(unsigned int max_sequence)
{
    this->max_sequence = max_sequence;
    Reset();
}

void ReliabilitySystem::Reset()
{
    local_sequence = 0;
    remote_sequence = 0;
    sentQueue.clear();
    receivedQueue.clear();
    pendingAckQueue.clear();
    ackedQueue.clear();
    sent_packets = 0;
    recv_packets = 0;
    lost_packets = 0;
    acked_packets = 0;
    sent_bandwidth = 0.0f;
    acked_bandwidth = 0.0f;
    rtt = 0.0f;
    rtt_maximum = 1.0f;
}

void ReliabilitySystem::PacketSent(int size)
{
    assert( !sentQueue.exists( local_sequence ) );
    assert( !pendingAckQueue.exists( local_sequence ) );
    PacketData data;
    data.sequence = local_sequence;
    data.time = 0.0f;
    data.size = size;
    sentQueue.push_back( data );
    pendingAckQueue.push_back( data );
    sent_packets++;
    local_sequence++;
    if ( local_sequence > max_sequence ) {
        local_sequence = 0;
    }
}

void ReliabilitySystem::PacketReceived(unsigned int sequence, int size)
{
    recv_packets++;
    if ( receivedQueue.exists( sequence ) ) {
        return;
    }
    PacketData data;
    data.sequence = sequence;
    data.time = 0.0f;
    data.size = size;
    receivedQueue.push_back( data );
    if ( sequence_more_recent( sequence, remote_sequence, max_sequence ) ) {
        remote_sequence = sequence;
    }
}

unsigned int ReliabilitySystem::GenerateAckBits()
{
    return generate_ack_bits( GetRemoteSequence(), receivedQueue, max_sequence );
}

void ReliabilitySystem::ProcessAck(unsigned int ack, unsigned int ack_bits)
{
    process_ack( ack, ack_bits, pendingAckQueue, ackedQueue, acks, acked_packets, rtt, max_sequence );
}

void ReliabilitySystem::Update(float deltaTime)
{
    acks.clear();
    AdvanceQueueTime( deltaTime );
    UpdateQueues();
    UpdateStats();
#ifdef NET_UNIT_TEST
    sentQueue.verify_sorted( max_sequence );
    receivedQueue.verify_sorted( max_sequence );
    pendingAckQueue.verify_sorted( max_sequence );
    ackedQueue.verify_sorted( max_sequence );
#endif
}

// utility functions

/*
bool ReliabilitySystem::sequence_more_recent( unsigned int s1, unsigned int s2, unsigned int max_sequence )
{
    return (( s1 > s2 ) && ( s1 - s2 <= max_sequence/2 )) || (( s2 > s1 ) && ( s2 - s1 > max_sequence/2 ));
}
*/

int ReliabilitySystem::bit_index_for_sequence( unsigned int sequence, unsigned int ack, unsigned int max_sequence )
{
    assert( sequence != ack );
    assert( !sequence_more_recent( sequence, ack, max_sequence ) );
    if ( sequence > ack ) {
        assert( ack < 33 );
        assert( max_sequence >= sequence );
        return ack + ( max_sequence - sequence );
    } else {
        assert( ack >= 1 );
        assert( sequence <= ack - 1 );
        return ack - 1 - sequence;
    }
}

unsigned int ReliabilitySystem::generate_ack_bits( unsigned int ack, const PacketQueue & received_queue, unsigned int max_sequence )
{
    unsigned int ack_bits = 0;
    for ( PacketQueue::const_iterator itor = received_queue.begin(); itor != received_queue.end(); itor++ ) {
        if ( itor->sequence == ack || sequence_more_recent( itor->sequence, ack, max_sequence ) ) {
            break;
        }
        int bit_index = bit_index_for_sequence( itor->sequence, ack, max_sequence );
        if ( bit_index <= 31 ) {
            ack_bits |= 1 << bit_index;
        }
    }
    return ack_bits;
}

void ReliabilitySystem::process_ack( unsigned int ack, unsigned int ack_bits, 
        PacketQueue & pending_ack_queue, PacketQueue & acked_queue, 
        std::vector<unsigned int> & acks, unsigned int & acked_packets, 
        float & rtt, unsigned int max_sequence )
{
    if ( pending_ack_queue.empty() ) {
        return;
    }

    PacketQueue::iterator itor = pending_ack_queue.begin();
    while ( itor != pending_ack_queue.end() ) {
        bool acked = false;

        if ( itor->sequence == ack ) {
            acked = true;
        } else if ( !sequence_more_recent( itor->sequence, ack, max_sequence ) ) {
            int bit_index = bit_index_for_sequence( itor->sequence, ack, max_sequence );
            if ( bit_index <= 31 ) {
                acked = ( ack_bits >> bit_index ) & 1;
            }
        }

        if ( acked ) {
            rtt += ( itor->time - rtt ) * 0.1f;

            acked_queue.insert_sorted( *itor, max_sequence );
            acks.push_back( itor->sequence );
            acked_packets++;
            itor = pending_ack_queue.erase( itor );
        } else {
            ++itor;
        }
    }
}


void ReliabilitySystem::AdvanceQueueTime( float deltaTime )
{
    for ( PacketQueue::iterator itor = sentQueue.begin(); itor != sentQueue.end(); itor++ ) {
        itor->time += deltaTime;
    }

    for ( PacketQueue::iterator itor = receivedQueue.begin(); itor != receivedQueue.end(); itor++ ) {
        itor->time += deltaTime;
    }

    for ( PacketQueue::iterator itor = pendingAckQueue.begin(); itor != pendingAckQueue.end(); itor++ ) {
        itor->time += deltaTime;
    }

    for ( PacketQueue::iterator itor = ackedQueue.begin(); itor != ackedQueue.end(); itor++ ) {
        itor->time += deltaTime;
    }
}

void ReliabilitySystem::UpdateQueues()
{
    const float epsilon = 0.001f;

    while ( sentQueue.size() && sentQueue.front().time > rtt_maximum + epsilon ) {
        sentQueue.pop_front();
    }

    if ( receivedQueue.size() ) {
        const unsigned int latest_sequence = receivedQueue.back().sequence;
        const unsigned int minimum_sequence = latest_sequence >= 34 ? ( latest_sequence - 34 ) : max_sequence - ( 34 - latest_sequence );
        while ( receivedQueue.size() && !sequence_more_recent( receivedQueue.front().sequence, minimum_sequence, max_sequence ) ) {
            receivedQueue.pop_front();
        }
    }

    while ( ackedQueue.size() && ackedQueue.front().time > rtt_maximum * 2 - epsilon ) {
        ackedQueue.pop_front();
    }

    while ( pendingAckQueue.size() && pendingAckQueue.front().time > rtt_maximum + epsilon ) {
        last_lost_packet_seq = pendingAckQueue.front().sequence;
        pendingAckQueue.pop_front();
        lost_packets++;
    }
}

void ReliabilitySystem::UpdateStats()
{
    int sent_bytes_per_second = 0;
    for ( PacketQueue::iterator itor = sentQueue.begin(); itor != sentQueue.end(); ++itor ) {
        sent_bytes_per_second += itor->size;
    }
    int acked_packets_per_second = 0;
    int acked_bytes_per_second = 0;
    for ( PacketQueue::iterator itor = ackedQueue.begin(); itor != ackedQueue.end(); ++itor ) {
        if ( itor->time >= rtt_maximum ) {
            acked_packets_per_second++;
            acked_bytes_per_second += itor->size;
        }
    }
    sent_bytes_per_second /= rtt_maximum;
    acked_bytes_per_second /= rtt_maximum;
    sent_bandwidth = sent_bytes_per_second * ( 8 / 1000.0f );
    acked_bandwidth = acked_bytes_per_second * ( 8 / 1000.0f );
}


ReliableConnection::ReliableConnection( unsigned int protocolId, float timeout, unsigned int max_sequence ) :
    Connection(protocolId, timeout),
    reliabilitySystem(max_sequence)
{
    ClearData();
#ifdef NET_UNIT_TEST
    packet_loss_mask = 0;
#endif
}

ReliableConnection::~ReliableConnection()
{
    if (IsRunning()) {
        Stop();
    }
}

// overriden functions from "Connection"
				
bool ReliableConnection::SendPacket( const void *data, int size )
{
#ifdef NET_UNIT_TEST
    if ( reliabilitySystem.GetLocalSequence() & packet_loss_mask ) {
        reliabilitySystem.PacketSent( size );
        return true;
    }
#endif
    const int header = 12;
    unsigned char packet[header+size];
    unsigned int seq = reliabilitySystem.GetLocalSequence();
    unsigned int ack = reliabilitySystem.GetRemoteSequence();
    unsigned int ack_bits = reliabilitySystem.GenerateAckBits();
    WriteHeader( packet, seq, ack, ack_bits );
    memcpy( packet + header, data, size );
    if ( !Connection::SendPacket( packet, size + header ) ) {
        return false;
    }
    reliabilitySystem.PacketSent( size );
    return true;
}	

int ReliableConnection::ReceivePacket( void *data, int size )
{
    const int header = 12;
    if ( size <= 0 ) {
        return 0;
    }
    unsigned char packet[header+size];
    int received_bytes = Connection::ReceivePacket( packet, size + header );
    if ( received_bytes == 0 ) {
        return 0;
    }
    if ( received_bytes <= header ) {
        return 0;
    }
    unsigned int packet_sequence = 0;
    unsigned int packet_ack = 0;
    unsigned int packet_ack_bits = 0;
    ReadHeader( packet, packet_sequence, packet_ack, packet_ack_bits );
    reliabilitySystem.PacketReceived( packet_sequence, received_bytes - header );
    reliabilitySystem.ProcessAck( packet_ack, packet_ack_bits );
    memcpy( data, packet + header, received_bytes - header );
    return received_bytes - header;
}

void ReliableConnection::Update( float deltaTime )
{
    Connection::Update( deltaTime );
    reliabilitySystem.Update( deltaTime );
}

void ReliableConnection::WriteInteger( unsigned char * data, unsigned int value )
{
    data[0] = (unsigned char) ( value >> 24 );
    data[1] = (unsigned char) ( ( value >> 16 ) & 0xFF );
    data[2] = (unsigned char) ( ( value >> 8 ) & 0xFF );
    data[3] = (unsigned char) ( value & 0xFF );
}

void ReliableConnection::WriteHeader( unsigned char * header, unsigned int sequence, unsigned int ack, unsigned int ack_bits )
{
    WriteInteger( header, sequence );
    WriteInteger( header + 4, ack );
    WriteInteger( header + 8, ack_bits );
}

void ReliableConnection::ReadInteger( const unsigned char * data, unsigned int & value )
{
    value = ( ( (unsigned int)data[0] << 24 ) | ( (unsigned int)data[1] << 16 ) | 
            ( (unsigned int)data[2] << 8 )  | ( (unsigned int)data[3] ) );				
}

void ReliableConnection::ReadHeader( const unsigned char * header, unsigned int & sequence, unsigned int & ack, unsigned int & ack_bits )
{
    ReadInteger( header, sequence );
    ReadInteger( header + 4, ack );
    ReadInteger( header + 8, ack_bits );
}

void ReliableConnection::OnStop()
{
    ClearData();
}

void ReliableConnection::OnDisconnect()
{
    ClearData();
}




}// end namespace


#ifdef _TEST_
#include <stdio.h>

using namespace dragonfighting;

//#define check(n) if ( !n ) { printf( "check failed\n" ); exit(1); }
#define check assert
int main(int argc, char **argv)
{
	const int ServerPort = 30000;
	const int ClientPort = 30001;
	const int ProtocolId = 0x11112222;
	const float DeltaTime = 1.0f;
	const float TimeOut = 5.0f;
	const unsigned int PacketCount = 100;
	
	ReliableConnection client( ProtocolId, TimeOut );
	ReliableConnection server( ProtocolId, TimeOut );
	
	check( client.Start( ClientPort ) );
	check( server.Start( ServerPort ) );
	
	client.Connect( Address(127,0,0,1,ServerPort ) );
	server.Listen();
		
	bool clientAckedPackets[PacketCount];
 	bool serverAckedPackets[PacketCount];
	for ( unsigned int i = 0; i < PacketCount; ++i )
	{
		clientAckedPackets[i] = false;
		serverAckedPackets[i] = false;
	}
	
	bool allPacketsAcked = false;

	while ( true )
	{
		if ( !client.IsConnecting() && client.IsConnectFailed() )
			break;
			
		if ( allPacketsAcked )
			break;
		
		unsigned char packet[256];
		for ( unsigned int i = 0; i < sizeof(packet); ++i )
			packet[i] = (unsigned char) i;
		
		server.SendPacket( packet, sizeof(packet) );
		client.SendPacket( packet, sizeof(packet) );
		
		while ( true )
		{
			unsigned char packet[256];
			int bytes_read = client.ReceivePacket( packet, sizeof(packet) );
			if ( bytes_read == 0 )
				break;
			check( bytes_read == sizeof(packet) );
			for ( unsigned int i = 0; i < sizeof(packet); ++i )
				check( packet[i] == (unsigned char) i );
		}

		while ( true )
		{
			unsigned char packet[256];
			int bytes_read = server.ReceivePacket( packet, sizeof(packet) );
			if ( bytes_read == 0 )
				break;
			check( bytes_read == sizeof(packet) );
			for ( unsigned int i = 0; i < sizeof(packet); ++i )
				check( packet[i] == (unsigned char) i );
		}
		
		int ack_count = 0;
		unsigned int * acks = NULL;
		client.GetReliabilitySystem().GetAcks( &acks, ack_count );
		check( ack_count == 0 || ack_count != 0 && acks );
		for ( int i = 0; i < ack_count; ++i )
		{
			unsigned int ack = acks[i];
			if ( ack < PacketCount )
			{
				check( clientAckedPackets[ack] == false );
				clientAckedPackets[ack] = true;
			}
		}

		server.GetReliabilitySystem().GetAcks( &acks, ack_count );
		check( ack_count == 0 || ack_count != 0 && acks );
		for ( int i = 0; i < ack_count; ++i )
		{
			unsigned int ack = acks[i];
			if ( ack < PacketCount )
			{
				check( serverAckedPackets[ack] == false );
				serverAckedPackets[ack] = true;
			}
		}
		
		unsigned int clientAckCount = 0;
		unsigned int serverAckCount = 0;
		for ( unsigned int i = 0; i < PacketCount; ++i )
		{
 			clientAckCount += clientAckedPackets[i];
 			serverAckCount += serverAckedPackets[i];
		}
		allPacketsAcked = clientAckCount == PacketCount && serverAckCount == PacketCount;
		
		client.Update( DeltaTime );
		server.Update( DeltaTime );
	}
	
	check( client.IsConnected() );
	check( server.IsConnected() );
}

#if 0
int main(int argc, char **argv)
{
    enum Mode
    {
        Client,
        Server
    };

    enum TestCase
    {
        SOCKET_TEST,
        CONNECTION_TEST
    };

    Mode mode = Server;
    TestCase testcase = SOCKET_TEST;
    Address address;

    if ( argc >= 2 ) {
        if (strcmp(argv[1], "server") == 0) {
            mode = Server;
            address = Address(127,0,0,1,26801);
        } else if (strcmp(argv[1], "client") == 0) {
            mode = Client;
            address = Address(127,0,0,1,26800);
        } else {
            printf("Usage:\n");
            return 1;
        }

        if (argc >=3) {
            if (strcmp(argv[2], "socket") == 0) {
                testcase = SOCKET_TEST;
            } else if (strcmp(argv[2], "connection") == 0) {
                testcase = CONNECTION_TEST;
            } else {
                printf("Usage:\n");
                return 1;
            }
        }
    } else {
        printf("Usage:\n");
        return 1;
    }

    // initialize

    if ( !InitializeSockets() ) {
        printf( "failed to initialize sockets\n" );
        return 1;
    }

    if (testcase == SOCKET_TEST) {
        Socket socket;

        socket.Open(mode == Server ? 26800 : 26801);

        int count = 0;
        char buff[16];
        while (1) {
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "%d", count);
            if (!socket.Send(address, buff, strlen(buff))) {
                continue;
            }
            memset(buff, 0, sizeof(buff));
            if (socket.Receive(address, buff, sizeof(buff)) > 0) {
                printf("recv:%s\n", buff);
            }
            count ++;
            usleep(1000000);
        }
    } else if (testcase == CONNECTION_TEST) {
        const int ProtocolId = 0x11223344;
        const float TimeOut = 5.0f;
        const float DeltaTime = 1.0f;

        ReliableConnection connection(ProtocolId, TimeOut);

        if (!connection.Start(mode == Server ? 26800 : 26801)) {
            printf( "failed to start\n" );
            return -1;
        }

        if (mode == Server) {
            connection.Listen();
        } else {
            connection.Connect(address);
        }

        bool connected = false;
        int count = 0;
        char buff[16];
        while (1) {
            if ( mode == Server && connected && !connection.IsConnected() ) {
                printf( "reset flow control\n" );
                connected = false;
            }

            if ( !connected && connection.IsConnected() ) {
                printf( "client connected to server\n" );
                connected = true;
            }

            if ( !connected && connection.IsConnectFailed() ) {
                printf( "connection failed\n" );
                break;
            }

            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "%d", count);
            if (!connection.SendPacket(buff, strlen(buff))) {
                printf("send error\n");
            }
            memset(buff, 0, sizeof(buff));
            if (connection.ReceivePacket(buff, sizeof(buff)) > 0) {
                printf("recv:%s\n", buff);
            }

            connection.Update(DeltaTime);

            count ++;
            usleep(1000000);
        }
    } else {
    }

    return 0;
}
#endif


#endif
