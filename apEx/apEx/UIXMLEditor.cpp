#include "BasePCH.h"
#include "UIXMLEditor.h"
#include "WorkBench.h"
#include "UICSSEditor.h"
#define WINDOWNAME _T("Interface XML Editor")
#define WINDOWXML _T("XMLEditor")

CapexUIXMLEditor::CapexUIXMLEditor() : CapexWindow()
{
  EditedWindow = (APEXWINDOW)-1;
  Filename = "";
}

CapexUIXMLEditor::CapexUIXMLEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexUIXMLEditor::~CapexUIXMLEditor()
{

}

void CapexUIXMLEditor::SetEditedWindow( CapexWindow *w )
{
  EditedWindow = w->GetWindowType();
  Filename = _T( "Data/UI/" ) + w->GetXMLName() + _T( ".xml" );

  CStreamReaderMemory f;
  if ( !f.Open( Filename.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading XML: file '%s' not found", Filename.GetPointer() );
    return;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );

  CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "xmleditor" ), _T( "textbox" ) );
  if ( ib ) ib->SetText( s );
}

TBOOL CapexUIXMLEditor::SaveAndRefresh()
{
  CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "xmleditor" ), _T( "textbox" ) );
  if ( !ib ) return true;

  CapexWindow *w = WorkBench->GetWindow( EditedWindow );
  if ( !w ) return true;
  Filename = _T( "Data/UI/" ) + w->GetXMLName() + _T( ".xml" );

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

TBOOL CapexUIXMLEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
    if ( Message.Key == VK_F5 )
    {
      CapexUICSSEditor *e = (CapexUICSSEditor*)WorkBench->GetWindow( apEx_CSSEditor );
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

void CapexUIXMLEditor::UpdateData()
{

}
