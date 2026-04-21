#version 400 core

layout (vertices = 16) out;

uniform int controlPointsNumber;

void main() {
    gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = 1024;

	if (gl_InvocationID < controlPointsNumber) {
		gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	}
}