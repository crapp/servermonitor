//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013 - 2015  Christian Rapp
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

#ifndef MAILER_H
#define MAILER_H

#include <cstdio>  //needed for popen and FILE handle
#include <sstream>
#include <string>
#include <unistd.h>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <simplelogger/simplelogger.h>

#include "smconfig.h"

class Mailer
{
public:
    Mailer(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<SimpleLogger> log);
    void sendmail(const int &threadID, bool data, std::string subject,
                  std::string message);

private:
    boost::shared_ptr<SMConfig> cfg;
    boost::shared_ptr<SimpleLogger> log;
    boost::mutex mtx;

    void collectData(std::string &msg, const int &threadID);
    std::string machineName();
};

#endif  // MAILER_H
