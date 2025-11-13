add_library(common_lib STATIC
    src/common.cpp
)

target_include_directories(common_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_compile_features(common_lib PUBLIC cxx_std_17)

add_library(RGLC::common ALIAS common_lib)
