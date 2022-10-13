cmake_minimum_required(VERSION 3.14)

project(stoz)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rdparty/zlib)

add_library(stoz src/stoz.cpp src/stb_image.h include/stoz.hpp)
add_library(stoz::stoz ALIAS stoz)

target_link_libraries(stoz PRIVATE 3rdparty_zlib)

target_include_directories(stoz PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS stoz EXPORT stoz)