cmake_minimum_required(VERSION 3.21)

project(VulkanToolboxSuite VERSION 1.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(VULKANTOOLBOX_RANDOMPATH "Build the RandomPath target" ON)
option(VULKANTOOLBOX_SHADOWCASTER "Build the ShadowCaster target" ON)

add_subdirectory(GlobalExternalLibraries)
add_subdirectory(GlobalInternalLibraries)
add_subdirectory(VulkanToolbox)

if(VULKANTOOLBOX_RANDOMPATH)
add_subdirectory(RandomPath)
endif()
if(VULKANTOOLBOX_SHADOWCASTER)
add_subdirectory(ShadowCaster)
endif()


