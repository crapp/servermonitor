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

#ifndef MONITORWORKER_H
#define MONITORWORKER_H

#include <fcntl.h>     /* For O_RDWR */
#include <sys/stat.h>  //for mkfifo
#include <unistd.h>    /* For open(), create() */
#include <cerrno>      //error numbers
#include <iostream>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include "spdlog/spdlog.h"

#include "appobserver.h"
#include "cpuobserver.h"
#include "globalutils.h"
#include "mailer.h"
#include "memoryobserver.h"
#include "smconfig.h"

class MonitorWorker {
   public:
    MonitorWorker(std::shared_ptr<SMConfig> cfg,
                  std::shared_ptr<Mailer> mail);
    ~MonitorWorker();
    int startMonitoring();

   private:
    std::shared_ptr<SMConfig> cfg;
    std::shared_ptr<spdlog::logger> log;
    std::shared_ptr<Mailer> mail;
    std::shared_ptr<MemoryObserver> mwatch;
    std::shared_ptr<CPUObserver> cpuwatch;
    std::shared_ptr<AppObserver> appwatch;
    std::shared_ptr<boost::thread> mwatchThread;
    std::shared_ptr<boost::thread> cpuwatchThread;
    std::shared_ptr<boost::thread> appwatchThread;

    int threadID;
    std::string fifopath;

    int daysNextSyshealth;
    boost::gregorian::date syshealthDate;

    void ipcNamedPipe();
    void stopService();
};

#endif  // MONITORWORKER_H
