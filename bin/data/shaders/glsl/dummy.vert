#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 vertex_color;
layout(location = 1) out float dt;

layout(set=0, binding=0) uniform Resource {
   float dt_resource;
};

void main()
{
  vertex_color = color;
  dt = dt_resource;
  gl_Position = vec4(position, 1.0);
}