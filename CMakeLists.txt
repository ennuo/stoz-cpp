cmake_minimum_required(VERSION 3.14)
project(stoz)
set(CMAKE_CXX_STANDARD 20)

add_executable(App)

target_include_directories(App PRIVATE 
    include/
)

target_sources(App PRIVATE
    src/main.cpp
    src/stoz.cpp
)

target_link_libraries(App PRIVATE)