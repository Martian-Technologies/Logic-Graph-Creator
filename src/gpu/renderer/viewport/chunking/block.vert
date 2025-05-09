#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTex;

layout(location = 0) out vec2 tex;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

#include "stateBuffer.glsl"

const int vertsPerBlock = 6;

void main() {
	// extract state from states array
	uint val = states[(gl_VertexIndex / vertsPerBlock) / statesPerWord];
	uint state = (val >> (((gl_VertexIndex / vertsPerBlock) % statesPerWord) * 8)) & 0xFFu;
	// offset uv by state
	tex = inTex + vec2(0.0, state * 0.25);
	
	gl_Position = push.mvp * vec4(inPosition, 0.0, 1.0);
}
