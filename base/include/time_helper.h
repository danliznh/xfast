#ifndef XFAST_TIME_HELPER_H__
#define XFAST_TIME_HELPER_H__

#include <memory>
#include <iostream>
#include <string>
#include <sys/time.h>
#include "xthread.h"
#include "condition.h"
#include "mutex.h"


using namespace std;

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define TNOW     xfast::xTimeHelper::getInstance()->getNow()
#define TNOWMS   xfast::xTimeHelper::getInstance()->getNowMs()


namespace xfast{

class xTimeHelper{

public:
    // 单件模式
    static xTimeHelper* getInstance();


    xTimeHelper() : _terminate(false),_use_tsc(true),_cpu_cycle(0),_buf_idx(0), 
                    cond_(mutex_),thread_(std::bind(&xTimeHelper::run, this))
    {
        memset(_t,0,sizeof(_t));
        memset(_tsc,0,sizeof(_tsc));

        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        _t[0] = tv;
        _t[1] = tv;
    }

   ~xTimeHelper(); 

   time_t getNow()     {  return _t[_buf_idx].tv_sec; }

    /**
     * @brief 获取时间.
     *
	 * @para timeval 
     * @return void 
     */
    void getNow(timeval * tv);

    /**
     * @brief 获取ms时间.
     *
	 * @para timeval 
     * @return void 
     */
    int64_t getNowMs();
    
    /**
     * @brief 获取cpu主频.
     *  
     * @return float cpu主频
     */  

    float cpuMHz();

protected:
    void run();
    void start();

private:
    void setTsc(timeval& tt);

    void addTimeOffset(timeval& tt,const int &idx);
protected:
    static xMutex   g_tm;
    static xTimeHelper*   g_th;

private:
    bool    _terminate;

    bool    _use_tsc;

    float           _cpu_cycle; 

    volatile int    _buf_idx;

    timeval         _t[2];

    uint64_t        _tsc[2];  

    xMutex mutex_;
    xCondition cond_;
    xThread thread_;
};


}



#endif // XFAST_TIME_HELPER_H__
