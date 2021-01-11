#version 450

layout(location = 0) out vec4 outColor;

layout (input_attachment_index=0, set=0, binding=0) uniform subpassInput BaseColorTexture;
layout (input_attachment_index=0, set=0, binding=1) uniform subpassInput WorldPositionTexture;
layout (input_attachment_index=1, set=0, binding=2) uniform subpassInput WorldNormalTexture;
layout (input_attachment_index=2, set=0, binding=3) uniform subpassInput MetallicTexture;
layout (input_attachment_index=3, set=0, binding=4) uniform subpassInput RoughnessTexture;

void main()
{
  outColor = subpassLoad(BaseColorTexture);
}