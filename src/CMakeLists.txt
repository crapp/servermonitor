

set (CONFIGDIR "/etc/sm" CACHE PATH "Define the directory where the config file is located")


# Initialize CXXFLAGS for Linux
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -Wno-reorder")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Compiler-specific C++11 activation.
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.9 or greater.")
    endif ()
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "${PROJECT_NAME} requires clang 3.3 or greater")
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

if (EALOGGER_EXTERNAL)
    message(STATUS "Trying to get ealogger sources from github and compile the library")
    include(ExternalProject)

    # add ealogger as external project. this will pull the sources from github,
    # compile them and install the library to a subfolder. as we are using a static
    # library we can link it directly
    ExternalProject_Add(
        ealogger_external
        URL https://github.com/crapp/ealogger/archive/v0.8.1.tar.gz
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ealogger"
        INSTALL_DIR install
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_BUILD_TYPE=Release
                   -DBUILD_SHARED_LIBS=OFF
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        )
    # get the install dir
    ExternalProject_Get_Property(ealogger_external install_dir)
    set (EALOGGER_LIB
        "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ealogger${CMAKE_STATIC_LIBRARY_SUFFIX}"
        )
    include_directories(
        "${install_dir}/include"
        )
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
    find_package(Threads REQUIRED)
else(EALOGGER_EXTERNAL)
    message(STATUS "Searching for ealogger in default locations")
    find_library(EALOGGER_LIB
        NAMES ealogger)
    if (EALOGGER_LIB)
        message(STATUS "Found ealogger library")
    else(EALOGGER_LIB)
        message(FATAL_ERROR "ealogger library not found")
    endif(EALOGGER_LIB)
    # search for ealogger
endif(EALOGGER_EXTERNAL)

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
target_link_libraries(serverMonitor ${Boost_LIBRARIES} ${SIMPLELOGGER_LIB} ${EALOGGER_LIB})

# make sure dependencies are build before our main target
if (EALOGGER_EXTERNAL)
    add_dependencies(serverMonitor ealogger_external)
    target_link_libraries(serverMonitor Threads::Threads)
endif()

INSTALL(TARGETS serverMonitor
    RUNTIME DESTINATION bin
)
INSTALL(FILES ${CONFIG_FILES} DESTINATION ${CONFIGDIR})

