cmake_minimum_required(VERSION 3.16)

project(rglc LANGUAGES CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IPC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/IPC")

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
find_package(LibArchive REQUIRED)
# ==============================

# Set Release options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "[CMAKE] [!] Using Release build...")
    message(STATUS "[CMAKE] [!] Compiler CXX: ${CMAKE_CXX_COMPILER}")
    message(STATUS "[CMAKE] [!] Compiler CXX flags: ${CMAKE_CXX_FLAGS_RELEASE}")
    message(STATUS "[CMAKE] [!] Compiler C: ${CMAKE_C_COMPILER}")

    set(HOME_DIR $ENV{HOME})
    set(CMAKE_INSTALL_PREFIX "${HOME_DIR}/.local")
endif()

# Path of binary for parent project (need for IPC)
set(RGC_BIN_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/bin/rglc")

# ==============================
# CONFIGURING HEADER FILES
# ==============================
execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE RGC_GIT_COMMIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "[CMAKE] [!] Current git commit SHA: ${RGC_GIT_COMMIT_SHA}")

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
# BUILDING STATIC LIBRARIES (MODULES)
# ==============================
add_subdirectory(lib/exception)
add_subdirectory(lib/common)
add_subdirectory(lib/log)
add_subdirectory(lib/fs)
add_subdirectory(lib/json_io)
add_subdirectory(lib/network)
add_subdirectory(lib/proto)
# ==============================

# ==============================
# SETUP TESTS USING THIRD-PARTY TEST LIBRARY
# ==============================
enable_testing()
add_subdirectory(tests)
# ==============================

# Get all sources and headers of project
file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE INC_FILES "inc/*.h" "inc/*.hpp")

# Get all third-party dependencies
file(GLOB TC_SRC_FILES "third-party/*.c" "third-party/*.cpp" "third-party/*.cc")
file(GLOB TC_INC_FILES "third-party/*.h" "third-party/*.hpp")

add_executable(rglc ${SRC_FILES} ${INC_FILES} ${TC_INC_FILES} ${TC_SRC_FILES} ${PROTO_GEN_SRC_FILES} ${PROTO_GEN_INC_FILES})

# Set dependency for force build
add_dependencies(rglc test_runner)

target_link_libraries(rglc PRIVATE
        LibArchive::LibArchive
        pthread
        RGLC::network
        RGLC::json_io
        RGLC::log
        RGLC::proto)

target_include_directories(rglc PRIVATE
    ${LIBARCHIVE_INCLUDE_DIRS}
    ${PROJECT_INCLUDE_DIRS}
)

# Install software into ~/.local/bin
install(TARGETS rglc DESTINATION bin)

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
