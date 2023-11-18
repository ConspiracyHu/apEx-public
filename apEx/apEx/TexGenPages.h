#pragma once
#include "apExWindow.h"
#include "..\Phoenix_Tool\TexgenPage.h"

class CapexTexGenPages : public CapexWindow
{
  //CWBList *List;
  //CWBButton *NewPageButton;
  //CWBButton *DeletePageButton;

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  void CreatePage();
  void DeletePage( TS32 ID );
  void SelectPage( TS32 ID );

  CapexTexGenPages();
  CapexTexGenPages( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTexGenPages();
  virtual APEXWINDOW GetWindowType() { return apEx_TexGenPages; }

  void Update();
  virtual void UpdateData();
};
