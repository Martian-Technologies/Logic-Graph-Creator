#version 450

vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
);

layout( push_constant ) uniform constants
{
	mat4 iMvp;
	float gridFade;
} push;

layout(location = 0) out vec2 screenCord;
layout(location = 1) out vec2 worldCord;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	screenCord = gl_Position.xy;
	worldCord = (push.iMvp * gl_Position).xy;
}
