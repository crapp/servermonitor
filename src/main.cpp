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

#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/fstream.hpp>
#include "config.h"

using namespace std;

//command line arguments are not used. I would say we use a config XML file
int main (int argc, char *argv[]) {
    boost::shared_ptr<Config> cfg(new Config());
    boost::filesystem::ifstream procMemInfo ("/proc/meminfo");
    if (procMemInfo.is_open())
    {
        string line;
        while(procMemInfo.good())
        {
            getline(procMemInfo, line);
            cout << line << endl;
        }
        procMemInfo.close();
    }
    return 0;
}

