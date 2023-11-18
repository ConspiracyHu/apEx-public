#include "BasePCH.h"
#include "RenderTargets.h"
#include "../Phoenix/RenderLayer.h"
#include "../Phoenix/phxEngine.h"
#include "apxProject.h"

TBOOL CreateRenderTarget( TS32 XRes, TS32 YRes, DXGI_FORMAT Format, TS32 FormatSize, TBOOL AutoMipmap, ID3D11Texture2D *&Texture, ID3D11ShaderResourceView *&View, ID3D11RenderTargetView *&RTView )
{
  if ( View ) View->Release();
  View = NULL;
  if ( RTView ) RTView->Release();
  RTView = NULL;
  if ( Texture ) Texture->Release();
  Texture = NULL;

  D3D11_TEXTURE2D_DESC tex;
  memset( &tex, 0, sizeof( D3D11_TEXTURE2D_DESC ) );
  tex.ArraySize = 1;
  tex.Width = XRes;
  tex.Height = YRes;
  tex.MipLevels = AutoMipmap ? 0 : 1;
  tex.MiscFlags = AutoMipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
  tex.Format = Format;
  tex.SampleDesc.Count = 1;
  tex.SampleDesc.Quality = 0;
  tex.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

  HRESULT result = phxDev->CreateTexture2D( &tex, NULL, &Texture );
  if ( result != S_OK )
  {
    _com_error err( result );
    LOG( LOG_ERROR, _T( "[texpool] Failed to create texture (%s)" ), err.ErrorMessage() );
    return false;
  }
  result = phxDev->CreateShaderResourceView( Texture, NULL, &View );
  if ( result != S_OK )
  {
    _com_error err( result );
    LOG( LOG_ERROR, _T( "[texpool] Failed to create shader resultource view (%s)" ), err.ErrorMessage() );
    return false;
  }

  D3D11_RENDER_TARGET_VIEW_DESC rt;
  rt.Format = Format;
  rt.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
  rt.Texture2D.MipSlice = 0;
  result = phxDev->CreateRenderTargetView( Texture, &rt, &RTView );
  if ( result != S_OK )
  {
    _com_error err( result );
    LOG( LOG_ERROR, _T( "[texpool] Failed to rendertarget view (%s)" ), err.ErrorMessage() );
    return false;
  }
  return true;
}


CphxRenderTarget_Tool::CphxRenderTarget_Tool()
{
  //RenderTarget=NULL;
  rt.View = NULL;
  rt.Texture = NULL;
  rt.RTView = NULL;
  Name = _T( "New Rendertarget" );
  ResolutionDescriptor = CreateResolutionDescription( true, 0, 0 );
  ZResolution = 0;
  rt.XRes = 0;
  rt.YRes = 0;
  External = false;
}

CphxRenderTarget_Tool::~CphxRenderTarget_Tool()
{
  //if (rt.View) rt.View->Release();
  //if (rt.RTView) rt.RTView->Release();
  //if (RenderTarget) RenderTarget->Release();
}

//void CphxRenderTarget_Tool::Reallocate(CCoreDevice *dev)
//{
//	GetRenderTargetResolution(ResolutionDescriptor,rt.XRes,rt.YRes,Project.DemoResolutionX,Project.DemoResolutionY);
//
//	CreateRenderTarget(rt.XRes,rt.YRes,DXGI_FORMAT_R16G16B16A16_FLOAT,8,RenderTarget,rt.View,rt.RTView);
//
//	//Project.ApplyRenderTargets();
//}

void CphxRenderTarget_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ) ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "ResolutionDescriptor" ) ).SetInt( ResolutionDescriptor );
  Node->AddChild( _T( "ZResolution" ) ).SetInt( ZResolution );
  Node->AddChild( _T( "HiddenFromTimeline" ), false ).SetInt( HiddenFromTimeline );
  Node->AddChild( _T( "PixelFormat" ), false ).SetInt( pixelFormat );
  Node->AddChild( _T( "CubeMap" ), false ).SetInt( cubeMap );
}

void CphxRenderTarget_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  TS32 x = ResolutionDescriptor;
  if ( Node->GetChildCount( _T( "ResolutionDescriptor" ) ) ) Node->GetChild( _T( "ResolutionDescriptor" ) ).GetValue( x );
  ResolutionDescriptor = x;
  if ( Node->GetChildCount( _T( "ZResolution" ) ) ) Node->GetChild( _T( "ZResolution" ) ).GetValue( x );
  else x = 0;
  ZResolution = x;
  if ( Node->GetChildCount( _T( "HiddenFromTimeline" ) ) ) Node->GetChild( _T( "HiddenFromTimeline" ) ).GetValue( HiddenFromTimeline );
  if ( Node->GetChildCount( _T( "CubeMap" ) ) ) Node->GetChild( _T( "CubeMap" ) ).GetValue( cubeMap );

  x = 0;
  if ( Node->GetChildCount( _T( "PixelFormat" ) ) ) Node->GetChild( _T( "PixelFormat" ) ).GetValue( x );
  pixelFormat = ( CphxRenderTarget::PixelFormat )x;
}

TBOOL CphxRenderTarget_Tool::GenerateResource( CCoreDevice *Dev )
{
  //Reallocate(Dev);
  return true;
}

TU8 CreateResolutionDescription( TBOOL DemoResRelative, TS32 XFactor, TS32 YFactor )
{
  return ( DemoResRelative * 0x80 ) + ( max( 0, min( 7, XFactor ) ) << 3 ) + ( max( 0, min( 7, YFactor ) ) );
}
