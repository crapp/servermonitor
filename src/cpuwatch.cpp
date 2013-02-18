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

#include "cpuwatch.h"

CPUWatch::CPUWatch(boost::shared_ptr<Config> cfg)
{
    this->watch = true;
    this->cfg = cfg;
    this->foundSomething = false;
    this->procStreamPath = this->cfg->procCpuAvgLoad;
    this->msToWait = this->cfg->waitCpuThread;
}

void CPUWatch::queryCPUProc()
{
}

void CPUWatch::handleStreamData(vector<string> v)
{
    this->cpuLoad = v;
}

void CPUWatch::checkStreamData()
{
//    if (this->checkLastDetection() == false)
//    {
//        this->cpuLoad.clear();
//        return;
//    }
    float avg5, avg15;
    if (cpuLoad.size() == 5)
    {
        try
        {
            avg5 = boost::lexical_cast<float>(cpuLoad[1]);
            avg15 = boost::lexical_cast<float>(cpuLoad[2]);
        }
        catch (boost::bad_lexical_cast &ex)
        {
            cerr << "Can not cast: " << ex.what() << endl;
            return;
        }
        if (avg5 > this->cfg->cpuAvgLoad5 || avg15 > this->cfg->cpuAvgLoad15)
        {
            //TODO: Collect data and send e-mail!
            cout << "Average CPU load exceeded threshold ";
            BOOST_FOREACH(string &s, this->cpuLoad)
            {
                cout << s << " ";
            }
            cout << endl;
            this->foundSomething = true;
            this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
        }
    } else {
        cerr << "Average CPU load returned wrong number of informations: " << cpuLoad.size() << ", expected 5" << endl;
    }
    cpuLoad.clear();
}
