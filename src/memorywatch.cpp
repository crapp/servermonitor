#include "memorywatch.h"

MemoryWatch::MemoryWatch(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log)
{
    this->cfg = cfg;
    this->log = log;
    this->watch = true;
    this->foundSomething = false;
    this->procStreamPath = this->cfg->getConfigValue("/config/sysstat/memory/processFilesystem");
    string msToWait = this->cfg->getConfigValue("/config/sysstat/memory/pollTime");
    if (!boost::spirit::qi::parse(msToWait.begin(), msToWait.end(), this->msToWait))
        this->msToWait = 60000;
    string nextMailAfter = this->cfg->getConfigValue("/config/email/secondsNextMail");
    if (!boost::spirit::qi::parse(nextMailAfter.begin(), nextMailAfter.end(), this->nextMailAfter))
        this->nextMailAfter = 43200; //every 12 hours
    string minMemFree = this->cfg->getConfigValue("/config/sysstat/memory/minimumFree");
    if (!boost::spirit::qi::parse(minMemFree.begin(), minMemFree.end(), this->minMemFree))
        this->minMemFree = 10000;
    string maxSwap = this->cfg->getConfigValue("/config/sysstat/memory/maximumSwap");
    if (!boost::spirit::qi::parse(maxSwap.begin(), maxSwap.end(), maxSwap))
        this->maxSwap = 0;
    string noValuesToCompare = this->cfg->getConfigValue("/config/sysstat/memory/noValuesCompare");
    if (!boost::spirit::qi::parse(noValuesToCompare.begin(), noValuesToCompare.end(), this->noValuesToCompare))
        this->noValuesToCompare = 10;
    this->threadID = 2;
}

void MemoryWatch::queryMemProc()
{
}

void MemoryWatch::handleStreamData(vector<string> &v)
{
    if (v.size() >= 2)
    {
        try
        {
            //erase last colon from key
            boost::algorithm::erase_last(v[0], ":");
            float f;
            bool cast = boost::spirit::qi::parse(v[1].begin(), v[1].end(), f);
            if (cast)
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
        //TODO: List size is hardcoded. Should be set in config
        if (this->lastMemFreeValues.size() < this->noValuesToCompare)
        {
            this->lastMemFreeValues.push_back(this->memInfoMap["MemFree"]);
        } else {
            this->lastMemFreeValues.erase(this->lastMemFreeValues.begin());
            this->lastMemFreeValues.push_back(this->memInfoMap["MemFree"]);
            /*NOTE: we could hold a sum as a class member and update it everytime the vector gets updated.
             *      No more need for foreach here. But ok the list is small...
             */
            float sum;
            BOOST_FOREACH(const float &f, this->lastMemFreeValues)
            {
                sum += f;
            }
            if (this->memInfoMap["MemTotal"] - (sum/this->lastMemFreeValues.size()) < this->minMemFree)
            {
                //TODO: Send E-Mail on low memory!
                this->log->writeToLog(LVLDEBUG, this->threadID, "Not much memory left");
                this->foundSomething = true;
                this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
            }
        }

        if (this->memInfoMap["SwapTotal"] - this->memInfoMap["SwapFree"] > this->maxSwap)
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
