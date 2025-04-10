#version 450

layout( push_constant ) uniform PushConstants
{
	vec2 translation;
} push;

layout (set = 0, binding = 0) uniform ViewData {
	mat4 pixelView;
} view;

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out vec4 fragColor;

void main() {
	fragTexCoord = inTexCoord;
	fragColor = inColor;
	
    gl_Position = view.pixelView * vec4(inPosition + push.translation, 0, 1);
}
