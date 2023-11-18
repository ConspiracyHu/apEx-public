#include "BasePCH.h"
#include "ModelView.h"
#define WINDOWNAME _T("Model View")
#define WINDOWXML _T("ModelView")
#include "WorkBench.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include "../Phoenix/RenderLayer.h"
#include "ModelParameters.h"
#include "Config.h"

#include "apExRoot.h"
extern CapexRoot *Root;

D3DXMATRIX * WINAPI MatrixRotationAxis( D3DXMATRIX *out, const D3DXVECTOR3 *v, FLOAT angle )
{
  D3DXVECTOR3 nv;
  FLOAT sangle, cangle, cdiff;

  //TRACE("out %p, v %p, angle %f\n", out, v, angle);

  D3DXVec3Normalize( &nv, v );
  sangle = sinf( angle );
  cangle = cosf( angle );
  cdiff = 1.0f - cangle;

  out->m[ 0 ][ 0 ] = cdiff * nv.x * nv.x + cangle;
  out->m[ 1 ][ 0 ] = cdiff * nv.x * nv.y - sangle * nv.z;
  out->m[ 2 ][ 0 ] = cdiff * nv.x * nv.z + sangle * nv.y;
  out->m[ 3 ][ 0 ] = 0.0f;
  out->m[ 0 ][ 1 ] = cdiff * nv.y * nv.x + sangle * nv.z;
  out->m[ 1 ][ 1 ] = cdiff * nv.y * nv.y + cangle;
  out->m[ 2 ][ 1 ] = cdiff * nv.y * nv.z - sangle * nv.x;
  out->m[ 3 ][ 1 ] = 0.0f;
  out->m[ 0 ][ 2 ] = cdiff * nv.z * nv.x - sangle * nv.y;
  out->m[ 1 ][ 2 ] = cdiff * nv.z * nv.y + sangle * nv.x;
  out->m[ 2 ][ 2 ] = cdiff * nv.z * nv.z + cangle;
  out->m[ 3 ][ 2 ] = 0.0f;
  out->m[ 0 ][ 3 ] = 0.0f;
  out->m[ 1 ][ 3 ] = 0.0f;
  out->m[ 2 ][ 3 ] = 0.0f;
  out->m[ 3 ][ 3 ] = 1.0f;

  return out;
}

CString CameraModeNames[ 8 ] =
{
  _T( "Left View" ),
  _T( "Right View" ),
  _T( "Top View" ),
  _T( "Bottom View" ),
  _T( "Front View" ),
  _T( "Back View" ),
  _T( "3D View" ),
  _T( "Camera View" ),
};

CCoreVertexBuffer *GridVertexBuffer = NULL;
CCoreIndexBuffer *GridIndexBuffer = NULL;
CCorePixelShader *GridPixelShader = NULL;
CCorePixelShader *ErrPixelShader = NULL;
CCorePixelShader *WirePixelShader = NULL;
//CCorePixelShader *WirePixelShader_Selected = NULL;
//CCorePixelShader *WirePixelShader_Highlighted = NULL;
CCoreVertexShader *GridVertexShader = NULL;
CCoreRasterizerState *AntialiasedLineState = NULL;
CCoreBlendState *DisableBlend = NULL;
CCoreBlendState *AlphaBlendState = NULL;
CCoreDepthStencilState *DepthLessEqual = NULL;
CCoreDepthStencilState *RenderDepthState_Normal = NULL;
CCoreDepthStencilState *RenderDepthState_Wireframe = NULL;
CCoreRasterizerState *RenderRasterState = NULL;
CCoreSamplerState *RenderSamplerState = NULL;
CCoreSamplerState* RenderPointSamplerState = NULL;

extern CphxModelObject_Tool_Mesh *Cube;

void InitializeModelView( CCoreDevice *Device )
{
  if ( GridVertexBuffer ) return;

  PHXVERTEXDATA *Vertices = new PHXVERTEXDATA[ 640 ];

  TF32 offset = 0;
  TS32 vxcnt = 0;

  PHXVERTEXDATA v;
  v.Normal = D3DXVECTOR3( 1, 1, 1 );

  for ( TS32 y = 0; y < 2; y++ )
  {
    for ( TS32 x = 0; x < 80; x++ )
    {
      v.Color = CColor::FromARGB( 0xff606060 );
      if ( offset > 0 )
        v.Color = CColor::FromARGB( 0xff404040 );

      if ( offset == 0 && x == 40 ) v.Color = CColor::FromARGB( 0xffa0a0a0 );

      v.Position = D3DXVECTOR3( -40, 0, x + offset - 40.0f );
      Vertices[ vxcnt++ ] = v;
      v.Position = D3DXVECTOR3( 40, 0, x + offset - 40.0f );
      Vertices[ vxcnt++ ] = v;

      if ( offset == 0 && x == 40 ) v.Color = CColor::FromARGB( 0xffa0a0a0 );

      v.Position = D3DXVECTOR3( x + offset - 40.0f, 0, -40.0f );
      Vertices[ vxcnt++ ] = v;
      v.Position = D3DXVECTOR3( x + offset - 40.0f, 0, 40.0f );
      Vertices[ vxcnt++ ] = v;
    }
    offset += 0.5f;
  }

  GridVertexBuffer = Device->CreateVertexBuffer( (TU8*)Vertices, sizeof( PHXVERTEXDATA ) * 640 );
  GridIndexBuffer = Device->CreateIndexBuffer( 640, 4 );
  TS32 *data = NULL;
  GridIndexBuffer->Lock( (void**)&data );
  for ( TS32 x = 0; x < 640; x++ ) data[ x ] = x;
  GridIndexBuffer->UnLock();

  LPCSTR  shader = _T(
    "cbuffer b : register(b0){float4x4 viewmat;float4x4 projmat;}\n"
    "cbuffer c : register(b1){float4x4 worldmat;float4x4 itworldmat;float4 color;}\n"
    "struct VSIN\n"
    "{\n"
    "	float3 Position : POSITION0;\n"
    "	float3 Position2: POSITION1;\n"
    "	float3 Normal : NORMAL0;\n"
    "	float4 Color : COLOR0;\n"
    "	float4 UV : TEXCOORD0;\n"
    "	float4 UV2: TEXCOORD1;\n"
    "};\n\n"
    "struct VSOUT\n"
    "{\n"
    "	float4 Position : SV_POSITION;\n"
    " float4 Color:TEXCOORD0;\n"
    "};\n"
    "\n"
    "VSOUT v(VSIN x)\n"
    "{\n"
    "	VSOUT k;\n"
    "	k.Position = mul(projmat,mul(viewmat,mul(worldmat,float4(x.Position,1))));\n"
    "	k.Color=x.Color;\n"
    "	return k;\n"
    "}\n"
    "\n"
    "float4 p(VSOUT v) : SV_TARGET0\n"
    "{\n"
    "	return float4(v.Color.xyz,1);\n"
    "}\n"
    "float4 w(float4 Position : SV_POSITION) : SV_TARGET0\n"
    "{\n"
    "	return color;//float4(140,140,140,255)/255.0f;\n"
    "}\n"
    //"float4 ws(float4 Position : SV_POSITION) : SV_TARGET0\n"
    //"{\n"
    //"	return float4(0,122,204,255)/255.0f;\n"
    //"}\n"
    //"float4 wsh(float4 Position : SV_POSITION) : SV_TARGET0\n"
    //"{\n"
    //"	return float4(196,196,196,255)/255.0f;\n"
    //"}\n"
    "float4 we(float4 Position : SV_POSITION) : SV_TARGET0\n"
    "{\n"
    "	return float4(1,0,0,1);\n"
    "}\n" );

  GridVertexShader = Device->CreateVertexShader( shader, strlen( shader ), "v", "vs_5_0" );
  GridPixelShader = Device->CreatePixelShader( shader, strlen( shader ), "p", "ps_5_0" );
  ErrPixelShader = Device->CreatePixelShader( shader, strlen( shader ), "we", "ps_5_0" );
  WirePixelShader = Device->CreatePixelShader( shader, strlen( shader ), "w", "ps_5_0" );
  //WirePixelShader_Selected = Device->CreatePixelShader(shader, strlen(shader), "ws", "ps_5_0");
  //WirePixelShader_Highlighted = Device->CreatePixelShader(shader, strlen(shader), "wsh", "ps_5_0");

  AntialiasedLineState = Device->CreateRasterizerState();
  AntialiasedLineState->SetAntialiasedLineEnable( true );
  AntialiasedLineState->SetMultisampleEnable( false );
  AntialiasedLineState->Update();

  DisableBlend = Device->CreateBlendState();
  DisableBlend->SetBlendEnable( 0, false );
  DisableBlend->Update();

  AlphaBlendState = Device->CreateBlendState();
  AlphaBlendState->SetBlendEnable( 0, true );
  AlphaBlendState->SetSrcBlend( 0, COREBLEND_SRCALPHA );
  AlphaBlendState->SetDestBlend( 0, COREBLEND_INVSRCALPHA );
  AlphaBlendState->Update();

  DepthLessEqual = Device->CreateDepthStencilState();
  DepthLessEqual->SetDepthFunc( CORECMP_LEQUAL );
  DepthLessEqual->Update();

  RenderDepthState_Normal = Device->CreateDepthStencilState();
  RenderDepthState_Normal->SetDepthEnable( true );
  RenderDepthState_Normal->SetDepthFunc( CORECMP_LEQUAL );
  RenderDepthState_Normal->Apply();

  RenderDepthState_Wireframe = Device->CreateDepthStencilState();
  RenderDepthState_Wireframe->SetDepthEnable( true );
  RenderDepthState_Wireframe->SetDepthFunc( CORECMP_ALWAYS );
  RenderDepthState_Wireframe->Apply();

  RenderRasterState = Device->CreateRasterizerState();
  RenderRasterState->SetAntialiasedLineEnable( false );
  RenderRasterState->Apply();

  RenderSamplerState = Device->CreateSamplerState();
  RenderSamplerState->SetAddressU( CORETEXADDRESS_WRAP );
  RenderSamplerState->SetAddressV( CORETEXADDRESS_WRAP );
  RenderSamplerState->SetAddressW( CORETEXADDRESS_WRAP );
  RenderSamplerState->SetFilter( COREFILTER_MIN_MAG_MIP_LINEAR );
  RenderSamplerState->Update();

  RenderPointSamplerState = Device->CreateSamplerState();
  RenderPointSamplerState->SetAddressU( CORETEXADDRESS_WRAP );
  RenderPointSamplerState->SetAddressV( CORETEXADDRESS_WRAP );
  RenderPointSamplerState->SetAddressW( CORETEXADDRESS_WRAP );
  RenderPointSamplerState->SetFilter( COREFILTER_MIN_MAG_MIP_POINT );
  RenderPointSamplerState->Update();

  InitUberTool( Device );

  SAFEDELETEA( Vertices );

}

