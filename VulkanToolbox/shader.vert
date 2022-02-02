  #version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) buffer MVPData{
mat4 MVPs[];
}mvpData;

layout(set = 0, binding = 1) buffer ColorData{
vec4 ColorArray[];
}colors;

layout ( push_constant ) uniform data {
    mat4 PVmatrix;
    float objectCount;
	float deltaTime;
} camData;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

void main()
{
	//const array of positions for the triangle
	const vec3 positions[3] = vec3[3](
		vec3(1.f,1.f, 0.0f),
		vec3(-1.f,1.f, 0.0f),
		vec3(0.f,-1.f, 0.0f)
	);

	mat4 mat;
    mat[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    mat[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
    mat[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    mat[3] = vec4(0.0f, 0.0f, 1.0f, 1.0f);

	//output the position of each vertex
	gl_Position = mvpData.MVPs[gl_InstanceIndex] * vec4(pos, 1.0f);
	outColor = colors.ColorArray[gl_InstanceIndex].xyz;
	outNormal = normalize(mvpData.MVPs[gl_InstanceIndex]*vec4(normal, 0)).xyz;
}
