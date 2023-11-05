
#ifndef XFAST_EXCEPTION_H__
#define XFAST_EXCEPTION_H__

#include <stdexcept>
using namespace std;

namespace xfast
{

/**
* @brief 异常类.
*/
class xException : public exception
{
public:
    /**
	 * @brief 构造函数，提供了一个可以传入errno的构造函数， 
	 *  
	 *  	  异常抛出时直接获取的错误信息
	 *  
	 * @param buffer 异常的告警信息 
     */
	explicit xException(const string &buffer);

    /**
	 * @brief 构造函数,提供了一个可以传入errno的构造函数， 
	 *  
	 *  	  异常抛出时直接获取的错误信息
	 *  
     * @param buffer 异常的告警信息 
     * @param err    错误码, 可用strerror获取错误信息
     */
	xException(const string &buffer, int err);

    /**
     * @brief 析够数函
     */
    virtual ~xException() throw();

    /**
     * @brief 错误信息.
     *
     * @return const char*
     */
    virtual const char* what() const throw();

    /**
     * @brief 获取错误码
     * 
     * @return 成功获取返回0
     */
    int getErrCode() { return _code; }

    void getBacktrace();

private:
    /**
	 * 异常的相关信息
     */
    string  _buffer;

	/**
	 * 错误码
     */
    int     _code;

};

}
#endif

