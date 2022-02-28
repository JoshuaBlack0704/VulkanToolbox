file(GLOB src "*.cpp")
file(GLOB head "Headers/*.hpp")
add_library(VulkanToolbox src head)
target_include_directories(VulkanToolbox INTERFACE "Headers/")