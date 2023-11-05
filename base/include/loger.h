#ifndef XFAST_LOGER_H__
#define XFAST_LOGER_H__

#include "util.h"
#include <iostream>

using namespace std;

namespace xfast{

#define XFAST_LOG(level)  cout << "["<<xUtil::now2strLog()<<"|" <<  __FILE__ <<":"<< __LINE__<< " " << level <<"] "<<  __func__ <<" "


#define XFAST_DEBUG XFAST_LOG("DEBUG")
#define XFAST_ERROR XFAST_LOG("ERROR")
#define XFAST_TRACE XFAST_LOG("TRACE")
#define XFAST_INFO XFAST_LOG("INFO")
#define XFAST_WARN XFAST_LOG("WARN")

}

#endif //XFAST_LOGER_H__
