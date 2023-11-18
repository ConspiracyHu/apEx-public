#include "BasePCH.h"
#include "SceneObjectParameters.h"
#define WINDOWNAME _T("Scene Object Parameters")
#define WINDOWXML _T("SceneObjectParameters")
#include "WorkBench.h"
#include "SceneList.h"
#include "ModelList.h"
#include "apExRoot.h"
#include "SceneClips.h"

extern CapexRoot *Root;

CString ObjectNames[] =
{
  _T( "model" ),
  _T( "light" ),
  _T( "camera" ),
  _T( "dummy" ),
  _T( "subscene" ),
  _T( "emitter" ),
  _T( "emittercpu" ),
  _T( "emitterdrag" ),
  _T( "emittergravity" ),
  _T( "emitterturbulence" ),
  _T( "logicobject" ),
};

/*

<dropdown id = "emittershape" text = "Emitter Shape">
<trackbar id = "particlebuffer" minimum = "1" maximum = "16" default = "10" / >
<button id = "agingparticles" text = "Aging Particles" / >
<trackbar id = "startcount" minimum = "0" maximum = "255" default = "127" / >
<trackbar id = "randseed" minimum = "0" maximum = "255" default = "0" / >
<button id = "sortparticle" text = "Z-Sort" / >

*/

void AddModelSceneContextItem( CWBContextMenu* ctx, CString& text, int data )
{
  CStringArray path = text.Explode( _T( "/" ) );

  if ( path.NumItems() <= 1 )
  {
    ctx->AddItem( text, data );
    return;
  }

  for ( int x = 0; x < path.NumItems(); x++ )
    path[ x ].ToLower();

  CWBContextItem* subMenu = ctx->GetItem( path[ 0 ] );
  if ( !subMenu )
    subMenu = ctx->AddItem( path[ 0 ], data );

  for ( int x = 1; x < path.NumItems(); x++ )
  {
    CWBContextItem* parent = subMenu;
    subMenu = subMenu->GetItem( path[x] );
    if ( !subMenu )
      subMenu = parent->AddItem( path[ x ].GetPointer(), data );
  }
}

void OpenModelContextMenu( CWBContextMenu* ctx )
{
  for ( int x = 0; x < Project.GetModelCount(); x++ )
    AddModelSceneContextItem( ctx, Project.GetModelByIndex( x )->GetName(), x );
}

void OpenSceneContextMenu( CWBContextMenu* ctx, CphxScene_Tool* excluded )
{
  for ( int x = 0; x < Project.GetSceneCount(); x++ )
    Project.GetSceneByIndex( x )->useCount = 0;

  for ( int x = 0; x < Project.GetEventCount(); x++ )
  {
    if ( Project.GetEventByIndex( x )->GetEventType() == EVENT_RENDERSCENE )
    {
      auto* scene = ( (CphxEvent_RenderScene_Tool*)Project.GetEventByIndex( x ) )->Scene;
      if ( scene )
        scene->useCount++;
    }
  }

  for ( int x = 0; x < Project.GetSceneCount(); x++ )
    if ( !excluded || excluded->CanContain( Project.GetSceneByIndex( x ) ) )
      AddModelSceneContextItem( ctx, Project.GetSceneByIndex( x )->GetName() + " " + CString::Format( "(%d)", Project.GetSceneByIndex( x )->useCount ), x );
}

void OpenSceneContextMenu( CWBContextMenu* ctx )
{
  OpenSceneContextMenu( ctx, nullptr );
}

void CapexSceneObjectParameters::LoadCSS()
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

CapexSceneObjectParameters::CapexSceneObjectParameters() : CapexWindow()
{
  LoadCSS();
}

CapexSceneObjectParameters::CapexSceneObjectParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  LoadCSS();
}

void CapexSceneObjectParameters::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}


CapexSceneObjectParameters::~CapexSceneObjectParameters()
{

}

void CapexSceneObjectParameters::UpdateData()
{
  if ( !WorkBench->GetEditedScene() || WorkBench->GetEditedScene()->GetObjectIndex( EditedObj ) < 0 )
  {
    SelectSceneObject( NULL );
    return;
  }
  else
  {
    SelectSceneObject( EditedObj );
  }
}

