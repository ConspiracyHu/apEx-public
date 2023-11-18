#pragma once
#include "apExWindow.h"
#include "..\Phoenix_Tool\phxResource.h"
#include "..\Phoenix_Tool\Scene_tool.h"

class CphxSpline_Tool;

struct SPLINEMAPDATA
{
  CphxSpline_Tool *Spline;
  CphxGUID ParamGUID;
  TS32 Index;
  PHXSPLINETYPE Type;
  CphxModelObject_Tool *Object;
  CColor Color;

  SPLINEMAPDATA()
  {
    Spline = NULL;
    Index = 0;
    Object = NULL;
    Color = CColor::FromARGB( 0xffffffff );
  }

  SPLINEMAPDATA( CphxSpline_Tool* spline, PHXSPLINETYPE typ, CphxGUID g, CphxModelObject_Tool *obj )
  {
    Spline = spline;
    ParamGUID = g;
    Index = 0;
    Type = typ;
    Object = obj;
    Color = CColor::FromARGB( 0xffffffff );
  }

  SPLINEMAPDATA( CphxSpline_Tool* spline, PHXSPLINETYPE typ, CphxGUID g, TS32 idx, CColor color, CphxModelObject_Tool *obj )
  {
    Spline = spline;
    ParamGUID = g;
    Index = idx;
    Type = typ;
    Object = obj;
    Color = color;
  }

  bool operator==( const SPLINEMAPDATA &p )
  {
    if ( Type != p.Type ) return false;
    if ( Index != p.Index ) return false;
    if ( Type != Spline_MaterialParam ) return true;
    if ( Object != p.Object ) return false;
    return ParamGUID == p.ParamGUID;
  }
};

class CapexSceneSplineEditor : public CapexWindow
{

  TBOOL AutoKey;
  TBOOL SnapToGrid;

  TBOOL MessageProc( CWBMessage &Message );
  void UpdateSplines();

  CDictionary<TS32, SPLINEMAPDATA> SplineMap;

  virtual void OnDraw( CWBDrawAPI *API );

public:

  CapexSceneSplineEditor();
  CapexSceneSplineEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexSceneSplineEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_SceneSplineEditor; }
  virtual void UpdateData();

  void SetAutoKey( TBOOL ak );
  TBOOL IsAutoKeySet();

  void SetSnapToGrid( TBOOL ak );
  TBOOL IsSnapToGridSet();

  TF32 GetTimePos();
};
