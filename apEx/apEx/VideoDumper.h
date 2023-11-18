#pragma once
#include "apExWindow.h"
class CapexRenderTargetCollection;

class CapexVideoDumperPreview : public CWBItem
{
  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

  WB_DECLARE_GUIITEM( _T( "dumperpreview" ), CWBItem );

public:

  void PrepareFrame();

  CapexRenderTargetCollection *RenderTargets;

  CapexVideoDumperPreview( CWBItem *Parent, const CRect &Pos );
  virtual ~CapexVideoDumperPreview();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
};

class CapexVideoDumper : public CWBItem
{

  TBOOL Recording = false;
  TS32 CurrentFrame = 0;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

  CWBBox *CenterBox = NULL;

  TS32 XRes, YRes, StartFrame, EndFrame;
  TF32 Framerate;

public:

  CapexVideoDumper();
  CapexVideoDumper( CWBItem *Parent, const CRect &Pos );
  virtual ~CapexVideoDumper();
  virtual APEXWINDOW GetWindowType() { return apEx_Help; }
  virtual void UpdateData();

};