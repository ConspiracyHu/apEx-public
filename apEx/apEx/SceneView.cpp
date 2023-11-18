#include "BasePCH.h"
#include "SceneView.h"
#define WINDOWNAME _T("Scene View")
#define WINDOWXML _T("SceneView")
#include "apExRoot.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include "SceneObjectParameters.h"
#include "SceneSplineEditor.h"
#include "Config.h"

void InitializeModelView( CCoreDevice *Device );
extern CCoreVertexBuffer *GridVertexBuffer;
extern CCoreIndexBuffer *GridIndexBuffer;
extern CCorePixelShader *GridPixelShader;
extern CCorePixelShader *ErrPixelShader;
extern CCorePixelShader *WirePixelShader;
//extern CCorePixelShader *WirePixelShader_Selected;
//extern CCorePixelShader *WirePixelShader_Highlighted;
extern CCoreVertexShader *GridVertexShader;
extern CCoreRasterizerState *AntialiasedLineState;
extern CCoreBlendState *DisableBlend;
extern CCoreBlendState *AlphaBlendState;
extern CCoreDepthStencilState *DepthLessEqual;
extern CCoreDepthStencilState *RenderDepthState_Normal;
extern CCoreDepthStencilState *RenderDepthState_Wireframe;
extern CCoreRasterizerState *RenderRasterState;
extern CCoreSamplerState *RenderSamplerState;
extern CapexRoot *Root;
extern CString CameraModeNames[ 7 ];

bool CWBSceneDisplay::ParticlesCalculatedInFrame = false;

D3DXMATRIX * WINAPI MatrixRotationAxis( D3DXMATRIX *out, const D3DXVECTOR3 *v, FLOAT angle );


CapexSceneView::CapexSceneView() : CapexWindow()
{
}

CapexSceneView::CapexSceneView( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  SelectUberTool( UBERTOOL_MOVE );
  CWBButton *b = (CWBButton*)FindChildByID( _T( "gridbutton" ), _T( "button" ) );
  if ( b ) b->Push( true );
  b = (CWBButton*)FindChildByID( _T( "helperbutton" ), _T( "button" ) );
  if ( b ) b->Push( true );
  JustBeenMaximized = true;
  Maximized = false;
  NormalPosition = MaximizedPosition = Pos;
}

CapexSceneView::~CapexSceneView()
{
}

void CapexSceneView::UpdateData()
{

}

