#include "framecontext.h"
#include "framegraph.h"

const ImageView& FrameContext::GetImageView(const ResourceId& id) const
{
  return renderGraph->GetImageView(id);
}
