cmake_minimum_required(VERSION 3.14)

project(stoz)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rdparty/zlib)
add_library(stoz  include/stoz.hpp include/stb_image.h)
target_include_directories(stoz PUBLIC include/)
