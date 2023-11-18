#define _WINSOCKAPI_
#include "BasePCH.h"
#include "../Phoenix/Timeline.h"
#include "TimelinePreview.h"
#define WINDOWNAME _T("Timeline Preview")
#define WINDOWXML _T("TimelinePreview")
#include "../Phoenix_Tool/Timeline_tool.h"
#include "../Phoenix_Tool/apxProject.h"
#include "TimelineEditor.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include "apExRoot.h"
#include "Config.h"

extern CCoreRasterizerState *RenderRasterState;
extern CCoreSamplerState *RenderSamplerState;
extern CCoreSamplerState* RenderPointSamplerState;
extern CCoreDepthStencilState *RenderDepthState_Normal;
extern CCoreBlendState *DisableBlend;

void CWBDemoPreview::OnDraw( CWBDrawAPI *API )
{
  CWBItem::OnDraw( API );

  int cx = GetClientRect().Width();
  int cy = GetClientRect().Height();

  if ( !resolutionX || !resolutionY )
  {
    RenderTargets->UpdateCollection( cx, cy, true );
    EnableHScrollbar( false, false );
    EnableVScrollbar( false, false );
  }
  else
  {
    EnableHScrollbar( true, true );
    EnableVScrollbar( true, true );

    cx = resolutionX;
    cy = resolutionY;
    RenderTargets->UpdateCollection( resolutionX, resolutionY, true );
  }

  int ox = ( GetClientRect().Width() - cx ) / 2;
  int oy = ( GetClientRect().Height() - cy ) / 2;

  Project.ApplyRenderTargets( RenderTargets );
  API->DrawRect( GetClientRect(), CColor::FromARGB( 0xff000000 ) );

  TS32 tax = Project.Timeline->Timeline->AspectX;
  TS32 tay = Project.Timeline->Timeline->AspectY;

  TS32 max = Config::MonitorAspectX;
  TS32 may = Config::MonitorAspectY;

  TS32 sax = GetSystemMetrics( SM_CXSCREEN );
  TS32 say = GetSystemMetrics( SM_CYSCREEN );

  TF32 aspect = ( tax*max*say ) / (TF32)( tay*may*sax );
  //TF32 aspect=tax/(TF32)tay*(max/(TF32)sax)/(may/(TF32)say);

  int xs = cx;
  int ys = cy;

  if ( aspect > xs / (TF32)ys )
    ys = (TS32)( xs / aspect );
  else xs = (TS32)( ys*aspect );

  int x1 = ( cx - xs ) / 2;
  int y1 = ( cy - ys ) / 2;

  CRect DisplayRect = CRect( x1, y1, x1 + xs, y1 + ys ) + CPoint( ox, oy );

  SetHScrollbarParameters( DisplayRect.x1, DisplayRect.x2, GetClientRect().Width() );
  SetVScrollbarParameters( DisplayRect.y1, DisplayRect.y2, GetClientRect().Height() );

  if ( DisplayRect.Width() <= GetClientRect().Width() )
    SetHScrollbarPos( 0 );

  if ( DisplayRect.Height() <= GetClientRect().Height() )
    SetVScrollbarPos( 0 );

  DisplayRect -= CPoint( GetHScrollbarPos(), GetVScrollbarPos() );

  API->FlushDrawBuffer();
  API->SetRenderView( ClientToScreen( CRect( GetClientRect().TopLeft(), GetClientRect().TopLeft() + CPoint( cx, cy ) ) ) );

  RenderDepthState_Normal->Apply();
  RenderRasterState->Apply();
  RenderSamplerState->Apply( CORESMP_PS0 );
  RenderSamplerState->Apply( CORESMP_VS0 );
  //App->GetDevice()->SetRenderState(RenderDepthState_Normal);
  //App->GetDevice()->SetRenderState(RenderRasterState);
  //App->GetDevice()->SetSamplerState(CORESMP_PS0, RenderSamplerState);
  //App->GetDevice()->SetSamplerState(CORESMP_VS0, RenderSamplerState);

  auto wba = Root->GetWorkBench( Root->GetSelectedWorkBench() );

  if ( wba && wba->GetEditedScene() )
    wba->GetEditedScene()->SwapParticleBuffers();

  TS32 frame = Project.GetFrameToRender();
  Project.Timeline->Render( (TF32)frame );

  if ( wba && wba->GetEditedScene() )
    wba->GetEditedScene()->SwapParticleBuffers();

  API->GetDevice()->ForceStateReset();
  API->SetUIRenderState();

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    CphxEvent_Tool *e = Project.Timeline->Events[ x ];
    if ( e->Event->StartFrame <= frame && e->Event->EndFrame >= frame )
      e->RequestContent();
  }

  ID3D11ShaderResourceView* rt = nullptr;
  if ( Project.Timeline->Timeline->Target )
    rt = Project.Timeline->Timeline->Target->View;

  if ( renderTargetIdx == -2 )
    rt = phxDepthBufferShaderView;

  if ( renderTargetIdx >= 0 && Project.Timeline->Timeline->RenderTargets[ renderTargetIdx ] )
    rt = Project.Timeline->Timeline->RenderTargets[ renderTargetIdx ]->View;

  if ( rt )
  {
    CCoreDX11Texture2D *dummy = new CCoreDX11Texture2D( (CCoreDX11Device*)API->GetDevice() );
    dummy->SetView( rt );

    API->GetResolutionData()->Reset();

    CVector4 Resolution = CVector4( (TF32)App->GetXRes(), (TF32)App->GetYRes(), BadProjector, 1 );
    API->GetResolutionData()->AddData( &Resolution, sizeof( CVector4 ) );
    API->GetResolutionData()->Upload();
    CCoreConstantBuffer *rd = API->GetResolutionData();
    API->GetDevice()->SetShaderConstants( 0, 1, &rd );

    API->FlushDrawBuffer();
    API->GetDevice()->SetRenderState( DisableBlend );
    API->GetDevice()->SetTexture( CORESMP_PS0, dummy );
    API->GetDevice()->SetPixelShader( Root->GammaDisplayShader ); //counteract the degamma in the display

    if ( pointSampling )
    {
      API->GetDevice()->SetSamplerState( CORESMP_PS0, RenderPointSamplerState );
      API->GetDevice()->SetSamplerState( CORESMP_VS0, RenderPointSamplerState );
    }

    API->DrawRect( DisplayRect, 0, 0, 1, 1 );

    API->FlushDrawBuffer();
    API->SetUIRenderState();

    dummy->SetTextureHandle( NULL );
    dummy->SetView( NULL );
    SAFEDELETE( dummy );

    if ( HistogramOpen )
      DrawHistogram( API );
  }
  else
    API->DrawRectBorder( DisplayRect, CColor::FromARGB( 0xff808080 ) );

  API->DrawRectBorder( DisplayRect + CRect( 1, 1, 1, 1 ), CColor::FromARGB( 0x10808080 ) );
}

