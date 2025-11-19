cmake_minimum_required(VERSION 3.8)
project(test LANGUAGES CXX VERSION 0.0.0)

find_package(Catch2 REQUIRED)

add_executable(all_tests
    test_libnetwork.cpp
)

target_link_libraries(all_tests
        PRIVATE
        Catch2::Catch2WithMain
        RGLC::network
)

target_compile_features(all_tests PRIVATE cxx_std_17)
