#version 450

layout(location = 0) flat in uint inState;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(0.0f);
	outColor += vec4(151.0f/255.0f, 169.0f/255.0f, 225.0f/255.0f, 0.93) * float(inState == 0); // normal
	outColor += vec4(143.0f/255.0f, 233.0f/255.0f, 127.0f/255.0f, 0.93) * float(inState == 1); // powered on
	outColor += vec4(176.0f/255.0f, 164.0f/255.0f, 255.0f/255.0f, 0.93) * float(inState == 2); // floating
	outColor += vec4(255.0f/255.0f, 167.0f/255.0f, 164.0f/255.0f, 0.93) * float(inState == 4); // x
}
