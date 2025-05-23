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

// magic rotation bitmasks for getting rotation coordinates
// 0x1C, 0x0E, 0x38, 0x70
uint bitmasksX[4] = uint[4](0x1C, 0x0E, 0x23, 0x31);
uint bitmasksY[4] = uint[4](0x0E, 0x1C, 0x31, 0x23);

void main() {
	// extract state from states array
	uint val = states[gl_InstanceIndex / statesPerWord];
	uint state = (val >> ((gl_InstanceIndex % statesPerWord) * 8)) & 0xFFu;

	// position offset
	uint b = 1 << (gl_VertexIndex % vertsPerBlock);
	vec2 posCoord = vec2((bitmasksX[0] & b) != 0, (bitmasksY[0] & b) != 0);
    vec2 uvCoord = vec2((bitmasksX[inRotation] & b) != 0, (bitmasksY[inRotation] & b) != 0);

	outTex = vec2(inTexX + uvCoord.x/**float(inSize.x)*/*push.uvCellSizeX, (push.uvCellSizeY/**float(inSize.y)*/)*(uvCoord.y+float(state)));
	gl_Position = push.mvp * vec4(inPosition + posCoord*inSize, 0.0, 1.0);
}
