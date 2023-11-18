#pragma once
#include "apExWindow.h"

class CapexUICSSEditor : public CapexWindow
{
  APEXWINDOW EditedWindow;
  CString Filename;

  TBOOL MessageProc( CWBMessage &Message );

public:

  CapexUICSSEditor();
  CapexUICSSEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexUICSSEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_CSSEditor; }

  void SetEditedWindow( CapexWindow *w );
  TBOOL SaveAndRefresh();
  virtual void UpdateData();
};
