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

#ifndef GLOBALUTILS_H
#define GLOBALUTILS_H

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdexcept>      // std::invalid_argument

#define DEVELOPMENT 0
#define VERSION "1.0"

using namespace std;

extern int noOfActiveThreads;

/**
 *toString template
 */
template <typename T>
string toString(T t)
{
    ostringstream os;
    os << t;
    return os.str();
}

/**
 *
 *Use this template with "Explicit template argument specification"
 */
template<typename T>
T ConvertStringToNumber(const string& str)
{
    istringstream ss(str);
    T number = 0; //set a default value so we don't get some uninitialized garbage
    ss >> number;

    if (ss.fail( ))
    {
        // don't forget to catch this exception
        throw invalid_argument("ConvertStringToNumber:" + str);
    }

    return number;
}

/**
 * @brief executes a shell command and returns it's ouput as string
 * @param cmd -> The command to execute
 * @return returns ERROR when something failed, otherwise a string with the command
 * ouput will be returned.
 */
inline string execSysCmd(const char* cmd) {
    //NOTE: Append export LC_MESSAGES=C && to all commands?!
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        return "ERROR";
    char buffer[128];
    string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
        {
            try
            {
                result += buffer;
            }
            catch (...)
            {
                return "ERROR";
            }
        }
    }
    int retClose = pclose(pipe);
    if (retClose == -1)
        return "ERROR";
    return result;
}

#endif // GLOBALUTILS_H
