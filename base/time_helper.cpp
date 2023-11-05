#include "time_helper.h"
#include "loger.h"




namespace xfast{

xMutex xTimeHelper::g_tm;
xTimeHelper* xTimeHelper::g_th = NULL;// std::make_shared<xTimeHelper>();


xTimeHelper* xTimeHelper::getInstance()
{
    if(!g_th)
    {
        {
            xMutexLockGuard lock(g_tm);
            if(!g_th){
                //g_th = std::make_shared<xTimeHelper>();
                g_th = new xTimeHelper();
                g_th->start();
            }
        }
        
    }
    return g_th;
}


void xTimeHelper::run(){

    while(!_terminate){
        timeval& tt = _t[!_buf_idx];

        ::gettimeofday(&tt, NULL);

        setTsc(tt);  

        _buf_idx = !_buf_idx;

        xMutexLockGuard lock(mutex_);
        cond_.timedWait(800); //修改800时 需对应修改addTimeOffset中offset判读值
    }
}

xTimeHelper::~xTimeHelper(){
    // 最小范围枷锁
    {
        _terminate = true;
        xMutexLockGuard lock(mutex_);
        cond_.signal(); 
    }
    
    thread_.join();
}

void xTimeHelper::start(){

    thread_.start();
}

void xTimeHelper::getNow(timeval *tv)  
{ 
    int idx = _buf_idx;
    *tv = _t[idx];

    if(_cpu_cycle != 0 && _use_tsc)//cpu-cycle在两个interval周期后采集完成
    {    
        addTimeOffset(*tv,idx); 
    }
    else
    {
        ::gettimeofday(tv, NULL);
    }
}
int64_t xTimeHelper::getNowMs()
{
    struct timeval tv;
    getNow(&tv);
    return tv.tv_sec * (int64_t)1000 + tv.tv_usec/1000;
}

float xTimeHelper::cpuMHz()
{
    if(_cpu_cycle != 0)
        return 1.0/_cpu_cycle;

    return 0;
}

void xTimeHelper::setTsc(timeval& tt)
{
    uint32_t low    = 0;
    uint32_t high   = 0;
    rdtsc(low,high);
    uint64_t current_tsc    = ((uint64_t)high << 32) | low;

    uint64_t& last_tsc      = _tsc[!_buf_idx];
    timeval& last_tt        = _t[_buf_idx];

    if(_tsc[_buf_idx] == 0 || _tsc[!_buf_idx] == 0 )
    {
        _cpu_cycle      = 0;
        last_tsc        = current_tsc;
    }
    else
    {
        time_t sptime   = (tt.tv_sec -  last_tt.tv_sec)*1000*1000 + (tt.tv_usec - last_tt.tv_usec);  
        _cpu_cycle      = (float)sptime/(current_tsc - _tsc[_buf_idx]); //us 
        last_tsc        = current_tsc;
    } 
}


void xTimeHelper::addTimeOffset(timeval& tt,const int &idx)
{
    uint32_t low    = 0;
    uint32_t high   = 0;
    rdtsc(low,high);
    uint64_t current_tsc = ((uint64_t)high << 32) | low;
    int64_t t =  (int64_t)(current_tsc - _tsc[idx]);
    time_t offset =  (time_t)(t*_cpu_cycle);
    if(t < -1000 || offset > 1000000){//毫秒
        _use_tsc = false;
        ::gettimeofday(&tt, NULL);
        return;
    }
    tt.tv_usec += offset;
    while (tt.tv_usec >= 1000000) { 
        tt.tv_usec -= 1000000; tt.tv_sec++;
    } 
}


}
