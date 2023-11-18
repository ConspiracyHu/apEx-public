#include "BasePCH.h"
#include "ScenePrimitives.h"
#define WINDOWNAME _T("Scene Primitives")
#define WINDOWXML _T("ScenePrimitives")
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"
#include "SceneList.h"
#include "ModelList.h"
#include "SceneGraph.h"
#include "SceneObjectParameters.h"

CapexScenePrimitives::CapexScenePrimitives() : CapexWindow()
{
}

CapexScenePrimitives::CapexScenePrimitives( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexScenePrimitives::~CapexScenePrimitives()
{

}

void CapexScenePrimitives::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( List )
  {
    TS32 Cursor = List->GetCursorPosition();

    List->Flush();

    for ( TS32 x = 0; x < Project.GetModelCount(); x++ )
      List->AddItem( Project.GetModelByIndex( x )->GetName() );

    List->SelectItemByIndex( Cursor );
  }

  SubSceneMap.Flush();

  CphxScene_Tool *currentscene = WorkBench->GetEditedScene();

  List = (CWBItemSelector*)FindChildByID( _T( "subscenelist" ), _T( "itemselector" ) );
  if ( List )
  {
    TS32 Cursor = List->GetCursorPosition();

    List->Flush();
    if ( !currentscene ) return;

    for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
    {
      if ( currentscene->CanContain( Project.GetSceneByIndex( x ) ) )
      {
        TS32 id = List->AddItem( Project.GetSceneByIndex( x )->GetName() );
        SubSceneMap += Project.GetSceneByIndex( x );
      }
    }

    List->SelectItemByIndex( Cursor );
  }


}

TBOOL CapexScenePrimitives::MessageProc( CWBMessage &Message )
{
  CphxScene_Tool *Scene = WorkBench->GetEditedScene();

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
    if ( !b || !Scene ) break;
    if ( b->GetID() == _T( "addmodel" ) ) { CreateObject( Object_Model ); return true; }
    if ( b->GetID() == _T( "addlight" ) )
    {
      TS32 lc = 0;
      for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
        if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_Light ) lc++;

      if ( lc < MAX_LIGHT_COUNT )
        CreateObject( Object_Light );
      else
        SetStatusbarText( _T( "Cannot place new light: Maximum number of lights reached." ) );
      return true;
    }
    if ( b->GetID() == _T( "adddummy" ) ) { CreateObject( Object_Dummy ); return true; }
    if ( b->GetID() == _T( "addcamera" ) ) { CreateObject( Object_CamEye ); return true; }
    if ( b->GetID() == _T( "addsubscene" ) ) { CreateObject( Object_SubScene ); return true; }
    if ( b->GetID() == _T( "addemittercpu" ) ) { CreateObject( Object_ParticleEmitterCPU ); return true; }
    if ( b->GetID() == _T( "adddrag" ) ) { CreateObject( Object_ParticleDrag ); return true; }
    if ( b->GetID() == _T( "addgravity" ) ) { CreateObject( Object_ParticleGravity ); return true; }
    if ( b->GetID() == _T( "addturbulence" ) ) { CreateObject( Object_ParticleTurbulence ); return true; }
    if ( b->GetID() == _T( "addvortex" ) ) { CreateObject( Object_ParticleVortex ); return true; }
    if ( b->GetID() == _T( "addlogicobject" ) ) { CreateObject( Object_LogicObject ); return true; }

    break;
  }

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexScenePrimitives::CreateObject( PHXOBJECT Objtype )
{
  CapexWorkBench *wb = GetWorkBench();
  CphxScene_Tool *Scene = NULL;
  if ( wb ) Scene = wb->GetEditedScene();
  if ( !Scene ) return;

  void *Data = NULL;

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( List && Objtype == Object_Model )
  {
    Data = Project.GetModelByIndex( List->GetCursorPosition() );
    if ( !Data )
      return;
  }

  List = (CWBItemSelector*)FindChildByID( _T( "subscenelist" ), _T( "itemselector" ) );
  if ( List && Objtype == Object_SubScene )
  {
    if ( List->GetCursorPosition() >= 0 && List->GetCursorPosition() < SubSceneMap.NumItems() )
      Data = SubSceneMap[ List->GetCursorPosition() ];
    if ( !Data )
      return;
  }

  CphxObject_Tool *o = Scene->AddObject( Objtype, Data );
  if ( !o ) return;

  GetActiveWorkBench()->UpdateWindows( apEx_SceneList );
  GetActiveWorkBench()->UpdateWindows( apEx_ModelList );

  CapexSceneGraph *mg = (CapexSceneGraph *)WorkBench->GetWindow( apEx_SceneGraph );
  if ( !mg ) return;

  //if (p == Mesh_Copy)
  //{
  //	CapexSceneParameters *mp = (CapexSceneParameters *)WorkBench->GetWindow(apEx_SceneParameters);
  //	if (!mp || !mp->GetEditedObj()) return;
  //	TS32 i = Scene->GetObjectIndex(mp->GetEditedObj());
  //	if (i >= 0)
  //	{
  //		o->ParentGUIDS[0] = mp->GetEditedObj()->GetGUID();
  //		o->AddParent(mp->GetEditedObj());
  //		o->GetParameters()[0] = i;
  //	}
  //}

  mg->UpdateData();
  CWBItemSelector *b = (CWBItemSelector*)mg->FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
  if ( !b ) return;
  b->SelectItemByIndex( b->NumItems() - 1 );
}

