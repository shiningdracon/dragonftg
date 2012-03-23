/*
Reference:
	Simple Network Library from "Networking for Game Programmers"
	http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/

#ifndef _NETUDP_H_
#define _NETUDP_H_yy

// platform detection

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

	#include <winsock2.h>
	#pragma comment( lib, "wsock32.lib" )

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <unistd.h>

#else

	#error unknown platform!

#endif

#include <string.h>
#include <assert.h>
#include <vector>
#include <map>
#include <stack>
#include <list>
#include <algorithm>
#include <functional>


namespace dragonfighting {

    // internet address

    class Address
    {
        public:

            Address()
            {
                address = 0;
                port = 0;
            }

            Address( unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port )
            {
                this->address = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
                this->port = port;
            }

            Address( unsigned int address, unsigned short port )
            {
                this->address = address;
                this->port = port;
            }

            unsigned int GetAddress() const
            {
                return address;
            }

            unsigned char GetA() const
            {
                return ( unsigned char ) ( address >> 24 );
            }

            unsigned char GetB() const
            {
                return ( unsigned char ) ( address >> 16 );
            }

            unsigned char GetC() const
            {
                return ( unsigned char ) ( address >> 8 );
            }

            unsigned char GetD() const
            {
                return ( unsigned char ) ( address );
            }

            unsigned short GetPort() const
            { 
                return port;
            }

            bool operator == ( const Address & other ) const
            {
                return address == other.address && port == other.port;
            }

            bool operator != ( const Address & other ) const
            {
                return ! ( *this == other );
            }

            bool operator < ( const Address & other ) const
            {
                // note: this is so we can use address as a key in std::map
                if ( address < other.address )
                    return true;
                if ( address > other.address )
                    return false;
                else
                    return port < other.port;
            }

        private:

            unsigned int address;
            unsigned short port;
    };


    // sockets

    inline bool InitializeSockets()
    {
#if PLATFORM == PLATFORM_WINDOWS
        WSADATA WsaData;
        return WSAStartup( MAKEWORD(2,2), &WsaData ) != NO_ERROR;
#else
        return true;
#endif
    }

    inline void ShutdownSockets()
    {
#if PLATFORM == PLATFORM_WINDOWS
        WSACleanup();
#endif
    }


    class Socket
    {
        private:
            int socket;

        public:
            Socket();
            ~Socket();
            void Open(unsigned short port);
            void Close();
            bool IsOpen() const;
            bool Send(const Address &destination, const void *data, int size);
            int Receive(Address &sender, void *data, int size);
    };

    class Connection
    {
        public:
            enum Mode {
                None,
                Client,
                Server
            };

            Connection(unsigned int protocolId, float timeout);
            virtual ~Connection();
            bool Start(int port);
            void Stop();
            void Listen();
            void Connect(const Address &address);
            bool IsRunning() const { return running; }
            bool IsConnecting() const { return state == Connecting; }
            bool IsConnectFailed() const { return state == ConnectFail; }
            bool IsConnected() const { return state == Connected; }
            bool IsListening() const { return state == Listening; }
            Mode GetMode() const { return mode; }
            int GetHeaderSize() const { return 4; }
            virtual void Update(float dt);
            virtual bool SendPacket(const void *data, int size);
            virtual int ReceivePacket(void *data, int size);

        protected:
            virtual void OnStart()		{}
            virtual void OnStop()		{}
            virtual void OnConnect()    {}
            virtual void OnDisconnect() {}

        private:
            enum State
            {
                Disconnected,
                Listening,
                Connecting,
                ConnectFail,
                Connected
            };

            unsigned int protocolId;
            float timeout;

            bool running;
            Mode mode;
            State state;
            Socket socket;
            float timeoutAccumulator;
            Address address;

            void ClearData()
            {
                state = Disconnected;
                timeoutAccumulator = 0.0f;
                address = Address();
            }
    };

    // packet queue to store information about sent and received packets sorted in sequence order
    //  + we define ordering using the "sequence_more_recent" function, this works provided there is a large gap when sequence wrap occurs

    struct PacketData
    {
        unsigned int sequence;			// packet sequence number
        float time;					    // time offset since packet was sent or received (depending on context)
        int size;						// packet size in bytes
    };

    inline bool sequence_more_recent( unsigned int s1, unsigned int s2, unsigned int max_sequence )
    {
        return (( s1 > s2 ) && ( s1 - s2 <= max_sequence/2 )) || (( s2 > s1 ) && ( s2 - s1 > max_sequence/2 ));
    }


	class PacketQueue : public std::list<PacketData>
    {
        public:
            bool exists( unsigned int sequence );
            void insert_sorted( const PacketData & p, unsigned int max_sequence );
            void verify_sorted( unsigned int max_sequence );
    };


    class ReliabilitySystem
    {
        private:
            unsigned int max_sequence;			// maximum sequence value before wrap around (used to test sequence wrap at low # values)
            unsigned int local_sequence;		// local sequence number for most recently sent packet
            unsigned int remote_sequence;		// remote sequence number for most recently received packet

            unsigned int sent_packets;			// total number of packets sent
            unsigned int recv_packets;			// total number of packets received
            unsigned int lost_packets;			// total number of packets lost
            unsigned int acked_packets;			// total number of packets acked

            float sent_bandwidth;				// approximate sent bandwidth over the last second
            float acked_bandwidth;				// approximate acked bandwidth over the last second
            float rtt;							// estimated round trip time
            float rtt_maximum;					// maximum expected round trip time (hard coded to one second for the moment)

            std::vector<unsigned int> acks;		// acked packets from last set of packet receives. cleared each update!

            PacketQueue sentQueue;				// sent packets used to calculate sent bandwidth (kept until rtt_maximum)
            PacketQueue pendingAckQueue;		// sent packets which have not been acked yet (kept until rtt_maximum * 2 )
            PacketQueue receivedQueue;			// received packets for determining acks to send (kept up to most recent recv sequence - 32)
            PacketQueue ackedQueue;				// acked packets (kept until rtt_maximum * 2)

        public:
            ReliabilitySystem(unsigned int max_sequence = 0xFFFFFFFF);
            void Reset();
            void PacketSent(int size);
            void PacketReceived(unsigned int sequence, int size);
            unsigned int GenerateAckBits();
            void ProcessAck(unsigned int ack, unsigned int ack_bits);
            void Update(float deltaTime);

            //static bool sequence_more_recent( unsigned int s1, unsigned int s2, unsigned int max_sequence );
            static int bit_index_for_sequence( unsigned int sequence, unsigned int ack, unsigned int max_sequence );
            static unsigned int generate_ack_bits( unsigned int ack, const PacketQueue & received_queue, unsigned int max_sequence );
            void process_ack( unsigned int ack, unsigned int ack_bits, 
                    PacketQueue & pending_ack_queue, PacketQueue & acked_queue, 
                    std::vector<unsigned int> & acks, unsigned int & acked_packets, 
                    float & rtt, unsigned int max_sequence );

            unsigned int GetLocalSequence() const { return local_sequence; }
            unsigned int GetRemoteSequence() const { return remote_sequence; }
            unsigned int GetMaxSequence() const { return max_sequence; }
            void GetAcks( unsigned int ** acks, int & count ) {
                *acks = &this->acks[0];
                count = (int) this->acks.size();
            }
            unsigned int GetSentPackets() const { return sent_packets; }
            unsigned int GetReceivedPackets() const { return recv_packets; }
            unsigned int GetLostPackets() const { return lost_packets; }
            unsigned int GetAckedPackets() const { return acked_packets; }
            float GetSentBandwidth() const { return sent_bandwidth; }
            float GetAckedBandwidth() const { return acked_bandwidth; }
            float GetRoundTripTime() const { return rtt; }
            int GetHeaderSize() const { return 12; }

        protected:
            void AdvanceQueueTime(float deltaTime);
            void UpdateQueues();
            void UpdateStats();
    };


    // connection with reliability (seq/ack)

    class ReliableConnection : public Connection
    {
        private:
            void ClearData() { reliabilitySystem.Reset(); }

#ifdef NET_UNIT_TEST
            unsigned int packet_loss_mask;			// mask sequence number, if non-zero, drop packet - for unit test only
#endif

            ReliabilitySystem reliabilitySystem;	// reliability system: manages sequence numbers and acks, tracks network stats etc.

        public:
            ReliableConnection( unsigned int protocolId, float timeout, unsigned int max_sequence = 0xFFFFFFFF );
            ~ReliableConnection();
            bool SendPacket( const void *data, int size );
            int ReceivePacket( void *data, int size );
            void Update( float deltaTime );
            int GetHeaderSize() const { return Connection::GetHeaderSize() + reliabilitySystem.GetHeaderSize(); }
            ReliabilitySystem & GetReliabilitySystem() { return reliabilitySystem; }
            // unit test controls
#ifdef NET_UNIT_TEST
            void SetPacketLossMask( unsigned int mask )
            {
                packet_loss_mask = mask;
            }
#endif

        protected:
            void WriteInteger( unsigned char * data, unsigned int value );
            void WriteHeader( unsigned char * header, unsigned int sequence, unsigned int ack, unsigned int ack_bits );
            void ReadInteger( const unsigned char * data, unsigned int & value );
            void ReadHeader( const unsigned char * header, unsigned int & sequence, unsigned int & ack, unsigned int & ack_bits );
            virtual void OnStop();
            virtual void OnDisconnect();
    };


}
#endif
