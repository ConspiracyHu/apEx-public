#pragma once
#include "apExWindow.h"
#include "../Phoenix/Mesh.h"

class CapexModelPrimitives : public CapexWindow
{

  TBOOL MessageProc( CWBMessage &Message );

  void CreatePrimitive( PHXMESHPRIMITIVE p );
  void CreateFilter( PHXMESHFILTER p );

public:

  CapexModelPrimitives();
  CapexModelPrimitives( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelPrimitives();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelPrimitives; }
  virtual void UpdateData();
};
