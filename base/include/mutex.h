
#ifndef XFAST_MUTEX_H__
#define XFAST_MUTEX_H__

#include <exception.h>
#include <noncopyable.h>
#include <iostream>
#include <cassert>
#include <pthread.h>
#include <cstring>
#include <errno.h>


namespace xfast{

struct xMutexException : public xException{
    xMutexException(const string &buffer) : xException(buffer){};
    xMutexException(const string &buffer, int err) : xException(buffer, err){};
    ~xMutexException() throw() {};
};


class xMutex : public NoneCopyable{
public:
  xMutex();
  virtual ~xMutex();

  /**
    * @brief 加锁
    */
  void lock() const;

  /**
    * @brief 解锁
    */
  void unlock() const;
protected:
  friend class xCondition;
  mutable pthread_mutex_t _mutex;
};



class  xMutexLockGuard : NoneCopyable
{
 public:
  explicit xMutexLockGuard(xMutex& mutex)
    : mutex_(mutex)
  {
    mutex_.lock();
  }

  ~xMutexLockGuard()
  {
    mutex_.unlock();
  }

 private:
    xMutex& mutex_;
};

// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock
#define xMutexLockGuard(x) error "Missing guard object name"


}// namespace
#endif // !XFAST_EPOLLER_H__

