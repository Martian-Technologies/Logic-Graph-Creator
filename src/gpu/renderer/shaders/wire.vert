#version 450

layout(location = 0) in vec2 inPosition;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

void main() {
	gl_Position = push.mvp * vec4(inPosition, 0.0, 1.0);
}
