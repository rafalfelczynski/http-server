# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.13)

project ("HttpServer" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable("HttpServer"
    HttpServer/main.cpp
    HttpServer/main.cpp
    HttpServer/Socket.cpp
    HttpServer/SocketClientsHolder.cpp
    HttpServer/ThreadPool.cpp
)

# Include sub-projects.
add_subdirectory ("HttpServer")
