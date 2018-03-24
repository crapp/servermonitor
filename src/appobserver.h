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

#ifndef APPOBSERVER_H
#define APPOBSERVER_H

#include "observer.h"
#include "sysvapp.h"

class AppObserver : public Observer {
   public:
    AppObserver(std::shared_ptr<SMConfig> cfg,
                std::shared_ptr<Mailer> mail);

   private:
    bool getData();
    void handleStreamData(std::vector<std::string> &v);
    void checkStreamData();
    void initLastDetection();

    void checkAppsRunning();
    void fillProcessPidMap();

    std::map<std::string, std::vector<std::string>> appsToCheck;

    std::vector<SysVApp> appVector;
    std::map<std::string, pid_t> processNamePid;
};

#endif  // APPOBSERVER_H
