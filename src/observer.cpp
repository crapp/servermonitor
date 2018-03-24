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

#include "observer.h"

#include "constants.h"

Observer::Observer(std::shared_ptr<SMConfig> cfg,
                   std::shared_ptr<Mailer> mail)
    : cfg(cfg), mail(mail) {
    this->log = spdlog::get(sm_constants::LOGGER);
}

void Observer::start() {
    try {
        while (this->watch) {
            if (!getData()) break;
            boost::this_thread::sleep(
                boost::posix_time::milliseconds(this->msToWait));
        }
    } catch (boost::thread_interrupted) {
        this->log->debug("Thread interrupted");
        noOfActiveThreads--;
        return;
    }
    noOfActiveThreads--;
    this->log->warn("Thread stopped because of an error");
    this->mail->sendmail(
        this->threadID, false, "Thread stopped unexpectedly",
        "Thread stopped because of an error. Number if still active Threads" +
            std::to_string(noOfActiveThreads));
}

bool Observer::checkTimeoutMail(const boost::posix_time::ptime &pt) {
    boost::posix_time::ptime ptimeNow =
        boost::posix_time::second_clock::universal_time();
    boost::posix_time::time_duration td = ptimeNow - pt;
    this->log->debug("TimeoutMail duration total seconds: " +
                     std::to_string(td.total_seconds()));
    this->log->debug("Threshold for new mail: " +
                     std::to_string(this->nextMailAfter));

    if (td.total_seconds() < this->nextMailAfter) {
        return false;
    }
    return true;
}
