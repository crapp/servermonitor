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
#include "pugixml/pugixml.hpp" //xml Parser
//TODO: Can not include logger, then we need some forward declaration. Do we need this?
//#include "logger.h"

using namespace std;

class SMConfig
{
public:
    SMConfig();
    string getConfigValue(const string &xpath);
    map< string, vector<string> > getConfigMap(const string &xpath);
private:
    string configFile;
    pugi::xml_document cfgdoc;
};

#endif // SMCONFIG_H
