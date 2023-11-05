#include "xclient.h"


namespace xfast{

void closeConnection(xLoopPtr loop, connectionClosedCallBack ccb,const xConnectionPtr& connPtr){
    XFAST_DEBUG << "[closeConnection] the connection, peer Addr:" << connPtr->getPeerName()  <<endl;
    ccb(connPtr);
    loop->scheduleToLoopThread(std::bind(&xConnection::setClosed, connPtr));
}


xClient::xClient(xLoopPtr loop, const xIpAddr& serverAddr)
    :isConnected_(false), loop_(loop),serverAddr_(serverAddr), 
     connectorPtr_(new xConnector(loop_, serverAddr, std::bind(&xClient::connectedToServer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))){

}


xClient::~xClient(){
    XFAST_DEBUG <<"[xClient::~xClient]"<<endl; 
    xConnectionPtr conn;
    {
        xMutexLockGuard lock(lock_);
        conn = connectionPtr_;
    }

    if(conn){ // there is a connection
        connectionClosedCallBack cb = std::bind(&closeConnection, loop_, connCloseCb_, std::placeholders::_1);
        loop_->scheduleToLoopThread(std::bind(&xConnection::setConnectionClosedCallBack, conn, cb));// to keep a shared pointer in the queue
    }
}

void xClient::start(){
    isConnected_ = true;
    connectorPtr_->doConnect();
}

void xClient::stop(){
    isConnected_ = false;
    xMutexLockGuard guad(lock_);// FIXME: is real thread safe????
    if(connectionPtr_){
        connectionPtr_->closeConnection();  // to close the connection
    }
}


void xClient::connectedToServer(xSocket&& connSock, const xIpAddr& localAddr, int errcode){
    if(errcode != 0){
        connectionFinished(nullptr);
        return;
    }else{
        xConnectionPtr connPtr(new xConnection(loop_, std::move(connSock), localAddr, serverAddr_,
                std::bind(&xClient::defaultMsgDecode, this, std::placeholders::_1, std::placeholders::_2),
                //nullptr,nullptr, nullptr, nullptr
                std::bind(&xClient::messageComing, this,std::placeholders::_1, std::placeholders::_2),
                std::bind(&xClient::defaultSentBytes, this,std::placeholders::_1, std::placeholders::_2),
                std::bind(&xClient::connectionClosed, this,std::placeholders::_1),
                std::bind(&xClient::connectionFinished, this, std::placeholders::_1)
                )
        );
        //connPtr->RegistEventToLoop();
        //msgDecodeCallback cb = std::bind(&xClient::defaultMsgDecode, this);
        //connPtr->setMsgDecodeCallBack(cb);
        
        {
            xMutexLockGuard guad(lock_);
            connectionPtr_ = connPtr;
        }

        loop_->scheduleToLoopThread(std::bind(&xConnection::makeConnected, connectionPtr_));

    }
}
 int xClient::defaultMsgDecode(const char* buffer, int buffLen){
     if(msgDecodeCb_){
         try{
            return msgDecodeCb_(buffer, buffLen);
         }catch(std::exception& e){
            XFAST_DEBUG << "[xClient::defaultMsgDecode] exception from msg decode, what: " << e.what() << endl;
            return XF_MSG_DECODE_ERR;// quit the buffer
         }
        
    }

    return buffLen;
 }

void xClient::defaultSentBytes(const xConnectionPtr& connPtr, size_t bytes){
    XFAST_DEBUG << "[xClient::defaultSentBytes] sent " << bytes << " bytes to "<<  connPtr->getPeerName() << endl;
    if (sentBytesCb_){
        sentBytesCb_(connPtr, bytes);
    }
}

void xClient::messageComing(const xConnectionPtr& connPtr, xBuffer& buffer){
    XFAST_DEBUG << "[xClient::messageComing] comming a message from " <<  connPtr->getPeerName() << ", len:" << buffer.readableBytes() << endl;
    if(msgCommitCb_){
        try{
            msgCommitCb_(connPtr, buffer);
        }catch(std::exception& e){
             XFAST_ERROR <<  "[xClient::messageComing] callback catched exception, what:" << e.what()<<endl; 
        }
    }
}

void xClient::connectionClosed(const xConnectionPtr& connPtr){
    XFAST_TRACE << "[xClient::connectionClosed] the connection before close, local addr:" << connPtr->getLocalName() << ", peer addr:" << connPtr->getPeerName() << endl;
    // wait all of the events finished in the loop
    loop_->scheduleToLoopThread(std::bind(&xClient::doCloseConnection, this, connPtr));
}

void xClient::doCloseConnection(const xConnectionPtr& connPtr){
    XFAST_TRACE << "[xClient::doCloseConnection] the connection closed, local addr:" << connPtr->getLocalName() << ", peer addr:" << connPtr->getPeerName() << endl;
    {
        xMutexLockGuard guad(lock_);
        connectionPtr_.reset();
    }

     connectionClosedCallBack cb = std::bind(&closeConnection, loop_, connCloseCb_, std::placeholders::_1);
    loop_->scheduleToLoopThread(std::bind(&xConnection::setConnectionClosedCallBack, connPtr, cb));// to keep a shared pointer in the queue
}

void xClient::connectionFinished(const xConnectionPtr& connPtr){
    if(connFinishedCb_){
        try{
            connFinishedCb_(connPtr);
        }catch(std::exception& e){
             XFAST_ERROR <<  "[xClient::connectionFinished] callback catched exception, what:" << e.what()<<endl; 
        }
    }
}



}