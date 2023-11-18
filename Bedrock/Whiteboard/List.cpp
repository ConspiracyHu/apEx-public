#include "BasePCH.h"
#include "List.h"
#include "TextBox.h"

void CWBList::OnDraw( CWBDrawAPI *API )
{
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
    Font->Write( API, List[ x ].GetText(), TextPos, List[ x ].IsColorSet() ? List[ x ].GetColor() : TextColor, TextTransform );
  }

  DrawBorder( API );
}

CWBList::CWBList() : CWBItemSelector()
{
}

CWBList::CWBList( CWBItem *Parent, const CRect &Pos ) : CWBItemSelector()
{
  Initialize( Parent, Pos );
}

CWBList::~CWBList()
{

}

TBOOL CWBList::Initialize( CWBItem *Parent, const CRect &Position )
{
  EditBox = NULL;

  if ( !CWBItemSelector::Initialize( Parent, Position ) ) return false;

  EnableVScrollbar( true, true );

  AllowRename = true;
  AllowMultiselect = false;

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );
  ListItemStyle.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );
  ListItemStyle.DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3f3f46 ) );
  ListItemStyle.DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3399ff ) );
  ListItemStyle.PositionDescriptor.SetValue( WB_HEIGHT, 0, 12 );

  SetBorderSizes( 1, 1, 1, 1 );

  return true;
}

TBOOL CWBList::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
  {
    TBOOL b = CWBItem::MessageProc( Message );
    TS32 mi, ma, vi;

    GetHScrollbarParameters( mi, ma, vi );
    SetHScrollbarParameters( mi, ma, GetClientRect().Width() );
    GetVScrollbarParameters( mi, ma, vi );
    SetVScrollbarParameters( mi, ma, GetClientRect().Height() );

    if ( GetClientRect().Height() >= GetItemHeight()*List.NumItems() )
      SetVScrollbarPos( 0, true );

    return b;
  }
  break;
  case WBM_LEFTBUTTONDOWN:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseItem() == this )
    {
      if ( !AllowMultiselect )
        App->SetCapture( this );

      StopRenaming( true );

      TS32 idx = GetMouseItemID( Message.GetPosition() );
      if ( idx != -1 )
        SelectItemEx( idx, ( App->GetCtrlState()*WBFLAG_TOGGLE ) | ( App->GetShiftState()*WBFLAG_RANGESELECT ) );
    }
    return true;

  case WBM_MOUSEMOVE:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseCaptureItem() == this )
    {
      TS32 idx = GetMouseItemID( Message.GetPosition(), false );
      if ( idx != -1 )
        SelectItemEx( idx, ( App->GetCtrlState()*WBFLAG_TOGGLE ) | ( App->GetShiftState()*WBFLAG_RANGESELECT ) );
      return true;
    }
    break;

  case WBM_LEFTBUTTONUP:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseCaptureItem() == this )
    {
      App->ReleaseCapture();
      return true;
    }
    break;

  case WBM_MOUSEWHEEL:
    SetVScrollbarPos( GetVScrollbarPos() - Message.Data * 3 * GetItemHeight(), true );

    return true;

  case WBM_VSCROLL:
  {
    TBOOL Result = CWBItem::MessageProc( Message );
    RepositionEditBox();
    return Result;
  }
  break;

  case WBM_FOCUSLOST:
    if ( EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( true );
      return true;
    }
    break;

  case WBM_COMMAND:
    if ( EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( true );
      return true;
    }
    break;

  case WBM_KEYDOWN:
    if ( !InFocus() ) break;

    if ( Message.Key == VK_ESCAPE && EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( false );
    }

    if ( EditBox ) break;

    //handle cursor movement keys
    switch ( Message.Key )
    {
    case VK_UP:
    {
      if ( List.NumItems() )
      {
        CursorPosition = max( 0, min( List.NumItems() - 1, CursorPosition - 1 ) );
        SelectItemEx( List[ CursorPosition ].GetID(), App->GetShiftState()*WBFLAG_RANGESELECT );
      }
      return true;
    }
    break;
    case VK_DOWN:
    {
      if ( List.NumItems() )
      {
        CursorPosition = max( 0, min( List.NumItems() - 1, CursorPosition + 1 ) );
        SelectItemEx( List[ CursorPosition ].GetID(), App->GetShiftState()*WBFLAG_RANGESELECT );
      }
      return true;
    }
    break;
    case VK_PRIOR:
    {
      TS32 height = max( 1, GetClientRect().Height() / GetItemHeight() - 1 );
      if ( List.NumItems() )
      {
        CursorPosition = max( 0, min( List.NumItems() - 1, CursorPosition - height ) );
        SelectItemEx( List[ CursorPosition ].GetID(), App->GetShiftState()*WBFLAG_RANGESELECT );
      }
      return true;
    }
    break;
    case VK_NEXT:
    {
      TS32 height = max( 1, GetClientRect().Height() / GetItemHeight() - 1 );
      if ( List.NumItems() )
      {
        CursorPosition = max( 0, min( List.NumItems() - 1, CursorPosition + height ) );
        SelectItemEx( List[ CursorPosition ].GetID(), App->GetShiftState()*WBFLAG_RANGESELECT );
      }
      return true;
    }
    case VK_HOME:
    {
      SelectItemEx( List[ 0 ].GetID(), App->GetShiftState()*WBFLAG_RANGESELECT );
      return true;
    }
    break;
    case VK_END:
    {
      SelectItemEx( List[ List.NumItems() - 1 ].GetID(), App->GetShiftState()*WBFLAG_RANGESELECT );
      return true;
    }
    case VK_F2:
    {
      if ( !List.NumItems() ) break;
      if ( CursorPosition < 0 || CursorPosition >= List.NumItems() ) break;
      if ( !AllowRename ) break;

      StartRenaming();

      return true;
    }
    break;
    }
    break;

  case WBM_SELECTITEM:
    UpdateScrollPositionToShowItem( Message.Data );
    App->SendMessage( CWBMessage( App, WBM_ITEMSELECTED, GetGuid(), Message.Data ) );
    return true;
  }

  return CWBItemSelector::MessageProc( Message );
}

