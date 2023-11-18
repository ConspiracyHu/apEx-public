#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Timeline_tool.h"
#include "../Phoenix_Tool/Material_Tool.h"

enum TIMELINEDRAGMODE
{
  DRAGMODE_NONE = 0,
  DRAGMODE_DRAG = 1,
  DRAGMODE_SELECT = 2,
  DRAGMODE_MOVE = 3,
  DRAGMODE_NEWEVENT = 4,
  DRAGMODE_TIMESEEK = 5,
  DRAGMODE_RESIZELEFT = 6,
  DRAGMODE_RESIZERIGHT = 7,
  DRAGMODE_DRAGLASTDRAWNPASS = 8,
};

struct EVENTCREATORLINK
{
  //TS32 LinkID;
  enum PHXEVENTTYPE EventType;
  CphxMaterialTechnique_Tool *Tech;
};

class CWBTimeline : public CWBItem
{
  friend class CapexTimelineEditor;

  CphxEvent_Tool *ResizedEvent;

  TIMELINEDRAGMODE Dragmode;
  PHXEVENTTYPE EventToCreate;
  CphxMaterialTechnique_Tool *Tech;

  CPoint LastMousePos;

  TS32 LineHeight;
  TF32 Zoom;
  TS32 Offset, StoredOffset;
  TS32 OffsetPass, StoredOffsetPass;
  TS32 RemainingFrames, RemainingPasses;


  CRect GridRect;

  TS32 ClickedPass;
  TS32 ClickedFrame;

  CphxGUID SelectedEvent;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

  WB_DECLARE_GUIITEM( _T( "timelineeditorphx" ), CWBItem );

  CRect GetEventPosition( CphxEvent_Tool *Event );
  CRect GetEventPosition( TS32 x1, TS32 x2, TS32 pass );
  void DrawEvent( CWBDrawAPI *API, CphxEvent_Tool *Event, TBOOL ForceSelect );
  void DoEventDragging( CPoint mousePos );

  TS32 ClientToFrame( TS32 c );
  TS32 FrameToClient( TS32 c );

  TS32 GetCursorSnap( TS32 original, CphxEvent_Tool *Skipped, CPoint mousePos );

  void SelectEvent( CphxEvent_Tool *ev );
  void WriteWithOutline( CWBDrawAPI *API, CWBFont *Font, CString &text, CPoint Pos );
  void DeleteSelectedEvents();

  void StartRenaming();
  void StopRenaming( TBOOL ApplyChanges );
  CWBItem *EditBox;

public:

  CWBTimeline( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBTimeline();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

  CphxEvent_Tool *GetSelectedEvent();

};

class CapexTimelineEditor : public CapexWindow
{
  virtual TBOOL MessageProc( CWBMessage &Message );
  CArray<EVENTCREATORLINK> EventLinks;

  void AddEventLink( CWBItemSelector *m, PHXEVENTTYPE type, CphxMaterialTechnique_Tool *Tech, CString Text );
  void OpenSongImport();
  virtual void OnDraw( CWBDrawAPI *API );

public:

  CapexTimelineEditor();
  CapexTimelineEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTimelineEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_TimelineEditor; }
  virtual void UpdateData();

  void SelectEvent( CphxEvent_Tool *ev );
  void CopySelected();
  void CopyEvent( CphxEvent_Tool *Event, TS32 TargetPass );
};

