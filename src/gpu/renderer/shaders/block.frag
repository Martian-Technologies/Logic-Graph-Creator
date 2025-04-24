#version 450

layout(location = 0) in vec2 tex;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D displayTexture;

void main() {
	outColor = texture(displayTexture, tex);
}
