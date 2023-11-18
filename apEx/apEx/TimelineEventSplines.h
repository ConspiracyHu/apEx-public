#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Timeline_tool.h"

class CapexTimelineEventSplines : public CapexWindow
{
  CphxEvent_Tool *EditedEvent;

  virtual TBOOL MessageProc( CWBMessage &Message );
  virtual void OnDraw( CWBDrawAPI *API );

  CDictionary<TS32, class  CphxSpline_Tool_float16*> SplineLinks;

public:

  CapexTimelineEventSplines();
  CapexTimelineEventSplines( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTimelineEventSplines();
  virtual APEXWINDOW GetWindowType() { return apEx_TimelineEventSplines; }
  virtual void UpdateData();

  void UpdateSplines( CphxEvent_Tool *e );
};
