#include "util.h"
#include "loki.h"
#include <signal.h>
#include <sys/time.h>
#include <cmath>

using namespace std;

namespace xfast{
	

string xUtil::tm2str(const struct tm &stTm, const string &sFormat)
{
    char sTimeString[255] = "\0";

    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), &stTm);

    return string(sTimeString);
}

string xUtil::tm2str(const time_t &t, const string &sFormat)
{
    struct tm tt;
    localtime_r(&t, &tt);

    return tm2str(tt, sFormat);
}

string xUtil::now2str(const string &sFormat)
{
    time_t t = time(NULL);
    return tm2str(t, sFormat.c_str());
}

string xUtil::now2strLog(){
    char sTimeString[256] = "\0";
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);

    tm = localtime(&tv.tv_sec);

    sprintf(sTimeString, "%04d-%02d-%02d %02d:%02d:%02d.%04d",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec,
            (int) (tv.tv_usec / 10000)
        );

    return string(sTimeString);
}

template<typename T>
string xUtil::tostr(const T &t)
{
    ostringstream sBuffer;
    sBuffer << t;
    return sBuffer.str();
}

template<typename T>
string xUtil::tostr(const vector<T> &t)
{
    string s;
    for(size_t i = 0; i < t.size(); i++)
    {
        s += tostr(t[i]);
        s += " ";
    }
    return s;
}


template <>
string xUtil::tostr<bool>(const bool &t)
{
	char buf[2];
	buf[0] = t ? '1' : '0';
	buf[1] = '\0';
	return string(buf);
}


template <>
string xUtil::tostr<char>(const char &t)
{
	char buf[2];
	snprintf(buf, 2, "%c", t);
	return string(buf);
}

template <>
string xUtil::tostr<unsigned char>(const unsigned char &t)
{
	char buf[2];
	snprintf(buf, 2, "%c", t);
	return string(buf);
}

template <>
string xUtil::tostr<short>(const short &t)
{
	char buf[16];
	snprintf(buf, 16, "%d", t);
	return string(buf);
}

template <>
string xUtil::tostr<unsigned short>(const unsigned short &t)
{
	char buf[16];
	snprintf(buf, 16, "%u", t);
	return string(buf);
}

template <>
string xUtil::tostr<int>(const int &t)
{
	char buf[16];
	snprintf(buf, 16, "%d", t);
	return string(buf);
}

template <>
string xUtil::tostr<unsigned int>(const unsigned int &t)
{
	char buf[16];
	snprintf(buf, 16, "%u", t);
	return string(buf);
}

template <>
string xUtil::tostr<long>(const long &t)
{
	char buf[32];
	snprintf(buf, 32, "%ld", t);
	return string(buf);
}

template <>
string xUtil::tostr<long long>(const long long &t)
{
	char buf[32];
	snprintf(buf, 32, "%lld", t);
	return string(buf);
}


template <>
string xUtil::tostr<unsigned long>(const unsigned long &t)
{
	char buf[32];
	snprintf(buf, 32, "%lu", t);
	return string(buf);
}

template <>
string xUtil::tostr<float>(const float &t)
{
	char buf[32];
	snprintf(buf, 32, "%.5f", t);
	string s(buf);

	//去掉无效0, eg. 1.0300 -> 1.03;1.00 -> 1
	bool bFlag = false;
	int pos = int(s.size() - 1);
	for(; pos > 0; --pos)
	{
		if(s[pos] == '0')
		{
			bFlag = true;
			if(s[pos-1] == '.')
			{
				//-2为了去掉"."号
				pos -= 2;
				break;
			}
		}
		else
		{
			break;
		}
	}

	if(bFlag)
		s = s.substr(0, pos+1);

	return s;
}

template <>
string xUtil::tostr<double>(const double &t)
{
	char buf[32];
	snprintf(buf, 32, "%.5f", t);
	string s(buf);

	//去掉无效0, eg. 1.0300 -> 1.03;1.00 -> 1
	bool bFlag = false;
	int pos = int(s.size() - 1);
	for(; pos > 0; --pos)
	{
		if(s[pos] == '0')
		{
			bFlag = true;
			if(s[pos-1] == '.')
			{
				//-2为了去掉"."号
				pos -= 2;
				break;
			}
		}
		else
		{
			break;
		}
	}

	if(bFlag)
		s = s.substr(0, pos+1);

	return s;

}

template <>
string xUtil::tostr<long double>(const long double &t)
{
	char buf[32];
	snprintf(buf, 32, "%Lf", t);
	string s(buf);

	//去掉无效0, eg. 1.0300 -> 1.03;1.00 -> 1
	bool bFlag = false;
	int pos = int(s.size() - 1);
	for(; pos > 0; --pos)
	{
		if(s[pos] == '0')
		{
			bFlag = true;
			if(s[pos-1] == '.')
			{
				//-2为了去掉"."号
				pos -= 2;
				break;
			}
		}
		else
		{
			break;
		}
	}

	if(bFlag)
		s = s.substr(0, pos+1);

	return s;

}

template <>
string xUtil::tostr<std::string>(const std::string &t)
{
	return t;
}



}