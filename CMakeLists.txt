cmake_minimum_required(VERSION 3.2)
project(serverMonitor)

#The version number of servermonitor
set (servermonitor_VERSION_MAJOR 0)
set (servermonitor_VERSION_MINOR 5)
set (servermonitor_VERSION_PATCH 0)

option (EALOGGER_EXTERNAL "Download and build ealogger" ON)

add_subdirectory(config)
add_subdirectory(thirdparty/pugixml-1.6/src)
add_subdirectory(src)

