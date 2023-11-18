#include "BasePCH.h"
#include "apExRoot.h"
#include "apExWindow.h"
#include "WorkBench.h"
#include "UICSSEditor.h"
#include "UIXMLEditor.h"

void CapexWindow::LoadStyle()
{
  windowStyleManager.Reset();

  CString CSSPath = CString( Root->GetCSSPath() + XMLName + ".css" );

  CStreamReaderMemory f;
  if ( !f.Open( CSSPath.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading CSS: file '%s' not found", CSSPath.GetPointer() );
    return;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );

  windowStyleManager.ParseStyleData( s );
}

void CapexWindow::ApplyStyle()
{
  App->ApplyStyle( this );
  windowStyleManager.ApplyStyles( this );

  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );
}

void CapexWindow::OnDraw( CWBDrawAPI *API )
{
  if ( GetParent()->InstanceOf( "apexdocker" ) )
    Style &= ~WB_WINDOW_RESIZABLE;
  else
    Style |= WB_WINDOW_RESIZABLE;

  CWBWindow::OnDraw( API );
}

CapexWindow::CapexWindow() : CWBWindow()
{
  WorkBench = NULL;
}

CapexWindow::CapexWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB, const TCHAR *txt, const TCHAR *xmln, TU32 style ) : CWBWindow( Parent, Pos, txt, style )
{
  WorkBench = WB;

  XMLName = xmln;

  //don't use reloadlayout here as it calls the virtual updatedata()!
  CString xmlname = CString( _T( "Data/UI/" ) ) + XMLName;
  App->LoadXMLLayoutFromFile( xmlname + ".xml" );

  //App->LoadCSSFromFile( Root->GetCSSPath() + _T( "apEx.css" ) );
  //App->LoadCSSFromFile( Root->GetCSSPath() + XMLName + ".css", false );
  LoadStyle();
  App->GenerateGUI( this, XMLName );
  ApplyStyle();
  //App->LoadCSSFromFile( Root->GetCSSPath() + _T( "apEx.css" ) );
}

CapexWindow::~CapexWindow()
{
  WorkBench->UndockWindow( this );
  WorkBench->WindowBeingDeleted( this );
}

TBOOL CapexWindow::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
    if ( Message.Key == VK_F3 )
    {
      LOG_NFO( "[gui] Reloading %s", XMLName.GetPointer() );

      ReloadLayout();
      return true;
    }
    if ( Message.Key == VK_F4 )
    {
      for ( TS32 x = 0; x < WorkBench->GetWindowCount(); x++ )
      {
        if ( WorkBench->GetWindowByIndex( x )->GetWindowType() == apEx_XMLEditor )
          ( (CapexUIXMLEditor *)WorkBench->GetWindowByIndex( x ) )->SetEditedWindow( this );
        if ( WorkBench->GetWindowByIndex( x )->GetWindowType() == apEx_CSSEditor )
          ( (CapexUICSSEditor *)WorkBench->GetWindowByIndex( x ) )->SetEditedWindow( this );
      }
      return true;
    }
    break;
  case WBM_REPOSITION:
    if ( DragMode == WB_DRAGMODE_MOVE && App->GetMouseCaptureItem() == this )
    {
      CPoint dist = App->GetMousePos() - App->GetLeftDownPos();

      if ( ( abs( dist.x ) > 10 || abs( dist.y ) > 10 ) && GetParent() != WorkBench )
        WorkBench->UndockWindow( this );
    }
    break;
  case WBM_CLOSE:
    WorkBench->UndockWindow( this );
    break;
  }

  return CWBWindow::MessageProc( Message );
}

void CapexWindow::ReloadLayout()
{
  DeleteChildren();

  CString xmlname = CString( _T( "Data/UI/" ) ) + XMLName;
  App->LoadXMLLayoutFromFile( xmlname + ".xml" );
  //App->LoadCSSFromFile( Root->GetCSSPath() + _T( "apEx.css" ) );
  //App->LoadCSSFromFile( Root->GetCSSPath() + XMLName + ".css", false );
  LoadStyle();
  App->GenerateGUI( this, XMLName );
  ApplyStyle();
  UpdateData();
  //App->LoadCSSFromFile( Root->GetCSSPath() + _T( "apEx.css" ) );
}

