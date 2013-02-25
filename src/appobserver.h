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

#ifndef APPOBSERVER_H
#define APPOBSERVER_H

#include <map>
#include "observer.h"

class AppObserver : public Observer
{
public:
    AppObserver(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log);
    bool getData();
    void handleStreamData(vector<string> &v);
    void checkStreamData();
private:
    int threadID;
};

#endif // APPOBSERVER_H
