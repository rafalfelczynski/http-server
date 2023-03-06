file(GLOB TEST_FILES
    "tests/*.cpp")

add_executable(HttpServer-ut ${TEST_FILES})

target_include_directories(HttpServer-ut SYSTEM PUBLIC ${googletest_SOURCE_DIR}/googletest/include)
target_include_directories(HttpServer-ut SYSTEM PUBLIC ${googletest_SOURCE_DIR}/googlemock/include)
target_link_libraries(HttpServer-ut gtest)

