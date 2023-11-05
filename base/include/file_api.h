#ifndef XFAST_FILE_API_H__
#define  XFAST_FILE_API_H__


#include <unistd.h>
namespace xfast{

namespace xFileApi{


int readFd(int fd, void *buf, int count);
int writeFd(int fd, const void *buf, int count);

}// namespace xFileApi


}// namespace xfast



#endif // XFAST_FILE_API_H__