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


#include "memoryobserver.h"

MemoryObserver::MemoryObserver(boost::shared_ptr<SMConfig> cfg, boost::shared_ptr<Logger> log)
{
    this->cfg = cfg;
    this->log = log;
    this->watch = true;
    this->foundSomething = false;
    this->procStreamPath = this->cfg->getConfigValue("/config/sysstat/memory/processFilesystem");
    string msToWait = this->cfg->getConfigValue("/config/sysstat/memory/pollTime");
    if (!boost::spirit::qi::parse(msToWait.begin(), msToWait.end(), this->msToWait))
        this->msToWait = 60000;
    string nextMailAfter = this->cfg->getConfigValue("/config/email/secondsNextMail");
    if (!boost::spirit::qi::parse(nextMailAfter.begin(), nextMailAfter.end(), this->nextMailAfter))
        this->nextMailAfter = 43200; //every 12 hours
    string minMemFree = this->cfg->getConfigValue("/config/sysstat/memory/minimumFree");
    if (!boost::spirit::qi::parse(minMemFree.begin(), minMemFree.end(), this->minMemFree))
        this->minMemFree = 10000;
    string maxSwap = this->cfg->getConfigValue("/config/sysstat/memory/maximumSwap");
    if (!boost::spirit::qi::parse(maxSwap.begin(), maxSwap.end(), this->maxSwap))
        this->maxSwap = 0;
    string noValuesToCompare = this->cfg->getConfigValue("/config/sysstat/memory/noValuesCompare");
    if (!boost::spirit::qi::parse(noValuesToCompare.begin(), noValuesToCompare.end(), this->noValuesToCompare))
        this->noValuesToCompare = 10;
    this->threadID = 2;
    this->memInfoMap = boost::make_shared< map<string, float> >();
    this->lastMemFreeValues = boost::make_shared< list<float> >();
    this->log->writeToLog(LVLDEBUG, this->threadID, "MemoryObserver thread is starting");
}

void MemoryObserver::queryMemProc()
{
}

void MemoryObserver::handleStreamData(vector<string> &v)
{
    if (v.size() >= 2)
    {
        try
        {
            //erase last colon from key
            boost::algorithm::erase_last(v[0], ":");
            float f;
            bool cast = boost::spirit::qi::parse(v[1].begin(), v[1].end(), f);
            if (cast)
                this->memInfoMap->insert(pair<string, float>(v[0], f));
        }
        catch(exception &ex)
        {
            string s(ex.what());
            this->log->writeToLog(LVLERROR, this->threadID, "Exception: " + s);
        }
        catch(...)
        {
            this->log->writeToLog(LVLERROR, this->threadID, "MemoryWatch queryProc failed.");
            cerr << "MemoryWatch queryProc failed." << endl;
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Fuck the duck vector wrong size :/");
    }
}

void MemoryObserver::checkStreamData()
{
    if (this->checkLastDetection() == false)
    {
        this->memInfoMap->clear();
        return;
    }
    if (!this->memInfoMap->size() > 0)
            return;
    if (this->memInfoMap->find("MemTotal") != this->memInfoMap->end() &&
            this->memInfoMap->find("MemFree") != this->memInfoMap->end() &&
            this->memInfoMap->find("SwapTotal") != this->memInfoMap->end() &&
            this->memInfoMap->find("SwapFree") != this->memInfoMap->end())
    {
        //TODO: List size is hardcoded. Should be set in config
        if (this->lastMemFreeValues->size() < this->noValuesToCompare)
        {
            this->lastMemFreeValues->push_back((*this->memInfoMap)["MemFree"]);
        } else {
            this->lastMemFreeValues->erase(this->lastMemFreeValues->begin());
            this->lastMemFreeValues->push_back((*this->memInfoMap)["MemFree"]);
            /*NOTE: we could hold a sum as a class member and update it everytime the vector gets updated.
             *      No more need for foreach here. But ok the list is usually small...
             */
            float sum = 0;
            BOOST_FOREACH(const float &f, (*this->lastMemFreeValues))
            {
                sum += f;
            }
            if (((*this->memInfoMap)["MemTotal"] - (sum/this->lastMemFreeValues->size())) < this->minMemFree)
            {
                //TODO: Send E-Mail on low memory!
                this->log->writeToLog(LVLDEBUG, this->threadID, "Not much memory left");
                this->foundSomething = true;
                this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
            }
        }

        if (((*this->memInfoMap)["SwapTotal"] - (*this->memInfoMap)["SwapFree"]) > this->maxSwap)
        {
            //TODO: Send E-Mail on swap usage!
            this->log->writeToLog(LVLDEBUG, this->threadID, "System is swapping :/");
            this->foundSomething = true;
            this->ptimeLastDetection = boost::posix_time::second_clock::universal_time();
        }
    } else {
        this->log->writeToLog(LVLERROR, this->threadID, "Fuck the duck not in memInfoMap :/");
    }
    this->memInfoMap->clear();
}
