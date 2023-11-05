#ifndef XFAST_SOCKAPI_H__
#define XFAST_SOCKAPI_H__



#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>

#include "ipaddr.h"

using namespace std;

namespace xfast{

namespace xSockApi{

#define INVALID_SOCK -1


namespace xSockOpt{
    int nonBlock(string& err, int fd);
    int block(string& err, int fd);

    int setKeepAlive(string& err, int fd, int interval=0);

    int enableTcpNoDelay(string& err, int fd);
    int disableTcpNoDelay(string& err, int fd);



    int setReuseAddr(string& err, int fd);
    int setReusePort(string& err, int fd);

    int setRecvTimeout(string& err, int fd, long long ms);
    int setSendTimeout(string& err, int fd, long long ms);
    int setSendBuffer(string& err, int fd, int buffsize);

    int getSocketError(int fd);

} // namespace xSockAp

string getError(const string& tips);
int createSocket(string& err, int domain, int sockType, int protocol = 0);

const static int XFNET_CONNECT_NONE = 0;
const static int  XFNET_CONNECT_NONBLOCK = 1;
const static int  XFNET_CONNECT_BE_BINDING = 2;
/* Best effort binding. */
int tcpConnect(string& err,int flags, const string& addr, int port, xfast::xIpAddr& localAddr);
int tcpConnect(string& err,int flags, const xfast::xIpAddr& svrAddr,  xfast::xIpAddr& localAddr);
int tcpAccept(string& err, int fd, xfast::xIpAddr& peerAddr);
int tcpListen(string& err, int fd);
int tcpBind(string& err, int fd, const xfast::xIpAddr& localAddr);
int tcpBindAndListen(string& err, int fd, const xfast::xIpAddr& localAddr);


int getLocalAddr(string& err, int s, int family, xfast::xIpAddr& localAddr);

int readFd(int fd, char *buf, int count);
int writeFd(int fd, const char *buf, int count);
ssize_t readvFd(int fd, const struct iovec *iov, int iovcnt);

int getTcpInfo(int fd, string& tcpInfo);

};




}







#endif //XFAST_SOCKAPI_H__
