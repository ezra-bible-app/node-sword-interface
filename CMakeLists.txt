cmake_minimum_required(VERSION 3.10)

project(node_sword_cli)

set(CMAKE_CXX_FLAGS "-g -rdynamic -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/src/sword_backend)

include_directories(${CMAKE_SOURCE_DIR}/sword/include)

# Include N-API wrappers
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api/external-napi/")

link_directories(${CMAKE_SOURCE_DIR}/sword_build)

# ICU Components Reference:
# https://cmake.org/cmake/help/latest/module/FindICU.html
# ICU components = data, i18n, io, le, lx, test, tu and uc.

find_package(ICU COMPONENTS uc i18n data REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)

set(SWORD_LIBRARY_NAME "libsword.a")
set(DEPENDENT_LIBS ICU::uc ICU::data ICU::i18n ${ZLIB_LIBRARIES} ${CURL_LIBRARIES})

if (UNIX)
    set(DEPENDENT_LIBS ${DEPENDENT_LIBS} pthread)
endif (UNIX)

if (WIN32)
    set(SWORD_LIBRARY_NAME "sword.dll")
    include_directories(${CMAKE_SOURCE_DIR}/build/sword-build-win32/include)
    link_directories(${CMAKE_SOURCE_DIR}/build/sword-build-win32/lib)
endif (WIN32)

set(SOURCES
${CMAKE_SOURCE_DIR}/src/node_sword_cli.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/strongs_entry.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/repository_interface.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/sword_status_reporter.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/module_helper.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/string_helper.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/file_system_helper.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/module_installer.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/text_processor.cpp
${CMAKE_SOURCE_DIR}/src/sword_backend/module_search.cpp
)

add_executable(node_sword_cli ${SOURCES})
target_link_libraries(node_sword_cli ${SWORD_LIBRARY_NAME} ${DEPENDENT_LIBS} )
