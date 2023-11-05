#include "handler.h"
#include "loger.h"
#include "handler_group.h"


namespace xfast{

void xHandler::run(){
    while(!isTerminate_){
        std::shared_ptr<xHandlerGroup> groupPtr = groupWeakPtr_.lock();
        if(groupPtr == nullptr){
            XFAST_INFO << "[xHandler::run] get shared pointer from weak pointer return null, so stop to dispatch messages"<<endl;
            stop();
        }
        xBuffer buffer;
        bool hasMsg = groupPtr->readMsg(buffer);
        if (hasMsg){
            XFAST_INFO << "[xHandler::run] wait one message, size:" << buffer.readableBytes() <<endl;
            try{
                int uid =buffer.readInt32(); // the uid is prepend to the buffer
                dispatch(uid, buffer);
            }catch(std::exception& e){
                XFAST_ERROR << "[xHandler::run] catch an exception from dispatch message, what:" << e.what() <<endl;
            }
        }
    }
    XFAST_INFO << "[xHandler::run] hander exit here" <<endl;
}


int xHandler::sendResponse(int uid, xBuffer& buffer){
     std::shared_ptr<xHandlerGroup> groupPtr = groupWeakPtr_.lock();
     if(groupPtr == nullptr){
        XFAST_INFO << "[xHandler::sendRsponse] get shared pointer from weak pointer return null, so stop to dispatch messages"<<endl;
        stop();
    }

    return groupPtr->sendResponse(uid, buffer);
}

 void xHandler::setHandlerGroup(std::shared_ptr<xHandlerGroup> groupPtr){
        groupWeakPtr_ = groupPtr;
    }

void xHandler::Connected(const xConnectionPtr connPtr){
     XFAST_INFO << "[xHandler::Connected]" << "connected:" << connPtr->GetConnFd() <<endl;
}

} // namespace xfas

