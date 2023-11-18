#include "BasePCH.h"
#include "TrackBar.h"

void CWBTrackBar::DrawButton( CWBDrawAPI *API, CRect Pos, TBOOL Pushed, CWBCSSPropertyBatch& cssProps )
{
  TBOOL mouseover = MouseOver() && Pos.Contains( ScreenToClient( App->GetMousePos() ) );

  WBITEMSTATE state = WB_STATE_NORMAL;
  if ( mouseover ) state = WB_STATE_HOVER;
  if ( Pushed ) state = WB_STATE_ACTIVE;

  DrawBackground( API, Pos, state, cssProps );
  DrawBorder( API, Pos, cssProps );
}

CRect CWBTrackBar::GetButtonPos( TS32 ButtonID )
{
  CRect original;

  if ( ButtonID == 2 )
    return TrackbarProperties.PositionDescriptor.GetPosition( GetClientRect().Size(), GetClientRect().Size(), original );

  return ButtonProperties[ ButtonID ].PositionDescriptor.GetPosition( GetClientRect().Size(), GetClientRect().Size(), original );
}

void CWBTrackBar::OnDraw( CWBDrawAPI *API )
{
  WBITEMSTATE i = GetState();
  DrawBackground( API );
  DrawBorder( API );

  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  CColor textColor = CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_FONTCOLOR );

  //if (ButtonWidth)
  {
    CRect b1 = GetButtonPos( 0 );
    CRect b2 = GetButtonPos( 1 );

    if ( b1.Area() > 0 )
    {
      DrawButton( API, b1, ButtonClicked == 0, ButtonProperties[ 0 ] );
      CWBFont *bfont = ButtonProperties[ 0 ].GetFont( App, i );
      if ( bfont /*&& bfont!=App->GetDefaultFont()*/ )
        bfont->Write( API, _T( "-" ), Font->GetCenter( _T( "+" ), b1 ), textColor );
    }

    if ( b2.Area() > 0 )
    {
      DrawButton( API, b2, ButtonClicked == 1, ButtonProperties[ 1 ] );
      CWBFont *bfont = ButtonProperties[ 1 ].GetFont( App, i );
      if ( bfont /*&& bfont != App->GetDefaultFont()*/ )
        bfont->Write( API, _T( "+" ), Font->GetCenter( _T( "+" ), b2 ), textColor );
    }
  }

  TF32 f = GetValuePositionRelativeToConstraints();

  CRect bar = GetButtonPos( 2 );

  WBITEMSTATE state = WB_STATE_NORMAL;
  if ( App->GetMouseItem() == this && bar.Contains( ScreenToClient( App->GetMousePos() ) ) )
    state = WB_STATE_HOVER;
  if ( ButtonClicked == 2 )
    state = WB_STATE_ACTIVE;

  DrawBackground( API, bar, state, TrackbarProperties );
  DrawBorder( API, bar, TrackbarProperties );

  CColor fillercolor = TrackbarProperties.DisplayDescriptor.GetColor( state, WB_ITEM_FOREGROUNDCOLOR );// CColor::FromARGB(0xff3e3e40);

  CRect bar2 = bar;
  bar2.x2 = Lerp( bar2.x1, bar2.x2, f );

  API->DrawRect( bar2, fillercolor );

  API->SetCropRect( ClientToScreen( bar ) );
  Font->Write( API, Text, Font->GetCenter( Text, bar, TextTransform ), textColor, TextTransform );
}

CWBTrackBar::CWBTrackBar() : CWBItem()
{
}

CWBTrackBar::CWBTrackBar( CWBItem *Parent, const CRect &Pos, TS32 min, TS32 max, TS32 value ) : CWBItem()
{
  Initialize( Parent, Pos, min, max, value );
}

CWBTrackBar::~CWBTrackBar()
{

}

