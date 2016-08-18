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

#include "appobserver.h"

AppObserver::AppObserver(boost::shared_ptr<SMConfig> cfg,
                         boost::shared_ptr<SimpleLogger> log,
                         boost::shared_ptr<Mailer> mail)
    : Observer(cfg, log, mail)
{
    this->threadID = 3;
    this->watch = true;
    try {
        this->msToWait = std::stoi(
            this->cfg->getConfigValue("/config/observer/applications/pollTime"));
    } catch (const std::exception &ex) {
        this->log->writeLog(SimpleLogger::logLevels::ERROR,
                            "Can not parse \"observer/applications/pollTime\" " +
                                std::string(ex.what()));
        this->msToWait = 1000;
    }
    try {
        this->nextMailAfter = std::stoi(this->cfg->getConfigValue(
            "/config/observer/applications/secondsNextMail"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"email/secondsNextMail\" " + std::string(ex.what()));
        ;
        this->nextMailAfter = 43200;  // every 12 hours
    }

    this->initLastDetection();

    this->checkAppsRunning();

    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "AppObserver Object instantiated");
}

bool AppObserver::getData()
{
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "AppObserver is checking processes");

    auto nonRunningApp =
        std::find_if_not(this->appVector.begin(), this->appVector.end(),
                         [](const SysVApp &app) { return app.isAlive(); });

    if (nonRunningApp != this->appVector.end()) {
        this->log->writeLog(
            SimpleLogger::logLevels::DEBUG,
            "Found non running apps. Will check all Processes now");
        this->checkAppsRunning();
    } else {
        this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                            "All Applications are running");
    }

    return true;
}

void AppObserver::handleStreamData(std::vector<std::string> &v) {}

void AppObserver::checkStreamData() {}

void AppObserver::initLastDetection()
{
    typedef std::pair<std::string, std::vector<std::string>>
        ApplicationAttributes;
    boost::posix_time::ptime pt =
        boost::posix_time::second_clock::universal_time() -
        boost::posix_time::seconds(this->nextMailAfter);
    BOOST_FOREACH (const ApplicationAttributes &AppAttrPair, this->appsToCheck) {
        this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                            "Initializing last detection map for application: " +
                                AppAttrPair.first);
        this->mapLastDetection.insert(
            std::pair<std::string, boost::posix_time::ptime>(AppAttrPair.first,
                                                             pt));
    }
}

void AppObserver::checkAppsRunning()
{
    this->appsToCheck =
        this->cfg->getConfigMap("/config/observer/applications//app");
    if (this->appsToCheck.empty())
        this->watch = false;

    this->processNamePid.clear();
    this->fillProcessPidMap();

    bool appsNotRunningSendMail = false;
    std::string nonRunning = "";
    // cycle through all apps that we should check
    for (const std::pair<std::string, std::vector<std::string>> &mapEntry :
         this->appsToCheck) {
        try {
            // do we need to check this one?
            if (std::stoi(mapEntry.second.at(1))) {
                // search map of running processes
                std::map<std::string, pid_t>::iterator iter;
                iter = this->processNamePid.find(mapEntry.first);
                if (iter != this->processNamePid.end()) {
                    // app is running, generate object and push_back
                    this->appVector.push_back(
                        SysVApp(this->log, iter->first, iter->second));
                    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                                        iter->first + " is running");
                } else {
                    this->log->writeLog(
                        SimpleLogger::logLevels::WARNING,
                        "Application " + mapEntry.first + " is not running");
                    // Process not running, check if we need to send an E-Mail
                    if (this->checkTimeoutMail(
                            this->mapLastDetection.at(mapEntry.first))) {
                        this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                                            "Will notify user");
                        appsNotRunningSendMail = true;
                    }
                    nonRunning += " " + iter->first;
                    this->mapLastDetection[mapEntry.first] =
                        boost::posix_time::second_clock::universal_time();
                }
            } else {
                this->log->writeLog(
                    SimpleLogger::logLevels::DEBUG,
                    "Checking of " + mapEntry.first + " is deactivated");
            }
        } catch (const std::exception &ex) {
            this->log->writeLog(SimpleLogger::logLevels::ERROR,
                                "Can not check if application " +
                                    mapEntry.first + " is running.\n" +
                                    ex.what());
        }
    }
}

void AppObserver::fillProcessPidMap()
{
    namespace fs = boost::filesystem;
    boost::regex onlyNumericDir("^[0-9]+$");
    char readlinkBuf[1024];
    try {
        // go through all entries in /proc
        for (fs::directory_iterator dir_itr("/proc");
             dir_itr != fs::directory_iterator(); ++dir_itr) {
            // check 1. if it is a directory and 2. if the directory name
            // consists only off numeric chars.
            if (fs::is_directory(dir_itr->status()) &&
                boost::regex_match(dir_itr->path().stem().string(),
                                   onlyNumericDir)) {
                memset(readlinkBuf, 0, sizeof(readlinkBuf));
                std::string p = dir_itr->path().string() + "/exe";
                if (readlink(p.c_str(), readlinkBuf, sizeof(readlinkBuf) - 1) !=
                    -1) {
                    this->processNamePid.insert(
                        {fs::path(readlinkBuf).stem().string(),
                         std::stoi(dir_itr->path().stem().string())});
                } else {
                    this->log->writeLog(SimpleLogger::logLevels::ERROR,
                                        "Can not read exe link in /proc/" + p +
                                            "\n" + strerror(errno));
                    // TODO: Should we exit the loop here and clear the map?
                }
            }
        }
        this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                            "Detected " +
                                std::to_string(this->processNamePid.size()) +
                                " running processes");
    } catch (const std::exception &ex) {
        this->log->writeLog(SimpleLogger::logLevels::ERROR,
                            "Could not collect running processes from /proc \n" +
                                std::string(ex.what()));
        // make sure the map is empty
        this->processNamePid.clear();
    }
}
