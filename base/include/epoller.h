#ifndef XFAST_EPOLLER_H__
#define XFAST_EPOLLER_H__

#include <assert.h>
#include <sys/epoll.h>

#include <noncopyable.h>
#include <list>
#include <string>
#include <stdlib.h>

#include "time_helper.h"

namespace xfast{

#define XE_NONE 0       /* No events registered. */
#define XE_READABLE 1   /* Fire when descriptor is readable. */
#define XE_WRITABLE 2   /* Fire when descriptor is writable. */
#define XE_BARRIER 4    /* With WRITABLE, never fire the event if the */
                        /* READABLE event already fired in the same event */
                        /* loop iteration. Useful when you want to persist */
                        /* things to disk before sending replies, and want */
                        /* to do that in a group fashion. */
#define XE_ERROR    8    /* Fire when descriptor is error. */
#define XE_HUP      16  /* Fire when descriptor is hup*/


class xEvent{
public:
    int efd;
    int emask;
    long long edata;
    int64_t fireTime;
public:
    xEvent(int fd, long long data, int mask)
        :efd(fd), emask(mask), edata(data), fireTime(TNOWMS){

    }
    xEvent(const struct epoll_event *event){
        assert(NULL != event);
        efd = event->data.fd;

        int mask = 0;

        if (event->events & EPOLLIN) mask |= XE_READABLE;
        if (event->events & EPOLLOUT) mask |= XE_WRITABLE;
        if (event->events & EPOLLERR) mask |= XE_ERROR;
        if (event->events & EPOLLHUP) mask |= XE_HUP;
        
        emask = mask;
        edata = event->data.u64;
    }

    int Getfd()const{
        return efd;
    }

    int GetMask()const{
        return emask;
    }
    long long GetData() const{
        return edata;
    }
}; 




class xEventLoop;




class xEpoller: public NoneCopyable{

public:

    xEpoller();
    ~xEpoller();
    int Create(int max_conn);
    int GetFd()const{ return _efd;}
protected:
    void ctrl(int fd, long long data, int mask, int op);

public:
    void Add(const xEvent& xe);
    void Modify(const xEvent& xe);
    void Delete(const xEvent& xe);
    int Wait(int millsecond,  std::list<xEvent>& lxe);  // 返回本次事件个数

     struct epoll_event& GetEvent(int indx){  // 获取原始事件
       assert(_events != 0); 
       return _events[indx];
    };
private:
    std::string eventToStr(int fd, int mask);
private:
    int _efd;
    int _max_connections;
    struct epoll_event *_events;
};


};





#endif 
