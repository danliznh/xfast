
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <cassert>
#include <iostream>
#include "netapi.h"
#include "xendian.h"
#include "loger.h"

using namespace std;

namespace xfast{
namespace xNetApi{

void netSetError(char *err, const char *fmt, ...){
    va_list ap;

    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, XNET_ERR_LEN, fmt, ap);
    va_end(ap);
}


int resolveNetAddr(const char *addr, int port,  int af,int st, struct addrinfo *sinfo,bool listen){
    int rc = -1;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints;

    snprintf(_port,6,"%d",port);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = st;
    if (listen){
        hints.ai_flags = AI_PASSIVE;    /* No effect if addr != NULL */
    }

    if ((rc = getaddrinfo(addr,_port,&hints,&sinfo)) != 0) {
        XFAST_ERROR << "syscall[getaddrinfo] error:" <<  gai_strerror(rc) << ",errno:" << rc<<endl;
        return XFNET_ERR;
    }

    return XFNET_SUCC;
    
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr){
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr){
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr){
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr){
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr){
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}


const struct sockaddr* sockaddr_cast(struct sockaddr_storage* addr){
   return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr_storage* addr){
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr_storage* addr){
   return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}
const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr* addr){
     return static_cast<const struct sockaddr_storage*>(implicit_cast<const void*>(addr));
}
const struct sockaddr_storage* sockaddr_storage_in_cast(const struct sockaddr_in* addr){
    return static_cast<const struct sockaddr_storage*>(implicit_cast<const void*>(addr));
}
const struct sockaddr_storage* sockaddr_storage_in6_cast(const struct sockaddr_in6* addr){
   return static_cast<const struct sockaddr_storage*>(implicit_cast<const void*>(addr));
}
// ip and port

void toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr){
    toIp(buf,size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = networkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}

void toIp(char* buf, size_t size, const struct sockaddr* addr){
    if (addr->sa_family == AF_INET){
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6){
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr){
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        XFAST_ERROR << "fromIpPort"<<endl;
    }
}

void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr){
    addr->sin6_family = AF_INET6;
    addr->sin6_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
    {
        XFAST_ERROR << "fromIpPort" <<endl;
    }
}



}// namespace xNetApi

}// namespace xfast