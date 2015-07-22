
#The version number of servermonitor
set (servermonitor_VERSION_MAJOR 2)
set (servermonitor_VERSION_MINOR 0)
set (servermonitor_VERSION_MINOR 0)

set (CONFIGDIR "/etc" CACHE PATH "Define the directory where the config file is located")

# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-Wextra -O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")


# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()


set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.47.0 COMPONENTS system filesystem thread date_time regex REQUIRED)

find_path (
    SIMPLELOGGER_HEADER
    simplelogger.h
    PATH_SUFFIXES simplelogger/
)

find_library (
    SIMPLELOGGER_LIB
    simplelogger
)

find_path(
    SYSTEMD_INCLUDE_DIR
    NAMES sd-daemon.h
    PATH_SUFFIXES include/ systemd/
)


# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} # add bin dir because of the config header file
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/thirdparty/pugixml-1.6/src
    ${SIMPLELOGGER_HEADER}
)

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/appobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpuobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/globalutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mailer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memoryobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/monitorworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/observer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/procobserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/smconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sysvapp.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/appobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpuobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/globalutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mailer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memoryobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/monitorworker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/observer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/procobserver.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/smconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sysvapp.h
)

add_executable(serverMonitor ${HEADERS} ${SOURCE} ${PUGIHEADERS} ${PUGISOURCE})
target_link_libraries(serverMonitor ${Boost_LIBRARIES} ${SIMPLELOGGER_LIB})

INSTALL(TARGETS serverMonitor
    RUNTIME DESTINATION bin
)
INSTALL(FILES ${CONFIG_FILES} DESTINATION ${CONFIGDIR})

