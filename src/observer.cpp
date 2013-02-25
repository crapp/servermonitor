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

#include "observer.h"

Observer::Observer()
{
}

Observer::~Observer()
{
}

void Observer::procFSThreadLoop()
{
    try
    {
        while(this->watch)
        {
            if (!getData())
                break;
            boost::this_thread::sleep(boost::posix_time::milliseconds(this->msToWait));
        }
    }
    catch (boost::thread_interrupted)
    {
        this->log->writeToLog(LVLDEBUG, this->threadID, "Thread interrupted");
    }
}

bool Observer::checkLastDetection()
{
    if (this->foundSomething)
    {
        boost::posix_time::ptime ptimeNow = boost::posix_time::second_clock::universal_time();
        boost::posix_time::time_duration td = ptimeNow - this->ptimeLastDetection;
        if (td.total_seconds() < this->nextMailAfter)
        {
            return false;
        }
    }
    return true;
}
