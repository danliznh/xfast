#include "handler_group.h"
#include "handler.h"
#include "xserver.h"
#include "loger.h"

namespace xfast{


xHandlerGroup::~xHandlerGroup(){
    for (auto &item : handleWorkers_){
        item->stop();
    }
}


int xHandlerGroup::sendResponse(int uid, xBuffer& buffer){
     std::shared_ptr<xServer> srvPtr = xServerWptr_.lock();
     int ret = 0;
     if (srvPtr != nullptr){
        ret = srvPtr->sendResponse(uid, buffer);
         if(ret != 0){
             XFAST_ERROR << "[xHandlerGroup::sendResponse] send response error:" << ret <<endl;
         }
     }else{
         XFAST_ERROR << "[xHandlerGroup::sendResponse] get shared pointer from weak pointer failed." <<endl;
     }

     return ret;
}

void xHandlerGroup::addMsg(xBuffer& buffer){
    msgQueue_.push_back(buffer);
    XFAST_DEBUG << "[xHandlerGroup::getActiveHandleCount] queue message count:" << msgQueue_.size() <<endl;
}

bool xHandlerGroup::readMsg(xBuffer& buffer){
    return msgQueue_.pop_front(buffer);
}

size_t xHandlerGroup::getMsgCount(){
    return msgQueue_.size();
}

size_t xHandlerGroup::getActiveHandleCount(){
    size_t count = 0;
    XFAST_DEBUG << "[xHandlerGroup::getActiveHandleCount] worker count:" << handleWorkers_.size() <<endl;
    for (auto &item : handleWorkers_){
        if(item->isRunning()){
            count++;
        }
    }
    XFAST_DEBUG << "[xHandlerGroup::getActiveHandleCount] active count:" << count <<endl;
    return count;
}

 void xHandlerGroup::stopAllWorkers(){
    for (auto &item : handleWorkers_){
        item->stop();
    }
 }

void xHandlerGroup::handleConnected(const xConnectionPtr connPtr){
    bool ishandle = false;
    for (auto &item : handleWorkers_){
        if(!item->isRunning())
            continue;
        
        ishandle = true;
        item->Connected(connPtr);
        break;// only one handler is notified
    }

    if (ishandle == false){
        XFAST_ERROR << "[xHandlerGroup::handleConnected] no handler for handle connected event." <<endl;
    }
}


} // namespace xfast
