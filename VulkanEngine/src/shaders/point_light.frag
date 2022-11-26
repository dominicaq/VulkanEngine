#version 450

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push{
	vec4 position;
	vec4 color;
	float radius;
} push;

const float M_PI = 3.1415926538;

void main(){
	float distance = sqrt(dot(fragOffset, fragOffset));
	if(distance >= 1.0f){
		discard;
	}

	float cosDis = 0.5f * (cos(distance * M_PI) + 1.0f);
	outColor = vec4(push.color.xyz + 0.5 * cosDis, cosDis);
}
