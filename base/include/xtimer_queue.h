#ifndef XFAST_XTIMER_QUEUE_H__
#define XFAST_XTIMER_QUEUE_H__

#include <set>
#include <memory>

#include "xtimer.h"
#include "noncopyable.h"

using namespace std;

namespace xfast{

class xTimerQueue:public NoneCopyable, public std::enable_shared_from_this<xTimerQueue>{
    // default constructors and destructor
public:
    xTimerId AddTimer(xTimer * timer);
    void RemoveTime(xTimerId timerId);

    void triggerTimer();


private:
    std::set<xTimerUinPtr, xTimer::Cmp> timers_;
};


}//namespace









#endif //XFAST_XTIMER_QUEUE_H__
