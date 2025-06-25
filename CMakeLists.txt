cmake_minimum_required(VERSION 3.16)
project(EpollChatServer C)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

# 디버깅 기본값
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)


# Proto 경로
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/_proto)
set(PROTO_GEN_DIR ${CMAKE_SOURCE_DIR}/proto)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/core/task
    ${CMAKE_SOURCE_DIR}/contents
    ${CMAKE_SOURCE_DIR}/packet
    ${PROTO_GEN_DIR}
)

# MongoDB
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF_C REQUIRED libprotobuf-c)
pkg_check_modules(MONGOC REQUIRED libmongoc-1.0)
pkg_check_modules(BSON REQUIRED libbson-1.0)

include_directories(
    ${PROTOBUF_C_INCLUDE_DIRS}
    ${MONGOC_INCLUDE_DIRS}
    ${BSON_INCLUDE_DIRS}
)
link_directories(
    ${PROTOBUF_C_LIBRARY_DIRS}
    ${MONGOC_LIBRARY_DIRS}
    ${BSON_LIBRARY_DIRS}
)

# proto 파일 탐색
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

if(NOT PROTO_FILES)
    message(FATAL_ERROR "No proto files found in ${PROTO_DIR}")
endif()

# proto 개별 처리
set(GENERATED_PROTO_SRCS "")
set(GENERATED_PROTO_HDRS "")

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_filename ${proto_file} NAME_WE)
    set(proto_src "${PROTO_GEN_DIR}/${proto_filename}.pb-c.c")
    set(proto_hdr "${PROTO_GEN_DIR}/${proto_filename}.pb-c.h")

    add_custom_command(
        OUTPUT ${proto_src} ${proto_hdr}
        COMMAND protoc -I=${PROTO_DIR} --c_out=${PROTO_GEN_DIR} ${proto_file}
        DEPENDS ${proto_file}
        COMMENT "Generating ${proto_filename}.pb-c.*"
    )

    list(APPEND GENERATED_PROTO_SRCS ${proto_src})
    list(APPEND GENERATED_PROTO_HDRS ${proto_hdr})
endforeach()

add_custom_target(proto_gen DEPENDS ${GENERATED_PROTO_SRCS} ${GENERATED_PROTO_HDRS})

add_library(proto_files STATIC ${GENERATED_PROTO_SRCS})
add_dependencies(proto_files proto_gen)

# 전체 C 소스
file(GLOB_RECURSE SOURCES
    main.c
    core/*.c
    core/task/*.c
    contents/*.c
    packet/*.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    proto_files
    ${PROTOBUF_C_LIBRARIES}
    ${MONGOC_LIBRARIES}
    ${BSON_LIBRARIES}
    pthread
)

add_dependencies(${PROJECT_NAME} proto_gen)
