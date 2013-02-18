#ifndef MEMORYWATCH_H
#define MEMORYWATCH_H


#include "config.h"
#include "procwatch.h"

class MemoryWatch : public ProcWatch
{
public:
    MemoryWatch(boost::shared_ptr<Config> cfg, boost::shared_ptr<Logger> log);
    void queryMemProc();
private:
    map<string, float> memInfoMap;

    void handleStreamData(vector<string> v);
    void checkStreamData();
};

#endif // MEMORYWATCH_H
