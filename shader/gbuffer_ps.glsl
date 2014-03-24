#version 330 core

in vec3 world_pos;
in vec3 world_normal;

layout (location = 0) out  vec4 g_buffer0;

void main()
{
    g_buffer0 = vec4(world_normal, 1.0);
}