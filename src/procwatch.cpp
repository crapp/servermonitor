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

#include "procwatch.h"

ProcWatch::ProcWatch()
{
}

ProcWatch::~ProcWatch()
{
}

void ProcWatch::procWatchThreadLoop()
{
    try
    {
        while(this->watch)
        {
            this->procStream.open(this->procStreamPath);
            if (this->procStream.is_open())
            {
                string line;
                while(this->procStream.good())
                {
                    getline(this->procStream, line);
                    if (boost::algorithm::trim_copy(line) == "")
                        continue;
                    vector<string> v;
                    try
                    {
                        //do a regex split on all whitespaces and not only single ones
                        boost::algorithm::split_regex(v, line, boost::regex(" +"));
                        //erase last colon from key
                    }
                    //TODO: we need to catch specific boost exceptions
                    catch(exception &ex)
                    {
                        this->log->writeToLog(LVLERROR, this->threadID, ex.what());
                    }
                    catch(...)
                    {
                        this->log->writeToLog(LVLERROR, this->threadID, "MemoryWatch queryProc failed.");
                    }
                    this->handleStreamData(v);
                }
                this->checkStreamData();
                this->procStream.close();
                boost::this_thread::sleep(boost::posix_time::milliseconds(this->msToWait));
            } else {
                this->log->writeToLog(LVLERROR, this->threadID, "Can not open: " + this->procStreamPath.native());
                break;
            }
        }
    }
    catch (boost::thread_interrupted)
    {
        this->log->writeToLog(LVLDEBUG, this->threadID, "Thread interrupted");
    }
}

bool ProcWatch::checkLastDetection()
{
    if (this->foundSomething)
    {
        boost::posix_time::ptime ptimeNow = boost::posix_time::second_clock::universal_time();
        boost::posix_time::time_duration td = ptimeNow - this->ptimeLastDetection;
        if (td.total_seconds() < this->cfg->nextMailAfter)
        {
            return false;
        }
    }
    return true;
}
