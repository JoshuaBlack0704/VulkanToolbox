cmake_minimum_required(VERSION 3.21)

project(VulkanToolboxSuite VERSION 1.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(GlobalExternalLibraries)
add_subdirectory(GlobalInternalLibraries)
add_subdirectory(VulkanToolbox)
add_subdirectory(RandomPath)


