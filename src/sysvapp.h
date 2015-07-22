//  ServerMonitor is a service to monitor a linux system
//  Copyright (C) 2013 - 2015  Christian Rapp
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

#ifndef SYSVAPP_H
#define SYSVAPP_H

/**
 * Include to check whether pid is a running process using readlink on exe
 */
#include <unistd.h>
#include <cstring>

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <simplelogger/simplelogger.h>

class SysVApp
{
public:
    SysVApp(boost::shared_ptr<SimpleLogger> log, std::string processName,
            pid_t pid);
    virtual ~SysVApp();

    bool isAlive() const;

private:
    boost::shared_ptr<SimpleLogger> log;
    std::string name;
    pid_t pid;
};

#endif  // SYSVAPP_H
