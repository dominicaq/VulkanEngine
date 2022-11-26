#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outFragColor;
layout(location = 1) out vec3 outFragPositionWorld;
layout(location = 2) out vec3 outFragNormalWorld;
layout(location = 3) out vec2 outFragTexCoord;

// Shadow
layout(location = 4) out vec3 outViewVec;
layout(location = 5) out vec3 outLightVec;
layout(location = 6) out vec4 outShadowCoord;

struct PointLight{
	vec4 position;
	vec4 color;
};

struct DirectionalLight {
	vec4 position;
	mat4 viewProjection;
};

layout(set = 0, binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 modelNormal;
} ubo;

layout(set = 0, binding = 1) uniform UniformBufferScene{
	mat4 inverseView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	DirectionalLight directionalLight;
	int numLights;
} ubs;

layout(push_constant) uniform Push {
	mat4 model;
	mat4 modelNormal;
	int texId;
} push;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
	// Object
	vec4 worldSpace = push.model * vec4(inPosition, 1.0);
	outFragNormalWorld = normalize(mat3(push.modelNormal) * inNormal);
	outFragPositionWorld = worldSpace.xyz;
	outFragColor = inColor;
	outFragTexCoord = outFragTexCoord;
	gl_Position = ubo.projection * ubo.view * worldSpace;

	// Shadow
	outLightVec = normalize(ubs.directionalLight.position.xyz - inPosition);
	outViewVec = -worldSpace.xyz;
	outShadowCoord = (biasMat * ubs.directionalLight.viewProjection * push.model) * vec4(inPosition, 1.0f);
}