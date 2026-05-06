#version 330

layout (location = 0) in vec3 aPosition;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 FragPos;
out vec3 Normal;

void main(void) {
    vec4 worldPos = matModel * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;

    Normal = normalize(aPosition);

    gl_Position = matProjection * matView * worldPos;
}