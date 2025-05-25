#version 450

layout( push_constant ) uniform constants
{
	layout(offset = 80) bool inverted;
	float alpha;
} push;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(push.inverted ? vec3(1.0f, 0.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f), push.alpha);
}
