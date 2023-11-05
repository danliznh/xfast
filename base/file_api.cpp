
#include "file_api.h"
#include <errno.h>
#include <unistd.h>

using namespace std;

namespace xfast{

namespace xFileApi{



/* Like read(2) but make sure 'count' is read before to return
 * (unless error or EOF condition is encountered) */
int readFd(int fd, void *buf, int count){
    char* buffer = (char* )buf;
    ssize_t nread, totlen = 0;
    while(totlen != count) {
        nread = ::read(fd,buffer,count-totlen);
        if (nread == 0) return totlen;
        if (nread == -1) {
            if (errno == EINTR) continue;  // there is no EINTR in nonblock read
            return -1;
        }
        totlen += nread;
        buffer += nread;
    }
    return totlen;
}

/* Like write(2) but make sure 'count' is written before to return
 * (unless error is encountered) */
int writeFd(int fd,const void *buf, int count){
    char* buffer = (char* )buf;
    ssize_t nwritten, totlen = 0;
    while(totlen != count) {
        nwritten = ::write(fd,buffer,count-totlen);
        if (nwritten == 0) return totlen;
        if (nwritten == -1) {
            if (errno == EINTR) continue;  // there is no EINTR in nonblock write
            return -1;
        }
        totlen += nwritten;
        buffer += nwritten;
    }
    return totlen;
}


}// namespace xFileApi


}// namespace xfast