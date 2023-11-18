#pragma once
#include "phxEngine.h"

class CphxRenderTarget
{

public:

  enum PixelFormat : unsigned char
  {
    RGBA16Float = 0,
    R32Float = 1,
  };

  bool cubeMap;
  int XRes, YRes;
  ID3D11RenderTargetView *RTView;
  ID3D11ShaderResourceView *View;
  ID3D11Texture2D *Texture;

  void SetViewport();

};