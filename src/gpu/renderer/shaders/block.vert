#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = push.mvp * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}
