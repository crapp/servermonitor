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

MonitorWorker::MonitorWorker(boost::shared_ptr<Config> cfg, boost::shared_ptr<Logger> log) :
    cfg(cfg), log(log)
{
    this->threadID = 0;
}

MonitorWorker::~MonitorWorker()
{
}

int MonitorWorker::startMonitoring()
{
    //starting memory monitor
    this->mwatch = boost::make_shared<MemoryWatch>(this->cfg, this->log);
    this->mwatchThread = boost::make_shared<boost::thread>(boost::bind(&MemoryWatch::procWatchThreadLoop, this->mwatch));
    this->cpuwatch = boost::make_shared<CPUWatch>(this->cfg, this->log);
    this->cpuwatchThread = boost::make_shared<boost::thread>(boost::bind(&CPUWatch::procWatchThreadLoop, this->cpuwatch));
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
    if (boost::filesystem::exists(this->cfg->fifoPath))
    {
        try
        {
            boost::filesystem::remove(this->cfg->fifoPath);
        }
        catch(boost::filesystem::filesystem_error &ex)
        {
            //TODO: Stop ServerMonitor
            string s(ex.what());
            this->log->writeToLog(LVLERROR, this->threadID, "Can not delete existing named pipe: " + s);
        }
    }
    status = mkfifo(this->cfg->fifoPath.c_str(), 0666);
    //TODO: Check status of mkfifo and errno!
    while(1)
    {
        char buf[80];
        memset(&buf[0], 0, sizeof(buf)); //reset/initialize char array
        fifo = open(this->cfg->fifoPath.c_str(), O_RDONLY); //this is blocking until we receive something
        //TODO: Check status of open and errno
        do
        {
            //FIXME: This overrides our buffer buf in every loop
            res = read(fifo, buf, sizeof(buf));
            bytes_read += res;
        }while (res > 0);
        string pipeString(buf, bytes_read-1);
        bytes_read = 0;
        close(fifo);
        if (boost::algorithm::to_lower_copy(pipeString) == "exit")
        {
            this->mwatchThread->interrupt();
            this->cpuwatchThread->interrupt();
            this->mwatchThread->join();
            this->cpuwatchThread->join();
            break;
        }
    }
    try
    {
        boost::filesystem::remove(this->cfg->fifoPath);
    }
    catch(boost::filesystem::filesystem_error &ex)
    {
        //TODO: Stop ServerMonitor
        string s(ex.what());
        this->log->writeToLog(LVLERROR, this->threadID, "Can not delete existing named pipe: " + s);
    }
}
