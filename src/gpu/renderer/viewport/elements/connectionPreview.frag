#version 450
#extension GL_GOOGLE_include_directive : require

#include "../sharedLogic/wireConstants.glsl"

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(WIRE_LOW_COLOR, WIRE_OPACITY);
}