TBOOL CWBTrackBar::Initialize( CWBItem *Parent, const CRect &Position, TS32 min, TS32 max, TS32 value )
{
  //ButtonWidth = 13;
  ButtonClicked = -1;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  if ( !CWBValueInput::Initialize( WBVI_INT ) ) return false;
  SetConstraints( min, max );
  //SetValue(value);

  for ( int x = 0; x < 2; x++ )
  {
    ButtonProperties[ x ].DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff2d2d30 ) );
    ButtonProperties[ x ].DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3e3e40 ) );
    ButtonProperties[ x ].DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1c97ea ) );
  }

  ButtonProperties[ 0 ].PositionDescriptor.SetValue( WB_MARGIN_LEFT, 0, 0 );
  ButtonProperties[ 0 ].PositionDescriptor.SetValue( WB_HEIGHT, 1, 0 );
  ButtonProperties[ 0 ].PositionDescriptor.SetValue( WB_WIDTH, 0, 13 );

  ButtonProperties[ 1 ].PositionDescriptor.SetValue( WB_MARGIN_RIGHT, 0, 0 );
  ButtonProperties[ 1 ].PositionDescriptor.SetValue( WB_HEIGHT, 1, 0 );
  ButtonProperties[ 1 ].PositionDescriptor.SetValue( WB_WIDTH, 0, 13 );

  TrackbarProperties.PositionDescriptor.SetValue( WB_MARGIN_LEFT, 0, 13 );
  TrackbarProperties.PositionDescriptor.SetValue( WB_MARGIN_RIGHT, 0, 13 );
  TrackbarProperties.PositionDescriptor.SetValue( WB_HEIGHT, 0.7f, 0 );


  //SetDisplayProperty(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB(0xff2d2d30));
  //SetDisplayProperty(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB(0xff3e3e40));
  //SetDisplayProperty(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB(0xff1c97ea));

  TrackbarProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_FOREGROUNDCOLOR, CColor::FromARGB( 0xff3e3e40 ) );
  TrackbarProperties.DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_FOREGROUNDCOLOR, CColor::FromARGB( 0xff686868 ) );

  //SetDisplayProperty(WB_STATE_NORMAL,WB_ITEM_CLIENTCOLOR,CColor::FromARGB( 0xff3e3e40 ));
  //SetDisplayProperty(WB_STATE_HOVER,WB_ITEM_CLIENTCOLOR,CColor::FromARGB( 0xff686868 ));

  return true;
}

WBITEMSTATE CWBTrackBar::GetState()
{
  if ( ButtonClicked != -1 )
    return WB_STATE_NORMAL;

  WBITEMSTATE i = CWBItem::GetState();
  if ( App->GetMouseCaptureItem() == this && MouseOver() ) i = WB_STATE_ACTIVE;
  else
  {
    if ( IsEnabled() )
      i = MouseOver() ? WB_STATE_HOVER : WB_STATE_NORMAL;
    else
      i = WB_STATE_DISABLED;
  }

  if ( i == WB_STATE_HOVER && App->GetMouseCaptureItem() )
    if ( App->GetMouseCaptureItem() != this && App->GetMouseCaptureItem()->GetType() != _T( "contextmenu" ) ) i = WB_STATE_NORMAL;

  return i;
}

TBOOL CWBTrackBar::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  TBOOL ElementTarget = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "incbutton" ) || pseudo[ x ] == _T( "decbutton" ) || pseudo[ x ] == _T( "bar" ) )
    {
      ElementTarget = true;
      break;
    }
  }

  if ( !ElementTarget ) return CWBItem::ApplyStyle( prop, value, pseudo );

  TBOOL Handled = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "incbutton" ) )
    {
      Handled |= ButtonProperties[ 1 ].ApplyStyle( this, prop, value, pseudo );
      continue;
    }
    if ( pseudo[ x ] == _T( "decbutton" ) )
    {
      Handled |= ButtonProperties[ 0 ].ApplyStyle( this, prop, value, pseudo );
      continue;
    }
    if ( pseudo[ x ] == _T( "bar" ) )
    {
      Handled |= TrackbarProperties.ApplyStyle( this, prop, value, pseudo );
      continue;
    }
  }

  return Handled;
}

