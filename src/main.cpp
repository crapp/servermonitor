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

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <boost/filesystem/fstream.hpp>

#include "spdlog/spdlog.h"

#include "config.h"
#include "constants.h"
#include "globalutils.h"
#include "mailer.h"
#include "monitorworker.h"
#include "smconfig.h"

namespace smc = sm_constants;

int main(BOOST_ATTRIBUTE_UNUSED int argc, BOOST_ATTRIBUTE_UNUSED char *argv[]) {
    std::shared_ptr<SMConfig> cfg = std::make_shared<SMConfig>(CONFIGPATH);

    if (!cfg->getConfigFileOK()) {
        std::cout << "ServerMontior could not open the config file."
                  << std::endl
                  << "Should be here /etc/serverMonitor/config.xml" << std::endl
                  << "Also check if the xml is well formed." << std::endl;
        return 1;
    }

    std::shared_ptr<spdlog::logger> log = nullptr;

    try {
        std::string logfile =
            cfg->getConfigValue("/config/logger/logDir") + "servermonitor.log";

        spdlog::set_async_mode(4096);
        log = spdlog::rotating_logger_mt(smc::LOGGER, logfile, 1048576 * 5, 3);

        // std::string logTimeFormat =
        // cfg->getConfigValue("/config/logger/logTimeFormat");

    } catch (const std::exception &ex) {
        std::cerr << "Could not initialize logger for ServerMonitor"
                  << std::endl;
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    std::shared_ptr<Mailer> mail = std::make_shared<Mailer>(cfg);

    std::stringstream version;
    version << VERSION_MAJOR << "." << VERSION_MINOR;
    if (VERSION_PATCH != 0) {
        version << "." << VERSION_PATCH;
    }
    log->info("Starting ServerMonitor " + version.str());

    std::shared_ptr<MonitorWorker> mw =
        std::make_shared<MonitorWorker>(cfg, mail);
    mw->startMonitoring();

    log->info("ServerMonitor has stopped");
    log->debug("NoOfActiveThreads: " + std::to_string(noOfActiveThreads));
    return 0;
}
