#ifndef CPUWATCH_H
#define CPUWATCH_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "config.h"

class CPUWatch
{
public:
    CPUWatch(boost::shared_ptr<Config> cfg);
private:
    boost::shared_ptr<Config> cfg;
    //ps -eo pcpu,pid,user,args | sort -r -k1
};

#endif // CPUWATCH_H
