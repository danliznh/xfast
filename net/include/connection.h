#ifndef XFAST_CONNECTION_H__
#define XFAST_CONNECTION_H__

#include <memory>

#include "noncopyable.h"
#include "socket.h"
#include "buffer.h"
#include "connector.h"
#include "xloop.h"


using namespace std;

namespace xfast{

class xIpAddr;// declare forward


class xConnection;

typedef std::shared_ptr<xConnection> xConnectionPtr;

typedef enum{
    XF_MSG_DECODE_OK = 0,
    XF_MSG_DECODE_ERR = -1,
    XF_MSG_DECODE_NOT_ENOUGH = -2,
}XF_EN_MSG_DECODE;

//typedef std::function<void (int, const xIpAddr&, const xIpAddr&)> newConnectionCallback; // fd, localAddress, peerAddress
//typedef std::function<int (int, string& )> closeConnectionCallback; // fd and errmsg

typedef std::function<int (const char*, int buffLen)> msgDecodeCallback; // buffer start pointer
typedef std::function<void (const xConnectionPtr&, size_t bytes)> sentBytesCallBack;
typedef std::function<void (const xConnectionPtr&, xBuffer&) >  msgCommitCallBack;
typedef std::function<void (const xConnectionPtr&)> connectionClosedCallBack;
typedef std::function<void (const xConnectionPtr& connPtr)> connectionFinishedCallBack;

class xConnection: public NoneCopyable, public std::enable_shared_from_this<xConnection>{
public:

    // a connection from listeners
    xConnection(xLoopPtr loop, xSocket&& sock, const xIpAddr& localAddr, const xIpAddr& peerAddr, 
                msgDecodeCallback dcb, msgCommitCallBack mccb,sentBytesCallBack scb,connectionClosedCallBack ccb, connectionFinishedCallBack cfcb )
        :connStatus_(XF_CONN_STATUS_CONNECTING),writeEnabled_(false), loop_(loop), sock_(std::move(sock)), local_(localAddr), peer_(peerAddr), 
         sentBytesCb_(scb), decodeCb_(dcb), msgCommitCb_(mccb), connCloseCb_(ccb), connFinishCb_(cfcb){
        //loop_->reg(sock_.getFd(), 0, XE_READABLE|XE_WRITABLE, std::bind(&xConnection::HandleEvent, shared_from_this(), std::placeholders::_1));
        sock_.setNonBlock(true); // nonblock
    }
    ~xConnection(){
        XFAST_DEBUG << "close:" << sock_.getFd()<<endl;
        // do nothing
    }


    void RegistEventToLoop(){
        assert(loop_->isLoopThread());
        loop_->reg(sock_.getFd(), 0, XE_READABLE|XE_WRITABLE, std::bind(&xConnection::HandleEvent, shared_from_this(), std::placeholders::_1));
    }

    void RegistReadEvent(){
        assert(loop_->isLoopThread());
        loop_->reg(sock_.getFd(), 0, XE_READABLE, std::bind(&xConnection::HandleEvent, shared_from_this(), std::placeholders::_1));
    }


    void makeConnected(){
        assert(loop_->isLoopThread());
        XFAST_DEBUG <<"fd:" << sock_.getFd() << " connected" << endl;
        connStatus_ = XF_CONN_STATUS_CONNECTED;
        RegistReadEvent();

        connFinishCb_(shared_from_this());
    }

    void sendMsg(const string& buffer){
        XFAST_DEBUG <<"send msg status:" << connStatus_ << " size:" << buffer.size() << endl;
        if (connStatus_ != XF_CONN_STATUS_CONNECTED){
            return;
        }
        if(loop_->isLoopThread()){
            sendMsgToBuffer(buffer);
        }else{
            loop_->scheduleToLoopThread(std::bind(&xConnection::sendMsgToBuffer, shared_from_this(), buffer));
        }
    }

    void sendMsg(const xBuffer& buffer){
        XFAST_DEBUG <<"send msg status:" << connStatus_ << " size:" << buffer.readableBytes() << endl;
        if (connStatus_ != XF_CONN_STATUS_CONNECTED){
            return;
        }
         if(loop_->isLoopThread()){
            sendMsg(buffer.toString());
        }else{
            loop_->scheduleToLoopThread(std::bind(&xConnection::sendMsgToBuffer, shared_from_this(), buffer.toString()));
        }
    }

    void closeConnection(){
        if (connStatus_ != XF_CONN_STATUS_CONNECTED){
            return;
        }
        if(loop_->isLoopThread()){
            closeConn();
        }else{
            loop_->scheduleToLoopThread(std::bind(&xConnection::closeConn, shared_from_this()));
        }
    }

    void closeConnectionAfter(int milliseSeconds){
        if (connStatus_ != XF_CONN_STATUS_CONNECTED){
            return;
        }
        loop_->runAfter(milliseSeconds, std::bind(&xConnection::closeConn, shared_from_this()));
    }
    

    void setMsgDecodeCallBack(msgDecodeCallback cb){
        decodeCb_ = cb;
    }

    void setsentBytesCallBack(sentBytesCallBack cb){
        sentBytesCb_ = cb;
    }

    void setMsgCommitCallBack(msgCommitCallBack cb){
        msgCommitCb_ = cb;
    }

    void setConnectionFinishedCallBack(connectionFinishedCallBack cb){
        connFinishCb_ = cb;
    }

    void setConnectionClosedCallBack(connectionClosedCallBack cb){
        connCloseCb_ = cb;
    }

    string getPeerName(){
        return peer_.toIpPort();
    }

    string getLocalName(){
        return local_.toIpPort();
    }

    int GetConnFd(){
        return sock_.getFd();
    }

    bool isConnected(){
        return connStatus_ == XF_CONN_STATUS_CONNECTED;
    } 

    void setClosed(){
        connStatus_ = XF_CONN_STATUS_DISCONNECTED;
    }
    
protected:
    void HandleEvent(const xEvent& xe);
    void sendMsgToBuffer(const string& msg); // call by loop event
    int sendMsg();// call by descriptor event
    int readMsg();
    int closeConn();
private:
    void parseProtocol();
   
private:
    mutable xConnStatus connStatus_;
    bool writeEnabled_;
    xLoopPtr loop_;
    xSocket sock_;
    xIpAddr local_;
    xIpAddr peer_;
    xBuffer readBuff_;
    xBuffer writeBuff_;
    
    
    sentBytesCallBack sentBytesCb_;
    msgDecodeCallback decodeCb_;
    msgCommitCallBack msgCommitCb_;
    connectionClosedCallBack connCloseCb_;
    connectionFinishedCallBack connFinishCb_;
};



}// namespace




#endif //XFAST_CONNECTION_H__
