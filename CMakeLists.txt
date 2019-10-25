cmake_minimum_required(VERSION 3.10)

project(node_sword_cli)

set(CMAKE_CXX_FLAGS "-g -rdynamic -std=c++11")

include_directories(/usr/include/sword /usr/local/include/sword ${CMAKE_SOURCE_DIR}/sword/include)

# Include N-API wrappers
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")
include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api/external-napi/")

link_directories(${CMAKE_SOURCE_DIR}/sword_build)

add_executable(node_sword_cli main.cpp strongs_entry.cpp sword_facade.cpp string_helper.cpp file_system_helper.cpp)
target_link_libraries(node_sword_cli libsword.a libcurl.so libicuio.so libicuuc.so libicudata.so libicui18n.so libz.so /usr/lib/x86_64-linux-gnu/libcurl.so.4 pthread)
