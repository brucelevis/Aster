#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec3 normal;

layout(location = 0) out vec3 normal_out;
layout(location = 1) out vec2 uv_out;

layout(set=0, binding=0) uniform PerStaticMeshResource {
   mat4 MVP;
};

void main()
{
  normal_out = normal;
  uv_out = uv;

  gl_Position = MVP * vec4(position, 1.0);
}