void DeinitializeModelView()
{
  DeinitUberTool();
  SAFEDELETE( AntialiasedLineState );
  SAFEDELETE( WirePixelShader );
  //SAFEDELETE(WirePixelShader_Selected);
  //SAFEDELETE(WirePixelShader_Highlighted);
  SAFEDELETE( GridVertexBuffer );
  SAFEDELETE( GridIndexBuffer );
  SAFEDELETE( GridVertexShader );
  SAFEDELETE( GridPixelShader );
  SAFEDELETE( ErrPixelShader );
  SAFEDELETE( DisableBlend );
  SAFEDELETE( DepthLessEqual );
  SAFEDELETE( AlphaBlendState );
  SAFEDELETE( RenderDepthState_Normal );
  SAFEDELETE( RenderDepthState_Wireframe );
  SAFEDELETE( RenderRasterState );
  SAFEDELETE( RenderSamplerState );
  SAFEDELETE( RenderPointSamplerState );
}

//////////////////////////////////////////////////////////////////////////
//model view window

CapexModelView::CapexModelView() : CapexWindow()
{
  Maximized = false;
}

CapexModelView::CapexModelView( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  Maximized = false;
  SelectUberTool( UBERTOOL_MOVE );
  CWBButton *b = (CWBButton*)FindChildByID( _T( "gridbutton" ), _T( "button" ) );
  if ( b ) b->Push( true );
  JustBeenMaximized = true;
  NormalPosition = MaximizedPosition = Pos;
}

CapexModelView::~CapexModelView()
{

}

void CapexModelView::UpdateData()
{

}

TBOOL CapexModelView::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
    if ( Message.GetTarget() == GetGuid() )
    {
      if ( JustBeenMaximized )
      {
        JustBeenMaximized = false;
        break;
      }
      if ( Maximized )
        MaximizedPosition = Message.Rectangle;
      else
        NormalPosition = Message.Rectangle;
    }
    break;
  case WBM_KEYDOWN:
  {
    CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "snapvalue" ), _T( "textbox" ) );
    if ( ib && ib->InFocus() ) return false;
    switch ( Message.Key )
    {
    case VK_DELETE:
    {
      if ( WorkBench->GetEditedModel() )
      {
        WorkBench->GetEditedModel()->DeleteSelected();
        WorkBench->UpdateWindows( apEx_ModelGraph );
        WorkBench->UpdateWindows( apEx_ModelParameters );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
    }
    return true;
    break;
    case 'C':
    {
      if ( WorkBench->GetEditedModel() )
      {
        WorkBench->GetEditedModel()->CopySelected();
        WorkBench->UpdateWindows( apEx_ModelGraph );
        WorkBench->UpdateWindows( apEx_ModelParameters );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
    }
    return true;
    break;

    case 'Q':
    {
      SelectUberTool( UBERTOOL_MOVE );
      break;
    }

    case 'W':
    {
      SelectUberTool( UBERTOOL_ROTATE );
      break;
    }

    case 'E':
    {
      SelectUberTool( UBERTOOL_SCALE );
      break;
    }

    case 'G':
    {
      CWBButton *b = (CWBButton*)FindChildByID( _T( "gridbutton" ), _T( "button" ) );
      if ( !b ) break;
      b->Push( !b->IsPushed() );
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->SetGridView( b->IsPushed() );
      return true;
    }
    case 'F':
    {
      CWBButton *b = (CWBButton*)FindChildByID( _T( "maximizebutton" ), _T( "button" ) );
      if ( !b ) break;
      b->Push( !b->IsPushed() );
      Maximized = b->IsPushed();
      if ( b->IsPushed() )
        SetPosition( MaximizedPosition );
      else
        SetPosition( NormalPosition );
      JustBeenMaximized = true;
      return true;
      break;
    }
    case 'S':
    {
      if ( App->GetCtrlState() )
        break;
      CWBButton *b = (CWBButton*)FindChildByID( _T( "wfbutton" ), _T( "button" ) );
      if ( !b ) break;
      b->Push( !b->IsPushed() );
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->SetSolidView( b->IsPushed() );
      return true;
    }
    }

  }
  break;

  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "cameraselector" ) )
    {
      CWBContextMenu *ctx = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );
      ctx->AddItem( _T( "3D View" ), CAMERA_NORMAL );
      ctx->AddItem( _T( "Left" ), CAMERA_LEFT );
      ctx->AddItem( _T( "Right" ), CAMERA_RIGHT );
      ctx->AddItem( _T( "Top" ), CAMERA_TOP );
      ctx->AddItem( _T( "Bottom" ), CAMERA_BOTTOM );
      ctx->AddItem( _T( "Front" ), CAMERA_FRONT );
      ctx->AddItem( _T( "Back" ), CAMERA_BACK );
      return true;
    }

    if ( b->GetID() == _T( "rtselector" ) )
    {
      CWBContextMenu *ctx = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );
      CArray<CphxRenderTarget_Tool*> rts;
      for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
        for ( TS32 y = 0; y < Project.GetRenderLayerByIndex( x )->RenderTargets.NumItems(); y++ )
          rts.AddUnique( Project.GetRenderLayerByIndex( x )->RenderTargets[ y ] );

      for ( TS32 x = 0; x < rts.NumItems(); x++ )
        ctx->AddItem( rts[ x ]->Name.GetPointer(), x + 1000 );

      return true;
    }

    if ( b->GetID() == _T( "rotatebutton" ) )
    {
      SelectUberTool( UBERTOOL_ROTATE );
      return true;
    }

    if ( b->GetID() == _T( "scalebutton" ) )
    {
      SelectUberTool( UBERTOOL_SCALE );
      return true;
    }

    if ( b->GetID() == _T( "movebutton" ) )
    {
      SelectUberTool( UBERTOOL_MOVE );
      return true;
    }

    if ( b->GetID() == _T( "objspacebutton" ) )
    {
      SwitchObjSpaceMode();
      return true;
    }

    if ( b->GetID() == _T( "worldspacebutton" ) )
    {
      SwitchWorldSpaceMode();
      return true;
    }

    if ( b->GetID() == _T( "tintbutton" ) )
    {
      b->Push( !b->IsPushed() );
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->SetTintView( b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "gridbutton" ) )
    {
      b->Push( !b->IsPushed() );
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->SetGridView( b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "wfbutton" ) )
    {
      b->Push( !b->IsPushed() );
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->SetSolidView( b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "maximizebutton" ) )
    {
      b->Push( !b->IsPushed() );
      Maximized = b->IsPushed();
      if ( b->IsPushed() )
        SetPosition( MaximizedPosition );
      else
        SetPosition( NormalPosition );
      JustBeenMaximized = true;
      return true;
    }

    break;
  }

  case WBM_CONTEXTMESSAGE:
  {
    switch ( Message.Data )
    {
    case CAMERA_NORMAL:
    case CAMERA_LEFT:
    case CAMERA_RIGHT:
    case CAMERA_TOP:
    case CAMERA_BOTTOM:
    case CAMERA_FRONT:
    case CAMERA_BACK:
    {
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->FlushCamData();
      SetCameraMode( (MODELVIEWCAMERAMODE)Message.Data );
      return true;
    }
    }

    if ( Message.Data >= 1000 )
    {
      CArray<CphxRenderTarget_Tool*> rts;
      for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
        for ( TS32 y = 0; y < Project.GetRenderLayerByIndex( x )->RenderTargets.NumItems(); y++ )
          rts.AddUnique( Project.GetRenderLayerByIndex( x )->RenderTargets[ y ] );
      CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
      if ( !t ) break;
      t->SetRenderTarget( rts[ Message.Data - 1000 ]->GetGUID() );
      return true;
    }

    return true;
    break;
  }

  case WBM_LEFTBUTTONUP:
  {
    CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
    if ( !t ) break;
    t->UpdateRendertargetCollection();
    //Project.ForceDemoResolution(GetClientRect().Width(), GetClientRect().Height(), App->GetDevice());
    break;
  }
  case WBM_FOCUSGAINED:
  {
    CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
    if ( !t ) return true;
    t->UpdateRendertargetCollection();
    //Project.ForceDemoResolution(GetClientRect().Width(), GetClientRect().Height(), App->GetDevice());
    return true;
  }
  break;

  case WBM_TEXTCHANGED:
  {
    CWBTextBox *b = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "snapvalue" ) )
    {
      TF32 v = Root->GetUbertoolSnap();
      if ( b->GetText().Scan( _T( "%f" ), &v ) == 1 )
        Root->SetUbertoolSnap( v );
      return true;
    }

  }
  break;

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexModelView::ExportWindow( CXMLNode *node )
{
  CapexWindow::ExportWindow( node );
  if ( !node ) return;
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( !t ) return;
  node->SetAttributeFromInteger( _T( "cameramode" ), t->GetMode() );
  node->SetAttributeFromInteger( _T( "maximized" ), Maximized );
  node->SetAttributeFromInteger( _T( "nx1" ), NormalPosition.x1 );
  node->SetAttributeFromInteger( _T( "ny1" ), NormalPosition.y1 );
  node->SetAttributeFromInteger( _T( "nx2" ), NormalPosition.x2 );
  node->SetAttributeFromInteger( _T( "ny2" ), NormalPosition.y2 );
  node->SetAttributeFromInteger( _T( "mx1" ), MaximizedPosition.x1 );
  node->SetAttributeFromInteger( _T( "my1" ), MaximizedPosition.y1 );
  node->SetAttributeFromInteger( _T( "mx2" ), MaximizedPosition.x2 );
  node->SetAttributeFromInteger( _T( "my2" ), MaximizedPosition.y2 );
}

