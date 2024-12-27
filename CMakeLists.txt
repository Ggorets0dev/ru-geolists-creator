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

# Поиск всех файлов в папках src и inc
file(GLOB SRC_FILES "src/*.c" "src/*.cpp")
file(GLOB INC_FILES "inc/*.h" "inc/*.hpp")

add_executable(RuGeolistsCreator ${SRC_FILES} ${INC_FILES})

target_link_libraries(RuGeolistsCreator CURL::libcurl jsoncpp_lib ${LIBARCHIVE_LIBRARIES})

target_include_directories(RuGeolistsCreator PRIVATE ${LIBARCHIVE_INCLUDE_DIRS} "inc/")
