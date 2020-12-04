#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 vertex_color;
layout(location = 1) out float dt;

layout(set=0, binding=0) uniform Resource {
   vec4 color_resource;
   float dt_resource;
};

void main()
{
  vertex_color = color_resource;
  dt = dt_resource;
  gl_Position = vec4(position, 0.0, 1.0);
}