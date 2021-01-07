#version 450

vec2 quadVertices[4] = vec2[4](
  vec2(-1.0f, -1.0f),
  vec2(1.0f, -1.0f),
  vec2(-1.0f, 1.0f),
  vec2(1.0f, 1.0f)
);


void main()
{
  gl_Position = vec4(quadVertices[gl_VertexIndex], 0.0f, 1.0f);
}