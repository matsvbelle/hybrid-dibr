#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out float depth;

uniform mat4 camMatrix;
uniform vec2 focal;
uniform vec2 pp;
uniform vec2 res;
uniform float z_far;
uniform float z_near;

void main()
{
    vec4 cameraPosition = inverse(camMatrix) * vec4(-aPos.x, -aPos.y, aPos.z, 1.0);
	cameraPosition /= cameraPosition.w;
    depth = (cameraPosition.z - z_near) / (z_far - z_near);

    float img_x = focal.x * (cameraPosition.x / cameraPosition.z) + pp.x - 0.5;
    float img_y = focal.y * (cameraPosition.y / cameraPosition.z) + pp.y - 0.5;

    float u = img_x / res.x;
    float v = img_y / res.y;

    gl_Position = vec4(u*2.0-1.0, v*2.0-1.0, depth, 1.0);
}