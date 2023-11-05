
#include "option.h"
#include "xserver.h"

using namespace xfast;

class RollBackHandler: public xfast::xHandler{
public:
    void Connected(const xConnectionPtr connPtr){
        if(connPtr == nullptr){
            XFAST_ERROR << "Connected failed" << endl;
            exit(-1);
        }

        XFAST_DEBUG << "Connected:"<<  connPtr->GetConnFd() << endl;
        string str("hello client, welcome");
        connPtr->sendMsg(str);
    }

    int dispatch(int uid, xBuffer& buffer){
        XFAST_DEBUG << buffer.peek() << endl;
        int ret = sendResponse(uid, buffer);
        XFAST_DEBUG << "ret:"<<  ret << endl;
        return ret;
    }
};



int main(int argc, char *argv[]){
    xfast::xOption opt;
    opt.decode(argc, argv);

    string ip = opt.getValue("ip");
    if(ip.empty()){
        ip = "0.0.0.0";
    }
    int port = xUtil::strto<int>(opt.getValue("port"),"8081");
    XFAST_DEBUG << "ip:" << ip << ", port:" << port << endl;
    xLoopPtr loop(new xLoop);
    xServerPtr server(new xServer(loop,ip, port));
    server->start();
    server->registerHandler<RollBackHandler>(1);

   
    loop->start();

    XFAST_DEBUG << "loop finished" << endl;

    return 0;

}