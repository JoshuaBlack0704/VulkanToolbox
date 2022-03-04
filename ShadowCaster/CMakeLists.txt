file(GLOB src "Source/*.cpp")
file(GLOB head "Headers/*.hpp")
file(GLOB shader "Shaders/*.vert" "Shaders/*.frag" "Shaders/*.comp")
file(GLOB shaderBuild "Shaders/*.bat")

add_executable(ShadowCaster ${src} ${head} ${shader} ${shaderBuild})
set_target_properties(ShadowCaster PROPERTIES FOLDER Tests) 
source_group("Shaders" FILES ${shader} ${shaderBuild})

if(WIN32)
    foreach(file ${shader})
        add_custom_command(TARGET ShadowCaster POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShadersV2.bat" ${file} "${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders")
        add_custom_command(TARGET ShadowCaster POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShadersV2.bat" ${file} "${CMAKE_CURRENT_BINARY_DIR}/shaders")
        add_custom_command(TARGET ShadowCaster POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShadersV2.bat" ${file} "${CMAKE_CURRENT_BINARY_DIR}/Release/shaders")
    endforeach()
target_compile_options(ShadowCaster PRIVATE /MP)
endif()

target_link_libraries(ShadowCaster PRIVATE Tools glm vk-bootstrap)