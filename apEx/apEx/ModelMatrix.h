#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Model_Tool.h"

class CapexModelMatrix : public CapexWindow
{
  CStyleManager StyleManager;
  void LoadCSS();

  TBOOL MessageProc( CWBMessage &Message );

  CDictionary<TS32, CphxGUID> TextureIDMap;

  void SetMatrix( const D3DXMATRIX& matrix );
  void SetSRT( D3DXFLOAT16 srt[ 12 ] );
  void GetSRT( D3DXFLOAT16 srt[ 10 ] );
  void ClearValues();
  D3DXMATRIX GetMatrixFromSRT();

public:

  CapexModelMatrix();
  CapexModelMatrix( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelMatrix();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelMatrix; }
  virtual void UpdateData();

  virtual void ReloadLayout();
};
