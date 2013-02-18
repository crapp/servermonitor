# The version number.
set (ServerMonitor_VERSION_MAJOR 0)
set (ServerMonitor_VERSION_MINOR 1)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

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
    set_xcode_property(ServerMonitor LIBRARY_SEARCH_PATHS[arch=*] "/opt/local/lib")
    set_xcode_property(ServerMonitor HEADER_SEARCH_PATHS[arch=*]
        "/opt/local/include")
    message("Setting some XCode specific options")
endif(APPLE)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.42.0 COMPONENTS system filesystem thread date_time regex)

if(Boost_FOUND)
    message("Boost found in: " ${Boost_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(ServerMonitor main.cpp config.h cpuwatch.h cpuwatch.cpp logger.h logger.cpp mailer.h mailer.cpp memorywatch.h memorywatch.cpp monitorworker.h monitorworker.cpp procwatch.h procwatch.cpp)
    target_link_libraries(ServerMonitor ${Boost_LIBRARIES})
endif(Boost_FOUND)