TBOOL CWBDemoPreview::MessageProc( CWBMessage &Message )
{
  return CWBItem::MessageProc( Message );
}

CWBDemoPreview::CWBDemoPreview( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  RenderTargets = Project.SpawnRenderTargetCollection( App->GetDevice(), Pos.Width(), Pos.Height(), true );

  //	PerformMinMax = App->GetDevice()->CreateComputeShader();
  //
  //	CString MinMaxCode = "RWBuffer<float4> MinMaxDestFloat : register(u0);                                                             \n"
  //"																																	   \n"
  //"	[numthreads(HGRAM_TILES_PER_BLOCK, HGRAM_TILES_PER_BLOCK, 1)]																	   \n"
  //"	void RENDERDOC_TileMinMaxCS(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)												   \n"
  //"	{																																   \n"
  //"		uint texType = SHADER_RESTYPE;																								   \n"
  //"																																	   \n"
  //"		uint3 texDim = uint3(HistogramTextureResolution);																			   \n"
  //"																																	   \n"
  //"		uint blocksX = (int)ceil(float(texDim.x) / float(HGRAM_PIXELS_PER_TILE*HGRAM_TILES_PER_BLOCK));								   \n"
  //"																																	   \n"
  //"		uint2 topleft = (gid.xy*HGRAM_TILES_PER_BLOCK + tid.xy)*HGRAM_PIXELS_PER_TILE;												   \n"
  //"																																	   \n"
  //"		uint outIdx = (tid.y*HGRAM_TILES_PER_BLOCK + tid.x) + (gid.y*blocksX + gid.x)*(HGRAM_TILES_PER_BLOCK*HGRAM_TILES_PER_BLOCK);   \n"
  //"																																	   \n"
  //"		int i = 0;																													   \n"
  //"																																	   \n"
  //"	{																																   \n"
  //"		float4 minval = 0;																											   \n"
  //"		float4 maxval = 0;																											   \n"
  //"																																	   \n"
  //"		for (uint y = topleft.y; y < min(texDim.y, topleft.y + HGRAM_PIXELS_PER_TILE); y++)											   \n"
  //"		{																															   \n"
  //"			for (uint x = topleft.x; x < min(texDim.x, topleft.x + HGRAM_PIXELS_PER_TILE); x++)										   \n"
  //"			{																														   \n"
  //"				float4 data = SampleTextureFloat4(texType, false, float2(x, y) / float2(texDim.xy),									   \n"
  //"												  HistogramSlice, HistogramMip, HistogramSample, texDim);							   \n"
  //"																																	   \n"
  //"				if (i == 0)																											   \n"
  //"				{																													   \n"
  //"					minval = maxval = data;																							   \n"
  //"				}																													   \n"
  //"				else																												   \n"
  //"				{																													   \n"
  //"					minval = min(minval, data);																						   \n"
  //"					maxval = max(maxval, data);																						   \n"
  //"				}																													   \n"
  //"																																	   \n"
  //"				i++;																												   \n"
  //"			}																														   \n"
  //"		}																															   \n"
  //"																																	   \n"
  //"		MinMaxDestFloat[outIdx * 2 + 0] = minval;																					   \n"
  //"		MinMaxDestFloat[outIdx * 2 + 1] = maxval;																					   \n"
  //"		return;																														   \n"
  //"	}																																   \n"
  //"	}";
  //
  //	PerformMinMax->SetCode(MinMaxCode, CString("RENDERDOC_TileMinMaxCS"), CString("cs_5_0"));
  //	CString Err;
  //	if (!PerformMinMax->CompileAndCreate(&Err))
  //	{
  //		LOG_ERR("Failed to compile compute shader: %s", Err.GetPointer());
  //	}

}

