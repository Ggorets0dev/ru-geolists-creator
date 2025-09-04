cmake_minimum_required(VERSION 3.15)

project(RuGeolistsCreator LANGUAGES CXX VERSION 0.12.0)

# Create version string
set(RGC_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECT_INCLUDE_DIRS
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

# Genereate software_info.hpp with correct version
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/software_info.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/software_info.hpp
    @ONLY
)

set(PROTO_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/release_notes.proto)

# Build protobuf sources
protobuf_generate_cpp(PROTO_GEN_SRC_FILES PROTO_GEN_INC_FILES ${PROTO_SRC_FILES})

message("(PROTOBUF) Generated sources: ${PROTO_GEN_SRC_FILES}")
message("(PROTOBUF) Generated headers: ${PROTO_GEN_INC_FILES}")

# Get all sources and headers of project
file(GLOB SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB INC_FILES "inc/*.h" "inc/*.hpp")

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

target_include_directories(RuGeolistsCreator PRIVATE ${LIBARCHIVE_INCLUDE_DIRS} ${PROJECT_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

# Set .local as destination for deploy
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(HOME_DIR $ENV{HOME})
    set(CMAKE_INSTALL_PREFIX "${HOME_DIR}/.local")
endif()

# Install software into ~/.local/bin
install(TARGETS RuGeolistsCreator DESTINATION bin)

# Install log config into ~./config/..
install(CODE "
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory \$ENV{HOME}/.config/ru-geolists-creator)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/log4cxx.properties
    \$ENV{HOME}/.config/ru-geolists-creator/log4cxx.properties
  )
")
