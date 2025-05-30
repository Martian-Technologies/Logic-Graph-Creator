#version 450
#extension GL_GOOGLE_include_directive : require
#include "sharedArrow.glsl"

layout( push_constant ) uniform constants
{
	layout(offset = 72) uint depth;
} push;

layout(location = 0) in vec2 inTex;

layout(location = 0) out vec4 outColor;

const float THRESHOLD = 0.4f; 

void main() {
	float dist = length(inTex);
	float smoothing = fwidth(dist);
	float alpha = 1.0f - smoothstep(THRESHOLD-smoothing, THRESHOLD, dist);

	vec4 col = arrowColorOrder[push.depth % numArrowColors];
	outColor = vec4(col.xyz, alpha * col.a);
}
