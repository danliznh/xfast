
#include <iostream>

#include "ipaddr.h"
#include "loger.h"
#include "netapi.h"
#include "xendian.h"

using namespace std;

namespace xfast{

//      /* Structure describing a socket address.  */
//      struct sockaddr {  
//          sa_family_t sin_family;//地址族
// 　　     char sa_data[14]; //14字节，包含套接字中的目标地址和端口信息               
// 　　     }; 

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

//  /* for resovle ipv6 and select best addr */
//  struct addrinfo {
//      int ai_flags;   /* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
//      int ai_family;  /* PF_xxx */
//      int ai_socktype;    /* SOCK_xxx */
//      int ai_protocol;    /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
//      socklen_t ai_addrlen;   /* length of ai_addr */
//      char    *ai_canonname;  /* canonical name for hostname */
//      struct  sockaddr *ai_addr;  /* binary address */
//      struct  addrinfo *ai_next;  /* next structure in linked list */
// };

xIpAddr::xIpAddr(uint16_t port , bool lookback , bool ipv6){
    if (ipv6){
        memzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = lookback ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = xNetApi::hostToNetwork16(port);
    }
    else{
        memzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = lookback ? (in_addr_t)INADDR_LOOPBACK : (in_addr_t)INADDR_ANY;
        addr_.sin_addr.s_addr = xNetApi::hostToNetwork32(ip);
        addr_.sin_port = xNetApi::hostToNetwork16(port);
    }
}
xIpAddr::xIpAddr(const struct addrinfo* sinfo){
    setAddrInfo(sinfo);
}

xIpAddr::xIpAddr(const string& ip, uint16_t port, bool lookback ,  bool ipv6 ){
    if (ipv6){
        memzero(&addr6_, sizeof addr6_);
        xNetApi::fromIpPort(ip.c_str(), port, &addr6_);
    }
    else{
        memzero(&addr_, sizeof addr_);
        xNetApi::fromIpPort(ip.c_str(), port, &addr_);
    }
 }


 void xIpAddr::setAddrInfo(const struct addrinfo* sinfo){
    assert(sinfo != NULL);
    if (sinfo->ai_family == AF_INET6){
        addr6_ = *xNetApi::sockaddr_in6_cast(sinfo->ai_addr);
    }else{
        addr_ = *xNetApi::sockaddr_in_cast(sinfo->ai_addr);
    }
 }

size_t xIpAddr::getSockLen()const{
    if (family() == AF_INET6){
        return sizeof(addr6_);
    }else{
        return sizeof(addr_);
    }
}

string xIpAddr::toIpPort() const{
    char buf[256] = "";
    xNetApi::toIpPort(buf, sizeof(buf), getSockAddr());
    return buf;
    
}

string xIpAddr::toIp() const{
    char buf[256] = "";
    xNetApi::toIp(buf, sizeof(buf), getSockAddr());
    return buf;
}

uint32_t xIpAddr::ipNetEndian() const{
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t xIpAddr::toPort() const{
    return xNetApi::networkToHost16(portNetEndian());
}

uint16_t xIpAddr::portNetEndian() const {
    return addr_.sin_port;
}





}// namespace xfast