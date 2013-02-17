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

MonitorWorker::MonitorWorker(boost::shared_ptr<Config> cfg) : cfg(cfg)
{
}

MonitorWorker::~MonitorWorker()
{

}

int MonitorWorker::startMonitoring()
{
    //starting memory monitor
    this->mwatch = boost::make_shared<MemoryWatch>(this->cfg);
    //MemoryWatch *mwatch = new MemoryWatch(this->cfg);
    //mwatch->queryProc();
    boost::thread mwatchThread(boost::bind(&MemoryWatch::queryProc, mwatch));
    mwatchThread.join();
    return 0;
}
