#version 330

in vec3 vColor;
out vec4 color;

uniform bool swapColors;

void main(){
	vec3 c = vColor;
	if(swapColors) c = vec3(c.g, c.r, c.b);
	color = vec4(c, 1.0);
}