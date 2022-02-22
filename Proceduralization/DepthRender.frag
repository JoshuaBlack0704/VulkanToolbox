#version 460

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D depthSampler;

layout (push_constant) uniform data {
    vec3 lightColor;
	vec3 lightDirection;
	vec3 ambientColor;
	float directionIntensity;
	float ambientIntensity;
} lightData;

void main() {
	vec4 depthVal = texture(depthSampler, texCoord);
    outColor = vec4(depthVal.x, depthVal.x, depthVal.x, 1.0f);
}