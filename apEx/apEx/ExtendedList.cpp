#include "BasePCH.h"
#include "ExtendedList.h"

extern TS32 wbItemIDCounter;

void CapexList::OnDraw( CWBDrawAPI *API )
{
  TS32 TabSize = 10;

  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );

  DrawBackground( API );
  API->SetCropToClient( this );

  CPoint p = GetClientRect().TopLeft() + Offset;
  CPoint mousepos = ScreenToClient( App->GetMousePos() );

  TS32 ItemHeight = GetItemHeight();
  CRect ZeroRect = CRect( 0, 0, 0, 0 );

  for ( TS32 x = 0; x < List.NumItems(); x++ )
  {
    if ( p.y > GetClientRect().Height() ) break;

    CRect r = CRect( p, CPoint( ( GetClientRect() + Offset ).x2, p.y + ItemHeight ) );
    p.y += ItemHeight;

    if ( !GetClientRect().Intersects( r ) ) continue;

    WBITEMSTATE bgState = WB_STATE_NORMAL;
    if ( List[ x ].IsSelected() || ( r.Contains( mousepos ) && MouseOver() ) ) bgState = WB_STATE_HOVER;
    if ( List[ x ].IsSelected() && InFocus() ) bgState = WB_STATE_ACTIVE;

    DrawBackgroundItem( API, ListItemStyle.DisplayDescriptor, r, bgState );

    r = ListItemStyle.PositionDescriptor.GetPadding( r.Size(), ZeroRect ) + r.TopLeft();

    CColor TextColor = ListItemStyle.DisplayDescriptor.GetColor( bgState, WB_ITEM_FONTCOLOR );
    WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)ListItemStyle.DisplayDescriptor.GetValue( bgState, WB_ITEM_TEXTTRANSFORM );
    CWBFont *Font = ListItemStyle.GetFont( App, bgState );

    CPoint TextPos = Font->GetTextPosition( List[ x ].GetText(), r, ListItemStyle.TextAlignX, ListItemStyle.TextAlignY, TextTransform );

    //this single line makes this whole class unique:
    TextPos += CPoint( Levels[ x ] * TabSize, 0 );

    Font->Write( API, List[ x ].GetText(), TextPos, TextColor, TextTransform );
  }

  DrawBorder( API );
}

void CapexList::StartRenaming()
{
  if ( !AllowRename ) return;
  if ( !( CursorPosition >= 0 && CursorPosition < List.NumItems() ) ) return;
  if ( EditBox ) SAFEDELETE( EditBox );

  TS32 ItemHeight = GetItemHeight();
  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
  CPoint p = GetClientRect().TopLeft() + Offset;
  p += CPoint( 0, ItemHeight*CursorPosition );
  CRect r = CRect( p, CPoint( ( GetClientRect() + Offset ).x2, p.y + ItemHeight ) );

  CWBTextBox *b = new CWBTextBox( this, r );
  App->ApplyStyle( b );

  b->SetText( List[ CursorPosition ].GetText() );
  b->SetFocus();
  b->EnableHScrollbar( false, false );
  b->EnableVScrollbar( false, false );
  //b->SetBackgroundColor(rgbBackgroundSelectedFocus);
  b->SetSelection( 0, b->GetText().Length() );

  EditBox = b;
}

CapexList::CapexList() : CWBList()
{

}

CapexList::CapexList( CWBItem *Parent, const CRect &Pos ) : CWBList( Parent, Pos )
{

}

CapexList::~CapexList()
{

}

TS32 CapexList::AddItem( const CString &Text )
{
  List += CWBSelectableItem( Text, wbItemIDCounter++, false );
  Levels += 0;

  SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );

  return List.Last().GetID();
}

TS32 CapexList::AddItem( const CString &Text, TS32 ID )
{
  List += CWBSelectableItem( Text, wbItemIDCounter++, false );
  List.Last().SetID( ID );
  Levels += 0;

  SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );

  return List.Last().GetID();
}

TS32 CapexList::AddItemWithLevel( const CString &Text, TS32 Level )
{
  List += CWBSelectableItem( Text, wbItemIDCounter++, false );
  Levels += Level;

  SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );

  return List.Last().GetID();

}

TS32 CapexList::AddItemWithLevel( const CString &Text, TS32 ID, TS32 Level )
{
  List += CWBSelectableItem( Text, wbItemIDCounter++, false );
  List.Last().SetID( ID );
  Levels += Level;

  SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );

  return List.Last().GetID();
}

CWBItem * CapexList::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CapexList * list = new CapexList( Root, Pos );
  TS32 val = 0;
  node.GetAttributeAsInteger( _T( "multiselect" ), &val );
  list->AllowMultiselect = val != 0;
  return list;
}

TBOOL CapexList::DeleteItem( SELECTABLEID ID )
{
  for ( TS32 x = 0; x < List.NumItems(); x++ )
  {
    if ( List[ x ].GetID() == ID )
    {
      List.DeleteByIndex( x );
      Levels.DeleteByIndex( x );

      if ( !AllowMultiselect && List.NumItems() )
      {
        AnchorPosition = max( 0, min( List.NumItems() - 1, x ) );
        SelectItem( List[ AnchorPosition ].GetID() );
      }

      SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );
      return true;
    }
  }

  return false;
}

void CapexList::Flush()
{
  List.Flush();
  Levels.Flush();
}
