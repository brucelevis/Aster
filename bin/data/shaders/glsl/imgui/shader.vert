// https://github.com/ocornut/imgui/blob/master/backends/vulkan/glsl_shader.vert

#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(set = 0, binding = 0) uniform Constants{
  vec2 uScale;
  vec2 uTranslate;
};

layout(location = 0) out struct {
    vec4 Color;
    vec2 UV;
} Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * uScale + uTranslate, 0, 1);
}
