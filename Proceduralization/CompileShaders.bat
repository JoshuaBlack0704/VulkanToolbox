md shaders
md Builds\Debug\x64\shaders
md Builds\Release\x64\shaders

glslc.exe shader.frag -o shaders\frag.spv
glslc.exe shader.frag -o Builds\Debug\x64\shaders\frag.spv
glslc.exe shader.frag -o Builds\Release\x64\shaders\frag.spv

glslc.exe shader.vert -o shaders\vert.spv
glslc.exe shader.vert -o Builds\Debug\x64\shaders\vert.spv
glslc.exe shader.vert -o Builds\Release\x64\shaders\vert.spv

glslc.exe RandomGen.comp -o shaders\randomgen.spv
glslc.exe RandomGen.comp -o Builds\Debug\x64\shaders\randomgen.spv
glslc.exe RandomGen.comp -o Builds\Release\x64\shaders\randomgen.spv

glslc.exe StateUpdate.comp -o shaders\stateupdate.spv
glslc.exe StateUpdate.comp -o Builds\Debug\x64\shaders\stateupdate.spv
glslc.exe StateUpdate.comp -o Builds\Release\x64\shaders\stateupdate.spv

glslc.exe SimpleTriangle.vert -o shaders\simpletrianglevert.spv
glslc.exe SimpleTriangle.vert -o Builds\Debug\x64\shaders\simpletrianglevert.spv
glslc.exe SimpleTriangle.vert -o Builds\Release\x64\shaders\simpletrianglevert.spv

glslc.exe SimpleTriangle.frag -o shaders\simpletrianglefrag.spv
glslc.exe SimpleTriangle.frag -o Builds\Debug\x64\shaders\simpletrianglefrag.spv
glslc.exe SimpleTriangle.frag -o Builds\Release\x64\shaders\simpletrianglefrag.spv
