#include "BasePCH.h"
#include "TexGenPages.h"
#include "WorkBench.h"
#define WINDOWNAME _T("Texgen Pages")
#define WINDOWXML _T("TexgenPages")
#include "../Phoenix_Tool/apxPhoenix.h"

CapexTexGenPages::CapexTexGenPages() : CapexWindow()
{
  //List=NULL;
  //NewPageButton=NULL;
  //DeletePageButton=NULL;
}

CapexTexGenPages::CapexTexGenPages( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML )
{
  //List=(CWBItemSelector*)FindChildByID(_T("pagelist"),_T("itemselector"));
  //NewPageButton=(CWBButton*)FindChildByID(_T("newpage"),_T("button"));
  //DeletePageButton=(CWBButton*)FindChildByID(_T("deletepage"),_T("button"));
  Update();
}

CapexTexGenPages::~CapexTexGenPages()
{

}

void CapexTexGenPages::Update()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "pagelist" ), _T( "itemselector" ) );
  if ( !List ) return;

  List->Flush();
  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    CapexTexGenPage *pge = Project.GetTexgenPageByIndex( x );
    SELECTABLEID e = List->AddItem( pge->GetName() );
    List->GetItem( e )->SetID( pge->GetID() );
  }

  List->SelectItem( WorkBench->GetEditedPage() );
}

void CapexTexGenPages::CreatePage()
{
  CapexTexGenPage *pge = Project.CreateTexgenPage();

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "pagelist" ), _T( "itemselector" ) );
  if ( !List ) return;
  SELECTABLEID e = List->AddItem( pge->GetName() );
  List->GetItem( e )->SetID( pge->GetID() );
  List->SelectItem( pge->GetID() );
}

void CapexTexGenPages::DeletePage( TS32 ID )
{
  Project.DeleteTexgenPage( ID );

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "pagelist" ), _T( "itemselector" ) );
  if ( !List ) return;
  List->DeleteItem( ID );
}

void CapexTexGenPages::SelectPage( TS32 ID )
{
  WorkBench->SetEditedPage( ID );

  CWBTrackBar *x = (CWBTrackBar*)FindChildByIDs( _T( "xres" ), _T( "trackbar" ), _T( "numpad" ) );
  CWBTrackBar *y = (CWBTrackBar*)FindChildByIDs( _T( "yres" ), _T( "trackbar" ), _T( "numpad" ) );
  CWBButton *hdr = (CWBButton*)FindChildByIDs( _T( "hdr" ), _T( "button" ) );

  CapexTexGenPage *p = Project.GetTexgenPage( ID );
  if ( p )
  {
    if ( x ) x->SetValue( p->GetBaseXRes() );
    if ( y ) y->SetValue( p->GetBaseYRes() );
    if ( hdr ) hdr->Push( p->IsHDR() );
  }
}

TBOOL CapexTexGenPages::MessageProc( CWBMessage &Message )
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "pagelist" ), _T( "itemselector" ) );

  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
    if ( !InFocus() ) break;
    if ( Message.Key == VK_DELETE )
    {
      if ( List )
        DeletePage( List->GetCursorItemID() );
      return true;
    }
    break;
  case WBM_COMMAND:
    if ( Message.IsTargetID( _T( "newpage" ) ) )
    {
      CreatePage();
      return true;
    }
    if ( List && Message.IsTargetID( _T( "deletepage" ) ) )
    {
      DeletePage( List->GetCursorItemID() );
      return true;
    }
    if ( List && Message.IsTargetID( _T( "hdr" ) ) )
    {
      auto* page = Project.GetTexgenPage( List->GetCursorItemID() );
      if ( page )
      {
        page->ToggleHDR();
        CWBButton *hdr = (CWBButton*)FindChildByIDs( _T( "hdr" ), _T( "button" ) );
        if ( hdr ) hdr->Push( page->IsHDR() );
      }
      return true;
    }
    break;
  case WBM_ITEMSELECTED:
    if ( List && Message.GetTarget() == List->GetGuid() )
    {
      SelectPage( List->GetCursorItemID() );
      return true;
    }
    break;
  case WBM_ITEMRENAMED:
    if ( List && Message.GetTarget() == List->GetGuid() )
    {
      CapexTexGenPage *p = Project.GetTexgenPage( List->GetCursorItemID() );
      if ( p && List->GetCursorItem() )
        p->SetName( List->GetCursorItem()->GetText() );
      return true;
    }
    break;
  case WBM_VALUECHANGED:
  {
    CWBTrackBar *x = (CWBTrackBar*)FindChildByIDs( _T( "xres" ), _T( "trackbar" ), _T( "numpad" ) );
    CWBTrackBar *y = (CWBTrackBar*)FindChildByIDs( _T( "yres" ), _T( "trackbar" ), _T( "numpad" ) );

    if ( x && Message.IsTargetID( _T( "xres" ) ) )
    {
      x->SetText( CString::Format( _T( "X Resolution: %d" ), 1 << Message.Data ) );
      if ( List )
      {
        CapexTexGenPage *p = Project.GetTexgenPage( List->GetCursorItemID() );
        if ( p ) p->SetBaseXRes( Message.Data );
      }
      return true;
    }

    if ( y && Message.IsTargetID( _T( "yres" ) ) )
    {
      y->SetText( CString::Format( _T( "Y Resolution: %d" ), 1 << Message.Data ) );
      if ( List )
      {
        CapexTexGenPage *p = Project.GetTexgenPage( List->GetCursorItemID() );
        if ( p ) p->SetBaseYRes( Message.Data );
      }
      return true;
    }

  }
  break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTexGenPages::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "pagelist" ), _T( "itemselector" ) );
  if ( !List ) return;

  TS32 cursorpos = List->GetCursorPosition();
  List->Flush();

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    TS32 e = List->AddItem( Project.GetTexgenPageByIndex( x )->GetName() );
    List->GetItem( e )->SetID( Project.GetTexgenPageByIndex( x )->GetID() );
  }

  bool found = false;

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    if ( Project.GetTexgenPageByIndex( x )->GetID() == GetWorkBench()->GetEditedPage() )
    {
      if ( !List->GetItemByIndex( x ).IsSelected() )
      {
        List->SelectItemByIndex( x );
        found = true;
      }
    }
  }

  if ( !found )
    List->SelectItemByIndex( cursorpos );
}
