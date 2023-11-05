#ifndef __TC_OPTION_H
#define __TC_OPTION_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

namespace xfast
{


// support ./main.exe --name=value --with abc def 

class xOption
{
public:

    xOption(){};

    void decode(int argc, char *argv[]);

    bool hasParam(const string &sName);
    string getValue(const string &sName);
    map<string, string>& getMulti();
    vector<string>& getSingle();

protected:

    void parse(const string &s);

protected:
    map<string, string> _mParam;
    vector<string>      _vSingle;
};

}// namespace

#endif

