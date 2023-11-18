#include "BasePCH.h"
#include "SceneList.h"
#define WINDOWNAME _T("Scene List")
#define WINDOWXML _T("SceneList")
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"
#include "../Phoenix_Tool/Scene_tool.h"
#include "SceneObjectParameters.h"

CapexSceneList::CapexSceneList() : CapexWindow()
{
}

CapexSceneList::CapexSceneList( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexSceneList::~CapexSceneList()
{

}

void CapexSceneList::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "scenelist" ), _T( "itemselector" ) );
  if ( !List ) return;

  TS32 Cursor = List->GetCursorPosition();

  List->Flush();

  bool found = false;

  for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
  {
    auto *scene = Project.GetSceneByIndex( x );

    bool subScene = scene->GetChildCount( PHX_OBJECT ) != 0 || scene->GetWeakChildCount( PHX_OBJECT ) != 0;
    bool usedScene = scene->GetChildCount( PHX_EVENT ) != 0;
    auto itemId = List->AddItem( scene->GetName());
    if ( subScene )
      List->SetItemColor( itemId, CColor( 51, 255, 173, 255 ) );
    if ( usedScene )
      List->SetItemColor( itemId, CColor( 255, 255, 154, 255 ) );

    if ( GetWorkBench()->GetEditedScene() == Project.GetSceneByIndex( x ) )
    {
      List->SelectItemByIndex( x );
      found = true;
    }
  }

  if ( !found )
    List->SelectItemByIndex( Cursor );

}

TBOOL CapexSceneList::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "newscene" ) )
    {
      CreateScene();

      return true;
    }

    if ( b->GetID() == _T( "copyscene" ) )
    {
      CopyScene( WorkBench->GetEditedScene() );

      return true;
    }

    if ( b->GetID() == _T( "deletescene" ) )
    {
      CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "scenelist" ), _T( "itemselector" ) );
      if ( List )
        DeleteScene( List->GetCursorPosition() );
      return true;
    }

  }
  break;

  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "scenelist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      CphxScene_Tool *t = Project.GetSceneByIndex( List->GetCursorPosition() );
      if ( t ) t->SetName( List->GetCursorItem()->GetText() );
      WorkBench->UpdateWindows( apEx_ScenePrimitives );
      return true;
    }
  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "scenelist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      WorkBench->SetEditedScene( Project.GetSceneByIndex( List->GetCursorPosition() ) );
      WorkBench->UpdateWindows( apEx_ScenePrimitives );
      return true;
    }

  }
  break;

  case WBM_KEYDOWN:
  {
    switch ( Message.Key )
    {
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

  default:
    break;
  }


  return CapexWindow::MessageProc( Message );
}

CphxScene_Tool *CapexSceneList::CreateScene()
{
  CphxScene_Tool *t = Project.CreateScene();
  t->AddClip();
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "scenelist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->AddItem( t->GetName() );
    SelectScene( List->NumItems() - 1 );
  }
  return t;
}

void CapexSceneList::DeleteScene( TS32 ID )
{
  CphxScene_Tool *s = Project.GetSceneByIndex( ID );
  if ( s )
  {
    if ( s->GetChildCount() )
    {
      SetStatusbarText( _T( "WARNING: Scene could not be deleted due to dependency issues." ) );
      return;
    }
    Project.DeleteScene( s->GetGUID() );
  }
  UpdateData();
  SelectScene( min( Project.GetSceneCount() - 1, ID ) );
}

void CapexSceneList::SelectScene( TS32 ID )
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "scenelist" ), _T( "itemselector" ) );
  if ( !List ) return;

  List->SelectItemByIndex( ID );
  //List->SetCursorPosition(ID);
  WorkBench->SetEditedScene( Project.GetSceneByIndex( ID ) );
}

void CapexSceneList::CopyScene( CphxScene_Tool *source )
{
  CphxScene_Tool *ns = CreateScene();
  ns->CopyFrom( source );
  UpdateData();
}
