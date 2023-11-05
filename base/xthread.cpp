
#include <xthread.h>
#include <sys/prctl.h>
#include "loger.h"

namespace xfast{

struct xThreadData
{
    xThread::ThreadFunc func_;
    string name_;
    pid_t* tid_;

    xThreadData(xThread::ThreadFunc func, const string& name, pid_t* tid)
        :func_(std::move(func)),name_(name),tid_(tid){
    }

void runInThread(){
    // name_ = name_.empty() ? "xfastThread" : name_.c_str();
    // ::prctl(PR_SET_NAME, name_.c_str());
    XFAST_DEBUG << "start thread here, tid:" << *tid_<<endl;
    try{
        func_();
    }
    catch (const xException& ex) {
        fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (const std::exception& ex){
        fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...){
        fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
        throw; // rethrow
    }
}
};

void startThread(void* obj){
    xThreadData* data = static_cast<xThreadData*>(obj);
    data->runInThread();
    delete data;
}


xThread::xThread(ThreadFunc func, const string& name)
    :started_(false), joined_(false), tid_(0),
    name_(name), func_(std::move(func)){

}

xThread::~xThread(){
    if (started_ && !joined_){
        th_.detach();
    }
}


void xThread::start(){
    started_ = true;
    xThreadData* xtData = new xThreadData(func_, name_, &tid_);
    th_ = std::thread (startThread,xtData);
}

void xThread::join(){
    joined_ = true;
    th_.join();
}




}
