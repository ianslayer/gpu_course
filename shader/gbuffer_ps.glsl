#version 330 core

in vec3 world_pos;

out vec4 out_pos;

void main()
{
    out_pos = vec4(world_pos, 1.0);
}