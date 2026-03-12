#version 330

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec3 aColor;

uniform float offsetX;
uniform float offsetY;

out vec3 vColor;

void main(){
    gl_Position = vec4(aPosition + vec2(offsetX, offsetY), 0.0, 1.0);
    vColor = aColor;
}