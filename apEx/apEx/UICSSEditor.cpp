#include "BasePCH.h"
#include "apExRoot.h"
#include "UICSSEditor.h"
#include "WorkBench.h"
#include "UIXMLEditor.h"
#define WINDOWNAME _T("Interface CSS Editor")
#define WINDOWXML _T("CSSEditor")

CapexUICSSEditor::CapexUICSSEditor() : CapexWindow()
{
  EditedWindow = (APEXWINDOW)-1;
  Filename = "";
}

CapexUICSSEditor::CapexUICSSEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexUICSSEditor::~CapexUICSSEditor()
{

}

void CapexUICSSEditor::SetEditedWindow( CapexWindow *w )
{
  if ( !w ) return;

  EditedWindow = w->GetWindowType();
  Filename = Root->GetCSSPath() + w->GetXMLName() + _T( ".css" );

  CStreamReaderMemory f;
  if ( !f.Open( Filename.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading CSS: file '%s' not found", Filename.GetPointer() );
    return;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );

  CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "csseditor" ), _T( "textbox" ) );
  if ( ib ) ib->SetText( s );
}

TBOOL CapexUICSSEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
    if ( Message.Key == VK_F5 )
    {
      CapexUIXMLEditor *e = (CapexUIXMLEditor*)WorkBench->GetWindow( apEx_XMLEditor );
      if ( e ) e->SaveAndRefresh();
      TBOOL r = SaveAndRefresh();

      for ( TS32 x = 0; x < WorkBench->GetWindowCount(); x++ )
        if ( WorkBench->GetWindowByIndex( x )->GetWindowType() == EditedWindow )
          WorkBench->GetWindowByIndex( x )->ReloadLayout();

      return r;
    }
    break;
  }

  return CWBWindow::MessageProc( Message );
}

TBOOL CapexUICSSEditor::SaveAndRefresh()
{
  CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "csseditor" ), _T( "textbox" ) );
  if ( !ib ) return true;

  CapexWindow *w = WorkBench->GetWindow( EditedWindow );
  if ( !w ) return true;
  Filename = Root->GetCSSPath() + w->GetXMLName() + _T( ".css" );

  {
    CStreamWriterFile f;
    if ( !f.Open( Filename.GetPointer() ) )
    {
      LOG_ERR( "[apex] Error opening file '%s' for writing.", Filename );
      return true;
    }

    f.Write( ib->GetText().GetPointer(), ib->GetText().Length() );
  }

  return true;
}

void CapexUICSSEditor::UpdateData()
{

}
