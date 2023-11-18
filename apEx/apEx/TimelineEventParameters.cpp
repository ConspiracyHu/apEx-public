#define _WINSOCKAPI_
#include "BasePCH.h"
#include "../Phoenix/Timeline.h"
#include "TimelineEventParameters.h"
#define WINDOWNAME _T("Timeline Event Parameters")
#define WINDOWXML _T("TimelineEventParams")
#include "../Phoenix_Tool/apxProject.h"
#include "apExRoot.h"

extern CapexRoot *Root;

void CapexTimelineEventParameters::LoadCSS()
{
  CString xmlname = Root->GetCSSPath() + WINDOWXML + _T( ".css" );
  CStreamReaderMemory f;
  if ( !f.Open( xmlname.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading CSS: file '%s' not found", xmlname.GetPointer() );
    return;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );
  StyleManager.Reset();
  StyleManager.ParseStyleData( s );
}

void OpenSceneContextMenu( CWBContextMenu* ctx );


CapexTimelineEventParameters::CapexTimelineEventParameters() : CapexWindow()
{
  LoadCSS();
}

CapexTimelineEventParameters::CapexTimelineEventParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  LoadCSS();
}

CapexTimelineEventParameters::~CapexTimelineEventParameters()
{

}

void CapexTimelineEventParameters::UpdateData()
{
  SetEditedEvent( Project.GetEvent( EditedEvent ) );
}

void CapexTimelineEventParameters::FlushParameters()
{
  GeneratedParamRoots.FlushFast();
  DeleteChildren();

  //CString xmlname=CString(_T("Data/UI/"))+WINDOWXML;
  //App->LoadCSSFromFile(xmlname+".css");

  App->GenerateGUI( this, WINDOWXML );
  StyleManager.ApplyStyles( this );

  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );
}


