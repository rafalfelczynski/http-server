# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.13)

project (HttpServer LANGUAGES CXX)

option(BUILD_TESTS "Build tests" ON)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/ThirdParty)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std:c++20")

add_executable(HttpServer
    HttpServer/main.cpp
    HttpServer/HttpServer.cpp
    HttpServer/Socket.cpp
    HttpServer/ConnectedSocket.cpp
    HttpServer/SocketClientsHolder.cpp
    HttpServer/ThreadPool.cpp
    HttpServer/SocketController.cpp
    HttpServer/Url.cpp
    HttpServer/HtmlDecoder.cpp
    HttpServer/FileReader.cpp
)

include(ExternalProject)
ExternalProject_Add(Timer
    PREFIX ${CMAKE_SOURCE_DIR}/libs
    DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/libs
    BINARY_DIR ${EXTERNAL_INSTALL_LOCATION}/timer
    GIT_REPOSITORY https://github.com/rafalfelczynski/timer
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
    UPDATE_DISCONNECTED TRUE
)

include_directories(${EXTERNAL_INSTALL_LOCATION})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/HttpServer)

add_dependencies(HttpServer Timer)
target_link_libraries(HttpServer ${EXTERNAL_INSTALL_LOCATION}/timer/Release/Timer.lib
    ws2_32.lib)

# Include sub-projects.
if (BUILD_TESTS)
    set(GTEST_ROOT_DIR ${CMAKE_SOURCE_DIR}/libs/googletest)

    include(FetchContent)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # include_directories(${googletest_SOURCE_DIR}/googletest/include)
    # include_directories(${googletest_SOURCE_DIR}/googlemock/include)

    # # Single download(git clone)
    # ExternalProject_Add(googletest
    #     PREFIX ${GTEST_ROOT_DIR}
    #     DOWNLOAD_DIR ${GTEST_ROOT_DIR}
    #     BINARY_DIR ${EXTERNAL_INSTALL_LOCATION}/lib/gtest
    #     GIT_REPOSITORY https://github.com/google/googletest.git
    #     CMAKE_ARGS -Dgtest_force_shared_crt=ON
    #     GIT_TAG v1.13.0
    #     # Disable update step
    #     UPDATE_COMMAND ""
    #     # Disable install step
    #     INSTALL_COMMAND ""
    # )
    # ExternalProject_Get_Property(googletest source_dir)
    # ExternalProject_Get_Property(googletest binary_dir)

    # include_directories(${source_dir}/googletest/include)
    # include_directories(${source_dir}/googlemock/include)

    # add_library(gtest STATIC IMPORTED)
    # set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/Release/gtest.lib)

    # add_library(gtest_main STATIC IMPORTED)
    # set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/Release/gtest_main.lib)

    # add_library(gmock STATIC IMPORTED)
    # set_property(TARGET gmock PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/Release/gmock.lib)

    # add_library(gmock_main STATIC IMPORTED)
    # set_property(TARGET gmock_main PROPERTY IMPORTED_LOCATION ${binary_dir}/lib/Release/gmock_main.lib)


    # add_dependencies(gtest googletest)
    # add_dependencies(gtest_main googletest)
    # add_dependencies(gmock googletest)
    # add_dependencies(gmock_main googletest)

    include(cmake/TestFiles.cmake)
endif()
