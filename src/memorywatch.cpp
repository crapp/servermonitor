#include "memorywatch.h"

MemoryWatch::MemoryWatch(boost::shared_ptr<Config> cfg) : cfg(cfg)
{
    this->watch = true;
}

void MemoryWatch::queryMemProc()
{
    //howto start a thread in a class
    //boost::thread t1(boost::bind(&MemoryWatch::memoryWatchThread, this));
    //t1.join();
    try
    {
        while(this->watch)
        {
            this->memInfoStream.open(boost::filesystem::path(this->cfg->memInfo));
            if (this->memInfoStream.is_open())
            {
                string line;
                map<string, float> memInfoMap;
                while(this->memInfoStream.good())
                {
                    getline(this->memInfoStream, line);
                    if (boost::algorithm::trim_copy(line) == "")
                        continue;
                    vector<string> v;
                    try
                    {
                        boost::algorithm::split_regex(v, line, boost::regex(" +"));
                        boost::algorithm::erase_last(v[0], ":");
                        memInfoMap.insert(pair<string, float>(v[0], boost::lexical_cast<float>(v[1])));
                    }
                    //TODO: we need to catch specific boost exceptions
                    catch(boost::bad_lexical_cast &ex)
                    {
                        cerr << ex.what() << endl;
                    }
                    catch(...)
                    {
                        cerr << "MemoryWatch queryProc failed." << endl;
                    }

//                    boost::regex expr("MemFree: .*");
//                    if (boost::regex_match(line, expr))
//                    {
//                        cout << line << endl;
//                    }
                }
                this->memInfoStream.close();
                if (memInfoMap.find("MemTotal") != memInfoMap.end() &&
                        memInfoMap.find("MemFree") != memInfoMap.end() &&
                        memInfoMap.find("SwapTotal") != memInfoMap.end() &&
                        memInfoMap.find("SwapFree") != memInfoMap.end())
                {
                    if (memInfoMap["MemTotal"] - memInfoMap["MemFree"] < this->cfg->memMinFree)
                    {
                        //TODO: Send E-Mail on low memory!
                        cout << "Not much memory left" << endl;
                    }
                    if (memInfoMap["SwapTotal"] - memInfoMap["SwapFree"] > 0)
                    {
                        //TODO: Send E-Mail on swap usage!
                        cout << "System is swapping :/" << endl;
                    }
                } else {
                    cerr << "Fuck the duck not in memInfoMap :/" << endl;
                }
                memInfoMap.clear();
                boost::this_thread::sleep(boost::posix_time::milliseconds(this->cfg->waitMemThread));
            } else {
                cerr << "Can not open: " << this->cfg->memInfo << endl;
            }
        }
    }
    catch (boost::thread_interrupted&)
    {
        cout << "Memory thread interrupted";
    }
}

void MemoryWatch::memoryWatchThread()
{

}
