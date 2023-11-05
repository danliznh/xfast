#ifndef XFAST_XLOOP_H__
#define XFAST_XLOOP_H__

#include "noncopyable.h"
#include "epoller.h"
#include "xtimer.h"
#include "xtimer_queue.h"
#include "xthread.h"
#include "mutex.h"
#include <map>
#include <list>
#include <memory>
#include <unistd.h>



namespace xfast{

typedef std::function<void()>  xFunctor;
typedef std::function<void (const xEvent& )> xEventCallBack;
struct xEventState{
    int mask;
    long long data;
    xEventCallBack callback;
    xEventState(int m, long long d, xEventCallBack c)
        :mask(m), data(d), callback(std::move(c)){

    }
    ~xEventState(){
        //XFAST_DEBUG << "fd:" << data <<endl;
    }

    void setData(int m, long long d, xEventCallBack c){
        mask = m;
        data = d;
        callback = std::move(c);
    }
};




class xLoop : public NoneCopyable, public std::enable_shared_from_this<xLoop>{

public:
    xLoop()
        :stop_(false), wakeupFd_(createEventfd()), threadId_(xCurrentThread::getTid()){
        timerQueue_ =  make_shared<xTimerQueue>();// new xTimerQueue();
        poller_.Create(1023);/* 1024 is just a hint for the kernel, added one in poller */
        reg(wakeupFd_, 0, XE_READABLE, std::bind(&xLoop::wakeUpCallBack, this, std::placeholders::_1));
    }

    ~xLoop(){
        ::close(wakeupFd_);
    }

    void stop(){
        stop_ = true;
    };
    void start();
    bool isLoopThread(){
        return xCurrentThread::getTid() == threadId_;
    }

    void scheduleToLoopThread(xFunctor handle);
// timers Api, always called by other threads
public:
    xTimerId runAfter(int interval, xTimerCallBack cb);
    xTimerId runAt(int expiration, xTimerCallBack cb);
    xTimerId runEvery(int interval, xTimerCallBack cb);
    xTimerId runRepeat(int repeat, int interval, xTimerCallBack cb);
    void removeTimer(xTimerId timerId);

public:
    int reg(int fd, long long data, int mask, xEventCallBack callback);
    int remove(int fd);
    int addEvent(int fd, int mask);
    int removeEvent(int fd, int mask);
    //int update(int fd, long long data, int mask);

protected:
    void process();
    int createEventfd();
    void wakeUpCallBack(const xEvent& xe);
    void notify();
protected:
    bool stop_;
    int wakeupFd_;
    xThreadId threadId_;
    xEpoller poller_;
    std::map<int, xEventState*> register_;
    std::list<xEvent> fired_;
    std::shared_ptr<xTimerQueue> timerQueue_;
    
    xMutex extraMutex_;
    std::list<xFunctor> extraEvents_;
    

};

typedef std::shared_ptr<xLoop> xLoopPtr;


}





#endif //XFAST_XLOOP_H__