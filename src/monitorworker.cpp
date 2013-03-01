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

#include "monitorworker.h"

MonitorWorker::MonitorWorker(boost::shared_ptr<SMConfig> cfg,
                             boost::shared_ptr<Logger> log, boost::shared_ptr<Mailer> mail) :
    cfg(cfg), log(log), mail(mail)
{
    this->threadID = 0;
    this->fifopath = this->cfg->getConfigValue("/config/common/fifoPath");
}

MonitorWorker::~MonitorWorker()
{
}

int MonitorWorker::startMonitoring()
{
    //starting cpu, memory and app observer
    if (this->cfg->getConfigValue("/config/sysstat/cpu/check") == "true")
    {
        this->log->writeToLog(LVLINFO, this->threadID, "CPU Observer is activated");
        this->cpuwatch = boost::make_shared<CPUObserver>(this->cfg, this->log, this->mail);
        this->cpuwatchThread = boost::make_shared<boost::thread>(boost::bind(&CPUObserver::threadLoop, this->cpuwatch));
        noOfActiveThreads++;
    }
    if (this->cfg->getConfigValue("/config/sysstat/memory/check") == "true")
    {
        this->log->writeToLog(LVLINFO, this->threadID, "Memory Observer is activated");
        this->mwatch = boost::make_shared<MemoryObserver>(this->cfg, this->log, this->mail);
        this->mwatchThread = boost::make_shared<boost::thread>(boost::bind(&MemoryObserver::threadLoop, this->mwatch));
        noOfActiveThreads++;
    }
    if (this->cfg->getConfigValue("/config/applications/check") == "true")
    {
        this->log->writeToLog(LVLINFO, this->threadID, "Applications Observer is activated");
        this->appwatch = boost::make_shared<AppObserver>(this->cfg, this->log, this->mail);
        this->appwatchThread = boost::make_shared<boost::thread>(boost::bind(&AppObserver::threadLoop, this->appwatch));
        noOfActiveThreads++;
    }
    log->writeToLog(LVLDEBUG, 0, "NoOfActiveThreads: " + toString(noOfActiveThreads));
    //howto start a thread in a class
    //boost::thread t1(boost::bind(&MemoryWatch::memoryWatchThread, this));
    //t1.join();
    this->ipcNamedPipe();
    return 0;
}

void MonitorWorker::ipcNamedPipe()
{
    int status, fifo, res;
    int bytes_read = 0;
    if (boost::filesystem::exists(this->fifopath))
    {
        try
        {
            boost::filesystem::remove(this->fifopath);
        }
        catch(boost::filesystem::filesystem_error &ex)
        {
            string s(ex.what());
            this->log->writeToLog(LVLERROR, this->threadID,
                                  "Can not delete existing named pipe: " + s);
            this->stopService();
        }
    }
    status = mkfifo(this->fifopath.c_str(), 0666);
    if (status != 0)
    {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not create named pipe: " +
                              this->fifopath +  " -- Error Number: " + toString(errno));
        this->stopService();
    }
    while(1)
    {
        this->log->writeToLog(LVLINFO, this->threadID, "Starting to read from fifo");
        char buf[80];
        memset(&buf[0], 0, sizeof(buf)); //reset/initialize char array
        fifo = open(this->fifopath.c_str(), O_RDONLY); //this is blocking until we receive something
        string pipeString = "";
        do
        {
            res = read(fifo, buf, sizeof(buf));
            bytes_read += res;
            if (res > 0)
            {
                string s(buf, res-1);
                pipeString += s;
            } else if (res == -1) {
                this->log->writeToLog(LVLERROR, this->threadID, "Can not read from named pipe: "
                                      + this->fifopath +  " -- Error Number: " + toString(errno));
            }
        } while (res > 0);
        bytes_read = 0;

        this->log->writeToLog(LVLINFO, this->threadID, "Received message from fifo: "
                              + pipeString);

        close(fifo);
        if (boost::algorithm::to_lower_copy(pipeString) == "exit")
        {
            this->stopService();
            break;
        }
    }
}

void MonitorWorker::stopService()
{
    this->log->writeToLog(LVLINFO, this->threadID, "Stopping service...");
    try
    {
        boost::filesystem::remove(this->fifopath);
    }
    catch(boost::filesystem::filesystem_error &ex)
    {
        string s(ex.what());
        this->log->writeToLog(LVLERROR, this->threadID, "Can not delete existing named pipe: " + s);
    }
    if (this->cpuwatchThread != 0)
    {
        this->log->writeToLog(LVLINFO, this->threadID, "Interrupting CPU observer");
        this->cpuwatchThread->interrupt();
        if (this->cpuwatchThread->joinable())
        {
            this->log->writeToLog(LVLDEBUG, this->threadID, "Waiting for CPU observer to stop");
            this->cpuwatchThread->join();
        }
    }
    if (this->mwatchThread != 0)
    {
        this->log->writeToLog(LVLINFO, this->threadID, "Interrupting Memory observer");
        this->mwatchThread->interrupt();
        if (this->mwatchThread->joinable())
        {
            this->log->writeToLog(LVLDEBUG, this->threadID, "Waiting for Memory observer to stop");
            this->mwatchThread->join();
        }
    }
    if (this->appwatchThread != 0)
    {
        this->log->writeToLog(LVLINFO, this->threadID, "Interrupting Applications observer");
        this->appwatchThread->interrupt();
        if (this->appwatchThread->joinable())
        {
            this->log->writeToLog(LVLDEBUG, this->threadID, "Waiting for Applications observer to stop");
            this->appwatchThread->join();
        }
    }
}

