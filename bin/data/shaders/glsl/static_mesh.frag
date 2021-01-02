#version 450

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D BaseColorTexture;
layout(set = 0, binding = 2) uniform sampler2D NormalTexture;
layout(set = 0, binding = 3) uniform sampler2D MetallicRoughnessTexture;

void main()
{
  outColor = texture(BaseColorTexture, uv);
}