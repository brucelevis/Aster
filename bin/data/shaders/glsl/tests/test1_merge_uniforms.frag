#version 450

layout(set=0, binding=0) uniform ubo_uniform {
   vec4 v1;
   float v2;
};

layout(location = 0) out vec4 outColors;

void main()
{
  outColors = vec4(1.0, 1.0, 1.0, 1.0);
}