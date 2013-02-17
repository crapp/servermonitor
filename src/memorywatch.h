#ifndef MEMORYWATCH_H
#define MEMORYWATCH_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include "config.h"

class MemoryWatch
{
public:
    bool watch;
    MemoryWatch(boost::shared_ptr<Config> cfg);
    void queryProc();

private:
    boost::shared_ptr<Config> cfg;
    boost::filesystem::ifstream memInfoStream;
    void memoryWatchThread();
};

#endif // MEMORYWATCH_H