CWBDemoPreview::~CWBDemoPreview()
{
  SAFEDELETE( PerformMinMax );
  SAFEDELETE( GetMinMaxResult );
  SAFEDELETE( GetHistogramResult );
  SAFEDELETE( RenderTargets );
}

CWBItem * CWBDemoPreview::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new CWBDemoPreview( Root, Pos );
}

#include <DirectXPackedVector.h>
#include <DirectXMath.h>

void CWBDemoPreview::DrawHistogram( CWBDrawAPI *API )
{
  TS32 count = GetClientRect().Width() / 3;
  if ( count <= 0 ) return;

  if ( !Project.Timeline->Timeline->Target ) return;

  CCoreDX11Device *dev = (CCoreDX11Device*)App->GetDevice();
  CStreamWriterMemory writer;

  CCoreDX11Texture2D *rt = RenderTargets->GetRenderTarget( Project.Timeline->Timeline->Target );
  if ( renderTargetIdx >= 0 )
    rt = RenderTargets->GetRenderTarget( Project.Timeline->Timeline->RenderTargets[ renderTargetIdx ] );
  if ( !rt ) return;

  //rt->ExportToImage(CString("test.png"), true, CORE_PNG);

  if ( SaveDDSTexture( dev->GetDeviceContext(), rt->GetTextureHandle(), writer ) != S_OK )
    LOG_ERR( "[apex] Error capturing framebuffer data" );

  TS32 datalength = writer.GetLength();
  TU8 *data = writer.GetData();

  struct DDSHEAD
  {
    TU32 DDS;
    TS32 dwSize;
    TS32 dwFlags;
    TS32 dwHeight;
    TS32 dwWidth;
    TS32 dwPitchOrLinearSize;
    TS32 dwDepth;
    TS32 dwMipMapCount;
    TS32 dwReserved1[ 11 ];

    TS32 _dwSize;
    TS32 _dwFlags;
    TS32 dwFourCC;
    TS32 dwRGBBitCount;
    TS32 dwRBitMask;
    TS32 dwGBitMask;
    TS32 dwBBitMask;
    TS32 dwABitMask;

    TS32 dwCaps;
    TS32 dwCaps2;
    TS32 dwCaps3;
    TS32 dwCaps4;
    TS32 dwReserved2;
  };

  DDSHEAD head;
  memcpy( &head, data, sizeof( DDSHEAD ) );
  data += head.dwSize + 4;

  float *fdata = new float[ head.dwWidth*head.dwHeight * 4 ];
  DirectX::PackedVector::XMConvertHalfToFloatStream( fdata, 4, ( const DirectX::PackedVector::HALF* )data, 2, head.dwWidth*head.dwHeight * 4 );

  float fmin = 0;
  float fmax = 1;// log2(max(max(fdata[0], fdata[1]), fdata[2]));

  ////degamma
  //for (TS32 x = 0; x < head.dwWidth*head.dwHeight * 4; x++)
  //	fdata[x] = pow(fdata[x], 2.2f);

  for ( TS32 x = 0; x < head.dwWidth*head.dwHeight; x++ )
  {
    float l = max( max( fdata[ x * 4 ], fdata[ x * 4 + 1 ] ), fdata[ x * 4 + 2 ] );
    fmin = min( fmin, l );
    fmax = max( fmax, l );
  }

  float *buckets = new float[ count ];
  memset( buckets, 0, count * 4 );

  for ( TS32 x = 0; x < head.dwWidth*head.dwHeight; x++ )
  {
    float l = max( max( fdata[ x * 4 ], fdata[ x * 4 + 1 ] ), fdata[ x * 4 + 2 ] );
    float t = ( l - fmin ) / ( fmax - fmin )*( count - 1 )*0.9999f;
    TS32 b0 = (TS32)t;
    float w = t - b0;
    buckets[ b0 ] += 1 - w;
    buckets[ b0 + 1 ] += w;
  }

  CRect Target;
  Target.x2 = GetClientRect().x2 - 16;
  Target.y2 = GetClientRect().y2 - 1;
  Target.x1 = Target.x2 - count - 4;
  Target.y1 = Target.y2 - GetClientRect().Height() / 3 - 4;

  API->DrawRect( Target, CColor::FromARGB( 0xC0000000 ) );
  API->DrawRectBorder( Target, CColor::FromARGB( 0x80ffffff ) );

  TF32 maxcnt = 0;

  for ( TS32 x = 0; x < count; x++ )
    maxcnt = max( maxcnt, buckets[ x ] );

  TF32 maxcnt2 = 0;

  for ( TS32 x = 0; x < count; x++ )
    if ( buckets[ x ] < maxcnt )
      maxcnt2 = max( maxcnt2, buckets[ x ] );

  TS32 b0 = (TS32)( ( 0 - fmin ) / ( fmax - fmin )*count*0.9999f );
  TS32 b1 = (TS32)( ( 1 - fmin ) / ( fmax - fmin )*count*0.9999f );

  API->DrawRect( CRect( Target.x1 + b0 + 2, Target.y1 + 2, Target.x1 + b0 + 3, Target.y2 - 2 ), CColor::FromARGB( 0xffffffff ) );
  API->DrawRect( CRect( Target.x1 + b1 + 2, Target.y1 + 2, Target.x1 + b1 + 3, Target.y2 - 2 ), CColor::FromARGB( 0xffffffff ) );

  for ( TS32 x = 0; x < count; x++ )
  {
    TF32 t = min( 1, buckets[ x ] / (TF32)maxcnt2*0.7f );
    //t = sqrt(t);
    TS32 height = (TS32)( t*( Target.Height() - 4 ) );
    API->DrawRect( CRect( Target.x1 + x + 2, Target.y2 - 2 - height, Target.x1 + x + 3, Target.y2 - 2 ), CColor::FromARGB( 0xff808080 ) );
  }

  SAFEDELETE( buckets );
  SAFEDELETE( fdata );

}