void CapexWindow::ExportWindow( CXMLNode *node )
{
  if ( !node ) return;
  node->SetAttribute( _T( "typename" ), GetXMLName().GetPointer() );
  node->SetAttributeFromInteger( _T( "type" ), GetWindowType() );
  node->SetAttributeFromInteger( _T( "x1" ), GetPosition().x1 );
  node->SetAttributeFromInteger( _T( "y1" ), GetPosition().y1 );
  node->SetAttributeFromInteger( _T( "x2" ), GetPosition().x2 );
  node->SetAttributeFromInteger( _T( "y2" ), GetPosition().y2 );
}

void CapexWindow::WindowHasDocked()
{
  ReleaseCapture();
  DragMode = 0;
}

void CapexWindow::OnPostDraw( CWBDrawAPI*API )
{
  if ( App->GetMousePos() == App->GetLeftDownPos() )
    return;

  if ( !App->GetMouseCaptureItem() )
    return;

  if ( App->GetMouseCaptureItem() == this )
    return;

  if ( !App->GetMouseCaptureItem()->InstanceOf( "apexwindow" ) )
    return;

  if ( !GetParent()->InstanceOf( "apexdocker" ) )
    return;

  CapexWindow *dragged = (CapexWindow*)App->GetMouseCaptureItem();

  if ( !GetScreenRect().Contains( App->GetMousePos() ) )
    return;

  CWBWindow* wnd = (CWBWindow*)App->GetMouseCaptureItem();
  if ( wnd->GetDragMode() != WB_DRAGMODE_MOVE )
    return;

  CRect cl = GetScreenRect();

  CRect left = CRect( cl.Center() - CPoint( 25, 25 ), cl.Center() + CPoint( 25, 25 ) ) - CPoint( 50, 0 );
  CRect right = CRect( cl.Center() - CPoint( 25, 25 ), cl.Center() + CPoint( 25, 25 ) ) + CPoint( 50, 0 );

  CRect top = CRect( cl.Center() - CPoint( 25, 25 ), cl.Center() + CPoint( 25, 25 ) ) - CPoint( 0, 50 );
  CRect bottom = CRect( cl.Center() - CPoint( 25, 25 ), cl.Center() + CPoint( 25, 25 ) ) + CPoint( 0, 50 );

  CPoint p = App->GetMousePos();

  WindowDockDisplayInfo dleft( left.Center(), CRect( cl.x1, cl.y1, cl.Center().x, cl.y2 ), DockPosition::Left, dragged, nullptr, this );
  WindowDockDisplayInfo dright( right.Center(), CRect( cl.Center().x, cl.y1, cl.x2, cl.y2 ), DockPosition::Right, dragged, nullptr, this );
  WindowDockDisplayInfo dtop( top.Center(), CRect( cl.x1, cl.y1, cl.x2, cl.Center().y ), DockPosition::Top, dragged, nullptr, this );
  WindowDockDisplayInfo dbottom( bottom.Center(), CRect( cl.x1, cl.Center().y, cl.x2, cl.y2 ), DockPosition::Bottom, dragged, nullptr, this );

  GetWorkBench()->dockerItems += dleft;
  GetWorkBench()->dockerItems += dright;
  GetWorkBench()->dockerItems += dtop;
  GetWorkBench()->dockerItems += dbottom;

  if ( left.Contains( p ) )
    GetWorkBench()->activeItem = dleft;

  if ( right.Contains( p ) )
    GetWorkBench()->activeItem = dright;

  if ( top.Contains( p ) )
    GetWorkBench()->activeItem = dtop;

  if ( bottom.Contains( p ) )
    GetWorkBench()->activeItem = dbottom;
}

#include "apExRoot.h"

CapexWorkBench* CapexWindow::GetWorkBench()
{
  CWBItem *i = this;

  while ( 1 )
  {
    if ( !i )
      break;

    if ( i->GetType() == _T( "workbench" ) )
      return (CapexWorkBench*)i;

    i = i->GetParent();

  }

  extern CapexRoot *Root;
  if ( !Root ) return NULL;
  return Root->GetWorkBench( Root->GetSelectedWorkBench() );

}
