echo InputShader
echo %~1
echo OutputDir
echo %~2
md %~f2
md %~p0\compiledShaders

echo Making output shader
glslc.exe %~1 -o %~2\%~n1.spv
echo Making storage shader
glslc.exe %~1 -o %~p0\compiledShaders\%~n1.spv

pause