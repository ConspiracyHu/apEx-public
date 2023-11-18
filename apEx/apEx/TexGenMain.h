#pragma once
#include "apExWindow.h"
#include "..\Phoenix_Tool\Texgen_tool.h"
#include "..\Phoenix_Tool\TexgenPage.h"

class CapexTexGenNewOpWindow : public CapexWindow
{
  CWBButton *NopButton;
  CWBButton *LoadButton;
  CWBButton *SaveButton;
  CWBButton *SubRoutineButton;

  CDictionary<WBGUID, CphxTextureFilter_Tool *> FilterPairing;
  CDictionary<WBGUID, CphxTextureOperator_Subroutine *> SubroutinePairing;
  class CapexTexGenMainWindow *Owner;

  CString searchfilter;

public:

  TS32 FilterCounters[ 10 ];

  CapexTexGenNewOpWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench, CapexTexGenMainWindow *Owner );
  virtual ~CapexTexGenNewOpWindow();
  virtual APEXWINDOW GetWindowType() { return apEx_TexGenNewOpWindow; }

  WB_DECLARE_GUIITEM( _T( "texgennewopwindow" ), CapexWindow );

  TBOOL MessageProc( CWBMessage &Message );
  CWBButton *AddFilter( TS32 Type, CphxTextureFilter_Tool *ID, const CString &Name );
  CWBButton *AddSubroutine( TS32 Type, CphxTextureOperator_Subroutine *Sub );
  virtual void UpdateData();

};

struct SUBROUTINEPARAMLINK
{
  CphxTextureOperator_Tool *Op;
  TS32 OpParam;
  TS32 SubParam;
};

class CapexTexGenMainWindow : public CapexWindow
{
  friend CapexTexGenNewOpWindow;

  enum dragMode
  {
    dragmode_none,
    dragmode_multiselect,
    dragmode_operatormove,
    dragmode_operatorresize,
    dragmode_operatorcopy,
  };

  TS32 GridSize;
  CPoint PanStartPos;
  TBOOL Panning;
  dragMode DragMode;

  CphxTextureOperator_Tool *ResizedOperator;
  CphxTextureOperator_Tool *JustCreatedOperator;

  CphxTextureOperator_Tool *EditedOperator;

  CWBItem *EditBox;

  CDictionary<TS32, SUBROUTINEPARAMLINK> ContextParamLinks;

  virtual TBOOL MessageProc( CWBMessage &Message );
  virtual void OnDraw( CWBDrawAPI *API );

  void DisplayOperator( CWBDrawAPI *API, CphxTextureOperator_Tool *Op, TBOOL ForceSelection = false );
  TBOOL MouseOverOp( CphxTextureOperator_Tool *Op );

  void UpdateScrollbars();
  void OpenNewOpWindow();
  void PreviewOperator( APEXOPID ID );
  void CopySelectedOperators();
  void OpenRightClickMenu( CphxTextureOperator_Tool *Op );
  void StartRenaming();
  void StopRenaming( TBOOL ApplyChanges );

  CRect GetOpPosition( CphxTextureOperator_Tool *Op );

  CDictionary< APEXPAGEID, CPoint > pageOffsets;
  virtual void ResetUIData() override;

public:

  CapexTexGenMainWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTexGenMainWindow();
  virtual APEXWINDOW GetWindowType() { return apEx_TexGenMain; }

  void RestorePageOffset( APEXPAGEID ID );
  void CreateNewOperator( CphxTextureFilter_Tool *Filter );
  void CreateNOPOperator();
  void CreateSaveOperator();
  void CreateLoadOperator();
  void CreateSubroutineOperator();
  void CreateNewSubroutineCall( CphxTextureOperator_Subroutine *Sub );

  void CenterOperator( CphxTextureOperator_Tool *Op );

  virtual void UpdateData();
  void SetEditedOperator( CphxTextureOperator_Tool *op );
};
