#include "xtimer_queue.h"


namespace xfast{


xTimerId xTimerQueue::AddTimer(xTimer * timer){
    XFAST_TRACE << "[xTimerQueue::AddTimer] add timer:" << timer->toString()<<endl;
    xTimerUinPtr timerPtr(timer);
    xTimerId timerId(timer);
    timers_.insert(std::move(timerPtr));

    return timerId;
}

void xTimerQueue::RemoveTime(xTimerId timerId){
    xTimerUinPtr timerPtr(timerId.timer_);
    timers_.erase(std::move(timerPtr)); // 转引用， 所有权没有变

    timerPtr.release();// 释放所有权
}


void xTimerQueue::triggerTimer(){
    int64_t timeNow =TNOWMS;
    auto it = timers_.begin();
    while(it != timers_.end()){
        //XFAST_TRACE << (*it)->toString() << ", now:" << timeNow <<endl;
        if((*it)->getExpiration() > timeNow){
            break;
        }

        (*it)->doCallBack();

        if((*it)->hasNextTime()){  // delete the stoped timers
             (*it)->nextRepeat();
             it++;
        }else{
            it = timers_.erase(it);
            continue;
        }
    }
}

}// namespace