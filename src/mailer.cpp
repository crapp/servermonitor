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

Mailer::Mailer(boost::shared_ptr<Config> cfg, boost::shared_ptr<Logger> log) :
    cfg(cfg), log(log)
{
    this->mtx = boost::make_shared<boost::mutex>();
}

bool Mailer::sendmail(const string &subject, const string &message) {

    //make this email sender thread safe with a simple lock
    this->mtx->lock();
    bool success = false;

    try {
        FILE *mta = popen("/usr/lib/sendmail -t", "w");
        if (mta != 0) {
            fprintf(mta, "To: %s\n", this->cfg->mailTo.c_str());
            fprintf(mta, "From: %s\n", this->cfg->mailFrom.c_str());
            fprintf(mta, "Subject: %s\n\n", subject.c_str());
            fwrite(message.c_str(), 1, strlen(message.c_str()), mta);
            fwrite(".\n", 1, 2, mta);

            pclose(mta);

            success = true;
        }
    } catch (...) {
        this->log->writeToLog(LVLERROR, -1, "Can not send email");
    }
    this->mtx->unlock();
    return success;
}
