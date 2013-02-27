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


#include "logger.h"

Logger::Logger(boost::shared_ptr<SMConfig> cfg) : cfg(cfg)
{
}

//define static mutex
boost::mutex Logger::mtx;

void Logger::writeToLog(const int &debugLevel, const int &threadID, const string &msg)
{
    boost::lock_guard<boost::mutex> lockGuard(Logger::mtx);
    boost::posix_time::ptime dt = boost::posix_time::microsec_clock::local_time();
    if (debugLevel == LVLDEBUG)
    {
        cout << boost::posix_time::to_simple_string(dt) << ": " << "Thread: " << threadID << " sysID: " << boost::this_thread::get_id() << " -- " << msg << endl;
    }
    if (debugLevel == LVLERROR)
    {
        cerr << boost::posix_time::to_simple_string(dt) << ": " << "Thread: " << threadID << " sysID: " << boost::this_thread::get_id() << " -- " << msg << endl;
    }
}