void CWBList::SelectItemEx( SELECTABLEID ID, TS32 Flags )
{
  if ( !AllowMultiselect ) //single-select fallback
  {
    CWBItemSelector::SelectItem( ID );
    return;
  }

  //multi-select

  if ( !List.NumItems() ) return;
  TBOOL SelectionChanged = false;

  TS32 ListStatusSize = List.NumItems();
  TBOOL *ListStatus = new TBOOL[ ListStatusSize ];
  for ( TS32 x = 0; x < ListStatusSize; x++ )
    ListStatus[ x ] = List[ x ].IsSelected();

  //multi select
  if ( !( Flags & WBFLAG_TOGGLE ) )
    for ( TS32 x = 0; x < List.NumItems(); x++ )
      List[ x ].Select( false );

  if ( Flags & WBFLAG_RANGESELECT )
  {
    //shift selection
    TS32 trg = GetItemIndex( ID );

    if ( trg == -1 )
    {
      SAFEDELETEA( ListStatus );
      return;
    }

    CursorPosition = trg;

    TS32 mi = min( trg, AnchorPosition );
    TS32 ma = max( trg, AnchorPosition );
    mi = max( 0, min( mi, List.NumItems() - 1 ) );
    ma = max( 0, min( ma, List.NumItems() - 1 ) );

    for ( TS32 x = mi; x <= ma; x++ )
    {
      List[ x ].Select( true );
      App->SendMessage( CWBMessage( App, WBM_SELECTITEM, GetGuid(), List[ x ].GetID() ) );
      //LOG_DBG("%s got item %d selected", GetID().GetPointer(), List[x].GetID());
    }
  }
  else
  {
    //normal selection
    for ( TS32 x = 0; x < List.NumItems(); x++ )
      if ( List[ x ].GetID() == ID )
      {
        CursorPosition = AnchorPosition = x;

        if ( !( Flags & WBFLAG_TOGGLE ) )
        {
          List[ x ].Select( true );
          App->SendMessage( CWBMessage( App, WBM_SELECTITEM, GetGuid(), List[ x ].GetID() ) );
          //LOG_DBG("%s got item %d selected", GetID().GetPointer(), List[x].GetID());
        }
        else
        {
          List[ x ].Select( !List[ x ].IsSelected() );
          if ( List[ x ].IsSelected() )
          {
            App->SendMessage( CWBMessage( App, WBM_SELECTITEM, GetGuid(), List[ x ].GetID() ) );
            //LOG_DBG("%s got item %d selected", GetID().GetPointer(), List[x].GetID());
          }
        }
        break;
      }
  }

  for ( TS32 x = 0; x < ListStatusSize; x++ )
    if ( List[ x ].IsSelected() != ListStatus[ x ] )
    {
      SelectionChanged = true;
      break;
    }

  if ( SelectionChanged )
    App->SendMessage( CWBMessage( App, WBM_SELECTIONCHANGE, GetGuid(), 0 ) );

  SAFEDELETEA( ListStatus );
}

