# Try to find libcurl in system
find_package(CURL REQUIRED)

# Try to find c-ares in system
pkg_check_modules(CARES REQUIRED libcares)

add_library(network_lib STATIC
    src/network.cpp
)

target_include_directories(network_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(network_lib PUBLIC
    CURL::libcurl
    ${CARES_LIBRARIES}
    RGLC::json_io
    RGLC::exception
    RGLC::common
)

target_compile_features(network_lib PUBLIC cxx_std_17)

add_library(RGLC::network ALIAS network_lib)
