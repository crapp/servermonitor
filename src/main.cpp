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

#include <iostream>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/fstream.hpp>

#include <simplelogger/simplelogger.h>

#include "smconfig.h"
#include "mailer.h"
#include "monitorworker.h"
#include "globalutils.h"
#include "config.h"

int main(int argc, char *argv[])
{
    boost::shared_ptr<SMConfig> cfg = boost::make_shared<SMConfig>(CONFIGPATH);

    if (!cfg->getConfigFileOK()) {
        std::cerr << "ServerMontior could not open the config file." << std::endl
                  << "Should be here /etc/serverMonitor/config.xml" << std::endl
                  << "Also check if the xml is well formed." << std::endl;
        return 1;
    }

    boost::shared_ptr<SimpleLogger> log = nullptr;

    try {
        std::string logfile =
            cfg->getConfigValue("/config/logger/logDir") + "servermonitor.log";
        SimpleLogger::logLevels loglevel = static_cast<SimpleLogger::logLevels>(
            std::stoi(cfg->getConfigValue("/config/logger/minLogLevel")));
        bool logToFile =
            std::stoi(cfg->getConfigValue("/config/logger/logToFile"));
        bool logToSyslog =
            std::stoi(cfg->getConfigValue("/config/logger/logToSyslog"));
        std::string logTimeFormat =
            cfg->getConfigValue("/config/logger/logTimeFormat");
        log = boost::make_shared<SimpleLogger>(loglevel, false, logToFile,
                                               logToSyslog, true, false,
                                               logTimeFormat, logfile);
    } catch (const std::exception &ex) {
        std::cerr << "Could not initialize logger for ServerMonitor"
                  << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    boost::shared_ptr<Mailer> mail = boost::make_shared<Mailer>(cfg, log);

    std::stringstream version;
    version << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Starting ServerMonitor " + version.str());

    boost::shared_ptr<MonitorWorker> mw =
        boost::make_shared<MonitorWorker>(cfg, log, mail);
    mw->startMonitoring();

    log->writeLog(SimpleLogger::logLevels::INFO, "ServerMonitor has stopped");
    log->writeLog(SimpleLogger::logLevels::DEBUG,
                  "NoOfActiveThreads: " + std::to_string(noOfActiveThreads));
    return 0;
}
