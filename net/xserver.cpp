#include "xserver.h"
#include "loger.h"

namespace xfast{
xServer::xServer(xLoopPtr loop, const string& ip, int port)
    :loop_(loop), localAddr_(ip, port){
    //init();
}

xServer::xServer(xLoopPtr loop, const string& ip, int port,
        msgDecodeCallback mdcb, NewConnectionCallback nccb, connectionClosedCallBack cccb, sentBytesCallBack sbcb)
    :loop_(loop), localAddr_(ip, port), newConnCb_(nccb), msgDecodeCb_(mdcb), sentBytesCb_(sbcb),connCloseCb_(cccb){
    
    //init();
}

 void xServer::init(){
     
     // 这里必须要使用weak ptr管理shared pointer，group的sendresponse在不同的线程中
     groupPtr_ = std::make_shared<xHandlerGroup>(shared_from_this()); 

     // 这里可以直接绑定shared pointer， 因为xServer和xlistener销毁都在loop线程中
     NewConnectionCallback cb = std::bind(&xServer::acceptNewConnection, shared_from_this(), std::placeholders::_1, std::placeholders::_2);
     listenner_ = std::make_shared<xListener>(loop_,cb);

 }

 xServer::~xServer(){
    groupPtr_->stopAllWorkers();

    for (auto& item : connections_){
        xConnectionPtr conn(item.second); // keep the pointers
        item.second.reset();
        loop_->scheduleToLoopThread(std::bind(&xConnection::closeConnection, conn)); // maybe destroy in other thread
    }
 }

void xServer::start(){
    init();
    listenner_->doListen(localAddr_);
}



void xServer::acceptNewConnection(xSocket&& connSock, const xIpAddr& peerAddr){

    xConnectionPtr connPtr(new xConnection(loop_, std::move(connSock), localAddr_, peerAddr,
                std::bind(&xServer::defaultMsgDecode, shared_from_this() ,std::placeholders::_1, std::placeholders::_2),
                std::bind(&xServer::messageComing, shared_from_this(),std::placeholders::_1, std::placeholders::_2),
                std::bind(&xServer::defaultSentBytes, shared_from_this(),std::placeholders::_1, std::placeholders::_2),
                std::bind(&xServer::connectionClosed, shared_from_this(),std::placeholders::_1),
                std::bind(&xServer::connectionFinished, shared_from_this(), std::placeholders::_1)
                )
            );
    connections_[connPtr->GetConnFd()] = connPtr;

    loop_->scheduleToLoopThread(std::bind(&xConnection::makeConnected, connPtr));

}

int xServer::defaultMsgDecode(const char* const buffer, int buffLen){
    if(msgDecodeCb_){
        return msgDecodeCb_(buffer, buffLen);
    }

    return buffLen;
}


void xServer::defaultSentBytes(const xConnectionPtr& connPtr, size_t bytes){
    XFAST_TRACE << "[xServer::defaultSentBytes] sent " << bytes << " bytes to "<<  connPtr->getPeerName() << endl;
    if (sentBytesCb_){
        sentBytesCb_(connPtr, bytes);
    }
}

void xServer::messageComing(const xConnectionPtr& connPtr, xBuffer& buffer){
    XFAST_TRACE << "[xServer::messageComing] comming a message from " <<  connPtr->getPeerName() << ", len:" << buffer.readableBytes() << endl;
    if(groupPtr_->getActiveHandleCount() != 0){
        buffer.prependInt32(connPtr->GetConnFd());
        groupPtr_->addMsg(buffer);
    }else{
        XFAST_INFO << "[xServer::messageComing] there is no active handler for recv messgae, drop it. " << endl;
    }
}

void xServer::connectionClosed(const xConnectionPtr& connPtr){
    XFAST_TRACE << "[xServer::connectionClosed] the connection before close, local addr:" << connPtr->getLocalName() << ", peer addr:" << connPtr->getPeerName() << endl;
    // wait all of the events finished in the loop
    loop_->scheduleToLoopThread(std::bind(&xServer::doCloseConnection, shared_from_this(), connPtr));
}


void xServer::connectionFinished(const xConnectionPtr& connPtr){
    XFAST_TRACE << "[xServer::connectionFinished] the connection finished, local addr:" << connPtr->getLocalName() << ", peer addr:" << connPtr->getPeerName() << endl;
    if(connFinishedCb_ != nullptr){
        connFinishedCb_(connPtr);
    }
}

void xServer::doCloseConnection(const xConnectionPtr& connPtr){
    XFAST_TRACE << "[xServer::doCloseConnection] the connection closed, local addr:" << connPtr->getLocalName() << ", peer addr:" << connPtr->getPeerName() << endl;
    size_t n = connections_.erase(connPtr->GetConnFd());
    (void)n;
    assert(n == 1); // the connections only bind in loop
    if(connCloseCb_){
        try{
            connCloseCb_(connPtr);
        }catch(std::exception& e){
             XFAST_ERROR <<  "[xServer::doCloseConnection] callback catched exception, what:" << e.what()<<endl; 
        }
    }

    connPtr->setClosed();
}

// called by multi thread
int xServer::sendResponse(int uid, xBuffer& resp){
    string buffer = resp.readAll();
    loop_->scheduleToLoopThread(std::bind(&xServer::doSendResponse, shared_from_this(), uid, buffer));
    return 0;
}


// in loop thread
void xServer::doSendResponse(int uid, const string& buffer){
    auto it = connections_.find(uid);
    if (it == connections_.end()){
        XFAST_ERROR <<  "[xServer::doSendResponse] no connection to send, drop it:" << uid <<endl;
        return;
    }


    it->second->sendMsg(buffer);
}







}// namespace