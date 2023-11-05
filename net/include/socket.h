#ifndef XFAST_SOCKET_H__
#define XFAST_SOCKET_H__

#include "exception.h"
#include "noncopyable.h"
#include "sockapi.h"
#include "ipaddr.h"

namespace xfast{

/**
* @brief socket异常类
*/
struct xSocket_Exception : public xException
{
    xSocket_Exception(const string &buffer) : xException(buffer){};
    xSocket_Exception(const string &buffer, int err) : xException(buffer, err){};
    ~xSocket_Exception() throw() {};
};

class xSocket :public NoneCopyable
{

public:
    xSocket()
        :fd_(INVALID_SOCK){

        }
    
    explicit xSocket(int fd)
        :fd_(fd){

        }
    explicit xSocket(bool die); 

    //move constructor
    xSocket(xSocket&& rth){
        this->fd_ = INVALID_SOCK;
       
        std::swap(this->fd_, rth.fd_);
    }
    xSocket& operator=(xSocket&& rth){// move assignment
       if (this->fd_ != INVALID_SOCK){
            ::close(this->fd_ );
            this->fd_ = INVALID_SOCK;
        }
        std::swap(this->fd_, rth.fd_);
        return *this;
    }
    void create(int domain = AF_INET, int socketType = SOCK_STREAM);// using two-stage constructor
    
    ~xSocket();

public:
    int getFd()const;
    int valid();
    bool getTcpInfo(string& tcpInfo);
    
    int release(){
        int tmpFd = INVALID_SOCK;
        std::swap(tmpFd, fd_);
        return tmpFd;
    }

public:
    int bind(const xIpAddr& localAddr);
    int accept(xIpAddr& peerAddr);
    int listen();
    static int connectNonblock(const xIpAddr& serverAddr);
    static int connect(const xIpAddr& serverAddr, xIpAddr& localAddr);



/* tcp options*/
public:
  void setTcpNoDelay(bool on);
  void setNonBlock(bool on);
/* these are opened by default*/
  void setReuseAddr();  
  void setReusePort();
  void setKeepAlive(); 


private:
    // modification is not be allowed
    // but function init is allowed  
    int fd_; 
};
    
} // namespace




#endif //XFAST_SOCKET_H__

