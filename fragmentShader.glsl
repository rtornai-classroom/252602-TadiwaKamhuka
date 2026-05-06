#version 330

in vec3 FragPos;
in vec3 Normal;


uniform vec3 lightPos;
uniform vec3 lightColor;
uniform bool lightEnabled;
uniform bool magentaEnabled;
uniform bool useTexture;

uniform sampler2D sunTexture;

out vec3 outColor;

void main() {

    vec3 baseColor = vec3(1.0);

	vec3 normal = normalize(FragPos);
	bool isSide = abs(Normal.y) < 0.7;

	if (magentaEnabled && isSide) {
		baseColor = vec3(1.0, 0.0, 1.0);
	}

	if (useTexture) {
		outColor = texture(sunTexture, Normal.xy * 0.5 + 0.5).rgb;
		return;
	}

	if (!lightEnabled) {
		outColor = baseColor;
		return;
	}

	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 diffuse = diff * lightColor;

	outColor = diffuse * baseColor;
}