#TODO: Adding a Version Number and Configured Header File
#http://cmake.org/cmake/help/cmake_tutorial.html
#The version number.
set (serverMonitor_VERSION_MAJOR 1)
set (serverMonitor_VERSION_MINOR 0)
set (ConfigDir /etc/serverMonitor)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

#set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)
#set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /opt/local/include)

#define a macro which we can use to set XCode properties.
macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY}
        ${XCODE_VALUE})
endmacro (set_xcode_property)

#if apple then use our macro to set XCode properties
if(APPLE)
    #set_xcode_property(GDALTester CLANG_CXX_LANGUAGE_STANDARD "c++11")
    #set_xcode_property(GDALTester CLANG_CXX_LIBRARY "libc++")
    set_xcode_property(serverMonitor LIBRARY_SEARCH_PATHS[arch=*] "/opt/local/lib")
    set_xcode_property(serverMonitor HEADER_SEARCH_PATHS[arch=*]
        "/opt/local/include")
    message("Setting some XCode specific options")
endif(APPLE)

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/appobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpuobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mailer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memoryobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/monitorworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/observer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/procobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/smconfig.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/appobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpuobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/globalutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mailer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memoryobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/monitorworker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/observer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/procobserver.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/smconfig.h
)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.42.0 COMPONENTS system filesystem thread date_time regex)

if(Boost_FOUND)
    message("Boost found in: " ${Boost_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})
    add_subdirectory(config)
    add_subdirectory(pugixml)
    add_executable(serverMonitor ${HEADERS} ${SOURCE} ${PUGIHEADERS} ${PUGISOURCE})
    target_link_libraries(serverMonitor ${Boost_LIBRARIES})

    INSTALL(TARGETS serverMonitor
        RUNTIME DESTINATION bin
    )
INSTALL(FILES ${CONFIG_FILES} DESTINATION ${ConfigDir})
endif(Boost_FOUND)

