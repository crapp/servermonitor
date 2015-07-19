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

#ifndef SMCONFIG_H
#define SMCONFIG_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "globalutils.h"
#include "pugixml.hpp"  //xml Parser

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
    std::map<std::string, std::vector<std::string>> getConfigMap(
        const std::string &xpath);

    bool getConfigFileOK();

private:
    std::string configFile;
    pugi::xml_document cfgdoc;
    bool configFileOK;
};

#endif  // SMCONFIG_H
