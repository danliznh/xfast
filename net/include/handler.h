#ifndef XFAST_HANDLER_H__
#define XFAST_HANDLER_H__


#include <memory>
#include "channel.h"
#include "xthread.h"
#include "buffer.h"
#include "connection.h"



namespace xfast{

class xHandlerGroup;

class xHandler:public std::enable_shared_from_this<xHandler>{
public:
    xHandler()
        :isTerminate_(false), thread_(std::bind(&xHandler::run, this)){ // this pointer is safe ,we must join the thread in d-tor
            thread_.start();
        }

    virtual ~xHandler(){
        isTerminate_ = true;
        thread_.join();
    }

    void setHandlerGroup(std::shared_ptr<xHandlerGroup> groupPtr);

    bool isRunning()const{
        return isTerminate_ == false;
    }

    void stop(){
        isTerminate_ = true;
    }

    virtual int dispatch(int uid, xBuffer& buffer) = 0;

    virtual void Connected(const xConnectionPtr connPtr);

protected:
    int sendResponse(int uid, xBuffer& buffer);

private:
    void run();

private:
    mutable int isTerminate_;

    std::weak_ptr<xHandlerGroup> groupWeakPtr_;
    xThread thread_;
};


}








#endif //XFAST_HANDLER_H__