void CapexModelView::ImportConfig( CXMLNode *node, CRect &Pos )
{
  if ( !node ) return;
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( !t ) return;

  TS32 d = CAMERA_NORMAL;
  if ( node->HasAttribute( _T( "cameramode" ) ) ) node->GetAttributeAsInteger( _T( "cameramode" ), &d );
  SetCameraMode( (MODELVIEWCAMERAMODE)d );

  MaximizedPosition = NormalPosition = Pos;
  TS32 i = Maximized;
  if ( node->HasAttribute( _T( "maximized" ) ) ) node->GetAttributeAsInteger( _T( "maximized" ), &i );
  Maximized = 0;
  if ( node->HasAttribute( _T( "nx1" ) ) ) node->GetAttributeAsInteger( _T( "nx1" ), &NormalPosition.x1 );
  if ( node->HasAttribute( _T( "ny1" ) ) ) node->GetAttributeAsInteger( _T( "ny1" ), &NormalPosition.y1 );
  if ( node->HasAttribute( _T( "nx2" ) ) ) node->GetAttributeAsInteger( _T( "nx2" ), &NormalPosition.x2 );
  if ( node->HasAttribute( _T( "ny2" ) ) ) node->GetAttributeAsInteger( _T( "ny2" ), &NormalPosition.y2 );
  if ( node->HasAttribute( _T( "mx1" ) ) ) node->GetAttributeAsInteger( _T( "mx1" ), &MaximizedPosition.x1 );
  if ( node->HasAttribute( _T( "my1" ) ) ) node->GetAttributeAsInteger( _T( "my1" ), &MaximizedPosition.y1 );
  if ( node->HasAttribute( _T( "mx2" ) ) ) node->GetAttributeAsInteger( _T( "mx2" ), &MaximizedPosition.x2 );
  if ( node->HasAttribute( _T( "my2" ) ) ) node->GetAttributeAsInteger( _T( "my2" ), &MaximizedPosition.y2 );
  if ( Maximized ) SetPosition( MaximizedPosition );
  else SetPosition( NormalPosition );

  CWBButton *b = (CWBButton*)FindChildByID( _T( "maximizebutton" ), _T( "button" ) );
  if ( b ) b->Push( Maximized );

  t->SetRenderTarget( t->RenderTarget );
}

void CapexModelView::SetCameraMode( MODELVIEWCAMERAMODE m )
{
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( t )
  {
    t->SetCameraMode( m );
    CWBButton *b = (CWBButton*)FindChildByID( _T( "cameraselector" ), _T( "button" ) );
    b->SetText( CameraModeNames[ m - CAMERA_LEFT ] );

  }
}

void CapexModelView::SelectUberTool( UBERTOOLTYPE Type )
{
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( !t ) return;
  t->SelectUberTool( Type );

  CWBButton *r = (CWBButton*)FindChildByID( _T( "rotatebutton" ), _T( "button" ) );
  CWBButton *m = (CWBButton*)FindChildByID( _T( "movebutton" ), _T( "button" ) );
  CWBButton *s = (CWBButton*)FindChildByID( _T( "scalebutton" ), _T( "button" ) );

  TBOOL pushed[ 3 ];
  TBOOL pushed2[ 3 ];
  pushed[ 0 ] = pushed[ 1 ] = pushed[ 2 ] = false;
  pushed2[ 0 ] = pushed2[ 1 ] = pushed2[ 2 ] = false;

  if ( r ) pushed[ 0 ] = r->IsPushed();
  if ( m ) pushed[ 1 ] = m->IsPushed();
  if ( s ) pushed[ 2 ] = s->IsPushed();
  if ( Type == UBERTOOL_ROTATE ) pushed2[ 0 ] = true;
  if ( Type == UBERTOOL_MOVE ) pushed2[ 1 ] = true;
  if ( Type == UBERTOOL_SCALE ) pushed2[ 2 ] = true;

  if ( pushed[ 0 ] != pushed2[ 0 ] && r ) r->Push( pushed2[ 0 ] );
  if ( pushed[ 1 ] != pushed2[ 1 ] && m ) m->Push( pushed2[ 1 ] );
  if ( pushed[ 2 ] != pushed2[ 2 ] && s ) s->Push( pushed2[ 2 ] );
}

void CapexModelView::SwitchObjSpaceMode()
{
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( !t ) return;

  t->ObjectSpaceTransform = !t->ObjectSpaceTransform;
  CWBButton *r = (CWBButton*)FindChildByID( _T( "objspacebutton" ), _T( "button" ) );
  if ( r ) r->Push( t->ObjectSpaceTransform );

}

void CapexModelView::SwitchWorldSpaceMode()
{
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( !t ) return;

  t->CenterTransform = !t->CenterTransform;
  CWBButton *r = (CWBButton*)FindChildByID( _T( "worldspacebutton" ), _T( "button" ) );
  if ( r ) r->Push( t->CenterTransform );

}

void CapexModelView::SetUbertoolSnap()
{
  CWBModelDisplay *t = (CWBModelDisplay*)FindChildByID( _T( "display" ), _T( "modeldisplay" ) );
  if ( !t ) return;
  CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "snapvalue" ), _T( "textbox" ) );
  if ( ib && !ib->InFocus() )
  {
    TF32 f = Root->GetUbertoolSnap();
    TF32 v = 0;
    if ( ib->GetText().Scan( _T( "%f" ), &v ) == 1 )
      if ( v == f ) return;

    ib->SetText( CString::Format( _T( "%.3f" ), f ) );
  }
}

//////////////////////////////////////////////////////////////////////////
//model display guiitem

int WireframeShaderSorter( CphxRenderDataInstance **a, CphxRenderDataInstance **b )
{
  if ( !*a || !*b ) return 0;
  if ( ( *a )->PS == ( *b )->PS ) return 0;

  TS32 v1 = 0;
  TS32 v2 = 0;

  if ( ( *a )->MaterialData[ 0 ] == 140 / 255.0f ) v1 = 0;
  if ( ( *a )->MaterialData[ 0 ] == 0 ) v1 = 1;
  if ( ( *a )->MaterialData[ 0 ] == 196 / 255.0f ) v1 = 2;

  if ( ( *b )->MaterialData[ 0 ] == 140 / 255.0f ) v2 = 0;
  if ( ( *b )->MaterialData[ 0 ] == 0 ) v2 = 1;
  if ( ( *b )->MaterialData[ 0 ] == 196 / 255.0f ) v2 = 2;

  return v1 - v2;
}

