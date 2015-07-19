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
    this->appsToCheck =
        this->cfg->getConfigMap("/config/observer/applications//app");
    if (this->appsToCheck.empty())
        this->watch = false;
    this->initLastDetection();
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "AppObserver Object instantiated");
}

bool AppObserver::getData()
{
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "AppObserver is reading data");
    typedef std::pair<std::string, std::vector<std::string>>
        ApplicationAttributes;
    BOOST_FOREACH (const ApplicationAttributes &AppAttrPair, this->appsToCheck) {
        if (AppAttrPair.second[2] == "false")
            continue;

        this->log->writeLog(SimpleLogger::logLevels::INFO,
                            "Checking process " + AppAttrPair.first);
        std::string cmd = "ps -C " + AppAttrPair.first + " 2>&1";
        std::string psOut = execSysCmd(cmd.c_str());
        this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                            "Command output: " + psOut);
        if (psOut.compare("ERROR") ==
            0)  // TODO: Should we really quit the loop?
        {
            this->mail->sendmail(
                this->threadID, false, "Command execution failed",
                "Could not execute command" + cmd + "\n\n" + psOut +
                    "\n\n Will stop AppObserver Thread");
            this->log->writeLog(SimpleLogger::logLevels::ERROR,
                                "Restart command failed: " + psOut);
            return false;
        }
        std::vector<std::string> lines;
        boost::algorithm::split_regex(lines, psOut, boost::regex("\\n+"));
        bool running = false;
        BOOST_FOREACH (const std::string &line, lines) {
            boost::regex pattern(
                "^\\s*[0-9]+\\s+([a-z]+/"
                "[0-9]|\\?)\\s+[0-9]{2}:[0-9]{2}:[0-9]{2}\\s+\\w+$");
            if (boost::regex_match(line, pattern)) {
                // everything is all right, process is running
                this->log->writeLog(
                    SimpleLogger::logLevels::INFO,
                    "Process " + AppAttrPair.first + " is running");
                running = true;
                break;
            }
        }
        if (!running) {
            this->log->writeLog(SimpleLogger::logLevels::WARNING,
                                AppAttrPair.first + " is not running.");
            if (!this->checkTimeoutMail(
                    this->mapLastDetection[AppAttrPair.first])) {
                this->log->writeLog(
                    SimpleLogger::logLevels::WARNING,
                    "Won't notify user via E-Mail because timeout not reached");
                continue;
            }
            if (AppAttrPair.second[1] == "false") {
                this->mail->sendmail(
                    this->threadID, false, "Application not running",
                    "Application " + AppAttrPair.first + " is not running. " +
                        "Restart is disabled!");
                this->log->writeLog(
                    SimpleLogger::logLevels::WARNING,
                    AppAttrPair.first + " Automatic restart is disabled.");
            } else {
                this->log->writeLog(
                    SimpleLogger::logLevels::WARNING,
                    "Trying to restart process " + AppAttrPair.first);
                cmd = AppAttrPair.second[3] + " 2>&1";
                psOut = execSysCmd(cmd.c_str());
                if (psOut.compare("ERROR") ==
                    0)  // TODO: Should we really quit the loop?
                {
                    this->mail->sendmail(
                        this->threadID, false, "Command execution failed",
                        "Could not execute command" + cmd + "\n\n" + psOut +
                            "\n\n Will stop AppObserver Thread");
                    this->log->writeLog(SimpleLogger::logLevels::ERROR,

                                        "Restart command failed: " + psOut);
                    return false;
                }
                this->log->writeLog(SimpleLogger::logLevels::INFO,

                                    "Application " + AppAttrPair.first +
                                        " was not running. Tried to restart the "
                                        "application:\n " +
                                        psOut);
                this->mail->sendmail(this->threadID, false,
                                     "Restarted an application",
                                     "Application " + AppAttrPair.first +
                                         " was not running. Tried to restart "
                                         "the application:\n " +
                                         psOut);
            }
            this->mapLastDetection[AppAttrPair.first] =
                boost::posix_time::second_clock::universal_time();
        }
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
