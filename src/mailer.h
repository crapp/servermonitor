//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013 - 2018 Christian Rapp
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

#include <unistd.h>
#include <cstdio>  //needed for popen and FILE handle
#include <memory>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

#include "spdlog/spdlog.h"

#include "smconfig.h"

class Mailer {
   public:
    Mailer(std::shared_ptr<SMConfig> cfg);
    void sendmail(int threadID, bool data, std::string subject,
                  std::string message);

   private:
    std::shared_ptr<SMConfig> cfg;
    std::shared_ptr<spdlog::logger> log;
    boost::mutex mtx;

    void collectData(std::string &msg, int threadID);
    std::string machineName();
};

#endif  // MAILER_H