void CWBModelDisplay::OnDraw( CWBDrawAPI *API )
{
  CWBItem::OnDraw( API );
  API->DrawRect( GetClientRect(), CColor::FromABGR( 0xff000000 ) );

  CapexWorkBench *wb = GetActiveWorkBench();
  if ( !wb ) return;
  if ( !wb->GetEditedModel() ) return;

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif

  CphxScene_Tool *sc = wb->GetModelPreviewScene();

  UpdateRendertargetCollection();
  Project.ApplyRenderTargets( RenderTargets );

  DefaultMaterial->RequestContent();
  CphxModel_Tool *m = wb->GetEditedModel();
  m->RequestContent();
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
    m->GetObjectByIndex( x )->RequestContent();

  if ( sc && sc->Scene.LayerCount )
    m->UpdateMaterialStates();

  m->SetModelerMode();

  if ( sc && !sc->Scene.LayerCount )
    sc->UpdateLayers();

  if ( sc && sc->Scene.LayerCount )
  {
    //Project.ForceDemoResolution(GetClientRect().Width(), GetClientRect().Height(), App->GetDevice());
    //PhxDepthBufferView = Project.GetDepthBuffer();

    for ( int x = 0; x < sc->Scene.LayerCount; x++ )
      if ( !sc->Scene.RenderLayers[ x ]->Descriptor->IgnoreHelperObjects )
      {
        DefaultTech->Tech->TargetLayer = sc->Scene.RenderLayers[ x ]->Descriptor;
        break;
      }

    CalculateMatrices();

    if ( Mode != CAMERA_NORMAL )
    {
      phxProjectionMatrix._33 = 0;
      phxProjectionMatrix._43 = 0.5;
    }

    API->FlushDrawBuffer();
    API->SetRenderView( ClientToScreen( GetClientRect() ) );

    RenderDepthState_Normal->Apply();
    RenderRasterState->Apply();
    RenderSamplerState->Apply( CORESMP_PS0 );
    RenderSamplerState->Apply( CORESMP_VS0 );

    sc->UpdateSceneGraph( 0, 0 );

    //draw camera grid
    if ( GridView )
    {
      if ( Mode != CAMERA_NORMAL ) DrawOrthoGrid( API );
      else
      {
        AntialiasedLineState->Apply();

        for ( TS32 x = 0; x < sc->Scene.LayerCount; x++ )
          if ( !sc->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
          {
            if ( sc->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
              continue;

            if ( sc->Scene.RenderLayers[ x ]->Descriptor->IgnoreHelperObjects )
              continue;

#ifdef MEMORY_TRACKING
            memTracker.Pause();
#endif
            CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
            memTracker.Resume();
#endif

            *i = *CameraGrid;
            i->DS = NULL;
            i->GS = NULL;
            i->HS = NULL;
            i->Indexed = true;
            sc->Scene.RenderLayers[ x ]->RenderInstances.Add( i );
          }
      }
    }

    if ( Mode < CAMERA_NORMAL )
    {
      //change to wireframe
      for ( TS32 x = 0; x < sc->Scene.LayerCount; x++ )
      {
        if ( sc->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
          continue;

        if ( SolidView )
          continue;

        for ( TS32 y = 0; y < sc->Scene.RenderLayers[ x ]->RenderInstances.NumItems(); y++ )
        {
          CphxRenderDataInstance *inst = sc->Scene.RenderLayers[ x ]->RenderInstances[ y ];

          inst->Indexed = true;
          inst->Wireframe = true;
          if ( !inst->VS ) inst->VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
          inst->GS = NULL;
          inst->VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
          inst->PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
          inst->MaterialData[ 0 ] = 140 / 255.0f;
          inst->MaterialData[ 1 ] = 140 / 255.0f;
          inst->MaterialData[ 2 ] = 140 / 255.0f;
          inst->MaterialData[ 3 ] = 1;
          inst->DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Wireframe->GetHandle();
          inst->BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
          inst->RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
          inst->RenderPriority = -10;
          if ( inst->ToolData )
          {
            CphxModelObject_Tool *t = (CphxModelObject_Tool*)inst->ToolData;

            inst->MaterialData[ 0 ] = t->WireframeColor.R() / 255.0f;
            inst->MaterialData[ 1 ] = t->WireframeColor.G() / 255.0f;
            inst->MaterialData[ 2 ] = t->WireframeColor.B() / 255.0f;
            inst->MaterialData[ 3 ] = 0.5f;

            if ( t->Selected )
            {
              inst->PS = (ID3D11PixelShader *)WirePixelShader->GetHandle();
              inst->MaterialData[ 0 ] = 1;//0;
              inst->MaterialData[ 1 ] = 1;//122/255.0f;
              inst->MaterialData[ 2 ] = 1;//204/255.0f;
              inst->MaterialData[ 3 ] = 1;
              inst->RenderPriority = -11;
            }
            else
              if ( t == MouseObject )
              {
                inst->PS = (ID3D11PixelShader *)WirePixelShader->GetHandle();
                inst->MaterialData[ 0 ] = 196 / 255.0f;
                inst->MaterialData[ 1 ] = 196 / 255.0f;
                inst->MaterialData[ 2 ] = 196 / 255.0f;
                inst->MaterialData[ 3 ] = 1;
                inst->RenderPriority = -12;
              }
          }
        }
        qsort( sc->Scene.RenderLayers[ x ]->RenderInstances.Array, sc->Scene.RenderLayers[ x ]->RenderInstances.NumItems(), sizeof( CphxRenderLayer* ), ( int( _cdecl* )( const void*, const void* ) )WireframeShaderSorter );
        //sc->Scene.RenderLayers[x]->RenderInstances.Sort(WireframeShaderSorter); //selected items are drawn later
      }
    }
    else
    {
      PHXMESHFILTER EditedFilterType = GetActiveWorkBench()->GetEditedModelFilterType();
      CphxMeshFilter_Tool *EditedFilter = GetActiveWorkBench()->GetEditedModelFilter();
      for ( TS32 x = 0; x < sc->Scene.LayerCount; x++ )
      {
        if ( sc->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
          continue;
        if ( sc->Scene.RenderLayers[ x ]->Descriptor->IgnoreHelperObjects )
          continue;

        for ( TS32 y = 0; y < sc->Scene.RenderLayers[ x ]->RenderInstances.NumItems(); y++ )
          if ( !sc->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
          {
            CphxRenderDataInstance *instance = sc->Scene.RenderLayers[ x ]->RenderInstances[ y ];
            CphxModelObject_Tool *t = (CphxModelObject_Tool*)instance->ToolData;

            if ( TintView || ( ( EditedFilterType == ModelFilter_TintMesh || EditedFilterType == ModelFilter_TintMeshShape ) && EditedFilter && EditedFilter->ParentObject == instance->ToolData ) )
            {
              //tint preview
              instance->VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
              instance->PS = (ID3D11PixelShader*)GridPixelShader->GetHandle();
              //instance->BlendState = (ID3D11BlendState*)TintBlendState->GetHandle();
            }
            else
            {
              if ( instance->ToolData )
              {
                if ( t->Selected )
                {
                  CphxRenderDataInstance *i = new CphxRenderDataInstance();
                  i->Indexed = true;
                  *i = *instance;

                  i->VertexBuffer = Cube->GetModelObject()->VxBuffer;
                  i->WireBuffer = Cube->GetModelObject()->WireBuffer;
                  i->WireIndexCount = Cube->GetModelObject()->EdgeCount * 2;

                  i->DS = NULL;
                  i->GS = NULL;
                  i->HS = NULL;

                  i->Wireframe = true;
                  //if (i->ToolData)
                  //	i->Matrices[0]=((CphxModelObject_Tool*)i->ToolData)->GetBoundingBoxMatrix();
                  i->ToolData = NULL;
                  i->PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
                  i->VS = (ID3D11VertexShader *)GridVertexShader->GetHandle();
                  i->MaterialData[ 0 ] = 0;
                  i->MaterialData[ 1 ] = 122 / 255.0f;
                  i->MaterialData[ 2 ] = 204 / 255.0f;
                  i->MaterialData[ 3 ] = 1;
                  i->RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
                  i->BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
                  i->DepthStencilState = (ID3D11DepthStencilState *)DepthLessEqual->GetHandle();
                  sc->Scene.RenderLayers[ x ]->RenderInstances.Add( i );
                }
              }
            }
          }
      }
    }

    D3DXVECTOR3 eye = D3DXVECTOR3( 0, 0, 0 );

    if ( GetCamData() )
      eye = GetCamData()->Eye;

    phxCameraPos = D3DXVECTOR4( eye.x, eye.y, eye.z, 1 );

    sc->KillInvalidBatches( Mode != CAMERA_NORMAL );
    sc->SortRenderLayers();

    if ( Mode != CAMERA_NORMAL )
    {
      for ( int x = 0; x < sc->Scene.LayerCount; x++ )
        if ( !sc->Scene.RenderLayers[ x ]->Descriptor->IgnoreHelperObjects )
          sc->Scene.RenderLayers[ x ]->Descriptor->SetEnvironment( true, true, 0 );

      sc->Scene.Render( false, false, 0 );
    }
    else
      sc->Render();

    TS32 tricount = sc->GetTriCount();
    CWBLabel *tric = (CWBLabel*)GetParent()->FindChildByID( _T( "tricount" ), _T( "label" ) );
    if ( tric )
      tric->SetText( CString::Format( _T( "Primitives: %d" ), tricount ) );

    //if (GridView)
    //{
    //	if (Mode == CAMERA_NORMAL)
    //		sc->Scene.RenderLayers[0]->RenderInstances[0] = NULL;
    //}

  }

  //Scene.ProcessTree(API->GetDevice());
  //Scene.Render(API->GetDevice());

  CphxRenderTarget_Tool *t = GetDisplayedRenderTarget();

  if ( sc && sc->Scene.LayerCount && t )
  {
    //pick mesh
    MouseObject = NULL;
    if ( !UberTool->IsPicked() )
    {
      CalculateMatrices();
      if ( MouseObject = Pick() )
      {
        App->SelectMouseCursor( CM_CROSS );
        if ( !UberTool->IsDragged() )
          SetTooltip( MouseObject->GetName() );
      }
    }
  }

  API->GetDevice()->ForceStateReset();
  API->SetUIRenderState();

  //display scene


  if ( sc && sc->Scene.LayerCount && t )
  {
    CCoreDX11Texture2D *dummy = new CCoreDX11Texture2D( (CCoreDX11Device*)API->GetDevice() );
    dummy->SetView( t->rt.View ); // sc->Scene.RenderLayers[sc->Scene.LayerCount-1]->Descriptor->Targets[0]->View

    API->FlushDrawBuffer();
    if ( Mode == CAMERA_NORMAL ) API->GetDevice()->SetRenderState( DisableBlend );
    API->GetDevice()->SetTexture( CORESMP_PS0, dummy );
    API->GetDevice()->SetPixelShader( Root->GammaDisplayShader ); //counteract the degamma in the display
    API->DrawRect( GetClientRect(), 0, 0, 1, 1 );

    API->FlushDrawBuffer();
    API->SetUIRenderState();

    dummy->SetTextureHandle( NULL );
    dummy->SetView( NULL );
    SAFEDELETE( dummy );

    DisplayUberTool( API );

    //free renderlayers
    //for (TS32 x = 0; x < sc->Scene.LayerCount; x++)
    //	sc->Scene.RenderLayers[x]->RenderInstances.FreeArray();

#ifdef MEMORY_TRACKING
    memTracker.SetMissingIgnore( false );
#endif
  }
}

CWBModelDisplay::CWBModelDisplay() : CWBItem()
{
  Mode = CAMERA_TOP;
  DragMode = CAMDRAGMODE_NONE;
  InitCameraGrid();
  ObjectSpaceTransform = false;
  CenterTransform = false;
  GridView = true;
  TintView = false;
  RenderTargets = NULL;
}

CWBModelDisplay::CWBModelDisplay( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  Mode = CAMERA_TOP;
  DragMode = CAMDRAGMODE_NONE;
  InitCameraGrid();
  SetCameraMode( CAMERA_NORMAL );
  UberTool = new CUberTool( App->GetDevice() );
  SelectUberTool( UBERTOOL_MOVE );
  ObjectSpaceTransform = false;
  CenterTransform = false;
  MouseObject = NULL;
  GridView = true;
  TintView = false;
  RenderTargets = Project.SpawnRenderTargetCollection( App->GetDevice(), Pos.Width(), Pos.Height(), false );
}

CWBModelDisplay::~CWBModelDisplay()
{
  UberTool->Gizmos.FreeArray();
  SAFEDELETE( UberTool );
  SAFEDELETE( CameraGrid );
  SAFEDELETE( RenderTargets );
}

CWBItem * CWBModelDisplay::Factory( CWBItem *RootItem, CXMLNode &node, CRect &Pos )
{
  return new CWBModelDisplay( RootItem, Pos );
}

void CWBModelDisplay::InitCameraGrid()
{
  InitializeModelView( App->GetDevice() );
  CameraGrid = new CphxRenderDataInstance;
  CameraGrid->Indexed = true;
  CameraGrid->Wireframe = true;
  CameraGrid->VertexBuffer = (ID3D11Buffer*)GridVertexBuffer->GetHandle();
  CameraGrid->IndexBuffer = (ID3D11Buffer*)GridIndexBuffer->GetHandle();
  CameraGrid->TriIndexCount = 0;
  CameraGrid->WireBuffer = (ID3D11Buffer*)GridIndexBuffer->GetHandle();
  CameraGrid->WireIndexCount = 640;
  CameraGrid->DS = NULL;
  CameraGrid->GS = NULL;
  CameraGrid->HS = NULL;
  CameraGrid->VS = (ID3D11VertexShader *)GridVertexShader->GetHandle();
  CameraGrid->PS = (ID3D11PixelShader *)GridPixelShader->GetHandle();
  CameraGrid->BlendState = NULL;
  CameraGrid->DepthStencilState = NULL;
  CameraGrid->RasterizerState = NULL;
  CameraGrid->ToolData = NULL;
  CameraGrid->RenderPriority = 256;
  D3DXMatrixIdentity( &CameraGrid->Matrices[ 0 ] );
  D3DXMatrixIdentity( &CameraGrid->Matrices[ 1 ] );
  for ( TS32 x = 0; x < 8; x++ )
    CameraGrid->Textures[ x ] = NULL;
}

TBOOL CWBModelDisplay::MessageProc( CWBMessage &Message )
{
  CAMERADATA *cd = GetCamData();

  switch ( Message.GetMessage() )
  {
  case WBM_MOUSEWHEEL:
  {
    if ( !cd ) break;

    if ( Mode == CAMERA_NORMAL )
    {
      D3DXVECTOR3 d = cd->Eye - cd->Target;
      cd->Eye = cd->Target + d*( 1 - Message.Data / 7.0f );
    }
    else
    {
      CRect r = GetClientRect();
      CPoint mx = ScreenToClient( App->GetMousePos() );
      float xv = ( mx.x / (float)( r.x2 - r.x1 ) - 0.5f )*( ( r.x2 - r.x1 ) / (float)( r.y2 - r.y1 ) );
      float yv = ( mx.y / (float)( r.y2 - r.y1 ) - 0.5f );

      //zoom with cursor center
      D3DXVECTOR3 CursorPos = cd->Target + cd->OrthoX*xv*OrthoZoom + cd->OrthoY*yv*OrthoZoom;

      D3DXVECTOR3 vt = cd->Target - CursorPos;
      D3DXVECTOR3 ve = cd->Eye - CursorPos;

      OrthoZoom *= 1 - Message.Data / 7.0f;

      CursorPos = cd->Target + cd->OrthoX*xv*OrthoZoom + cd->OrthoY*yv*OrthoZoom;
      cd->Target = vt + CursorPos;
      cd->Eye = ve + CursorPos;
    }
    return true;
    break;
  }
  case WBM_MOUSEMOVE:
  {
    if ( !cd ) break;
    CPoint sum = App->GetMousePos() - App->GetRightDownPos();
    if ( DragStartButton == 2 ) sum = App->GetMousePos() - App->GetMidDownPos();
    if ( DragStartButton == 0 ) sum = App->GetMousePos() - App->GetLeftDownPos();
    CRect r = GetClientRect();

    float xv = sum.x / (float)( r.x2 - r.x1 )*( ( r.x2 - r.x1 ) / (float)( r.y2 - r.y1 ) );
    float yv = sum.y / (float)( r.y2 - r.y1 );


    switch ( DragMode )
    {
    case CAMDRAGMODE_NONE:
      break;
    case CAMDRAGMODE_ROTATE:
    {
      D3DXMATRIX rot;

      D3DXVECTOR3 v = cd->_Eye - cd->_Target;
      D3DXVECTOR3 t;

      MatrixRotationAxis( &rot, &D3DXVECTOR3( cd->Up ), -sum.x*3.1415f / 180.0f );

      D3DXVECTOR4 o;
      D3DXVec3Transform( &o, &v, &rot );
      v = D3DXVECTOR3( o.x, o.y, o.z );

      MatrixRotationAxis( &rot, D3DXVec3Cross( &t, &v, &D3DXVECTOR3( cd->Up ) ), sum.y*3.1415f / 180.0f );
      D3DXVec3Transform( &o, &v, &rot );
      cd->Eye = cd->_Target + D3DXVECTOR3( o.x, o.y, o.z );
      return true;
      break;
    }
    case CAMDRAGMODE_PAN:
    {
      if ( !cd ) break;
      if ( Mode == CAMERA_NORMAL )
      {
        D3DXVECTOR3 vz = cd->_Eye - cd->_Target;
        D3DXVECTOR3 vx, vy;

        float l = D3DXVec3Length( &vz );
        D3DXVec3Normalize( &vz, &vz );

        D3DXVec3Cross( &vx, &vz, &D3DXVECTOR3( cd->_Up ) );
        D3DXVec3Normalize( &vx, &vx );
        D3DXVec3Cross( &vy, &vx, &vz );
        D3DXVec3Normalize( &vy, &vy );

        D3DXVECTOR3 movementvec = ( vx*xv + vy*yv )*l;

        cd->Eye = cd->_Eye + movementvec;
        cd->Target = cd->_Target + movementvec;
      }
      else
      {
        cd->Eye = cd->_Eye + cd->OrthoX*xv*OrthoZoom + cd->OrthoY*yv*OrthoZoom;
        cd->Target = cd->_Target + cd->OrthoX*xv*OrthoZoom + cd->OrthoY*yv*OrthoZoom;
      }
      return true;
      break;
    }

    case CAMDRAGMODE_UBERTOOL:
    {
      CalculateMatrices();
      UberTool->Drag( GetClientRect(), ScreenToClient( App->GetMousePos() ), phxProjectionMatrix, phxViewMatrix );
      ApplyUberToolValues();

      return true;
      break;
    }

    default:
      break;
    }

    break;
  }

  case WBM_LEFTBUTTONDOWN:
  {
    if ( App->GetMouseItem() != this ) break;
    if ( DragMode != CAMDRAGMODE_NONE ) break;
    App->SetCapture( this );
    DragStartButton = 0;

    if ( UberToolVisible() )
    {
      CalculateMatrices();
      UberTool->StartDrag( GetClientRect(), ScreenToClient( App->GetLeftDownPos() ), phxProjectionMatrix, phxViewMatrix );

      CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();

      //store current object positions here
      for ( int x = 0; x < m->GetObjectCount(); x++ )
        m->GetObjectByIndex( x )->_Transformation = m->GetObjectByIndex( x )->GetMatrix();

      if ( GetActiveWorkBench()->GetEditedModelFilter() )
      {
        for ( int x = 0; x < 12; x++ )
          GetActiveWorkBench()->GetEditedModelFilter()->_srt[ x ] = GetActiveWorkBench()->GetEditedModelFilter()->srt[ x ];
      }

      if ( UberTool->IsDragged() )
      {
        DragMode = CAMDRAGMODE_UBERTOOL;
        return true;
      }

    }

    if ( !UberTool->IsDragged() || !UberToolVisible() )
    {
      CphxModelObject_Tool *o = MouseObject;
      CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();
      if ( !m ) break;
      if ( !App->GetCtrlState() || !o )
      {
        for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
        {
          m->GetObjectByIndex( x )->Selected = false;
          for ( TS32 y = 0; y < m->GetObjectByIndex( x )->GetFilterCount(); y++ )
            m->GetObjectByIndex( x )->GetFilter( y )->Selected = false;
        }
      }

      if ( o )
        o->Selected = App->GetCtrlState() ? !o->Selected : true;
      GetActiveWorkBench()->SetEditedModelObject( o );
      GetActiveWorkBench()->UpdateWindows( apEx_ModelGraph );

      //WorkBench->SelectModelObject(o);
      //((wndModelGraph*)phxWindows[phxWindow_ModelGraph])->UpdateListHighlights();
    }

    return true;
    break;
  }

  case WBM_RIGHTBUTTONDOWN:
  {
    UberTool->SetUndo( true );
    ApplyUberToolValues();

    if ( App->GetMouseItem() != this ) break;
    if ( DragMode != CAMDRAGMODE_NONE ) break;

    App->SetCapture( this );

    if ( cd )
    {
      cd->_Eye = cd->Eye;
      cd->_Up = cd->Up;
      cd->_Target = cd->Target;
    }
    DragStartButton = 1;
    DragMode = CAMDRAGMODE_ROTATE;
    if ( App->GetShiftState() || Mode != CAMERA_NORMAL ) DragMode = CAMDRAGMODE_PAN;
    return true;
    break;
  }

  case WBM_MIDDLEBUTTONDOWN:
  {
    if ( App->GetMouseItem() != this ) break;
    if ( DragMode != CAMDRAGMODE_NONE ) break;
    App->SetCapture( this );
    if ( cd )
    {
      cd->_Eye = cd->Eye;
      cd->_Up = cd->Up;
      cd->_Target = cd->Target;
    }
    DragMode = CAMDRAGMODE_PAN;
    DragStartButton = 2;
    return true;
    break;
  }

  case WBM_RIGHTBUTTONDBLCLK:
    if ( !Config::RightDoubleClickCameraReset ) break;
  case WBM_MIDDLEBUTTONDBLCLK:
  {
    if ( DragMode == CAMDRAGMODE_ROTATE || DragMode == CAMDRAGMODE_PAN )
      SetCameraMode( Mode );

    if ( DragMode != CAMDRAGMODE_UBERTOOL )
      DragMode = CAMDRAGMODE_NONE;
    return true;
  }

  case WBM_LEFTBUTTONUP:
  {
    UberTool->EndDrag();
    if ( DragStartButton == 0 )
    {
      DragMode = CAMDRAGMODE_NONE;
      App->ReleaseCapture();
    }
    return true;
    break;
  }

  case WBM_RIGHTBUTTONUP:
  {
    UberTool->SetUndo( false );
    ApplyUberToolValues();
    if ( DragStartButton == 1 )
    {
      DragMode = CAMDRAGMODE_NONE;
      App->ReleaseCapture();
    }
    return true;
    break;
  }

  case WBM_MIDDLEBUTTONUP:
  {
    if ( DragStartButton == 2 )
    {
      DragMode = CAMDRAGMODE_NONE;
      App->ReleaseCapture();
    }
    return true;
    break;
  }

  default:
    break;
  }


  return CWBItem::MessageProc( Message );
}

void CWBModelDisplay::SetCameraMode( MODELVIEWCAMERAMODE m )
{
  CAMERADATA d;

  Mode = m;
  d.OrthoY = d.Up = D3DXVECTOR3( 0, 1, 0 );

  d.Up = D3DXVECTOR3( 0, 1, 0 );
  d.Target = D3DXVECTOR3( 0, 0, 0 );
  d.Eye = D3DXVECTOR3( 0, 0, -1 );

  switch ( Mode )
  {
  case CAMERA_NORMAL:
    d.Up = D3DXVECTOR3( 0, 1, 0 );
    d.Target = D3DXVECTOR3( 0, 0, 0 );
    d.Eye = D3DXVECTOR3( 0, 1, 3 );
    break;
  case CAMERA_LEFT:
    d.OrthoZ = d.Eye = D3DXVECTOR3( -4, 0, 0 );
    break;
  case CAMERA_RIGHT:
    d.OrthoZ = d.Eye = D3DXVECTOR3( 4, 0, 0 );
    break;
  case CAMERA_TOP:
    d.OrthoZ = d.Eye = D3DXVECTOR3( 0, 4, 0 );
    d.OrthoY = d.Up = D3DXVECTOR3( 0, 0, -4 );
    break;
  case CAMERA_BOTTOM:
    d.OrthoZ = d.Eye = D3DXVECTOR3( 0, -4, 0 );
    d.OrthoY = d.Up = D3DXVECTOR3( 0, 0, 4 );
    break;
  case CAMERA_FRONT:
    d.OrthoZ = d.Eye = D3DXVECTOR3( 0, 0, 4 );
    break;
  case CAMERA_BACK:
    d.OrthoZ = d.Eye = D3DXVECTOR3( 0, 0, -4 );
    break;
  default:
    break;
  }

  D3DXVec3Normalize( &d.OrthoY, &d.OrthoY );
  D3DXVec3Normalize( &d.OrthoZ, &d.OrthoZ );

  if ( Mode != CAMERA_NORMAL )
    UberTool->SetOrthoZ( d.OrthoZ );

  D3DXVec3Cross( &d.OrthoX, &d.OrthoZ, &d.OrthoY );
  OrthoZoom = 7.9f; //so the ortho grid has both colors of lines

  if ( GetActiveWorkBench()->GetEditedModel() )
    CameraData[ GetActiveWorkBench()->GetEditedModel()->GetGUID() ] = d;

}

void CWBModelDisplay::DrawOrthoGrid( CWBDrawAPI *API )
{
  TF32 GridDensity = 8;
  TU8 GridColor = 48;

  //the camera and projection matrices have already been set so we can use them to calculate the position of the origin
  D3DXVECTOR4 o = D3DXVECTOR4( 0, 0, 0, 1 );
  D3DXVec4Transform( &o, &o, &phxViewMatrix );
  D3DXVec4Transform( &o, &o, &phxProjectionMatrix );

  o /= 2.0f;
  o.y *= -1;
  o += D3DXVECTOR4( 0.5, 0.5, 0, 0 );

  //o is the position of the origin in viewport space

  CRect r = GetClientRect();
  //GUI->SetCropRect(r);

  TF32 StepSize = 1;

  //OrthoZoom equals the world space size of the height of the viewport
  //we want the grid to split the viewport TS32o about 8 equal areas

  while ( StepSize < OrthoZoom / GridDensity ) StepSize *= 2.0f;
  while ( StepSize > OrthoZoom / GridDensity ) StepSize /= 2.0f;

  StepSize *= ( r.y2 - r.y1 ) / OrthoZoom;

  D3DXVECTOR2 origin = D3DXVECTOR2( ( r.x2 - r.x1 )*o.x + r.x1, ( r.y2 - r.y1 )*o.y + r.y1 );
  D3DXVECTOR2 topleft = D3DXVECTOR2( (TF32)r.x1, (TF32)r.y1 );
  D3DXVECTOR2 bottomright = D3DXVECTOR2( (TF32)r.x2, (TF32)r.y2 );

  //big grid
  StepSize /= 2.0f;

  D3DXVECTOR2 v1 = ( topleft - origin ) / StepSize;
  TS32 v1x = (TS32)v1.x;
  TS32 v1y = (TS32)v1.y;

  D3DXVECTOR2 v2 = ( bottomright - origin ) / StepSize;
  TS32 v2x = (TS32)v2.x;
  TS32 v2y = (TS32)v2.y;


  TF32 bigstepsize = ( r.y2 - r.y1 ) / ( GridDensity * 2 );
  TF32 smallstepsize = ( r.y2 - r.y1 ) / ( GridDensity * 4 );
  TU8 gridalpha = (TU8)( ( StepSize - smallstepsize ) / ( bigstepsize - smallstepsize )*GridColor );

  for ( TS32 x = v1x - 1; x < v2x + 1; x++ )
  {
    D3DXVECTOR2 GridPos = origin + ( (TF32)x )*StepSize*D3DXVECTOR2( 1, 0 );
    API->DrawRect( CRect( CPoint( (TS32)GridPos.x, r.y1 ), CPoint( (TS32)GridPos.x + 1, r.y2 - 1 ) ), gridalpha * 0x010101 + 0xff000000 );
  }

  for ( TS32 y = v1y - 1; y < v2y + 1; y++ )
  {
    D3DXVECTOR2 GridPos = origin + ( (TF32)y )*StepSize*D3DXVECTOR2( 0, 1 );
    API->DrawRect( CRect( CPoint( r.x1, (TS32)GridPos.y ), CPoint( r.x2 - 1, (TS32)GridPos.y + 1 ) ), gridalpha * 0x010101 + 0xff000000 );
  }

  //big grid
  StepSize *= 2.0f;

  v1 = ( topleft - origin ) / StepSize;
  v1x = (TS32)v1.x;
  v1y = (TS32)v1.y;

  v2 = ( bottomright - origin ) / StepSize;
  v2x = (TS32)v2.x;
  v2y = (TS32)v2.y;

  for ( TS32 x = v1x - 1; x < v2x + 1; x++ )
  {
    D3DXVECTOR2 GridPos = origin + ( (TF32)x )*StepSize*D3DXVECTOR2( 1, 0 );
    API->DrawRect( CRect( CPoint( (TS32)GridPos.x, r.y1 ), CPoint( (TS32)GridPos.x + 1, r.y2 - 1 ) ), 0xff010101 * GridColor );
  }

  for ( TS32 y = v1y - 1; y < v2y + 1; y++ )
  {
    D3DXVECTOR2 GridPos = origin + ( (TF32)y )*StepSize*D3DXVECTOR2( 0, 1 );
    API->DrawRect( CRect( CPoint( r.x1, (TS32)GridPos.y ), CPoint( r.x2 - 1, (TS32)GridPos.y + 1 ) ), 0xff010101 * GridColor );
  }

  API->DrawRect( CRect( CPoint( r.x1, (TS32)origin.y ), CPoint( r.x2, (TS32)origin.y + 1 ) ), 0xff808080 );
  API->DrawRect( CRect( CPoint( (TS32)origin.x, r.y1 ), CPoint( (TS32)origin.x + 1, r.y2 - 1 ) ), 0xff808080 );
}

void CWBModelDisplay::SelectUberTool( UBERTOOLTYPE Tool )
{
  UberTool->Gizmos.FreeArray();
  SelectedUberTool = Tool;

  CUberToolGizmo *r;

  //MoveTool->Push(false);
  //RotateTool->Push(false);
  //ScaleTool->Push(false);

  switch ( Tool )
  {
  case UBERTOOL_MOVE:
  {
    D3DXMATRIX m;
    //mover planes
    r = new CUberToolGizmo_Move_Plane_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveYZ );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 0, -1, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Move_Plane_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveXZ );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 1, 0, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Move_Plane_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveXY );
    UberTool->Gizmos.Add( r );

    //mover axis
    r = new CUberToolGizmo_Move_Axis_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveZ );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Move_Axis_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveY );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( -1, 0, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Move_Axis_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveX );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 0, 1, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    //MoveTool->Push(true);
  }
  break;
  case UBERTOOL_ROTATE:
  {
    //rotators
    r = new CUberToolGizmo_Rotation_Standard( App->GetDevice() );
    r->SetType( UberTool_RotateZ );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Rotation_Standard( App->GetDevice() );
    r->SetType( UberTool_RotateY );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( -1, 0, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Rotation_Standard( App->GetDevice() );
    r->SetType( UberTool_RotateX );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 0, 1, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    //RotateTool->Push(true);
  }
  break;
  case UBERTOOL_SCALE:
  {
    //uniform scaler
    r = new CUberToolGizmo_Scale_Uniform_Standard( App->GetDevice() );
    r->SetType( UberTool_ScaleUniform );
    UberTool->Gizmos.Add( r );

    //uniform planes
    r = new CUberToolGizmo_Scale_Plane_Uniform_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveYZ );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 0, -1, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Scale_Plane_Uniform_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveXZ );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 1, 0, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Scale_Plane_Uniform_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveXY );
    UberTool->Gizmos.Add( r );


    //planes
    r = new CUberToolGizmo_Scale_Plane_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveYZ );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 0, -1, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Scale_Plane_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveXZ );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 1, 0, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Scale_Plane_Standard( App->GetDevice() );
    r->SetType( ubertool_MoveXY );
    UberTool->Gizmos.Add( r );

    //Scaler axis
    r = new CUberToolGizmo_Scale_Axis_Standard( App->GetDevice() );
    r->SetType( UberTool_ScaleZ );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Scale_Axis_Standard( App->GetDevice() );
    r->SetType( UberTool_ScaleY );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( -1, 0, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    r = new CUberToolGizmo_Scale_Axis_Standard( App->GetDevice() );
    r->SetType( UberTool_ScaleX );
    MatrixRotationAxis( &r->Transformation, &D3DXVECTOR3( 0, 1, 0 ), 3.14159265f / 2.0f );
    UberTool->Gizmos.Add( r );

    //ScaleTool->Push(true);
  }
  break;
  }
}

