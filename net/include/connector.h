#ifndef XFAST_CONNECTOR_H__
#define XFAST_CONNECTOR_H__

#include "noncopyable.h"
#include "ipaddr.h"
#include "xloop.h"
#include "socket.h"

#include <string>
#include <functional>
#include <memory>



using namespace std;

namespace xfast{

enum xConnStatus{
    XF_CONN_STATUS_UNKOWN = 0,
    XF_CONN_STATUS_CONNECTING = 1,
    XF_CONN_STATUS_CONNECTED = 2,
    XF_CONN_STATUS_TIMEOUT = 3,
    XF_CONN_STATUS_DISCONNECTING = 4,
    XF_CONN_STATUS_DISCONNECTED = 5,
};

class xConnector;
typedef std::shared_ptr<xConnector> xConnectorPtr;

typedef std::function<void (xSocket&& connSock, const xIpAddr& localAddr, int errcode)> ConnectedCallBack;

class xConnector: public NoneCopyable, public std::enable_shared_from_this<xConnector>{
    static const int ReConnntervalMs = 500;
    static const int ReConnMaxTimeMs = 3000;  
public:


    xConnector(xLoopPtr loop, const xIpAddr& peerAddr, ConnectedCallBack finishcb )
        :reConnMs_(0),startTimestamp_(TNOWMS),status_(XF_CONN_STATUS_UNKOWN),
        connFinishedCb_(finishcb), peerAddr_(peerAddr),loop_(loop){

        }

    
    void doConnect();

    void doReConnect();


    xConnStatus getStatus(){
        return status_;
    }

    string getStatusDesc();

    void setConnectionFinishedCallBack(ConnectedCallBack cb){
        connFinishedCb_ = cb;
    }

   

private:
    // the following apis run in loop thread
    void beforeConnect();
    void onEventHandle(const xEvent&);
    void connectOnError(int , int);
    void connectOnSucc(int fd);
    void setStatus(xConnStatus status){
        status_ = status;
    }

     void onCheckConnectionStatus();

private:
    void reConnect();
    void onReConnect();

private:
    int reConnMs_;
    int64_t  startTimestamp_;
    xConnStatus status_;
    ConnectedCallBack connFinishedCb_;
    xIpAddr peerAddr_;
    xLoopPtr loop_;

};



}// namespace 





#endif //XFAST_CONNECTOR_H__


