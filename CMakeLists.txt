# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.13)

project (HttpServer LANGUAGES CXX)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/ThirdParty)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(HttpServer
    HttpServer/main.cpp
    HttpServer/HttpServer.cpp
    HttpServer/Socket.cpp
    HttpServer/SocketClientsHolder.cpp
    HttpServer/ThreadPool.cpp
    HttpServer/SocketController.cpp
    HttpServer/SocketClientDataReceiver.cpp
    HttpServer/Url.cpp
    HttpServer/HtmlDecoder.cpp
    HttpServer/FileReader.cpp
)

ExternalProject_Add(Timer
    GIT_REPOSITORY https://github.com/rafalfelczynski/timer
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

include_directories(${EXTERNAL_INSTALL_LOCATION})
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_dependencies(HttpServer Timer)
target_link_libraries(HttpServer ${EXTERNAL_INSTALL_LOCATION}/lib/Timer.lib)

# Include sub-projects.
