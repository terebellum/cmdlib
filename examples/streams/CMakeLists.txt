cmake_minimum_required(VERSION 3.14)
project(cmdlib-streams LANGUAGES CXX)

include("../../cmake/utils.cmake")
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

if(is_top_level)
    find_package(cmdlib REQUIRED)
endif()

file(COPY inouterr.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(sources main.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

add_executable(cmdlib-streams)
target_sources(cmdlib-streams PRIVATE ${sources})
target_link_libraries(cmdlib-streams PRIVATE cmdlib::cmdlib)

if(NOT is_top_level)
    win_copy_deps_to_target_dir(cmdlib-streams cmdlib::cmdlib)
endif()
