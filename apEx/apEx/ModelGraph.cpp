#include "BasePCH.h"
#include "ModelGraph.h"
#define WINDOWNAME _T("Model Graph")
#define WINDOWXML _T("ModelGraph")
#include "WorkBench.h"
#include "../Phoenix_Tool/Model_Tool.h"
#include "ModelParameters.h"
#include "ModelMatrix.h"

TCHAR *ModelFilterNames[] =
{
  _T( "UV Map" ),
  _T( "Bevel" ),
  _T( "Map XForm" ),
  _T( "Mesh Smooth" ),
  _T( "Smooth Group" ),
  _T( "Tint" ),
  _T( "Tint With Shape" ),
  _T( "Replicate" ),
  _T( "Normal Deform" ),
  _T( "Bool" ),
  _T( "Greeble" ),
};

CapexModelGraph::CapexModelGraph() : CapexWindow()
{
}

CapexModelGraph::CapexModelGraph( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexModelGraph::~CapexModelGraph()
{

}

void CapexModelGraph::UpdateData()
{
  ModelObjects.Flush();
  ModelFilters.Flush();

  CWBItemSelector *b = (CWBItemSelector*)FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
  if ( !b ) return;

  TS32 cursor = -1;// b->GetCursorPosition();
  b->Flush();

  if ( !WorkBench->GetEditedModel() ) return;
  CphxModel_Tool *m = WorkBench->GetEditedModel();

  CphxModelObject_Tool *Selected = nullptr;

  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    CphxModelObject_Tool *t = m->GetObjectByIndex( x );
    TS32 i = b->AddItem( t->GetName() );
    ModelObjects[ i ] = t;
    b->GetItem( i )->Select( m->GetObjectByIndex( x )->Selected );
    if ( m->GetObjectByIndex( x )->Selected )
    {
      if ( !Selected )
        Selected = m->GetObjectByIndex( x );
      cursor = x;
    }
    for ( TS32 y = 0; y < t->GetFilterCount(); y++ )
    {
      CString s = t->GetFilter( y )->GetName();
      if ( s.Length() == 0 )
        s = ModelFilterNames[ t->GetFilter( y )->Filter ];

      TS32 j = b->AddItem( _T( "  +" ) + s );
      b->GetItem( j )->Select( t->GetFilter( y ) == WorkBench->GetEditedModelFilter() );
      ModelFilters[ j ] = t->GetFilter( y );
    }
  }

  if ( !WorkBench->GetEditedModelFilter() )
    WorkBench->SetEditedModelObject( Selected );
  //b->SetCursorPosition(cursor);
}

TBOOL CapexModelGraph::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
  {
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
    }

  }
  break;
  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *b = (CWBItemSelector*)FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
    if ( !b ) break;
    CphxModel_Tool *m = WorkBench->GetEditedModel();
    if ( !m ) break;

    if ( ModelFilters.HasKey( Message.Data ) )
    {
      WorkBench->SetEditedModelFilter( ModelFilters[ Message.Data ] );
    }

    if ( ModelObjects.HasKey( Message.Data ) )
    {
      WorkBench->SetEditedModelFilter( nullptr );
      WorkBench->SetEditedModelObject( ModelObjects[ Message.Data ] );
    }

    break;
  }
  case WBM_SELECTIONCHANGE:
  {
    CWBItemSelector *b = (CWBItemSelector*)FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
    if ( !b ) break;

    CphxModel_Tool *m = WorkBench->GetEditedModel();
    if ( !m ) break;

    for ( TS32 x = 0; x < b->NumItems(); x++ )
    {
      if ( ModelObjects.HasKey( b->GetItemByIndex( x ).GetID() ) )
        ModelObjects[ b->GetItemByIndex( x ).GetID() ]->Selected = b->GetItemByIndex( x ).IsSelected();
      if ( ModelFilters.HasKey( b->GetItemByIndex( x ).GetID() ) )
        ModelFilters[ b->GetItemByIndex( x ).GetID() ]->Selected = b->GetItemByIndex( x ).IsSelected();
    }
  }
  break;
  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *b = (CWBItemSelector*)FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
    if ( !b ) break;
    CphxModel_Tool *m = WorkBench->GetEditedModel();
    if ( !m ) break;

    if ( ModelObjects.HasKey( Message.Data ) )
    {
      ModelObjects[ Message.Data ]->SetName( b->GetItem( Message.Data )->GetText() );
      return true;
    }

    if ( ModelFilters.HasKey( Message.Data ) )
    {
      ModelFilters[ Message.Data ]->SetName( b->GetItem( Message.Data )->GetText().GetPointer() );
      return true;
    }


  }
  break;
  }


  return CapexWindow::MessageProc( Message );
}
