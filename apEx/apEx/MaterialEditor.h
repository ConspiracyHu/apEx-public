#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Material_Tool.h"

class CapexMaterialEditor : public CapexWindow
{
  CStyleManager StyleManager;
  void LoadCSS();

  virtual TBOOL MessageProc( CWBMessage &Message );

  void SelectTech();
  void SelectPass();

  void SelectTechParam();
  void SelectPassParam();

  void SelectMaterial();
  void CopyTech( CphxMaterialTechnique_Tool *Tech );
  void ExportTech( CphxMaterialTechnique_Tool *Tech );
  void ExportMaterial( CphxMaterial_Tool *Tech );
  void BuildParamDefaultValueGUI( CWBItem *Root, CphxMaterialParameter_Tool *Param );
  virtual void ReloadLayout();

  void BuildTextureSelectorUI( CWBItem *Button, TBOOL RTOnly = false );
  CDictionary<TS32, CphxGUID> TextureIDMap;

public:

  CapexMaterialEditor();
  CapexMaterialEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexMaterialEditor();

  virtual APEXWINDOW GetWindowType() { return apEx_MaterialEditor; }
  virtual void UpdateData();

  CphxMaterialTechnique_Tool *GetEditedTech();
  CphxMaterialRenderPass_Tool *GetEditedPass();

  CphxMaterialParameter_Tool *GetEditedTechParam();
  CphxMaterialParameter_Tool *GetEditedPassParam();

  CphxMaterial_Tool *GetEditedMaterial();
};
