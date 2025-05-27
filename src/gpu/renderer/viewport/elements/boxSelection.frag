#version 450

layout( push_constant ) uniform constants
{
	layout(offset = 80) uint state;
} push;

layout(location = 0) out vec4 outColor;

const float OPACITY = 0.3f;

const vec3 NORMAL_COLOR = vec3(0.0f, 0.0f, 1.0f);
const vec3 INVERTED_COLOR = vec3(1.0f, 0.0f, 0.0f);
const vec3 SPECIAL_COLOR = vec3(1.0f, 0.0f, 1.0f);

void main() {
	outColor = vec4(0.0f, 0.0f, 0.0f, 0.3f);
	outColor.xyz += float(push.state == 0) * NORMAL_COLOR;
	outColor.xyz += float(push.state == 1) * INVERTED_COLOR;
	outColor.xyz += float(push.state == 2) * SPECIAL_COLOR;
}
