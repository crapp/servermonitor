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

#include "sysvapp.h"
#include "constants.h"

SysVApp::SysVApp(std::string processName, pid_t pid)
    : name(processName), pid(pid) {}

SysVApp::~SysVApp() { this->log = spdlog::get(sm_constants::LOGGER); }

bool SysVApp::isAlive() const {
    namespace fs = boost::filesystem;
    if (fs::is_directory("/proc/" + std::to_string(this->pid))) {
        char readlinkBuf[1024];
        std::string p = "/proc/" + std::to_string(this->pid) + "/exe";
        if (readlink(p.c_str(), readlinkBuf, sizeof(readlinkBuf) - 1) != -1) {
            if (fs::path(readlinkBuf).filename() == this->name) {
                return true;
            }
        } else {
            this->log->error("Can not read exe link in /proc/" +
                             std::to_string(pid) + "\n" + strerror(errno));
        }
    }
    return false;
}
