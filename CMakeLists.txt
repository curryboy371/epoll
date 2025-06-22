cmake_minimum_required(VERSION 3.16)
project(EpollChatServer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

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
    ${CMAKE_SOURCE_DIR}/worker
    ${CMAKE_SOURCE_DIR}/system
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

# proto 파일 처리
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

set(PROTO_GEN_C)
set(PROTO_GEN_H)
foreach(proto ${PROTO_FILES})
    get_filename_component(proto_name ${proto} NAME_WE)

    add_custom_command(
        OUTPUT ${PROTO_GEN_DIR}/${proto_name}.pb-c.c ${PROTO_GEN_DIR}/${proto_name}.pb-c.h
        COMMAND protoc --proto_path=${PROTO_DIR} --c_out=${PROTO_GEN_DIR} ${proto}
        DEPENDS ${proto}
    )

    list(APPEND PROTO_GEN_C ${PROTO_GEN_DIR}/${proto_name}.pb-c.c)
    list(APPEND PROTO_GEN_H ${PROTO_GEN_DIR}/${proto_name}.pb-c.h)
endforeach()

# proto 라이브러리 target 생성
add_library(proto_files STATIC ${PROTO_GEN_C})
add_custom_target(proto_gen DEPENDS ${PROTO_GEN_C})

# 전체 C 소스
file(GLOB_RECURSE SOURCES
    main.c
    core/*.c
    core/task/*.c
    contents/*.c
    packet/*.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

# 빌드 순서 보장
add_dependencies(${PROJECT_NAME} proto_gen)

target_link_libraries(${PROJECT_NAME}
    proto_files
    ${PROTOBUF_C_LIBRARIES}
    ${MONGOC_LIBRARIES}
    ${BSON_LIBRARIES}
    pthread
)
