#include "logger.h"

Logger::Logger(boost::shared_ptr<SMConfig> cfg) : cfg(cfg)
{
    this->mtx = boost::make_shared<boost::mutex>();
}

void Logger::writeToLog(const int &debugLevel, const int &threadID, const string &msg)
{
    //boost::lock_guard<boost::mutex> lockGuard(this->mtx);
    this->mtx->lock();
    if (debugLevel == LVLDEBUG)
    {
        cout << "Thread: " << threadID << " -- " << msg << endl;
    }
    if (debugLevel == LVLERROR)
    {
        cerr << "Thread: " << threadID << " -- " << msg << endl;
    }
    this->mtx->unlock();
}
