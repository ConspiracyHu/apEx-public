#include "BasePCH.h"
#include "NumPad.h"

CWBNumPad::CWBNumPad() : CWBTrackBar()
{
  textBox = nullptr;
}

CWBNumPad::CWBNumPad( CWBItem *Parent, const CRect &Pos, TS32 Min, TS32 Max, TS32 Value ) : CWBTrackBar()
{
  Initialize( Parent, Pos, Min, Max, Value );
}

CWBNumPad::~CWBNumPad()
{

}

TBOOL CWBNumPad::Initialize( CWBItem *Parent, const CRect &Position, TS32 Min, TS32 Max, TS32 Value )
{
  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  if ( !CWBValueInput::Initialize( WBVI_INT ) ) return false;

  toText = []( TS32 value )->CString 
  { 
    return CString::Format( "%d", value ); 
  };
  fromText = []( CWBValueInput* value, CString& text ) 
  {
    TS32 val;
    if ( text.Scan( "%d", &val ) == 1 )
      value->SetValue( val );
  };

  SetConstraints( Min, Max );

  textBox = new CWBTextBox( this, Position );
  textBox->ApplyStyleDeclarations( "height:100%;width:36px;text-align:center;vertical-align:middle;" );
  textBox->EnableHScrollbar( false, false );
  textBox->EnableVScrollbar( false, false );

  SetValue( Value );

  return true;

}

//void CWBNumPad::SetText(const CString& text)
//{
//  int x = 0;
//  //if (trackBar)
//  //  trackBar->SetText(text);
//}

CWBItem * CWBNumPad::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  TS32 min = 0;
  TS32 max = 255;
  TS32 def = 127;

  node.GetAttributeAsInteger( _T( "minimum" ), &min );
  node.GetAttributeAsInteger( _T( "maximum" ), &max );
  node.GetAttributeAsInteger( _T( "default" ), &def );

  CWBNumPad * trackbar = new CWBNumPad( Root, Pos, min, max, def );
  if ( node.HasAttribute( _T( "text" ) ) )
    trackbar->SetText( node.GetAttribute( _T( "text" ) ) );
  return trackbar;

}

void CWBNumPad::SetTextConversionCallbacks( TOTEXTCALLBACK _toText, FROMTEXTCALLBACK _fromText )
{
  toText = _toText;
  fromText = _fromText;
}

TBOOL CWBNumPad::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  TBOOL ElementTarget = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "wrapper" ) )
    {
      ElementTarget = true;
      break;
    }
  }

  if ( !ElementTarget ) return CWBTrackBar::ApplyStyle( prop, value, pseudo );

  TBOOL Handled = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "wrapper" ) )
    {
      Handled |= wrapperBox.ApplyStyle( this, prop, value, pseudo );
      continue;
    }
  }

  return Handled;
}

void CWBNumPad::SendValueChangeMessage()
{
  App->SendMessage( CWBMessage( App, WBM_VALUECHANGED, GetGuid(), GetValueAsInt() ) );
  App->SendMessage( CWBMessage( App, WBM_NEEDSNUMPADTEXTUPDATE, GetGuid(), GetValueAsInt() ) );
}

TBOOL CWBNumPad::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_TEXTCHANGED:
    if ( textBox && Message.GetTarget() == textBox->GetGuid() )
    {
      if ( fromText )
        fromText( this, textBox->GetText() );
    }
    break;

  case WBM_MOUSEWHEEL:
    if ( App->GetMouseItem() == this || ( MouseOver() && App->GetMouseItem() && App->GetMouseItem()->FindItemInParentTree( this ) ) )
    {
      ApplyStep( Message.Data );
      return true;
    }
    break;
  case WBM_NEEDSNUMPADTEXTUPDATE:
    if ( textBox )
    {
      if ( toText )
        textBox->SetText( toText( Message.Data ) );
    }
    return true;

  case WBM_VALUECHANGED:
    return true;
  }

  return CWBTrackBar::MessageProc( Message );
}

void CWBNumPad::OnDraw( CWBDrawAPI *API )
{
  CRect original;
  DrawBorder( API, wrapperBox.PositionDescriptor.GetPosition( GetClientRect().Size(), GetClientRect().Size(), original ), wrapperBox );

  CWBTrackBar::OnDraw( API );

  //if ( !toText || !fromText )
  //  API->DrawRectBorder( GetClientRect(), CColor( 0xffff0000 ) );
}
