//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013 - 2016  Christian Rapp
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

#ifndef CPUOBSERVER_H
#define CPUOBSERVER_H

#include "procobserver.h"

class CPUObserver : public ProcObserver
{
public:
    CPUObserver(boost::shared_ptr<SMConfig> cfg,
                boost::shared_ptr<SimpleLogger> log,
                boost::shared_ptr<Mailer> mail);

    ~CPUObserver();

private:
    std::vector<std::string> cpuLoad;
    float cpuAvgLoad5;
    float cpuAvgLoad15;

    void handleStreamData(std::vector<std::string> &v);
    void checkStreamData();
    void initLastDetection();
    void composeMailMessage(std::string &msg);
};

#endif  // CPUOBSERVER_H
