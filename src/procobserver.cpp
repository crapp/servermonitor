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

#include "procobserver.h"

ProcObserver::ProcObserver(std::shared_ptr<SMConfig> cfg,
                           std::shared_ptr<Mailer> mail)
    : Observer(cfg, mail)

{}

bool ProcObserver::getData() {
    this->log->debug("Open stream " + this->procStreamPath);
    this->procStream.open(this->procStreamPath.c_str(), std::ifstream::in);
    if (this->procStream.is_open()) {
        std::string line;
        while (this->procStream.good()) {
            getline(this->procStream, line);
            if (boost::algorithm::trim_copy(line).empty()) continue;
            this->log->debug("Received line: " + line);
            std::vector<std::string> v;
            try {
                // do a regex split on all whitespaces and not only single ones
                boost::algorithm::split_regex(v, line, boost::regex(" +"));
            } catch (const std::exception &ex) {
                this->log->error(ex.what());
            } catch (...) {
                this->log->error(
                    "Could not split stream data with "
                    "regex_split. General exception occured");
            }
            this->handleStreamData(v);
        }
        this->checkStreamData();
        this->procStream.close();
    } else {
        this->mail->sendmail(this->threadID, false,
                             "Can not open file in proc FS",
                             "Can not open " + this->procStreamPath + "\n" +
                                 "This thread will be stopped");
        this->log->error("Can not open: " + this->procStreamPath);
        // No need to write an email here. Observer Object is doing that for us.
        return false;
    }
    return true;
}
