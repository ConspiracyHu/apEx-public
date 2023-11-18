#pragma once
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"
#include "..\Phoenix_Tool\phxSplineExt.h"

class CphxSpline_Tool;
class CphxSpline_Tool_float16;
class CphxSpline_Tool_Quaternion16;

class CSplineEditorLink
{

public:

  CColor SplineColor;
  CString SplineName;
  CphxSpline_Tool *Spline;

  CSplineEditorLink();

};

#define WBM_SPLINECHANGED		0x10000
#define WBM_TIMEPOSITIONCHANGED 0x10001

enum SPLINEDRAGMODE
{
  DRAG_NONE,
  DRAG_KEY,
  DRAG_KEYHELPER,
  DRAG_SELECTION,
  DRAG_SUBKEY,
  DRAG_TIMEMOVE,
  DRAG_DRAGVIEW,
};

class CapexSplineEditor_phx : public CWBItem
{
  CphxSpline_Tool *ContextSpline = NULL;
  static CArray<CphxSplineKey_Tool> CopiedSplineKeys;

  SPLINEDRAGMODE DragMode;

  CArray<CSplineEditorLink> Splines;
  TS32 KeySize;
  TS32 SplineClickSensitivity;
  CColor KeyColor;
  CColor KeyColorHighlight;
  CColor KeyColorSelectNoFocus;
  CColor KeyColorSelectFocus;

  virtual void OnDraw( CWBDrawAPI *API );

  TBOOL Limited;

  TBOOL MouseOverKey( CphxSpline_Tool *&Spline, TS32 &Key );
  TBOOL MouseOverSpline( CphxSpline_Tool *&Spline, TF32 &t );
  TBOOL MouseOverSpline( CphxSpline_Tool *&Spline, TF32 &t, CString &name );
  TBOOL MouseOverKeyHelper( CphxSpline_Tool *&Spline, TS32 &Key, TS32 &HelperID );

  void StoreKeyPositions();
  void DeselectAllKeys();
  void MouseMoveSplineKeys();
  void MouseMoveSplineKeyHelper();
  void DeleteSelectedKeys();
  void DrawGrid( CWBDrawAPI *API );
  void DrawGridNumbers( CWBDrawAPI *API );
  void WriteWithOutline( CWBDrawAPI *API, CWBFont *Font, CString &text, CPoint Pos );

  TS32 TimeToX( TS32 Time );
  TS32 XToTime( TS32 Time );
  TF32 ValueToY( TF32 Value );
  TF32 YToValue( TS32 Y );

  TF32 SnapValue( TF32 StoredValue, TF32 v, TBOOL BezierHelper );
  TS32 SnapTime( TS32 StoredTime, TS32 t, TBOOL BezierHelper );

  WB_DECLARE_GUIITEM( _T( "splineeditorphx" ), CWBItem );

  TF32 TimePos;

  TF32 ZoomX, ZoomY;
  TF32 OffsetX, OffsetY;
  TF32 OffsetX_, OffsetY_;

  TBOOL ContextMenuViable;

  CphxSplineKey_Tool *DraggedKey;
  TS32 DraggedHelper;
  CphxSpline_Tool *DraggedSpline;

  virtual TBOOL MessageProc( CWBMessage &Message );
  TBOOL ApplySplineDragUpdate();
  TF32 GridDensity;
  TS32 GridDensityX = 8;

public:

  TBOOL HighlightBackground;
  TBOOL SnapToGrid;

  CapexSplineEditor_phx( CWBItem *Parent, const CRect &Pos );
  virtual ~CapexSplineEditor_phx();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

  void AddSpline( CphxSpline_Tool *Spline, CColor Color, CString &Name );
  void FlushSplines();
  TF32 GetTimePosition();
  void SetTimePosition( TF32 t );
  void SetGridDensity( TF32 g );
};