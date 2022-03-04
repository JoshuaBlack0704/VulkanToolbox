file(GLOB src "Source/*.cpp" "Shaders/*.bat")
file(GLOB head "Headers/*.hpp")
file(GLOB_RECURSE shaderGroup "*.vert" "*.frag" "*.comp")

get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})

add_executable(${ProjectId} ${src} ${head} ${shaderGroup} ${shaderBuild})
set_target_properties(${ProjectId} PROPERTIES FOLDER Tests) 

foreach(shader ${shaderGroup})
    cmake_path(GET shader EXTENSION shaderType)
    source_group("Shaders/${shaderType}" FILES ${shader})
endforeach()


if(MSVC)
    foreach(file ${shaderGroup})
        add_custom_command(TARGET ${ProjectId} POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShaders.bat" ${file} "${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders")
        add_custom_command(TARGET ${ProjectId} POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShaders.bat" ${file} "${CMAKE_CURRENT_BINARY_DIR}/shaders")
        add_custom_command(TARGET ${ProjectId} POST_BUILD COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/CompileShaders.bat" ${file} "${CMAKE_CURRENT_BINARY_DIR}/Release/shaders")
    endforeach()
target_compile_options(${ProjectId} PRIVATE /MP)
endif()

target_link_libraries(${ProjectId} PRIVATE Tools glm vk-bootstrap)