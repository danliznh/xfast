#include "xloop.h"
#include "loger.h"
#include "file_api.h"

#include <sys/eventfd.h>
#include <signal.h>

namespace xfast{



#pragma GCC diagnostic ignored "-Wold-style-cast"
class xIgnoreSignals{
public:
    xIgnoreSignals(){
        ::signal(SIGPIPE, SIG_IGN);
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

xIgnoreSignals initObj;


void xLoop::start(){
    while(!stop_){
        process();
    }
}

void xLoop::process(){
    int eventnum = poller_.Wait(500,fired_);
    (void)eventnum;
    while(!fired_.empty()){
        const xEvent& xe = fired_.front();   
        map<int, xEventState*>::iterator it = register_.find(xe.efd) ;
        if (it != register_.end()){
            it->second->callback(xe);
        }else{
            XFAST_DEBUG << "fd:" << xe.efd << "missing callback"<<endl;
        }
        fired_.pop_front();
    }

    // run events
    for(auto &fun : extraEvents_){
            fun();
    }
    extraEvents_.clear();


    // handle timers
    timerQueue_->triggerTimer();
}


int xLoop::reg(int fd, long long data, int mask, xEventCallBack callback){
    assert(isLoopThread());
    //XFAST_TRACE <<"event add fd:" << fd << ",data: " << data << ",mask:"<< mask <<endl;
   
   xEventState* xes = nullptr;
    bool mod = false;
    if (register_.find(fd) != register_.end()){
        mod = true;
        xes = register_[fd];
        xes->setData(mask, data, std::move(callback));
    }else{
        xes = new xEventState(mask, data, std::move(callback));
    }

    register_[fd] = xes;

   xEvent xe(fd, data, mask);
   if (mod){
        poller_.Modify(xe);       
   }else{
        poller_.Add(xe);
   }
   
   return 0;
}
int xLoop::remove(int fd){
    assert(isLoopThread());
    map<int, xEventState*>::iterator it = register_.find(fd) ;
    if (it  == register_.end()){
        return 0;
    }

    xEventState* xes = it->second;
    xEvent xe(fd, xes->data, XE_NONE);
    poller_.Delete(xe);
    register_.erase(it);
    delete xes;
    return 0;
}

int xLoop::addEvent(int fd, int mask){
    assert(isLoopThread());
    map<int, xEventState*>::iterator it = register_.find(fd) ;
    if (it  == register_.end()){
        return 0;
    }
    xEventState* xes = it->second;
    if((xes->mask & mask) != 0){ // event already registered
        return 0;
    }

    xes->mask |= mask;

    xEvent xe(fd, xes->data, xes->mask);
    poller_.Modify(xe);
    return 0;
}
int xLoop::removeEvent(int fd, int mask){
    assert(isLoopThread());
    map<int, xEventState*>::iterator it = register_.find(fd) ;
    if (it  == register_.end()){
        return 0;
    }

    xEventState* xes = it->second;
    if((xes->mask & mask) == 0){ 
        return 0;
    }
    xes->mask |= (~mask);
    if (xes->mask == XE_NONE){
        return remove(fd);
    }

    xEvent xe(fd, xes->data, xes->mask);
    poller_.Modify(xe);
    return 0;
}

void xLoop::scheduleToLoopThread(xFunctor handle){
    if(isLoopThread()){
        handle();
    }else{
        xMutexLockGuard guard(extraMutex_);
        extraEvents_.push_back(handle);
        notify();// to wake up the event loop
    }
}

xTimerId xLoop::runAfter(int interval, xTimerCallBack cb){
    xTimerUinPtr timerPtr(new xTimer(TNOWMS + interval, cb));
    xTimerId timerId(timerPtr.get());
    xFunctor handle = std::bind(&xTimerQueue::AddTimer, timerQueue_, timerPtr.release());
    scheduleToLoopThread(handle);

    return timerId;

}
xTimerId xLoop::runAt(int expiration, xTimerCallBack cb){
    xTimerUinPtr timerPtr(new xTimer(expiration, cb));
    xTimerId timerId(timerPtr.get());
    xFunctor handle = std::bind(&xTimerQueue::AddTimer, timerQueue_, timerPtr.release());
    scheduleToLoopThread(handle);

    return timerId;
}
xTimerId xLoop::runEvery(int interval, xTimerCallBack cb){
    xTimerUinPtr timerPtr(new xTimer(-1,interval , cb));
    xTimerId timerId(timerPtr.get());
    xFunctor handle = std::bind(&xTimerQueue::AddTimer, timerQueue_, timerPtr.release());
    scheduleToLoopThread(handle);

    return timerId;
}
xTimerId xLoop::runRepeat(int repeat, int interval, xTimerCallBack cb){
    assert(repeat > 0);
    xTimerUinPtr timerPtr(new xTimer(repeat,interval , cb));
    xTimerId timerId(timerPtr.get());
    xFunctor handle = std::bind(&xTimerQueue::AddTimer, timerQueue_, timerPtr.release());
    scheduleToLoopThread(handle);

    return timerId;
}
void xLoop::removeTimer(xTimerId timerId){
    xFunctor handle = std::bind(&xTimerQueue::RemoveTime, timerQueue_, timerId); // create xTimerId with the default contructor
    scheduleToLoopThread(handle);
}


int xLoop::createEventfd(){
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0){
        cerr << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

void xLoop::notify(){
    uint64_t one = 1;
    xfast::xFileApi::writeFd(wakeupFd_, &one, sizeof(one));
}

void xLoop::wakeUpCallBack(const xEvent& xe){
    XFAST_TRACE <<"wakeup fd:" << xe.efd  << ",mask:"<< xe.emask <<endl;
    uint64_t one = 1;
    xfast::xFileApi::readFd(wakeupFd_, &one, sizeof(one));
}

}// namespace