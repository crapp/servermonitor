//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013 - 2016  Christian Rapp
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
#include <boost/thread.hpp>

#include <simplelogger/simplelogger.h>

#include "smconfig.h"
#include "globalutils.h"
#include "mailer.h"

class Observer
{
public:
    // Observer();
    virtual ~Observer();
    void start();

protected:
    Observer(boost::shared_ptr<SMConfig> cfg,
             boost::shared_ptr<SimpleLogger> log,
             boost::shared_ptr<Mailer> mail);

    bool watch;
    int msToWait;
    int threadID;
    int nextMailAfter;

    boost::shared_ptr<SMConfig> cfg;
    boost::shared_ptr<SimpleLogger> log;
    boost::shared_ptr<Mailer> mail;
    std::ifstream procStream;
    std::map<std::string, boost::posix_time::ptime> mapLastDetection;
    std::string procStreamPath;

    bool checkTimeoutMail(const boost::posix_time::ptime &pt);
    virtual bool getData() = 0;
    virtual void handleStreamData(std::vector<std::string> &v) = 0;
    virtual void checkStreamData() = 0;
    virtual void initLastDetection() = 0;
};

#endif  // OBSERVER_H
