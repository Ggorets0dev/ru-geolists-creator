cmake_minimum_required(VERSION 3.5)

project(RuGeolistsCreator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск библиотеки libcurl
find_package(CURL REQUIRED)

# Поиск библиотеки libjsoncpp
find_package(jsoncpp REQUIRED)

add_executable(RuGeolistsCreator main.cpp
    filter.hpp
    filter.cpp
    extract.hpp
    extract.cpp
    network.hpp
    network.cpp
    log.hpp)

# Подключаем libcurl к целевому исполняемому файлу
target_link_libraries(RuGeolistsCreator CURL::libcurl jsoncpp_lib)
