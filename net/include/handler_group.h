#ifndef XFAST_HANDLER_GROUP_H__
#define XFAST_HANDLER_GROUP_H__

#include <memory>
#include <vector>
#include "noncopyable.h"
#include "channel.h"
#include "buffer.h"
#include "handler.h"

namespace xfast{
class xServer;


class xHandlerGroup:public NoneCopyable, public std::enable_shared_from_this<xHandlerGroup>{

public:
    xHandlerGroup(std::shared_ptr<xServer> xServerPtr)
        :xServerWptr_(xServerPtr){

        }

    // default dtor is ok
    ~xHandlerGroup();

    template<class T>
    void registerHandler(size_t handlerNum);

    int sendResponse(int uid, xBuffer& buffer);

    void addMsg(xBuffer& buffer);

    size_t getMsgCount();
    size_t getActiveHandleCount();

    bool readMsg(xBuffer& buffer);

    void stopAllWorkers();

    void handleConnected(const xConnectionPtr connPtr);
    


friend class xHandler;
private:
    xChan<xBuffer> msgQueue_;
    std::vector<std::shared_ptr<xHandler> > handleWorkers_;
    std::weak_ptr<xServer> xServerWptr_;
};



typedef std::shared_ptr<xHandlerGroup> xHandlerGroupPtr;
typedef std::weak_ptr<xHandlerGroup> xHandlerGroupWeakPtr;

template<class T>
void xHandlerGroup::registerHandler(size_t handlerNum){
    for(size_t i = 0; i < handlerNum; ++i ){
        std::shared_ptr<xHandler> handler(new T);
        handler->setHandlerGroup(shared_from_this());
        handleWorkers_.push_back(handler);
    }
}

}// namespace





#endif //XFAST_HANDLER_GROUP_H__
