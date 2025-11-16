# Try to find libcurl in system
find_package(CURL REQUIRED)

# Try to find c-ares in system
pkg_check_modules(CARES REQUIRED libcares)

# Ищем заголовки bgpdump.h
find_path(BGPDUMP_INCLUDE_DIR bgpdump.h
        PATHS /usr/local/include /usr/include
)

# Ищем библиотеку libbgpdump.so / .a
find_library(BGPDUMP_LIBRARY bgpdump
        PATHS /usr/local/lib /usr/lib
)

#if(NOT BGPDUMP_INCLUDE_DIR OR NOT BGPDUMP_LIBRARY)
#    message(FATAL_ERROR "libbgpdump not found")
#endif()

file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE INC_FILES "inc/*.h" "inc/*.hpp")

add_library(network_lib STATIC
    ${SRC_FILES}
)

target_include_directories(network_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(network_lib PUBLIC
    CURL::libcurl
    ${CARES_LIBRARIES}
    ${BGPDUMP_LIBRARY}
    RGLC::json_io
    RGLC::exception
    RGLC::common
)

target_compile_features(network_lib PUBLIC cxx_std_17)

add_library(RGLC::network ALIAS network_lib)