TBOOL CWBModelDisplay::UberToolVisible()
{
  if ( !GetActiveWorkBench()->GetEditedModel() ) return false;

  int selectedcount = 0;
  for ( int x = 0; x < GetActiveWorkBench()->GetEditedModel()->GetObjectCount(); x++ )
    if ( GetActiveWorkBench()->GetEditedModel()->GetObjectByIndex( x )->Selected ) selectedcount++;
  return selectedcount &&
    GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_NONE ||
    GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_MapXForm ||
    GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_TintMeshShape ||
    GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_UVMap ||
    GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_Replicate;
}

void CWBModelDisplay::ApplyUberToolValues()
{
  CAMERADATA *cd = GetCamData();
  if ( !cd ) return;

  if ( !GetActiveWorkBench()->GetEditedModel() ) return;
  if ( !UberTool->IsDragged() ) return;
  CPRS srt = UberTool->GetDragResult( App->GetShiftState(), Root->GetUbertoolSnap() );

  switch ( GetActiveWorkBench()->GetEditedModelFilterType() )
  {
  case ModelFilter_UVMap:
  case ModelFilter_TintMeshShape:
  {
    CphxMeshFilter_Tool *f = GetActiveWorkBench()->GetEditedModelFilter();

    D3DXMATRIX basetrans = f->GetTransformationMatrix();
    if ( f->ParentObject )
      D3DXMatrixMultiply( &basetrans, &basetrans, &f->ParentObject->GetMatrix() );

    if ( f->ParentObject )
    {
      D3DXVECTOR3 scale, translate;
      D3DXQUATERNION rot;

      D3DXMatrixDecompose( &scale, &rot, &translate, &f->GetTransformationMatrix() );
      srt.Translation *= CQuaternion( &rot.x );
    }

    GetActiveWorkBench()->GetEditedModelFilter()->ApplyTransformation( srt );
    GetActiveWorkBench()->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();

    UberTool->SetPosition( cd->Eye, cd->Target, D3DXVECTOR3( basetrans.m[ 3 ][ 0 ], basetrans.m[ 3 ][ 1 ], basetrans.m[ 3 ][ 2 ] ) );
    GetActiveWorkBench()->UpdateWindows( apEx_ModelMatrix );
    break;
  }
  case ModelFilter_MapXForm:
    GetActiveWorkBench()->GetEditedModelFilter()->ApplyTransformation( srt );
    GetActiveWorkBench()->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
    GetActiveWorkBench()->UpdateWindows( apEx_ModelMatrix );
    break;
  case ModelFilter_Replicate:
  {
    D3DXMATRIX Transformation;
    D3DXQUATERNION q;
    q.x = srt.Rotation.x;
    q.y = srt.Rotation.y;
    q.z = srt.Rotation.z;
    q.w = srt.Rotation.s;

    CphxMeshFilter_Tool *f = GetActiveWorkBench()->GetEditedModelFilter();
    D3DXMATRIX om = f->ParentObject->GetMatrix();
    D3DXMATRIX fm = f->GetRawMatrix_();

    //replicate*objtransform*ubertooltransform = X*objtransform - we need X
    //A*B*C = X*B
    //A*B*C*B^-1=X

    if ( !CenterTransform && !ObjectSpaceTransform )
    {
      D3DXMatrixTransformation( &Transformation, &UberTool->UberToolPos_DragStart, NULL, &D3DXVECTOR3( srt.Scale ), &UberTool->UberToolPos_DragStart, &q, &D3DXVECTOR3( srt.Translation ) );
    }
    else
    {
      D3DXMatrixTransformation( &Transformation, NULL, NULL, &D3DXVECTOR3( srt.Scale ), NULL, &q, &D3DXVECTOR3( srt.Translation ) );
    }

    D3DXMATRIX A = fm;
    D3DXMATRIX B = om;
    D3DXMATRIX C = Transformation;
    D3DXMATRIX Bi;
    D3DXMatrixInverse( &Bi, NULL, &B );

    D3DXMATRIX AB;
    D3DXMatrixMultiply( &AB, &A, &B );
    D3DXMATRIX ABC;

    if ( !ObjectSpaceTransform )
      D3DXMatrixMultiply( &ABC, &AB, &C );
    else
      D3DXMatrixMultiply( &ABC, &C, &AB );

    D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
    D3DXVec4Transform( &v, &v, &ABC );
    D3DXVECTOR3 ubertoolpos = D3DXVECTOR3( v.x, v.y, v.z );

    D3DXMATRIX ABCBi;
    D3DXMatrixMultiply( &ABCBi, &ABC, &Bi );

    f->SetRawMatrix( ABCBi );
    GetActiveWorkBench()->UpdateWindows( apEx_ModelMatrix );

    GetActiveWorkBench()->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
    UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );
    break;
  }
  case ModelFilter_NONE:
  {
    D3DXMATRIX Transformation;
    D3DXQUATERNION q;
    q.x = srt.Rotation.x;
    q.y = srt.Rotation.y;
    q.z = srt.Rotation.z;
    q.w = srt.Rotation.s;

    if ( !CenterTransform && !ObjectSpaceTransform )
    {
      D3DXMatrixTransformation( &Transformation, &UberTool->UberToolPos_DragStart, NULL, &D3DXVECTOR3( srt.Scale ), &UberTool->UberToolPos_DragStart, &q, &D3DXVECTOR3( srt.Translation ) );
    }
    else
    {
      D3DXMatrixTransformation( &Transformation, NULL, NULL, &D3DXVECTOR3( srt.Scale ), NULL, &q, &D3DXVECTOR3( srt.Translation ) );
    }

    D3DXVECTOR3 ubertoolpos = D3DXVECTOR3( 0, 0, 0 );
    int cnt = 0;

    //apply transformations here
    for ( int x = 0; x < GetActiveWorkBench()->GetEditedModel()->GetObjectCount(); x++ )
      if ( GetActiveWorkBench()->GetEditedModel()->GetObjectByIndex( x )->Selected )
      {
        CphxModelObject_Tool *m = GetActiveWorkBench()->GetEditedModel()->GetObjectByIndex( x );
        D3DXMATRIX t = m->GetMatrix();

        if ( !ObjectSpaceTransform )
          D3DXMatrixMultiply( &t, &m->_Transformation, &Transformation );
        else
          D3DXMatrixMultiply( &t, &Transformation, &m->_Transformation );
        D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
        D3DXVec4Transform( &v, &v, &t );
        ubertoolpos += D3DXVECTOR3( v.x, v.y, v.z );

        m->SetMatrix( t );
        GetActiveWorkBench()->UpdateWindows( apEx_ModelMatrix );
        cnt++;
      }

    if ( cnt ) ubertoolpos /= (float)cnt;

    if ( CenterTransform && !ObjectSpaceTransform ) ubertoolpos = D3DXVECTOR3( 0, 0, 0 );

    UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );
  }
  }

}

