#pragma once
#include "apExWindow.h"

class CphxScene_Tool;

class CapexSceneList : public CapexWindow
{

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexSceneList();
  CapexSceneList( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexSceneList();
  virtual APEXWINDOW GetWindowType() { return apEx_SceneList; }
  virtual void UpdateData();

  CphxScene_Tool *CreateScene();
  void DeleteScene( TS32 ID );
  void SelectScene( TS32 ID );
  void CopyScene( CphxScene_Tool *source );

};
