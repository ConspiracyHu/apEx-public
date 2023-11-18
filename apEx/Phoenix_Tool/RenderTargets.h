#pragma once
#include "../../Bedrock/CoRE2/Core2.h"
#include "phxResource.h"
#include "../Phoenix/RenderTarget.h"

class CphxRenderTarget_Tool : public CphxResource
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  CString Name;
  TBOOL External;

  TBOOL HiddenFromTimeline = false;
  TBOOL cubeMap = false;

  TU8 ResolutionDescriptor;
  TU8 ZResolution = 0;

  CphxRenderTarget::PixelFormat pixelFormat = CphxRenderTarget::PixelFormat::RGBA16Float;
  CphxRenderTarget rt;

  //ID3D11Texture2D *RenderTarget;
  //ID3D11ShaderResourceView *View;
  //ID3D11RenderTargetView *RTView;

  CphxRenderTarget_Tool();
  virtual ~CphxRenderTarget_Tool();

  //void Reallocate(CCoreDevice *dev);
  //void RecreateRTView(CCoreDevice *dev);

  TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType() { return PHX_RENDERTARGET; }

};

TBOOL CreateRenderTarget( TS32 XRes, TS32 YRes, DXGI_FORMAT Format, TS32 FormatSize, TBOOL AutoMipmap, ID3D11Texture2D *&Texture, ID3D11ShaderResourceView *&View, ID3D11RenderTargetView *&RTView );
TU8 CreateResolutionDescription( TBOOL DemoResRelative, TS32 XFactor, TS32 YFactor );