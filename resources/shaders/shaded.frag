#version 330 core

in vec3 worldPos;

out vec4 color;

uniform vec3 baseColor;

void main()
{
    vec3 dx = dFdx(worldPos);
    vec3 dy = dFdy(worldPos);

    vec3 N = normalize(cross(dx, dy));
    vec3 L1 = normalize(vec3(0.0, 50.0, -15.0) - worldPos);
    vec3 L2 = normalize(vec3(15.0, 50.0, 15.0) - worldPos);

    float diffuse1 = max(dot(N, L1), 0.0);
    float diffuse2 = max(dot(N, L2), 0.0);

    color = vec4(baseColor.xyz, 1.0f) * (0.2 + 0.5 * diffuse1 + 0.3 * diffuse2);
}