void CapexTimelineEventParameters::SetEditedEvent( CphxEvent_Tool *event )
{
  TextureLocatorMap.Flush();

  FlushParameters();
  EditedEvent = GUIDServer.RequestGUID(); //flush
  ParamUIMap.Flush();

  if ( event )
    EditedEvent = event->GetGUID();

  DeleteChildren();
  if ( !event ) return;

  switch ( event->Type )
  {
  case EVENT_SHADERTOY:
  {
    App->GenerateGUITemplate( this, WINDOWXML, _T( "shadertoy" ) );

    CWBBox *parambox = (CWBBox*)FindChildByID( _T( "eventparambox" ), _T( "box" ) );
    if ( !parambox ) break;
    parambox->DeleteChildren();

    if ( !event ) break;
    CphxEvent_Shadertoy_Tool *m = (CphxEvent_Shadertoy_Tool *)event;
    if ( !m->GetTech() ) return;
    CphxMaterialTechnique_Tool *t = m->GetTech();

    for ( TS32 y = 0; y < t->TechParameters.Parameters.NumItems(); y++ )
      AddParameterUI( parambox, t->TechParameters.Parameters[ y ] );
    for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      for ( TS32 z = 0; z < t->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ )
        AddParameterUI( parambox, t->Passes[ y ]->PassParameters.Parameters[ z ] );

  }
  break;
  case EVENT_RENDERSCENE:
    App->GenerateGUITemplate( this, WINDOWXML, _T( "3devent" ) );
    break;

  case EVENT_PARTICLECALC:
    App->GenerateGUITemplate( this, WINDOWXML, _T( "particlecalc" ) );
    break;

  case EVENT_CAMERAOVERRIDE:
    App->GenerateGUITemplate( this, WINDOWXML, _T( "particlecalc" ) );
    break;

  case EVENT_CAMERASHAKE:
    App->GenerateGUITemplate( this, WINDOWXML, _T( "camshake" ) );
    break;
  default:
    break;
  }

  CWBItemSelector *dd = (CWBItemSelector*)FindChildByID( _T( "rtbutton" ), _T( "itemselector" ) );
  if ( dd )
  {
    for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
      if ( event->CanWriteToRenderTarget( Project.GetRenderTargetByIndex( x ) ) )
      {
        CphxRenderTarget_Tool *rt = Project.GetRenderTargetByIndex( x );
        dd->AddItem( rt->Name.GetPointer(), x );
      }

    dd->SelectItem( Project.GetRenderTargetIndex( event->TargetID ) );
  }

  dd = (CWBItemSelector *)FindChildByID( _T( "scenebutton" ), _T( "itemselector" ) );
  if ( dd && event->Type == EVENT_RENDERSCENE )
  {
    for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
      dd->AddItem( Project.GetSceneByIndex( x )->GetName().GetPointer(), x );

    CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)event;
    if ( r->Scene ) dd->SelectItem( Project.GetSceneIndex( r->Scene->GetGUID() ) );
  }

  if ( dd && event->Type == EVENT_PARTICLECALC )
  {
    for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
      dd->AddItem( Project.GetSceneByIndex( x )->GetName().GetPointer(), x );

    CphxEvent_ParticleCalc_Tool *r = (CphxEvent_ParticleCalc_Tool*)event;
    if ( r->Scene ) dd->SelectItem( Project.GetSceneIndex( r->Scene->GetGUID() ) );
  }

  if ( dd && event->Type == EVENT_CAMERAOVERRIDE )
  {
    for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
      dd->AddItem( Project.GetSceneByIndex( x )->GetName().GetPointer(), x );

    CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)event;
    if ( r->Scene ) dd->SelectItem( Project.GetSceneIndex( r->Scene->GetGUID() ) );
  }

  CWBButton* vb = (CWBButton*)FindChildByID( _T( "scenebutton" ), _T( "button" ) );
  if ( vb && event->Type == EVENT_RENDERSCENE )
  {
    CphxEvent_RenderScene_Tool* r = (CphxEvent_RenderScene_Tool*)event;
    if ( r->Scene ) vb->SetText( r->Scene->GetName() );
  }

  if ( vb && event->Type == EVENT_PARTICLECALC )
  {
    CphxEvent_ParticleCalc_Tool* r = (CphxEvent_ParticleCalc_Tool*)event;
    if ( r->Scene ) vb->SetText( r->Scene->GetName() );
  }

  if ( vb && event->Type == EVENT_CAMERAOVERRIDE )
  {
    CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)event;
    if ( r->Scene ) vb->SetText( r->Scene->GetName() );
  }

  if ( event->Type == EVENT_RENDERSCENE || event->Type == EVENT_PARTICLECALC || event->Type == EVENT_CAMERAOVERRIDE )
  {
    CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)event;
    UpdateSceneCamClipLists( r->Scene );
  }

  dd = (CWBItemSelector *)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
  if ( dd && event->Type == EVENT_RENDERSCENE )
  {
    CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)event;
    if ( r->Scene )
      if ( r->Clip ) dd->SelectItem( r->Scene->GetClipIndex( r->Clip->GetGUID() ) );
  }

  if ( dd && event->Type == EVENT_PARTICLECALC )
  {
    CphxEvent_ParticleCalc_Tool *r = (CphxEvent_ParticleCalc_Tool*)event;
    if ( r->Scene )
      if ( r->Clip ) dd->SelectItem( r->Scene->GetClipIndex( r->Clip->GetGUID() ) );
  }

  if ( dd && event->Type == EVENT_CAMERAOVERRIDE )
  {
    CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)event;
    if ( r->Scene )
      if ( r->Clip ) dd->SelectItem( r->Scene->GetClipIndex( r->Clip->GetGUID() ) );
  }

  dd = (CWBItemSelector *)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
  if ( dd && event->Type == EVENT_RENDERSCENE )
  {
    CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)event;
    if ( r->Scene )
      if ( r->Camera ) dd->SelectItem( r->Scene->GetObjectIndex( r->Camera->GetGUID() ) );
  }

  if ( dd && event->Type == EVENT_PARTICLECALC )
  {
    CphxEvent_ParticleCalc_Tool *r = (CphxEvent_ParticleCalc_Tool*)event;
    if ( r->Scene )
      if ( r->Camera ) dd->SelectItem( r->Scene->GetObjectIndex( r->Camera->GetGUID() ) );
  }

  if ( dd && event->Type == EVENT_CAMERAOVERRIDE )
  {
    CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)event;
    if ( r->Scene )
      if ( r->Camera ) dd->SelectItem( r->Scene->GetObjectIndex( r->Camera->GetGUID() ) );
  }

  vb = (CWBButton*)FindChildByID( _T( "sceneclearcolor" ), _T( "button" ) );
  if ( vb && event->Type == EVENT_RENDERSCENE )
  {
    CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)event;
    CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)r->Event;
    vb->Push( e->ClearColor );
  }

  vb = (CWBButton*)FindChildByID( _T( "sceneclearz" ), _T( "button" ) );
  if ( vb && event->Type == EVENT_RENDERSCENE )
  {
    CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)event;
    CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)r->Event;
    vb->Push( e->ClearZ );
  }

  CWBTrackBar *tb = (CWBTrackBar*)FindChildByID( _T( "shakespersec" ), _T( "trackbar" ) );
  if ( tb && event->Type == EVENT_CAMERASHAKE )
  {
    CphxEvent_CameraShake_Tool *r = (CphxEvent_CameraShake_Tool*)event;
    CphxEvent_CameraShake *e = (CphxEvent_CameraShake*)r->Event;
    tb->SetValue( e->ShakesPerSec );
  }

  StyleManager.ApplyStyles( this );

  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );
}

