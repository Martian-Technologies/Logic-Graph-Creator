#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 pointA;
layout(location = 1) in vec2 pointB;
layout(location = 2) in uint inStateIndex;

layout(location = 0) out vec3 outColor;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

#include "stateBuffer.glsl"

vec2 positions[6] = vec2[](
    vec2(0.0, 0.5),
    vec2(1.0, 0.5),
    vec2(1.0, -0.5),
	vec2(0.0, 0.5),
    vec2(1.0, -0.5),
    vec2(0.0, -0.5)
);

const float LINE_WIDTH = 0.07f;

void main() {
	// extract state from states array
	uint val = states[inStateIndex / statesPerWord];
	uint state = (val >> ((inStateIndex % statesPerWord) * 8)) & 0xFFu;

	// get line vertex
	vec2 xBasis = pointB - pointA;
	vec2 yBasis = normalize(vec2(-xBasis.y, xBasis.x));
	vec2 sampledPosition = positions[gl_VertexIndex % 6];
	vec2 position = pointA + xBasis * sampledPosition.x + yBasis * LINE_WIDTH * sampledPosition.y;
	
	gl_Position = push.mvp * vec4(position, 0.0, 1.0);

	outColor = vec3(0.0f);
	outColor += vec3(151.0f/255.0f, 169.0f/255.0f, 225.0f/255.0f) * float(state == 0); // normal
	outColor += vec3(143.0f/255.0f, 233.0f/255.0f, 127.0f/255.0f) * float(state == 1); // powered on
	outColor += vec3(176.0f/255.0f, 164.0f/255.0f, 255.0f/255.0f) * float(state == 2); // z (floating)
	outColor += vec3(255.0f/255.0f, 167.0f/255.0f, 164.0f/255.0f) * float(state == 4); // x
}
