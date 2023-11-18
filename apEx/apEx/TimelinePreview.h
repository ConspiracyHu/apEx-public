#pragma once
#include "apExWindow.h"
class CapexRenderTargetCollection;

class CWBDemoPreview : public CWBItem
{
  CCoreComputeShader *PerformMinMax = NULL, *GetMinMaxResult = NULL, *GetHistogramResult = NULL;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

  WB_DECLARE_GUIITEM( _T( "demopreview" ), CWBItem );
  int resolutionX = 0;
  int resolutionY = 0;
  int renderTargetIdx = -1;

public:
  CapexRenderTargetCollection *RenderTargets;

  CWBDemoPreview( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBDemoPreview();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

  void DrawHistogram( CWBDrawAPI *API );
  TBOOL HistogramOpen = false;
  TBOOL BadProjector = false;
  TBOOL pointSampling = true;

  void SetResolution( int x, int y );
  void SetRenderTargetIndex( int idx );
};

class CapexTimelinePreview : public CapexWindow
{

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexTimelinePreview();
  CapexTimelinePreview( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTimelinePreview();
  virtual APEXWINDOW GetWindowType() { return apEx_TimelinePreview; }
  virtual void UpdateData();
  void ToggleHistogram();
  void ToggleBadProjector();
  void TogglePointSampling();
};
