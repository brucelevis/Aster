#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

layout(location=0) out vec3 outWorldPosition;
layout(location=1) out vec3 outWorldNormal;

layout(set=0, binding=0) uniform Camera {
  mat4 Projection;
  mat4 View;
  vec3 WorldPosition;
};

void main()
{
  outWorldPosition = position;
  outWorldNormal = normal;
  gl_Position = Projection * View * vec4(position, 1.0f);
}
