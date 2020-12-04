#version 450

layout(location = 0) out vec4 outColors;

layout(location = 0) in vec4 vertex_color;
layout(location = 1) in float dt;

void main()
{
  outColors = vec4(vertex_color.rgb * sin(dt), 1.0);
}