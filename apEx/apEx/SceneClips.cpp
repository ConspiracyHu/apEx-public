#include "BasePCH.h"
#include "SceneClips.h"
#define WINDOWNAME _T("Scene Clips")
#define WINDOWXML _T("SceneClips")
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"

CapexSceneClips::CapexSceneClips() : CapexWindow()
{
}

CapexSceneClips::CapexSceneClips( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexSceneClips::~CapexSceneClips()
{

}

void CapexSceneClips::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
  if ( !List ) return;

  TS32 Cursor = List->GetCursorPosition();

  List->Flush();

  ClearRequiredFlagForAllResources();
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
  if ( !editedscene ) return;

  for ( TS32 x = 0; x < editedscene->GetClipCount(); x++ )
  {
    bool usedScene = editedscene->GetClipByIndex( x )->GetChildCount( PHX_EVENT ) != 0;
    bool usedSubScene = !usedScene && editedscene->GetClipByIndex(x)->IsRequired();
    auto itemId = List->AddItem( editedscene->GetClipByIndex( x )->GetName() );

    if ( usedSubScene )
      List->SetItemColor( itemId, CColor( 51, 255, 173, 255 ) );
    if ( usedScene )
      List->SetItemColor( itemId, CColor( 255, 255, 154, 255 ) );
  }

  List->SelectItemByIndex( editedscene->GetActiveClip() );
}

TBOOL CapexSceneClips::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "newclip" ) )
    {
      CreateClip();

      return true;
    }

    if ( b->GetID() == _T( "copyclip" ) )
    {
      CopyClip();
      return true;
    }

    if ( b->GetID() == _T( "deleteclip" ) )
    {
      CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
      if ( List ) DeleteClip( List->GetCursorPosition() );
      return true;
    }

  }
  break;

  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
      if ( !editedscene ) return true;

      CphxSceneClip *s = editedscene->GetClipByIndex( List->GetCursorPosition() );
      if ( s ) s->SetName( List->GetCursorItem()->GetText() );
      return true;
    }
  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
      if ( !editedscene ) return true;

      editedscene->SetActiveClip( List->GetCursorPosition() );
      WorkBench->UpdateWindows( apEx_SceneSplineEditor );
      WorkBench->UpdateWindows( apEx_SceneObjectParameters );
      return true;
    }

  }
  break;

  default:
    break;
  }


  return CapexWindow::MessageProc( Message );
}

void CapexSceneClips::CreateClip()
{
  CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
  if ( !editedscene ) return;

  editedscene->AddClip();
  editedscene->RebuildMinimalData();
  editedscene->SetActiveClip( editedscene->GetClipCount() - 1 );

  CphxSceneClip *t = editedscene->GetClipByIndex( editedscene->GetActiveClip() );

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->AddItem( t->GetName() );
    SelectClip( List->NumItems() - 1 );
  }
}

void CapexSceneClips::CopyClip()
{
  CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
  if ( !editedscene ) return;

  CphxSceneClip *o = editedscene->GetClipByIndex( editedscene->GetActiveClip() );

  editedscene->CopyClip( o->GetGUID() );
  editedscene->RebuildMinimalData();
  editedscene->SetActiveClip( editedscene->GetClipCount() - 1 );

  CphxSceneClip *t = editedscene->GetClipByIndex( editedscene->GetActiveClip() );

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->AddItem( t->GetName() );
    SelectClip( List->NumItems() - 1 );
  }
}

void CapexSceneClips::DeleteClip( TS32 ID )
{
  CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
  if ( !editedscene ) return;
  if ( editedscene->GetClipCount() <= 1 )
  {
    SetStatusbarText( _T( "A Scene must always have at least one clip. Last one can't be deleted." ) );
    return;
  }
}

void CapexSceneClips::SelectClip( TS32 ID )
{
  CphxScene_Tool *editedscene = WorkBench->GetEditedScene();
  if ( !editedscene ) return;

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "cliplist" ), _T( "itemselector" ) );
  if ( !List ) return;

  List->SelectItemByIndex( ID );
  editedscene->SetActiveClip( ID );
  WorkBench->UpdateWindows( apEx_SceneSplineEditor );
  WorkBench->UpdateWindows( apEx_SceneObjectParameters );
}
