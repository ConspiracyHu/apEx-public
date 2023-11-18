#pragma once
#include "Application.h"

class CWBFieldSet : public CWBItem
{
  CString Text;
  virtual void OnDraw( CWBDrawAPI *API );

public:

  CWBFieldSet();
  CWBFieldSet( CWBItem *Parent, const CRect &Pos, const TCHAR *txt = _T( "" ) );
  virtual ~CWBFieldSet();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *txt = _T( "" ) );

  CString GetText() const { return Text; }
  void SetText( CString val ) { Text = val; }

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "fieldset" ), CWBItem );
};