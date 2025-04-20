#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTex;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

layout(location = 0) out vec2 tex;

void main() {
	gl_Position = push.mvp * vec4(inPosition, 0.0, 1.0);
	tex = inTex;
}
