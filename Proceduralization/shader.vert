  #version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

void main()
{
	//output the position of each vertex
	gl_Position = vec4(pos, 1.0);
}
