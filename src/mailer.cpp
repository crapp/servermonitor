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

#include "mailer.h"

Mailer::Mailer(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log) :
    cfg(cfg), log(log)
{
}

//define static mutex
boost::mutex Mailer::mtx;

void Mailer::sendmail(const int &threadID, bool data, string subject, string message)
{
    //make this email sender thread safe with a simple lock
    boost::lock_guard<boost::mutex> lockGuard(Mailer::mtx);

    if (data)
        this->collectData(message, threadID);
    try {
        subject.insert(0, "Machine: " + machineName() + " -- ");
        //Open mail command with popen
        FILE *mta = popen(this->cfg->getConfigValue("/config/email/mailCommand").c_str(), "w");
        if (mta != 0) {
            fprintf(mta, "To: %s\n", this->cfg->getConfigValue("/config/email/mailTo").c_str());
            fprintf(mta, "From: %s\n", this->cfg->getConfigValue("/config/email/mailFrom").c_str());
            fprintf(mta, "Subject: %s\n\n", subject.c_str());
            fwrite(message.c_str(), 1, strlen(message.c_str()), mta);
            fwrite(".\n", 1, 2, mta);

            pclose(mta);
        } else {
            this->log->writeToLog(LVLERROR, threadID, "Can not send an email, was not able to open mail command " +
                                  this->cfg->getConfigValue("/config/email/mailCommand"));
        }
    }
    catch (exception &ex)
    {
        this->log->writeToLog(LVLERROR, threadID, "Can not send email, "
                              + toString(ex.what()));
    }
    catch (...) {
        this->log->writeToLog(LVLERROR, threadID, "Can not send email, general exception occured");
    }
    this->log->writeToLog(LVLINFO, threadID, "Mail has been sended successfully");
}

void Mailer::collectData(string &msg, const int &threadID)
{
    map< string, vector<string> > dataCollectors;
    dataCollectors = this->cfg->getConfigMap("/config/email/dataCollectors//collector");
    if (dataCollectors.size() > 0)
    {
        typedef pair< string, vector<string> > DCollectors;
        BOOST_FOREACH(const DCollectors &dc, dataCollectors)
        {
            msg += "\n\n---------------------------------------------------------------------------\n";
            msg += "Collecting data from command: " + dc.first + "\n\n";
            this->log->writeToLog(LVLINFO, threadID, "Collecting data from command: "
                                  + dc.first);
            msg += execSysCmd(dc.second[1].c_str());
        }
    } else {
        this->log->writeToLog(LVLWARN, threadID, "No data collectors defined");
    }
    msg += "\n\nFinished collecting data";
}

string Mailer::machineName()
{
    string s = "";
    char *p = getenv("HOSTNAME");
    if (p != NULL)
        s = p;
    return s;
}