TBOOL CapexTimelineEventParameters::MessageProc( CWBMessage &Message )
{
  CphxEvent_Tool *e = Project.GetEvent( EditedEvent );

  CphxEvent_Shadertoy_Tool *sh = NULL;
  if ( e && e->Type == EVENT_SHADERTOY )
    sh = (CphxEvent_Shadertoy_Tool *)e;

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "sceneclearcolor" ) && e && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene *r = (CphxEvent_RenderScene*)e->Event;
      r->ClearColor = !r->ClearColor;
      b->Push( r->ClearColor );

      return true;
    }

    if ( b->GetID() == _T( "sceneclearz" ) && e && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene *r = (CphxEvent_RenderScene*)e->Event;
      r->ClearZ = !r->ClearZ;
      b->Push( r->ClearZ );

      return true;
    }

    if ( b->GetID() == _T( "textureselector" ) )
    {
      if ( !sh ) return true;
      BuildTextureSelectorUI( b );
      return true;
    }

    if ( b->GetID() == _T( "texturelocator" ) )
    {
      if ( !sh ) return true;
      if ( TextureLocatorMap.HasKey( b->GetGuid() ) )
      {
        CphxGUID &g = sh->MaterialData.MaterialTextures[ TextureLocatorMap[ b->GetGuid() ] ];
        CphxTextureOperator_Tool *op = Project.GetTexgenOp( g );
        if ( op )
          Root->GoToTexture( op, !App->GetCtrlState() );
      }
      return true;
    }

    if ( b->GetID() == _T( "scenelocator" ) && e && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)e;
      if ( r->Scene )
        Root->GoToScene( r->Scene, !App->GetCtrlState() );
      return true;
    }

    if ( b->GetID() == _T( "rendertargetselector" ) )
    {
      if ( !sh ) return true;
      BuildTextureSelectorUI( b, true, true );
      return true;
    }

    if ( b->GetID() == _T( "wireframe" ) )
    {
      if ( !sh ) return true;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      sh->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe = !sh->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe;
      b->Push( sh->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe );
    }

    if ( b->GetID() == _T( "zenable" ) )
    {
      if ( !sh ) return true;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = sh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode;
      TBOOL DepthEnable = !( bm & 1 );
      TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
      DepthEnable = !DepthEnable;
      bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
      sh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode = bm;
      b->Push( DepthEnable );
    }

    if ( b->GetID() == _T( "zwrite" ) )
    {
      if ( !sh ) return true;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = sh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode;
      TBOOL DepthEnable = !( bm & 1 );
      TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
      ZWriteEnable = !ZWriteEnable;
      bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
      sh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode = bm;
      b->Push( ZWriteEnable );
    }

    if ( e && b->GetID() == _T( "scenebutton" ) )
    {
      CWBContextMenu* c = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );
      OpenSceneContextMenu( c );
    }

  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *b = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !b ) break;

    if ( e && b->GetID() == _T( "rtbutton" ) )
    {
      CphxRenderTarget_Tool *rt = Project.GetRenderTargetByIndex( Message.Data );
      if ( rt )
      {
        e->TargetID = rt->GetGUID();
        e->Event->Target = &rt->rt;
      }
      return true;
    }

    if ( e && b->GetID() == _T( "scenebutton" ) && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)e;
      r->SetScene( Project.GetSceneByIndex( Message.Data ) );

      //set the other two dropdowns to the proper item

      UpdateSceneCamClipLists( r->Scene );

      if ( r->Scene )
      {
        b = (CWBItemSelector*)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Camera ? r->Scene->GetObjectIndex( r->Camera ) : -1 );

        b = (CWBItemSelector*)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Clip ? r->Scene->GetClipIndex( r->Clip->GetGUID() ) : -1 );
      }

      return true;
    }

    if ( e && b->GetID() == _T( "scenebutton" ) && e->Type == EVENT_PARTICLECALC )
    {
      CphxEvent_ParticleCalc_Tool *r = (CphxEvent_ParticleCalc_Tool*)e;
      r->SetScene( Project.GetSceneByIndex( Message.Data ) );

      auto *ev = (CphxEvent_ParticleCalc*)r->Event;
      ev->OnScreenLastFrame = false;

      //set the other two dropdowns to the proper item

      UpdateSceneCamClipLists( r->Scene );

      if ( r->Scene )
      {
        b = (CWBItemSelector*)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Camera ? r->Scene->GetObjectIndex( r->Camera ) : -1 );

        b = (CWBItemSelector*)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Clip ? r->Scene->GetClipIndex( r->Clip->GetGUID() ) : -1 );
      }

      return true;
    }

    if ( e && b->GetID() == _T( "scenebutton" ) && e->Type == EVENT_CAMERAOVERRIDE )
    {
      CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)e;
      r->SetScene( Project.GetSceneByIndex( Message.Data ) );

      auto* ev = (CphxEvent_CameraOverride*)r->Event;
      ev->OnScreenLastFrame = false;

      //set the other two dropdowns to the proper item

      UpdateSceneCamClipLists( r->Scene );

      if ( r->Scene )
      {
        b = (CWBItemSelector*)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Camera ? r->Scene->GetObjectIndex( r->Camera ) : -1 );

        b = (CWBItemSelector*)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Clip ? r->Scene->GetClipIndex( r->Clip->GetGUID() ) : -1 );
      }

      return true;
    }

    if ( e && b->GetID() == _T( "camerabutton" ) && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)e;
      if ( !r->Scene ) return true;
      r->SetCamera( r->Scene->GetObjectByIndex( Message.Data ) );
      return true;
    }

    if ( e && b->GetID() == _T( "camerabutton" ) && e->Type == EVENT_PARTICLECALC )
    {
      CphxEvent_ParticleCalc_Tool *r = (CphxEvent_ParticleCalc_Tool*)e;
      auto *ev = (CphxEvent_ParticleCalc*)r->Event;
      ev->OnScreenLastFrame = false;
      if ( !r->Scene ) return true;
      r->SetCamera( r->Scene->GetObjectByIndex( Message.Data ) );
      return true;
    }

    if ( e && b->GetID() == _T( "camerabutton" ) && e->Type == EVENT_CAMERAOVERRIDE )
    {
      CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)e;
      auto* ev = (CphxEvent_CameraOverride*)r->Event;
      ev->OnScreenLastFrame = false;
      if ( !r->Scene ) return true;
      r->SetCamera( r->Scene->GetObjectByIndex( Message.Data ) );
      return true;
    }

    if ( e && b->GetID() == _T( "clipbutton" ) && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene_Tool *r = (CphxEvent_RenderScene_Tool*)e;
      if ( !r->Scene ) return true;
      r->SetClip( r->Scene->GetClipByIndex( Message.Data ) );
      return true;
    }

    if ( e && b->GetID() == _T( "clipbutton" ) && e->Type == EVENT_PARTICLECALC )
    {
      CphxEvent_ParticleCalc_Tool *r = (CphxEvent_ParticleCalc_Tool*)e;
      auto *ev = (CphxEvent_ParticleCalc*)r->Event;
      ev->OnScreenLastFrame = false;
      if ( !r->Scene ) return true;
      r->SetClip( r->Scene->GetClipByIndex( Message.Data ) );
      return true;
    }

    if ( e && b->GetID() == _T( "clipbutton" ) && e->Type == EVENT_CAMERAOVERRIDE )
    {
      CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)e;
      auto* ev = (CphxEvent_CameraOverride*)r->Event;
      ev->OnScreenLastFrame = false;
      if ( !r->Scene ) return true;
      r->SetClip( r->Scene->GetClipByIndex( Message.Data ) );
      return true;
    }

    if ( sh && b->GetID() == _T( "srcblend" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = sh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode;
      bm = b->GetCursorPosition() | ( bm & ~15 );
      sh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode = bm;
      return true;
    }

    if ( sh && b->GetID() == _T( "dstblend" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = sh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode;
      bm = ( b->GetCursorPosition() << 4 ) | ( bm & 15 );
      sh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode = bm;
      return true;
    }

    if ( sh && b->GetID() == _T( "culllist" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      sh->MaterialData.MaterialParams[ p->GetGUID() ].CullMode = (D3D11_CULL_MODE)( b->GetCursorPosition() + 1 );
      return true;
    }

    if ( sh && b->GetID() == _T( "zfunctlist" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      sh->MaterialData.MaterialParams[ p->GetGUID() ].ZFunction = (D3D11_COMPARISON_FUNC)( b->GetCursorPosition() + 1 );
      return true;
    }

    break;
  }

  case WBM_CONTEXTMESSAGE:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( sh && b->GetID() == _T( "textureselector" ) )
    {
      if ( TextureIDMap.HasKey( Message.Data ) )
      {
        CphxGUID g = TextureIDMap[ Message.Data ];
        CphxMaterialParameter_Tool *p = GetTargetParam( b );
        if ( !p ) return true;

        if ( sh->MaterialData.MaterialTextures.HasKey( p->GetGUID() ) )
          sh->RemoveParent( Project.GetResource( sh->MaterialData.MaterialTextures[ p->GetGUID() ] ) );

        sh->MaterialData.MaterialTextures[ p->GetGUID() ] = g;
        sh->AddParent( Project.GetResource( g ) );

        if ( Project.GetTexgenOp( g ) )
        {
          b->SetText( Project.GetTexgenOp( g )->GetName().GetPointer() );
          return true;
        }
        if ( Project.GetRenderTarget( g ) )
        {
          b->SetText( Project.GetRenderTarget( g )->Name.GetPointer() );
          return true;
        }
        b->SetText( _T( "Select Texture" ) );

      }
      return true;
    }

    if ( sh && b->GetID() == _T( "rendertargetselector" ) )
    {
      if ( TextureIDMap.HasKey( Message.Data ) )
      {
        CphxGUID g = TextureIDMap[ Message.Data ];
        CphxMaterialParameter_Tool *p = GetTargetParam( b );
        if ( !p ) return true;
        sh->MaterialData.MaterialTextures[ p->GetGUID() ] = g;

        if ( Project.GetRenderTarget( g ) )
        {
          b->SetText( Project.GetRenderTarget( g )->Name.GetPointer() );
          return true;
        }
        b->SetText( _T( "Select RenderTarget" ) );

      }
      return true;
    }

    if ( e && b->GetID() == _T( "scenebutton" ) && e->Type == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene_Tool* r = (CphxEvent_RenderScene_Tool*)e;
      r->SetScene( Project.GetSceneByIndex( Message.Data ) );

      //set the other two dropdowns to the proper item

      UpdateSceneCamClipLists( r->Scene );

      if ( r->Scene )
        b->SetText( r->Scene->GetName() );
      else
        b->SetText( "" );

      if ( r->Scene )
      {
        CWBItemSelector* b = (CWBItemSelector*)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Camera ? r->Scene->GetObjectIndex( r->Camera ) : -1 );

        b = (CWBItemSelector*)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Clip ? r->Scene->GetClipIndex( r->Clip->GetGUID() ) : -1 );
      }

      return true;
    }

    if ( e && b->GetID() == _T( "scenebutton" ) && e->Type == EVENT_PARTICLECALC )
    {
      CphxEvent_ParticleCalc_Tool* r = (CphxEvent_ParticleCalc_Tool*)e;
      r->SetScene( Project.GetSceneByIndex( Message.Data ) );

      auto* ev = (CphxEvent_ParticleCalc*)r->Event;
      ev->OnScreenLastFrame = false;

      //set the other two dropdowns to the proper item

      UpdateSceneCamClipLists( r->Scene );

      if ( r->Scene )
        b->SetText( r->Scene->GetName() );
      else
        b->SetText( "" );

      if ( r->Scene )
      {
        CWBItemSelector* b = (CWBItemSelector*)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Camera ? r->Scene->GetObjectIndex( r->Camera ) : -1 );

        b = (CWBItemSelector*)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Clip ? r->Scene->GetClipIndex( r->Clip->GetGUID() ) : -1 );
      }

      return true;
    }

    if ( e && b->GetID() == _T( "scenebutton" ) && e->Type == EVENT_CAMERAOVERRIDE )
    {
      CphxEvent_CameraOverride_Tool* r = (CphxEvent_CameraOverride_Tool*)e;
      r->SetScene( Project.GetSceneByIndex( Message.Data ) );

      auto* ev = (CphxEvent_CameraOverride*)r->Event;
      ev->OnScreenLastFrame = false;

      //set the other two dropdowns to the proper item

      UpdateSceneCamClipLists( r->Scene );

      if ( r->Scene )
        b->SetText( r->Scene->GetName() );
      else
        b->SetText( "" );

      if ( r->Scene )
      {
        CWBItemSelector* b = (CWBItemSelector*)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Camera ? r->Scene->GetObjectIndex( r->Camera ) : -1 );

        b = (CWBItemSelector*)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
        if ( b ) b->SelectItem( r->Clip ? r->Scene->GetClipIndex( r->Clip->GetGUID() ) : -1 );
      }

      return true;
    }

  }

  break;

  case WBM_VALUECHANGED:
  {
    CWBTrackBar *b = (CWBTrackBar *)App->FindItemByGuid( Message.GetTarget(), _T( "trackbar" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "shakespersec" ) )
    {
      CphxEvent_CameraShake_Tool *sh = NULL;
      if ( e && e->Type == EVENT_CAMERASHAKE )
        sh = (CphxEvent_CameraShake_Tool *)e;

      ( (CphxEvent_CameraShake*)sh->Event )->ShakesPerSec = Message.Data;
      b->SetText( CString::Format( _T( "Shakes per Second: %d" ), Message.Data ) );

      return true;
    }


    CphxMaterialParameter_Tool *p = GetTargetParam( b );
    if ( !p ) return true;
    if ( !sh ) return true;

    if ( b->GetID() == _T( "red" ) )
    {
      sh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 0 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Red: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "green" ) )
    {
      sh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 1 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Green: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "blue" ) )
    {
      sh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 2 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Blue: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "alpha" ) )
    {
      sh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 3 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Alpha: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "float" ) )
    {
      sh->MaterialData.MaterialParams[ p->GetGUID() ].Float = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "renderpriority" ) )
    {
      sh->MaterialData.MaterialParams[ p->GetGUID() ].RenderPriority = Message.Data;
      b->SetText( CString::Format( _T( "Render Priority: %d" ), Message.Data ) );
      return true;
    }

  }

  break;
  }


  return CapexWindow::MessageProc( Message );
}

