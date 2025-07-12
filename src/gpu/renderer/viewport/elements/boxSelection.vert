#version 450

layout( push_constant ) uniform constants
{
	mat4 mvp;
	vec2 position;
	vec2 size;
} push;

void main() {
	// offsets
	uint b = 1 << (gl_VertexIndex % 6);
	vec2 posCoord = vec2((0x1C & b) != 0, (0x0E & b) != 0);
	
	gl_Position = push.mvp * vec4(push.position + (posCoord * push.size), 0.0, 1.0);
}
