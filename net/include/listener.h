#ifndef XFAST_LISTENER_H__
#define XFAST_LISTENER_H__

#include "noncopyable.h"
#include "socket.h"
#include "epoller.h"
#include "xloop.h"



namespace xfast{

class xIpAddr;

typedef std::function<void (xSocket&& connSock, const xIpAddr&)> NewConnectionCallback;

class xListener : public NoneCopyable, public std::enable_shared_from_this<xListener>{

public:
    xListener(xLoopPtr loop, NewConnectionCallback cb)
        :isListen_(false), loop_(loop), newConnectionCallback_(cb){

        }
    ~xListener(){

    };


    int doListen(const xIpAddr& listenAddr);
    void handleAccept();  // it will bind for reflecting events

    void HandleEvent(const xEvent& xe);


    int getListenerFd()const{
        return listenSock_.getFd();
    }

    void setNewConnectionCb(NewConnectionCallback cb){
        newConnectionCallback_ = cb;
    }

    bool listening()const{
        return isListen_;
    }

private:
    void addEventsForAccept();

private:
    /* data */
    bool isListen_;
    xLoopPtr loop_;
    xSocket listenSock_;
    NewConnectionCallback newConnectionCallback_;
   
    
};

typedef std::shared_ptr<xListener> xListennerPtr;

}// namespace

#endif //XFAST_LISTENER_H__