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

#ifndef SMCONFIG_H
#define SMCONFIG_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "globalutils.h"
#include "pugixml.hpp" //xml Parser

namespace smconfig_constants
{
// NODES
const char *const N_CONFIG = "config";
const char *const N_COMMON = "common";

const char *const N_EMAIL = "email";
const char *const N_MAILCOMMAND = "mailCommand";
const char *const N_MAILTO = "mailTo";
const char *const N_MAILFROM = "mailFrom";
const char *const N_DATACOLLETORS = "dataCollectors";
const char *const N_COLLECTOR = "collector";

const char *const N_LOGGER = "logger";
const char *const N_LOGDIR = "logDir";
const char *const N_MINLOGLEVEL = "minLogLevel";
const char *const N_logtimeformat = "logtimeformat";

const char *const N_OBSERVER = "observer";
const char *const N_SYSHEALTH = "syshealth";
const char *const N_CHECK = "check";
const char *const N_SYSSTAT = "sysstat";
const char *const N_POLLTIME = "pollTime";
const char *const N_SECONDSNEXTMAIL = "secondsNextMail";
const char *const N_CPU = "cpu";
const char *const N_PROCESSFILESYSTEM = "processFilesystem";
const char *const N_AVG5THRESHOLD = "avg5threshold";
const char *const N_AVG15THRESHOLD = "avg15threshold";
const char *const N_MEMORY = "memory";
const char *const N_MINIMUMFREE = "minimumFree";
const char *const N_MAXSWAP = "maximumSwap";
const char *const N_NOVALUES = "noValuesCompare";
const char *const N_FILESYSTEM = "filesystem";
const char *const N_PATH = "path";
const char *const N_APPLICATIONS = "applications";
const char *const N_SYSV = "sysv";
const char *const N_APP = "app";
const char *const N_SYSTEMD = "systemd";
const char *const N_SERVICE = "service";

//parameters

const char *const N_PATH_NAME = "name";
const char *const N_PATH_ABSPATH = "abspath";
const char *const N_PATH_MINFREE = "minfree";
const char *const N_PATH_CHECK = "check";

const char *const N_APP_NAME = "name";
const char *const N_APP_RESTART = "true";
const char *const N_APP_CHECK = "1";
const char *const N_APP_RESTARTCMD = "restartcmd";

const char *const N_SERVICE_NAME = "name";
const char *const N_SERVICE_CHECK = "check";

} // namespace smconfig_constants

/**
 * @brief The SMConfig class provides access to the xml based configuration via
 * xPath querries
 *
 * @author Christian Rapp crapp
 */
class SMConfig
{
public:
    SMConfig(const std::string &configPath);
    std::string getConfigValue(const std::string &xpath);
    std::map<std::string, std::vector<std::string>>
    getConfigMap(const std::string &xpath);

    bool getConfigFileOK();

private:
    std::string configFile;
    pugi::xml_document cfgdoc;
    bool configFileOK;
};

#endif // SMCONFIG_H
