#ifndef _NETTCP_H_
#define _NETTCP_H_

#include <vector>

using std::vector;

namespace dragonfighting {

class NetIOServer
{
protected:
    int sockfd;
    int opponentfd;
    vector<int> observerfdArray;
    int listenBacklog;

public:
    NetIOServer();
    ~NetIOServer();

    void bindAndListenSocket(int port);
    int acceptSocket();
    void disconnectSocket();
    int recvSocket(void *buffer, size_t length);
    int sendSocket(void *buffer, size_t length);
};

class NetIOClient
{
protected:
    int sockfd;

public:
    NetIOClient();
    ~NetIOClient();

    void connectSocket(const char *addr, int port);
    void disconnectSocket();
    int recvSocket(void *buffer, size_t length);
    int sendSocket(void *buffer, size_t length);
};


}

#endif
