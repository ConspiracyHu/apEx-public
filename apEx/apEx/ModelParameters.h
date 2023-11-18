#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Model_Tool.h"

class CapexModelParameters : public CapexWindow
{

  CStyleManager StyleManager;
  void LoadCSS();

  CDictionary<CString, CString> TrackBarTextMap;
  CDictionary<CString, TS32> UIParameterMap;
  CDictionary<CString, TS32> UIParameterFilterMap;
  CDictionary<TS32, CphxTextureOperator_Tool*> TextureListMap;

  TBOOL MessageProc( CWBMessage &Message );

  void SetTrackBarValue( TCHAR *Name );
  void SetTrackBarValue_Filter( TCHAR *Name );
  void SetButtonValue( TCHAR *Name );
  void SetButtonValue_Filter( TCHAR *Name );
  void SetButtonTextureValue_Filter( TCHAR *Name );
  void BuildArcList( TCHAR *Name );
  void BuildMeshList( TCHAR *Name );
  void BuildTreeSpeciesList( TCHAR *Name );
  void BuildCloneSourceList( TCHAR *Name );
  void BuildCloneContentList( TCHAR *Name );
  void BuildMergeSourceList( TCHAR* Name );
  void BuildMergeContentList( TCHAR* Name );
  void SetlistSelection( TCHAR *Name );
  void SetlistSelection_Filter( TCHAR *Name );
  void InitTrackBarMap();
  void CloneAdd();
  void CloneRemove();
  void MergeAdd();
  void MergeRemove();
  void BuildTextureList( TCHAR *Name );
  void BuildCopyList( TCHAR *Name );
  void SetTextData( TCHAR *Name );
  void SetSrtInputValue( TCHAR *Name );
  void BuildCSGList( TCHAR *Name );
  void BuildTintTextureSelectorUI( CWBButton* button );

  CDictionary<TS32, CphxGUID> TextureIDMap;

  void UpdateObjectParamUI();
  void UpdateObjectFilterUI();

public:

  CapexModelParameters();
  CapexModelParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelParameters();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelParameters; }
  virtual void UpdateData();

  virtual void ReloadLayout();
};
