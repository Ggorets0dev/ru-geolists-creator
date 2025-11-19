# Try to find protobuf in system
find_package(protobuf REQUIRED CONFIG)

# Try to find abseil in system
find_package(absl REQUIRED)

# ==============================
# BUILDING PROTOBUF SOURCES
# ==============================
set(PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/release_notes.proto")

protobuf_generate_cpp(PROTO_GEN_SRC_FILES PROTO_GEN_INC_FILES
        ${PROTO_FILES}
)

#protobuf_generate_cpp(PROTO_GEN_SRC_FILES PROTO_GEN_INC_FILES ${PROTO_SRC_FILES})

message(STATUS "[CMAKE] [!] Protoc path: " ${Protobuf_PROTOC_EXECUTABLE})
message(STATUS "[CMAKE] [!] Generated protobuf sources: ${PROTO_GEN_SRC_FILES}")
message(STATUS "[CMAKE] [!] Generated protobuf headers: ${PROTO_GEN_INC_FILES}")
# ==============================

add_library(proto STATIC
        ${PROTO_GEN_SRC_FILES}
        ${PROTO_GEN_INC_FILES}
)

target_link_libraries(proto
        PUBLIC
        protobuf::protobuf
        absl::strings
        absl::base
        absl::flat_hash_map
        # добавляй сюда другие компоненты absl, если используешь
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(proto PRIVATE
            absl::log_internal_check_op)
endif()

target_include_directories(proto PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_features(proto PUBLIC cxx_std_17)

set_target_properties(proto PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

add_library(RGLC::proto ALIAS proto)