#ifndef XFAST_ATOMIC_H__
#define XFAST_ATOMIC_H__

#include <stdint.h>

#include "noncopyable.h"

namespace xfast{

template<typename T>
class xAtomic : NoneCopyable{
public:
  xAtomic()
    : value_(0){
  }


  T get()
  {
    // in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)
    return __sync_val_compare_and_swap(&value_, 0, 0);
  }

  T getAndAdd(T x)
  {
    // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
    return __sync_fetch_and_add(&value_, x);
  }

  T addAndGet(T x)
  {
    return getAndAdd(x) + x;
  }

  T incrAndGet()
  {
    return addAndGet(1);
  }

  T decrAndGet()
  {
    return addAndGet(-1);
  }

  void add(T x)
  {
    getAndAdd(x);
  }

  void incr()
  {
    incrAndGet();
  }

  void decr()
  {
    decrAndGet();
  }

  T getAndSet(T newValue)
  {
    // in gcc >= 4.7: __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST)
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile T value_;
};


typedef xAtomic<int32_t> xAtomicInt32;
typedef xAtomic<int64_t> xAtomicInt64;

}// namespace

#endif //XFAST_ATOMIC_H__

