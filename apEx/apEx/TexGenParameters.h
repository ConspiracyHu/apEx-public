#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Texgen_tool.h"

class CapexTextureOpParameters : public CapexWindow
{
  CStyleManager StyleManager;
  TBOOL VisibleSplines[ 4 ];

  //CphxTextureOperator_Tool *EditedOperator;
  APEXOPID EditedOp;
  CArray<CWBItem*> GeneratedParamRoots;

  void FlushParameters();
  void AddParameter( TS32 ParamID );
  virtual TBOOL MessageProc( CWBMessage &Message );
  void LoadCSS();
  void OpenImageImport();

  TBOOL HandleByteParamChange( TU8 Data, CWBTrackBar* v, unsigned char ParamArray[ TEXGEN_MAX_PARAMS ], TS32 s, TS32 ParamType, CString &Name );

public:

  CapexTextureOpParameters();
  CapexTextureOpParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTextureOpParameters();
  virtual APEXWINDOW GetWindowType() { return apEx_TextureOpParameters; }

  virtual void UpdateData();

  void SetEditedOperator( CphxTextureOperator_Tool *op );
  virtual void ReloadLayout();
  void BackupSplineUiStateTo( CphxTextureOperator_Tool* op );
};
