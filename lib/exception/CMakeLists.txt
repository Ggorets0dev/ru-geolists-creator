add_library(exception_lib STATIC
    src/exception.cpp
)

target_include_directories(exception_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_compile_features(exception_lib PUBLIC cxx_std_17)

add_library(RGLC::exception ALIAS exception_lib)
