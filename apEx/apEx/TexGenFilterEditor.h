#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/apxPhoenix.h"

class CapexTexGenFilterEditor : public CapexWindow
{
  CphxTextureFilter_Tool *EditedFilter;

  TBOOL MessageProc( CWBMessage &Message );
  void RebuildParameterList();
  void CompileShader();
  void MinimizeShader();
  void TestHLSLLexer();

public:

  CapexTexGenFilterEditor();
  CapexTexGenFilterEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTexGenFilterEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_TexGenFilterEditor; }

  void SetEditedFilter( CphxTextureFilter_Tool *f );
  void SetEditedParam( TS32 Index );
  void AddFilter( CphxTextureFilter_Tool *f );
  virtual void UpdateData();
};
