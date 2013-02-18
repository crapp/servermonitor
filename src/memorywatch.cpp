#include "memorywatch.h"

MemoryWatch::MemoryWatch(boost::shared_ptr<Config> cfg)
{
    this->cfg = cfg;
    this->watch = true;
    this->foundSomething = false;
    this->procStreamPath = this->cfg->memInfo;
    this->msToWait = this->cfg->waitMemThread;
}

void MemoryWatch::queryMemProc()
{
}

void MemoryWatch::handleStreamData(vector<string> v)
{
    try
    {
        //erase last colon from key
        boost::algorithm::erase_last(v[0], ":");
        this->memInfoMap.insert(pair<string, float>(v[0], boost::lexical_cast<float>(v[1])));
    }
    catch(boost::bad_lexical_cast &ex)
    {
        cerr << "Lexical cast failed: " << ex.what() << endl;
    }
    catch(exception &ex)
    {
        cerr << "Exception: " << ex.what() << endl;
    }
    catch(...)
    {
        cerr << "MemoryWatch queryProc failed." << endl;
    }
}

void MemoryWatch::checkStreamData()
{
//    if (this->checkLastDetection() == false)
//    {
//        this->memInfoMap.clear();
//        return;
//    }
    if (this->memInfoMap.find("MemTotal") != this->memInfoMap.end() &&
            this->memInfoMap.find("MemFree") != this->memInfoMap.end() &&
            this->memInfoMap.find("SwapTotal") != this->memInfoMap.end() &&
            this->memInfoMap.find("SwapFree") != this->memInfoMap.end())
    {
        if (this->memInfoMap["MemTotal"] - this->memInfoMap["MemFree"] < this->cfg->memMinFree)
        {
            //TODO: Send E-Mail on low memory!
            cout << "Not much memory left" << endl;
            this->foundSomething = true;
            this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
        }
        if (this->memInfoMap["SwapTotal"] - this->memInfoMap["SwapFree"] > 0)
        {
            //TODO: Send E-Mail on swap usage!
            cout << "System is swapping :/" << endl;
            this->foundSomething = true;
            this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
        }
    } else {
        cerr << "Fuck the duck not in memInfoMap :/" << endl;
    }
    this->memInfoMap.clear();
}
