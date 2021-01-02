#pragma once

enum class ImageType
{
  Present,
  DepthOnlyAttachment,
  StencilOnlyAttachment,
  DepthStencilAttachment,
  InputAttachment,
  OutputColorAttachment
};
