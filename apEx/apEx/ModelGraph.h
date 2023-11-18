#pragma once
#include "apExWindow.h"

class CphxModelObject_Tool;
class CphxMeshFilter_Tool;

class CapexModelGraph : public CapexWindow
{
  TBOOL MessageProc( CWBMessage &Message );
  CDictionary<TS32, CphxModelObject_Tool*> ModelObjects;

public:
  CDictionary<TS32, CphxMeshFilter_Tool*> ModelFilters;


  CapexModelGraph();
  CapexModelGraph( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelGraph();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelGraph; }
  virtual void UpdateData();
};
