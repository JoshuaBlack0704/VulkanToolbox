md shaders

glslc.exe shader.frag -o shaders\frag.spv

glslc.exe shader.vert -o shaders\vert.spv

glslc.exe RandomGen.comp -o shaders\randomgen.spv

glslc.exe StateUpdate.comp -o shaders\stateupdate.spv

glslc.exe SimpleTriangle.vert -o shaders\simpletrianglevert.spv

glslc.exe SimpleTriangle.frag -o shaders\simpletrianglefrag.spv

glslc.exe DepthRender.vert -o shaders\depthrendervert.spv

glslc.exe DepthRender.frag -o shaders\depthrenderfrag.spv

pause