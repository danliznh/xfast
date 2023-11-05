#ifndef XFAST_NETAPI_H__
#define XFAST_NETAPI_H__

#include <arpa/inet.h>
#include <netdb.h>

#include "netdef.h"
#include "exception.h"


namespace xfast{
namespace xNetApi{

struct xNetApiException : public xException
{
    xNetApiException(const string &buffer) : xException(buffer){};
    xNetApiException(const string &buffer, int err) : xException(buffer, err){};
    ~xNetApiException() throw() {};
};

template<class T, class U> 
T implicit_cast(U u) { return u; }

void netSetError(char *err, const char *fmt, ...);

int resolveNetAddr(const char *addr, int port,  int af,int st, struct addrinfo *sinfo,bool listen=false);

void toIpPort(char* buf, size_t size,
              const struct sockaddr* addr);
void toIp(char* buf, size_t size,
          const struct sockaddr* addr);

void fromIpPort(const char* ip, uint16_t port,
                struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port,
                struct sockaddr_in6* addr);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

const struct sockaddr* sockaddr_cast(struct sockaddr_storage* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr_storage* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr_storage* addr);
const struct sockaddr_storage* sockaddr_storage_cast(struct sockaddr* addr);
const struct sockaddr_storage* sockaddr_storage_in_cast(const struct sockaddr_in* addr);
const struct sockaddr_storage* sockaddr_storage_in6_cast(const struct sockaddr_in6* addr);

} // end namespace xNetApi
}// end namespace xfast


#endif //XFAST_NETAPI_H__ 