void CapexTimelineEventParameters::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}

void CapexTimelineEventParameters::AddParameterUI( CWBItem *Root, CphxMaterialParameter_Tool *Param )
{
  if ( Param->Parameter.Scope != PARAM_VARIABLE ) return;

  CphxEvent_Tool *Edited = Project.GetEvent( EditedEvent );

  if ( !Edited ) return;
  if ( Edited->Type != EVENT_SHADERTOY ) return;
  CphxEvent_Shadertoy_Tool *shtoy = (CphxEvent_Shadertoy_Tool*)Edited;

  CWBBox *nb = new CWBBox( Root, CRect( 10, 10, 50, 50 ) );
  ParamUIMap[ Param->GetGUID() ] = nb;

  CString templatename;

  switch ( Param->Parameter.Type )
  {
  case PARAM_TEXTURE0:
  case PARAM_TEXTURE1:
  case PARAM_TEXTURE2:
  case PARAM_TEXTURE3:
  case PARAM_TEXTURE4:
  case PARAM_TEXTURE5:
  case PARAM_TEXTURE6:
  case PARAM_TEXTURE7:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "texturetemplate" ) ) );
    CWBButton *b = (CWBButton *)nb->FindChildByID( _T( "textureselector" ), _T( "button" ) );
    if ( b )
    {
      if ( shtoy->MaterialData.MaterialTextures.HasKey( Param->GetGUID() ) )
      {
        CphxGUID &g = shtoy->MaterialData.MaterialTextures[ Param->GetGUID() ];
        CphxRenderTarget_Tool *t = Project.GetRenderTarget( g );
        if ( t ) b->SetText( t->Name );
        else
        {
          CphxTextureOperator_Tool *o = Project.GetTexgenOp( g );
          if ( o ) b->SetText( o->GetName() );
          else b->SetText( _T( "Select Texture" ) );
        }
      }
      else b->SetText( _T( "Select Texture" ) );
    }

    b = (CWBButton *)nb->FindChildByID( _T( "texturelocator" ), _T( "button" ) );
    if ( b )
      TextureLocatorMap[ b->GetGuid() ] = Param->GetGUID();
  }
  break;
  case PARAM_RENDERTARGET:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "rendertargettemplate" ) ) );
    CWBButton *b = (CWBButton *)nb->FindChildByID( _T( "rendertargetselector" ), _T( "button" ) );
    if ( b )
    {
      if ( shtoy->MaterialData.MaterialTextures.HasKey( Param->GetGUID() ) )
      {
        CphxGUID &g = shtoy->MaterialData.MaterialTextures[ Param->GetGUID() ];
        CphxRenderTarget_Tool *t = Project.GetRenderTarget( g );
        if ( t ) b->SetText( t->Name );
        else b->SetText( _T( "Select RenderTarget" ) );
      }
      else b->SetText( _T( "Select RenderTarget" ) );
    }
  }
  break;
  case PARAM_BLENDMODE0:
  case PARAM_BLENDMODE1:
  case PARAM_BLENDMODE2:
  case PARAM_BLENDMODE3:
  case PARAM_BLENDMODE4:
  case PARAM_BLENDMODE5:
  case PARAM_BLENDMODE6:
  case PARAM_BLENDMODE7:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "blendtemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].BlendMode = phxSrcBlend_ONE | phxDstBlend_ZERO;

    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *src = (CWBItemSelector*)nb->FindChildByID( _T( "srcblend" ), _T( "itemselector" ) );
    CWBItemSelector *dst = (CWBItemSelector*)nb->FindChildByID( _T( "dstblend" ), _T( "itemselector" ) );

    if ( src )
      src->SelectItemByIndex( v.BlendMode & 15 );
    if ( dst )
      dst->SelectItemByIndex( v.BlendMode >> 4 );
  }
  break;

  case PARAM_COLOR:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "colortemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      for ( TS32 c = 0; c < 4; c++ )
        shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].Color[ c ] = 0.25;

    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByID( _T( "red" ), _T( "trackbar" ) );
    CWBTrackBar *g = (CWBTrackBar*)nb->FindChildByID( _T( "green" ), _T( "trackbar" ) );
    CWBTrackBar *b = (CWBTrackBar*)nb->FindChildByID( _T( "blue" ), _T( "trackbar" ) );
    CWBTrackBar *a = (CWBTrackBar*)nb->FindChildByID( _T( "alpha" ), _T( "trackbar" ) );

    if ( r ) r->SetValue( (TS32)( v.Color[ 0 ] * 255 + 0.5f ) );
    if ( g ) g->SetValue( (TS32)( v.Color[ 1 ] * 255 + 0.5f ) );
    if ( b ) b->SetValue( (TS32)( v.Color[ 2 ] * 255 + 0.5f ) );
    if ( a ) a->SetValue( (TS32)( v.Color[ 3 ] * 255 + 0.5f ) );

    break;
  }

  case PARAM_FLOAT:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "floattemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].Float = 0;

    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByID( _T( "float" ), _T( "trackbar" ) );

    if ( r ) r->SetValue( (TS32)( v.Float * 255 + 0.5f ) );
    break;
  }

  case PARAM_RENDERPRIORITY:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "prioritytemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].RenderPriority = 127;

    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByID( _T( "renderpriority" ), _T( "trackbar" ) );

    if ( r )
      r->SetValue( v.RenderPriority );
    break;
  }

  case PARAM_CULLMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "culltemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].CullMode = D3D11_CULL_NONE;
    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *cull = (CWBItemSelector*)nb->FindChildByID( _T( "culllist" ), _T( "itemselector" ) );

    if ( cull )
      cull->SelectItemByIndex( v.CullMode - 1 );
    break;
  }

  case PARAM_FILLMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "filltemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].Wireframe = false;
    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBButton *wf = (CWBButton*)nb->FindChildByID( _T( "wireframe" ), _T( "button" ) );
    if ( wf )
      wf->Push( v.Wireframe );
    break;
  }

  case PARAM_ZFUNCTION:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "zfuncttemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].ZFunction = D3D11_COMPARISON_LESS;
    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *zf = (CWBItemSelector*)nb->FindChildByID( _T( "zfunctlist" ), _T( "itemselector" ) );

    if ( zf )
      zf->SelectItemByIndex( v.ZFunction - 1 );
    break;
  }

  case PARAM_ZMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "zmodetemplate" ) ) );
    if ( !shtoy->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      shtoy->MaterialData.MaterialParams[ Param->GetGUID() ].ZMode = 0;
    MATERIALVALUE v = shtoy->MaterialData.MaterialParams[ Param->GetGUID() ];

    CWBButton *zenable = (CWBButton*)nb->FindChildByID( _T( "zenable" ), _T( "button" ) );
    CWBButton *zwrite = (CWBButton*)nb->FindChildByID( _T( "zwrite" ), _T( "button" ) );

    if ( zenable ) zenable->Push( !( v.ZMode & 1 ) );
    if ( zwrite ) zwrite->Push( !( ( v.ZMode >> 1 ) & 1 ) );

    break;
  }

  default:
    break;
  }

  CWBLabel *pn = (CWBLabel*)nb->FindChildByID( _T( "paramname" ), _T( "label" ) );
  if ( pn ) pn->SetText( Param->Name );
}

