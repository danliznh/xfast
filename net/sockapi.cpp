#include "sockapi.h"
#include "util.h"
#include "file_api.h"

#include<cassert>
#include<string>
#include <sstream> 

using namespace std;

namespace xfast{

namespace xSockApi{

string getError(const string& tips)
{
    std::stringstream ostr;
    ostr <<  tips << ",errno:" << errno <<",msg:" << strerror(errno);
    return ostr.str();
}

static string gasGetError(int rc, const string& tips){
    std::stringstream ostr;
    ostr <<  tips << ",errno:" << errno <<",msg:" <<  gai_strerror(rc);
    return ostr.str();
}


namespace xSockOpt{


static int setBlock(string& err, int fd, int non_block) {
    int flags;

    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
         err = getError("fcntl(F_GETFL).");
        return XFNET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
         err = getError("fcntl(F_SETFL,O_NONBLOCK).");
        return XFNET_ERR;
    }
    return XFNET_SUCC;
}

int nonBlock(string& err, int fd) {
    return setBlock(err,fd,1);
}

int block(string& err, int fd) {
    return setBlock(err,fd,0);
}

/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
int setKeepAlive(string& err, int fd, int interval)
{
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
         err = getError("setsockopt SO_KEEPALIVE.");
        return XFNET_ERR;
    }
    if (interval > 0){
    #ifdef __linux__
        /* Default settings are more or less garbage, with the keepalive time
        * set to 7200 by default on Linux. Modify settings to make the feature
        * actually useful. */

        /* Send first probe after interval. */
        val = interval;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
            err = getError("setsockopt TCP_KEEPIDLE.");
            return XFNET_ERR;
        }

        /* Send next probes after the specified interval. Note that we set the
        * delay as interval / 3, as we send three probes before detecting
        * an error (see the next setsockopt call). */
        val = interval/3;
        if (val == 0) val = 1;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
            err = getError("setsockopt TCP_KEEPINTVL.");
            return XFNET_ERR;
        }

        /* Consider the socket in error state after three we send three ACK
        * probes without getting a reply. */
        val = 3;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
            err = getError("setsockopt TCP_KEEPCNT.");
            return XFNET_ERR;
        }
    #else
        ((void) interval); /* Avoid unused var warning for non Linux systems. */
    #endif

    }

    return XFNET_SUCC;
}

static int setTcpNoDelay(string& err, int fd, int val)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
         err = getError("setsockopt TCP_NODELAY.");
        return XFNET_ERR;
    }
    return XFNET_SUCC;
}

/* disable/enable Nagle's algorithm */
int enableTcpNoDelay(string& err, int fd){
    return setTcpNoDelay(err, fd, 1);
}

int disableTcpNoDelay(string& err, int fd){
    return setTcpNoDelay(err, fd, 0);
}


int setSendBuffer(string& err, int fd, int buffsize){
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize)) == -1)
    {
         err = getError("setsockopt SO_SNDBUF.");
        return XFNET_ERR;
    }
    return XFNET_SUCC;
}

int setReuseAddr(string& err, int fd){
    int yes = 1;
    /* Make sure connection-intensive things
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        err = getError("setsockopt SO_REUSEADDR.");
        return XFNET_ERR;
    }
    return XFNET_SUCC;
}

int setReusePort(string& err, int fd){
#ifdef SO_REUSEPORT
  int yes = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1){
        err = getError("setsockopt SO_REUSEPORT.");
        return XFNET_ERR;
  }
#else
    XFAST_ERROR << "SO_REUSEPORT is not supported.";
#endif
    return XFNET_SUCC;
}

/* Set the socket receive timeout (SO_RCVTIMEO socket option) to the specified
 * number of milliseconds, or disable it if the 'ms' argument is zero. */
int setRecvTimeout(string& err, int fd, long long ms) {
    struct timeval tv;

    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        err= getError("setsockopt SO_RCVTIMEO.");
        return XFNET_ERR;
    }
    return XFNET_SUCC;
}