void CapexSceneObjectParameters::SelectSceneObject( CphxObject_Tool *Obj )
{
  EditedObj = Obj;

  CWBBox *box = (CWBBox*)FindChildByID( _T( "parambox" ), _T( "box" ) );
  if ( !box ) return;

  box->DeleteChildren();
  ParamUIMap.Flush();

  if ( !Obj ) return;

  App->GenerateGUITemplate( box, CString( WINDOWXML ), ObjectNames[ Obj->GetObjectType() ] );

  TextureLocatorMap.Flush();

  switch ( Obj->GetObjectType() )
  {
  case Object_Model:
  {
    CphxObject_Model_Tool *l = (CphxObject_Model_Tool*)Obj;
    CWBButton *b = (CWBButton*)FindChildByID( _T( "swapmodel" ), _T( "button" ) );
    if ( b && l->Model )
      b->SetText( l->Model->GetName() );
  }
  break;
  case Object_SubScene:
  {
    CphxObject_SubScene_Tool *l = (CphxObject_SubScene_Tool*)Obj;
    CWBButton *b = (CWBButton*)FindChildByID( _T( "swapscene" ), _T( "button" ) );
    if ( b )
    {
      auto *scene = WorkBench->GetEditedScene();
      if ( scene )
      {
        auto id = scene->GetActiveClip();
        auto clip = l->GetClip( id );
        if ( clip && clip->GetSubSceneTarget() )
        {
          b->SetText( clip->GetSubSceneTarget()->GetName() );
        }        
      }
      else
        b->SetText( "### ERROR ###" );
    }
  }
  break;
  case Object_Light:
  {
    CphxObject_Light_Tool *l = (CphxObject_Light_Tool*)Obj;
    CWBButton *b = (CWBButton*)FindChildByID( _T( "dotlight" ), _T( "button" ) );
    if ( b ) b->Push( l->IsPointLight() );
    b = (CWBButton*)FindChildByID( _T( "lighttarget" ), _T( "button" ) );
    if ( b && l->TargetObject )
      b->SetText( _T( "Target: " ) + l->TargetObject->GetName() );
  }
  break;
  case Object_CamEye:
  {
    CWBButton *b = (CWBButton*)FindChildByID( _T( "camtarget" ), _T( "button" ) );
    if ( b && Obj->TargetObject )
      b->SetText( _T( "Target: " ) + Obj->TargetObject->GetName() );

    CWBTrackBar *tb = (CWBTrackBar*)FindChildByIDs( _T( "camcenterx" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      tb->SetValue( Obj->GetObject()->camCenterX );
      tb->SetText( CString::Format( _T( "Camera Center X: %f" ), Obj->GetObject()->camCenterX / 127.0f ) );
    }

    tb = (CWBTrackBar*)FindChildByIDs( _T( "camcentery" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      tb->SetValue( Obj->GetObject()->camCenterY );
      tb->SetText( CString::Format( _T( "Camera Center Y: %f" ), Obj->GetObject()->camCenterY / 127.0f ) );
    }
  }
  break;
  case Object_ParticleGravity:
  {
    CphxObject_ParticleGravity_Tool *m = (CphxObject_ParticleGravity_Tool*)Obj;
    CphxObject_ParticleGravity *o = (CphxObject_ParticleGravity*)m->GetObject();

    CWBButton *b = (CWBButton*)FindChildByID( _T( "limitedaffector" ), _T( "button" ) );
    if ( b )
      b->Push( o->AreaType );
    b = (CWBButton*)FindChildByID( _T( "directionalgravity" ), _T( "button" ) );
    if ( b )
      b->Push( o->Directional );
  }
  break;
  case Object_ParticleDrag:
  {
    CphxObject_ParticleDrag_Tool *m = (CphxObject_ParticleDrag_Tool*)Obj;
    CphxObject_ParticleDrag *o = (CphxObject_ParticleDrag*)m->GetObject();

    CWBButton *b = (CWBButton*)FindChildByID( _T( "limitedaffector" ), _T( "button" ) );
    if ( b )
      b->Push( o->AreaType );
  }
  break;
  case Object_ParticleTurbulence:
  {
    CphxObject_ParticleTurbulence_Tool *m = (CphxObject_ParticleTurbulence_Tool*)Obj;
    CphxObject_ParticleTurbulence *o = (CphxObject_ParticleTurbulence*)m->GetObject();

    CWBButton *b = (CWBButton*)FindChildByID( _T( "limitedaffector" ), _T( "button" ) );
    if ( b )
      b->Push( o->AreaType );

    CWBTextBox *t = (CWBTextBox*)FindChildByID( _T( "turbulencefrequency" ), _T( "textbox" ) );
    if ( t )
    {
      int freq = 0;

      auto* scene = WorkBench->GetEditedScene();
      if ( scene )
      {
        auto id = scene->GetActiveClip();
        auto clip = m->GetClip( id );
        freq = clip->GetTurbulenceFreq();
      }
      else
        freq = -25472;

      t->SetText( CString::Format( _T( "%d" ), freq ) );
    }

    CWBTrackBar *tb = (CWBTrackBar*)FindChildByIDs( _T( "turbulencerandseed" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      int seed = 0;

      auto* scene = WorkBench->GetEditedScene();
      if ( scene )
      {
        auto id = scene->GetActiveClip();
        auto clip = m->GetClip( id );
        seed = clip->GetRandSeed();
      }
      else
        seed = -25472;

      tb->SetValue( seed );
      tb->SetText( CString::Format( _T( "Randseed: %d" ), seed ) );
    }
  }
  break;
  case Object_ParticleVortex:
  {
    CphxObject_ParticleVortex_Tool* m = (CphxObject_ParticleVortex_Tool*)Obj;
    CphxObject_ParticleVortex* o = (CphxObject_ParticleVortex*)m->GetObject();

    CWBButton* b = (CWBButton*)FindChildByID( _T( "limitedaffector" ), _T( "button" ) );
    if ( b )
      b->Push( o->AreaType );
  }
  break;
  case Object_LogicObject:
  {
    CWBItemSelector *is = (CWBItemSelector*)FindChildByIDs( _T( "logictype" ), _T( "itemselector" ) );
    if ( is )
    {
      auto* o = Obj->GetObject();
      is->SelectItemByIndex( o->camCenterX );
    }

    CWBTrackBar *tb = (CWBTrackBar*)FindChildByIDs( _T( "logicdata" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      auto* o = Obj->GetObject();
      tb->SetValue( (unsigned char) o->camCenterY );
      tb->SetText( CString::Format( _T( "Logic Data: %d" ), (unsigned char)o->camCenterY ) );
    }
  }
  break;
  case Object_ParticleEmitterCPU:
  {
    CphxObject_ParticleEmitter_CPU_Tool *m = (CphxObject_ParticleEmitter_CPU_Tool*)Obj;
    CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)m->GetObject();

    CWBButton *b = (CWBButton*)FindChildByID( _T( "emittermaterial" ), _T( "button" ) );
    if ( b ) b->SetText( _T( "Select Material" ) );

    if ( m->Material )
    {
      CphxMaterial_Tool *mat = m->Material;

      if ( b ) b->SetText( mat->Name );

      for ( TS32 x = 0; x < mat->Techniques.NumItems(); x++ )
      {
        CphxMaterialTechnique_Tool *t = mat->Techniques[ x ];
        for ( TS32 y = 0; y < t->TechParameters.Parameters.NumItems(); y++ )
          AddMaterialParameterUI( &m->MaterialData, box, t->TechParameters.Parameters[ y ] );
        for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
          for ( TS32 z = 0; z < t->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ )
            AddMaterialParameterUI( &m->MaterialData, box, t->Passes[ y ]->PassParameters.Parameters[ z ] );
      }
    }

    b = (CWBButton*)FindChildByID( _T( "forcetarget" ), _T( "button" ) );
    if ( b && Obj->TargetObject )
      b->SetText( _T( "Target: " ) + Obj->TargetObject->GetName() );

    CWBItemSelector *is = (CWBItemSelector*)FindChildByID( _T( "emittershape" ), _T( "itemselector" ) );
    if ( is ) is->SelectItemByIndex( o->EmitterType );

    CWBTrackBar *tb = (CWBTrackBar*)FindChildByIDs( _T( "particlebuffer" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      tb->SetValue( o->BufferSize );
      tb->SetText( CString::Format( _T( "Max Particles: %d" ), 1 << o->BufferSize ) );
    }

    tb = (CWBTrackBar*)FindChildByIDs( _T( "innerradius" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      tb->SetValue( o->InnerRadius );
      tb->SetText( CString::Format( _T( "Inner Radius: %d" ), o->InnerRadius ) );
    }

    b = (CWBButton*)FindChildByID( _T( "agingparticles" ), _T( "button" ) );
    if ( b ) b->Push( o->Aging );

    b = (CWBButton*)FindChildByID( _T( "randomrotate" ), _T( "button" ) );
    if ( b ) b->Push( o->RandRotate );

    b = (CWBButton*)FindChildByID( _T( "mirrorrotation" ), _T( "button" ) );
    if ( b ) b->Push( o->TwoDirRotate );

    b = (CWBButton*)FindChildByID( _T( "rotatetodir" ), _T( "button" ) );
    if ( b ) b->Push( o->RotateToDirection );

    tb = (CWBTrackBar*)FindChildByIDs( _T( "startcount" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      tb->SetValue( o->StartCount );
      tb->SetText( CString::Format( _T( "Start Count: %d" ), ( ( 1 << o->BufferSize )*o->StartCount ) / 255 ) );
    }

    tb = (CWBTrackBar*)FindChildByIDs( _T( "randseed" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( tb )
    {
      tb->SetValue( o->RandSeed );
      tb->SetText( CString::Format( _T( "Randseed: %d" ), o->RandSeed ) );
    }

    b = (CWBButton*)FindChildByID( _T( "sortparticle" ), _T( "button" ) );
    if ( b ) b->Push( o->Sort );

    b = (CWBButton*)FindChildByID( _T( "modellist" ), _T( "button" ) );
    if ( b )
    {
      if ( !m->EmitedObject )
        b->SetText( "No Object Emitted" );
      else
        b->SetText( m->EmitedObject->GetName() );
    }

    b = (CWBButton*)FindChildByID( _T( "scenelist" ), _T( "button" ) );
    if ( b )
    {
      if ( !m->EmitedScene )
        b->SetText( "No Scene Emitted" );
      else
        b->SetText( m->EmitedScene->GetName() );
    }

    StyleManager.ApplyStyles( this );

    CWBMessage me;
    BuildPositionMessage( GetPosition(), me );
    me.Resized = true;
    MessageProc( me );
  }
  break;
  default:
    break;
  }

  StyleManager.ApplyStyles( this );
  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );

}

TBOOL CapexSceneObjectParameters::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
  {
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
              GetWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
              break;
            }
        }
        else
          WorkBench->GetEditedScene()->MarkCopyPaste();
      }
    }
    return true;
    break;
    case  'V':
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
              GetWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
              break;
            }
        }
        else
          break;
      }
    }
    return true;
    break;
    }

  }
  break;
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) return true;
    if ( !EditedObj ) return true;

    if ( b->GetID() == _T( "modellist" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );

      c->AddItem( "No object emitted", -1 );

      OpenModelContextMenu( c );

      return true;
    }

    if ( b->GetID() == _T( "scenelist" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );

      c->AddItem( "No scene emitted", -1 );

      OpenSceneContextMenu( c, GetActiveWorkBench()->GetEditedScene() );

      return true;
    }

    if ( b->GetID() == _T( "emittermaterial" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );
      for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
      {
        TBOOL ParticleMat = false;

        CphxMaterial_Tool *m = Project.GetMaterialByIndex( x );
        for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
          if ( m->Techniques[ y ]->Tech->Type == TECH_PARTICLE ) ParticleMat = true;

        if ( ParticleMat ) //only particle materials allowed here
          c->AddItem( Project.GetMaterialByIndex( x )->Name.GetPointer(), x );
      }

      return true;
    }

    if ( b->GetID() == _T( "swapmodel" ) )
    {
      if ( EditedObj->GetObjectType() == Object_Model )
      {
        auto* ctx = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );

        OpenModelContextMenu( ctx );
      }
      return true;
    }

    if ( b->GetID() == _T( "swapscene" ) )
    {
      if ( EditedObj->GetObjectType() == Object_SubScene )
      {
        auto* ctx = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );
        
        OpenSceneContextMenu( ctx, GetActiveWorkBench()->GetEditedScene() );
      }
      return true;
    }


    if ( b->GetID() == _T( "modellocator" ) )
    {
      if ( EditedObj->GetObjectType() == Object_Model )
      {
        CphxObject_Model_Tool *l = (CphxObject_Model_Tool*)EditedObj;
        Root->GoToModel( l->Model, !App->GetCtrlState() );
      }
      if ( EditedObj->GetObjectType() == Object_ParticleEmitterCPU )
      {
        CphxObject_ParticleEmitter_CPU_Tool *l = (CphxObject_ParticleEmitter_CPU_Tool*)EditedObj;
        Root->GoToModel( l->EmitedObject, !App->GetCtrlState() );
      }
      return true;
    }

    if ( b->GetID() == _T( "scenelocator" ) )
    {
      if ( EditedObj->GetObjectType() == Object_SubScene )
      {
        CphxObject_SubScene_Tool *l = (CphxObject_SubScene_Tool*)EditedObj;

        Root->GoToScene( l->GetClip( WorkBench->GetEditedScene()->GetActiveClip() )->GetSubSceneTarget(), !App->GetCtrlState() );
      }
      if ( EditedObj->GetObjectType() == Object_ParticleEmitterCPU )
      {
        CphxObject_ParticleEmitter_CPU_Tool *l = (CphxObject_ParticleEmitter_CPU_Tool*)EditedObj;

        Root->GoToScene( l->EmitedScene, !App->GetCtrlState() );
      }
      return true;
    }

    if ( b->GetID() == _T( "dotlight" ) )
    {
      if ( EditedObj->GetObjectType() == Object_Light )
      {
        CphxObject_Light_Tool *l = (CphxObject_Light_Tool*)EditedObj;
        b->Push( !b->IsPushed() );
        l->SetPointLight( b->IsPushed() );
      }
      return true;
    }

    if ( b->GetID() == _T( "limitedaffector" ) )
    {
      if ( EditedObj->GetObjectType() == Object_ParticleGravity )
      {
        CphxObject_ParticleGravity_Tool *l = (CphxObject_ParticleGravity_Tool*)EditedObj;
        CphxObject_ParticleGravity *o = (CphxObject_ParticleGravity*)EditedObj->GetObject();
        b->Push( !b->IsPushed() );
        o->AreaType = b->IsPushed();
      }
      if ( EditedObj->GetObjectType() == Object_ParticleDrag )
      {
        CphxObject_ParticleDrag_Tool *l = (CphxObject_ParticleDrag_Tool*)EditedObj;
        CphxObject_ParticleDrag *o = (CphxObject_ParticleDrag*)EditedObj->GetObject();
        b->Push( !b->IsPushed() );
        o->AreaType = b->IsPushed();
      }
      if ( EditedObj->GetObjectType() == Object_ParticleTurbulence )
      {
        CphxObject_ParticleTurbulence_Tool *l = (CphxObject_ParticleTurbulence_Tool*)EditedObj;
        CphxObject_ParticleTurbulence *o = (CphxObject_ParticleTurbulence*)EditedObj->GetObject();
        b->Push( !b->IsPushed() );
        o->AreaType = b->IsPushed();
      }
      if ( EditedObj->GetObjectType() == Object_ParticleVortex )
      {
        CphxObject_ParticleVortex_Tool* l = (CphxObject_ParticleVortex_Tool*)EditedObj;
        CphxObject_ParticleVortex* o = (CphxObject_ParticleVortex*)EditedObj->GetObject();
        b->Push( !b->IsPushed() );
        o->AreaType = b->IsPushed();
      }
      return true;
    }

    if ( b->GetID() == _T( "directionalgravity" ) )
    {
      if ( EditedObj->GetObjectType() == Object_ParticleGravity )
      {
        CphxObject_ParticleGravity_Tool *l = (CphxObject_ParticleGravity_Tool*)EditedObj;
        CphxObject_ParticleGravity *o = (CphxObject_ParticleGravity*)EditedObj->GetObject();
        b->Push( !b->IsPushed() );
        o->Directional = b->IsPushed() != 0;
      }
      return true;
    }

    if ( b->GetID() == _T( "lighttarget" ) )
    {
      if ( EditedObj->GetObjectType() != Object_Light ) return true;
      BuildObjSelectorContextMenu( b );
      return true;
    }

    if ( b->GetID() == _T( "camtarget" ) )
    {
      if ( EditedObj->GetObjectType() != Object_CamEye ) return true;
      BuildObjSelectorContextMenu( b );
      return true;
    }

    if ( b->GetID() == _T( "forcetarget" ) )
    {
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      BuildObjSelectorContextMenu( b );
      return true;
    }

    if ( b->GetID() == _T( "selectmaterial" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );
      for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
        c->AddItem( Project.GetMaterialByIndex( x )->Name.GetPointer(), x );

      return true;
    }

    if ( b->GetID() == _T( "textureselector" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      BuildTextureSelectorUI( b );
      return true;
    }

    if ( b->GetID() == _T( "texturelocator" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      if ( TextureLocatorMap.HasKey( b->GetGuid() ) )
      {
        CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;

        CphxGUID &g = o->MaterialData.MaterialTextures[ TextureLocatorMap[ b->GetGuid() ] ];
        CphxTextureOperator_Tool *op = Project.GetTexgenOp( g );
        if ( op )
          Root->GoToTexture( op, !App->GetCtrlState() );
      }
      return true;
    }

    if ( b->GetID() == _T( "wireframe" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      o->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe = !o->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe;
      b->Push( o->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe );
    }

    if ( b->GetID() == _T( "zenable" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = o->MaterialData.MaterialParams[ p->GetGUID() ].ZMode;
      TBOOL DepthEnable = !( bm & 1 );
      TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
      DepthEnable = !DepthEnable;
      bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
      o->MaterialData.MaterialParams[ p->GetGUID() ].ZMode = bm;
      b->Push( DepthEnable );
    }

    if ( b->GetID() == _T( "zwrite" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = o->MaterialData.MaterialParams[ p->GetGUID() ].ZMode;
      TBOOL DepthEnable = !( bm & 1 );
      TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
      ZWriteEnable = !ZWriteEnable;
      bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
      o->MaterialData.MaterialParams[ p->GetGUID() ].ZMode = bm;
      b->Push( ZWriteEnable );
    }

    if ( b->GetID() == _T( "agingparticles" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU*)o->GetObject();
      ob->Aging = !ob->Aging;
      b->Push( ob->Aging );
    }

    if ( b->GetID() == _T( "randomrotate" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU*)o->GetObject();
      ob->RandRotate = !ob->RandRotate;
      b->Push( ob->RandRotate );
    }

    if ( b->GetID() == _T( "mirrorrotation" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU*)o->GetObject();
      ob->TwoDirRotate = !ob->TwoDirRotate;
      b->Push( ob->TwoDirRotate );
    }

    if ( b->GetID() == _T( "rotatetodir" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU*)o->GetObject();
      ob->RotateToDirection = !ob->RotateToDirection;
      b->Push( ob->RotateToDirection );
    }


    if ( b->GetID() == _T( "sortparticle" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU*)o->GetObject();
      ob->Sort = !ob->Sort;
      b->Push( ob->Sort );
    }
  }
  break;

  case WBM_CONTEXTMESSAGE:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) return false;
    if ( !EditedObj ) return false;
    CphxScene_Tool *s = WorkBench->GetEditedScene();
    if ( !s ) return false;

    if ( b->GetID() == _T( "modellist" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *m = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxModel_Tool *model = Project.GetModelByIndex( Message.Data );

      m->SetEmittedModel( model );
      if ( m->EmitedObject ) b->SetText( m->EmitedObject->GetName() );
      else b->SetText( "No object emitted" );
      SelectSceneObject( EditedObj );

      WorkBench->UpdateWindows( apEx_SceneSplineEditor );
      WorkBench->UpdateWindows( apEx_SceneObjectParameters );

      return true;
    }

    if ( b->GetID() == _T( "scenelist" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *m = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxScene_Tool *model = Project.GetSceneByIndex( Message.Data );

      m->SetEmittedScene( model );
      if ( m->EmitedScene ) b->SetText( m->EmitedScene->GetName() );
      else b->SetText( "No scene emitted" );
      SelectSceneObject( EditedObj );

      WorkBench->UpdateWindows( apEx_SceneSplineEditor );
      WorkBench->UpdateWindows( apEx_SceneObjectParameters );

      return true;
    }

    if ( b->GetID() == _T( "emittermaterial" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *m = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxMaterial_Tool *mat = Project.GetMaterialByIndex( Message.Data );

      if ( mat == m->GetMaterial() ) break;

      m->SetMaterial( mat );
      if ( m->Material ) b->SetText( m->Material->Name );
      SelectSceneObject( EditedObj );
      return true;
    }

    if ( b->GetID() == _T( "lighttarget" ) )
    {
      if ( EditedObj->GetObjectType() != Object_Light ) return true;

      CphxObject_Tool *o = s->GetObjectByIndex( Message.Data );
      EditedObj->SetTarget( o );

      if ( !o ) b->SetText( _T( "Select Light Target" ) );
      else b->SetText( _T( "Target: " ) + o->GetName() );

      return true;
    }

    if ( b->GetID() == _T( "camtarget" ) )
    {
      if ( EditedObj->GetObjectType() != Object_CamEye ) return true;

      CphxObject_Tool *o = s->GetObjectByIndex( Message.Data );
      EditedObj->SetTarget( o );

      if ( !o ) b->SetText( _T( "Select Camera Target" ) );
      else b->SetText( _T( "Target: " ) + o->GetName() );

      return true;
    }

    if ( b->GetID() == _T( "forcetarget" ) )
    {
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;

      CphxObject_Tool *o = s->GetObjectByIndex( Message.Data );
      EditedObj->SetTarget( o );

      if ( !o ) b->SetText( _T( "Select Force Target" ) );
      else b->SetText( _T( "Target: " ) + o->GetName() );

      return true;
    }

    if ( b->GetID() == _T( "selectmaterial" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *m = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxMaterial_Tool *mat = Project.GetMaterialByIndex( Message.Data );

      if ( mat == m->GetMaterial() ) break;

      m->SetMaterial( mat );
      if ( m->Material ) b->SetText( m->Material->Name );
      SelectSceneObject( EditedObj );
    }

    if ( b->GetID() == _T( "textureselector" ) )
    {
      if ( TextureIDMap.HasKey( Message.Data ) )
      {
        if ( !EditedObj ) return true;
        if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
        CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;

        CphxGUID g = TextureIDMap[ Message.Data ];
        CphxMaterialParameter_Tool *p = GetTargetParam( b );
        if ( !p ) return true;
        o->MaterialData.MaterialTextures[ p->GetGUID() ] = g;

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

    if ( b->GetID() == _T( "swapmodel" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_Model ) return true;
      CphxObject_Model_Tool *m = (CphxObject_Model_Tool*)EditedObj;
      m->SwapModel( Project.GetModelByIndex( Message.Data ) );
      WorkBench->UpdateWindows( apEx_SceneSplineEditor );

      CWBButton *b = (CWBButton*)FindChildByID( _T( "swapmodel" ), _T( "button" ) );
      if ( b && m->Model )
        b->SetText( m->Model->GetName() );

      WorkBench->UpdateWindows( apEx_ModelList );

      return true;
    }

    if ( b->GetID() == _T( "swapscene" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_SubScene ) return true;
      CphxObject_SubScene_Tool *m = (CphxObject_SubScene_Tool*)EditedObj;
      auto* scene = Project.GetSceneByIndex( Message.Data );

      m->SwapScene( scene, WorkBench->GetEditedScene()->GetActiveClip() );
      WorkBench->UpdateWindows( apEx_SceneSplineEditor );

      CWBButton *b = (CWBButton*)FindChildByID( _T( "swapscene" ), _T( "button" ) );
      if ( b && scene )
        b->SetText( scene->GetName() );

      WorkBench->UpdateWindows( apEx_SceneList );

      return true;
    }

    break;
  }

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *b = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "logictype" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_LogicObject ) return true;
      auto *o = EditedObj->GetObject();
      o->camCenterX = b->GetItemIndex( Message.Data );
      return true;
    }

    if ( b->GetID() == _T( "srcblend" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = o->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode;
      bm = b->GetCursorPosition() | ( bm & ~15 );
      o->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode = bm;
      return true;
    }

    if ( b->GetID() == _T( "dstblend" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = o->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode;
      bm = ( b->GetCursorPosition() << 4 ) | ( bm & 15 );
      o->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode = bm;
      return true;
    }

    if ( b->GetID() == _T( "culllist" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      o->MaterialData.MaterialParams[ p->GetGUID() ].CullMode = (D3D11_CULL_MODE)( b->GetCursorPosition() + 1 );
      return true;
    }

    if ( b->GetID() == _T( "zfunctlist" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      o->MaterialData.MaterialParams[ p->GetGUID() ].ZFunction = (D3D11_COMPARISON_FUNC)( b->GetCursorPosition() + 1 );
      return true;
    }

    if ( b->GetID() == _T( "emittershape" ) )
    {
      if ( !EditedObj ) return true;
      if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
      CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
      CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU *)EditedObj->GetObject();
      ob->EmitterType = b->GetItemIndex( Message.Data );
      return true;
    }

    break;
  }

  case WBM_TEXTCHANGED:
  {
    if ( !EditedObj ) return true;

    CWBTextBox *b = (CWBTextBox *)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( !b ) break;

    if ( EditedObj->GetObjectType() == Object_ParticleTurbulence )
    {
      CphxObject_ParticleTurbulence_Tool *l = (CphxObject_ParticleTurbulence_Tool*)EditedObj;
      CphxObject_ParticleTurbulence *o = (CphxObject_ParticleTurbulence*)EditedObj->GetObject();

      int val = 0;

      if ( b->GetText().Scan( "%d", &val ) == 1 )
      {
        val = max( 0, min( 255, val ) );
        auto* scene = WorkBench->GetEditedScene();
        if ( scene )
        {
          auto id = scene->GetActiveClip();
          auto clip = l->GetClip( id );
          clip->SetTurbulenceFreq( val );
          if ( CString::Format( "%d", val ) != b->GetText() )
            b->SetText( CString::Format( "%d", val ) );
        }

        o->TurbulenceFrequency = val;
        l->RebuildMinimalData();
      }

      return true;
    }
  }
  break;

  case WBM_VALUECHANGED:
  {
    if ( !EditedObj ) return true;

    CWBTrackBar *b = (CWBTrackBar *)App->FindItemByGuid( Message.GetTarget(), _T( "trackbar" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "camcenterx" ) )
    {
      EditedObj->GetObject()->camCenterX = Message.Data;
      b->SetText( CString::Format( _T( "Camera Center X: %f" ), EditedObj->GetObject()->camCenterX / 127.0f ) );
      return true;
    }

    if ( b->GetID() == _T( "camcentery" ) )
    {
      EditedObj->GetObject()->camCenterY = Message.Data;
      b->SetText( CString::Format( _T( "Camera Center Y: %f" ), EditedObj->GetObject()->camCenterY / 127.0f ) );
      return true;
    }

    if ( EditedObj->GetObjectType() == Object_ParticleTurbulence )
    {
      CphxObject_ParticleTurbulence_Tool *l = (CphxObject_ParticleTurbulence_Tool*)EditedObj;
      CphxObject_ParticleTurbulence *o = (CphxObject_ParticleTurbulence*)EditedObj->GetObject();

      auto* scene = WorkBench->GetEditedScene();
      if ( scene )
      {
        auto id = scene->GetActiveClip();
        auto clip = l->GetClip( id );
        clip->SetRandSeed(Message.Data);
      }

      o->RandSeed = Message.Data;
      l->RebuildMinimalData();
      o->InitKernel();
      b->SetText( CString::Format( _T( "Randseed: %d" ), Message.Data ) );
    }

    if ( b->GetID() == _T( "logicdata" ) )
    {
      auto *o = EditedObj->GetObject();
      o->camCenterY = Message.Data;
      b->SetText( CString::Format( _T( "Logic Data: %d" ), Message.Data ) );
      return true;
    }

    if ( EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return true;
    CphxObject_ParticleEmitter_CPU_Tool *o = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
    CphxObject_ParticleEmitter_CPU *ob = (CphxObject_ParticleEmitter_CPU *)EditedObj->GetObject();

    CphxMaterialParameter_Tool *p = GetTargetParam( b );

    if ( p )
    {

      if ( b->GetID() == _T( "red" ) )
      {
        o->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 0 ] = Message.Data / 255.0f;
        b->SetText( CString::Format( _T( "%s Red: %d" ), p->Name.GetPointer(), Message.Data ) );
        return true;
      }

      if ( b->GetID() == _T( "green" ) )
      {
        o->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 1 ] = Message.Data / 255.0f;
        b->SetText( CString::Format( _T( "%s Green: %d" ), p->Name.GetPointer(), Message.Data ) );
        return true;
      }

      if ( b->GetID() == _T( "blue" ) )
      {
        o->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 2 ] = Message.Data / 255.0f;
        b->SetText( CString::Format( _T( "%s Blue: %d" ), p->Name.GetPointer(), Message.Data ) );
        return true;
      }

      if ( b->GetID() == _T( "alpha" ) )
      {
        o->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 3 ] = Message.Data / 255.0f;
        b->SetText( CString::Format( _T( "%s Alpha: %d" ), p->Name.GetPointer(), Message.Data ) );
        return true;
      }

      if ( b->GetID() == _T( "float" ) )
      {
        o->MaterialData.MaterialParams[ p->GetGUID() ].Float = Message.Data / 255.0f;
        b->SetText( CString::Format( _T( "%s: %d" ), p->Name.GetPointer(), Message.Data ) );
        return true;
      }

      if ( b->GetID() == _T( "renderpriority" ) )
      {
        o->MaterialData.MaterialParams[ p->GetGUID() ].RenderPriority = Message.Data;
        b->SetText( CString::Format( _T( "Render Priority: %d" ), Message.Data ) );
        return true;
      }
    }

    if ( b->GetID() == _T( "particlebuffer" ) )
    {
      if ( ob->BufferSize != Message.Data )
      {
        ob->BufferSize = Message.Data;
        o->UpdateParticleResources( App->GetDevice() );
      }
      b->SetText( CString::Format( _T( "Max Particles: %d" ), 1 << Message.Data ) );

      CWBTrackBar *bb = (CWBTrackBar*)FindChildByIDs( _T( "startcount" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( bb ) bb->SetText( CString::Format( _T( "Start Count: %d" ), ( ( 1 << ob->BufferSize )*ob->StartCount ) / 255 ) );

      return true;
    }

    if ( b->GetID() == _T( "innerradius" ) )
    {
      if ( ob->InnerRadius != Message.Data )
      {
        ob->InnerRadius = Message.Data;
        //o->UpdateParticleResources(App->GetDevice());
      }
      b->SetText( CString::Format( _T( "Inner Radius: %d" ), Message.Data ) );

      return true;
    }

    if ( b->GetID() == _T( "startcount" ) )
    {
      if ( ob->StartCount != Message.Data )
      {
        ob->StartCount = Message.Data;
        o->UpdateParticleResources( App->GetDevice() );
      }
      b->SetText( CString::Format( _T( "Start Count: %d" ), ( ( 1 << ob->BufferSize )*Message.Data ) / 255 ) );
      return true;
    }

    if ( b->GetID() == _T( "randseed" ) )
    {
      if ( ob->RandSeed != Message.Data )
      {
        ob->RandSeed = Message.Data;
        o->UpdateParticleResources( App->GetDevice() );
      }
      b->SetText( CString::Format( _T( "Randseed: %d" ), Message.Data ) );
      return true;
    }

  }

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexSceneObjectParameters::BuildObjSelectorContextMenu( CWBItem *Button )
{
  CphxScene_Tool *s = WorkBench->GetEditedScene();
  if ( !s ) return;

  CWBContextMenu *m = Button->OpenContextMenu( Button->ClientToScreen( Button->GetClientRect() ).BottomLeft() );

  m->AddItem( _T( "No Target" ), -1 );

  for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
    m->AddItem( s->GetObjectByIndex( x )->GetName().GetPointer(), x );

}

void CapexSceneObjectParameters::AddMaterialParameterUI( CphxMaterialDataStorage_Tool *matdat, CWBItem *Root, CphxMaterialParameter_Tool *Param )
{
  if ( Param->Parameter.Scope != PARAM_VARIABLE ) return;
  //if (!EditedObject) return;
  //if (EditedObject->GetPrimitive() == Mesh_Clone) return;
  //CphxModelObject_Tool_Mesh *editedmesh = (CphxModelObject_Tool_Mesh *)EditedObject;

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
      if ( matdat->MaterialTextures.HasKey( Param->GetGUID() ) )
      {
        CphxGUID &g = matdat->MaterialTextures[ Param->GetGUID() ];
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
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].BlendMode = phxSrcBlend_ONE | phxDstBlend_ZERO;

    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
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
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      for ( TS32 c = 0; c < 4; c++ )
        matdat->MaterialParams[ Param->GetGUID() ].Color[ c ] = 0.25;

    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByIDs( _T( "red" ), _T( "trackbar" ), _T( "numpad" ) );
    CWBTrackBar *g = (CWBTrackBar*)nb->FindChildByIDs( _T( "green" ), _T( "trackbar" ), _T( "numpad" ) );
    CWBTrackBar *b = (CWBTrackBar*)nb->FindChildByIDs( _T( "blue" ), _T( "trackbar" ), _T( "numpad" ) );
    CWBTrackBar *a = (CWBTrackBar*)nb->FindChildByIDs( _T( "alpha" ), _T( "trackbar" ), _T( "numpad" ) );

    if ( r ) r->SetValue( (TS32)( v.Color[ 0 ] * 255 + 0.5f ) );
    if ( g ) g->SetValue( (TS32)( v.Color[ 1 ] * 255 + 0.5f ) );
    if ( b ) b->SetValue( (TS32)( v.Color[ 2 ] * 255 + 0.5f ) );
    if ( a ) a->SetValue( (TS32)( v.Color[ 3 ] * 255 + 0.5f ) );

    break;
  }

  case PARAM_FLOAT:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "floattemplate" ) ) );
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].Float = 0;

    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByIDs( _T( "float" ), _T( "trackbar" ), _T( "numpad" ) );

    if ( r ) r->SetValue( (TS32)( v.Float * 255 + 0.5f ) );
    break;
  }

  case PARAM_RENDERPRIORITY:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "prioritytemplate" ) ) );
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].RenderPriority = 127;

    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByIDs( _T( "renderpriority" ), _T( "trackbar" ), _T( "numpad" ) );

    if ( r )
      r->SetValue( v.RenderPriority );
    break;
  }

  case PARAM_CULLMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "culltemplate" ) ) );
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].CullMode = D3D11_CULL_NONE;
    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *cull = (CWBItemSelector*)nb->FindChildByID( _T( "culllist" ), _T( "itemselector" ) );

    if ( cull )
      cull->SelectItemByIndex( v.CullMode - 1 );
    break;
  }

  case PARAM_FILLMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "filltemplate" ) ) );
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].Wireframe = false;
    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
    CWBButton *wf = (CWBButton*)nb->FindChildByID( _T( "wireframe" ), _T( "button" ) );
    if ( wf )
      wf->Push( v.Wireframe );
    break;
  }

  case PARAM_ZFUNCTION:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "zfuncttemplate" ) ) );
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].ZFunction = D3D11_COMPARISON_LESS;
    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *zf = (CWBItemSelector*)nb->FindChildByID( _T( "zfunctlist" ), _T( "itemselector" ) );

    if ( zf )
      zf->SelectItemByIndex( v.ZFunction - 1 );
    break;
  }

  case PARAM_ZMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "zmodetemplate" ) ) );
    if ( !matdat->MaterialParams.HasKey( Param->GetGUID() ) )
      matdat->MaterialParams[ Param->GetGUID() ].ZMode = 0;
    MATERIALVALUE v = matdat->MaterialParams[ Param->GetGUID() ];

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

void CapexSceneObjectParameters::BuildTextureSelectorUI( CWBItem *Button )
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
      it->AddItem( Project.GetRenderTargetByIndex( x )->Name.GetPointer(), id );
      TextureIDMap[ id ] = Project.GetRenderTargetByIndex( x )->GetGUID();
      id++;
    }

    c->AddSeparator();
  }

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

CphxMaterialParameter_Tool * CapexSceneObjectParameters::GetTargetParam( CWBItem *i )
{
  if ( !EditedObj || EditedObj->GetObjectType() != Object_ParticleEmitterCPU ) return NULL;
  CphxObject_ParticleEmitter_CPU_Tool *m = (CphxObject_ParticleEmitter_CPU_Tool *)EditedObj;
  if ( !m->Material ) return NULL;
  CphxMaterial_Tool *mat = m->Material;

  for ( TS32 x = 0; x < ParamUIMap.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CWBItem *>::KDPair *kdp = ParamUIMap.GetKDPair( x );
    if ( i->FindItemInParentTree( kdp->Data ) )
      return mat->GetParameter( kdp->Key );
  }

  return NULL;
}
