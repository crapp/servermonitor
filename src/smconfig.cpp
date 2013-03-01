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

#include "smconfig.h"

SMConfig::SMConfig()
{
    if (DEVELOPMENT == 1)
    {
        this->configFile = "../../src/config/config.xml";
    } else {
        this->configFile = "/etc/serverMonitor/config.xml";
    }
    pugi::xml_parse_result result = this->cfgdoc.load_file(this->configFile.c_str());
    if (!result)
    {
        //TODO: Log error and quit application
    }
}

string SMConfig::getConfigValue(const string &xpath)
{
    try {
        pugi::xpath_node node = this->cfgdoc.select_single_node(xpath.c_str());
        if (node != 0)
        {
            pugi::xml_text t = node.node().text();
            return t.as_string();
        }
    }
    catch (pugi::xpath_exception &ex)
    {
        //TODO: Patch this to the logger and end application
        cerr << ex.result() << endl;
        cerr << ex.what() << endl;
    }
    return "";
}

map< string, vector<string> > SMConfig::getConfigMap(const string &xpath)
{
    map< string, vector<string> > appMap;
    try {
        pugi::xpath_node_set nodes = this->cfgdoc.select_nodes(xpath.c_str());
        if (nodes.size() > 0)
        {
            //TODO: How can we loop over a pugi::xpath_node_set with BOOST_FOREACH
            for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); it++)
            {
                pugi::xpath_node nd = *it;
                vector<string> attribs;
                for (pugi::xml_attribute_iterator ait = nd.node().attributes_begin(); ait != nd.node().attributes_end(); ++ait)
                {
                    attribs.push_back(ait->value());
                }
                appMap.insert(pair< string, vector<string> >(attribs[0], attribs));
            }
        }
    }
    catch (pugi::xpath_exception &ex)
    {
        //TODO: Patch this to the logger and end application
        cout << ex.result() << endl;
        cout << ex.what() << endl;
    }
    return appMap;
}
