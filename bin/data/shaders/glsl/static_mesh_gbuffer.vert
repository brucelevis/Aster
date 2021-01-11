#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec3 normal;

layout(location = 0) out vec3 tangent_out;
layout(location = 1) out vec3 bitangent_out;
layout(location = 2) out vec3 normal_out;
layout(location = 3) out vec2 uv_out;
layout(location = 4) out vec3 worldPosition;

layout(set=0, binding=0) uniform PerStaticMeshResource {
   mat4 Projection;
   mat4 View;
   mat4 Model;
};

void main()
{
  tangent_out = normalize(vec3(Model * vec4(tangent, 0.0f)));
  bitangent_out = normalize(vec3(Model * vec4(bitangent, 0.0f)));
  normal_out = normalize(vec3(Model * vec4(normal, 0.0f)));
  uv_out = uv;
  worldPosition = vec3(Model * vec4(position, 1.0f));

  gl_Position = Projection * View * Model * vec4(position, 1.0);
}