SELECTABLEID CWBList::GetMouseItemID( CPoint MousePos, TBOOL CropCursor /*= true*/ )
{
  CPoint mp = ScreenToClient( MousePos );
  if ( CropCursor && !GetClientRect().Contains( mp ) ) return -1;
  CPoint Offset = CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
  mp += Offset;

  TS32 p = 0;
  TS32 ItemHeight = GetItemHeight();

  for ( TS32 x = 0; x < List.NumItems(); x++ )
  {
    if ( mp.y >= p && mp.y < p + ItemHeight ) return List[ x ].GetID();
    p += ItemHeight;
  }

  return -1;
}

void CWBList::Sort( int( __cdecl* SortCallback )( CWBSelectableItem* a, CWBSelectableItem* b ) )
{
  List.Sort( SortCallback );
}

CWBSelectableItem* CWBList::GetItemByUserData( int data )
{
  for ( int x = 0; x < List.NumItems(); x++ )
    if ( List[ x ].GetData() == data )
      return List.GetPointer( x );

  return nullptr;
}

void CWBList::StartRenaming()
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

void CWBList::StopRenaming( TBOOL ApplyChanges )
{
  if ( !AllowRename ) return;
  if ( !( CursorPosition >= 0 && CursorPosition < List.NumItems() ) ) return;

  if ( !EditBox ) return;

  if ( ApplyChanges )
  {
    CWBTextBox *ib = (CWBTextBox*)EditBox;

    if ( List[ CursorPosition ].GetText() != ib->GetText() )
    {
      List[ CursorPosition ].SetText( ib->GetText() );
      App->SendMessage( CWBMessage( App, WBM_ITEMRENAMED, GetGuid(), List[ CursorPosition ].GetID() ) );
    }
  }

  SAFEDELETE( EditBox );
}

void CWBList::RepositionEditBox()
{
  if ( !EditBox ) return;
  TS32 ItemHeight = GetItemHeight();
  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
  CPoint p = GetClientRect().TopLeft() + Offset;
  p += CPoint( 0, ItemHeight*CursorPosition );
  CRect r = CRect( p, CPoint( ( GetClientRect() + Offset ).x2, p.y + ItemHeight ) );
  EditBox->SetPosition( r );
}

void CWBList::UpdateScrollPositionToShowItem( TS32 ID )
{
  TS32 FixItemHeight = GetItemHeight();

  TS32 pos = GetItemIndex( ID );
  TS32 start = max( 0, pos*FixItemHeight );
  TS32 end = start + FixItemHeight;

  CRect VisibleRect = GetClientRect() + CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
  CPoint CPos = CPoint( 0, start );
  if ( VisibleRect.Contains( CPos ) && VisibleRect.Contains( CPos + CPoint( 0, FixItemHeight ) ) ) return; //no need to adjust

  if ( CPos.y < VisibleRect.y1 ) SetVScrollbarPos( GetVScrollbarPos() - ( VisibleRect.y1 - CPos.y ) );
  if ( CPos.y + FixItemHeight > VisibleRect.y2 ) SetVScrollbarPos( GetVScrollbarPos() + ( CPos.y + FixItemHeight - VisibleRect.y2 ) );
}

TBOOL CWBList::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  TBOOL ElementTarget = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "listitem" ) )
    {
      ElementTarget = true;
      break;
    }
  }

  //apply font styling to list item anyway
  if ( !ElementTarget ) InterpretFontString( ListItemStyle, prop, value, pseudo );
  if ( !ElementTarget ) return CWBItem::ApplyStyle( prop, value, pseudo );

  TBOOL Handled = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "listitem" ) )
    {
      Handled |= ListItemStyle.ApplyStyle( this, prop, value, pseudo );
      if ( Handled )
        SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );
      continue;
    }
  }

  return Handled;
}

CWBItem * CWBList::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBList * list = new CWBList( Root, Pos );
  TS32 val = 0;
  node.GetAttributeAsInteger( _T( "multiselect" ), &val );
  list->AllowMultiselect = val != 0;
  return list;
}

SELECTABLEID CWBList::AddItem( const CString &Text )
{
  SELECTABLEID ID = CWBItemSelector::AddItem( Text );
  SetVScrollbarParameters( 0, List.NumItems()*GetItemHeight(), GetClientRect().Height() );
  return ID;
}

SELECTABLEID CWBList::AddItem( const CString& Text, int data )
{
  auto id = AddItem( Text );
  GetItem( id )->SetData( data );
  return id;
}

TS32 CWBList::GetItemHeight()
{
  CRect r = ListItemStyle.PositionDescriptor.GetPosition( GetClientRect().Size(), CSize( 0, 0 ), GetClientRect() );
  return r.Height();
}
