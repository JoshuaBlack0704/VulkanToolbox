#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 0) out vec4 outColor;



layout (push_constant) uniform data {
    vec3 lightColor;
	vec3 lightDirection;
	vec3 ambientColor;
	float directionIntensity;
	float ambientIntensity;
} lightData;

void main() {
    float diff = clamp(dot(inNormal, -normalize(lightData.lightDirection)), 0.0f, 1.0f);
    vec3 diffuse = lightData.lightColor * diff * lightData.directionIntensity;

    vec3 res = clamp(((lightData.ambientIntensity*lightData.ambientColor) + diffuse), 0.0f, 1.0f) * inColor;

    outColor = vec4(res, 1.0);
}