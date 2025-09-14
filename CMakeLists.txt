cmake_minimum_required(VERSION 3.16)

project(RuGeolistsCreator LANGUAGES CXX VERSION 0.14.0)

# Create version string
set(RGC_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IPC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC/proto")

set (PROJECT_INCLUDE_DIRS
    ${IPC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party)

# Try to find libcurl in system
find_package(CURL REQUIRED)

# Try to find log4cxx in system
find_package(log4cxx REQUIRED)

# Try to find libarchive in system
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

# Try to find jsoncpp in systemp
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Try to find protobuf in system
find_package(Protobuf REQUIRED)

# Try to find abseil in system
find_package(absl REQUIRED)

# Set .local as destination for deploy
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(HOME_DIR $ENV{HOME})
    set(CMAKE_INSTALL_PREFIX "${HOME_DIR}/.local")
endif()

# Path of binary for parent project (need for IPC)
set(RGC_BIN_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/bin/RuGeolistsCreator")

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

message("(PROTOBUF) Generated sources: ${PROTO_GEN_SRC_FILES}")
message("(PROTOBUF) Generated headers: ${PROTO_GEN_INC_FILES}")

# Get all sources and headers of project
file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE INC_FILES "inc/*.h" "inc/*.hpp")

# Get all third-party dependencies
file(GLOB TC_SRC_FILES "third-party/*.c" "third-party/*.cpp" "third-party/*.cc")
file(GLOB TC_INC_FILES "third-party/*.h" "third-party/*.hpp")

add_executable(RuGeolistsCreator ${SRC_FILES} ${INC_FILES} ${TC_INC_FILES} ${TC_SRC_FILES} ${PROTO_GEN_SRC_FILES} ${PROTO_GEN_INC_FILES})

target_link_libraries(RuGeolistsCreator PRIVATE
        CURL::libcurl
        ${LIBARCHIVE_LIBRARIES}
        ${JSONCPP_LIBRARIES}
        ${Protobuf_LIBRARIES}
        absl::log_internal_check_op
        absl::strings
        absl::base
        pthread
        log4cxx)

target_include_directories(RuGeolistsCreator PRIVATE
    ${LIBARCHIVE_INCLUDE_DIRS}
    ${PROJECT_INCLUDE_DIRS}
    ${PROTO_GEN_DIR}
)

# Install software into ~/.local/bin
install(TARGETS RuGeolistsCreator DESTINATION bin)

# =========== IPC
# Path of header with IPC chains
set(RGC_IPC_CHAIN_INC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC/ipc_chain.hpp" PARENT_SCOPE)

# Path of DIR with IPC stuff
set(RGC_IPC_CHAIN_DIR "${IPC_DIR}" PARENT_SCOPE)

set(RGC_IPC_PROTO_GEN_SRC_FILES "${PROTO_GEN_SRC_FILES}" CACHE INTERNAL "PROTOBUF SRC")
set(RGC_IPC_PROTO_GEN_INC_FILES "${PROTO_GEN_INC_FILES}" CACHE INTERNAL "PROTOBUF INC")

set(RGC_IPC_PROTO_INC_DIR "${PROTO_GEN_DIR}" PARENT_SCOPE)
# ===========

# Install log config into ~./config/..
install(CODE "
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{HOME}/.config/ru-geolists-creator)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/log4cxx.properties
    \$ENV{HOME}/.config/ru-geolists-creator/log4cxx.properties
  )
")
