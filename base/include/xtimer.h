#ifndef XFAST_XTIMER_H__
#define XFAST_XTIMER_H__

#include "xatomic.h"
#include "time_helper.h"
#include "loger.h"
#include <stdexcept>
#include <memory>
#include <string>

namespace xfast{

#define XTIMERID_PRE 4
#define XTIMERID_SUF 20

class xTimer;

class xTimerId{
public:

    xTimerId(xTimer* timer)
        :timer_(timer){

    }

    // only called by xLoop
    xTimer* getTimer(){
        return timer_;
    }

friend class xTimerQueue;
private:
    xTimer *timer_;
};

class xTimer;

typedef std::unique_ptr<xTimer> xTimerUinPtr;
typedef std::function<void ()> xTimerCallBack;

class xTimer{
public:
    class Cmp{
    public:
        bool operator()(const std::unique_ptr<xTimer> &lth, const std::unique_ptr<xTimer> & rth){
            if (lth->getExpiration() != rth->getExpiration()){
                return lth->getExpiration() < rth->getExpiration();
            }else{
                return lth->getSerial() < rth->getSerial();
            }
        }
    };


    xTimer(int64_t expiration, xTimerCallBack cb)
        :repeat_(1), interval_(0), expiration_(expiration),timerSerial_(s_nextSerial_.incrAndGet()), cb_(cb){

    }
    xTimer(int repeat, int interval, xTimerCallBack cb)
        :repeat_(repeat), interval_(interval),expiration_(TNOWMS+interval),timerSerial_(s_nextSerial_.incrAndGet()), cb_(cb) {

    }

    void nextRepeat(){
        assert(repeat_ > 0 || repeat_ == -1);
        if (repeat_ > 0){
              repeat_--;
        }
      
        expiration_ += interval_;
        if (expiration_ < TNOWMS){
            expiration_ = TNOWMS + interval_;
        }
    }

    bool hasNextTime()const{
        return repeat_ > 1 || repeat_ == -1;
    }

    bool isForever() const{
        return repeat_ == -1;
    }

    int64_t getSerial() const{
        return timerSerial_;
    }

    int getRepeat()const{
        return repeat_;
    }

    int getInterval()const{
        return interval_;
    }
    int64_t getExpiration()const{
        return expiration_;
    }
    void doCallBack(){
        try{
            cb_();
        }catch(const std::exception& ex){
            XFAST_ERROR << "[xTimer::doCallBack] catch exceptions, what:" << ex.what()<<endl;
        }
    }

    std::string toString(){
        std::stringstream ss;
        ss << "timerSerial:" << timerSerial_ << ",expiration:" << expiration_ << ",interval:" << interval_ << "," << repeat_;
        return ss.str();
    }
private:
    int repeat_;   // repeat times:-1 forever, 0:to stop timer, >0 repeat times
    int interval_; // interval of repeat timer
    int64_t expiration_;  // timeout; the timestamp should trigger
    int64_t timerSerial_;
    xTimerCallBack cb_;
public:
    static xAtomicInt64 s_nextSerial_;
};




}// namespace




#endif //XFAST_XTIMER_H__