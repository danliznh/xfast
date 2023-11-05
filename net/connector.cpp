#include "connector.h"
#include "socket.h"

namespace xfast{

void xConnector::doConnect(){
    loop_->scheduleToLoopThread(std::bind(&xConnector::beforeConnect, shared_from_this()));
}
void xConnector::doReConnect(){
    reConnMs_ = 0;
    startTimestamp_ = TNOWMS;
    status_= XF_CONN_STATUS_UNKOWN;
    doConnect();
}


void xConnector::beforeConnect(){
    assert(loop_->isLoopThread());
    int fd = xSocket::connectNonblock(peerAddr_);
    if (fd == INVALID_SOCK){
        XFAST_ERROR << "[xConnector::beforeConnect] connect to server(" << peerAddr_.toIpPort() << ") error:" << xSockApi::getError("")<<endl;
        setStatus(XF_CONN_STATUS_UNKOWN);
        xSocket sock(INVALID_SOCK);
        xIpAddr localAddr;
        connFinishedCb_(std::move(sock), localAddr, errno);
        return ;
    }

    setStatus(XF_CONN_STATUS_CONNECTING);
    loop_->reg(fd, 0, XE_WRITABLE, std::bind(&xConnector::onEventHandle, shared_from_this(), std::placeholders::_1));
    //loop_->runAfter(ReConnMaxTimeMs, std::bind(&xConnector::onCheckConnectionStatus, shared_from_this()));
}

void xConnector::onEventHandle(const xEvent& xe){

    if((xe.emask & XE_ERROR) || (xe.emask & XE_HUP) || getStatus() == XF_CONN_STATUS_DISCONNECTED){
        connectOnError(xe.efd, -4);
        return;
    }
    if(xe.emask & XE_WRITABLE){
        int rc = xSockApi::xSockOpt::getSocketError(xe.efd);
        if( rc != 0){
            XFAST_ERROR << "[xConnector::onEventHandle] connect to server(" << peerAddr_.toIpPort() << ") error:" << xSockApi::getError("")<<endl; 
            connectOnError(xe.efd, rc);
        }else{
            connectOnSucc(xe.efd);
        }
    }  
}

void xConnector::connectOnError(int fd, int errcode){
    setStatus(XF_CONN_STATUS_DISCONNECTED);
    loop_->remove(fd);
    ::close(fd);
    if(reConnMs_ < ReConnMaxTimeMs){
        XFAST_ERROR << "[xConnector::connectOnError] will reconnect in " << ReConnntervalMs << "ms"<<endl;
        loop_->runAfter(ReConnntervalMs, std::bind(&xConnector::onReConnect, shared_from_this()));
    }else{
         XFAST_ERROR << "[xConnector::connectOnError] connect failed , and reach max times " << ReConnMaxTimeMs<< "ms, quited"<<endl;
         xIpAddr localAddr;
         connFinishedCb_(std::move(xSocket(INVALID_SOCK)), localAddr, -7);
    }
}

void xConnector::connectOnSucc(int fd){
    // XFAST_INFO << "[xConnector::connectOnSucc] succ in" << TNOWMS - startTimestamp_ << "ms and fd is" << fd <<endl;
    xIpAddr localAddr;
    string err;
    int ret = xSockApi::getLocalAddr(err, fd, peerAddr_.family(),localAddr);
    if(ret){
         XFAST_ERROR << "[xConnector::connectOnSucc] get local address error:" <<  err <<endl;
    }
    
    setStatus(XF_CONN_STATUS_CONNECTED);
    connFinishedCb_(std::move(xSocket(fd)),localAddr, 0);
}

void xConnector::onCheckConnectionStatus(){
    XFAST_TRACE << "[xConnector::onCheckConnectionStatus] succ in" << TNOWMS - startTimestamp_ << "ms and status is " << status_ <<endl;
    if(getStatus() == XF_CONN_STATUS_CONNECTED){
        return;
    }
    
    setStatus(XF_CONN_STATUS_DISCONNECTED);
    xIpAddr localAddr;
    connFinishedCb_(std::move(xSocket(INVALID_SOCK)), localAddr, -7);
}


void xConnector::onReConnect(){
    XFAST_DEBUG << "[xConnector::onReConnect] do reconnect"<<endl;
    reConnMs_ += ReConnntervalMs;
    beforeConnect();
}

} //namespace
