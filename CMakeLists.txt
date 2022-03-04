cmake_minimum_required(VERSION 3.20..3.21)

project(VulkanToolboxSuite VERSION 1.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(VULKANTOOLBOX_BUILD_TESTS "Build the Tests" ON)

add_subdirectory(GlobalExternalLibraries)
add_subdirectory(GlobalInternalLibraries)
add_subdirectory(VulkanToolbox)

if(VULKANTOOLBOX_BUILD_TESTS)
add_subdirectory(RandomPath)
add_subdirectory(ShadowCaster)
add_subdirectory(ShadowCaster2)
endif()


