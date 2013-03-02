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

#include "cpuobserver.h"

CPUObserver::CPUObserver(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log,
                         boost::shared_ptr<Mailer> mail)
{
    this->watch = true;
    this->cfg = cfg;
    this->mail = mail;
    this->log = log;
    this->threadID = 1;
    this->procStreamPath = this->cfg->getConfigValue("/config/sysstat/cpu/processFilesystem");
    //TODO: How do we handle empty config values?
    if (this->procStreamPath == "")
        this->procStreamPath = "/proc/loadavg";
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
        this->cpuAvgLoad5 = ConvertStringToNumber<float>(this->cfg->getConfigValue("/config/sysstat/cpu/avg5threshold"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"sysstat/cpu/avg5threshold\" "
                              + toString(ex.what()));;
        this->cpuAvgLoad5 = 0.7;
    }
    try
    {
        this->cpuAvgLoad15 = ConvertStringToNumber<float>(this->cfg->getConfigValue("/config/sysstat/cpu/avg15threshold"));
    }
    catch (const invalid_argument &ex)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not parse \"sysstat/cpu/avg15threshold\" "
                              + toString(ex.what()));;
        this->cpuAvgLoad15 = 0.8;
    }
    this->initLastDetection();
    this->log->writeToLog(LVLDEBUG, this->threadID, "CPUObserver Object instantiated");
}

void CPUObserver::handleStreamData(vector<string> &v)
{
    this->cpuLoad = v;
}

void CPUObserver::checkStreamData()
{
    this->log->writeToLog(LVLDEBUG, this->threadID, "Checking stream data...");
    float avg5, avg15;
    if (this->cpuLoad.size() == 5)
    {
        try
        {
            avg5 = ConvertStringToNumber<float>(this->cpuLoad[1]);
            avg15 = ConvertStringToNumber<float>(this->cpuLoad[2]);
        }
        catch (const invalid_argument &ex)
        {
            this->log->writeToLog(LVLERROR, this->threadID, "Can not cast: " +
                                  this->cpuLoad[1] + ", " + this->cpuLoad[2] + ". Checking of avg CPU load failed \n "
                                  + toString(ex.what()));
            return;
        }
        if (this->checkTimeoutMail(this->mapLastDetection["avg5"]) && avg5 > this->cpuAvgLoad5)
        {
            this->mapLastDetection["avg5"] = boost::posix_time::second_clock::universal_time();
            string msg = "Average CPU load measured over the last 5 Minutes exceeded threshold("
                    + toString(this->cpuAvgLoad5) + ") ";
            this->log->writeToLog(LVLWARN, this->threadID, msg);
            composeMailMessage(msg);
        }
        if (this->checkTimeoutMail(this->mapLastDetection["avg15"]) && avg15 > this->cpuAvgLoad15)
        {
            this->mapLastDetection["avg15"] = boost::posix_time::second_clock::universal_time();
            string msg = "Average CPU load measured over the last 15 Minutes exceeded threshold("
                    + toString(this->cpuAvgLoad15) + ") ";
            this->log->writeToLog(LVLWARN, this->threadID, msg);
            composeMailMessage(msg);
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Average CPU load returned wrong number of informations: " +
                              toString(this->cpuLoad.size()) + ", expected 5");
        //TODO: Send email on errors but do not collect data.
    }
    cpuLoad.clear();
}

void CPUObserver::initLastDetection()
{
    //create a ptime object that is older as this->nextMailAfter + 60
    boost::posix_time::ptime pt = boost::posix_time::second_clock::universal_time()
            - boost::posix_time::seconds(this->nextMailAfter + 60);
    //add start values to map
    this->mapLastDetection.insert(pair<string, boost::posix_time::ptime>("avg5", pt));
    this->mapLastDetection.insert(pair<string, boost::posix_time::ptime>("avg15", pt));
}

void CPUObserver::composeMailMessage(string &msg)
{
    BOOST_FOREACH(const string &s, this->cpuLoad)
    {
        msg += s + " ";
    }
    this->log->writeToLog(LVLDEBUG, this->threadID, msg);
    //TODO: Collect data and send e-mail!
}