void CWBModelDisplay::CalculateMatrices()
{
  CAMERADATA *cd = GetCamData();
  if ( !cd ) return;

  D3DXMatrixLookAtRH( &phxViewMatrix, &cd->Eye, &cd->Target, &cd->Up );

  if ( Mode == CAMERA_NORMAL )
  {
    D3DXMatrixPerspectiveFovRH( &phxProjectionMatrix, 45, GetClientRect().Width() / (TF32)GetClientRect().Height(), 0.01f, 2000.0f );
  }
  else
  {
    D3DXMatrixOrthoRH( &phxProjectionMatrix, OrthoZoom * GetClientRect().Width() / (TF32)GetClientRect().Height(), OrthoZoom, 0.1f, 2000.0f );
    //phxProjectionMatrix._33 = 0;
    //phxProjectionMatrix._43 = 0.5;
  }
}

CphxModelObject_Tool * CWBModelDisplay::GetEditedModelObject()
{
  if ( !GetActiveWorkBench()->GetEditedModel() ) return NULL;
  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
    if ( m->GetObjectByIndex( x )->Selected ) return m->GetObjectByIndex( x );
  return NULL;
}

CphxModelObject_Tool * CWBModelDisplay::Pick()
{
  if ( !MouseOver() ) return false;
  if ( !GetActiveWorkBench()->GetEditedModel() ) return false;

  CphxScene *Scene = &GetActiveWorkBench()->GetModelPreviewScene()->Scene;
  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();

  CphxModelObject_Tool *picked = NULL;

  float mt;

  CphxRenderTarget_Tool *rt = GetDisplayedRenderTarget();

  for ( int x = 0; x < m->GetObjectCount(); x++ )
  {
    int cnt = 0;
    for ( int y = 0; y < Scene->LayerCount; y++ )
      for ( int z = 0; z < Scene->RenderLayers[ y ]->RenderInstances.NumItems(); z++ )
      {
        TBOOL foundrt = false;
        for ( TS32 a = 0; a < Scene->RenderLayers[ y ]->Descriptor->TargetCount; a++ )
          if ( Scene->RenderLayers[ y ]->Descriptor->Targets[ a ] == &rt->rt )
            foundrt = true;

        if ( foundrt && Scene->RenderLayers[ y ]->RenderInstances[ z ] && Scene->RenderLayers[ y ]->RenderInstances[ z ]->ToolData == m->GetObjectByIndex( x ) ) cnt++;
      }

    if ( cnt ) //no need to pick objects that aren't rendered at all
    {
      float t = 10000;
      D3DXMATRIX mx;
      D3DXMatrixIdentity( &mx );
      if ( m->GetObjectByIndex( x )->Pick( GetClientRect(), ScreenToClient( App->GetMousePos() ), phxProjectionMatrix, phxViewMatrix, mx, t ) )
        if ( !picked || t < mt )
        {
          mt = t;
          picked = m->GetObjectByIndex( x );
        }
    }
  }
  return picked;

}

