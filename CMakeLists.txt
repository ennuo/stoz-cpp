cmake_minimum_required(VERSION 3.14)
project(stoz)
set(CMAKE_CXX_STANDARD 20)

add_executable(App)

add_subdirectory(3rdparty/zlib)

target_include_directories(App PRIVATE 
    include/
)

target_sources(App PRIVATE
    src/main.cpp
    src/stoz.cpp
)

target_link_libraries(App PRIVATE 3rdparty_zlib)
