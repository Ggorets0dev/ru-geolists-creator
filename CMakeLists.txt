cmake_minimum_required(VERSION 3.16)

project(rglc LANGUAGES CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECT_INCLUDE_DIRS
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

add_executable(rglc ${SRC_FILES} ${INC_FILES} ${TC_INC_FILES} ${TC_SRC_FILES})

# Set dependency for force build
add_dependencies(rglc test_runner)

target_link_libraries(rglc PRIVATE
        LibArchive::LibArchive
        pthread
        RGLC::network
        RGLC::json_io
        RGLC::log)

target_include_directories(rglc PRIVATE
    ${LIBARCHIVE_INCLUDE_DIRS}
    ${PROJECT_INCLUDE_DIRS}
)

# Install software into ~/.local/bin
install(TARGETS rglc DESTINATION bin)
