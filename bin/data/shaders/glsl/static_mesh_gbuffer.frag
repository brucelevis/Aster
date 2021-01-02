#version 450

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outBaseColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMetallic;
layout(location = 3) out vec4 outRoughness;
layout(location = 4) out vec4 outDepth;

layout(set = 0, binding = 1) uniform sampler2D BaseColorTexture;
layout(set = 0, binding = 2) uniform sampler2D NormalTexture;
layout(set = 0, binding = 3) uniform sampler2D MetallicRoughnessTexture;

void main()
{
  outBaseColor = texture(BaseColorTexture,uv);
  outNormal = texture(NormalTexture,uv);
  outMetallic = texture(MetallicRoughnessTexture,uv);
  outRoughness = texture(MetallicRoughnessTexture,uv);
  outDepth = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}