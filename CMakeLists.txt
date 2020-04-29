cmake_minimum_required(VERSION 3.5.1)
project(TinyBehaviorTree)

#---- Add the subdirectory cmake ----
set(CMAKE_CONFIG_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

#---- Enable C++14 ----
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#---- project configuration ----
option(BUILD_EXAMPLES   "Build examples" ON)

set( BEHAVIOR_TREE_INC_DESTINATION   include )

add_library(TinyBehaviorTree INTERFACE)

target_include_directories(TinyBehaviorTree INTERFACE include/)

######################################################
# INSTALL

# INSTALL(TARGETS TinyBehaviorTree
#     DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
#     EXPORT TinyBehaviorTreeConfig
#     INCLUDES       DESTINATION ${BEHAVIOR_TREE_INC_DESTINATION}
#     FILES_MATCHING PATTERN "*.h*")

# install(EXPORT TinyBehaviorTreeConfig
#     FILE TinyBehaviorTree-config.cmake
#     DESTINATION "${include}/TinyBehaviorTree/cmake"
#     NAMESPACE TBT::)

# export(PACKAGE ${PROJECT_NAME})

######################################################
# EXAMPLES

if( BUILD_EXAMPLES )
    add_subdirectory(examples)
endif()
