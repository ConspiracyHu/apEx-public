#pragma once
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"

class CapexList : public CWBList
{
  CArray<TS32> Levels;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual void StartRenaming();

public:

  WB_DECLARE_GUIITEM( _T( "extendedlist" ), CWBList );

  CapexList();
  CapexList( CWBItem *Parent, const CRect &Pos );
  virtual ~CapexList();

  virtual TS32 AddItem( const CString &Text );
  virtual TS32 AddItem( const CString &Text, TS32 ID );
  virtual TS32 AddItemWithLevel( const CString &Text, TS32 Level );
  virtual TS32 AddItemWithLevel( const CString &Text, TS32 ID, TS32 Level );
  virtual TBOOL DeleteItem( SELECTABLEID ID );
  virtual void Flush();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
};