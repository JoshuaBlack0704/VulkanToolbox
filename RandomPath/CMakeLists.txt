file(GLOB src "Source/*.cpp")
file(GLOB head "Headers/*.hpp")
file(GLOB shader "Shaders/*.vert" "Shaders/*.frag" "Shaders/*.comp" "Shaders/*.bat")


add_executable(RandomPath ${src} ${head} ${shader})
source_group("Shaders" FILES ${shader})

if(WIN32)
    add_custom_command(TARGET RandomPath POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShaders.bat" "${CMAKE_CURRENT_SOURCE_DIR}/Shaders" "${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders")
    add_custom_command(TARGET RandomPath POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShaders.bat" "${CMAKE_CURRENT_SOURCE_DIR}/Shaders" "${CMAKE_CURRENT_BINARY_DIR}/Release/shaders")
endif()

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()
target_link_libraries(RandomPath PRIVATE spdlog::spdlog Tools glm vk-bootstrap)