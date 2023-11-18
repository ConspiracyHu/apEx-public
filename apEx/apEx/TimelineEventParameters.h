#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Timeline_tool.h"

class CapexTimelineEventParameters : public CapexWindow
{
  CStyleManager StyleManager;
  CphxGUID EditedEvent;

  CArray<CWBItem*> GeneratedParamRoots;

  void FlushParameters();
  void LoadCSS();

  CDictionary<CphxGUID, CWBItem *> ParamUIMap;
  CDictionary<TS32, CphxGUID> TextureIDMap;
  CphxMaterialParameter_Tool *GetTargetParam( CWBItem *i );
  void BuildTextureSelectorUI( CWBItem *Button, TBOOL RTOnly = false, TBOOL HideHidden = false );
  TBOOL MessageProc( CWBMessage &Message );
  virtual void AddParameterUI( CWBItem *Root, CphxMaterialParameter_Tool *Param );

  void UpdateSceneCamClipLists( CphxScene_Tool *Scene );

  CDictionary<WBGUID, CphxGUID> TextureLocatorMap;

public:

  CapexTimelineEventParameters();
  CapexTimelineEventParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTimelineEventParameters();
  virtual APEXWINDOW GetWindowType() { return apEx_TimelineEventParameters; }
  virtual void UpdateData();


  void SetEditedEvent( CphxEvent_Tool *event );
  virtual void ReloadLayout();
};
