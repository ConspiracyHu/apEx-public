#pragma once
#include "apExWindow.h"

class CapexSceneClips : public CapexWindow
{

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexSceneClips();
  CapexSceneClips( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexSceneClips();
  virtual APEXWINDOW GetWindowType() { return apEx_SceneClips; }
  virtual void UpdateData();

  void CreateClip();
  void CopyClip();
  void DeleteClip( TS32 ID );
  void SelectClip( TS32 ID );
};
