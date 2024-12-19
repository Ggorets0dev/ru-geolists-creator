cmake_minimum_required(VERSION 3.5)

project(RuGeolistsCreator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск библиотеки libcurl
find_package(CURL REQUIRED)

# Поиск библиотеки libjsoncpp
find_package(jsoncpp REQUIRED)

# Ищем библиотеку libarchive
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

add_executable(RuGeolistsCreator main.cpp
    filter.hpp
    filter.cpp
    extract.hpp
    extract.cpp
    network.hpp
    network.cpp
    log.hpp
    log.cpp
    dlc_toolchain.hpp
    dlc_toolchain.cpp
    json_io.hpp
    json_io.cpp
    archive.hpp
    archive.cpp)

target_link_libraries(RuGeolistsCreator CURL::libcurl jsoncpp_lib ${LIBARCHIVE_LIBRARIES})

target_include_directories(RuGeolistsCreator PRIVATE ${LIBARCHIVE_INCLUDE_DIRS})
