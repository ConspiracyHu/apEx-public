#pragma once
#include "apExWindow.h"

class CphxModel_Tool;

class CapexModelList : public CapexWindow
{

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexModelList();
  CapexModelList( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelList();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelList; }
  virtual void UpdateData();

  void CreateModel();
  void CopyModel();
  void DeleteModel( CphxModel_Tool *m );
  void SelectModel( TS32 ID );

  //void SetEditedScene(APEXSCENEID ID);

};
