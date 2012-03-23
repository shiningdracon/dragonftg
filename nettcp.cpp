#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include "nettcp.h"

namespace dragonfighting {

// Server

NetIOServer::NetIOServer() :
    sockfd(-1),
    opponentfd(-1),
    listenBacklog(50)
{
}

NetIOServer::~NetIOServer()
{
}

void NetIOServer::bindAndListenSocket(int port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret;
    char portbuff[16];

    snprintf(portbuff, sizeof(portbuff), "%d", port);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    ret = getaddrinfo(NULL, portbuff, &hints, &result);
    if (ret != 0) {
        throw gai_strerror(ret);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ((sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }

        if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }

    if (rp == NULL) {
        freeaddrinfo(result);
        throw "Could not bind";
    }
    freeaddrinfo(result);

    if (listen(sockfd, listenBacklog) == -1) {
        close(sockfd);
        throw "Listen failed";
    }
    int flag = 1;
    if (ioctl(sockfd, FIONBIO, &flag) == -1) {
        close(sockfd);
        throw "Set socket failed!";
    }
}

int NetIOServer::acceptSocket()
{
    int fd = -1;
    struct sockaddr_in addr;
    socklen_t peer_addr_size = sizeof(addr);

    memset(&addr, 0, sizeof(addr));

    if ((fd = accept(sockfd, (struct sockaddr*)&addr, &peer_addr_size)) != -1) {
        opponentfd = fd;
    }
    return opponentfd;
}

void NetIOServer::disconnectSocket()
{
    close(sockfd);
}

int NetIOServer::recvSocket(void *buffer, size_t length)
{
    return read(opponentfd, buffer, length);
}

int NetIOServer::sendSocket(void *buffer, size_t length)
{
    return write(opponentfd, buffer, length);
}



// Client

NetIOClient::NetIOClient() :
    sockfd(-1)
{
}

NetIOClient::~NetIOClient()
{
}

void NetIOClient::connectSocket(const char *straddr, int port)
{
    if (straddr == NULL || port == 0) {
        throw "empty hostname or port";
    }

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret;
    char portbuff[16];

    snprintf(portbuff, sizeof(portbuff), "%d", port);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    ret = getaddrinfo(straddr, portbuff, &hints, &result);
    if (ret != 0) {
        throw gai_strerror(ret);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ((sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            int flag = 1;
            if (ioctl(sockfd, FIONBIO, &flag) == -1) {
                close(sockfd);
                freeaddrinfo(result);
                throw "Set socket failed!";
            }
            break;
        }
        printf("%s\n", strerror(errno));
        close(sockfd);
    }

    if (rp == NULL) {
        freeaddrinfo(result);
        throw "Connect failed";
    }
    freeaddrinfo(result);
}

int NetIOClient::recvSocket(void *buffer, size_t length)
{
    return read(sockfd, buffer, length);
}

int NetIOClient::sendSocket(void *buffer, size_t length)
{
    return write(sockfd, buffer, length);
}


}


#ifdef FTG_TEST

using namespace dragonfighting;

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s [server | client]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        try {
            NetIOServer server;
            server.bindAndListenSocket(10080);
            char buffer[1024];
            while (1) {
                server.acceptSocket();
                if (server.recvSocket(buffer, sizeof(buffer)) > 0) {
                    printf("%s\n", buffer);
                }
            }
        } catch (const char *exception) {
            printf("%s\n", exception);
        }
    } else if (strcmp(argv[1], "client") == 0) {
        try {
            NetIOClient client;
            client.connectSocket("127.0.0.1", 10080);
            char buffer[1024];
            int i = 0;
            while (1) {
                snprintf(buffer, sizeof(buffer), "%d", i);
                client.sendSocket(buffer, sizeof(buffer));
                i++;
            }
        } catch (const char *exception) {
            printf("%s\n", exception);
        }
    } else {
        printf("usage: %s [server | client]\n", argv[0]);
        return 1;
    }

    return 0;
}


#endif