void CapexTimelineEventParameters::BuildTextureSelectorUI( CWBItem *Button, TBOOL RTOnly, TBOOL HideHidden )
{
  CWBContextMenu *c = Button->OpenContextMenu( Button->ClientToScreen( Button->GetClientRect().BottomLeft() ) );
  TextureIDMap.Flush();

  TS32 id = 0;

  c->AddItem( _T( "No Texture" ), id );
  CphxGUID g;
  g.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  TextureIDMap[ id ] = g;
  id++;

  c->AddSeparator();

  if ( Project.GetRenderTargetCount() )
  {
    CWBContextItem *it = c->AddItem( _T( "Rendertargets" ), id++ );

    for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
    {
      if ( !HideHidden || !Project.GetRenderTargetByIndex( x )->HiddenFromTimeline )
        it->AddItem( Project.GetRenderTargetByIndex( x )->Name.GetPointer(), id );
      TextureIDMap[ id ] = Project.GetRenderTargetByIndex( x )->GetGUID();
      id++;
    }
    c->AddSeparator();
  }

  if ( RTOnly ) return;

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    CapexTexGenPage *p = Project.GetTexgenPageByIndex( x );

    TS32 savecnt = 0;
    for ( TS32 y = 0; y < p->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *t = p->GetOp( y );
      if ( t->GetOpType() == TEXGEN_OP_SAVE ) savecnt++;
    }

    if ( savecnt )
    {
      CWBContextItem *it = c->AddItem( p->GetName().GetPointer(), id++ );

      for ( TS32 y = 0; y < p->GetOpCount(); y++ )
      {
        CphxTextureOperator_Tool *t = p->GetOp( y );
        if ( t->GetOpType() == TEXGEN_OP_SAVE )
        {
          it->AddItem( t->GetName().GetPointer(), id );
          TextureIDMap[ id ] = t->GetGUID();
          id++;
        }
      }
    }
  }

  //for (TS32 x = 0; x < Project.GetTexgenPageCount(); x++)
  //{
  //	CapexTexGenPage *p = Project.GetTexgenPageByIndex(x);
  //	for (TS32 y = 0; y < p->GetOpCount(); y++)
  //	{
  //		CphxTextureOperator_Tool *t = p->GetOp(y);
  //		if (t->GetOpType() == TEXGEN_OP_SAVE)
  //		{
  //			c->AddItem(t->GetName().GetPointer(), id);
  //			TextureIDMap[id] = t->GetGUID();
  //			id++;
  //		}
  //	}
  //}
}