/* Set the socket send timeout (SO_SNDTIMEO socket option) to the specified
 * number of milliseconds, or disable it if the 'ms' argument is zero. */
int setSendTimeout(string& err, int fd, long long ms) {
    struct timeval tv;

    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
        err= getError("setsockopt SO_SNDTIMEO.");
        return XFNET_ERR;
    }
    return XFNET_SUCC;
}

int getSocketError(int fd){
    int optval=0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){
        return errno;
    }
    return optval;
}


} // namespace xSockOpt




int createSocket(string& err,int domain, int sockType,int protocol){
    assert(sockType == SOCK_DGRAM || sockType == SOCK_STREAM);

    int s = INVALID_SOCK;
    if ((s = socket(domain, sockType, protocol)) == -1) {
        err = getError("creating socket.");
        return s;
    }
    if (xSockOpt::setReuseAddr(err, s) == XFNET_ERR){
        close(s);
        return INVALID_SOCK;
    }

    return s;
}


int tcpConnect(string& err, int flags, const string& addr, int port, xfast::xIpAddr& localAddr){
    int s = XFNET_ERR, rv;
    struct addrinfo hints, *servinfo, *p;

    string portstr = xUtil::tostr(port);
    memzero(&hints,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr.c_str(), portstr.c_str(),&hints,&servinfo)) != 0) {
        err = gasGetError(rv, "call getaddrinfo.");
        return XFNET_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        /* Try to create the socket and to connect it.
         * If we fail in the socket() call, or on connect(), we retry with
         * the next entry in servinfo. */

        if ((s = createSocket(err,p->ai_family,p->ai_socktype,p->ai_protocol)) == INVALID_SOCK)
            continue;

        if (flags & XFNET_CONNECT_NONBLOCK && xSockOpt::nonBlock(err,s) != XFNET_SUCC)
            goto error;

        if (connect(s,p->ai_addr,p->ai_addrlen) == -1) {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & XFNET_CONNECT_NONBLOCK)
                goto end;
            close(s);
            s = XFNET_ERR;
            continue;
        }

        /* If we ended an iteration of the for loop without errors, we
         * have a connected socket. Set LocalAddr and Let's return to the caller. */
        localAddr.setAddrInfo(p);
        goto end;
    }
    if (p == NULL)
        err = getError("connect socket.");

error:
    if (s != INVALID_SOCK) {
        close(s);
        s = XFNET_ERR;
    }

end:
    freeaddrinfo(servinfo);
    return s;
}


int tcpConnect(string& err,int flags, const xfast::xIpAddr& svrAddr,  xfast::xIpAddr& localAddr){
    int s = XFNET_ERR;
    if ((s = createSocket(err,svrAddr.family(), SOCK_STREAM)) == INVALID_SOCK){
        return XFNET_ERR;
    }
     if (flags & XFNET_CONNECT_NONBLOCK && xSockOpt::nonBlock(err,s) != XFNET_SUCC){
        close(s);
        return XFNET_ERR;
     }

    socklen_t socklen = svrAddr.getSockLen();
    const struct sockaddr* saddr = svrAddr.getSockAddr();
    if (connect(s,saddr, socklen) == -1) {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & XFNET_CONNECT_NONBLOCK){
                goto succ;
            }
            close(s);
            return XFNET_ERR;
    }


succ:
    /* if we have a connected socket. get Peer Address for LocalAddr and Let's return to the caller. */
    if(getLocalAddr(err, s, svrAddr.family(), localAddr) != XFNET_SUCC){
        close(s);
        return XFNET_ERR;
    }

    return s;

}


static int __doTcpAccept(string& err, int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while(1) { // No loop in nonblock server
        fd = accept(s,sa,len);
        if (fd == -1) {
            if (errno == EINTR) //  No EINTR in nonblock server
                continue;
            else {
                err = getError("accept.");
                return XFNET_ERR;
            }
        }
        break;
    }
    return fd;
}

