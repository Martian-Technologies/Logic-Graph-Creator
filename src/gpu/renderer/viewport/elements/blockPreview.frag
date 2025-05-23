#version 450

layout( push_constant ) uniform constants
{
	layout(offset = 96) float alpha;
} push;

layout(location = 0) in vec2 tex;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D displayTexture;

void main() {
	outColor = vec4(texture(displayTexture, tex).rgb, push.alpha);
}
