cmake_minimum_required(VERSION 3.10)

project(ezra_sword_cli)

set(CMAKE_CXX_FLAGS "-g -rdynamic -std=c++11")

include_directories(/usr/include/sword /usr/local/include/sword)

add_executable(ezra_sword_cli main.cpp ezra_sword_interface.cpp)
target_link_libraries(ezra_sword_cli sword /usr/lib/x86_64-linux-gnu/libcurl.so.4 pthread)
