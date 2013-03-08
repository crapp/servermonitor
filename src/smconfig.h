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

#ifndef SMCONFIG_H
#define SMCONFIG_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "globalutils.h"
#include "pugixml/pugixml.hpp" //xml Parser


using namespace std;

/**
 * @brief The SMConfig class provides access to the xml based configuration via
 * xPath querries
 *
 * @author Christian Rapp crapp
 */
class SMConfig
{
public:
    SMConfig();
    string getConfigValue(const string &xpath);
    map< string, vector<string> > getConfigMap(const string &xpath);

    bool getConfigFileOK();
private:
    string configFile;
    pugi::xml_document cfgdoc;
    bool configFileOK;
};

#endif // SMCONFIG_H
