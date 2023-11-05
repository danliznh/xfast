#ifndef XFAST_CONDITION_H
#define XFAST_CONDITION_H

#include "exception.h"
#include "noncopyable.h"
#include "mutex.h"

namespace xfast{

struct xConditionException : public xException
{
    xConditionException(const string &buffer) : xException(buffer){};
    xConditionException(const string &buffer, int err) : xException(buffer, err){};
    ~xConditionException() throw() {};
};


/**
 *  @brief 线程信号条件类, 所有锁可以在上面等待信号发生
 */
class xCondition: public NoneCopyable{
public:

    /**
     *  @brief 构造函数
     */
    explicit xCondition(xMutex& mutex)
    : mutex_(mutex)
  {
    int rc = pthread_cond_init(&_cond, NULL);
    if (rc < 0){
        cerr << "[xCondition::~xCondition] pthread_cond_init error:" << string(strerror(rc)) << endl;
    }
  }

    /**
     *  @brief 析构函数
     */
    ~xCondition();

    /**
     *  @brief 发送信号, 等待在该条件上的一个线程会醒
     */
    void signal();

    /**
     *  @brief 等待在该条件的所有线程都会醒
     */
    void broadcast();

    /**
     *  @brief 获取绝对等待时间
     */
    timespec abstime(int millsecond) const;

    /**
	 *  @brief 无限制等待.
	 *  
     */
    void wait() const
    {
        //xMutexLockGuard gd(mutex_);
        int rc = pthread_cond_wait(&_cond, &mutex_._mutex);
        if(rc != 0)
        {
            throw xConditionException("[Condition::wait] pthread_cond_wait error", errno);
        }
    }

    /**
	 * @brief 等待时间. 
	 *  
	 * @param M 
     * @return bool, false表示超时, true:表示有事件来了
     */
    bool timedWait( int millsecond) const
    {

        timespec ts = abstime(millsecond);
        //xMutexLockGuard gd(mutex_);
        int rc = pthread_cond_timedwait(&_cond, &mutex_._mutex, &ts);

        if(rc != 0)
        {
            if(rc != ETIMEDOUT)
            {
                throw xConditionException("[Condition::timedWait] pthread_cond_timedwait error", errno);
            }

            return false;
        }
        return true;
    }
private:

    /**
     * 线程条件
     */
    xMutex &mutex_;
    mutable pthread_cond_t _cond;

};


}




#endif //XFAST_CONDITION_H

