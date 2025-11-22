file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE INC_FILES "inc/*.h" "inc/*.hpp")

add_library(fs_lib STATIC
    ${SRC_FILES}
    ${INC_FILES}
)

target_include_directories(fs_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(fs_lib PUBLIC
    RGLC::log
    RGLC::common
)

target_compile_features(fs_lib PUBLIC cxx_std_17)

add_library(RGLC::fs ALIAS fs_lib)
