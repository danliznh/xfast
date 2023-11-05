#ifndef XFAST_XCLIENT_H__
#define XFAST_XCLIENT_H__


#include "xloop.h"
#include "connector.h"
#include "connection.h"
#include "mutex.h"

namespace xfast{


class xClient: public NoneCopyable{
public:
    xClient(xLoopPtr loop, const xIpAddr& serverAddr);
    ~xClient();

    void start();
    void stop();
    void setMsgDecodeCallBack(msgDecodeCallback mdcb){
        msgDecodeCb_ = mdcb;
    }
    void setConnectionFinishedCallBack(connectionFinishedCallBack cb){
        connFinishedCb_ = cb;
    }
    void setConnectionClosedCallBack(connectionClosedCallBack cb){
        connCloseCb_ = cb;
    }
    void setSentBytesCallBack(sentBytesCallBack cb){
        sentBytesCb_ = cb;
    }

    void setMsgCommitCallBack(msgCommitCallBack cb){
        msgCommitCb_ = cb;
    }

private:
    void connectedToServer(xSocket&& connSock, const xIpAddr& localAddr, int errcode);
    int defaultMsgDecode(const char*, int buffLen);
    void defaultSentBytes(const xConnectionPtr&, size_t bytes);
    void messageComing(const xConnectionPtr&, xBuffer& buffer);
    void connectionClosed(const xConnectionPtr&);
    void connectionFinished(const xConnectionPtr&);

private:
    void doStop();
    void doCloseConnection(const xConnectionPtr& connPtr);
private:
    bool isConnected_;
    xLoopPtr loop_;
    xIpAddr serverAddr_;
    xIpAddr localAddr_;
    xConnectorPtr connectorPtr_;
    xMutex lock_;
    xConnectionPtr connectionPtr_;

private:
    connectionFinishedCallBack connFinishedCb_;
    sentBytesCallBack sentBytesCb_;
    msgDecodeCallback msgDecodeCb_;
    msgCommitCallBack msgCommitCb_;
    connectionClosedCallBack connCloseCb_;
};


}//namespace


#endif //XFAST_XCLIENT_H__


