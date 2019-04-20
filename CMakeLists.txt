cmake_minimum_required(VERSION 3.10)

project(node_sword_cli)

set(CMAKE_CXX_FLAGS "-g -rdynamic -std=c++11")

include_directories(/usr/include/sword /usr/local/include/sword)

# Include N-API wrappers
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api/external-napi/")

add_executable(node_sword_cli main.cpp sword_facade.cpp file_system_helper.cpp)
target_link_libraries(node_sword_cli sword /usr/lib/x86_64-linux-gnu/libcurl.so.4 pthread)
