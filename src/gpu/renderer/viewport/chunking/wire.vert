#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uint inStateIndex;

layout(location = 0) out uint outState;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

#include "stateBuffer.glsl"

void main() {
	// extract state from states array
	uint val = states[inStateIndex / statesPerWord];
	outState = (val >> ((inStateIndex % statesPerWord) * 8)) & 0xFFu;
	
	gl_Position = push.mvp * vec4(inPosition, 0.0, 1.0);
}
