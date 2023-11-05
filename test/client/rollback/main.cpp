
#include "option.h"
#include "xclient.h"

using namespace xfast;

class RollBackClientHandler{
public:
    void connSucc(const xConnectionPtr& connPtr){
        if(connPtr == nullptr){
            XFAST_DEBUG << "connected failed, exit"<<endl;
            exit(-1);
        }
        XFAST_DEBUG << "on connected"<<endl;
        string hello = "hello";
        connPtr->sendMsg(hello);
    }
    void RecvMsg(const xConnectionPtr& connPtr, xBuffer& buffer){
          XFAST_DEBUG << "recv:"<< buffer.toString() <<endl;
          connPtr->sendMsg(buffer);
    }
    
};



int main(int argc, char *argv[]){
    xfast::xOption opt;
    opt.decode(argc, argv);

    string ip = opt.getValue("ip");
    if (ip.empty()){
        ip="127.0.0.1";
    }
    int port = xUtil::strto<int>(opt.getValue("port"),"8081");
    XFAST_DEBUG << "ip:" << ip << ", port:" << port << endl;
    xLoopPtr loop(new xLoop);
    
    xIpAddr serverAddr(ip, port);
    xClient* client = new xClient(loop,serverAddr);
    RollBackClientHandler* handler = new RollBackClientHandler();

    client->setConnectionFinishedCallBack(std::bind(&RollBackClientHandler::connSucc, handler, std::placeholders::_1));
    client->setMsgCommitCallBack(std::bind(&RollBackClientHandler::RecvMsg, handler, std::placeholders::_1, std::placeholders::_2));

    client->start();

   
    loop->start();
    client->stop();
    XFAST_DEBUG << "loop finished" << endl;

    return 0;

}