#include "buffer.h"
#include "sockapi.h"
#include <errno.h>
#include <sys/uio.h>


namespace xfast{

// reduce copy from char buffer to xBuffer
int xBuffer::fillFromFd(int fd){
    char extrabuf[8192] = "\0";
    struct iovec vec[2];
    const ssize_t writable = writableBytes();
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const int iovcnt = (writable < (ssize_t)sizeof(extrabuf)) ? 2 : 1;
    ssize_t n = xSockApi::readvFd(fd, vec, iovcnt);
    if (n < 0){
        // do nothing
    }
    else if (n <= writable){
        writerIndex_ += n;
    }
    else{
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

  return n;
}

}// namespace

