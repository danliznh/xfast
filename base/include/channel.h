#ifndef XFAST_CHANNEL_H__
#define XFAST_CHANNEL_H__

// 模仿go的channel, smart pointer required

#include <deque>
#include <noncopyable.h>

#include "xatomic.h"
#include "mutex.h"
#include "condition.h"

using namespace std;

namespace xfast{

template <typename T, typename D = deque<T> >
class xChan : public NoneCopyable{


public:
    typedef D queue_type;
    xChan() : cond_(mutex_){}; // constructor

    void push_back( const T& t );
    void push_front(const T& t);

    /**
     * @brief 从头部获取数据, 没有数据则等待.
     *
     * @param t 
	 * @param millsecond   阻塞等待时间(ms) 
	 *                    0 表示不阻塞 
     * 					 -1 永久等待
     * @return bool: true, 获取了数据, false, 无数据
     */
    bool pop_front(T& t, size_t millsecond = 0);


    size_t size() ;

    void clear();

    bool empty() ;

    void signal(); // 唤醒等待线程, 这里是broadcast的方式
protected:
    xAtomicInt32 size_;
    queue_type queue_;

    xMutex mutex_;
    xCondition cond_;

};



template<typename T, typename D> 
void xChan<T,D>::push_back(const T& t){
    size_.incr();
    xMutexLockGuard lock(mutex_);
    queue_.push_back(t);
    cond_.broadcast();
}

template<typename T, typename D> 
void xChan<T,D>::push_front(const T& t){
    size_.incr();
    xMutexLockGuard lock(mutex_);
    queue_.push_front(t);
    cond_.broadcast();
}

template<typename T, typename D> 
bool xChan<T,D>::pop_front(T& t, size_t millsecond){
    xMutexLockGuard lock(mutex_);
    if (size() == 0){
        if (millsecond == 0){
            return false;
        }else if (millsecond == -1){
            cond_.wait();
        }else{
            cond_.timedWait(millsecond);
        }
    }

    if(queue_.empty()){
        return false;
    }

    assert(size_.get()> 0);
    size_.decr();
     t = queue_.front();
    queue_.pop_front();
    
    return true;

}

template<typename T, typename D> 
size_t xChan<T,D>::size(){
    return (size_t)size_.get();
}

template<typename T, typename D> 
bool xChan<T,D>::empty(){
    return size_.get() == 0;
}

template<typename T, typename D> 
void xChan<T,D>::clear(){
    xMutexLockGuard lock(mutex_);
    size_.getAndSet(0);
    queue_.clear();
}

template<typename T, typename D> 
void xChan<T,D>::signal(){
    xMutexLockGuard lock(mutex_);
    cond_.broadcast();
}















}// namespace




#endif // XFAST_CHANNEL_H__



