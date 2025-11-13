cmake_minimum_required(VERSION 3.16)

project(RuGeolistsCreator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IPC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC/proto")

set (PROJECT_INCLUDE_DIRS
    ${IPC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party)

set(PROJECT_VERSION ${VERSION_STRING})
set(RGC_VERSION_STRING ${VERSION_STRING})
message(STATUS "[CMAKE] [!] Project version: ${PROJECT_VERSION}")

# ==============================
# FINDING DEPENDENCIES
# ==============================

# Try to find libcurl in system
find_package(CURL REQUIRED)

# Try to find log4cxx in system
find_package(log4cxx REQUIRED)

# Try to find libarchive in system
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

# Try to find jsoncpp in systemp
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Try to find c-ares in system
pkg_check_modules(CARES REQUIRED libcares)

# Try to find protobuf in system
find_package(Protobuf REQUIRED)

# Try to find abseil in system
find_package(absl REQUIRED)
# ==============================

# Set Release options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "[CMAKE] [!] Using Release build...") 

	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

    set(HOME_DIR $ENV{HOME})
    set(CMAKE_INSTALL_PREFIX "${HOME_DIR}/.local")
endif()

# Path of binary for parent project (need for IPC)
set(RGC_BIN_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/bin/RuGeolistsCreator")

# ==============================
# CONFIGURING HEADER FILES
# ==============================
execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE RGC_GIT_COMMIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Genereate software_info.hpp with correct version
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/software_info.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/software_info.hpp
    @ONLY
)

# Genereate ipc_chain.hpp with correct information
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC/ipc_chain.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC/ipc_chain.hpp
    @ONLY
)
# ==============================

# ==============================
# CREATING EXCEPTION STATIC LIBRARY
# ==============================
add_library(exception_lib STATIC
    lib/exception/src/exception.cpp
)

target_include_directories(exception_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/exception/inc
)

target_compile_features(exception_lib PUBLIC cxx_std_17)
# ==============================

# ==============================
# CREATING COMMON STATIC LIBRARY
# ==============================
add_library(common_lib STATIC
    lib/common/src/common.cpp
)

target_include_directories(common_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/common/inc
)

target_compile_features(common_lib PUBLIC cxx_std_17)
# ==============================

# ==============================
# CREATING LOG STATIC LIBRARY
# ==============================
add_library(log_lib STATIC
    lib/log/src/log.cpp
)

target_include_directories(log_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/log/inc
)

target_link_libraries(log_lib PUBLIC
    fs_lib
    log4cxx)

target_compile_features(log_lib PUBLIC cxx_std_17)
# ==============================

# ==============================
# CREATING FS STATIC LIBRARY
# ==============================
add_library(fs_lib STATIC
    lib/fs/src/fs_utils.cpp
)

target_include_directories(fs_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fs/inc
)

target_link_libraries(fs_lib PUBLIC
    log_lib)

target_compile_features(fs_lib PUBLIC cxx_std_17)
# ==============================

# ==============================
# CREATING JSON_IO STATIC LIBRARY
# ==============================
add_library(json_io_lib STATIC
    lib/json_io/src/json_io.cpp
)

target_include_directories(json_io_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/json_io/inc
)

target_link_libraries(json_io_lib PUBLIC
    ${JSONCPP_LIBRARIES}
    log_lib)

target_compile_features(json_io_lib PUBLIC cxx_std_17)
# ==============================

# ==============================
# CREATING NETWORK STATIC LIBRARY
# ==============================
add_library(network_lib STATIC
    lib/network/src/network.cpp
)

target_include_directories(network_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/network/inc
)

target_link_libraries(network_lib PUBLIC
    CURL::libcurl
    ${CARES_LIBRARIES}
    json_io_lib
    exception_lib
    common_lib)

target_compile_features(network_lib PUBLIC cxx_std_17)
# ==============================

# ==============================
# BUILDING PROTOBUF SOURCES
# ==============================
set(PROTO_SRC_FILES
    ${IPC_DIR}/proto/release_notes.proto)

# Create directory for generated protobuf files
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# Generate protobuf files using protoc directly at configure time
set(PROTO_GEN_SRC_FILES)
set(PROTO_GEN_INC_FILES)

foreach(PROTO_FILE ${PROTO_SRC_FILES})
    get_filename_component(PROTO_ABS_PATH ${PROTO_FILE} ABSOLUTE)
    get_filename_component(PROTO_NAME_WE ${PROTO_FILE} NAME_WE)

    set(GEN_SRC_FILE ${PROTO_GEN_DIR}/${PROTO_NAME_WE}.pb.cc)
    set(GEN_INC_FILE ${PROTO_GEN_DIR}/${PROTO_NAME_WE}.pb.h)

    # Generate files immediately during CMake configuration
    execute_process(
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --cpp_out=${PROTO_GEN_DIR}
        --proto_path=${IPC_DIR}/proto
        ${PROTO_ABS_PATH}
        RESULT_VARIABLE PROTOC_RESULT
        OUTPUT_VARIABLE PROTOC_OUTPUT
        ERROR_VARIABLE PROTOC_ERROR
    )

    if(PROTOC_RESULT EQUAL 0)
        message(STATUS "Successfully generated protobuf files for ${PROTO_NAME_WE}")
    else()
        message(FATAL_ERROR "Failed to generate protobuf files: ${PROTOC_ERROR}")
    endif()

    list(APPEND PROTO_GEN_SRC_FILES ${GEN_SRC_FILE})
    list(APPEND PROTO_GEN_INC_FILES ${GEN_INC_FILE})
endforeach()

message(STATUS "[CMAKE] [!] Generated protobuf sources: ${PROTO_GEN_SRC_FILES}")
message(STATUS "[CMAKE] [!] Generated protobuf headers: ${PROTO_GEN_INC_FILES}")
# ==============================

# Get all sources and headers of project
file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE INC_FILES "inc/*.h" "inc/*.hpp")

