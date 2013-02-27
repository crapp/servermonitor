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


#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "smconfig.h"

#define LVLDEBUG 0
#define LVLERROR 1

using namespace std;

class Logger
{
public:
    Logger(boost::shared_ptr<SMConfig> cfg);
    void writeToLog(const int &debugLevel, const int &threadID, const string &msg);
private:
    boost::shared_ptr<SMConfig> cfg;
    static boost::mutex mtx;
};

#endif // LOGGER_H
