#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
#include "smconfig.h"

#define LVLDEBUG 0
#define LVLERROR 1

using namespace std;

class Logger
{
public:
    Logger(boost::shared_ptr<SMConfig> cfg);
    void writeToLog(const int &debugLevel, const int &threadID, const string &msg);
private:
    boost::shared_ptr<SMConfig> cfg;
    boost::shared_ptr<boost::mutex> mtx;
};

#endif // LOGGER_H