int tcpAccept(string& err, int fd, xfast::xIpAddr& peerAddr){
    int s;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if ((s = __doTcpAccept(err,fd,(struct sockaddr*)&sa,&salen)) == -1)
        return XFNET_ERR;

    if (sa.ss_family == AF_INET) {
        const struct sockaddr_in *s = xNetApi::sockaddr_in_cast(&sa);
        peerAddr.setSockAddrIn(*s);
    } else {
        const struct sockaddr_in6 *s = xNetApi::sockaddr_in6_cast(&sa);
        peerAddr.setSockAddrIn6(*s);
    }
    return s;
}

int tcpListen(string& err, int fd){
    int backlog = SOMAXCONN; // using the system ctrl configure, default 128 in berkery
    if (listen(fd, backlog) == -1) {
        err = getError("listen.");
        return XFNET_ERR;
    }

    return XFNET_SUCC;
}

int tcpBind(string& err, int fd, const xfast::xIpAddr& localAddr){
    const struct sockaddr *sa = localAddr.getSockAddr();
    socklen_t len = localAddr.getSockLen();
    if (bind(fd,sa,len) == -1) {
         err = getError("bind.");
        return XFNET_ERR;
    }

    return XFNET_SUCC;
}

int tcpBindAndListen(string& err, int fd, const xfast::xIpAddr& localAddr){
    if (tcpBind(err, fd, localAddr) != XFNET_SUCC){
        return XFNET_ERR;
    }

    return tcpListen(err, fd);
}


int getLocalAddr(string& err, int s, int family, xfast::xIpAddr& localAddr){
    if (family == AF_INET6){
        socklen_t socklen = 0;
        struct sockaddr_in6 stSock;
        memzero(&stSock, sizeof(stSock));
        if(getsockname(s, (struct sockaddr*)&stSock, &socklen) < 0){
            err = getError("call getsockname.");
            return XFNET_ERR;
        }
        localAddr.setSockAddrIn6(stSock);
    }else{
        socklen_t socklen = 0;
        struct sockaddr_in stSock;
        memzero(&stSock, sizeof(stSock));
        if(getsockname(s, (struct sockaddr*)&stSock, &socklen) < 0){
            err = getError("call getsockname.");
            return XFNET_ERR;
        }
        localAddr.setSockAddrIn(stSock);
    }

    return XFNET_SUCC;
    
}


int readFd(int fd, char *buf, int count){
    return xFileApi::readFd(fd, buf, count);
}

/* Like write(2) but make sure 'count' is written before to return
 * (unless error is encountered) */
int writeFd(int fd, const char *buf, int count){
    return xFileApi::writeFd(fd, buf, count);
}

ssize_t readvFd(int fd, const struct iovec *iov, int iovcnt){
  return ::readv(fd, iov, iovcnt);
}


/* get tcp info for */
int getTcpInfo(int fd, string& tcpInfo){
    struct tcp_info tcpi;
    socklen_t len = sizeof(tcpi);
    memzero((void*)&tcpi, len);
    if( ::getsockopt(fd, SOL_TCP, TCP_INFO, (void*)&tcpi, &len) != 0){
        return XFNET_ERR;
    }

    char buf[256] ={0};
    len = 256;

     snprintf(buf, len, "unrecovered=%u "
        "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
        "lost=%u retrans=%u rtt=%u rttvar=%u "
        "sshthresh=%u cwnd=%u total_retrans=%u",
        tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
        tcpi.tcpi_rto,          // Retransmit timeout in usec
        tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
        tcpi.tcpi_snd_mss,
        tcpi.tcpi_rcv_mss,
        tcpi.tcpi_lost,         // Lost packets
        tcpi.tcpi_retrans,      // Retransmitted packets out
        tcpi.tcpi_rtt,          // Smoothed round trip time in usec
        tcpi.tcpi_rttvar,       // Medium deviation
        tcpi.tcpi_snd_ssthresh,
        tcpi.tcpi_snd_cwnd,
        tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    tcpInfo = string(buf);
    return XFNET_SUCC;
}


}// namespace xSockApi
}// namespace xfast