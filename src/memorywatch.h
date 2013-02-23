#ifndef MEMORYWATCH_H
#define MEMORYWATCH_H

#include <list>
#include "procwatch.h"

class MemoryWatch : public Observer
{
public:
    MemoryWatch(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log);
    void queryMemProc();
private:
    map<string, float> memInfoMap;
    list<float> lastMemFreeValues;
    int minMemFree;
    int maxSwap;
    uint noValuesToCompare;

    void handleStreamData(vector<string> &v);
    void checkStreamData();
};

#endif // MEMORYWATCH_H
