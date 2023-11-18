#pragma once
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"

class CapexWorkBench;

enum APEXWINDOW
{
  apEx_MaterialEditor = 1,
  apEx_MaterialList,
  apEx_MaterialShaderEditor,
  apEx_ModelGraph,
  apEx_ModelList,
  apEx_ModelMaterial,
  apEx_ModelParameters,
  apEx_ModelPrimitives,
  apEx_ModelView,
  apEx_ProjectSettings,
  apEx_SceneClips,
  apEx_SceneGraph,
  apEx_SceneList,
  apEx_SceneObjectParameters,
  apEx_ScenePrimitives,
  apEx_SceneSplineEditor,
  apEx_SceneView,
  apEx_TexGenFilterEditor,
  apEx_TexGenMain,
  apEx_TexGenPages,
  apEx_TextureOpParameters,
  apEx_TexGenPreview,
  apEx_TexGenNewOpWindow,
  apEx_TimelineEditor,
  apEx_TimelineEventParameters,
  apEx_TimelineEventSplines,
  apEx_TimelinePreview,
  apEx_TreeSpeciesEditor,
  apEx_Help,
  apEx_Console,
  apEx_XMLEditor,
  apEx_CSSEditor,
  apEx_RenderTargetEditor,
  apEx_VideoDumper,
  apEx_ModelMatrix,
  apEx_KKPViewer,
};

class CapexWindow : public CWBWindow
{
  CString XMLName;

  CStyleManager windowStyleManager;
  void LoadStyle();
  void ApplyStyle();

protected:

  CapexWorkBench *WorkBench;

public:

  virtual void OnDraw( CWBDrawAPI *API );

  CapexWindow();
  CapexWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench, const TCHAR *txt = _T( "" ), const TCHAR *XMLName = _T( "" ), TU32 style = WB_WINDOW_DEFAULT );
  virtual ~CapexWindow();

  virtual APEXWINDOW GetWindowType() = 0;
  virtual TBOOL MessageProc( CWBMessage &Message );
  CString &GetXMLName() { return XMLName; }

  virtual void ReloadLayout();
  virtual void UpdateData() = 0;

  WB_DECLARE_GUIITEM( _T( "apexwindow" ), CWBWindow );


  virtual void ExportWindow( CXMLNode *node );
  virtual void ImportConfig( CXMLNode *node, CRect &Pos ) {};
  virtual void ResetUIData() {};

  void WindowHasDocked();
  virtual void OnPostDraw( CWBDrawAPI*API );


  CapexWorkBench* GetWorkBench();
};

void UpdateWindowData( APEXWINDOW w );
void UpdateWindowData();
void SetStatusbarText( CString Text );
void SetStatusbarError( CString Text );