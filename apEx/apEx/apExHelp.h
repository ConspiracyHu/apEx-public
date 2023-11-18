#pragma once
#include "apExWindow.h"

class CapexHelp : public CapexWindow
{

public:

  CapexHelp();
  CapexHelp( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexHelp();
  virtual APEXWINDOW GetWindowType() { return apEx_Help; }
  virtual void UpdateData();

};