#version 450

layout(location=0) out vec4 outColor;

layout(location=0) in vec3 worldPosition;
layout(location=1) in vec3 worldNormal;

layout(set=0, binding=0) uniform Camera {
  mat4 Projection;
  mat4 View;
  vec3 CameraWorldPosition;
};

void main()
{
  vec3 lightDir = worldPosition - CameraWorldPosition;
  float t = clamp(dot(-lightDir, worldNormal), 0, 1.0f);

  vec3 color = vec3(0.7f, 0.7f, 0.7f) * t;

  outColor = vec4(color, 1.0f);
}