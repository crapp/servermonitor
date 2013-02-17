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

struct Config {
    const string mailTo;
    const string mailFrom;
    const string memInfo;
    const int memMinFree;
    const int waitMSThread;

    Config() :
        mailTo("admin@geo.uni-tuebingen.de"),
        mailFrom("monitor@geo.uni-tuebingen.de"),
        memInfo("/proc/meminfo"),
        memMinFree(100000), //100MB
        waitMSThread(1000)
    {
    }
};

#endif // CONFIG_H
