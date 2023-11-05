#ifndef XFAST_THREAD_H__
#define XFAST_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <exception.h>
#include <noncopyable.h>

using namespace std;

namespace xfast{

typedef std::thread::id xThreadId;
namespace xCurrentThread{
    inline  xThreadId getTid(){
        return std::this_thread::get_id();
    }
}// namespace xCurrentThread


struct xThreadException : public xException{
    xThreadException(const string &buffer) : xException(buffer){};
    xThreadException(const string &buffer, int err) : xException(buffer, err){};
    ~xThreadException() throw() {};
};

class xThread:public NoneCopyable{ 
public:
    typedef std::function<void ()> ThreadFunc;
    explicit xThread(ThreadFunc, const string& name = string());
     ~xThread();

    void start();
    void join();
    pid_t tid() const { return tid_; }
    const string& name() const { return name_; }

private:
    
   
    bool started_;
    bool joined_;
    pid_t  tid_;
    string name_;
    ThreadFunc func_;
    thread th_;

};


}




#endif //XFAST_THREAD_H__
