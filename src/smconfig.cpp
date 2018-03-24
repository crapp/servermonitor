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

#include "smconfig.h"

SMConfig::SMConfig(const std::string &configPath) {
#ifdef DEVELOPMENT
    this->configFile = "../../config/config.xml";
#else
    this->configFile = configPath + "/config.xml";
#endif
    this->configFileOK = false;
    pugi::xml_parse_result result =
        this->cfgdoc.load_file(this->configFile.c_str());
    if (result) {
        this->configFileOK = true;
    }
}

std::string SMConfig::getConfigValue(const std::string &xpath) {
    try {
        pugi::xpath_node node = this->cfgdoc.select_single_node(xpath.c_str());
        if (node != 0) {
            pugi::xml_text t = node.node().text();
            return t.as_string();
        }
    } catch (pugi::xpath_exception &ex) {
        std::cerr << ex.result() << std::endl;
        std::cerr << ex.what() << std::endl;
    }
    return "";
}

std::map<std::string, std::vector<std::string>> SMConfig::getConfigMap(
    const std::string &xpath) {
    std::map<std::string, std::vector<std::string>> appMap;
    try {
        pugi::xpath_node_set nodes = this->cfgdoc.select_nodes(xpath.c_str());
        if (nodes.size() > 0) {
            // TODO: How can we loop over a pugi::xpath_node_set with
            // BOOST_FOREACH
            for (pugi::xpath_node_set::const_iterator it = nodes.begin();
                 it != nodes.end(); it++) {
                pugi::xpath_node nd = *it;
                std::vector<std::string> attribs;
                for (pugi::xml_attribute_iterator ait =
                         nd.node().attributes_begin();
                     ait != nd.node().attributes_end(); ++ait) {
                    attribs.push_back(ait->value());
                }
                appMap.insert(std::pair<std::string, std::vector<std::string>>(
                    attribs[0], attribs));
            }
        }
    } catch (pugi::xpath_exception &ex) {
        std::cout << ex.result() << std::endl;
        std::cout << ex.what() << std::endl;
    }
    return appMap;
}

bool SMConfig::getConfigFileOK() { return this->configFileOK; }
