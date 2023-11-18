#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/Material_Tool.h"
#include "../Phoenix_Tool/Model_Tool.h"
class CphxObject_Tool;

class CapexSceneObjectParameters : public CapexWindow
{
  CDictionary<CphxGUID, CWBItem *> ParamUIMap;
  CDictionary<TS32, CphxGUID> TextureIDMap;

  CStyleManager StyleManager;
  void LoadCSS();

  CphxObject_Tool *EditedObj;
  TBOOL MessageProc( CWBMessage &Message );
  void BuildObjSelectorContextMenu( CWBItem *Button );
  void AddMaterialParameterUI( CphxMaterialDataStorage_Tool *matdat, CWBItem *Root, CphxMaterialParameter_Tool *Param );
  void BuildTextureSelectorUI( CWBItem *Button );
  CphxMaterialParameter_Tool *GetTargetParam( CWBItem *i );

  CDictionary<WBGUID, CphxGUID> TextureLocatorMap;

public:

  CapexSceneObjectParameters();
  CapexSceneObjectParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexSceneObjectParameters();
  virtual APEXWINDOW GetWindowType() { return apEx_SceneObjectParameters; }
  virtual void UpdateData();

  void SelectSceneObject( CphxObject_Tool *Obj );
  virtual void ReloadLayout();

  CphxObject_Tool *GetEditedObj() { return EditedObj; }

};
