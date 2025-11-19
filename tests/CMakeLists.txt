cmake_minimum_required(VERSION 3.16)
project(test_runner LANGUAGES CXX VERSION 0.0.0)

find_package(Catch2 3 REQUIRED)

add_executable(test_runner
    test_libnetwork.cpp
)

target_link_libraries(test_runner
        PRIVATE
        Catch2::Catch2WithMain
        RGLC::network
)

target_compile_features(test_runner PRIVATE cxx_std_17)

include(CTest)
include(Catch)
catch_discover_tests(test_runner)