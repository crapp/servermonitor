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

#include "procobserver.h"

ProcObserver::ProcObserver()
{
}

bool ProcObserver::getData()
{
    this->log->writeToLog(LVLDEBUG, this->threadID, "Open stream " + this->procStreamPath);
    this->procStream.open(this->procStreamPath.c_str(), ifstream::in);
    if (this->procStream.is_open())
    {
        string line;
        while(this->procStream.good())
        {
            getline(this->procStream, line);
            if (boost::algorithm::trim_copy(line) == "")
                continue;
            this->log->writeToLog(LVLDEBUG, this->threadID, "Received line: " + line);
            vector<string> v;
            try
            {
                //do a regex split on all whitespaces and not only single ones
                boost::algorithm::split_regex(v, line, boost::regex(" +"));
            }
            //TODO: we need to catch specific boost exceptions
            catch(exception &ex)
            {
                this->log->writeToLog(LVLERROR, this->threadID, ex.what());
            }
            catch(...)
            {
                this->log->writeToLog(LVLERROR, this->threadID, "Could not split stram data with regex_split. General exception occured");
            }
            this->handleStreamData(v);
        }
        this->checkStreamData();
        this->procStream.close();
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Can not open: " + this->procStreamPath);
        return false;
    }
    return true;
}
