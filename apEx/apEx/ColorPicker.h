#pragma once
#include "apExWindow.h"

class CapexColorPicker : public CapexWindow
{

public:

  CapexColorPicker();
  CapexColorPicker( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexColorPicker();
};
