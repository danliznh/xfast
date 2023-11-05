#include "exception.h"
#include <execinfo.h>
#include <string.h>
#include <stdlib.h>
#include <cerrno>

namespace xfast
{

xException::xException(const string &buffer)
:_buffer(buffer), _code(0)
{
    getBacktrace();
}

xException::xException(const string &buffer, int err)
{
	_buffer = buffer + " :" + strerror(err);
    _code   = err;
    //getBacktrace();
}

xException::~xException() throw()
{
}

const char* xException::what() const throw()
{
    return _buffer.c_str();
}

void xException::getBacktrace()
{
    void * array[64];
    int nSize = backtrace(array, 64);
    char ** symbols = backtrace_symbols(array, nSize);

    for (int i = 0; i < nSize; i++)
    {
        _buffer += symbols[i];
        _buffer += "\n";
    }
	free(symbols);
}

}
