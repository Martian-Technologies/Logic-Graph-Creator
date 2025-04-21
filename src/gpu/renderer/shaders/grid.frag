#version 450

layout(location = 0) in vec2 screenCord;
layout(location = 1) in vec2 worldCord;

layout (location = 0) out vec4 outColor;

layout( push_constant ) uniform constants
{
	mat4 iMvp;
	float gridFade;
} push;

// background constants
vec3 bgCol = vec3(0.69f * 1.3478f, 0.69f * 1.3478f, 0.69f * 1.3478f);
float gradientIntensity = 0.1;

// grid constants
vec3 gridCol = vec3(0.89, 0.878, 0.878);
vec3 bigGridCol = vec3(0.8, 0.8, 0.8);
float gridLineWidth = 0.1;

// thank you magic function from article
float grid(vec2 cord, float spacing) {
	vec2 lineAA = fwidth(cord * spacing);
	vec2 lineUV = 1.0 - abs(fract(cord * spacing) * 2.0 - 1.0);
	vec2 line2 = smoothstep(gridLineWidth * spacing + lineAA, gridLineWidth * spacing - lineAA, lineUV);
	return mix(line2.x, 1.0, line2.y);
}

void main() {
	float smallGrid = grid(worldCord, 1.0) * push.gridFade;
	float bigGrid = grid(worldCord, 1.0/10.0) * push.gridFade;
		
	vec3 col = mix(bgCol, gridCol, smallGrid);
	col = mix(col, bigGridCol, bigGrid);
	col *= (1.0 - length(screenCord)*gradientIntensity); // gradient
	
	outColor = vec4(col, 1.0f);
}

