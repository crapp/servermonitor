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


#include "logger.h"

Logger::Logger(boost::shared_ptr<SMConfig> cfg) : cfg(cfg), doLog(false)
{
    this->logDir = this->cfg->getConfigValue("/config/logger/logDir");
    this->logLevels = boost::assign::map_list_of (0, "DEBUG") (1, "INFO") (2, "WARN") (3, "ERROR");
    try
    {
        this->maxLogFiles = ConvertStringToNumber<int>(this->cfg->getConfigValue("/config/logger/maxLogFiles"));
    }
    catch (const invalid_argument &ex)
    {
        cerr << "Can not convert \"logger/maxLogFiles\" " << ex.what() << endl;
        this->maxLogFiles = 3;
    }
    this->checkLogDir();
    this->setLogFile();
}

//define static mutex
boost::mutex Logger::mtx;

void Logger::writeToLog(const int &debugLevel, const int &threadID, const string &msg)
{
    //lock the logger with a lock_guard. this will be freed if it runs out of scope
    boost::lock_guard<boost::mutex> lockGuard(Logger::mtx);
    if (doLog)
    {
        // check if date is still current date
        // + boost::gregorian::days(1) --> add one day
        if (this->logDate < boost::gregorian::day_clock::local_day())
        {
            this->setLogFile();
            this->checkMaxLogFile();
        }
        //get current posix time microsec resolution for log ouputs.
        boost::posix_time::ptime dt = boost::posix_time::microsec_clock::local_time();
        ofstream logFileStream;
        logFileStream.open(this->logFile.c_str(), ios::app);
        if (logFileStream.is_open())
        {
            logFileStream << boost::posix_time::to_simple_string(dt) << ": [" <<
                             this->logLevels[debugLevel] << "]" << "[" << boost::this_thread::get_id() <<
                             "|" << threadID << "] - " << msg << endl;
            logFileStream.close();
        } else {
            cerr << "Can not write to logfile " << this->logFile << endl;
        }
        //TODO: Add an alternative logger to stdout/stderr?
//        if (debugLevel == LVLDEBUG)
//        {
//            cout << boost::posix_time::to_simple_string(dt) << ": " <<
//                    "Thread: " << threadID << " sysID: " << boost::this_thread::get_id() <<
//                    " -- " << msg << endl;
//        }
//        if (debugLevel == LVLERROR)
//        {
//            cerr << boost::posix_time::to_simple_string(dt) << ": " <<
//                    "Thread: " << threadID << " sysID: " << boost::this_thread::get_id() <<
//                    " -- " << msg << endl;
//        }
    }
}

void Logger::checkLogDir()
{
    bool doLog = true;
    try
    {
        //get all parts of the logDir
        vector<string> pathParts;
        boost::algorithm::split(pathParts, this->logDir, boost::algorithm::is_any_of("/"));
        string path = "";
        // create every level of the logdir path if it not exists. if something goes wrong we set doLog to false.
        // this means there will be no logging activity anymore.
        for (vector<string>::const_iterator it = pathParts.begin(); it != pathParts.end(); it++)
        {
            if (*it == "")
                continue;
            if (!boost::filesystem::is_directory(path + "/" + *it))
            {
                boost::filesystem::create_directory(path + "/" + *it);
            }
            path += "/" + *it;
        }
    }
    catch(boost::filesystem::filesystem_error &ex)
    {
        cerr << ex.what() << "  " << ex.code() << endl;
        doLog = false;
    }
    catch (...)
    {
        cerr << "Error while creating logDir" << endl;
        doLog = false;
    }

    this->doLog = doLog;
}

void Logger::checkMaxLogFile()
{
    //second check there are not to many logFiles in logDir
    boost::filesystem::directory_iterator dirIter(this->logDir);
    boost::filesystem::directory_iterator endIter;
    int count = distance(dirIter, endIter);
    //TODO add an additional ckeck for logfile size
    if (count > this->maxLogFiles)
    {
        vector<string> filesInFolder;
        //Find all files in folder and put them into a vector
        for(boost::filesystem::directory_iterator dirIter(this->logDir); dirIter != endIter; dirIter++)
        {
            //Fix in older boost::filesystem implementations filename() directly returns a string
#if BOOST_VERSION / 100 % 1000 < 52
            filesInFolder.push_back(dirIter->path().filename());
#else
            filesInFolder.push_back(dirIter->path().filename().string());
#endif

        }
        //sort the vector, oldest files are topmost
        sort(filesInFolder.begin(), filesInFolder.end());
        //delet all files no longer needed
        for (int i = 0; i < count-this->maxLogFiles ; i++)
        {
            try
            {
                string fileToDel = this->logDir + "/" + filesInFolder[i];
                if (boost::filesystem::is_regular_file(fileToDel))
                    boost::filesystem::remove(fileToDel);
            }
            catch(const boost::filesystem::filesystem_error &ex)
            {
                cerr << ex.what() << "  " << ex.code() << endl;
            }
            catch (...)
            {
                cerr << "Error while deleting " << filesInFolder[i] << endl;
            }
        }
    }
}

void Logger::setLogFile()
{
    //first we need to find a new logFile name
    boost::gregorian::date d = boost::gregorian::day_clock::local_day();
    this->logDate = d;
    vector<string> dateParts;
    string extIsoDate = boost::gregorian::to_iso_extended_string(d);
    boost::algorithm::split(dateParts, extIsoDate, boost::algorithm::is_any_of("-"));
    this->logFile = this->logDir + "/" + "serverMonitor" + dateParts[0] + dateParts[1] + dateParts[2] + ".log";
}
