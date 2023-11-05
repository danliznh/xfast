
#include "listener.h"
#include "xkill.h"
#include "loger.h"


namespace xfast{

int xListener::doListen(const xIpAddr& listenAddr){
    try
    {
        listenSock_.create(listenAddr.family());
        listenSock_.setReuseAddr();
        listenSock_.setReusePort();
        listenSock_.bind(listenAddr);
        listenSock_.listen();
        listenSock_.setNonBlock(true);
        isListen_ = true;
        // XFAST_TRACE << "listen fd:" << listenSock_.getFd()<<endl;
        loop_->scheduleToLoopThread(std::bind(&xListener::addEventsForAccept, shared_from_this()));
    }
    catch(const std::exception& e)
    {
       XFAST_ERROR << "[xListener::doListen] execetion catched, it will killing, what:" << e.what() << endl;
       selfKill();
    }
    
    return 0;
}

// it will bind for reflecting events
void xListener::handleAccept(){
    xIpAddr peerAddr;
    int connfd = listenSock_.accept(peerAddr);
    if (connfd >= 0){
        // XFAST_TRACE << "Accepts of " << peerAddr.toIpPort()<<endl;
        if (newConnectionCallback_){
            try{
                xSocket sock(connfd);
                newConnectionCallback_(std::move(sock), peerAddr);
            }
            catch(const std::exception& e){
                ::close(connfd);// recycle by creator
                XFAST_ERROR << "[xListener::handleAccept] handle new connection exception, peer:"<< peerAddr.toIpPort()<<",what:"<< e.what()<<endl;
            }
    
        }else{
             XFAST_ERROR << "[xListener::handleAccept] no new Connection callback"<<endl;
            ::close(connfd);  // no call back set, close the connection
        }
    }else{
        XFAST_ERROR << "[xListener::handleAccept] accept a invalid socket, so continue agant"<<endl;
    }
}


void xListener::HandleEvent(const xEvent& xe){
    // XFAST_TRACE << "handler event on fd:" <<  xe.efd << ", mask:" << xe.emask <<endl;
    if((xe.emask & XE_ERROR) || xe.emask & XE_HUP){
        XFAST_TRACE << "handler error event on fd:" <<  xe.efd << ", mask:" << xe.emask << ", close the event" <<endl;
        loop_->remove(xe.efd);
        isListen_ = false;
    }else if(xe.emask & XE_READABLE){
        //  XFAST_TRACE << "handler read event on fd:" <<  xe.efd << ", mask:" << xe.emask <<endl;
         handleAccept();
    }

    assert((xe.emask & XE_WRITABLE) == 0);// no writable events

}


void xListener::addEventsForAccept(){
    // XFAST_TRACE << "add event for accept:" <<  listenSock_.getFd() <<endl;
    loop_->reg(listenSock_.getFd(), 0, XE_READABLE, std::bind(&xListener::HandleEvent, shared_from_this(), std::placeholders::_1));
}

}//namespace





