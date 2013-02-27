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

bool Mailer::sendmail(const string &subject, string &message) {

    //make this email sender thread safe with a simple lock
    boost::lock_guard<boost::mutex> lockGuard(Mailer::mtx);
    bool success = false;

    collectData(message);

    try {
        FILE *mta = popen(this->cfg->getConfigValue("/config/email/mailCommand").c_str(), "w");
        if (mta != 0) {
            fprintf(mta, "To: %s\n", this->cfg->getConfigValue("/config/email/mailTo").c_str());
            fprintf(mta, "From: %s\n", this->cfg->getConfigValue("/config/email/mailFrom").c_str());
            fprintf(mta, "Subject: %s\n\n", subject.c_str());
            fwrite(message.c_str(), 1, strlen(message.c_str()), mta);
            fwrite(".\n", 1, 2, mta);

            pclose(mta);

            success = true;
        }
    } catch (...) {
        this->log->writeToLog(LVLERROR, -1, "Can not send email");
    }
    return success;
}

void Mailer::collectData(string &msg)
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
            //string cmd = "export LC_MESSAGES=C && " + dc.second[1];
            msg += execSysCmd(dc.second[1].c_str());
        }
    }
    msg += "\n\nFinished collecting data";
}
