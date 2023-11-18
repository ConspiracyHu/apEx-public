#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Material_Tool.h"

class CapexMaterialShaderEditor : public CapexWindow
{

  TBOOL MessageProc( CWBMessage &Message );
  CphxMaterialRenderPass_Tool *GetEditedPass();

public:

  CapexMaterialShaderEditor();
  CapexMaterialShaderEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexMaterialShaderEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_MaterialShaderEditor; }
  virtual void UpdateData();
  void CompileEditedShader();
  void MinimizeShader();
  void TestHLSLLexer();
};
