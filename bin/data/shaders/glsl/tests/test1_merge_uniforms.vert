#version 450

layout(set=0, binding=0) uniform ubo_uniform {
   vec4 v1;
   float v2;
};

layout(set=0, binding=1) uniform single_ubo_uniform {
   vec4 v3;
};

void main()
{
  gl_Position = vec4(1.0f, 1.0f, 0.0, 1.0);
}