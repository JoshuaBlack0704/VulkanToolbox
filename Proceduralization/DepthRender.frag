#version 460

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D TextureSampler;

layout (push_constant) uniform data {
    vec3 lightColor;
	vec3 lightDirection;
	vec3 ambientColor;
	float directionIntensity;
	float ambientIntensity;
} lightData;

void main() {
	vec4 val = texture(TextureSampler, ivec2(texCoord));;
    outColor = vec4(val.x, val.x, val.x, val.w);
}