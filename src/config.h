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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

//TODO: Make a config class and use pugixml to parse a xml config file
struct Config {
    const string mailTo;
    const string mailFrom;
    const string memInfo;
    const long nextMailAfter; //use long as boost time_duration returns long
    const int memMinFree;
    const int memMaxSwap;
    const int waitMemThread;
    const string procCpuAvgLoad;
    const float cpuAvgLoad5;
    const float cpuAvgLoad15;
    const int waitCpuThread;
    const string fifoPath;

    Config() :
        mailTo("admin@geo.uni-tuebingen.de"),
        mailFrom("monitor@geo.uni-tuebingen.de"),
        memInfo("/proc/meminfo"),
        nextMailAfter(43200), //in seconds, 43200 = every 12 hours
        memMinFree(100000), //100MB
        memMaxSwap(0),
        waitMemThread(1000),
        procCpuAvgLoad("/proc/loadavg"),
        cpuAvgLoad5(1.5),
        cpuAvgLoad15(1.7),
        waitCpuThread(30000),
        fifoPath("/tmp/servermonitor.fifo")
    {
    }
};

#endif // CONFIG_H
