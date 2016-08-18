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

#ifndef MONITORWORKER_H
#define MONITORWORKER_H

#include <iostream>
#include <fcntl.h>      /* For O_RDWR */
#include <unistd.h>     /* For open(), create() */
#include <cerrno>       //error numbers
#include <sys/types.h>  //for what?
#include <sys/stat.h>   //for mkfifo
#include <string>

#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <simplelogger/simplelogger.h>

#include "smconfig.h"
#include "memoryobserver.h"
#include "cpuobserver.h"
#include "appobserver.h"
#include "globalutils.h"
#include "mailer.h"

class MonitorWorker
{
public:
    MonitorWorker(boost::shared_ptr<SMConfig> cfg,
                  boost::shared_ptr<SimpleLogger> log,
                  boost::shared_ptr<Mailer> mail);
    ~MonitorWorker();
    int startMonitoring();

private:
    boost::shared_ptr<SMConfig> cfg;
    boost::shared_ptr<SimpleLogger> log;
    boost::shared_ptr<Mailer> mail;
    boost::shared_ptr<MemoryObserver> mwatch;
    boost::shared_ptr<CPUObserver> cpuwatch;
    boost::shared_ptr<AppObserver> appwatch;
    boost::shared_ptr<boost::thread> mwatchThread;
    boost::shared_ptr<boost::thread> cpuwatchThread;
    boost::shared_ptr<boost::thread> appwatchThread;

    int threadID;
    std::string fifopath;

    int daysNextSyshealth;
    boost::gregorian::date syshealthDate;

    void ipcNamedPipe();
    void stopService();
};

#endif  // MONITORWORKER_H