void CWBModelDisplay::UpdateUberToolColor()
{
  switch ( GetActiveWorkBench()->GetEditedModelFilterType() )
  {
  case ModelFilter_NONE:
    for ( int x = 0; x < UberTool->Gizmos.NumItems(); x++ )
      UberTool->Gizmos[ x ]->SetType( UberTool->Gizmos[ x ]->GetType() );
    break;
  default:
  {
    //float r, g, b;
    //unsigned int col = CColor::FromFloat(0, 122 / 255.0f, 204 / 255.0f, 255 / 255.0f);
    //r = ((col & 0x00ff0000) >> 16) / 255.0f;
    //g = ((col & 0x0000ff00) >> 8) / 255.0f;
    //b = ((col & 0x000000ff)) / 255.0f;
    for ( int x = 0; x < UberTool->Gizmos.NumItems(); x++ )
      //UberTool->Gizmos[x]->SetColor(0, 122 / 255.0f / 0.7f, 204 / 255.0f / 0.7f);
      UberTool->Gizmos[ x ]->SetColor( 1, 1, 0 );
  }
  break;
  }
}

void CWBModelDisplay::DisplayUberTool( CWBDrawAPI *API )
{
  //display ubertool
  CAMERADATA *cd = GetCamData();
  if ( !cd ) return;

  CphxModel_Tool *Model = GetActiveWorkBench()->GetEditedModel();

  if ( UberToolVisible() )
  {
    UpdateUberToolColor();
    if ( !UberTool->IsDragged() )
    {
      D3DXVECTOR3 ubertoolpos = D3DXVECTOR3( 0, 0, 0 );
      int cnt = 0;

      //calculate ubertool center
      for ( int x = 0; x < Model->GetObjectCount(); x++ )
        if ( Model->GetObjectByIndex( x )->Selected )
        {
          D3DXMATRIX t = Model->GetObjectByIndex( x )->GetMatrix();
          D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
          D3DXVec4Transform( &v, &v, &t );
          ubertoolpos += D3DXVECTOR3( v.x, v.y, v.z );
          cnt++;
        }

      if ( cnt ) ubertoolpos /= (float)cnt;

      D3DXMATRIX basetrans;
      D3DXMatrixIdentity( &basetrans );

      if ( ObjectSpaceTransform && GetEditedModelObject() )
      {
        basetrans = GetEditedModelObject()->GetMatrix();
        basetrans.m[ 3 ][ 0 ] = basetrans.m[ 3 ][ 1 ] = basetrans.m[ 3 ][ 2 ] = basetrans.m[ 0 ][ 3 ] = basetrans.m[ 1 ][ 3 ] = basetrans.m[ 2 ][ 3 ] = 0;
      }

      UberTool->SetBaseTransform( basetrans );


      if ( CenterTransform && !ObjectSpaceTransform ) ubertoolpos = D3DXVECTOR3( 0, 0, 0 );

      switch ( GetActiveWorkBench()->GetEditedModelFilterType() )
      {
      case ModelFilter_UVMap:
      case ModelFilter_TintMeshShape:
      {
        D3DXMATRIX mx = basetrans;
        CphxMeshFilter_Tool *f = GetActiveWorkBench()->GetEditedModelFilter();
        basetrans = f->GetTransformationMatrix();

        if ( f->ParentObject )
          D3DXMatrixMultiply( &basetrans, &basetrans, &f->ParentObject->GetMatrix() );

        ubertoolpos = D3DXVECTOR3( basetrans.m[ 3 ][ 0 ], basetrans.m[ 3 ][ 1 ], basetrans.m[ 3 ][ 2 ] );

        //if ( SelectedUberTool == UBERTOOL_SCALE )
        UberTool->SetBaseTransform( basetrans );
        UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );

        //ubertoolpos = basetrans;

        //if ( SelectedUberTool != UBERTOOL_SCALE )
        //basetrans = mx;
        //else
        //  ubertoolpos = D3DXVECTOR3( 0, 0, 0 );
      }
      break;
      case ModelFilter_Replicate:
      {

        CphxMeshFilter_Tool *f = GetActiveWorkBench()->GetEditedModelFilter();
        D3DXMATRIX A = f->GetRawMatrix();
        D3DXMATRIX B = f->ParentObject->GetMatrix();
        D3DXMATRIX AB;
        D3DXMatrixMultiply( &AB, &A, &B );

        D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
        D3DXVec4Transform( &v, &v, &AB );
        ubertoolpos = D3DXVECTOR3( v.x, v.y, v.z );

        D3DXMatrixIdentity( &basetrans );

        if ( ObjectSpaceTransform )
        {
          basetrans = AB;
          basetrans.m[ 3 ][ 0 ] = basetrans.m[ 3 ][ 1 ] = basetrans.m[ 3 ][ 2 ] = basetrans.m[ 0 ][ 3 ] = basetrans.m[ 1 ][ 3 ] = basetrans.m[ 2 ][ 3 ] = 0;
        }

        UberTool->SetBaseTransform( basetrans );

        if ( CenterTransform && !ObjectSpaceTransform ) ubertoolpos = D3DXVECTOR3( 0, 0, 0 );
        UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );
      }
      break;
      default:
        UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );
        break;
      }

      if ( Mode == CAMERA_NORMAL )
      {
        D3DXVECTOR3 cp = cd->Eye - ubertoolpos;

        D3DXMATRIX m;
        if ( D3DXMatrixInverse( &m, NULL, &basetrans ) )
        {
          D3DXVECTOR4 v;
          D3DXVec3Transform( &v, &cp, &m );
          cp = D3DXVECTOR3( v.x, v.y, v.z ) / v.w;
        }

        if ( cp.x != 0 ) cp.x /= abs( cp.x );	else cp.x = 1;
        if ( cp.y != 0 ) cp.y /= abs( cp.y );	else cp.y = 1;
        if ( cp.z != 0 ) cp.z /= abs( cp.z );	else cp.z = 1;

        D3DXMATRIX m2;
        D3DXMatrixIdentity( &m2 );
        m2.m[ 0 ][ 0 ] *= cp.x;
        m2.m[ 1 ][ 1 ] *= cp.y;
        m2.m[ 2 ][ 2 ] *= cp.z;

        UberTool->SetDisplayTransform( m2 );
      }


    }
    else
    {
      //tooltips:

      CPRS srt = UberTool->GetDragResult( App->GetShiftState(), Root->GetUbertoolSnap() );
      CString c;

      if ( srt.Scale != CVector3( 1, 1, 1 ) ) c = CString::Format( "Scale: %.2f %.2f %.2f", ( (int)( srt.Scale.x * 100 ) ) / 100.0f, ( (int)( srt.Scale.y * 100 ) ) / 100.0f, ( (int)( srt.Scale.z * 100 ) ) / 100.0f );
      if ( srt.Translation != CVector3( 0, 0, 0 ) ) c = CString::Format( "Move: %.2f %.2f %.2f", ( (int)( srt.Translation.x * 100 ) ) / 100.0f, ( (int)( srt.Translation.y * 100 ) ) / 100.0f, ( (int)( srt.Translation.z * 100 ) ) / 100.0f );
      if ( srt.Rotation != CQuaternion( 0, 0, 0, 1 ) )
      {
        D3DXVECTOR3 v;
        float a;
        D3DXQUATERNION q;
        q.x = srt.Rotation.x;
        q.y = srt.Rotation.y;
        q.z = srt.Rotation.z;
        q.w = srt.Rotation.s;
        D3DXQuaternionToAxisAngle( &q, &v, &a );
        c = CString::Format( "Rotation: %.1f", a / 3.14159265f*180.0f );
      }

      if ( c.Length() )
      {
        SetTooltip( c );
      }

    }

    UberTool->EnableHelperDisplay( 0 );

    if ( GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_UVMap )
    {
      CphxMeshFilter_Tool *f = GetActiveWorkBench()->GetEditedModelFilter();
      if ( f->Parameters[ 0 ] != 4 )
        UberTool->EnableHelperDisplay( f->Parameters[ 0 ] + 1 );
    }

    if ( GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_TintMeshShape )
    {
      CphxMeshFilter_Tool *f = GetActiveWorkBench()->GetEditedModelFilter();
      switch ( f->Parameters[ 0 ] )
      {
      case 0:
        UberTool->EnableHelperDisplay( 4 );
        break;
      case 1:
        UberTool->EnableHelperDisplay( 2 );
        break;
      case 2:
        UberTool->EnableHelperDisplay( 1 );
        break;
      }
    }

    if ( Mode != CAMERA_NORMAL )	CalculateMatrices(); //fix ortho matrix for picking

    App->GetDevice()->Clear( false, true );

    CRect vpr = ClientToScreen( GetClientRect() );
    App->GetDevice()->SetViewport( vpr );

    if ( Mode != CAMERA_NORMAL )
      UberTool->SetScale( cd->Eye, cd->Target, OrthoZoom / 25.0f );
    else
      UberTool->SetScale( cd->Eye, cd->Target, 0.2f );

    UberTool->Pick( GetClientRect(), ScreenToClient( App->GetMousePos() ), phxProjectionMatrix, phxViewMatrix );

    D3DXMATRIX helper;
    D3DXMatrixIdentity( &helper );
    if ( GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_UVMap || GetActiveWorkBench()->GetEditedModelFilterType() == ModelFilter_TintMeshShape )
    {
      helper = GetActiveWorkBench()->GetEditedModelFilter()->GetTransformationMatrix();

      if ( GetActiveWorkBench()->GetEditedModelFilter()->ParentObject )
        D3DXMatrixMultiply( &helper, &helper, &GetActiveWorkBench()->GetEditedModelFilter()->ParentObject->GetMatrix() );
    }

    UberTool->Display( phxProjectionMatrix, phxViewMatrix, helper );

    API->GetDevice()->ForceStateReset();
    API->SetUIRenderState();
  }
}

