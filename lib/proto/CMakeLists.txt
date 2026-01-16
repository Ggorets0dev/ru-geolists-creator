cmake_minimum_required(VERSION 3.15)
project(proto_library CXX)

find_package(protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED CONFIG)
find_package(absl REQUIRED CONFIG)

# Utilities
get_target_property(PROTOC_EXE protobuf::protoc IMPORTED_LOCATION)
get_target_property(GRPC_CPP_PLUGIN_EXE gRPC::grpc_cpp_plugin IMPORTED_LOCATION)

set(PROTO_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/rglc_service.proto")

set(ALL_GENERATED_SRC "")
set(ALL_GENERATED_INC "")

foreach(PROTO_FILE ${PROTO_FILES})
	get_filename_component(PROTO_NAME_WE ${PROTO_FILE} NAME_WE)

	# Full paths to files in binary dir
	set(GEN_PB_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME_WE}.pb.cc")
	set(GEN_PB_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME_WE}.pb.h")
	set(GEN_GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME_WE}.grpc.pb.cc")
	set(GEN_GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME_WE}.grpc.pb.h")

	list(APPEND ALL_GENERATED_SRC ${GEN_PB_SRC} ${GEN_GRPC_SRC})
	list(APPEND ALL_GENERATED_INC ${GEN_PB_HDR} ${GEN_GRPC_HDR})

	add_custom_command(
			OUTPUT ${GEN_PB_SRC} ${GEN_PB_HDR} ${GEN_GRPC_SRC} ${GEN_GRPC_HDR}
			# Step 1: Create dir if it doesnt exist
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
			# Step 2: Launch generation
			COMMAND ${PROTOC_EXE}
			ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
			--grpc_out=${CMAKE_CURRENT_BINARY_DIR}
			--plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXE}
			-I ${CMAKE_CURRENT_SOURCE_DIR}
			${PROTO_FILE}
			DEPENDS ${PROTO_FILE} ${PROTOC_EXE} ${GRPC_CPP_PLUGIN_EXE}
			COMMENT "Generating gRPC/Proto: ${PROTO_NAME_WE}"
			VERBATIM
	)
endforeach()

message(STATUS "[CMAKE] [!] Protoc path: " ${PROTOC_EXE})
message(STATUS "[CMAKE] [!] Protoc (gRPC) path: " ${GRPC_CPP_PLUGIN_EXE})
message(STATUS "[CMAKE] [!] Generated gRPC/protobuf sources: ${ALL_GENERATED_SRC}")
message(STATUS "[CMAKE] [!] Generated gRPC/protobuf headers: ${ALL_GENERATED_INC}")

# Collection source files
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE INC_FILES "inc/*.h" "inc/*.hpp")

add_library(proto STATIC
		${ALL_GENERATED_SRC}
		${ALL_GENERATED_INC}
		${SRC_FILES}
		${INC_FILES}
)

# Важно: включаем BINARY_DIR, чтобы сгенерированные хедеры были видны
target_include_directories(proto PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

target_link_libraries(proto PUBLIC
		grpc::grpc
		protobuf::libprotobuf
		RGLC::log
		absl::base
		absl::strings
)

target_compile_features(proto PUBLIC cxx_std_17)
set_target_properties(proto PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(RGLC::proto ALIAS proto)