
#include "socket.h"
#include "xkill.h"
#include "loger.h"

namespace xfast{


using namespace xfast::xSockApi::xSockOpt;


xSocket::xSocket(bool die){
    string err;
    fd_ = xSockApi::createSocket(err, AF_INET, SOCK_STREAM);
    if (fd_ == INVALID_SOCK){
        XFAST_ERROR << "program will exit for create socket failed:" << err<<endl;
        selfKill();
    }
}

xSocket::~xSocket(){
    if(fd_ != INVALID_SOCK){
        ::close(fd_);
    }
}

void xSocket::create( int domain, int socketType ){
    string err;
     fd_ = xSockApi::createSocket(err, AF_INET, SOCK_STREAM);
     if (!valid()){
         throw xSocket_Exception(err);
     }
}

int xSocket::getFd()const{
    return fd_;
}
int xSocket::valid(){
    return fd_ != INVALID_SOCK;
}

bool xSocket::getTcpInfo(string& tcpInfo){
    if (xSockApi::getTcpInfo(fd_, tcpInfo) == XFNET_SUCC){
        return true;
    }
    return false;
}



int xSocket::bind(const xIpAddr& localAddr){
    if (!valid()){
        throw xSocket_Exception("[xSocket::bind] invalid socket");
    }

    string err;
    if (xSockApi::tcpBind(err, fd_, localAddr) != XFNET_SUCC) {
        throw xSocket_Exception(err);
    }

    return 0;
}
int xSocket::listen(){
    if (!valid()){
        throw xSocket_Exception("[xSocket::listen] invalid socket");
    }

    string err;
    if (xSockApi::tcpListen(err, fd_) != XFNET_SUCC){
        throw xSocket_Exception("[xSocket::listen] " + err);
    }
    return 0;
}
int xSocket::accept(xIpAddr& peerAddr){
    if (!valid()){
        return INVALID_SOCK;  // it's invalid socket for accept ,dont throw an exception
    }
    string err;
    int connfd = xSockApi::tcpAccept(err, fd_, peerAddr);
    if (connfd == XFNET_ERR){
        XFAST_ERROR<< "[xSocket::accept] accept error:" << err <<endl;
        return INVALID_SOCK;  // it's invalid socket for accept ,dont throw an exception
    }

    return connfd;
}

int xSocket::connectNonblock(const xIpAddr& serverAddr){
    xIpAddr localAddr;
    string err;
    int fd = xSockApi::tcpConnect(err,xSockApi::XFNET_CONNECT_NONBLOCK, serverAddr, localAddr );
    if (fd == XFNET_ERR){
         XFAST_ERROR<< "[xSocket::connect] accept error:" << err <<endl;
        return INVALID_SOCK;
    }
    (void)localAddr;
    return fd;
}

int xSocket::connect(const xIpAddr& serverAddr, xIpAddr& localAddr){
    string err;
    int fd = xSockApi::tcpConnect(err,xSockApi::XFNET_CONNECT_NONE, serverAddr, localAddr );
    if (fd == XFNET_ERR){
         XFAST_ERROR<< "[xSocket::connect] accept error:" << err <<endl;
        return INVALID_SOCK;
    }

    return fd;
}

void xSocket::setTcpNoDelay(bool on){
    if (!valid()){
        throw xSocket_Exception("[xSocket::setTcpNoDelay] invalid socket");
    }
    string err;
    if (on){
        if (xSockApi::xSockOpt::enableTcpNoDelay(err, fd_) != XFNET_SUCC){
            throw xSocket_Exception("[xSocket::setTcpNoDelay] " + err);
        }
    }else{
        if (xSockApi::xSockOpt::disableTcpNoDelay(err, fd_) != XFNET_SUCC){
            throw xSocket_Exception("[xSocket::setTcpNoDelay] " +err);
        }
    }
    
}

// nonblock is set before connect or after accept(new connection is coming on server) or after listen(on a server)
void xSocket::setNonBlock(bool on){
    if (!valid()){
        throw xSocket_Exception("[xSocket::setNonBlock] invalid socket");
    }
    string err;
    if (on){
        if (xSockApi::xSockOpt::nonBlock(err, fd_) != XFNET_SUCC){
            throw xSocket_Exception("[xSocket::setNonBlock] "+ err);
        }
    }else{
        if (xSockApi::xSockOpt::block(err, fd_) != XFNET_SUCC){
            throw xSocket_Exception("[xSocket::setNonBlock] "+ err);
        }
    }
}

/* these are opened by default*/
void xSocket::setReuseAddr(){
    if (!valid()){
        throw xSocket_Exception("[xSocket::setReuseAddr] invalid socket");
    }
    string err;
    if (xSockApi::xSockOpt::setReuseAddr(err, fd_) != XFNET_SUCC){
        throw xSocket_Exception("[xSocket::setReuseAddr] " + err);
    }
}
void xSocket::setReusePort(){
    if (!valid()){
        throw xSocket_Exception("[xSocket::setReusePort] invalid socket");
    }
    string err;
    if (xSockApi::xSockOpt::setReusePort(err, fd_) != XFNET_SUCC){
        throw xSocket_Exception("[xSocket::setReusePort] " + err);
    }
}
void xSocket::setKeepAlive(){
     if (!valid()){
        throw xSocket_Exception("[xSocket::setReusePort] invalid socket");
    }
    string err;
    if (xSockApi::xSockOpt::setKeepAlive(err, fd_) != XFNET_SUCC){
        throw xSocket_Exception("[xSocket::setReusePort] " + err);
    }
}

}// namespace 


