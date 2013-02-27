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
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/fstream.hpp>
#include "smconfig.h"
#include "logger.h"
#include "mailer.h"
#include "monitorworker.h"
#include "globalutils.h"

using namespace std;

extern int noOfActiveThreads;

//command line arguments are not used. I would say we use a config XML file
int main (int argc, char *argv[]) {
    //our config object that we share with other objects
    boost::shared_ptr<SMConfig> cfg(new SMConfig());
    boost::shared_ptr<Logger> log(new Logger(cfg));
    boost::shared_ptr<Mailer> mail(new Mailer(cfg, log));
    log->writeToLog(LVLDEBUG, 0, "ServerMonitor starting " + toString(VERSION));
    boost::shared_ptr<MonitorWorker> mw(new MonitorWorker(cfg, log, mail));
    mw->startMonitoring();
    log->writeToLog(LVLDEBUG, 0, "NoOfActiveThreads: " + toString(noOfActiveThreads));
    return 0;
}

