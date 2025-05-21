#version 450
#extension GL_GOOGLE_include_directive : require

// per instance
layout(location = 0) in vec2 inPosition;
layout(location = 1) in uvec2 inSize;
layout(location = 2) in float inTexX;
layout(location = 3) in uint inRotation;

layout(location = 0) out vec2 outTex;

layout( push_constant ) uniform constants
{
	mat4 mvp;
	float uvCellSizeX;
	float uvCellSizeY;
} push;

#include "stateBuffer.glsl"

const int vertsPerBlock = 6;

void main() {
	// extract state from states array
	uint val = states[gl_InstanceIndex / statesPerWord];
	uint state = (val >> ((gl_InstanceIndex % statesPerWord) * 8)) & 0xFFu;

	// position offset
	uint b = 1 << (gl_VertexIndex % vertsPerBlock);
    vec2 baseCoord = vec2((0x1C & b) != 0, (0xE & b) != 0);

	outTex = vec2(inTexX + baseCoord.x*float(inSize.x)*push.uvCellSizeX, (push.uvCellSizeY*float(inSize.y))*(baseCoord.y+float(state)));
	gl_Position = push.mvp * vec4(inPosition + baseCoord, 0.0, 1.0);
}
