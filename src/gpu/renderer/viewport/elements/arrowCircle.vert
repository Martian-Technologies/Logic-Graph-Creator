#version 450

layout( push_constant ) uniform constants
{
	mat4 mvp;
	vec2 position;
} push;

layout(location = 0) out vec2 outTex;

void main() {
	uint b = 1 << (gl_VertexIndex % 6);
	vec2 posCoord = vec2((0x1C & b) != 0, (0x0E & b) != 0);

	gl_Position = push.mvp * vec4(push.position + posCoord, 0.0, 1.0);
	outTex = 2.0f * (posCoord + vec2(-0.5, -0.5));
}