void CapexSceneView::ExportWindow( CXMLNode *node )
{
  CapexWindow::ExportWindow( node );
  if ( !node ) return;
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
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

void CapexSceneView::ImportConfig( CXMLNode *node, CRect &Pos )
{
  if ( !node ) return;
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
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

void CapexSceneView::SetCameraMode( MODELVIEWCAMERAMODE m, CphxObject_Tool *Camera )
{
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
  if ( t )
  {
    t->SetCameraMode( m, Camera );
    CWBButton *b = (CWBButton*)FindChildByID( _T( "cameraselector" ), _T( "button" ) );
    if ( !Camera )
    {
      b->SetText( CameraModeNames[ m - CAMERA_LEFT ] );
    }
    else
      b->SetText( Camera->GetName() );
  }
}

void CapexSceneView::SelectUberTool( UBERTOOLTYPE Type )
{
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
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

void CapexSceneView::SwitchObjSpaceMode()
{
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
  if ( !t ) return;

  t->ObjectSpaceTransform = !t->ObjectSpaceTransform;
  CWBButton *r = (CWBButton*)FindChildByID( _T( "objspacebutton" ), _T( "button" ) );
  if ( r ) r->Push( t->ObjectSpaceTransform );
}

void CapexSceneView::SwitchWorldSpaceMode()
{
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
  if ( !t ) return;

  t->CenterTransform = !t->CenterTransform;
  CWBButton *r = (CWBButton*)FindChildByID( _T( "worldspacebutton" ), _T( "button" ) );
  if ( r ) r->Push( t->CenterTransform );

}

void CapexSceneView::SetUbertoolSnap()
{
  CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
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

TBOOL CapexSceneView::MessageProc( CWBMessage &Message )
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
      if ( WorkBench->GetEditedScene() )
      {
        WorkBench->GetEditedScene()->DeleteSelected();
        WorkBench->UpdateWindows( apEx_SceneGraph );
        WorkBench->UpdateWindows( apEx_SceneObjectParameters );
        WorkBench->UpdateWindows( apEx_SceneSplineEditor );
      }
    }
    return true;
    break;
    case 'C':
    {
      if ( WorkBench->GetEditedScene() )
      {
        if ( !App->GetCtrlState() )
        {
          WorkBench->GetEditedScene()->CopySelected();
          WorkBench->UpdateWindows( apEx_SceneGraph );
          WorkBench->UpdateWindows( apEx_SceneObjectParameters );
          for ( int x = 0; x < WorkBench->GetEditedScene()->GetObjectCount(); x++ )
            if ( WorkBench->GetEditedScene()->GetObjectByIndex( x )->Selected )
            {
              CapexSceneObjectParameters* w = (CapexSceneObjectParameters*)WorkBench->GetWindow( apEx_SceneObjectParameters );
              if ( w ) w->SelectSceneObject( WorkBench->GetEditedScene()->GetObjectByIndex( x ) );
              GetActiveWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
              break;
            }
        }
        else
          WorkBench->GetEditedScene()->MarkCopyPaste();
      }
    }
    return true;
    break;
    case 'V':
    {
      if ( WorkBench->GetEditedScene() )
      {
        if ( App->GetCtrlState() )
        {
          WorkBench->GetEditedScene()->PasteObjects();
          WorkBench->UpdateWindows( apEx_SceneGraph );
          WorkBench->UpdateWindows( apEx_SceneObjectParameters );

          for ( int x = 0; x < WorkBench->GetEditedScene()->GetObjectCount(); x++ )
            if ( WorkBench->GetEditedScene()->GetObjectByIndex( x )->Selected )
            {
              CapexSceneObjectParameters* w = (CapexSceneObjectParameters*)WorkBench->GetWindow( apEx_SceneObjectParameters );
              if ( w ) w->SelectSceneObject( WorkBench->GetEditedScene()->GetObjectByIndex( x ) );
              GetActiveWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
              break;
            }
        }
        else
          break;
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
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetGridView( b->IsPushed() );
      return true;
      break;
    }
    case 'H':
    {
      CWBButton *b = (CWBButton*)FindChildByID( _T( "helperbutton" ), _T( "button" ) );
      if ( !b ) break;
      b->Push( !b->IsPushed() );
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetHelperView( b->IsPushed() );
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
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetSolidView( b->IsPushed() );
      return true;
      break;
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

      if ( WorkBench->GetEditedScene() )
      {
        for ( TS32 x = 0; x < WorkBench->GetEditedScene()->GetObjectCount(); x++ )
        {
          CphxObject_Tool *o = WorkBench->GetEditedScene()->GetObjectByIndex( x );
          if ( o->GetObjectType() == Object_CamEye )
            ctx->AddItem( o->GetName().GetPointer(), 100 + x );
        }
      }

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

    if ( b->GetID() == _T( "gridbutton" ) )
    {
      b->Push( !b->IsPushed() );
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetGridView( b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "wfbutton" ) )
    {
      b->Push( !b->IsPushed() );
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetSolidView( b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "helperbutton" ) )
    {
      b->Push( !b->IsPushed() );
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetHelperView( b->IsPushed() );
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
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
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
      CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
      if ( !t ) break;
      t->SetRenderTarget( rts[ Message.Data - 1000 ]->GetGUID() );
      return true;
    }

    if ( Message.Data >= 100 && WorkBench->GetEditedScene() )
    {
      CphxScene_Tool *s = WorkBench->GetEditedScene();
      if ( s->GetObjectCount() > Message.Data - 100 )
        if ( s->GetObjectByIndex( Message.Data - 100 )->GetObjectType() == Object_CamEye )
        {
          CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
          if ( !t ) break;
          SetCameraMode( CAMERA_CAMERAVIEW, s->GetObjectByIndex( Message.Data - 100 ) );
          return true;
        }
    }

    return true;
    break;
  }

  case WBM_LEFTBUTTONUP:
  {
    CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
    if ( !t ) break;
    t->UpdateRendertargetCollection();
    //Project.ForceDemoResolution(GetClientRect().Width(), GetClientRect().Height(), App->GetDevice());
    break;
  }
  case WBM_FOCUSGAINED:
  {
    CWBSceneDisplay *t = (CWBSceneDisplay*)FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
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

//////////////////////////////////////////////////////////////////////////
// scene display

int WireframeShaderSorter( CphxRenderDataInstance **a, CphxRenderDataInstance **b );

TBOOL CWBSceneDisplay::MessageProc( CWBMessage &Message )
{
  CAMERADATA *cd = GetCamData();

  switch ( Message.GetMessage() )
  {
  case WBM_MOUSEWHEEL:
  {
    if ( !cd ) break;
    if ( Mode >= CAMERA_NORMAL )
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
      if ( Mode >= CAMERA_NORMAL )
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

      CphxScene_Tool *m = GetActiveWorkBench()->GetEditedScene();
      if ( !m ) return true;

      TS32 c = m->GetActiveClip();

      float timepos = 0;
      CapexSceneSplineEditor *se = (CapexSceneSplineEditor*)GetActiveWorkBench()->GetWindow( apEx_SceneSplineEditor );
      if ( se )
      {
        timepos = se->GetTimePos();
        m->UpdateSceneGraph( c, timepos );
      }

      //store current object positions here
      for ( int x = 0; x < m->GetObjectCount(); x++ )
      {
        CphxObject_Tool *o = m->GetObjectByIndex( x );
        o->BackupSplineData();

        o->_Position = D3DXVECTOR3( 0, 0, 0 );
        D3DXQuaternionIdentity( &o->_Rotation );
        o->_Scale = D3DXVECTOR3( 1, 1, 1 );

        if ( o->GetSpline( c, Spline_Position_x ) && o->GetSpline( c, Spline_Position_y ) && o->GetSpline( c, Spline_Position_z ) )
          o->_Position = D3DXVECTOR3( o->GetSpline( c, Spline_Position_x )->ValueBackup[ 0 ], o->GetSpline( c, Spline_Position_y )->ValueBackup[ 0 ], o->GetSpline( c, Spline_Position_z )->ValueBackup[ 0 ] );
        if ( o->GetSpline( c, Spline_Rotation ) )
          o->_Rotation = D3DXQUATERNION( o->GetSpline( c, Spline_Rotation )->ValueBackup );
        if ( o->GetSpline( c, Spline_Scale_x ) && o->GetSpline( c, Spline_Scale_y ) && o->GetSpline( c, Spline_Scale_z ) )
          o->_Scale = D3DXVECTOR3( o->GetSpline( c, Spline_Scale_x )->ValueBackup[ 0 ], o->GetSpline( c, Spline_Scale_y )->ValueBackup[ 0 ], o->GetSpline( c, Spline_Scale_z )->ValueBackup[ 0 ] );
      }

      if ( UberTool->IsDragged() )
      {
        DragMode = CAMDRAGMODE_UBERTOOL;
        return true;
      }

    }

    if ( !UberTool->IsDragged() || !UberToolVisible() )
    {
      CphxObject_Tool *o = MouseObject;
      CphxScene_Tool *m = GetActiveWorkBench()->GetEditedScene();
      if ( !m ) break;
      if ( !App->GetCtrlState() || !o )
      {
        for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
          m->GetObjectByIndex( x )->Selected = false;
      }

      if ( o )
        o->Selected = App->GetCtrlState() ? !o->Selected : true;
      GetActiveWorkBench()->UpdateWindows( apEx_SceneGraph );
      GetActiveWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
      SetEditedObject( o );
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
    if ( App->GetShiftState() || Mode < CAMERA_NORMAL ) DragMode = CAMDRAGMODE_PAN;
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

void CWBSceneDisplay::OnDraw( CWBDrawAPI *API )
{
  CWBItem::OnDraw( API );
  API->DrawRect( GetClientRect(), CColor::FromABGR( 0xff000000 ) );

  CapexWorkBench *wb = GetActiveWorkBench();
  if ( !wb ) return;
  if ( !wb->GetEditedScene() ) return;

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif

  UpdateRendertargetCollection();
  Project.ApplyRenderTargets( RenderTargets );

  DefaultMaterial->RequestContent();
  CphxScene_Tool *s = wb->GetEditedScene();

  if ( s && s->Scene.LayerCount )
  {
    FindViableCamera();
    UpdateSceneContent( s );

    DefaultTech->Tech->TargetLayer = s->Scene.RenderLayers[ s->Scene.LayerCount - 1 ]->Descriptor;

    TF32 timepos = 0;

    CapexSceneSplineEditor *se = (CapexSceneSplineEditor*)GetActiveWorkBench()->GetWindow( apEx_SceneSplineEditor );
    if ( se ) timepos = se->GetTimePos();

    s->UpdateSceneGraph( s->GetActiveClip(), timepos );

    CalculateMatrices();

    if ( Mode < CAMERA_NORMAL )
    {
      phxProjectionMatrix._33 = 0;
      phxProjectionMatrix._43 = 0.5;
    }

    if ( !ParticlesCalculatedInFrame )
      s->SimulateParticles();

    API->FlushDrawBuffer();
    API->SetRenderView( ClientToScreen( GetClientRect() ) );

    RenderDepthState_Normal->Apply();
    RenderRasterState->Apply();
    RenderSamplerState->Apply( CORESMP_PS0 );
    RenderSamplerState->Apply( CORESMP_VS0 );

    DrawGrid( API, s );
    InjectWireframe( s );
    InjectHelperObjects( s );

    s->KillInvalidBatches( Mode < CAMERA_NORMAL );
    s->SortRenderLayers();

    if ( Mode < CAMERA_NORMAL )
    {
      for ( int x = 0; x < s->Scene.LayerCount; x++ )
        s->Scene.RenderLayers[ x ]->Descriptor->SetEnvironment( true, true, 0 );

      s->Scene.Render( false, false, 0 );
    }
    else
      s->Render();

    ParticlesCalculatedInFrame = true;

    TS32 tricount = s->GetTriCount();
    CWBLabel *tric = (CWBLabel*)GetParent()->FindChildByID( _T( "tricount" ), _T( "label" ) );
    if ( tric )
      tric->SetText( CString::Format( _T( "Primitives: %d" ), tricount ) );
  }

  API->GetDevice()->ForceStateReset();
  API->SetUIRenderState();

  //display scene

  CphxRenderTarget_Tool *t = GetDisplayedRenderTarget();

  if ( s && s->Scene.LayerCount && t )
  {
    CCoreDX11Texture2D *dummy = new CCoreDX11Texture2D( (CCoreDX11Device*)API->GetDevice() );
    CphxRenderTarget *rt = &t->rt;// s->Scene.RenderLayers[s->Scene.LayerCount - 1]->Descriptor->Targets[0];

    CPoint off = CPoint( GetClientRect().Width() - rt->XRes, GetClientRect().Height() - rt->YRes ) / 2;
    CRect disp = CRect( 0, 0, rt->XRes, rt->YRes ) + off;

    dummy->SetView( rt->View );// s->Scene.RenderLayers[s->Scene.LayerCount - 1]->Descriptor->Targets[0]->View);

    API->FlushDrawBuffer();
    if ( Mode >= CAMERA_NORMAL ) API->GetDevice()->SetRenderState( DisableBlend );
    API->GetDevice()->SetTexture( CORESMP_PS0, dummy );
    API->GetDevice()->SetPixelShader( Root->GammaDisplayShader ); //counteract the degamma in the display
    API->DrawRect( disp, 0, 0, 1, 1 );

    API->FlushDrawBuffer();
    API->SetUIRenderState();

    dummy->SetTextureHandle( NULL );
    dummy->SetView( NULL );
    SAFEDELETE( dummy );

    DisplayUberTool( API );

    //pick mesh
    MouseObject = NULL;
    if ( !UberTool->IsPicked() )
    {
      CalculateMatrices();
      if ( MouseObject = Pick( disp ) )
      {
        App->SelectMouseCursor( CM_CROSS );
        if ( !UberTool->IsDragged() )
          SetTooltip( MouseObject->GetName() );
      }
    }

    API->GetDevice()->ForceStateReset();
    API->SetUIRenderState();

    if ( Mode == CAMERA_CAMERAVIEW && GridView )
    {
      for ( TS32 x = 0; x < 72; x++ )
      {
        TF32 dist = ( disp.Height() ) / 2.0f;

        API->DrawLine( disp.Center() + CPoint( (TS32)( dist*sin( x / 72.0f * PI * 2 ) ), (TS32)( dist*cos( x / 72.0f * PI * 2 ) ) ),
                       disp.Center() + CPoint( (TS32)( dist*sin( ( x + 1 ) / 72.0f * PI * 2 ) ), (TS32)( dist*cos( ( x + 1 ) / 72.0f * PI * 2 ) ) ),
                       CColor::FromARGB( 0x80ffffff ) );
      }


      for ( int x = 0; x < 4; x++ )
      {
        API->DrawRect( CRect( disp.TopLeft() + CPoint( disp.Width()*x / 3, 0 ), disp.BottomLeft() + CPoint( disp.Width()*x / 3 + 1, 0 ) ), CColor::FromARGB( 0xa0000000 ) );
        API->DrawRect( CRect( disp.TopLeft() + CPoint( 0, disp.Height()*x / 3 ), disp.TopRight() + CPoint( 0, disp.Height()*x / 3 + 1 ) ), CColor::FromARGB( 0xa0000000 ) );
      }

      CRect r1 = disp;
      r1.x1 += (TS32)( disp.Width()*0.05f );
      r1.x2 -= (TS32)( disp.Width()*0.05f );
      r1.y1 += (TS32)( disp.Height()*0.05f );
      r1.y2 -= (TS32)( disp.Height()*0.05f );

      API->DrawRectBorder( r1, 0xffffffff );

      CRect r2 = disp;
      r2.x1 += (TS32)( disp.Width()*0.1f );
      r2.x2 -= (TS32)( disp.Width()*0.1f );
      r2.y1 += (TS32)( disp.Height()*0.1f );
      r2.y2 -= (TS32)( disp.Height()*0.1f );

      API->DrawRectBorder( r2, 0xff000000 );

      API->DrawRect( CRect( disp.Center().x, disp.y1, disp.Center().x + 1, disp.y2 ), 0x80ffffff );
      API->DrawRect( CRect( disp.x1, disp.Center().y, disp.x2, disp.Center().y + 1 ), 0x80ffffff );
    }

    //free renderlayers
    //for (TS32 x = 0; x < s->Scene.LayerCount; x++)
    //	s->Scene.RenderLayers[x]->RenderInstances.FreeArray();

#ifdef MEMORY_TRACKING
    memTracker.SetMissingIgnore( false );
#endif
  }
}

void CWBSceneDisplay::InitCameraGrid()
{
  InitializeModelView( App->GetDevice() );
  CameraGrid = new CphxRenderDataInstance;
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
  CameraGrid->Indexed = true;
  D3DXMatrixIdentity( &CameraGrid->Matrices[ 0 ] );
  D3DXMatrixIdentity( &CameraGrid->Matrices[ 1 ] );
  for ( TS32 x = 0; x < 8; x++ )
    CameraGrid->Textures[ x ] = NULL;
}

void CWBSceneDisplay::DrawOrthoGrid( CWBDrawAPI *API )
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
  //we want the grid to split the viewport into about 8 equal areas

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

TBOOL CWBSceneDisplay::UberToolVisible()
{
  if ( Mode == CAMERA_CAMERAVIEW ) return false;
  if ( !GetActiveWorkBench()->GetEditedScene() ) return false;

  int selectedcount = 0;
  for ( int x = 0; x < GetActiveWorkBench()->GetEditedScene()->GetObjectCount(); x++ )
    if ( GetActiveWorkBench()->GetEditedScene()->GetObjectByIndex( x )->Selected )
      selectedcount++;

  return selectedcount;
}

void CWBSceneDisplay::ApplyUberToolValues()
{
  CAMERADATA *cd = GetCamData();
  if ( !cd ) return;

  if ( !GetActiveWorkBench()->GetEditedScene() ) return;
  if ( !UberTool->IsDragged() ) return;
  CPRS srt = UberTool->GetDragResult( App->GetShiftState(), Root->GetUbertoolSnap() );

  CapexSceneObjectParameters *p = (CapexSceneObjectParameters*)GetActiveWorkBench()->GetWindow( apEx_SceneObjectParameters );

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

  D3DXMATRIX _Transformation;

  D3DXVECTOR3 ubertoolpos = D3DXVECTOR3( 0, 0, 0 );
  int cnt = 0;

  TBOOL AutoKey = false;
  CapexSceneSplineEditor *se = (CapexSceneSplineEditor*)GetActiveWorkBench()->GetWindow( apEx_SceneSplineEditor );
  if ( se ) AutoKey = se->IsAutoKeySet();
  TS32 Clip = GetActiveWorkBench()->GetEditedScene()->GetActiveClip();
  TF32 Timestamp = 0;// GetActiveWorkBench()->GetScene()->GetTimestamp();
  if ( se ) Timestamp = se->GetTimePos();

  //apply transformations here
  for ( int x = 0; x < GetActiveWorkBench()->GetEditedScene()->GetObjectCount(); x++ )
    if ( GetActiveWorkBench()->GetEditedScene()->GetObjectByIndex( x )->Selected )
    {
      CphxObject_Tool *m = GetActiveWorkBench()->GetEditedScene()->GetObjectByIndex( x );

      _Transformation = m->GetMatrix_();

      D3DXMATRIX t;

      if ( !ObjectSpaceTransform )
        D3DXMatrixMultiply( &t, &_Transformation, &Transformation );
      else
        D3DXMatrixMultiply( &t, &Transformation, &_Transformation );

      D3DXMATRIX pm, pmi, to;
      D3DXMatrixIdentity( &pm );
      if ( m->GetParentObject() )
        pm = m->GetParentObject()->GetMatrix_();

      to = t;
      D3DXMatrixInverse( &pmi, NULL, &pm );
      D3DXMatrixMultiply( &t, &t, &pmi );

      D3DXVECTOR3 pos, scale;
      D3DXQUATERNION rot;

      //D3DXMatrixDecompose(&D3DXVECTOR3(&m->GetObject()->SplineResults[Spline_Scale_x]), &m->GetObject()->RotationResult, &D3DXVECTOR3(&m->GetObject()->SplineResults[Spline_Position_x]), &t);
      D3DXMatrixDecompose( &scale, &rot, &pos, &t );

      switch ( SelectedUberTool )
      {
      case UBERTOOL_MOVE:
        m->ApplySplineTransformation( Clip, Spline_Position_x, Timestamp, pos.x, AutoKey );
        m->ApplySplineTransformation( Clip, Spline_Position_y, Timestamp, pos.y, AutoKey );
        m->ApplySplineTransformation( Clip, Spline_Position_z, Timestamp, pos.z, AutoKey );
        break;
      case UBERTOOL_ROTATE:
        m->ApplySplineTransformation( Clip, Spline_Rotation, Timestamp, rot, AutoKey );
        if ( CenterTransform )
        {
          m->ApplySplineTransformation( Clip, Spline_Position_x, Timestamp, pos.x, AutoKey );
          m->ApplySplineTransformation( Clip, Spline_Position_y, Timestamp, pos.y, AutoKey );
          m->ApplySplineTransformation( Clip, Spline_Position_z, Timestamp, pos.z, AutoKey );
        }
        break;
      case UBERTOOL_SCALE:
        m->ApplySplineTransformation( Clip, Spline_Scale_x, Timestamp, scale.x, AutoKey );
        m->ApplySplineTransformation( Clip, Spline_Scale_y, Timestamp, scale.y, AutoKey );
        m->ApplySplineTransformation( Clip, Spline_Scale_z, Timestamp, scale.z, AutoKey );
        break;
      }

      D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
      D3DXVec4Transform( &v, &v, &to );
      ubertoolpos += D3DXVECTOR3( v.x, v.y, v.z );

      //m->SetMatrix(t);
      cnt++;
    }

  if ( cnt ) ubertoolpos /= (float)cnt;

  if ( CenterTransform && !ObjectSpaceTransform ) ubertoolpos = D3DXVECTOR3( 0, 0, 0 );

  UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );
  //LOG_DBG("UbertoolPos*: %f %f %f", ubertoolpos.x, ubertoolpos.y, ubertoolpos.z);
}

void CWBSceneDisplay::CalculateMatrices()
{
  CAMERADATA *cd = GetCamData();
  if ( !cd ) return;

  if ( Mode == CAMERA_CAMERAVIEW )
  {
    if ( GetActiveWorkBench()->GetEditedScene() )
    {
      if ( SceneCameraMap.HasKey( GetActiveWorkBench()->GetEditedScene()->GetGUID() ) )
      {
        CphxGUID g = SceneCameraMap[ GetActiveWorkBench()->GetEditedScene()->GetGUID() ];
        CphxObject_Tool *o = GetActiveWorkBench()->GetEditedScene()->GetObject( g );
        if ( o && o->GetObjectType() == Object_CamEye )
        {
          D3DXVECTOR3 eye = o->GetObject()->WorldPosition;
          D3DXVECTOR3 dir = D3DXVECTOR3( &o->GetObject()->SplineResults[ Spot_Direction_X ] );
          phxCameraPos = D3DXVECTOR4( eye.x, eye.y, eye.z, 1 );

          //roll camera
          D3DXVECTOR3 Up = D3DXVECTOR3( 0, 1, 0 );
          D3DXMATRIX RollMat;
          D3DXMatrixRotationAxis( &RollMat, &dir, o->GetObject()->SplineResults[ Spline_Camera_Roll ] * 3.14159265359f*2.0f );
          D3DXVECTOR4 rolledup;
          D3DXVec3Transform( &rolledup, &Up, &RollMat );
          Up = D3DXVECTOR3( rolledup );

          D3DXMatrixLookAtRH( &phxViewMatrix, &eye, &( eye + dir ), &Up );
          
          float fovYper2 = ( o->GetObject()->SplineResults[ Spline_Camera_FOV ] * 3.14159265359f / 4.0f ) / 2.0f;
          float zn = 0.01f;
          float cotFov = cos( fovYper2 ) / sin( fovYper2 );
          float t = zn / cotFov;
          float r = zn * Project.GetAspect() / cotFov;
          float xOffset = o->GetObject()->camCenterX / 127.0f * r;
          float yOffset = o->GetObject()->camCenterY / 127.0f * t;

          D3DXMatrixPerspectiveOffCenterRH( &phxProjectionMatrix, -r + xOffset, r + xOffset, -t + yOffset, t + yOffset, zn, 2000.0f );

          //D3DXMatrixPerspectiveFovRH( &phxProjectionMatrix, o->GetObject()->SplineResults[ Spline_Camera_FOV ] * 3.14159265359f / 4.0f, Project.GetAspect(), 0.01f, 2000.0f );
          return;
        }
      }
    }
  }


  D3DXMatrixLookAtRH( &phxViewMatrix, &cd->Eye, &cd->Target, &cd->Up );

  if ( Mode >= CAMERA_NORMAL )
  {
    D3DXMatrixPerspectiveFovRH( &phxProjectionMatrix, 45, GetClientRect().Width() / (TF32)GetClientRect().Height(), 0.01f, 2000.0f );
  }
  else
  {
    D3DXMatrixOrthoRH( &phxProjectionMatrix, OrthoZoom * GetClientRect().Width() / (TF32)GetClientRect().Height(), OrthoZoom, 0.1f, 2000.0f );
    //phxProjectionMatrix._33 = 0;
    //phxProjectionMatrix._43 = 0.5;
  }

  phxCameraPos = D3DXVECTOR4( cd->Eye.x, cd->Eye.y, cd->Eye.z, 1 );
}

void CWBSceneDisplay::DisplayUberTool( CWBDrawAPI *API )
{
  CAMERADATA *cd = GetCamData();
  if ( !cd ) return;
  CphxScene_Tool *Scene = GetActiveWorkBench()->GetEditedScene();

  if ( UberToolVisible() )
  {
    //UpdateUberToolColor();
    if ( !UberTool->IsDragged() )
    {
      D3DXVECTOR3 ubertoolpos = D3DXVECTOR3( 0, 0, 0 );
      int cnt = 0;

      //calculate ubertool center
      for ( int x = 0; x < Scene->GetObjectCount(); x++ )
        if ( Scene->GetObjectByIndex( x )->Selected )
        {
          D3DXMATRIX t = Scene->GetObjectByIndex( x )->GetMatrix();
          D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
          D3DXVec4Transform( &v, &v, &t );
          ubertoolpos += D3DXVECTOR3( v.x, v.y, v.z );
          cnt++;
        }

      if ( cnt ) ubertoolpos /= (float)cnt;

      D3DXMATRIX basetrans;
      D3DXMatrixIdentity( &basetrans );

      if ( ObjectSpaceTransform && GetEditedObject() )
      {
        basetrans = GetEditedObject()->GetMatrix();
        basetrans.m[ 3 ][ 0 ] = basetrans.m[ 3 ][ 1 ] = basetrans.m[ 3 ][ 2 ] = basetrans.m[ 0 ][ 3 ] = basetrans.m[ 1 ][ 3 ] = basetrans.m[ 2 ][ 3 ] = 0;
      }

      UberTool->SetBaseTransform( basetrans );

      if ( CenterTransform && !ObjectSpaceTransform ) ubertoolpos = D3DXVECTOR3( 0, 0, 0 );

      UberTool->SetPosition( cd->Eye, cd->Target, ubertoolpos );
      //LOG_DBG("UbertoolPos: %f %f %f", ubertoolpos.x, ubertoolpos.y, ubertoolpos.z);

      if ( Mode >= CAMERA_NORMAL )
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

    if ( Mode < CAMERA_NORMAL )	CalculateMatrices(); //fix ortho matrix for picking

    App->GetDevice()->Clear( false, true );

    CRect vpr = ClientToScreen( GetClientRect() );
    App->GetDevice()->SetViewport( vpr );

    if ( Mode < CAMERA_NORMAL )
      UberTool->SetScale( cd->Eye, cd->Target, OrthoZoom / 25.0f );
    else
      UberTool->SetScale( cd->Eye, cd->Target, 0.2f );

    UberTool->Pick( GetClientRect(), ScreenToClient( App->GetMousePos() ), phxProjectionMatrix, phxViewMatrix );

    D3DXMATRIX helper;
    D3DXMatrixIdentity( &helper );
    UberTool->Display( phxProjectionMatrix, phxViewMatrix, helper );

    API->GetDevice()->ForceStateReset();
    API->SetUIRenderState();
  }

}

CWBSceneDisplay::CWBSceneDisplay() : CWBItem()
{
  Mode = CAMERA_TOP;
  DragMode = CAMDRAGMODE_NONE;
  InitCameraGrid();
  ObjectSpaceTransform = false;
  CenterTransform = false;
  GridView = true;
  RenderTargets = NULL;
  MouseObject = NULL;
  CameraMesh = NULL;
  CrossMesh = NULL;
  SphereMesh = NULL;
  CubeMesh = NULL;
  CylinderMesh = NULL;
  ConeMesh = NULL;
  LineMesh = NULL;
  CameraInstance.VertexBuffer = NULL;
  CameraInstance.IndexBuffer = NULL;
  CameraInstance.WireBuffer = NULL;
  SphereInstance.VertexBuffer = NULL;
  SphereInstance.IndexBuffer = NULL;
  SphereInstance.WireBuffer = NULL;
  CubeInstance.VertexBuffer = NULL;
  CubeInstance.IndexBuffer = NULL;
  CubeInstance.WireBuffer = NULL;
  CrossInstance.VertexBuffer = NULL;
  CrossInstance.IndexBuffer = NULL;
  CrossInstance.WireBuffer = NULL;
  CylinderInstance.VertexBuffer = NULL;
  CylinderInstance.IndexBuffer = NULL;
  CylinderInstance.WireBuffer = NULL;
  ConeInstance.VertexBuffer = NULL;
  ConeInstance.IndexBuffer = NULL;
  ConeInstance.WireBuffer = NULL;
  LineInstance.VertexBuffer = NULL;
  LineInstance.IndexBuffer = NULL;
  LineInstance.WireBuffer = NULL;
  BuildDummyMeshes();
}

CWBSceneDisplay::CWBSceneDisplay( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  Mode = CAMERA_TOP;
  DragMode = CAMDRAGMODE_NONE;
  InitCameraGrid();
  SetCameraMode( CAMERA_NORMAL );
  UberTool = new CUberTool( App->GetDevice() );
  SelectUberTool( UBERTOOL_MOVE );
  ObjectSpaceTransform = false;
  CenterTransform = false;
  GridView = true;
  RenderTargets = Project.SpawnRenderTargetCollection( App->GetDevice(), Pos.Width(), Pos.Height(), false );
  MouseObject = NULL;
  CameraInstance.VertexBuffer = NULL;
  CameraInstance.IndexBuffer = NULL;
  CameraInstance.WireBuffer = NULL;
  SphereInstance.VertexBuffer = NULL;
  SphereInstance.IndexBuffer = NULL;
  SphereInstance.WireBuffer = NULL;
  CubeInstance.VertexBuffer = NULL;
  CubeInstance.IndexBuffer = NULL;
  CubeInstance.WireBuffer = NULL;
  CrossInstance.VertexBuffer = NULL;
  CrossInstance.IndexBuffer = NULL;
  CrossInstance.WireBuffer = NULL;
  CylinderInstance.VertexBuffer = NULL;
  CylinderInstance.IndexBuffer = NULL;
  CylinderInstance.WireBuffer = NULL;
  ConeInstance.VertexBuffer = NULL;
  ConeInstance.IndexBuffer = NULL;
  ConeInstance.WireBuffer = NULL;
  LineInstance.VertexBuffer = NULL;
  LineInstance.IndexBuffer = NULL;
  LineInstance.WireBuffer = NULL;
  CameraMesh = NULL;
  SphereMesh = NULL;
  CubeMesh = NULL;
  CrossMesh = NULL;
  CylinderMesh = NULL;
  ConeMesh = NULL;
  LineMesh = NULL;
  BuildDummyMeshes();
}

CWBSceneDisplay::~CWBSceneDisplay()
{
  UberTool->Gizmos.FreeArray();
  SAFEDELETE( UberTool );
  SAFEDELETE( CameraGrid );
  SAFEDELETE( RenderTargets );
  if ( CameraInstance.WireBuffer ) CameraInstance.WireBuffer->Release();
  if ( CameraInstance.VertexBuffer ) CameraInstance.VertexBuffer->Release();
  if ( CameraInstance.IndexBuffer ) CameraInstance.IndexBuffer->Release();
  if ( SphereInstance.WireBuffer ) SphereInstance.WireBuffer->Release();
  if ( SphereInstance.VertexBuffer ) SphereInstance.VertexBuffer->Release();
  if ( SphereInstance.IndexBuffer ) SphereInstance.IndexBuffer->Release();
  if ( CubeInstance.WireBuffer ) CubeInstance.WireBuffer->Release();
  if ( CubeInstance.VertexBuffer ) CubeInstance.VertexBuffer->Release();
  if ( CubeInstance.IndexBuffer ) CubeInstance.IndexBuffer->Release();
  if ( CrossInstance.WireBuffer ) CrossInstance.WireBuffer->Release();
  if ( CrossInstance.VertexBuffer ) CrossInstance.VertexBuffer->Release();
  if ( CrossInstance.IndexBuffer ) CrossInstance.IndexBuffer->Release();
  if ( CylinderInstance.WireBuffer ) CylinderInstance.WireBuffer->Release();
  if ( CylinderInstance.VertexBuffer ) CylinderInstance.VertexBuffer->Release();
  if ( CylinderInstance.IndexBuffer ) CylinderInstance.IndexBuffer->Release();
  if ( ConeInstance.WireBuffer ) ConeInstance.WireBuffer->Release();
  if ( ConeInstance.VertexBuffer ) ConeInstance.VertexBuffer->Release();
  if ( ConeInstance.IndexBuffer ) ConeInstance.IndexBuffer->Release();
  if ( LineInstance.WireBuffer ) LineInstance.WireBuffer->Release();
  if ( LineInstance.VertexBuffer ) LineInstance.VertexBuffer->Release();
  if ( LineInstance.IndexBuffer ) LineInstance.IndexBuffer->Release();

#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  SAFEDELETE( CameraMesh );
  SAFEDELETE( SphereMesh );
  SAFEDELETE( CubeMesh );
  SAFEDELETE( CrossMesh );
  SAFEDELETE( CylinderMesh );
  SAFEDELETE( ConeMesh );
  SAFEDELETE( LineMesh );
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
}

CWBItem * CWBSceneDisplay::Factory( CWBItem *RootItem, CXMLNode &node, CRect &Pos )
{
  return new CWBSceneDisplay( RootItem, Pos );
}

void CWBSceneDisplay::SetCameraMode( MODELVIEWCAMERAMODE m, CphxObject_Tool *Camera )
{
  CAMERADATA c;

  Mode = m;
  c.OrthoY = c.Up = D3DXVECTOR3( 0, 1, 0 );

  c.Up = D3DXVECTOR3( 0, 1, 0 );
  c.Target = D3DXVECTOR3( 0, 0, 0 );
  c.Eye = D3DXVECTOR3( 0, 0, -1 );

  switch ( Mode )
  {
  case CAMERA_NORMAL:
    c.Up = D3DXVECTOR3( 0, 1, 0 );
    c.Target = D3DXVECTOR3( 0, 0, 0 );
    c.Eye = D3DXVECTOR3( 0, 1, 3 );
    break;
  case CAMERA_LEFT:
    c.OrthoZ = c.Eye = D3DXVECTOR3( -4, 0, 0 );
    break;
  case CAMERA_RIGHT:
    c.OrthoZ = c.Eye = D3DXVECTOR3( 4, 0, 0 );
    break;
  case CAMERA_TOP:
    c.OrthoZ = c.Eye = D3DXVECTOR3( 0, 4, 0 );
    c.OrthoY = c.Up = D3DXVECTOR3( 0, 0, -4 );
    break;
  case CAMERA_BOTTOM:
    c.OrthoZ = c.Eye = D3DXVECTOR3( 0, -4, 0 );
    c.OrthoY = c.Up = D3DXVECTOR3( 0, 0, 4 );
    break;
  case CAMERA_FRONT:
    c.OrthoZ = c.Eye = D3DXVECTOR3( 0, 0, 4 );
    break;
  case CAMERA_BACK:
    c.OrthoZ = c.Eye = D3DXVECTOR3( 0, 0, -4 );
    break;
  case CAMERA_CAMERAVIEW:
    if ( GetActiveWorkBench()->GetEditedScene() && Camera )
      SceneCameraMap[ GetActiveWorkBench()->GetEditedScene()->GetGUID() ] = Camera->GetGUID();
    break;
  default:
    break;
  }

  D3DXVec3Normalize( &c.OrthoY, &c.OrthoY );
  D3DXVec3Normalize( &c.OrthoZ, &c.OrthoZ );

  if ( Mode < CAMERA_NORMAL )
    UberTool->SetOrthoZ( c.OrthoZ );

  D3DXVec3Cross( &c.OrthoX, &c.OrthoZ, &c.OrthoY );
  OrthoZoom = 7.9f; //so the ortho grid has both colors of lines

  if ( GetActiveWorkBench()->GetEditedScene() )
    CameraData[ GetActiveWorkBench()->GetEditedScene()->GetGUID() ] = c;
}

void CWBSceneDisplay::SelectUberTool( UBERTOOLTYPE Tool )
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

CphxObject_Tool * CWBSceneDisplay::Pick( CRect ClientRect )
{
  if ( !MouseOver() ) return NULL;
  if ( !GetActiveWorkBench()->GetEditedScene() ) return NULL;

  CphxScene *Scene = &GetActiveWorkBench()->GetEditedScene()->Scene;
  CphxScene_Tool *m = GetActiveWorkBench()->GetEditedScene();

  CphxObject_Tool *picked = NULL;

  float mt;

  CphxRenderTarget_Tool *rt = GetDisplayedRenderTarget();

  for ( int x = 0; x < m->GetObjectCount(); x++ )
  {
    int cnt = 0;

    D3DXMATRIX helpermat;
    D3DXMatrixIdentity( &helpermat );

    PHXOBJECT o = m->GetObjectByIndex( x )->GetObjectType();
    if ( Mode >= CAMERA_NORMAL && o != Object_Model && o != Object_SubScene ) continue;

    switch ( o )
    {
    case Object_CamEye:
    case Object_Dummy:
    case Object_Light:
    {
      D3DXVECTOR3 zd = D3DXVECTOR3( &m->GetObjectByIndex( x )->GetObject()->SplineResults[ Spot_Direction_X ] );

      if ( zd.x == 0 && zd.y == 0 && zd.z == 0 ) //default direction
        zd = D3DXVECTOR3( 0, 0, -1 );

      cnt = 1;
      helpermat = GetHelperTransformationMatrix( zd );
    }
    break;
    default:
      break;
    }

    for ( int y = 0; y < Scene->LayerCount && !cnt; y++ )
      for ( int z = 0; z < Scene->RenderLayers[ y ]->RenderInstances.NumItems() && !cnt; z++ )
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
      D3DXMATRIX mx = m->GetObjectByIndex( x )->GetMatrix();

      switch ( m->GetObjectByIndex( x )->GetObjectType() )
      {
      case Object_CamEye:
      case Object_Dummy:
      case Object_Light:
        for ( int x = 0; x < 3; x++ )
          for ( int y = 0; y < 3; y++ )
            mx.m[ x ][ y ] = x == y ? 1.0f : 0.0f;
        break;
      }

      D3DXMatrixMultiply( &mx, &helpermat, &mx );
      if ( m->GetObjectByIndex( x )->Pick( ClientRect, ScreenToClient( App->GetMousePos() ), phxProjectionMatrix, phxViewMatrix, mx, t ) )
        if ( !picked || t < mt )
        {
          mt = t;
          picked = m->GetObjectByIndex( x );
        }
    }
  }
  return picked;
}

void CWBSceneDisplay::SetEditedObject( CphxObject_Tool *d )
{
  CapexSceneObjectParameters *p = (CapexSceneObjectParameters *)GetActiveWorkBench()->GetWindow( apEx_SceneObjectParameters );
  if ( !p ) return;
  p->SelectSceneObject( d );
}

CphxObject_Tool * CWBSceneDisplay::GetEditedObject()
{
  if ( !GetActiveWorkBench()->GetEditedScene() ) return NULL;
  CphxScene_Tool *m = GetActiveWorkBench()->GetEditedScene();
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
    if ( m->GetObjectByIndex( x )->Selected )
      return m->GetObjectByIndex( x );
  return NULL;
}

void CWBSceneDisplay::UpdateRendertargetCollection()
{
  RenderTargets->UpdateCollection( GetClientRect().Width(), GetClientRect().Height(), Mode > CAMERA_NORMAL );
}

void CWBSceneDisplay::BuildDummyMeshes()
{
  SAFEDELETE( CameraMesh );
  SAFEDELETE( SphereMesh );
  SAFEDELETE( CubeMesh );

#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  CameraMesh = new CphxMesh();
  float fx = 1.6757f, fy = 1, fz = 0.5492f;
  float xx = 0, xy = 0, xz = 0;
  int vxbase = 0;
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, -0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, -0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, 0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, 0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, -0.5f*fy + xy, 0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, 0.5f*fy + xy, 0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, 0.5f*fy + xy, 0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, -0.5f*fy + xy, 0.5f*fz + xz ) );

  CameraMesh->AddPolygon( vxbase + 0, vxbase + 1, vxbase + 1 );
  CameraMesh->AddPolygon( vxbase + 1, vxbase + 2, vxbase + 2 );
  CameraMesh->AddPolygon( vxbase + 2, vxbase + 3, vxbase + 3 );
  CameraMesh->AddPolygon( vxbase + 3, vxbase + 0, vxbase + 0 );

  CameraMesh->AddPolygon( vxbase + 4, vxbase + 5, vxbase + 5 );
  CameraMesh->AddPolygon( vxbase + 5, vxbase + 6, vxbase + 6 );
  CameraMesh->AddPolygon( vxbase + 6, vxbase + 7, vxbase + 7 );
  CameraMesh->AddPolygon( vxbase + 7, vxbase + 4, vxbase + 4 );

  CameraMesh->AddPolygon( vxbase + 0, vxbase + 4, vxbase + 4 );
  CameraMesh->AddPolygon( vxbase + 1, vxbase + 7, vxbase + 7 );
  CameraMesh->AddPolygon( vxbase + 2, vxbase + 6, vxbase + 6 );
  CameraMesh->AddPolygon( vxbase + 3, vxbase + 5, vxbase + 5 );

  fx = 0.4299f;
  fy = 0.4978f;
  fz = 0.4130f;
  xx = 1.053f;
  xy = 0;
  xz = 0;
  vxbase = 8;
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, -0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, -0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, 0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, 0.5f*fy + xy, -0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, -0.5f*fy + xy, 0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( -0.5f*fx + xx, 0.5f*fy + xy, 0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, 0.5f*fy + xy, 0.5f*fz + xz ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*fx + xx, -0.5f*fy + xy, 0.5f*fz + xz ) );

  CameraMesh->AddPolygon( vxbase + 0, vxbase + 1, vxbase + 1 );
  CameraMesh->AddPolygon( vxbase + 1, vxbase + 2, vxbase + 2 );
  CameraMesh->AddPolygon( vxbase + 2, vxbase + 3, vxbase + 3 );
  CameraMesh->AddPolygon( vxbase + 3, vxbase + 0, vxbase + 0 );

  CameraMesh->AddPolygon( vxbase + 4, vxbase + 5, vxbase + 5 );
  CameraMesh->AddPolygon( vxbase + 5, vxbase + 6, vxbase + 6 );
  CameraMesh->AddPolygon( vxbase + 6, vxbase + 7, vxbase + 7 );
  CameraMesh->AddPolygon( vxbase + 7, vxbase + 4, vxbase + 4 );

  CameraMesh->AddPolygon( vxbase + 0, vxbase + 4, vxbase + 4 );
  CameraMesh->AddPolygon( vxbase + 1, vxbase + 7, vxbase + 7 );
  CameraMesh->AddPolygon( vxbase + 2, vxbase + 6, vxbase + 6 );
  CameraMesh->AddPolygon( vxbase + 3, vxbase + 5, vxbase + 5 );

  vxbase = 16;
  int spres = 12;
  fx = 1;
  fy = 1;
  fz = 0.3307f;
  xx = -0.4994f;
  xy = 0.9848f;
  xz = 0;

  for ( int x = 0; x < spres; x++ )
    CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*sin( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xx, 0.5f*cos( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xy, 0.5f*fz + xz ) );
  for ( int x = 0; x < spres; x++ )
    CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*sin( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xx, 0.5f*cos( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xy, -0.5f*fz + xz ) );

  for ( int x = 0; x < spres; x++ )
  {
    CameraMesh->AddPolygon( x + vxbase, ( x + 1 ) % spres + vxbase, x + vxbase );
    CameraMesh->AddPolygon( x + spres + vxbase, ( x + 1 ) % spres + spres + vxbase, x + spres + vxbase );
    CameraMesh->AddPolygon( x + vxbase, x + spres + vxbase, x + vxbase );
  }

  vxbase += spres * 2;
  xx = -0.4994f + 0.9630f;

  for ( int x = 0; x < spres; x++ )
    CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*sin( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xx, 0.5f*cos( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xy, 0.5f*fz + xz ) );
  for ( int x = 0; x < spres; x++ )
    CameraMesh->AddVertex( D3DXVECTOR3( 0.5f*sin( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xx, 0.5f*cos( ( ( x + 0.5f ) / (float)spres )*PI * 2 ) + xy, -0.5f*fz + xz ) );

  for ( int x = 0; x < spres; x++ )
  {
    CameraMesh->AddPolygon( x + vxbase, ( x + 1 ) % spres + vxbase, x + vxbase );
    CameraMesh->AddPolygon( x + spres + vxbase, ( x + 1 ) % spres + spres + vxbase, x + spres + vxbase );
    CameraMesh->AddPolygon( x + vxbase, x + spres + vxbase, x + vxbase );
  }

  vxbase += spres * 2;

  CameraMesh->AddVertex( D3DXVECTOR3( 0.838648f, -0.000483f, 0 ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 1.672967f, -0.485900f, 0.487443f ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 1.672967f, 0.485935f, 0.487444f ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 1.672967f, 0.484938f, -0.487442f ) );
  CameraMesh->AddVertex( D3DXVECTOR3( 1.672967f, -0.485937f, -0.487445f ) );

  CameraMesh->AddPolygon( vxbase + 0, vxbase + 1, vxbase + 1 );
  CameraMesh->AddPolygon( vxbase + 0, vxbase + 2, vxbase + 2 );
  CameraMesh->AddPolygon( vxbase + 0, vxbase + 3, vxbase + 3 );
  CameraMesh->AddPolygon( vxbase + 0, vxbase + 4, vxbase + 4 );

  CameraMesh->AddPolygon( vxbase + 1, vxbase + 2, vxbase + 2 );
  CameraMesh->AddPolygon( vxbase + 2, vxbase + 3, vxbase + 3 );
  CameraMesh->AddPolygon( vxbase + 3, vxbase + 4, vxbase + 4 );
  CameraMesh->AddPolygon( vxbase + 4, vxbase + 1, vxbase + 1 );

  for ( TS32 x = 0; x < CameraMesh->Vertices.NumItems(); x++ )
  {
    D3DXVECTOR3 p = CameraMesh->Vertices[ x ].Position;
    CameraMesh->Vertices[ x ].Position = D3DXVECTOR3( p.z / 5.0f, p.y / 5.0f, p.x / 5.0f );
  }

  TS32 VxCount = 0;
  TS32 TriCount = 0;
  TS32 EdgeCount = 0;
  CameraMesh->BuildMesh( CameraInstance.VertexBuffer, CameraInstance.IndexBuffer, CameraInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  CameraInstance.TriIndexCount = 0;
  CameraInstance.WireIndexCount = EdgeCount * 2;
  CameraInstance.Wireframe = true;
  CameraInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  CameraInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  CameraInstance.GS = NULL;
  CameraInstance.HS = NULL;
  CameraInstance.DS = NULL;
  CameraInstance.ToolData = NULL;
  CameraInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  CameraInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  CameraInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( CameraInstance.Textures, 0, sizeof( CameraInstance.Textures ) );

  //cube

  CubeMesh = new CphxMesh();
  CubeMesh->AddVertex( D3DXVECTOR3( -0.5f, -0.5f, -0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( 0.5f, -0.5f, -0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( 0.5f, 0.5f, -0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( -0.5f, 0.5f, -0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( -0.5f, -0.5f, 0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( -0.5f, 0.5f, 0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( 0.5f, 0.5f, 0.5f ) );
  CubeMesh->AddVertex( D3DXVECTOR3( 0.5f, -0.5f, 0.5f ) );

  CubeMesh->AddPolygon( 0, 1, 1 );
  CubeMesh->AddPolygon( 1, 2, 2 );
  CubeMesh->AddPolygon( 2, 3, 3 );
  CubeMesh->AddPolygon( 3, 0, 0 );

  CubeMesh->AddPolygon( 4, 5, 5 );
  CubeMesh->AddPolygon( 5, 6, 6 );
  CubeMesh->AddPolygon( 6, 7, 7 );
  CubeMesh->AddPolygon( 7, 4, 4 );

  CubeMesh->AddPolygon( 0, 4, 4 );
  CubeMesh->AddPolygon( 1, 7, 7 );
  CubeMesh->AddPolygon( 2, 6, 6 );
  CubeMesh->AddPolygon( 3, 5, 5 );

  CubeMesh->BuildMesh( CubeInstance.VertexBuffer, CubeInstance.IndexBuffer, CubeInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  CubeInstance.TriIndexCount = 0;
  CubeInstance.WireIndexCount = EdgeCount * 2;
  CubeInstance.Wireframe = true;
  CubeInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  CubeInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  CubeInstance.GS = NULL;
  CubeInstance.HS = NULL;
  CubeInstance.DS = NULL;
  CubeInstance.ToolData = NULL;
  CubeInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  CubeInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  CubeInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( CubeInstance.Textures, 0, sizeof( CubeInstance.Textures ) );



  spres = 96;

  SphereMesh = new CphxMesh();

  for ( int x = 0; x < spres; x++ )
    SphereMesh->AddVertex( D3DXVECTOR3( sin( x / (float)spres*PI * 2 ), cos( x / (float)spres*PI * 2 ), 0 )*0.125f );
  for ( int x = 0; x < spres; x++ )
    SphereMesh->AddVertex( D3DXVECTOR3( sin( x / (float)spres*PI * 2 ), 0, cos( x / (float)spres*PI * 2 ) )*0.125f );
  for ( int x = 0; x < spres; x++ )
    SphereMesh->AddVertex( D3DXVECTOR3( 0, sin( x / (float)spres*PI * 2 ), cos( x / (float)spres*PI * 2 ) )*0.125f );

  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < spres; x++ )
      SphereMesh->AddPolygon( x + y*spres, ( x + 1 ) % spres + y*spres, x + y*spres );

  SphereMesh->BuildMesh( SphereInstance.VertexBuffer, SphereInstance.IndexBuffer, SphereInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  SphereInstance.TriIndexCount = 0;
  SphereInstance.WireIndexCount = EdgeCount * 2;
  SphereInstance.Wireframe = true;
  SphereInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  SphereInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  SphereInstance.GS = NULL;
  SphereInstance.HS = NULL;
  SphereInstance.DS = NULL;
  SphereInstance.ToolData = NULL;
  SphereInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  SphereInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  SphereInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( SphereInstance.Textures, 0, sizeof( SphereInstance.Textures ) );

  CylinderMesh = new CphxMesh();

  for ( int x = 0; x < spres; x++ )
    CylinderMesh->AddVertex( D3DXVECTOR3( 0.5f*sin( x / (float)spres*PI * 2 ), 0.5f, 0.5f*cos( x / (float)spres*PI * 2 ) ) );
  for ( int x = 0; x < spres; x++ )
    CylinderMesh->AddVertex( D3DXVECTOR3( 0.5f*sin( x / (float)spres*PI * 2 ), -0.5f, 0.5f*cos( x / (float)spres*PI * 2 ) ) );

  for ( int x = 0; x < spres; x++ )
  {
    CylinderMesh->AddPolygon( x, ( x + 1 ) % spres, x );
    CylinderMesh->AddPolygon( x + spres, ( x + 1 ) % spres + spres, x + spres );
    if ( floor( x / 8.0f ) - x / 8.0f == 0 )
      CylinderMesh->AddPolygon( x, x + spres, x );
  }

  CylinderMesh->BuildMesh( CylinderInstance.VertexBuffer, CylinderInstance.IndexBuffer, CylinderInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  CylinderInstance.TriIndexCount = 0;
  CylinderInstance.WireIndexCount = EdgeCount * 2;
  CylinderInstance.Wireframe = true;
  CylinderInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  CylinderInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  CylinderInstance.GS = NULL;
  CylinderInstance.HS = NULL;
  CylinderInstance.DS = NULL;
  CylinderInstance.ToolData = NULL;
  CylinderInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  CylinderInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  CylinderInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( CylinderInstance.Textures, 0, sizeof( CylinderInstance.Textures ) );

  ConeMesh = new CphxMesh();

  for ( int x = 0; x < spres; x++ )
    ConeMesh->AddVertex( D3DXVECTOR3( sin( x / (float)spres*PI * 2 ), cos( x / (float)spres*PI * 2 ), 1 ) );

  ConeMesh->AddVertex( D3DXVECTOR3( 0, 0, 0 ) );

  for ( int x = 0; x < spres; x++ )
  {
    ConeMesh->AddPolygon( x, ( x + 1 ) % spres, x );

    if ( floor( x / 24.0f ) - x / 24.0f == 0 )
      ConeMesh->AddPolygon( x, spres, x );
  }

  ConeMesh->BuildMesh( ConeInstance.VertexBuffer, ConeInstance.IndexBuffer, ConeInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  ConeInstance.TriIndexCount = 0;
  ConeInstance.WireIndexCount = EdgeCount * 2;
  ConeInstance.Wireframe = true;
  ConeInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  ConeInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  ConeInstance.GS = NULL;
  ConeInstance.HS = NULL;
  ConeInstance.DS = NULL;
  ConeInstance.ToolData = NULL;
  ConeInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  ConeInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  ConeInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( ConeInstance.Textures, 0, sizeof( ConeInstance.Textures ) );

  LineMesh = new CphxMesh();

  LineMesh->AddVertex( D3DXVECTOR3( 0, 0, 0 ) );
  LineMesh->AddVertex( D3DXVECTOR3( 0, 0, 1 ) );

  LineMesh->AddPolygon( 0, 1, 1 );

  LineMesh->BuildMesh( LineInstance.VertexBuffer, LineInstance.IndexBuffer, LineInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  LineInstance.TriIndexCount = 0;
  LineInstance.WireIndexCount = EdgeCount * 2;
  LineInstance.Wireframe = true;
  LineInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  LineInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  LineInstance.GS = NULL;
  LineInstance.HS = NULL;
  LineInstance.DS = NULL;
  LineInstance.ToolData = NULL;
  LineInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  LineInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  LineInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( LineInstance.Textures, 0, sizeof( LineInstance.Textures ) );


  CrossMesh = new CphxMesh();

  CrossMesh->AddVertex( D3DXVECTOR3( -0.5f, -0.5f, -0.5f )*0.25f );
  CrossMesh->AddVertex( D3DXVECTOR3( 0.5f, -0.5f, -0.5f )*0.25f );
  CrossMesh->AddVertex( D3DXVECTOR3( 0.5f, 0.5f, -0.5f )*0.25f );
  CrossMesh->AddVertex( D3DXVECTOR3( -0.5f, 0.5f, -0.5f )*0.25f );

  CrossMesh->AddVertex( D3DXVECTOR3( -0.5f, -0.5f, 0.5f )*0.25f );
  CrossMesh->AddVertex( D3DXVECTOR3( -0.5f, 0.5f, 0.5f )*0.25f );
  CrossMesh->AddVertex( D3DXVECTOR3( 0.5f, 0.5f, 0.5f )*0.25f );
  CrossMesh->AddVertex( D3DXVECTOR3( 0.5f, -0.5f, 0.5f )*0.25f );

  CrossMesh->AddPolygon( 0, 1, 1 );
  CrossMesh->AddPolygon( 1, 2, 2 );
  CrossMesh->AddPolygon( 2, 3, 3 );
  CrossMesh->AddPolygon( 3, 0, 0 );

  CrossMesh->AddPolygon( 4, 5, 5 );
  CrossMesh->AddPolygon( 5, 6, 6 );
  CrossMesh->AddPolygon( 6, 7, 7 );
  CrossMesh->AddPolygon( 7, 4, 4 );

  CrossMesh->AddPolygon( 0, 4, 4 );
  CrossMesh->AddPolygon( 1, 7, 7 );
  CrossMesh->AddPolygon( 2, 6, 6 );
  CrossMesh->AddPolygon( 3, 5, 5 );

  CrossMesh->BuildMesh( CrossInstance.VertexBuffer, CrossInstance.IndexBuffer, CrossInstance.WireBuffer, VxCount, TriCount, EdgeCount );
  CrossInstance.TriIndexCount = 0;
  CrossInstance.WireIndexCount = EdgeCount * 2;
  CrossInstance.Wireframe = true;
  CrossInstance.PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
  CrossInstance.VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
  CrossInstance.GS = NULL;
  CrossInstance.HS = NULL;
  CrossInstance.DS = NULL;
  CrossInstance.ToolData = NULL;
  CrossInstance.DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Normal->GetHandle();
  CrossInstance.BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
  CrossInstance.RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
  memset( CrossInstance.Textures, 0, sizeof( CrossInstance.Textures ) );


#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
}

void CWBSceneDisplay::FindViableCamera()
{
  //if we're in camera view mode but no camera has been set, try to find one
  //first looks for cameras that have targets set and sets the first one
  //if that fails it sets the first one it can find

  CphxScene_Tool *s = GetActiveWorkBench()->GetEditedScene();
  if ( !s ) return;
  if ( Mode == CAMERA_CAMERAVIEW )
  {
    if ( !SceneCameraMap.HasKey( s->GetGUID() ) )
    {
      for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
        if ( s->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
        {
          if ( s->GetObjectByIndex( x )->TargetObject )
          {
            SetCameraMode( CAMERA_CAMERAVIEW, s->GetObjectByIndex( x ) );
            return;
          }
        }
      for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
        if ( s->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
        {
          SetCameraMode( CAMERA_CAMERAVIEW, s->GetObjectByIndex( x ) );
          return;
        }
    }
  }
}

void CWBSceneDisplay::InjectHelperObjects( CphxScene_Tool *s )
{
  if ( !HelperView ) return;
  if ( Mode != CAMERA_CAMERAVIEW ) //don't draw these in the camera preview
    for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
      for ( TS32 y = 0; y < s->Scene.LayerCount; y++ )
      {
        if ( s->Scene.RenderLayers[ y ]->Descriptor->VoxelizerLayer )
          continue;

        if ( s->Scene.RenderLayers[ y ]->Descriptor->IgnoreHelperObjects )
          continue;

        CphxObject_Tool *c = s->GetObjectByIndex( x );

        //calculate rotation matrix for those that need it
        D3DXVECTOR3 zd = D3DXVECTOR3( &c->GetObject()->SplineResults[ Spot_Direction_X ] );

        if ( zd.x == 0 && zd.y == 0 && zd.z == 0 ) //default direction
          zd = D3DXVECTOR3( 0, 0, -1 );

        D3DXMATRIX mx = GetHelperTransformationMatrix( zd );

        switch ( s->GetObjectByIndex( x )->GetObjectType() )
        {
        case Object_CamEye:
        {
#ifdef MEMORY_TRACKING
          memTracker.Pause();
#endif
          CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
          memTracker.Resume();
#endif
          *i = CameraInstance;
          i->Matrices[ 0 ] = c->GetMatrix();
          for ( TS32 a = 0; a < 3; a++ )
            for ( TS32 b = 0; b < 3; b++ )
              i->Matrices[ 0 ].m[ a ][ b ] = a == b ? 1.0f : 0.0f;
          D3DXMatrixMultiply( &i->Matrices[ 0 ], &mx, &i->Matrices[ 0 ] );
          i->MaterialData[ 0 ] = 0;
          i->MaterialData[ 1 ] = 204 / 255.0f;
          i->MaterialData[ 2 ] = 122 / 255.0f;
          i->MaterialData[ 3 ] = 1;
          i->DS = NULL;
          i->GS = NULL;
          i->HS = NULL;
          i->Indexed = true;


          s->Scene.RenderLayers[ y ]->RenderInstances.Add( i );

          if ( c->TargetObject )
            InjectTargetLine( s, y, c, c->TargetObject, 0, 204 / 255.0f, 122 / 255.0f, 1 );

        }
        break;
        case Object_ParticleEmitterCPU:
        case Object_ParticleEmitter:
        {
#ifdef MEMORY_TRACKING
          memTracker.Pause();
#endif
          CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
          memTracker.Resume();
#endif
          D3DXMatrixIdentity( &mx );

          if ( ( (CphxObject_ParticleEmitter_CPU*)s->GetObjectByIndex( x )->GetObject() )->EmitterType == 0 )
          {
            *i = CubeInstance;
            i->Matrices[ 0 ] = c->GetMatrix();
          }
          else
          {
            *i = SphereInstance;
            i->Matrices[ 0 ] = c->GetMatrix();

            D3DXMATRIX sc;
            D3DXMatrixScaling( &sc, 4, 4, 4 );
            D3DXMatrixMultiply( &i->Matrices[ 0 ], &sc, &i->Matrices[ 0 ] );
          }

          D3DXMatrixMultiply( &i->Matrices[ 0 ], &mx, &i->Matrices[ 0 ] );
          i->MaterialData[ 0 ] = 0;
          i->MaterialData[ 1 ] = 1;
          i->MaterialData[ 2 ] = 0;
          i->MaterialData[ 3 ] = 1;
          i->DS = NULL;
          i->GS = NULL;
          i->HS = NULL;
          i->Indexed = true;


          s->Scene.RenderLayers[ y ]->RenderInstances.Add( i );

          //if (c->TargetObject)
          //	InjectTargetLine(s, y, c, c->TargetObject, 0, 204 / 255.0f, 122 / 255.0f, 1);

        }
        break;
        case Object_ParticleGravity:
        case Object_ParticleDrag:
        case Object_ParticleTurbulence:
        case Object_ParticleVortex:
        case Object_LogicObject:
        {
          CphxObject_ParticleAffector *obj = (CphxObject_ParticleAffector*)s->GetObjectByIndex( x )->GetObject();

#ifdef MEMORY_TRACKING
          memTracker.Pause();
#endif
          CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
          memTracker.Resume();
#endif
          D3DXMatrixIdentity( &mx );

          if ( obj->AreaType || s->GetObjectByIndex( x )->GetObjectType() == Object_LogicObject )
            *i = CubeInstance;
          else
            *i = SphereInstance;


          i->Matrices[ 0 ] = c->GetMatrix();

          D3DXMatrixMultiply( &i->Matrices[ 0 ], &mx, &i->Matrices[ 0 ] );
          if ( s->GetObjectByIndex( x )->GetObjectType() != Object_LogicObject )
          {
            i->MaterialData[ 0 ] = 0.5;
            i->MaterialData[ 1 ] = 0.5;
            i->MaterialData[ 2 ] = 1;
            i->MaterialData[ 3 ] = 1;
          }
          else
          {
            i->MaterialData[ 0 ] = 1;
            i->MaterialData[ 1 ] = 0;
            i->MaterialData[ 2 ] = 0;
            i->MaterialData[ 3 ] = 1;
          }
          i->DS = NULL;
          i->GS = NULL;
          i->HS = NULL;
          i->Indexed = true;

          s->Scene.RenderLayers[ y ]->RenderInstances.Add( i );

          //if (c->TargetObject)
          //	InjectTargetLine(s, y, c, c->TargetObject, 0, 204 / 255.0f, 122 / 255.0f, 1);

        }
        break;
        case Object_Dummy:
        {
#ifdef MEMORY_TRACKING
          memTracker.Pause();
#endif
          CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
          memTracker.Resume();
#endif
          *i = SphereInstance;
          i->Matrices[ 0 ] = c->GetMatrix();
          for ( TS32 a = 0; a < 3; a++ )
            for ( TS32 b = 0; b < 3; b++ )
              i->Matrices[ 0 ].m[ a ][ b ] = a == b ? 1.0f : 0.0f;
          D3DXMatrixMultiply( &i->Matrices[ 0 ], &mx, &i->Matrices[ 0 ] );
          //i->Matrices[0].m[0][0] *= OrthoZoom / 20.0f;
          //i->Matrices[0].m[1][1] *= OrthoZoom / 20.0f;
          //i->Matrices[0].m[2][2] *= OrthoZoom / 20.0f;

          i->MaterialData[ 0 ] = 140 / 255.0f;
          i->MaterialData[ 1 ] = 140 / 255.0f;
          i->MaterialData[ 2 ] = 140 / 255.0f;
          i->MaterialData[ 3 ] = 1;
          i->DS = NULL;
          i->GS = NULL;
          i->HS = NULL;
          i->Indexed = true;

          int cnt = 0;

          for ( TS32 z = 0; z < s->GetObjectCount(); z++ )
            if ( s->GetObjectByIndex( z )->TargetObject == c )
            {
              cnt++;
              switch ( s->GetObjectByIndex( z )->GetObjectType() )
              {
              case Object_CamEye:
                i->MaterialData[ 0 ] = 0;
                i->MaterialData[ 1 ] = 204 / 255.0f;
                i->MaterialData[ 2 ] = 122 / 255.0f;
                i->MaterialData[ 3 ] = 1;
                break;
              case Object_Light:
                i->MaterialData[ 0 ] = 1;
                i->MaterialData[ 1 ] = 1;
                i->MaterialData[ 2 ] = 0;
                i->MaterialData[ 3 ] = 1;
                break;
              }
            }

          if ( cnt > 1 )
          {
            i->MaterialData[ 0 ] = 0;
            i->MaterialData[ 1 ] = 1;
            i->MaterialData[ 2 ] = 1;
            i->MaterialData[ 3 ] = 1;
          }

          s->Scene.RenderLayers[ y ]->RenderInstances.Add( i );
        }
        break;

        case Object_Light:
        {
#ifdef MEMORY_TRACKING
          memTracker.Pause();
#endif
          CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
          memTracker.Resume();
#endif
          CphxObject_Light_Tool *l = (CphxObject_Light_Tool*)c;

          *i = CrossInstance;
          i->DS = NULL;
          i->GS = NULL;
          i->HS = NULL;
          i->Indexed = true;

          D3DXMATRIX m = c->GetMatrix();
          for ( TS32 a = 0; a < 3; a++ )
            for ( TS32 b = 0; b < 3; b++ )
              m.m[ a ][ b ] = a == b ? 1.0f : 0.0f;
          D3DXMatrixMultiply( &m, &mx, &m );
          //m.m[0][0] *= OrthoZoom / 20.0f;
          //m.m[1][1] *= OrthoZoom / 20.0f;
          //m.m[2][2] *= OrthoZoom / 20.0f;

          if ( l->IsPointLight() )
          {
            *i = SphereInstance;

            if ( l->TargetObject )
            {
              if ( l->GetObject()->SplineResults[ Spline_Light_Cutoff ] > 0 )
              {
                *i = ConeInstance;
                D3DXMatrixMultiply( &m, &mx, &c->GetMatrix() );

                float h = 1 / l->GetObject()->SplineResults[ Spline_Light_Cutoff ];
                float conescale = sqrtf( h*h - 1 );

                D3DXMATRIX scale;
                D3DXMatrixIdentity( &scale );
                scale.m[ 0 ][ 0 ] *= conescale*0.5f;
                scale.m[ 1 ][ 1 ] *= conescale*0.5f;
                scale.m[ 2 ][ 2 ] *= 1 * 0.5f;

                D3DXMatrixMultiply( &m, &scale, &m );

                if ( c->TargetObject )
                  InjectTargetLine( s, y, c, c->TargetObject, 1, 1, 0, 1 );

              }
            }
          }
          else
          {
            InjectTargetLine( s, y, c, l->GetParentObject(), 1, 1, 0, 1 );
            float width = l->GetObject()->SplineResults[ Spline_Light_OrthoX ] * 0.5f;
            float height = l->GetObject()->SplineResults[ Spline_Light_OrthoY ] * 0.5f;

            if ( width > 0 && height > 0 && l->GetParentObject() )
            {

              CVector3 p = CVector3( c->GetObject()->WorldPosition );
              CVector3 dir = CVector3( l->GetParentObject()->GetObject()->WorldPosition - c->GetObject()->WorldPosition ).Normalized();
              CVector3 up = CVector3( 0, 1, 0 );
              CVector3 wid = ( dir % up ).Normalized();
              CVector3 hei = ( wid % dir ).Normalized();
              CVector3 vs[ 8 ];
              vs[ 0 ] = p + wid*width + hei*height;
              vs[ 1 ] = p - wid*width + hei*height;
              vs[ 2 ] = p - wid*width - hei*height;
              vs[ 3 ] = p + wid*width - hei*height;
              InjectLine( s, y, vs[ 0 ], vs[ 0 ] + dir*100.0, 1.0f, 1.0f, 0.2f, 1 );
              InjectLine( s, y, vs[ 1 ], vs[ 1 ] + dir*100.0, 1.0f, 1.0f, 0.2f, 1 );
              InjectLine( s, y, vs[ 2 ], vs[ 2 ] + dir*100.0, 1.0f, 1.0f, 0.2f, 1 );
              InjectLine( s, y, vs[ 3 ], vs[ 3 ] + dir*100.0, 1.0f, 1.0f, 0.2f, 1 );

              InjectLine( s, y, vs[ 0 ], vs[ 1 ], 1.0f, 1.0f, 0.2f, 1 );
              InjectLine( s, y, vs[ 1 ], vs[ 2 ], 1.0f, 1.0f, 0.2f, 1 );
              InjectLine( s, y, vs[ 2 ], vs[ 3 ], 1.0f, 1.0f, 0.2f, 1 );
              InjectLine( s, y, vs[ 3 ], vs[ 0 ], 1.0f, 1.0f, 0.2f, 1 );
            }
          }

          i->Matrices[ 0 ] = m;
          i->MaterialData[ 0 ] = 1;
          i->MaterialData[ 1 ] = 1;
          i->MaterialData[ 2 ] = 0;
          i->MaterialData[ 3 ] = 1;

          s->Scene.RenderLayers[ y ]->RenderInstances.Add( i );
        }
        break;
        }
      }
}

void CWBSceneDisplay::UpdateSceneContent( CphxScene_Tool *s )
{
  if ( !s ) return;
  s->ForceUpdateContent();
  //s->RequestContent();
  //for (TS32 x = 0; x < s->GetObjectCount(); x++)
  //{
  //	s->GetObjectByIndex(x)->RequestContent();
  //	if (s->GetObjectByIndex(x)->GetObjectType() == Object_Model)
  //	{
  //		CphxObject_Model_Tool* o = (CphxObject_Model_Tool*)s->GetObjectByIndex(x);
  //		o->UpdateModel();
  //	}
  //}
  //s->SetKeyframerMode();
}

void CWBSceneDisplay::InjectWireframe( CphxScene_Tool *s )
{
  if ( SolidView )
    return;

  if ( Mode < CAMERA_NORMAL )
  {
    //change to wireframe
    for ( TS32 x = 0; x < s->Scene.LayerCount; x++ )
    {
      if ( s->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
        continue;

      for ( TS32 y = 0; y < s->Scene.RenderLayers[ x ]->RenderInstances.NumItems(); y++ )
      {
        CphxRenderDataInstance *inst = s->Scene.RenderLayers[ x ]->RenderInstances[ y ];
        inst->Indexed = true;

        inst->Wireframe = true;
        if ( !inst->VS ) inst->VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
        inst->VS = (ID3D11VertexShader*)GridVertexShader->GetHandle();
        inst->GS = NULL;
        inst->PS = (ID3D11PixelShader*)WirePixelShader->GetHandle();
        inst->MaterialData[ 0 ] = 140 / 255.0f;
        inst->MaterialData[ 1 ] = 140 / 255.0f;
        inst->MaterialData[ 2 ] = 140 / 255.0f;
        inst->MaterialData[ 3 ] = -10;
        inst->DepthStencilState = (ID3D11DepthStencilState *)RenderDepthState_Wireframe->GetHandle();
        inst->BlendState = (ID3D11BlendState *)DisableBlend->GetHandle();
        inst->RasterizerState = (ID3D11RasterizerState *)RenderRasterState->GetHandle();
        if ( inst->ToolData )
        {
          CphxObject_Tool *t = (CphxObject_Tool*)inst->ToolData;

          inst->MaterialData[ 0 ] = t->WireframeColor.R() / 255.0f;
          inst->MaterialData[ 1 ] = t->WireframeColor.G() / 255.0f;
          inst->MaterialData[ 2 ] = t->WireframeColor.B() / 255.0f;
          inst->MaterialData[ 3 ] = 0.5f;

          if ( t->Selected )
          {
            inst->PS = (ID3D11PixelShader *)WirePixelShader->GetHandle();
            inst->MaterialData[ 0 ] = 1;//0;
            inst->MaterialData[ 1 ] = 1;//122 / 255.0f;
            inst->MaterialData[ 2 ] = 1;//204 / 255.0f;
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

          //CphxObject_Tool *t = (CphxObject_Tool*)inst->ToolData;
          //if (t->Selected) s->Scene.RenderLayers[x]->RenderInstances[y]->PS = (ID3D11PixelShader *)WirePixelShader_Selected->GetHandle();
          //else 
          //	if (t == MouseObject) 
          //		s->Scene.RenderLayers[x]->RenderInstances[y]->PS = (ID3D11PixelShader *)WirePixelShader_Highlighted->GetHandle();
        }
      }
      //s->Scene.RenderLayers[x]->RenderInstances.Sort(WireframeShaderSorter); //selected items are drawn later
    }
  }
  else
  {
  }
}

void CWBSceneDisplay::DrawGrid( CWBDrawAPI *API, CphxScene_Tool *s )
{
  if ( GridView && Mode != CAMERA_CAMERAVIEW )
  {
    if ( Mode < CAMERA_NORMAL ) DrawOrthoGrid( API );
    else
    {
      AntialiasedLineState->Apply();

      for ( TS32 x = 0; x < s->Scene.LayerCount; x++ )
      {
        if ( s->Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer )
          continue;

        if ( s->Scene.RenderLayers[ x ]->Descriptor->IgnoreHelperObjects )
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
        s->Scene.RenderLayers[ x ]->RenderInstances.Add( i );
      }
    }
  }
}

void CWBSceneDisplay::InjectLine( CphxScene_Tool *s, TS32 LayerIDX, CVector3 src, CVector3 dst, float r, float g, float b, float a )
{
  if ( !HelperView ) return;
  if ( !s ) return;

#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
  *i = LineInstance;
  i->DS = NULL;
  i->GS = NULL;
  i->HS = NULL;
  i->Indexed = true;

  D3DXVECTOR3 zd = dst - src;
  D3DXVec3Normalize( &zd, &zd );
  D3DXVECTOR3 yd = D3DXVECTOR3( 0, 1, 0 );
  D3DXVECTOR3 xd;
  D3DXVec3Cross( &xd, &yd, &zd );
  D3DXVec3Normalize( &xd, &xd );
  D3DXVec3Cross( &yd, &zd, &xd );
  D3DXMATRIX mx;
  D3DXMatrixIdentity( &mx );

  mx.m[ 0 ][ 0 ] = xd.x;
  mx.m[ 0 ][ 1 ] = xd.y;
  mx.m[ 0 ][ 2 ] = xd.z;
  mx.m[ 1 ][ 0 ] = yd.x;
  mx.m[ 1 ][ 1 ] = yd.y;
  mx.m[ 1 ][ 2 ] = yd.z;
  mx.m[ 2 ][ 0 ] = ( dst - src ).x;
  mx.m[ 2 ][ 1 ] = ( dst - src ).y;
  mx.m[ 2 ][ 2 ] = ( dst - src ).z;
  mx.m[ 3 ][ 0 ] = src.x;
  mx.m[ 3 ][ 1 ] = src.y;
  mx.m[ 3 ][ 2 ] = src.z;

  //D3DXMatrixMultiply( &mx, &mx, &Source->GetMatrix() );

  i->Matrices[ 0 ] = mx;
  i->MaterialData[ 0 ] = r;
  i->MaterialData[ 1 ] = g;
  i->MaterialData[ 2 ] = b;
  i->MaterialData[ 3 ] = a;

  s->Scene.RenderLayers[ LayerIDX ]->RenderInstances.Add( i );

}

void CWBSceneDisplay::InjectTargetLine( CphxScene_Tool *s, TS32 LayerIDX, CphxObject_Tool *Source, CphxObject_Tool *Target, float r, float g, float b, float a )
{
  if ( !HelperView ) return;
  if ( !s ) return;
  if ( !Source ) return;

#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  CphxRenderDataInstance *i = new CphxRenderDataInstance();
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
  *i = LineInstance;
  i->DS = NULL;
  i->GS = NULL;
  i->HS = NULL;
  i->Indexed = true;

  D3DXVECTOR3 src = Source->GetObject()->WorldPosition;
  D3DXVECTOR3 dst = D3DXVECTOR3( 0, 0, 0 );
  if ( Target ) dst = Target->GetObject()->WorldPosition;


  D3DXVECTOR3 zd = dst - src;
  D3DXVec3Normalize( &zd, &zd );
  D3DXVECTOR3 yd = D3DXVECTOR3( 0, 1, 0 );
  D3DXVECTOR3 xd;
  D3DXVec3Cross( &xd, &yd, &zd );
  D3DXVec3Normalize( &xd, &xd );
  D3DXVec3Cross( &yd, &zd, &xd );
  D3DXMATRIX mx;
  D3DXMatrixIdentity( &mx );

  mx.m[ 0 ][ 0 ] = xd.x;
  mx.m[ 0 ][ 1 ] = xd.y;
  mx.m[ 0 ][ 2 ] = xd.z;
  mx.m[ 1 ][ 0 ] = yd.x;
  mx.m[ 1 ][ 1 ] = yd.y;
  mx.m[ 1 ][ 2 ] = yd.z;
  mx.m[ 2 ][ 0 ] = ( dst - src ).x;
  mx.m[ 2 ][ 1 ] = ( dst - src ).y;
  mx.m[ 2 ][ 2 ] = ( dst - src ).z;


  D3DXMATRIX srcMatrix = Source->GetMatrix();

  for ( int x = 0; x < 3; x++ )
    for ( int y = 0; y < 3; y++ )
      srcMatrix.m[ x ][ y ] = x == y ? 1.0f : 0.0f;

  D3DXMatrixMultiply( &mx, &mx, &srcMatrix );

  i->Matrices[ 0 ] = mx;
  i->MaterialData[ 0 ] = r;
  i->MaterialData[ 1 ] = g;
  i->MaterialData[ 2 ] = b;
  i->MaterialData[ 3 ] = a;

  s->Scene.RenderLayers[ LayerIDX ]->RenderInstances.Add( i );

}

CAMERADATA * CWBSceneDisplay::GetCamData()
{
  CphxScene_Tool *m = GetActiveWorkBench()->GetEditedScene();
  if ( !m ) return NULL;

  if ( !CameraData.HasKey( m->GetGUID() ) )
    SetCameraMode( Mode );

  return &CameraData[ m->GetGUID() ];

}

D3DXMATRIX CWBSceneDisplay::GetHelperTransformationMatrix( D3DXVECTOR3 zd )
{
  D3DXMATRIX mx;
  D3DXMatrixIdentity( &mx );

  if ( Mode > CAMERA_NORMAL )
    return mx;

  D3DXVec3Normalize( &zd, &zd );
  D3DXVECTOR3 yd = D3DXVECTOR3( 0, 1, 0 );
  D3DXVECTOR3 xd;
  D3DXVec3Cross( &xd, &yd, &zd );
  D3DXVec3Normalize( &xd, &xd );
  D3DXVec3Cross( &yd, &zd, &xd );

  float scale = OrthoZoom / 4.0f;

  mx.m[ 0 ][ 0 ] = xd.x*scale;
  mx.m[ 0 ][ 1 ] = xd.y*scale;
  mx.m[ 0 ][ 2 ] = xd.z*scale;
  mx.m[ 1 ][ 0 ] = yd.x*scale;
  mx.m[ 1 ][ 1 ] = yd.y*scale;
  mx.m[ 1 ][ 2 ] = yd.z*scale;
  mx.m[ 2 ][ 0 ] = zd.x*scale;
  mx.m[ 2 ][ 1 ] = zd.y*scale;
  mx.m[ 2 ][ 2 ] = zd.z*scale;
  return mx;
}

CphxRenderTarget_Tool * CWBSceneDisplay::GetDisplayedRenderTarget()
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

void CWBSceneDisplay::SetRenderTarget( CphxGUID guid )
{
  RenderTarget = guid;
  CphxRenderTarget_Tool *r = Project.GetRenderTarget( guid );
  if ( !r ) return;
  CWBButton *b = (CWBButton*)FindChildByID( _T( "rtselector" ), _T( "button" ) );
  if ( !b ) return;
  b->SetText( r->Name );
}

void CWBSceneDisplay::SetCameraData( CphxGUID guid, CAMERADATA c )
{
  CameraData[ guid ] = c;
}
