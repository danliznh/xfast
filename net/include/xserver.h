#ifndef XFAST_SERVER_H__
#define XFAST_SERVER_H__

#include "noncopyable.h"
#include "connection.h"
#include "handler.h"
#include "handler_group.h"
#include "listener.h"
#include <map>

namespace xfast{

class xServer: public NoneCopyable, public std::enable_shared_from_this<xServer>{
public:
    typedef std::map<int, xConnectionPtr> connectionSet;

    xServer(xLoopPtr loop, const string& ip, int port);
    xServer(xLoopPtr loop, const string& ip, int port,
        msgDecodeCallback mdcb, NewConnectionCallback nccb, connectionClosedCallBack ccbc, sentBytesCallBack sbcb);
    ~xServer();

    void init();

    void setMsgDecodeCallBack(msgDecodeCallback mdcb){
        msgDecodeCb_ = mdcb;
    }
    void setNewConnectionCallBack(NewConnectionCallback cb){
        newConnCb_ = cb;
    }
    void setConnectionClosedCallBack(connectionClosedCallBack cb){
        connCloseCb_ = cb;
    }
    void setSentBytesCallBack(sentBytesCallBack cb){
        sentBytesCb_ = cb;
    }

    void setConnectionFinishedCallBack(connectionFinishedCallBack cb){
        connFinishedCb_ = cb;
    }

    template<class T>
    void registerHandler(size_t handlerNum);
    void start();
    int sendResponse(int uid, xBuffer& resp);

private:
    void acceptNewConnection(xSocket&& connSock, const xIpAddr& peerAddr);
    int defaultMsgDecode(const char* const, int buffLen);
    void defaultSentBytes(const xConnectionPtr&, size_t bytes);
    void messageComing(const xConnectionPtr&, xBuffer& buffer);
    void connectionClosed(const xConnectionPtr&);
    void connectionFinished(const xConnectionPtr&);




private:
    void doCloseConnection(const xConnectionPtr&);
    void doSendResponse(int uid, const string& buffer);

private:
    xLoopPtr loop_;
    xHandlerGroupPtr groupPtr_;
    connectionSet    connections_;
    xListennerPtr    listenner_;
    xIpAddr          localAddr_;
private:
    
    NewConnectionCallback newConnCb_;
    msgDecodeCallback msgDecodeCb_;
    sentBytesCallBack sentBytesCb_;
    msgCommitCallBack msgCommitCb_;
    connectionClosedCallBack connCloseCb_;
    connectionFinishedCallBack connFinishedCb_;


};

typedef std::shared_ptr<xServer> xServerPtr;


template<class T>
void xServer::registerHandler(size_t handlerNum){
    groupPtr_->registerHandler<T>(handlerNum); // started the handler thread after created the handler objects
    connFinishedCb_ = std::bind(&xHandlerGroup::handleConnected, groupPtr_, std::placeholders::_1); // set the default connected function
}


} // namespace xfast





#endif //XFAST_SERVER_H__
