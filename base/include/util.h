#ifndef XFAST_UTIL_H__
#define XFAST_UTIL_H__

#include <string>
#include <sstream> // stringstream
#include <vector>

#include "loki.h"

using namespace std;

namespace xfast{

class xUtil{
public:
    static string tm2str(const struct tm &stTm, const string &sFormat = "%Y%m%d%H%M%S");
    static string tm2str(const time_t &t, const string &sFormat = "%Y%m%d%H%M%S");
    static string now2str(const string &sFormat = "%Y%m%d%H%M%S");
    static string now2strLog();

    template<typename T>
    static string tostr(const T &t);

    template<typename T>
    static string tostr(const vector<T> &t);


    template<typename T>
    static T strto(const string &sStr);
    template<typename T>
    static T strto(const string &sStr, const string &sDefault);

}; // end util


namespace p
{
    template<typename D>
    struct strto1
    {
        D operator()(const string &sStr)
        {
            string s = "0";

            if(!sStr.empty())
            {
                s = sStr;
            }

            istringstream sBuffer(s);

            D t;
            sBuffer >> t;

            return t;
        }
    };

	template<>
    struct strto1<char>
    {
        char operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return sStr[0];
			}
			return 0;
        }
    };

	template<>
    struct strto1<short>
    {
        short operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return atoi(sStr.c_str());
			}
			return 0;
        }
    };

	template<>
    struct strto1<unsigned short>
    {
        unsigned short operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return strtoul(sStr.c_str(), NULL, 10);
			}
			return 0;
        }
    };

	template<>
    struct strto1<int>
    {
        int operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return atoi(sStr.c_str());
			}
			return 0;
        }
    };

	template<>
    struct strto1<unsigned int>
    {
        unsigned int operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return strtoul(sStr.c_str(), NULL, 10);
			}
			return 0;
        }
    };

	template<>
    struct strto1<long>
    {
        long operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return atol(sStr.c_str());
			}
			return 0;
        }
    };

	template<>
    struct strto1<long long>
    {
        long long operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return atoll(sStr.c_str());
			}
			return 0;
        }
    };

	template<>
    struct strto1<unsigned long>
    {
        unsigned long operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return strtoul(sStr.c_str(), NULL, 10);
			}
			return 0;
        }
    };

	template<>
    struct strto1<float>
    {
        float operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return atof(sStr.c_str());
			}
			return 0;
        }
    };

	template<>
    struct strto1<double>
    {
        double operator()(const string &sStr)
        {
            if(!sStr.empty())
			{
				return atof(sStr.c_str());
			}
			return 0;
        }
    };

    template<typename D>
    struct strto2
    {
        D operator()(const string &sStr)
        {
            istringstream sBuffer(sStr);

            D t;
            sBuffer >> t;

            return t;
        }
    };

    template<>
    struct strto2<string>
    {
        string operator()(const string &sStr)
        {
            return sStr;
        }
    };

}

template<typename T>
T xUtil::strto(const string &sStr)
{
    typedef typename TL::TypeSelect<TL::TypeTraits<T>::isStdArith, p::strto1<T>, p::strto2<T> >::Result strto_type;

    return strto_type()(sStr);
}

template<typename T>
T xUtil::strto(const string &sStr, const string &sDefault)
{
    string s;

    if(!sStr.empty())
    {
        s = sStr;
    }
    else
    {
        s = sDefault;
    }

    return strto<T>(s);
}


}




#endif //XFAST_UTIL_H__

