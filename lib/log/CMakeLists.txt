# Try to find spdlog::spdlog in system
find_package(spdlog REQUIRED)

add_library(log_lib STATIC
    src/log.cpp
)

target_include_directories(log_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(log_lib PUBLIC
    RGLC::fs
    spdlog::spdlog
)

target_compile_features(log_lib PUBLIC cxx_std_17)

add_library(RGLC::log ALIAS log_lib)
