#include "BasePCH.h"
#include "Console.h"

CRingBuffer<CString> ConsoleLog;

void CapexConsole::UpdateScrollParameters()
{
  TS32 xsize = 0;

  WBITEMSTATE i = GetState();
  CWBFont *f = GetFont( i );

  for ( TS32 x = ConsoleLog.NumItems() - 1; x >= 0; x-- )
    xsize = max( xsize, f->GetWidth( ConsoleLog[ x ].GetPointer() ) );

  CSize c = GetClientRect().Size();

  SetHScrollbarParameters( 0, xsize + 4, c.x );

  TS32 mi = -ConsoleLog.NumItems()*f->GetLineHeight() + c.y;
  TS32 ma = c.y;

  SetVScrollbarParameters( mi, ma, c.y );

  //handle invalid position
  SetVScrollbarPos( GetVScrollbarPos(), true );

  if ( ma - mi < c.y ) SetVScrollbarPos( ma - c.y );

}

void CapexConsole::OnDraw( CWBDrawAPI *API )
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  if ( Logger.GetNewEntryCount() > 0 )
    UpdateScrollParameters();

  API->SetCropToClient( this );
  DrawBackground( API );
  //API->DrawRect(GetClientRect(),rgbBackground);

  CPoint pos = CPoint( 2 - GetHScrollbarPos(), GetClientRect().Height() - GetVScrollbarPos() );
  TS32 xsize = 0;

  for ( TS32 x = ConsoleLog.NumItems() - 1; x >= 0; x-- )
  {
    TS32 lh = Font->GetLineHeight();
    pos.y -= lh;
    if ( pos.y >= -lh && pos.y <= GetClientRect().Height() + lh )
    {
      CColor Color = 0xffffffff;

      TS32 p = ConsoleLog[ x ].Find( _T( "(" ) );
      if ( p >= 0 )
      {
        if ( ConsoleLog[ x ].Find( _T( "(Warn)" ) ) == p ) Color = CColor::FromARGB( 0xffffff00 );
        if ( ConsoleLog[ x ].Find( _T( "(Err)" ) ) == p ) Color = CColor::FromARGB( 0xffff0000 );
        if ( ConsoleLog[ x ].Find( _T( "(Dbg)" ) ) == p ) Color = CColor::FromARGB( 0xff00ffff );
      }

      Font->Write( API, ConsoleLog[ x ].GetPointer(), pos, Color, TextTransform );
    }
  }
}

CapexConsole::CapexConsole() : CWBItem()
{
  ClickThrough = false;
}

CapexConsole::CapexConsole( CWBItem *Parent, const CRect &Pos, TBOOL ClickThrough ) : CWBItem()
{
  EnableHScrollbar( true, true );
  EnableVScrollbar( true, true );
  Initialize( Parent, Pos, ClickThrough );
}

TBOOL CapexConsole::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_UNHIDE:
    UpdateScrollParameters();
    break;
  case WBM_REPOSITION:
  {
    TBOOL b = CWBItem::MessageProc( Message );
    if ( Message.Resized )
      UpdateScrollParameters();
    return b;
  }
  break;
  case WBM_MOUSEWHEEL:
    SetVScrollbarPos( GetVScrollbarPos() - Message.Data*GetClientRect().Height() / 3, true );
    break;
  default:
    break;
  }

  return CWBItem::MessageProc( Message );
}

TBOOL CapexConsole::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  if ( CWBItem::IsMouseTransparent( ClientSpacePoint, MessageType ) ) return true;
  return ClickThrough && MessageType != WBM_MOUSEWHEEL && GetClientRect().Contains( ClientSpacePoint );
}

CapexConsole::~CapexConsole()
{

}

TBOOL CapexConsole::Initialize( CWBItem *Parent, const CRect &Position, TBOOL ct )
{
  ClickThrough = ct;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  return true;
}

CWBItem * CapexConsole::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  bool clickThrough = false;

  if ( node.HasAttribute( _T( "clickthrough" ) ) )
  {
    int ct = 0;
    node.GetAttributeAsInteger( _T( "clickthrough" ), &ct );
    clickThrough = ct != 0;
  }

  return new CapexConsole( Root, Pos, clickThrough );
}

#define WINDOWNAME _T("Console Output")
#define WINDOWXML _T("ConsoleWindow")

CapexConsoleWindow::CapexConsoleWindow() : CapexWindow()
{
  Console = NULL;
}

CapexConsoleWindow::CapexConsoleWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  Console = new CapexConsole( this, GetClientRect(), true );
  App->ApplyStyle( Console );
}

CapexConsoleWindow::~CapexConsoleWindow()
{
}

TBOOL CapexConsoleWindow::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
    if ( Message.GetTarget() == GetGuid() )
    {
      TBOOL i = CWBWindow::MessageProc( Message );
      if ( Console )
        Console->SetPosition( GetClientRect() );
      return i;
    }
    break;
  default:
    break;
  }

  return CWBWindow::MessageProc( Message );
}

void CapexConsoleWindow::UpdateData()
{

}
