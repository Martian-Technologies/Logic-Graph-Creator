#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in uint stateIndex;

layout(location = 0) out uint state;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

layout(set = 0, binding = 0) readonly buffer StateBuffer {
   uint states[ ];
};

const int statesPerWord = 4;

void main() {
	// extract state from states array
	uint val = states[stateIndex / statesPerWord];
	state = (val >> ((stateIndex % statesPerWord) * 8)) & 0xFFu;
	
	gl_Position = push.mvp * vec4(inPosition, 0.0, 1.0);
}
