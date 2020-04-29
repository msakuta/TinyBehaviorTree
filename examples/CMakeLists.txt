cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_DEBUG_POSTFIX "")

# The plugin libdummy_nodes.so can be linked statically or
# loaded dynamically at run-time.
add_executable(basic_tree  basic_tree.cpp )
target_compile_definitions(basic_tree PRIVATE "MANUAL_STATIC_LINKING")

