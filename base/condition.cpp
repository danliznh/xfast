#include "condition.h"
#include <string.h>
#include <cassert>
#include <iostream>
#include <cstdint>
#include <sys/time.h>


using namespace std;


namespace xfast
{

xCondition::~xCondition()
{
    int rc = 0;
    rc = pthread_cond_destroy(&_cond);
    if(rc != 0)
    {
        cerr << "[xCondition::~xCondition] pthread_cond_destroy error:" << string(strerror(rc)) << endl;
    }
//    assert(rc == 0);
}

void xCondition::signal()
{
    int rc = pthread_cond_signal(&_cond);
    if(rc != 0)
    {
        throw xConditionException("[xCondition::signal] pthread_cond_signal error", errno);
    }
}

void xCondition::broadcast()
{
    int rc = pthread_cond_broadcast(&_cond);
    if(rc != 0)
    {
        throw xConditionException("[xCondition::broadcast] pthread_cond_broadcast error", errno);
    }
}

timespec xCondition::abstime( int millsecond) const
{
    struct timeval tv;

    gettimeofday(&tv, 0);

    int64_t it  = tv.tv_sec * (int64_t)1000000 + tv.tv_usec + (int64_t)millsecond * 1000;

    tv.tv_sec   = it / (int64_t)1000000;
    tv.tv_usec  = it % (int64_t)1000000;

    timespec ts;
    ts.tv_sec   = tv.tv_sec;
    ts.tv_nsec  = tv.tv_usec * 1000; 
      
    return ts; 
}


}

