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
    if (this->procStreamPath == "")
        this->procStreamPath = "/proc/loadavg";
    string msToWait = this->cfg->getConfigValue("/config/sysstat/cpu/pollTime");
    if (!boost::spirit::qi::parse(msToWait.begin(), msToWait.end(), this->msToWait))
        this->msToWait = 1000;
    string nextMailAfter = this->cfg->getConfigValue("/config/email/secondsNextMail");
    if (!boost::spirit::qi::parse(nextMailAfter.begin(), nextMailAfter.end(), this->nextMailAfter))
        this->nextMailAfter = 43200; //every 12 hours
    string cpuAvgLoad5 = this->cfg->getConfigValue("/config/sysstat/cpu/avg5threshold");
    string cpuAvgLoad15 = this->cfg->getConfigValue("/config/sysstat/cpu/avg15threshold");
    if (!boost::spirit::qi::parse(cpuAvgLoad5.begin(), cpuAvgLoad5.end(), this->cpuAvgLoad5))
        this->cpuAvgLoad5 = 0.7;
    if (!boost::spirit::qi::parse(cpuAvgLoad15.begin(), cpuAvgLoad15.end(), this->cpuAvgLoad15))
        this->cpuAvgLoad15 = 0.8;
    this->initLastDetection();
    this->log->writeToLog(LVLDEBUG, this->threadID, "CPUObserver Object instantiated");
}

void CPUObserver::handleStreamData(vector<string> &v)
{
    this->cpuLoad = v;
}

void CPUObserver::checkStreamData()
{
    float avg5, avg15;
    if (cpuLoad.size() == 5)
    {
        bool c1 = boost::spirit::qi::parse(cpuLoad[1].begin(), cpuLoad[1].end(), avg5);
        bool c2 = boost::spirit::qi::parse(cpuLoad[2].begin(), cpuLoad[2].end(), avg15);
        if (c1 == false || c2 == false)
        {
            this->log->writeToLog(LVLERROR, this->threadID, "Can not cast: " +
                                  cpuLoad[1] + ", " + cpuLoad[2]);
            return;
        }
        if (this->checkTimeoutMail(this->mapLastDetection["avg5"]) && avg5 > this->cpuAvgLoad5)
        {
            this->mapLastDetection["avg5"] = boost::posix_time::second_clock::universal_time();
            string msg = "Average CPU load measured over the last 5 Minutes exceeded threshold("
                    + toString(this->cpuAvgLoad5) + ") ";
            composeMailMessage(msg);
        }
        if (this->checkTimeoutMail(this->mapLastDetection["avg15"]) && avg15 > this->cpuAvgLoad15)
        {
            this->mapLastDetection["avg15"] = boost::posix_time::second_clock::universal_time();
            string msg = "Average CPU load measured over the last 15 Minutes exceeded threshold("
                    + toString(this->cpuAvgLoad15) + ") ";
            composeMailMessage(msg);
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Average CPU load returned wrong number of informations: " +
                              toString(cpuLoad.size()) + ", expected 5");
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
