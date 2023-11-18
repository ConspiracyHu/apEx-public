#pragma once
#include "apExWindow.h"

class CapexUIXMLEditor : public CapexWindow
{
  APEXWINDOW EditedWindow;
  CString Filename;

  TBOOL MessageProc( CWBMessage &Message );

public:

  CapexUIXMLEditor();
  CapexUIXMLEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexUIXMLEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_XMLEditor; }

  void SetEditedWindow( CapexWindow *w );
  TBOOL SaveAndRefresh();
  virtual void UpdateData();
};
