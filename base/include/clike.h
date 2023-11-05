#ifndef XFAST_CLIKE_H__
#define XFAST_CLIKE_H__

#include <stdint.h>
#include <string.h>  // memset
#include <string>

using namespace std;

namespace xfast{
inline void memzero(void* p, size_t n){
  memset(p, 0, n);
}


}// namespace




#endif //XFAST_CLIKE_H__