CphxMaterialParameter_Tool * CapexTimelineEventParameters::GetTargetParam( CWBItem *i )
{
  CphxEvent_Tool *e = Project.GetEvent( EditedEvent );

  CphxEvent_Shadertoy_Tool *sh = NULL;
  if ( e && e->Type == EVENT_SHADERTOY )
    sh = (CphxEvent_Shadertoy_Tool *)e;

  if ( !sh ) return NULL;

  if ( !sh->GetTech() ) return NULL;
  CphxMaterialTechnique_Tool *mat = sh->GetTech();

  for ( TS32 x = 0; x < ParamUIMap.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CWBItem *>::KDPair *kdp = ParamUIMap.GetKDPair( x );
    if ( i->FindItemInParentTree( kdp->Data ) )
      return mat->GetParameter( kdp->Key );
  }

  return NULL;
}

void CapexTimelineEventParameters::UpdateSceneCamClipLists( CphxScene_Tool *Scene )
{
  CWBItemSelector *dd = (CWBItemSelector *)FindChildByID( _T( "clipbutton" ), _T( "itemselector" ) );
  if ( dd )
  {
    dd->Flush();
    if ( Scene )
    {
      for ( TS32 x = 0; x < Scene->GetClipCount(); x++ )
        dd->AddItem( Scene->GetClipByIndex( x )->GetName().GetPointer(), x );
    }
    else dd->Flush();
  }

  dd = (CWBItemSelector *)FindChildByID( _T( "camerabutton" ), _T( "itemselector" ) );
  if ( dd )
  {
    dd->Flush();
    if ( Scene )
    {
      for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
        if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
          dd->AddItem( Scene->GetObjectByIndex( x )->GetName().GetPointer(), x );
    }
    else dd->Flush();
  }
}
