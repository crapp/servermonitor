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


#include "memoryobserver.h"

MemoryObserver::MemoryObserver(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log,
                               boost::shared_ptr<Mailer> mail)
{
    this->cfg = cfg;
    this->log = log;
    this->mail = mail;
    this->watch = true;
    this->threadID = 2;
    this->procStreamPath = this->cfg->getConfigValue("/config/sysstat/memory/processFilesystem");
    try
    {
        this->msToWait = ConvertStringToNumber<int>(this->cfg->getConfigValue("/config/applications/pollTime"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"applications/pollTime\" "
                              + toString(ex.what()));
        this->msToWait = 1000;
    }
    try
    {
        this->nextMailAfter = ConvertStringToNumber<int>(this->cfg->getConfigValue("/config/email/secondsNextMail"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"email/secondsNextMail\" "
                              + toString(ex.what()));;
        this->nextMailAfter = 43200; //every 12 hours
    }
    try
    {
        this->minMemFree = ConvertStringToNumber<int>(this->cfg->getConfigValue("/config/sysstat/memory/minimumFree"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"sysstat/memory/minimumFree\" "
                              + toString(ex.what()));;
        this->minMemFree = 10000; //100MB
    }
    try
    {
        this->maxSwap = ConvertStringToNumber<int>(this->cfg->getConfigValue("/config/sysstat/memory/maximumSwap"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"sysstat/memory/maximumSwap\" "
                              + toString(ex.what()));;
        this->noValuesToCompare = 10;
    }
    try
    {
        this->noValuesToCompare = ConvertStringToNumber<int>(this->cfg->getConfigValue("/config/sysstat/memory/noValuesCompare"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"sysstat/memory/noValuesCompare\" "
                              + toString(ex.what()));;
        this->noValuesToCompare = 10;
    }
    this->initLastDetection();
    this->log->writeToLog(LVLDEBUG, this->threadID, "MemoryObserver Object instantiated");
}

void MemoryObserver::handleStreamData(vector<string> &v)
{
    if (v.size() >= 2)
    {
        try
        {
            //erase last colon from key
            boost::algorithm::erase_last(v[0], ":");
            try
            {
                float f = ConvertStringToNumber<float>(v[1]);
                this->memInfoMap.insert(pair<string, float>(v[0], f));
            }
            catch (const invalid_argument &ex)
            {
                this->log->writeToLog(LVLERROR, this->threadID, "Can not cast "
                                      + v[1] + " to float. " + toString(ex.what()));
            }
        }
        catch(const exception &ex)
        {
            string s(ex.what());
            this->log->writeToLog(LVLERROR, this->threadID, "Exception: " + s);
        }
        catch(...)
        {
            this->log->writeToLog(LVLERROR, this->threadID, "MemoryWatch handleStreamData failed with unknown exception.");
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Vector wrong size, expected size two, got "
                              + toString(v.size()));
    }
}

void MemoryObserver::checkStreamData()
{
    this->log->writeToLog(LVLDEBUG, this->threadID, "Checking Memory stream data");
    if (!this->memInfoMap.size() > 0)
            return;
    //check if all keys are in the map
    if (this->memInfoMap.find("MemTotal") != this->memInfoMap.end() &&
            this->memInfoMap.find("MemFree") != this->memInfoMap.end() &&
            this->memInfoMap.find("SwapTotal") != this->memInfoMap.end() &&
            this->memInfoMap.find("SwapFree") != this->memInfoMap.end())
    {
        if (this->lastMemFreeValues.size() < this->noValuesToCompare)
        {
            this->log->writeToLog(LVLDEBUG, this->threadID, "Size of mem value list smaller than values to check, just push_back and continue");
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
        this->log->writeToLog(LVLERROR, this->threadID, "Missing keys in meminfomap :/");
        //TODO: Send E-Mail!
    }
    this->memInfoMap.clear();
}

void MemoryObserver::initLastDetection()
{
    //create a ptime object that is older as this->nextMailAfter + 60
    boost::posix_time::ptime pt = boost::posix_time::second_clock::universal_time()
            - boost::posix_time::seconds(this->nextMailAfter + 60);
    //add start values to map
    this->mapLastDetection.insert(pair<string, boost::posix_time::ptime>("Memory", pt));
    this->mapLastDetection.insert(pair<string, boost::posix_time::ptime>("Swap", pt));
}

bool MemoryObserver::checkMemory()
{
    /*NOTE: we could hold a sum as a class member and update it everytime the vector gets updated.
     *      No more need for foreach here. But ok the list is usually small...
     */
    float sum = 0;
    BOOST_FOREACH(const float &f, this->lastMemFreeValues)
    {
        sum += f;
    }
    if ((this->memInfoMap["MemTotal"] - (sum/this->lastMemFreeValues.size()))
            < this->minMemFree)
    {
        this->mapLastDetection["Memory"] = boost::posix_time::second_clock::universal_time();
        //TODO: Send E-Mail on low memory!
        this->log->writeToLog(LVLWARN, this->threadID, "Not much memory left "
                              + toString(this->memInfoMap["MemTotal"] - (sum/this->lastMemFreeValues.size())));
        return true;
    }
    this->log->writeToLog(LVLDEBUG, this->threadID, "Memory usage does not exceed threshold");
    return false;
}

bool MemoryObserver::checkSwap()
{
    if ((this->memInfoMap["SwapTotal"] - this->memInfoMap["SwapFree"]) > this->maxSwap)
    {
        this->mapLastDetection["Swap"] = boost::posix_time::second_clock::universal_time();
        //TODO: Send E-Mail on swap usage!
        this->log->writeToLog(LVLWARN, this->threadID, "System swap usage exceeded threshold: "
                              + toString((this->memInfoMap["SwapTotal"] - this->memInfoMap["SwapFree"]))
                              + " > " + toString(this->maxSwap));
        return true;
    }
    this->log->writeToLog(LVLDEBUG, this->threadID, "Swap usage does not exceed threshold");
    return false;
}
