md shaders
md Builds\Debug\x64\shaders
md Builds\Release\x64\shaders


glslc.exe AddBufferValues.comp -o shaders\AddBufferValues.spv
glslc.exe AddBufferValues.comp -o Builds\Debug\x64\shaders\AddBufferValues.spv
glslc.exe AddBufferValues.comp -o Builds\Release\x64\shaders\AddBufferValues.spv

glslc.exe StateUpdate.comp -o shaders\StateUpdate.spv
glslc.exe StateUpdate.comp -o Builds\Debug\x64\shaders\StateUpdate.spv
glslc.exe StateUpdate.comp -o Builds\Release\x64\shaders\StateUpdate.spv

glslc.exe shader.frag -o shaders\frag.spv
glslc.exe shader.frag -o Builds\Debug\x64\shaders\frag.spv
glslc.exe shader.frag -o Builds\Release\x64\shaders\frag.spv

glslc.exe shader.vert -o shaders\vert.spv
glslc.exe shader.vert -o Builds\Debug\x64\shaders\vert.spv
glslc.exe shader.vert -o Builds\Release\x64\shaders\vert.spv

pause
