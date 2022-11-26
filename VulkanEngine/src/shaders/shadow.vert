#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

struct PointLight{
	vec4 position;
	vec4 color;
};

struct DirectionalLight {
	vec4 position;
	mat4 viewProjection;
};

layout(set = 0, binding = 1) uniform UniformBufferScene{
	mat4 inverseView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	DirectionalLight directionalLight;
	int numLights;
} ubs;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

out gl_PerVertex { vec4 gl_Position; };

void main() {
	vec4 pos = push.modelMatrix * vec4(inPosition, 1.0);
	gl_Position = ubs.directionalLight.viewProjection * pos;
}