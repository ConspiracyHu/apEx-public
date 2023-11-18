#pragma once
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"

enum DockPosition
{
  Left,
  Right,
  Top,
  Bottom
};

class CapexDocker : public CWBBox
{
protected:

  TBOOL dragging = false;
  CPoint dragStartPos;
  CSize dragStartSize;

  CRect GetDragRect();

public:

  TS32 contentSize = 100;
  TS32 areaSize = 200;

  virtual void OnDraw( CWBDrawAPI *API );

  CapexDocker();
  CapexDocker( CWBItem *Parent, const CRect &Pos );
  virtual ~CapexDocker();

  virtual TBOOL MessageProc( CWBMessage &Message );
  WB_DECLARE_GUIITEM( _T( "apexdocker" ), CWBBox );

  void DockWindow( CWBItem* item, DockPosition dockPosition );
  void DockWindowToChild( CWBItem* childItem, CWBItem* item, DockPosition dockPosition );

  virtual void ResetUIData() {};
  virtual void OnPostDraw( CWBDrawAPI*API );
  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray & pseudo );

  void ApplyAutoScaleStyles();
};
