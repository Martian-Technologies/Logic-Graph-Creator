#version 450

layout( push_constant ) uniform constants
{
	vec2 translation;
} push;

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out vec4 fragColor;

void main() {
	fragTexCoord = inTexCoord;
	fragColor = inColor;
	
    gl_Position = vec4(inPosition + push.translation, 0, 1);
}