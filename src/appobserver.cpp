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

#include "appobserver.h"

AppObserver::AppObserver(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log)
{
    this->cfg = cfg;
    this->log = log;
    this->threadID = 3;
    this->watch = true;
    this->foundSomething = false;
    string msToWait = this->cfg->getConfigValue("/config/applications/pollTime");
    if (!boost::spirit::qi::parse(msToWait.begin(), msToWait.end(), this->msToWait))
        this->msToWait = 1000;
    string nextMailAfter = this->cfg->getConfigValue("/config/email/secondsNextMail");
    if (!boost::spirit::qi::parse(nextMailAfter.begin(), nextMailAfter.end(), this->nextMailAfter))
        this->nextMailAfter = 43200; //every 12 hours
    this->log->writeToLog(LVLDEBUG, this->threadID, "AppObserver thread is starting");
}

bool AppObserver::getData()
{
    map< string, vector<string> > appsToCheck = this->cfg->getConfigMap("/config/applications//app");
    typedef pair< string, vector<string> > ApplicationAttributes;
    BOOST_FOREACH(const ApplicationAttributes &AppAttrPair, appsToCheck)
    {
        if (AppAttrPair.second[2] == "false")
            continue;
        //this->log->writeToLog(LVLDEBUG, this->threadID, "Checking process " + AppAttrPair.first);
        string cmd = "ps -C " + AppAttrPair.first + " 2>&1";
        string psOut = execSysCmd(cmd.c_str());
        //this->log->writeToLog(LVLDEBUG, this->threadID, "psOut: " + psOut);
        if(psOut.compare("ERROR") == 0) //TODO: Write an E-Mail, should we really quit the loop?
            return false;
        vector<string> lines;
        boost::algorithm::split_regex(lines, psOut, boost::regex("\\n+"));
        bool running = false;
        BOOST_FOREACH(const string &line, lines)
        {
            boost::regex pattern("^\\s+[0-9]+\\s+([a-z]+/[0-9]|\\?)\\s+[0-9]{2}:[0-9]{2}:[0-9]{2}\\s+\\w+$");
            if (boost::regex_match(line, pattern))
            {
                running = true;
                //everything is all right, process is running
                continue;
            }
        }
        if(!running)
        {
            if (AppAttrPair.second[1] == "false")
            {
                //TODO: E-Mail Message with hint no restart
                this->log->writeToLog(LVLDEBUG, this->threadID, AppAttrPair.first + " is not running. Automatic restart is disabled.");
            } else {
                cmd = AppAttrPair.second[3] + " 2>&1";
                psOut = execSysCmd(cmd.c_str());
                if(psOut.compare("ERROR") == 0) //TODO: Write an E-Mail, should we really quit the loop?
                    continue;
                //TODO: Write E-Mail with output of cmd and that the process was not running
            }
        }
    }
    return true;
}

void AppObserver::handleStreamData(vector<string> &v)
{
}

void AppObserver::checkStreamData()
{
}
