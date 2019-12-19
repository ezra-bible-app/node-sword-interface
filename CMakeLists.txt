cmake_minimum_required(VERSION 3.10)

project(node_sword_cli)

set(CMAKE_CXX_FLAGS "-g -rdynamic -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/sword/include)

# Include N-API wrappers
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api/external-napi/")

link_directories(${CMAKE_SOURCE_DIR}/sword_build)

set(SOURCES
${CMAKE_SOURCE_DIR}/src/main.cpp
${CMAKE_SOURCE_DIR}/src/strongs_entry.cpp
${CMAKE_SOURCE_DIR}/src/sword_facade.cpp
${CMAKE_SOURCE_DIR}/src/string_helper.cpp
${CMAKE_SOURCE_DIR}/src/file_system_helper.cpp
)

# ICU Components Reference:
# https://cmake.org/cmake/help/latest/module/FindICU.html
# ICU components = data, i18n, io, le, lx, test, tu and uc.

find_package(ICU COMPONENTS uc i18n data io REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZIP2 REQUIRED)
find_package(CURL REQUIRED)

add_executable(node_sword_cli ${SOURCES})
target_link_libraries(node_sword_cli libsword.a ICU::io ICU::uc ICU::data ICU::i18n ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} CURL::libcurl pthread)
