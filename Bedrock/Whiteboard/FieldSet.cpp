#include "BasePCH.h"
#include "FieldSet.h"

void CWBFieldSet::OnDraw( CWBDrawAPI *API )
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  CRect r = GetWindowRect();

  TS32 FontCenter = 0;
  TS32 TextWidth = 0;

  if ( Font )
  {
    FontCenter = Font->GetMedian();
    TextWidth = Font->GetWidth( Text, false, TextTransform );
  }

  r += CRect( 0, -FontCenter, 0, 0 );

  //background display
  //API->DrawRect(r,rgbBackground);

  TS32 x1 = ( r.Width() - TextWidth ) / 2 - 5;
  TS32 x2 = ( r.Width() + TextWidth ) / 2 + 5;

  CColor FrameColor = CSSProperties.DisplayDescriptor.GetColor( GetState(), WB_ITEM_BORDERCOLOR );

  API->DrawRect( CRect( r.TopLeft(), r.TopLeft() + CPoint( x1, 1 ) ), FrameColor );
  API->DrawRect( CRect( r.TopLeft() + CPoint( x2, 0 ), r.TopRight() + CPoint( 0, 1 ) ), FrameColor );

  API->DrawRect( CRect( r.TopLeft(), r.BottomLeft() + CPoint( 1, 0 ) ), FrameColor );
  API->DrawRect( CRect( r.BottomLeft() - CPoint( 0, 1 ), r.BottomRight() ), FrameColor );
  API->DrawRect( CRect( r.TopRight() - CPoint( 1, 0 ), r.BottomRight() ), FrameColor );

  if ( Font )
    Font->Write( API, Text, r.TopLeft() + CPoint( x1 + 5, -FontCenter ), CSSProperties.DisplayDescriptor.GetColor( i, WB_ITEM_FONTCOLOR ), TextTransform );
}

CWBFieldSet::CWBFieldSet() : CWBItem()
{
}

CWBFieldSet::CWBFieldSet( CWBItem *Parent, const CRect &Pos, const TCHAR *Txt ) : CWBItem()
{
  Initialize( Parent, Pos, Txt );
}

CWBFieldSet::~CWBFieldSet()
{

}

TBOOL CWBFieldSet::Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *Txt )
{
  Text = Txt;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;

  //TextAlignment = WBTA_CENTERX;
  //DisplayDescriptor.SetValue(WB_STATE_NORMAL, WB_ITEM_BORDERCOLOR, CColor::FromARGB(0xff434346));

  CWBFont *Font = App->GetDefaultFont();
  TS32 top = 2;
  if ( Font ) top = Font->GetLineHeight();

  SetClientPadding( 2, top, 2, 2 );
  return true;
}


CWBItem * CWBFieldSet::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBFieldSet * fieldset = new CWBFieldSet( Root, Pos );
  if ( node.HasAttribute( _T( "text" ) ) )
    fieldset->SetText( node.GetAttribute( _T( "text" ) ) );
  return fieldset;
}
