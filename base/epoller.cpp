
#include "epoller.h"
#include "loger.h"
#include<cassert>


using namespace std;

namespace xfast
{
xEpoller::xEpoller()
    :_efd(-1), _max_connections(0),_events(NULL){

}
xEpoller::~xEpoller(){
    if (NULL != _events){
        delete _events;
    }
    if (_efd >0){
        _efd = -1;
    }
    _max_connections = 0;
}


int xEpoller::Create(int max_conn){
    _max_connections = max_conn;
    _efd = epoll_create(_max_connections + 1);
    assert(_efd > 0);
   if (NULL != _events){
        delete _events;
    }
    _events = new epoll_event[_max_connections + 1];

    return 0;
}

void xEpoller::ctrl(int fd, long long data, int mask, int op){
    //XFAST_DEBUG <<"fd:"<<  fd << ",mask:" << mask <<endl;
    struct epoll_event ev = {0}; /* avoid valgrind warning */
    ev.data.fd = fd;
    ev.events   = mask;

    // if (ev.events&EPOLLIN){
    //      XFAST_DEBUG <<"fd:"<<  fd << ",XE_READABLE" <<endl;
    // }
    // if (ev.events&EPOLLOUT){
    //      XFAST_DEBUG <<"fd:"<<  fd << ",XE_WRITABLE" <<endl;
    // }
    int rc = epoll_ctl(_efd, op, fd, &ev);
    assert(rc == 0);
}

void xEpoller::Add(const xEvent& xe){
   
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    int op = EPOLL_CTL_ADD;
    
    int mask = 0;
    if (xe.emask & XE_READABLE) {
        mask |= EPOLLIN;
    }
    if (xe.emask & XE_WRITABLE){
        mask |= EPOLLOUT;
    }

    ctrl(xe.efd, xe.edata, mask, op);

}

void xEpoller::Modify(const xEvent& xe){
   
    int op = EPOLL_CTL_MOD;
    int mask= 0;
    if (xe.emask & XE_READABLE) {
        mask |= EPOLLIN;
    }
    if (xe.emask & XE_WRITABLE){
        mask |= EPOLLOUT;
    }

    ctrl(xe.efd, xe.edata, mask, op);

}

void xEpoller::Delete(const xEvent& xe){
   
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an DEL operation. */
    int op = EPOLL_CTL_DEL;
    
    int mask = 0;
    // int emask = xe.emask & (~delMask);
    // if (emask & XE_READABLE) {
    //     mask |= EPOLLIN;
    // }
    // if (emask & XE_WRITABLE){
    //     mask |= EPOLLOUT;
    // }

    ctrl(xe.efd, xe.edata, mask, op);

}


int xEpoller::Wait(int millsecond, std::list<xEvent>& lxe){
    int retval, numevents = 0;

    retval = epoll_wait(_efd, _events, _max_connections + 1, millsecond);
    if (retval > 0) {
        numevents = retval;
        for (int j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = _events+j;
            // XFAST_DEBUG << eventToStr(e->data.fd, e->events)<<endl;

            if (e->events & EPOLLIN) mask |= XE_READABLE;
            if (e->events & EPOLLOUT) mask |= XE_WRITABLE;
            if (e->events & EPOLLERR) mask |= XE_ERROR;
            if (e->events & EPOLLHUP) mask |= XE_HUP;

            xEvent xe(e->data.fd, e->data.u64, mask);
            lxe.push_back(xe);
        }
    }
    return numevents;
}


std::string xEpoller::eventToStr(int fd, int mask)
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (mask & EPOLLIN)
    oss << "POLLIN ";
  if (mask & EPOLLPRI)
    oss << "POLLPRI ";
  if (mask & EPOLLOUT)
    oss << "POLLOUT ";
  if (mask & EPOLLHUP)
    oss << "POLLHUP ";
  if (mask & EPOLLRDHUP)
    oss << "POLLRDHUP ";
  if (mask & EPOLLERR)
    oss << "POLLERR ";

  return oss.str();
}

};