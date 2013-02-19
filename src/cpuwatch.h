//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013  Christian Rapp
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CPUWATCH_H
#define CPUWATCH_H

#include <boost/foreach.hpp>
#include <sstream>
#include "config.h"
#include "logger.h"
#include "procwatch.h"

class CPUWatch : public ProcWatch
{
public:
    CPUWatch(boost::shared_ptr<Config> cfg, boost::shared_ptr<Logger> log);
    void queryCPUProc();

private:
    vector<string> cpuLoad;

    void handleStreamData(vector<string> &v);
    void checkStreamData();
    //ps -eo pcpu,pid,user,args | sort -r -k1
};

#endif // CPUWATCH_H
