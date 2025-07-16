#version 450
#extension GL_GOOGLE_include_directive : require
#include "sharedArrow.glsl"

layout( push_constant ) uniform constants
{
	layout(offset = 80) uint depth;
} push;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = arrowColorOrder[push.depth % numArrowColors];
}