void CWBDemoPreview::SetResolution( int x, int y )
{
  resolutionX = x;
  resolutionY = y;
}

void CWBDemoPreview::SetRenderTargetIndex( int idx )
{
  renderTargetIdx = idx;
}

//////////////////////////////////////////////////////////////////////////
// editor window

CapexTimelinePreview::CapexTimelinePreview() : CapexWindow()
{
}

CapexTimelinePreview::CapexTimelinePreview( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexTimelinePreview::~CapexTimelinePreview()
{

}

void CapexTimelinePreview::UpdateData()
{
  CWBItemSelector* rtList = FindChildByID<CWBItemSelector>( _T( "rendertargets" ) );
  if ( !rtList )
    return;
  rtList->Flush();

  rtList->AddItem( "Default", -1 );
  rtList->AddItem( "Depth", -2 );

  for ( int x = 0; x < Project.GetRenderTargetCount(); x++ )
    rtList->AddItem( Project.GetRenderTargetByIndex( x )->Name, x );

  CWBDemoPreview* t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
  if ( t )
  {
    CWBButton* b = (CWBButton*)FindChildByID( _T( "togglepointsampling" ), _T( "button" ) );
    if ( b )
      b->Push( t->pointSampling );
  }
}

TBOOL CapexTimelinePreview::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONUP:
  {
    CWBDemoPreview *t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
    if ( !t ) break;
    //Project.ChangeDemoResolution(t->GetClientRect().Width(),t->GetClientRect().Height(),App->GetDevice());
    t->RenderTargets->UpdateCollection( t->GetClientRect().Width(), t->GetClientRect().Height(), true );
    break;
  }
  case WBM_FOCUSGAINED:
  {
    CWBDemoPreview *t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
    if ( !t ) return true;
    t->RenderTargets->UpdateCollection( t->GetClientRect().Width(), t->GetClientRect().Height(), true );
    //Project.ChangeDemoResolution(t->GetClientRect().Width(), t->GetClientRect().Height(), App->GetDevice());
    return true;
  }
  break;
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) return true;

    if ( b->GetID() == _T( "togglehistogram" ) )
    {
      ToggleHistogram();
      return true;
    }
    if ( b->GetID() == _T( "togglebadprojector" ) )
    {
      ToggleBadProjector();
      return true;
    }
    if ( b->GetID() == _T( "togglepointsampling" ) )
    {
      TogglePointSampling();
      return true;
    }
  }
  break;
  case WBM_ITEMSELECTED:
  {
    CWBItemSelector* list = FindChildByID<CWBItemSelector>( _T( "resolution" ) );
    if ( list && Message.GetTarget() == list->GetGuid() )
    {
      CWBDemoPreview *t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
      if ( !t ) return true;

      auto item = list->GetItem( Message.Data );
      CString s = item->GetText();
      int x, y;
      if ( s.Scan( "%dx%d", &x, &y ) == 2 )
        t->SetResolution( x, y );
      else
        t->SetResolution( 0, 0 );
    }

    list = FindChildByID<CWBItemSelector>( _T( "rendertargets" ) );
    if ( list && Message.GetTarget() == list->GetGuid() )
    {
      CWBDemoPreview* t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
      if ( !t ) return true;

      t->SetRenderTargetIndex( Message.Data );
    }
  }
  break;
  case WBM_CHAR:
    if ( Message.Key == 'H' || Message.Key == 'h' )
    {
      ToggleHistogram();
      return true;
    }
    if ( Message.Key == 'P' || Message.Key == 'p' )
    {
      ToggleBadProjector();
      return true;
    }
    break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTimelinePreview::ToggleHistogram()
{
  CWBDemoPreview *t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
  if ( !t ) return;
  CWBButton *b = (CWBButton*)FindChildByID( _T( "togglehistogram" ), _T( "button" ) );
  if ( !b ) return;
  t->HistogramOpen = !t->HistogramOpen;
  b->Push( t->HistogramOpen );
}

void CapexTimelinePreview::ToggleBadProjector()
{
  CWBDemoPreview *t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
  if ( !t ) return;
  CWBButton *b = (CWBButton*)FindChildByID( _T( "togglebadprojector" ), _T( "button" ) );
  if ( !b ) return;
  t->BadProjector = !t->BadProjector;
  b->Push( t->BadProjector );
}

void CapexTimelinePreview::TogglePointSampling()
{
  CWBDemoPreview* t = (CWBDemoPreview*)FindChildByID( _T( "demo" ), _T( "demopreview" ) );
  if ( !t ) return;
  CWBButton* b = (CWBButton*)FindChildByID( _T( "togglepointsampling" ), _T( "button" ) );
  if ( !b ) return;
  t->pointSampling = !t->pointSampling;
  b->Push( t->pointSampling );
}
