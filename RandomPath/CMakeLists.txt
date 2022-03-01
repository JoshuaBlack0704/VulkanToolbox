file(GLOB src "Source/*.cpp")
file(GLOB head "Headers/*.hpp")
file(GLOB shader "Shaders/*.vert" "Shaders/*.frag" "Shaders/*.comp" "Shaders/*.bat")

add_executable(RandomPath ${src} ${head} ${shader})
source_group("shaders" FILES ${shader})

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()
target_link_libraries(RandomPath PRIVATE spdlog::spdlog Tools glm vk-bootstrap)