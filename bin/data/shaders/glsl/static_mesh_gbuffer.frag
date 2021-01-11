#version 450

layout(location = 0) in vec3 tangent;
layout(location = 1) in vec3 bitangent;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 worldPosition;

layout(location = 0) out vec4 outBaseColor;
layout(location = 1) out vec4 outWorldPosition;
layout(location = 2) out vec4 outWorldNormal;
layout(location = 3) out vec4 outMetallic;
layout(location = 4) out vec4 outRoughness;
layout(location = 5) out vec4 outDepth;

layout(set = 0, binding = 1) uniform sampler2D BaseColorTexture;
layout(set = 0, binding = 2) uniform sampler2D NormalTexture;
layout(set = 0, binding = 3) uniform sampler2D MetallicRoughnessTexture;

void main()
{
  mat3 tbn = mat3(tangent, bitangent, normal);

  vec3 tangentNormal = texture(NormalTexture, uv).rgb * 2.0f -1.0f;
  vec3 worldNormal = tbn * tangentNormal;

  outBaseColor = texture(BaseColorTexture,uv);
  outWorldPosition = vec4(worldPosition, 1.0f);
  outWorldNormal = vec4((worldNormal + 1.0f) / 2.0f, 1.0f);
  outMetallic = texture(MetallicRoughnessTexture,uv);
  outRoughness = texture(MetallicRoughnessTexture,uv);
  outDepth = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}