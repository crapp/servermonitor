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

#include "memoryobserver.h"

MemoryObserver::MemoryObserver(boost::shared_ptr<SMConfig> cfg,
                               boost::shared_ptr<SimpleLogger> log,
                               boost::shared_ptr<Mailer> mail)
    : ProcObserver(cfg, log, mail)

{
    this->watch = true;
    this->threadID = 2;
    this->procStreamPath = this->cfg->getConfigValue(
        "/config/observer/sysstat/memory/processFilesystem");
    try {
        this->msToWait = std::stoi(this->cfg->getConfigValue(
            "/config/observer/sysstat/memory/pollTime"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            std::string("Can not parse \"observer/sysstat/memory/pollTime\" ") +
                ex.what());
        this->msToWait = 1000;
    }
    try {
        this->nextMailAfter = std::stoi(this->cfg->getConfigValue(
            "/config/observer/sysstat/memory/secondsNextMail"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"email/secondsNextMail\" " + std::string(ex.what()));
        ;
        this->nextMailAfter = 43200;  // every 12 hours
    }
    try {
        this->minMemFree = std::stoi(this->cfg->getConfigValue(
            "/config/observer/sysstat/memory/minimumFree"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"observer/sysstat/memory/minimumFree\" " +
                std::string(ex.what()));
        ;
        this->minMemFree = 10000;  // 100MB
    }
    try {
        this->maxSwap = std::stoi(this->cfg->getConfigValue(
            "/config/observer/sysstat/memory/maximumSwap"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"observer/sysstat/memory/maximumSwap\" " +
                std::string(ex.what()));
        ;
        this->maxSwap = 0;
    }
    try {
        this->noValuesToCompare = std::stoi(this->cfg->getConfigValue(
            "/config/observer/sysstat/memory/noValuesCompare"));
    } catch (const std::exception &ex) {
        this->log->writeLog(
            SimpleLogger::logLevels::ERROR,
            "Can not parse \"observer/sysstat/memory/noValuesCompare\" " +
                std::string(ex.what()));
        ;
        this->noValuesToCompare = 10;
    }
    this->initLastDetection();
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "MemoryObserver Object instantiated");
}

MemoryObserver::~MemoryObserver(){};

void MemoryObserver::handleStreamData(std::vector<std::string> &v)
{
    if (v.size() >= 2) {
        try {
            // erase last colon from key
            boost::algorithm::erase_last(v[0], ":");
            try {
                float f = std::stof(v[1]);
                this->memInfoMap.insert(std::pair<std::string, float>(v[0], f));
            } catch (const std::exception &ex) {
                this->mail->sendmail(this->threadID, false,
                                     "Error, could not cast value",
                                     "Could not cast " + v[1] + " to float");
                this->log->writeLog(SimpleLogger::logLevels::ERROR,

                                    "Can not cast " + v[1] + " to float. " +
                                        std::string(ex.what()));
            }
        } catch (const std::exception &ex) {
            std::string s(ex.what());
            this->log->writeLog(SimpleLogger::logLevels::ERROR,
                                "Exception: " + s);
        } catch (...) {
            this->log->writeLog(SimpleLogger::logLevels::ERROR,
                                "MemoryWatch handleStreamData failed with "
                                "unknown exception.");
        }
    } else {
        this->log->writeLog(SimpleLogger::logLevels::ERROR,
                            "Vector wrong size, expected size two, got " +
                                std::to_string(v.size()));
    }
}

void MemoryObserver::checkStreamData()
{
    this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                        "Checking Memory stream data");
    if (this->memInfoMap.size() == 0)
        return;
    // check if all keys are in the map
    if (this->memInfoMap.find("MemTotal") != this->memInfoMap.end() &&
        this->memInfoMap.find("MemFree") != this->memInfoMap.end() &&
        this->memInfoMap.find("SwapTotal") != this->memInfoMap.end() &&
        this->memInfoMap.find("SwapFree") != this->memInfoMap.end()) {
        if (this->lastMemFreeValues.size() < this->noValuesToCompare) {
            this->log->writeLog(SimpleLogger::logLevels::DEBUG,
                                "Size of mem value list smaller than values to "
                                "check, just push_back and continue");
            this->lastMemFreeValues.push_back(this->memInfoMap["MemFree"]);
        } else {
            this->lastMemFreeValues.erase(this->lastMemFreeValues.begin());
            this->lastMemFreeValues.push_back(this->memInfoMap["MemFree"]);

            if (this->checkTimeoutMail(this->mapLastDetection["Memory"]))
                this->checkMemory();
        }
        if (this->checkTimeoutMail(this->mapLastDetection["Swap"]))
            this->checkSwap();
    } else {
        this->log->writeLog(SimpleLogger::logLevels::ERROR,
                            "Missing keys in meminfomap :/");
    }
    this->memInfoMap.clear();
}

void MemoryObserver::initLastDetection()
{
    // create a ptime object that is older as this->nextMailAfter + 60
    boost::posix_time::ptime pt =
        boost::posix_time::second_clock::universal_time() -
        boost::posix_time::seconds(this->nextMailAfter + 60);
    // add start values to map
    this->mapLastDetection.insert(
        std::pair<std::string, boost::posix_time::ptime>("Memory", pt));
    this->mapLastDetection.insert(
        std::pair<std::string, boost::posix_time::ptime>("Swap", pt));
}

bool MemoryObserver::checkMemory()
{
    /*NOTE: we could hold a sum as a class member and update it everytime the vector gets updated.
     *      No more need for foreach here. But ok the list is usually small...
     */
    float sum = 0;
    BOOST_FOREACH (const float &f, this->lastMemFreeValues) {
        sum += f;
    }
    int avrgMemory =
        this->memInfoMap["MemTotal"] - (sum / this->lastMemFreeValues.size());
    this->log->writeLog(SimpleLogger::logLevels::INFO,
                        "Average memory usage: " + std::to_string(avrgMemory));
    if (avrgMemory < this->minMemFree) {
        this->mapLastDetection["Memory"] =
            boost::posix_time::second_clock::universal_time();
        this->mail->sendmail(
            this->threadID, true, "Average Memory usage exceeded threshold",
            "The average Memory usage is to high " + std::to_string(avrgMemory));
        this->log->writeLog(SimpleLogger::logLevels::WARNING,
                            "Average Memory usage exceeded threshold " +
                                std::to_string(avrgMemory));
        return true;
    }
    this->log->writeLog(SimpleLogger::logLevels::INFO,
                        "Memory usage does not exceed threshold");
    return false;
}

bool MemoryObserver::checkSwap()
{
    int swapUsage = this->memInfoMap["SwapTotal"] - this->memInfoMap["SwapFree"];
    this->log->writeLog(SimpleLogger::logLevels::INFO,
                        "Swap usage: " + std::to_string(swapUsage));
    if (swapUsage > this->maxSwap) {
        this->mapLastDetection["Swap"] =
            boost::posix_time::second_clock::universal_time();
        this->mail->sendmail(this->threadID, true,
                             "System swap usage exceeded threshold",
                             "System swap usage exceeded threshold: " +
                                 std::to_string((this->memInfoMap["SwapTotal"] -
                                                 this->memInfoMap["SwapFree"])) +
                                 " > " + std::to_string(this->maxSwap));
        this->log->writeLog(SimpleLogger::logLevels::WARNING,
                            "System swap usage exceeded threshold: " +
                                std::to_string((this->memInfoMap["SwapTotal"] -
                                                this->memInfoMap["SwapFree"])) +
                                " > " + std::to_string(this->maxSwap));
        return true;
    }
    this->log->writeLog(SimpleLogger::logLevels::INFO,
                        "Swap usage does not exceed threshold");
    return false;
}
