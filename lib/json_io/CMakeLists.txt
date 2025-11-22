# Try to find jsoncpp in system
find_package(jsoncpp REQUIRED)

add_library(json_io_lib STATIC
    src/json_io.cpp
)

target_include_directories(json_io_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(json_io_lib PUBLIC
    RGLC::log
    JsonCpp::JsonCpp
)

target_compile_features(json_io_lib PUBLIC cxx_std_17)

add_library(RGLC::json_io ALIAS json_io_lib)
