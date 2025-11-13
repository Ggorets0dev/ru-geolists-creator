add_library(fs_lib STATIC
    src/fs_utils.cpp
)

target_include_directories(fs_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(fs_lib PUBLIC
    RGLC::log
)

target_compile_features(fs_lib PUBLIC cxx_std_17)

add_library(RGLC::fs ALIAS fs_lib)
