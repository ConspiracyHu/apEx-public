#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/apxProject.h"

class CapexRenderTargetEditor : public CapexWindow
{

  CphxRenderTarget_Tool *GetEditedRT();
  CphxRenderLayerDescriptor_Tool *GetEditedRL();
  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexRenderTargetEditor();
  CapexRenderTargetEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexRenderTargetEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_RenderTargetEditor; }
  virtual void UpdateData();
};
