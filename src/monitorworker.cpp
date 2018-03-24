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

#include "monitorworker.h"
#include "constants.h"

MonitorWorker::MonitorWorker(std::shared_ptr<SMConfig> cfg,
                             std::shared_ptr<Mailer> mail)
    : cfg(cfg), mail(mail) {
    this->log = spdlog::get(sm_constants::LOGGER);
    this->threadID = 0;
    this->fifopath = this->cfg->getConfigValue("/config/common/fifoPath");
    try {
        this->daysNextSyshealth = std::stoi(
            this->cfg->getConfigValue("/config/observer/syshealth/pollTime"));
    } catch (const std::exception &ex) {
        this->log->error(std::string("Can not parse \"syshealth/pollTime\" ") +
                         ex.what());
        ;
        this->daysNextSyshealth = 7;  // once a week
    }
    this->syshealthDate = boost::gregorian::day_clock::local_day() +
                          boost::gregorian::days(this->daysNextSyshealth);
}

MonitorWorker::~MonitorWorker() {}

int MonitorWorker::startMonitoring() {
    // starting cpu, memory and app observer
    if (this->cfg->getConfigValue("/config/observer/sysstat/cpu/check") ==
        "true") {
        this->log->info("CPU Observer is activated");
        this->cpuwatch = std::make_shared<CPUObserver>(this->cfg, this->mail);
        this->cpuwatchThread = std::make_shared<boost::thread>(
            boost::bind(&CPUObserver::start, this->cpuwatch));
        noOfActiveThreads++;
    }
    if (this->cfg->getConfigValue("/config/observer/sysstat/memory/check") ==
        "true") {
        this->log->info("Memory Observer is activated");
        this->mwatch =
            std::make_shared<MemoryObserver>(this->cfg, this->mail);
        this->mwatchThread = std::make_shared<boost::thread>(
            boost::bind(&MemoryObserver::start, this->mwatch));
        noOfActiveThreads++;
    }
    if (this->cfg->getConfigValue("/config/observer/applications/check") ==
        "true") {
        this->log->info("observer/applications Observer is activated");
        this->appwatch = std::make_shared<AppObserver>(this->cfg, this->mail);
        this->appwatchThread = std::make_shared<boost::thread>(
            boost::bind(&AppObserver::start, this->appwatch));
        noOfActiveThreads++;
    }
    this->log->debug("NoOfActiveThreads: " + std::to_string(noOfActiveThreads));
    // howto start a thread in a class
    // boost::thread t1(boost::bind(&MemoryWatch::memoryWatchThread, this));
    this->ipcNamedPipe();
    return 0;
}

void MonitorWorker::ipcNamedPipe() {
    int status, fifo, res;
    if (boost::filesystem::exists(this->fifopath)) {
        try {
            boost::filesystem::remove(this->fifopath);
        } catch (const boost::filesystem::filesystem_error &ex) {
            std::string s(ex.what());
            this->log->error("Can not delete existing named pipe: " + s);
            this->mail->sendmail(
                this->threadID, false, "Can not delete existing named pipe",
                "Can not delete existing named pipe, stopping service \n" + s);
            this->stopService();
        }
    }
    status = mkfifo(this->fifopath.c_str(), 0666);
    if (status != 0) {
        this->log->error("Can not create named pipe: " + this->fifopath +
                         " -- Error Number: " + std::to_string(errno));
        this->mail->sendmail(this->threadID, false, "Can not create named pipe",
                             "Can not create named pipe" + this->fifopath +
                                 " -- Error Number: " + std::to_string(errno) +
                                 "\nStopping service");
        this->stopService();
    }
    this->log->info("Starting to read from fifo");
    while (1) {
        char buf[80];
        memset(&buf[0], 0, sizeof(buf));  // reset/initialize char array
        fifo = open(this->fifopath.c_str(),
                    O_RDONLY | O_NONBLOCK);  // O_NONBLOCK prevents blocking
        std::string pipeString = "";
        do {
            res = read(fifo, buf, sizeof(buf));
            if (res > 0) {
                std::string s(buf, res - 1);
                pipeString += s;
            } else if (res < 0) {
                // check errno here
                if (errno == 11) {
                    // if errno is EAGAIN set res to 1 and the let the while
                    // loop run once more to get the data in the pipe.
                    res = 1;
                } else {
                    this->log->error(
                        "Can not read from named pipe: " + this->fifopath +
                        " -- Error Number: " + std::to_string(errno));
                    this->mail->sendmail(
                        this->threadID, false, "Can not read from named pipe",
                        "Can not read from named pipe: " + this->fifopath +
                            " -- Error Number: " + std::to_string(errno));
                }
            }
        } while (res > 0);
        close(fifo);
        if (pipeString.length() > 0) {
            this->log->info("Received message from fifo: " + pipeString);
            if (boost::algorithm::to_lower_copy(pipeString) == "exit") {
                this->stopService();
                break;
            }
        }
        if (noOfActiveThreads == 0) {
            this->log->warn(
                "All observer threads have stopped. Shutting "
                "down serverMonitor");
            this->mail->sendmail(this->threadID, false,
                                 "Shutting down serverMonitor",
                                 "All observer threads have stopped. Shutting "
                                 "down serverMonitor");
            this->stopService();
            break;
        }
        if (this->cfg->getConfigValue("/config/observer/syshealth/check")
                .compare("true") == 0) {
            std::string nextHealthMailDate =
                boost::gregorian::to_iso_extended_string(this->syshealthDate);
            this->log->debug("Next system status E-Mail on: " +
                             nextHealthMailDate);
            if (this->syshealthDate ==
                boost::gregorian::day_clock::local_day()) {
                this->syshealthDate =
                    boost::gregorian::day_clock::local_day() +
                    boost::gregorian::days(this->daysNextSyshealth);
                this->log->info("Sending system status E-Mail");
                this->log->info("Next E-Mail: " +
                                boost::gregorian::to_iso_extended_string(
                                    this->syshealthDate));
                this->mail->sendmail(this->threadID, true,
                                     "System status overview", "");
            }
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    }
}

void MonitorWorker::stopService() {
    this->log->info("Stopping service...");
    try {
        boost::filesystem::remove(this->fifopath);
    } catch (const boost::filesystem::filesystem_error &ex) {
        std::string s(ex.what());
        this->log->error("Can not delete existing named pipe: " + s);
        this->mail->sendmail(
            this->threadID, false, "Can not delete existing named pipe",
            "Can not delete existing named pipe while stopping service \n" + s);
    }
    if (this->cpuwatchThread != 0) {
        this->log->debug("Interrupting CPU observer");
        this->cpuwatchThread->interrupt();
        if (this->cpuwatchThread->joinable()) {
            this->log->debug("Waiting for CPU observer to stop");
            this->cpuwatchThread->join();
        }
    }
    if (this->mwatchThread != 0) {
        this->log->debug("Interrupting Memory observer");
        this->mwatchThread->interrupt();
        if (this->mwatchThread->joinable()) {
            this->log->debug("Waiting for Memory observer to stop");
            this->mwatchThread->join();
        }
    }
    if (this->appwatchThread != 0) {
        this->log->debug("Interrupting Applications observer");
        this->appwatchThread->interrupt();
        if (this->appwatchThread->joinable()) {
            this->log->debug("Waiting for Applications observer to stop");
            this->appwatchThread->join();
        }
    }
}
