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

#ifndef GLOBALUTILS_H
#define GLOBALUTILS_H

#include <cstdio>
#include <sstream>
#include <stdexcept>  // std::invalid_argument
#include <string>

#define DEVELOPMENT

extern int noOfActiveThreads;

/**
 * @brief executes a shell command and returns it's ouput as string
 * @param cmd -> The command to execute
 * @return returns ERROR when something failed, otherwise a string with the
 * command ouput will be returned.
 */
inline std::string execSysCmd(const char* cmd) {
    // NOTE: Append export LC_MESSAGES=C && to all commands?!
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            try {
                result += buffer;
            } catch (...) {
                result = "ERROR";
            }
        }
    }
    int retClose = pclose(pipe);
    if (retClose == -1) return "ERROR";
    return result;
}

#endif  // GLOBALUTILS_H
