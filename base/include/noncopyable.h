
  
#ifndef XFAST_NONCOPYABLE__
#define XFAST_NONCOPYABLE__

namespace xfast{
    class NoneCopyable{
    public:
    protected:
        //= constructors and destructor

        NoneCopyable(){}

        ~NoneCopyable(){}
    private:
        // emphasize the following members are private 
        NoneCopyable(const NoneCopyable& rth);
        const NoneCopyable& operator= (const NoneCopyable& rth);

    };
}

#endif
