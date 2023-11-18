#pragma once
#include "../../Bedrock/WhiteBoard/TrackBar.h"

class CWBNumPad : public CWBTrackBar
{
  typedef CString(__cdecl *TOTEXTCALLBACK)(TS32 value);
  typedef void(__cdecl *FROMTEXTCALLBACK)(CWBValueInput* value , CString& text);

  TOTEXTCALLBACK toText;
  FROMTEXTCALLBACK fromText;

  CWBTextBox* textBox;
  CWBCSSPropertyBatch wrapperBox;

  virtual void SendValueChangeMessage();
  virtual TBOOL MessageProc(CWBMessage &Message);

  void OnDraw(CWBDrawAPI *API);

public:

  CWBNumPad();
  CWBNumPad(CWBItem *Parent, const CRect &Pos, TS32 Min, TS32 Max, TS32 Value);
  virtual ~CWBNumPad();
  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position, TS32 Min, TS32 Max, TS32 Value);

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T("numpad"), CWBItem, CWBValueInput);

  void SetTextConversionCallbacks(TOTEXTCALLBACK toText, FROMTEXTCALLBACK fromText);
  TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );
};
