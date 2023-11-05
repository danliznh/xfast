#include "connection.h"
#include "loger.h"

namespace xfast{

// runs in the loop thread
void xConnection::sendMsgToBuffer(const string& msg){\
    writeBuff_.append(msg); // FIXME more efficent????
    sendMsg();
}

int xConnection::sendMsg(){
    if(!isConnected()){
        XFAST_ERROR << "[xConnection::sendMsgToBuffer] connection is last, quit the mssage:" << writeBuff_.readableBytes()  <<endl;
        return -1;
    }
    
    int sent = 0;
    if(writeBuff_.readableBytes()){
        sent =  xSockApi::writeFd(sock_.getFd(), writeBuff_.peek(), (int)writeBuff_.readableBytes());
        if (sent < 0){
            if (errno == EAGAIN){
                return 0; //the write buffer is pull, it will notify again
            }
            XFAST_ERROR << "[xConnection::sendMsg] send error,close the connection, errno:" << errno <<endl;
            return -1;
        }

        writeBuff_.moveOver(sent);
        try{
            if (sentBytesCb_ != nullptr){
                sentBytesCb_( shared_from_this(),sent);
            }     
        }
        catch(const std::exception& e){
            XFAST_ERROR << "[xConnection::sendMsg] catch execption from sent bytes callback, what:" << e.what() <<endl;
        } 
    }

    if(writeBuff_.readableBytes() && writeEnabled_== false){
         loop_->addEvent(sock_.getFd(), XE_WRITABLE); // enable the write event again
         writeEnabled_ = true; 
    }
    if(writeBuff_.readableBytes() == 0 && writeEnabled_){
         loop_->removeEvent(sock_.getFd(), XE_WRITABLE);
        writeEnabled_ = false;
    }
    

    return sent;

}

int xConnection::readMsg(){
    int readSize = 0;
    while(true){
        int n = readBuff_.fillFromFd(sock_.getFd());
        XFAST_DEBUG << "[xCondition::readMsg] recv n bytes:"<< n <<", errno" << errno<<endl;
        if (n < 0){
            if (errno == EAGAIN){
                break; // no data in pool
            }
            XFAST_ERROR << "[xCondition::readMsg] an error from other side, quit the connection, errno" << errno<<endl;
            return -1;
        }

        if (n == 0){ // closed by other side
            XFAST_ERROR << "[xCondition::readMsg] recv zero byte in pool, will commit last msg and quit the connection"<<endl;
            return -1;// 
        }

        readSize += n;


    }
    
    if (readSize > 0){ // read data from client, try to parse portocal and commit messages
        parseProtocol();
    }

    return readSize;
    
}



int xConnection::closeConn(){
    assert(loop_->isLoopThread());
    XFAST_ERROR << "[xConnection::closeConn] now status:" << connStatus_<<endl;
    if(connStatus_ == XF_CONN_STATUS_DISCONNECTED || connStatus_ == XF_CONN_STATUS_DISCONNECTING){
        return 0;
    }
    connStatus_ = XF_CONN_STATUS_DISCONNECTING;
    
    loop_->remove(sock_.getFd());
    try
    {
        if(connCloseCb_)
            connCloseCb_(shared_from_this());
    }
    catch(const std::exception& e)
    {
        XFAST_ERROR << "[xConnection::closeConn] catch an exception from close callback, what:" << e.what()<<endl;
    }

    return 0;
}


void xConnection::HandleEvent(const xEvent& xe){
    XFAST_TRACE << "handle:" << xe.efd << ", mask:" << xe.emask<<endl;
    if (xe.emask & XE_ERROR || xe.emask & XE_HUP){
        closeConn();
        return;
    }

    if(xe.emask & XE_READABLE){  // read msg first
        int ret = readMsg();
        if (ret < 0){
            closeConn();
            return;
        }
    }

    if(xe.emask & XE_WRITABLE){
        int ret = sendMsg();
        if (ret < 0)
        {
            closeConn();
            return;
        }
    }
}


// only runs in loop thread
void xConnection::parseProtocol(){
    
    if(decodeCb_ != nullptr){
        try{
            while(readBuff_.readableBytes()){
                int bufferSize = readBuff_.readableBytes();
                int ret = decodeCb_(readBuff_.peek(), bufferSize);
                if(ret == XF_MSG_DECODE_ERR){
                    XFAST_ERROR << "[xConnection::parseProtocol] decode error, clean the buffer and try again"<<endl;
                    readBuff_.reset();
                    break;
                }else if(ret == XF_MSG_DECODE_NOT_ENOUGH){
                    // do nothing and try again next time
                    break;
                }else{
                    if(ret > bufferSize){
                        XFAST_INFO << "[xConnection::parseProtocol] the decode size is bigger than bufferSize, commit all buffed bytes" <<endl;
                        ret = bufferSize;
                    }

                    xBuffer commitBuffer(ret);
                    commitBuffer.append(readBuff_.peek(), ret);
                    readBuff_.moveOver(ret);
                    msgCommitCb_(shared_from_this(), commitBuffer);
                   XFAST_DEBUG << "[xConnection::parseProtocol] commit msg:" << commitBuffer.toString() <<endl;
                   
                }//end if
            } // end while(readBuff_.readableBytes())
            
        }
        catch(const std::exception& e)
        {
            XFAST_ERROR << "[xConnection::parseProtocol] catch an exception from callback, clean the buffer and try again, what:" << e.what()<<endl;
            readBuff_.reset();
            return;
        }
    }
}



}