#version 450
layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec3 inFragPositionWorld;
layout(location = 2) in vec3 inFragNormalWorld;
layout(location = 3) in vec2 inFragTexCoord;

// Shadow
layout(location = 4) in vec3 inViewVec;
layout(location = 5) in vec3 inLightVec;
layout(location = 6) in vec4 inShadowCoord;

layout (location = 0) out vec4 outFragColor;

struct PointLight{
	vec4 position;
	vec4 color;
};

struct DirectionalLight {
	vec4 position;
	mat4 viewProjection;
};

layout (set = 1, binding = 0) uniform sampler2D shadowMap;

layout(set = 0, binding = 1) uniform UniformBufferScene{
	mat4 inverseView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	DirectionalLight directionalLight;
	int numLights;
} ubs;

#define ambient 0.1
const int enablePCF = 1;

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main() {
	vec3 cameraPositionWorld = ubs.inverseView[3].xyz;
	vec3 viewDir = normalize(cameraPositionWorld - inFragPositionWorld);

	vec3 diffuseLight = ubs.ambientLightColor.xyz * ubs.ambientLightColor.w;
	vec3 surfaceNormal = normalize(inFragNormalWorld);
	vec3 specularLight = vec3(0.0f);

	// Point lights
	for (int i = 0; i < ubs.numLights; i++){
		PointLight light = ubs.pointLights[i];
		vec3 lightDir = light.position.xyz - inFragPositionWorld;
		float attenuation = 1.0 / dot(lightDir, lightDir);
		lightDir = normalize(lightDir);

		// Diffuse
		float cosAng = max(dot(surfaceNormal, lightDir), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;
		
		// Specular
		vec3 halfAngle = normalize(lightDir + viewDir);
		float blinn = dot(surfaceNormal, halfAngle);
		blinn = clamp(blinn, 0, 1);
		blinn = pow(blinn, 512.0f);

		specularLight += light.color.xyz * intensity * blinn;
		diffuseLight += intensity * cosAng;
	}

	// Direcitional Lights
	float shadow = (enablePCF == 1) ? filterPCF(inShadowCoord / inShadowCoord.w) : textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	vec4 lambertian = vec4(shadow * diffuseLight + specularLight, 1.0f);
	vec4 modelTexture = vec4(inFragTexCoord, 0.0, 1.0);
	outFragColor = lambertian * vec4(inFragColor, 1.0f);
}