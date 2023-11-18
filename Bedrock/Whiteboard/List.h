#pragma once
#include "Application.h"
#include "ItemSelector.h"

#define WBFLAG_TOGGLE		0x01
#define WBFLAG_RANGESELECT	0x02

class CWBList : public CWBItemSelector
{
protected:

  CWBItem *EditBox;

  TBOOL AllowRename;
  TBOOL AllowMultiselect;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

  virtual void StopRenaming( TBOOL ApplyChanges );
  virtual void StartRenaming();
  virtual void RepositionEditBox();

  CWBCSSPropertyBatch ListItemStyle;

  virtual void UpdateScrollPositionToShowItem( TS32 ID );
  virtual void SelectItemEx( SELECTABLEID ID, TS32 Flags );

  TS32 GetItemHeight();

public:

  CWBList();
  CWBList( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBList();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position );
  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "list" ), CWBItemSelector );

  virtual SELECTABLEID AddItem( const CString &Text );
  virtual SELECTABLEID AddItem( const CString& Text, int data );

  virtual SELECTABLEID GetMouseItemID( CPoint MousePos, TBOOL CropCursor = true );

  void Sort( int( __cdecl* SortCallback )( CWBSelectableItem* a, CWBSelectableItem* b ) );
  CWBSelectableItem* GetItemByUserData( int data );
};