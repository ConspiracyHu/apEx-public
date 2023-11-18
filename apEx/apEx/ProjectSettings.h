#pragma once
#include "apExWindow.h"

class CapexProjectSettings : public CapexWindow
{

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexProjectSettings();
  CapexProjectSettings( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexProjectSettings();
  virtual APEXWINDOW GetWindowType() { return apEx_ProjectSettings; }
  virtual void UpdateData();
};
