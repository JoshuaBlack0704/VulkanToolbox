echo InputDir
echo %~1
echo OutputDir
echo %~2
md %~f2
md %~f1\compiledShaders

glslc.exe %~1\shader.frag -o %~2\frag.spv
glslc.exe %~1\shader.frag -o %~1\compiledShaders\frag.spv

glslc.exe %~1\shader.vert -o %~2\vert.spv
glslc.exe %~1\shader.vert -o %~1\compiledShaders\vert.spv

glslc.exe %~1\RandomGen.comp -o %~2\randomgen.spv
glslc.exe %~1\RandomGen.comp -o %~1\compiledShaders\randomgen.spv

glslc.exe %~1\StateUpdate.comp -o %~2\stateupdate.spv
glslc.exe %~1\StateUpdate.comp -o %~1\compiledShaders\stateupdate.spv

glslc.exe %~1\SimpleTriangle.vert -o %~2\simpletrianglevert.spv
glslc.exe %~1\SimpleTriangle.vert -o %~1\compiledShaders\simpletrianglevert.spv

glslc.exe %~1\SimpleTriangle.frag -o %~2\simpletrianglefrag.spv
glslc.exe %~1\SimpleTriangle.frag -o %~1\compiledShaders\simpletrianglefrag.spv

glslc.exe %~1\DepthRender.vert -o %~2\depthrendervert.spv
glslc.exe %~1\DepthRender.vert -o %~1\compiledShaders\depthrendervert.spv

glslc.exe %~1\DepthRender.frag -o %~2\depthrenderfrag.spv
glslc.exe %~1\DepthRender.frag -o %~1\compiledShaders\depthrenderfrag.spv

pause