
#include <mutex.h>
#include <iostream>
#include <cassert>
#include <pthread.h>
#include <string>
#include <cstring>
#include <errno.h>

using namespace std;

namespace xfast{

xMutex::xMutex(){
    int rc;
    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    assert(rc == 0);

    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    assert(rc == 0);

    rc = pthread_mutex_init(&_mutex, &attr);
    assert(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    assert(rc == 0);
    if(rc != 0)
    {
        cerr << "[xMutex::~xMutex] pthread_mutexattr_destroy error:" << string(strerror(rc)) << endl;
    }
}

xMutex::~xMutex(){
     int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    if(rc != 0)
    {
        cerr << "[xMutex::~xMutex] pthread_mutex_destroy error:" << string(strerror(rc)) << endl;
    }
}

void xMutex::lock()const {
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
        if(rc == EDEADLK)
    	{
            throw xMutexException("[xMutex::lock] pthread_mutex_lock dead lock error", rc);
    	}
    	else
    	{
            throw xMutexException("[xMutex::lock] pthread_mutex_lock error", rc);
    	}
    }
}


void xMutex::unlock()const{
     int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
        throw xMutexException("[xMutex::unlock] pthread_mutex_unlock error", rc);
    }
}

}