void CWBModelDisplay::UpdateRendertargetCollection()
{
  RenderTargets->UpdateCollection( GetClientRect().Width(), GetClientRect().Height(), false );
}

CAMERADATA * CWBModelDisplay::GetCamData()
{
  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();
  if ( !m ) return NULL;

  if ( !CameraData.HasKey( m->GetGUID() ) )
    SetCameraMode( Mode );

  return &CameraData[ m->GetGUID() ];
}

CphxRenderTarget_Tool * CWBModelDisplay::GetDisplayedRenderTarget()
{
  CphxRenderTarget_Tool *t = Project.GetRenderTarget( RenderTarget );

  if ( !t )
  {
    for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
    {
      CphxRenderLayerDescriptor_Tool *r = Project.GetRenderLayerByIndex( x );
      if ( r->HasPicking )
      {
        t = r->RenderTargets[ 0 ];
        SetRenderTarget( t->GetGUID() );
        return t;
      }
    }
  }

  return t;
}

void CWBModelDisplay::SetRenderTarget( CphxGUID guid )
{
  RenderTarget = guid;
  CphxRenderTarget_Tool *r = Project.GetRenderTarget( guid );
  if ( !r ) return;
  CWBButton *b = (CWBButton*)FindChildByID( _T( "rtselector" ), _T( "button" ) );
  if ( !b ) return;
  b->SetText( r->Name );
}
