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

#ifndef OBSERVER_H
#define OBSERVER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/thread.hpp>
#include "smconfig.h"
#include "logger.h"
#include "globalutils.h"

class Observer
{
public:
    Observer();
    virtual ~Observer();
    void threadLoop();

protected:
    bool watch;
    int msToWait;
    int threadID;
    int nextMailAfter;

    boost::shared_ptr<SMConfig> cfg;
    boost::shared_ptr<Logger> log;
    ifstream procStream;
    map<string, boost::posix_time::ptime> mapLastDetection;
    string procStreamPath;

    bool checkTimeoutMail(const boost::posix_time::ptime &pt);
    virtual bool getData() = 0;
    virtual void handleStreamData(vector<string> &v) = 0;
    virtual void checkStreamData() = 0;
    virtual void initLastDetection() = 0;

};

#endif // OBSERVER_H
