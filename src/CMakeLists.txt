
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

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.55.0
    COMPONENTS system filesystem thread date_time regex REQUIRED)

find_package(Threads REQUIRED)

add_executable(servermonitor ${HEADERS} ${SOURCE} ${PUGIHEADERS} ${PUGISOURCE})

target_include_directories(servermonitor
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR} # add bin dir because of the config header file
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/pugixml-1.6/src
    PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/spdlog-0.16.3/include
)

target_link_libraries(servermonitor ${Boost_LIBRARIES} Threads::Threads)

set_property(TARGET servermonitor PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET servermonitor PROPERTY CXX_STANDARD 11)

install(TARGETS servermonitor
    RUNTIME DESTINATION bin
)
install(FILES ${CONFIG_FILES} DESTINATION ${CONFIGDIR})

