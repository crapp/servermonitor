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

#include "cpuobserver.h"

CPUObserver::CPUObserver(boost::shared_ptr<SMConfig> cfg,
                         boost::shared_ptr<SimpleLogger> log,
                         boost::shared_ptr<Mailer> mail)
    : ProcObserver(cfg, log, mail)
{
    this->log = log;
    this->threadID = 1;
    this->procStreamPath = this->cfg->getConfigValue(
        "/config/observer/sysstat/cpu/processFilesystem");
    // TODO: How do we handle empty config values?
    if (this->procStreamPath == "")
        this->procStreamPath = "/proc/loadavg";
    try {
        this->msToWait = std::stoi(
            this->cfg->getConfigValue("/config/observer/sysstat/cpu/pollTime"));
    } catch (const std::exception &ex) {
        this->log->writeLog(SimpleLogger::logLevels::ERROR,
                            "Can not parse \"observer/sysstat/cpu/pollTime\" " +
                                std::string(ex.what()));
        this->msToWait = 1000;
    }
    try {
        this->nextMailAfter = std::stoi(this->cfg->getConfigValue(
            "/config/observer/sysstat/cpu/secondsNextMail"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"email/secondsNextMail\" " + std::string(ex.what()));
        ;
        this->nextMailAfter = 3600;  // every 12 hours
    }
    try {
        this->cpuAvgLoad5 = std::stof(this->cfg->getConfigValue(
            "/config/observer/sysstat/cpu/avg5threshold"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"observer/sysstat/cpu/avg5threshold\" " +
                std::string(ex.what()));
        ;
        this->cpuAvgLoad5 = 0.7;
    }
    try {
        this->cpuAvgLoad15 = std::stof(this->cfg->getConfigValue(
            "/config/observer/sysstat/cpu/avg15threshold"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"observer/sysstat/cpu/avg15threshold\" " +
                std::string(ex.what()));
        ;
        this->cpuAvgLoad15 = 0.8;
    }
    this->initLastDetection();
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "CPUObserver Object instantiated");
}

CPUObserver::~CPUObserver() {}

void CPUObserver::handleStreamData(std::vector<std::string> &v)
{
    this->cpuLoad = v;
}

void CPUObserver::checkStreamData()
{
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "Checking stream data...");
    float avg5, avg15;
    if (this->cpuLoad.size() == 5) {
        try {
            avg5 = std::stof(this->cpuLoad[1]);
            avg15 = std::stof(this->cpuLoad[2]);
        } catch (const std::exception &ex) {
            this->log->writeLog(SimpleLogger::logLevels::ERROR,
                                "Can not cast: " + this->cpuLoad[1] + ", " +
                                    this->cpuLoad[2] +
                                    ". Checking of avg CPU load failed \n " +
                                    std::string(ex.what()));
            this->mail->sendmail(this->threadID, false, "Can not cast values",
                                 "Can not cast: " + this->cpuLoad[1] + ", " +
                                     this->cpuLoad[2] +
                                     ". Checking of avg CPU load failed \n " +
                                     std::string(ex.what()));
            return;
        }
        if (avg5 > this->cpuAvgLoad5) {
            std::string msg =
                "Average CPU load measured over the last 5 Minutes exceeded "
                "threshold(" +
                std::to_string(this->cpuAvgLoad5) + ") ";
            this->log->writeLog(SimpleLogger::logLevels::WARNING, msg);
            if (this->checkTimeoutMail(this->mapLastDetection["avg5"])) {
                this->mapLastDetection["avg5"] =
                    boost::posix_time::second_clock::universal_time();
                composeMailMessage(msg);
            }
        }
        if (avg15 > this->cpuAvgLoad15) {
            std::string msg =
                "Average CPU load measured over the last 15 Minutes exceeded "
                "threshold(" +
                std::to_string(this->cpuAvgLoad15) + ") ";
            this->log->writeLog(SimpleLogger::logLevels::WARNING, msg);
            if (this->checkTimeoutMail(this->mapLastDetection["avg15"])) {
                this->mapLastDetection["avg15"] =
                    boost::posix_time::second_clock::universal_time();
                composeMailMessage(msg);
            }
        }
    } else {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Average CPU load returned wrong number of informations: " +
                std::to_string(this->cpuLoad.size()) + ", expected 5");
        this->watch = false;
    }

    cpuLoad.clear();
}

void CPUObserver::initLastDetection()
{
    // create a ptime object that is older as this->nextMailAfter + 60
    boost::posix_time::ptime pt =
        boost::posix_time::second_clock::universal_time() -
        boost::posix_time::seconds(this->nextMailAfter + 60);
    // add start values to map
    this->mapLastDetection.insert(
        std::pair<std::string, boost::posix_time::ptime>("avg5", pt));
    this->mapLastDetection.insert(
        std::pair<std::string, boost::posix_time::ptime>("avg15", pt));
}

void CPUObserver::composeMailMessage(std::string &msg)
{
    BOOST_FOREACH (const std::string &s, this->cpuLoad) {
        msg += s + " ";
    }
    this->log->writeLog(SimpleLogger::logLevels::DEBUG, msg);
    this->mail->sendmail(this->threadID, true,
                         "Average CPU load exceeded threshold", msg);
}
