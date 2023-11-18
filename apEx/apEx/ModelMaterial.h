#pragma once
#include "apExWindow.h"
#include "ModelParameters.h"

class CphxMaterialParameter_Tool;

class CapexModelMaterial : public CapexWindow
{
  CStyleManager StyleManager;
  void LoadCSS();

  virtual TBOOL MessageProc( CWBMessage &Message );

  virtual void ReloadLayout();
  virtual void AddParameterUI( CWBItem *Root, CphxMaterialParameter_Tool *Param );

  CDictionary<CphxGUID, CWBItem *> ParamUIMap;
  CDictionary<TS32, CphxGUID> TextureIDMap;
  CphxMaterialParameter_Tool *GetTargetParam( CWBItem *i );
  void BuildTextureSelectorUI( CWBItem *Button );

  CDictionary<WBGUID, CphxGUID> TextureLocatorMap;

public:

  CapexModelMaterial();
  CapexModelMaterial( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelMaterial();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelMaterial; }
  virtual void UpdateData();
};
