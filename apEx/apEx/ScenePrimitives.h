#pragma once
#include "apExWindow.h"
#include "../Phoenix/Scene.h"

class CphxScene_Tool;

class CapexScenePrimitives : public CapexWindow
{

  TBOOL MessageProc( CWBMessage &Message );
  CArray<CphxScene_Tool*> SubSceneMap;

public:

  CapexScenePrimitives();
  CapexScenePrimitives( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexScenePrimitives();
  virtual APEXWINDOW GetWindowType() { return apEx_ScenePrimitives; }
  virtual void UpdateData();

  void CreateObject( PHXOBJECT Objtype );
};
