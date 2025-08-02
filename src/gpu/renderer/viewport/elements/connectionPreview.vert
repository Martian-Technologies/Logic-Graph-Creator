#version 450
#extension GL_GOOGLE_include_directive : require

#include "../logic/sharedLogic/wireConstants.glsl"

layout( push_constant ) uniform constants
{
	mat4 mvp;
	vec2 pointA;
	vec2 pointB;
} push;

void main() {
	// get line vertex
	vec2 xBasis = push.pointB - push.pointA;
	vec2 yBasis = normalize(vec2(-xBasis.y, xBasis.x));
	vec2 sampledPosition = positions[gl_VertexIndex % 6];
	vec2 position = push.pointA + xBasis * sampledPosition.x + yBasis * LINE_WIDTH * sampledPosition.y;
	
	gl_Position = push.mvp * vec4(position, 0.0, 1.0);
}
