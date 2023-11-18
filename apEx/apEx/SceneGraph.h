#pragma once
#include "apExWindow.h"

class CphxObject_Tool;

class CapexSceneGraph : public CapexWindow
{
  TBOOL MessageProc( CWBMessage &Message );
  CDictionary<TS32, CphxObject_Tool*> ObjectMap;

  TS32 RightClickedItem;
  TBOOL ListRightClicked;

  void SceneGraphBuilder( CphxObject_Tool *Parent, TS32 Level );

public:

  CapexSceneGraph();
  CapexSceneGraph( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexSceneGraph();
  virtual APEXWINDOW GetWindowType() { return apEx_SceneGraph; }
  virtual void UpdateData();
};

