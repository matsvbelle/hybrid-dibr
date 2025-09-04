#version 460 core

in float depth;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vec3(depth), 1.0);
}