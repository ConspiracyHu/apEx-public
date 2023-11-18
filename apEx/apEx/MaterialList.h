#pragma once
#include "apExWindow.h"

class CapexMaterialList : public CapexWindow
{

public:

  CapexMaterialList();
  CapexMaterialList( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexMaterialList();
  virtual APEXWINDOW GetWindowType() { return apEx_MaterialList; }
  virtual void UpdateData();
};
