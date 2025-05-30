#version 450

vec2 bodyPositions[6] = vec2[](
    vec2(0.0, 0.5),
    vec2(1.0, 0.5),
    vec2(1.0, -0.5),
	vec2(0.0, 0.5),
    vec2(1.0, -0.5),
    vec2(0.0, -0.5)
);

vec2 headPositions[3] = vec2[](
    vec2(0.0, 0.5),
	vec2(1.0, 0.0),
    vec2(0.0, -0.5)
);

layout( push_constant ) uniform constants
{
	mat4 mvp;
	vec2 pointA;
	vec2 pointB;
} push;

const float GIRTH = 0.2f;
const float HEAD_LENGTH = 0.35f;
const float HEAD_GIRTH = 0.35f;

void main() {
	vec2 xBasis = push.pointB - push.pointA;
	vec2 xBasisNorm = normalize(xBasis);
	vec2 yBasis = normalize(vec2(-xBasis.y, xBasis.x));

	// get body vertex
	vec2 sampledBodyPosition = bodyPositions[gl_VertexIndex % 6];
	vec2 bodyPosition = push.pointA + xBasis * sampledBodyPosition.x + yBasis * GIRTH * sampledBodyPosition.y;

	// get head position
	vec2 sampledHeadPosition = headPositions[gl_VertexIndex % 3];
	vec2 headPosition = push.pointB + xBasisNorm * HEAD_LENGTH * sampledHeadPosition.x + yBasis * HEAD_GIRTH * sampledHeadPosition.y;

	gl_Position = push.mvp * vec4(mix(bodyPosition, headPosition, float(gl_VertexIndex > 5)), 0.0f, 1.0f);
}
