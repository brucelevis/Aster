#pragma once

namespace RHI::Vulkan
{
  enum class ImageType
  {
    Present,
    DepthOnlyAttachment,
    StencilOnlyAttachment,
    DepthStencilAttachment,
    SubpassInput,
    OutputColorAttachment
  };
}
