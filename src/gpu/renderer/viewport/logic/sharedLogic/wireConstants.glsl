vec2 positions[6] = vec2[](
    vec2(0.0, 0.5),
    vec2(1.0, 0.5),
    vec2(1.0, -0.5),
	vec2(0.0, 0.5),
    vec2(1.0, -0.5),
    vec2(0.0, -0.5)
);

const float LINE_WIDTH = 0.07f;
const float WIRE_OPACITY = 0.93f;

const vec3 WIRE_LOW_COLOR = vec3(148.0f/255.0f, 165.0f/255.0f, 148.0f/255.0f);
const vec3 WIRE_HIGH_COLOR = vec3(143.0f/255.0f, 233.0f/255.0f, 127.0f/255.0f);
const vec3 WIRE_Z_COLOR = vec3(176.0f/255.0f, 164.0f/255.0f, 255.0f/255.0f);
const vec3 WIRE_X_COLOR = vec3(255.0f/255.0f, 167.0f/255.0f, 164.0f/255.0f);
