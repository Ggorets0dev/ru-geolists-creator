cmake_minimum_required(VERSION 3.15)

project(RuGeolistsCreator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECT_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/third-party
    ${CMAKE_SOURCE_DIR}/proto)

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

