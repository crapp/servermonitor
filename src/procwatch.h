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

#ifndef PROCWATCH_H
#define PROCWATCH_H

#include <iostream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/thread.hpp>
#include "config.h"
#include "logger.h"

class ProcWatch
{
public:
    ProcWatch();
    virtual ~ProcWatch();
    void procWatchThreadLoop();

protected:
    bool watch;
    bool foundSomething;
    int msToWait;
    int threadID;
    boost::shared_ptr<Config> cfg;
    boost::shared_ptr<Logger> log;
    boost::filesystem::ifstream procStream;
    boost::posix_time::ptime ptimeLastDetection;
    boost::filesystem::path procStreamPath;

    bool checkLastDetection();
    virtual void handleStreamData(vector<string> &v) = 0;
    virtual void checkStreamData() = 0;

};

#endif // PROCWATCH_H
