#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 0) out vec2 texCoord;

layout (set = 0, binding = 0) buffer MatrixData{
    mat4 matrix[];
} pMaxtrix;

layout (set = 0, binding = 1) buffer ModelMatData{
    mat4 data[];
} pModelMat;

void main()
{
	//output the position of each vertex
	texCoord = (pMaxtrix.matrix[gl_InstanceIndex]*vec4(pos,1.0f)).xy;
	gl_Position = pMaxtrix.matrix[gl_InstanceIndex]*vec4(pos,1.0f);

}