# Get all third-party dependencies
file(GLOB TC_SRC_FILES "third-party/*.c" "third-party/*.cpp" "third-party/*.cc")
file(GLOB TC_INC_FILES "third-party/*.h" "third-party/*.hpp")

add_executable(RuGeolistsCreator ${SRC_FILES} ${INC_FILES} ${TC_INC_FILES} ${TC_SRC_FILES} ${PROTO_GEN_SRC_FILES} ${PROTO_GEN_INC_FILES})

target_link_libraries(RuGeolistsCreator PRIVATE
        ${LIBARCHIVE_LIBRARIES}
        ${Protobuf_LIBRARIES}
        absl::strings
        absl::base
        pthread
        log4cxx
        network_lib
        json_io_lib
        log_lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(RuGeolistsCreator PRIVATE
        absl::log_internal_check_op)
endif()

target_include_directories(RuGeolistsCreator PRIVATE
    ${LIBARCHIVE_INCLUDE_DIRS}
    ${PROJECT_INCLUDE_DIRS}
    ${PROTO_GEN_DIR}
)

# Install software into ~/.local/bin
install(TARGETS RuGeolistsCreator DESTINATION bin)

# ==============================
# IPC SETTINGS
# ==============================
# Path of header with IPC chains
set(RGC_IPC_CHAIN_INC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC/ipc_chain.hpp" CACHE INTERNAL "IPC CHAIN FILE")

# Path of DIR with IPC stuff
set(RGC_IPC_CHAIN_DIR "${IPC_DIR}" CACHE INTERNAL "IPC CHAIN INCLUDE DIR")

set(RGC_IPC_PROTO_GEN_SRC_FILES "${PROTO_GEN_SRC_FILES}" CACHE INTERNAL "PROTOBUF SRC")
set(RGC_IPC_PROTO_GEN_INC_FILES "${PROTO_GEN_INC_FILES}" CACHE INTERNAL "PROTOBUF INC")

set(RGC_IPC_PROTO_INC_DIR "${PROTO_GEN_DIR}" CACHE INTERNAL "PROTOBUF INCLUDE DIR")
# ==============================

# Install log config into ~./config/..
install(CODE "
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{HOME}/.config/ru-geolists-creator)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/log4cxx.properties
    \$ENV{HOME}/.config/ru-geolists-creator/log4cxx.properties
  )
")
