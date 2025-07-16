#version 450

layout(location = 0) in vec2 screenCord;
layout(location = 1) in vec2 worldCord;

layout (location = 0) out vec4 outColor;

layout( push_constant ) uniform constants
{
	layout(offset = 64) float gridFade;
	float hasCircuitVisibility;
} push;

// background constants
vec3 bgCol = vec3(0.69f * 1.3478f, 0.69f * 1.3478f, 0.69f * 1.3478f);
float gradientIntensity = 0.1;

// grid constants
vec3 gridCol = vec3(0.89, 0.878, 0.878);
vec3 bigGridCol = vec3(0.8, 0.8, 0.8);
float gridLineWidth = 0.13;
float bigGridLineWidth = 0.7;

// thank you magic function from article
float grid(vec2 cord, float spacing, float width) {
	vec2 lineAA = fwidth(cord * spacing);
	vec2 lineUV = 1.0 - abs(fract(cord * spacing) * 2.0 - 1.0);
	vec2 line2 = smoothstep(width * spacing + lineAA, width * spacing - lineAA, lineUV);
	return mix(line2.x, 1.0, line2.y);
}

void main() {
	float smallGrid = grid(worldCord, 1.0, gridLineWidth) * push.gridFade;
	float bigGrid = grid(worldCord, 1.0/10.0, mix(gridLineWidth, bigGridLineWidth, 1 - push.gridFade));
	
	vec3 col = mix(bgCol, gridCol, smallGrid);
	col = mix(col, mix(bgCol, gridCol, bigGrid), 1 - push.gridFade);
	// col = mix(col, mix(bigGridCol, gridCol, 1 - push.gridFade), bigGrid);
	col *= (1.0 - length(screenCord)*gradientIntensity); // gradient
	
	outColor = vec4(col * mix(0.6f, 1.0f, push.hasCircuitVisibility), 1.0f);
}

