#include "memorywatch.h"

MemoryWatch::MemoryWatch(boost::shared_ptr<Config> cfg, boost::shared_ptr<Logger> log)
{
    this->cfg = cfg;
    this->log = log;
    this->watch = true;
    this->foundSomething = false;
    this->procStreamPath = this->cfg->memInfo;
    this->msToWait = this->cfg->waitMemThread;
    this->threadID = 2;
}

void MemoryWatch::queryMemProc()
{
}

void MemoryWatch::handleStreamData(vector<string> &v)
{
    if (v.size() > 2)
    {
        try
        {
            //erase last colon from key
            boost::algorithm::erase_last(v[0], ":");
            stringstream ss;
            float f;
            ss << v[1];
            ss >> f;
            this->memInfoMap.insert(pair<string, float>(v[0], f));
        }
        catch(exception &ex)
        {
            string s(ex.what());
            this->log->writeToLog(LVLERROR, this->threadID, "Exception: " + s);
        }
        catch(...)
        {
            this->log->writeToLog(LVLERROR, this->threadID, "MemoryWatch queryProc failed.");
            cerr << "MemoryWatch queryProc failed." << endl;
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Fuck the duck vector wrong size :/");
    }
}

void MemoryWatch::checkStreamData()
{
    if (this->checkLastDetection() == false)
    {
        this->memInfoMap.clear();
        return;
    }
    if (this->memInfoMap.find("MemTotal") != this->memInfoMap.end() &&
            this->memInfoMap.find("MemFree") != this->memInfoMap.end() &&
            this->memInfoMap.find("SwapTotal") != this->memInfoMap.end() &&
            this->memInfoMap.find("SwapFree") != this->memInfoMap.end())
    {
        if (this->memInfoMap["MemTotal"] - this->memInfoMap["MemFree"] < this->cfg->memMinFree)
        {
            //TODO: Send E-Mail on low memory!
            this->log->writeToLog(LVLDEBUG, this->threadID, "Not much memory left");
            this->foundSomething = true;
            this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
        }
        if (this->memInfoMap["SwapTotal"] - this->memInfoMap["SwapFree"] > 0)
        {
            //TODO: Send E-Mail on swap usage!
            this->log->writeToLog(LVLDEBUG, this->threadID, "System is swapping :/");
            this->foundSomething = true;
            this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Fuck the duck not in memInfoMap :/");
    }
    this->memInfoMap.clear();
}
