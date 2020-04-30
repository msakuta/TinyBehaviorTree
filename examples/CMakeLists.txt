cmake_minimum_required(VERSION 3.5.1)

include_directories(../include)

set(CMAKE_DEBUG_POSTFIX "")

add_executable(basic_tree  basic_tree.cpp )
target_compile_definitions(basic_tree PRIVATE "MANUAL_STATIC_LINKING")

add_executable(fallback_node  fallback_node.cpp )
target_compile_definitions(fallback_node PRIVATE "MANUAL_STATIC_LINKING")
