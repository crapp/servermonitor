//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013 - 2018 Christian Rapp
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

#include "constants.h"

Mailer::Mailer(std::shared_ptr<SMConfig> cfg) : cfg(cfg) {
    this->log = spdlog::get(sm_constants::LOGGER);
}

void Mailer::sendmail(int threadID, bool data, std::string subject,
                      std::string message) {
    // make this email sender thread safe with a simple lock
    boost::lock_guard<boost::mutex> lockGuard(Mailer::mtx);

    this->log->debug("trying to send this message: \n " + message);

    if (data) {
        this->collectData(message, threadID);
    }
    try {
        subject.insert(0, "Machine: " + machineName() + " -- ");
        // Open mail command with popen
        FILE *mta = popen(
            this->cfg->getConfigValue("/config/email/mailCommand").c_str(),
            "w");
        if (mta != nullptr) {
            fprintf(mta, "To: %s\n",
                    this->cfg->getConfigValue("/config/email/mailTo").c_str());
            fprintf(
                mta, "From: %s\n",
                this->cfg->getConfigValue("/config/email/mailFrom").c_str());
            fprintf(mta, "Subject: %s\n\n", subject.c_str());
            fwrite(message.c_str(), 1, strlen(message.c_str()), mta);
            fwrite(".\n", 1, 2, mta);

            pclose(mta);
        } else {
            this->log->error(
                "Can not send an email, was not able to open mail command " +
                this->cfg->getConfigValue("/config/email/mailCommand"));
        }
    } catch (const std::exception &ex) {
        this->log->error(std::string("Can not send email, ") + ex.what());
    } catch (...) {
        this->log->error("Can not send email, general exception occured");
    }
    this->log->info("Mail has been send successfully");
}

// TODO: Pass by value and move. Return the string and profit from RVO
void Mailer::collectData(std::string &msg,
                         BOOST_ATTRIBUTE_UNUSED int threadID) {
    std::map<std::string, std::vector<std::string>> dataCollectors;
    dataCollectors =
        this->cfg->getConfigMap("/config/email/dataCollectors//collector");
    if (dataCollectors.size() > 0) {
        typedef std::pair<std::string, std::vector<std::string>> DCollectors;
        BOOST_FOREACH (const DCollectors &dc, dataCollectors) {
            msg +=
                "\n\n----------------------------------------------------------"
                "-"
                "----------------\n";
            msg += "Collecting data from command: " + dc.first + "\n\n";
            this->log->info("Collecting data from command: " + dc.first);
            msg += execSysCmd(dc.second[1].c_str());
        }
    } else {
        this->log->warn("No data collectors defined");
    }
    msg += "\n\nFinished collecting data";
}

std::string Mailer::machineName() {
    std::stringstream s;
    char hostn[1024];
    memset(hostn, 0, 1024);
    int returnValue = gethostname(hostn, 1024);
    if (returnValue > -1) {
        s << hostn;
    } else {
        s << "UNDEFINED";
    }

    return s.str();
}
