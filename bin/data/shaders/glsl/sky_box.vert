#version 450

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 texPosition;

layout(set = 0, binding = 0) uniform PerFrame
{
  mat4 Projection;
  mat4 View;
  mat4 Model; //should be only scaled
};

void main()
{
  mat4 view = mat4(mat3(View)); //remove transform

  texPosition = position;
  texPosition.y *= -1.0f; //map to vulkan space

  gl_Position = Projection * view * Model * vec4(position, 1.0f);
}