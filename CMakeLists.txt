cmake_minimum_required(VERSION 3.5)
project(serverMonitor VERSION 0.5.0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/config.h
    @ONLY
)

set (CONFIGDIR "/etc/sm" CACHE PATH "Define the directory where the config file is located")

add_subdirectory(thirdparty/pugixml-1.6/src)
add_subdirectory(config)
add_subdirectory(src)

