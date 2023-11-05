#ifndef XFAST_IPADDR_H__
#define XFAST_IPADDR_H__

#include <sys/socket.h> 
#include <netinet/in.h>
#include <memory>
#include <cassert>

#include "netapi.h"
#include "clike.h"


using namespace std;

namespace xfast{




class xIpAddr :public std::enable_shared_from_this<xIpAddr>{

public:
    explicit xIpAddr(uint16_t port = 0, bool lookback = false, bool ipv6 = false);

    xIpAddr(const string& ip, uint16_t port, bool lookback = false,  bool ipv6 = false);

    explicit xIpAddr(const struct addrinfo* sinfo);

    explicit xIpAddr(const struct sockaddr_in& addr)
        : addr_(addr){
    }

    explicit xIpAddr(const struct sockaddr_in6& addr)
        : addr6_(addr){
        
    }

    sa_family_t family() const { 
        return addr_.sin_family;
    }

    const struct sockaddr* getSockAddr() const { return xNetApi::sockaddr_cast(&addr6_); }
    size_t getSockLen() const;
    void setSockAddrIn6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }
    void setSockAddrIn(const struct sockaddr_in& addr){addr_ = addr;}
    void setAddrInfo(const struct addrinfo* sinfo);

    string toIp() const;
    string toIpPort() const;
    uint16_t toPort() const;
    

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const; 
private:
    union{
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
    
};


typedef std::shared_ptr<xIpAddr> xIpAddrPtr;

}// namespace xfast

#endif //XFAST_IPADDR_H__
