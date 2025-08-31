cmake_minimum_required(VERSION 3.15)

project(RuGeolistsCreator LANGUAGES CXX VERSION 0.8.1)

# Формирование строки версии
set(RGC_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party
    ${CMAKE_CURRENT_SOURCE_DIR}/proto)

# Поиск библиотеки libcurl
find_package(CURL REQUIRED)

# Поиск библиотеки libarchive
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

# Поиск библиотеки libjsoncpp
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Поиск утилиты protobuf
find_package(Protobuf REQUIRED)

# Пытаемся найти Abseil
find_package(absl REQUIRED)

# Генерация version.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/software_info.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/software_info.hpp
    @ONLY
)

# Сборка файлов protobuf
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
	${CMAKE_CURRENT_SOURCE_DIR}/proto/release_notes.proto)

message("(PROTOBUF) Generated sources: ${PROTO_SRCS}")
message("(PROTOBUF) Generated headers: ${PROTO_HDRS}")

# Получение основных исходников проекта
file(GLOB SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB INC_FILES "inc/*.h" "inc/*.hpp")

# Получение дополнительных исходников проекта
file(GLOB TC_SRC_FILES "third-party/*.c" "third-party/*.cpp" "third-party/*.cc")
file(GLOB TC_INC_FILES "third-party/*.h" "third-party/*.hpp")

add_executable(RuGeolistsCreator ${SRC_FILES} ${INC_FILES} ${TC_INC_FILES} ${TC_SRC_FILES} ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(RuGeolistsCreator 
	CURL::libcurl 
	${LIBARCHIVE_LIBRARIES} 
	${JSONCPP_LIBRARIES}
	${Protobuf_LIBRARIES}     
	absl::log_internal_check_op
        absl::strings
        absl::base
        pthread)

target_include_directories(RuGeolistsCreator PRIVATE ${LIBARCHIVE_INCLUDE_DIRS} ${PROJECT_INCLUDE_DIRS})

