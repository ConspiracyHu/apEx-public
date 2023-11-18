#include "BasePCH.h"
#include "SceneGraph.h"
#define WINDOWNAME _T("Scene Graph")
#define WINDOWXML _T("SceneGraph")
#include "WorkBench.h"
#include "SceneObjectParameters.h"
#include "ExtendedList.h"

CapexSceneGraph::CapexSceneGraph() : CapexWindow()
{
}

CapexSceneGraph::CapexSceneGraph( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexSceneGraph::~CapexSceneGraph()
{

}

void CapexSceneGraph::SceneGraphBuilder( CphxObject_Tool *Parent, TS32 Level )
{
  CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
  if ( !b ) return;
  if ( !WorkBench->GetEditedScene() ) return;
  CphxScene_Tool *m = WorkBench->GetEditedScene();

  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    CphxObject_Tool *t = m->GetObjectByIndex( x );
    if ( t->GetParentObject() == Parent )
    {
      TS32 i = b->AddItemWithLevel( t->GetName(), Level );
      ObjectMap[ i ] = t;
      b->GetItem( i )->Select( m->GetObjectByIndex( x )->Selected );
      //if (m->GetObjectByIndex(x)->Selected) cursor = x;
      SceneGraphBuilder( t, Level + 1 );
    }
  }


}

void CapexSceneGraph::UpdateData()
{
  ObjectMap.Flush();

  CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
  if ( !b ) return;

  TS32 cursor = -1;// b->GetCursorPosition();
  b->Flush();

  if ( !WorkBench->GetEditedScene() ) return;
  CphxScene_Tool *m = WorkBench->GetEditedScene();

  SceneGraphBuilder( NULL, 0 );

  //b->SetCursorPosition(cursor);

}

TBOOL CapexSceneGraph::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_RIGHTBUTTONDOWN:
  {
    ListRightClicked = false;
    CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
    if ( !b ) break;
    if ( App->GetMouseItem() == b )
    {
      RightClickedItem = b->GetMouseItemID( Message.GetPosition() );
      App->SetCapture( this );
      ListRightClicked = true;
      return true;
    }
    break;
  }
  case WBM_RIGHTBUTTONUP:
  {
    if ( ListRightClicked )
    {
      ListRightClicked = false;
      CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
      if ( !b ) break;
      if ( b->GetMouseItemID( Message.GetPosition() ) == RightClickedItem )
      {
        if ( !WorkBench->GetEditedScene() ) return true;
        CphxScene_Tool *m = WorkBench->GetEditedScene();

        if ( ObjectMap.HasKey( RightClickedItem ) )
        {
          m->LinkSelectedTo( ObjectMap[ RightClickedItem ], App->GetCtrlState() );
          WorkBench->UpdateWindows( apEx_SceneGraph );
        }
        else
        {
          m->LinkSelectedTo( NULL, App->GetCtrlState() );
          WorkBench->UpdateWindows( apEx_SceneGraph );
        }
      }

      App->ReleaseCapture();

      return true;
    }
    break;
  }
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
  case WBM_ITEMSELECTED:
  {
    CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
    if ( !b ) break;
    CphxScene_Tool *m = WorkBench->GetEditedScene();
    if ( !m ) break;

    if ( ObjectMap.HasKey( Message.Data ) )
    {
      CapexSceneObjectParameters* w = (CapexSceneObjectParameters*)WorkBench->GetWindow( apEx_SceneObjectParameters );
      if ( w ) w->SelectSceneObject( ObjectMap[ Message.Data ] );
      GetWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
      return true;
    }

    break;
  }
  case WBM_SELECTIONCHANGE:
  {
    CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
    if ( !b ) break;

    CphxScene_Tool *m = WorkBench->GetEditedScene();
    if ( !m ) break;

    for ( TS32 x = 0; x < b->NumItems(); x++ )
    {
      if ( ObjectMap.HasKey( b->GetItemByIndex( x ).GetID() ) )
        ObjectMap[ b->GetItemByIndex( x ).GetID() ]->Selected = b->GetItemByIndex( x ).IsSelected();
    }
    GetWorkBench()->UpdateWindows( apEx_SceneSplineEditor );
  }
  break;
  case WBM_ITEMRENAMED:
  {
    CapexList *b = (CapexList*)FindChildByID( _T( "objectlist" ), _T( "extendedlist" ) );
    if ( !b ) break;
    CphxScene_Tool *m = WorkBench->GetEditedScene();
    if ( !m ) break;

    if ( ObjectMap.HasKey( Message.Data ) )
    {
      ObjectMap[ Message.Data ]->SetName( b->GetItem( Message.Data )->GetText() );
      return true;
    }

  }
  break;
  }


  return CapexWindow::MessageProc( Message );
}
