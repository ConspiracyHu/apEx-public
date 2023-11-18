#pragma once
#include "Application.h"
#include "ValueInput.h"

class CWBTrackBar : public CWBItem, public CWBValueInput
{
  CString Text;

  TS32 ButtonClicked;

  virtual void DrawButton( CWBDrawAPI *API, CRect Pos, TBOOL Pushed, CWBCSSPropertyBatch& cssProps );
  virtual CRect GetButtonPos( TS32 ButtonID );
  virtual void SetValueToMouseX( TS32 mx );

  virtual void SendValueChangeMessage();

  CWBCSSPropertyBatch TrackbarProperties;
  CWBCSSPropertyBatch ButtonProperties[ 2 ];

protected:
  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CWBTrackBar();
  CWBTrackBar( CWBItem *Parent, const CRect &Pos, TS32 Min, TS32 Max, TS32 Value );
  virtual ~CWBTrackBar();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, TS32 Min, TS32 Max, TS32 Value );
  virtual WBITEMSTATE GetState();
  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );

  CString GetText() const { return Text; }
  virtual void SetText( const CString& val );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "trackbar" ), CWBItem, CWBValueInput );

  void SetButtonWidth( TS32 buttonWidth );
};