void CWBTrackBar::SetValueToMouseX( TS32 mx )
{
  CRect bar = GetButtonPos( 2 );

  //TS32 w = GetClientRect().Width() - 2 * ButtonWidth;
  SetInterpolatedValue( ( ( mx - bar.x1 ) / (TF32)( bar.Width() ) ) );
}

TBOOL CWBTrackBar::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
    if ( App->GetMouseItem() == this )
    {
      ButtonClicked = -1;
      App->SetCapture( this );
      CPoint p = ScreenToClient( Message.GetPosition() );
      for ( TS32 x = 2; x >= 0; x-- )
      {
        if ( GetButtonPos( x ).Contains( p ) )
        {
          ButtonClicked = x;
          if ( x == 0 ) ApplyStep( -1 );
          if ( x == 1 ) ApplyStep( 1 );
          if ( x == 2 ) SetValueToMouseX( p.x );
        }
      }
      return true;
    }
    break;

  case WBM_LEFTBUTTONREPEAT:
    if ( App->GetMouseCaptureItem() == this )
    {
      CPoint p = ScreenToClient( Message.GetPosition() );
      for ( TS32 x = 2; x >= 0; x-- )
      {
        if ( ButtonClicked == x && GetButtonPos( x ).Contains( p ) )
        {
          if ( x == 0 ) ApplyStep( -1 );
          if ( x == 1 ) ApplyStep( +1 );
          //mousex is set on mouse movement, not here
        }
      }
      return true;
    }
    break;

  case WBM_MOUSEMOVE:
    if ( App->GetMouseCaptureItem() == this )
    {
      if ( ButtonClicked == 2 )
        SetValueToMouseX( ScreenToClient( App->GetMousePos() ).x );
      return true;
    }
    break;

  case WBM_RIGHTBUTTONDOWN:
  case WBM_MIDDLEBUTTONDOWN:
    if ( App->GetMouseItem() == this )
    {
      App->SetCapture( this );
      return true;
    }
    break;

  case WBM_LEFTBUTTONUP:
    App->ReleaseCapture();
    ButtonClicked = -1;
    return true;

  case WBM_RIGHTBUTTONUP:
    App->ReleaseCapture();
    return true;

  case WBM_MIDDLEBUTTONUP:
    App->ReleaseCapture();
    return true;

  case WBM_MOUSEWHEEL:
    if ( App->GetMouseItem() == this )
    {
      ApplyStep( Message.Data );
      return true;
    }
    break;

  case WBM_VALUECHANGED:
    SetText( CString::Format( _T( "%d" ), Message.Data ) );
    return true;

    // 	default:
    // 		return false;
  }

  return CWBItem::MessageProc( Message );
}

void CWBTrackBar::SetText( const CString& val )
{
  Text = val;
}

CWBItem * CWBTrackBar::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  TS32 min = 0;
  TS32 max = 255;
  TS32 def = 127;

  node.GetAttributeAsInteger( _T( "minimum" ), &min );
  node.GetAttributeAsInteger( _T( "maximum" ), &max );
  node.GetAttributeAsInteger( _T( "default" ), &def );

  CWBTrackBar * trackbar = new CWBTrackBar( Root, Pos, min, max, def );
  if ( node.HasAttribute( _T( "text" ) ) )
    trackbar->SetText( node.GetAttribute( _T( "text" ) ) );
  return trackbar;
}

void CWBTrackBar::SetButtonWidth( TS32 buttonWidth )
{
  //ButtonWidth = buttonWidth;
}

void CWBTrackBar::SendValueChangeMessage()
{
  App->SendMessage( CWBMessage( App, WBM_VALUECHANGED, GetGuid(), GetValueAsInt() ) );
}
