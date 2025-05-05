#version 450

layout(location = 0) flat in uint state;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(151.0f/255.0f, 169.0f/255.0f, 225.0f/255.0f, 0.93);
}
