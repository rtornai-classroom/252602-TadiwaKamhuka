#version 400 core

layout (isolines, equal_spacing, ccw) in;

uniform mat4 matModelView;
uniform mat4 matProjection;

uniform int controlPointsNumber;

/** N choose R */
float NCR(int n, int r) {
    if (r == 0) return 1.0;

    double result = 1.0;
    for (int k = 1; k <= r; ++k) {
        result *= n - k + 1;
        result /= k;
    }
    return float(result);
}

/** Bernstein basis */
float blending(int n, int i, float t) {
    return NCR(n, i) * pow(t, i) * pow(1.0 - t, n - i);
}

/** Full dynamic Bezier curve */
vec3 BezierCurve(float t) {
    vec3 p = vec3(0.0);

    for (int i = 0; i < controlPointsNumber; i++) {
        p += blending(controlPointsNumber - 1, i, t)
           * vec3(gl_in[i].gl_Position);
    }

    return p;
}

void main() {
    float t = gl_TessCoord.x;

    vec3 pos = BezierCurve(t);

    gl_Position = matProjection * matModelView * vec4(pos, 1.0);
}