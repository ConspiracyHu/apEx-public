#include "RenderTarget.h"


void CphxRenderTarget::SetViewport()
{
  D3D11_VIEWPORT v;
  v.TopLeftX = 0;
  v.TopLeftY = 0;
  v.Width = (float)XRes;
  v.Height = (float)YRes;
  v.MinDepth = 0;
  v.MaxDepth = 1;
  phxContext->RSSetViewports( 1, &v );
}
