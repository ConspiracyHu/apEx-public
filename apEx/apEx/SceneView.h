#pragma once
#include "apExWindow.h"
#include "ModelView.h"

class CWBSceneDisplay : public CWBItem
{
  TBOOL MessageProc( CWBMessage &Message );

  virtual void OnDraw( CWBDrawAPI *API );

  CphxRenderTarget_Tool *GetDisplayedRenderTarget();

  MODELVIEWCAMERAMODE Mode;

  CDictionary<CphxGUID, CAMERADATA> CameraData;
  TS32 DragStartButton;

  TF32 OrthoZoom;
  UBERTOOLTYPE SelectedUberTool;

  CAMERADRAGMODE DragMode;

  TBOOL GridView;
  TBOOL HelperView = true;
  TBOOL SolidView = false;

  void InitCameraGrid();
  void DrawOrthoGrid( CWBDrawAPI *API );

  class CphxRenderDataInstance *CameraGrid;

  CUberTool *UberTool;

  TBOOL UberToolVisible();
  void ApplyUberToolValues();
  void CalculateMatrices();
  void DisplayUberTool( CWBDrawAPI *API );
  CphxObject_Tool *Pick( CRect ClientRect );

  CphxObject_Tool *MouseObject;
  CphxMesh *CameraMesh;
  CphxMesh *CrossMesh;
  CphxMesh *SphereMesh;
  CphxMesh *CubeMesh;
  CphxMesh *CylinderMesh;
  CphxMesh *ConeMesh;
  CphxMesh *LineMesh;
  void BuildDummyMeshes();
  CphxRenderDataInstance CameraInstance;
  CphxRenderDataInstance CrossInstance;
  CphxRenderDataInstance SphereInstance;
  CphxRenderDataInstance CubeInstance;
  CphxRenderDataInstance CylinderInstance;
  CphxRenderDataInstance ConeInstance;
  CphxRenderDataInstance LineInstance;

  CDictionary<CphxGUID, CphxGUID> SceneCameraMap;

  void FindViableCamera();
  void DrawGrid( CWBDrawAPI *API, CphxScene_Tool *s );
  void UpdateSceneContent( CphxScene_Tool *s );
  void InjectHelperObjects( CphxScene_Tool *s );
  void InjectWireframe( CphxScene_Tool *s );
  void InjectLine( CphxScene_Tool *s, TS32 LayerIDX, CVector3 src, CVector3 trg, float r, float g, float b, float a );
  void InjectTargetLine( CphxScene_Tool *s, TS32 LayerIDX, CphxObject_Tool *Source, CphxObject_Tool *Target, float r, float g, float b, float a );

  CAMERADATA *GetCamData();
  D3DXMATRIX GetHelperTransformationMatrix( D3DXVECTOR3 zd );

public:

  static bool ParticlesCalculatedInFrame;

  CapexRenderTargetCollection *RenderTargets;

  TBOOL ObjectSpaceTransform;
  TBOOL CenterTransform;
  CphxGUID RenderTarget;

  CWBSceneDisplay();
  CWBSceneDisplay( CWBItem *Parent, const CRect &Pos );

  virtual ~CWBSceneDisplay();

  static CWBItem * Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "scenedisplay" ), CWBItem );

  void SetCameraMode( MODELVIEWCAMERAMODE m, CphxObject_Tool *Camera = NULL );
  MODELVIEWCAMERAMODE GetMode() { return Mode; }

  void SelectUberTool( UBERTOOLTYPE Type );

  void SetGridView( TBOOL g ) { GridView = g; }
  void SetHelperView( TBOOL g ) { HelperView = g; }
  void SetSolidView( TBOOL g ) { SolidView = g; }
  void SetEditedObject( CphxObject_Tool *obj );

  CphxObject_Tool *GetEditedObject();
  void UpdateRendertargetCollection();

  void FlushCamData() { CameraData.Flush(); }
  void SetRenderTarget( CphxGUID guid );

  void SetCameraData( CphxGUID guid, CAMERADATA c );

};

class CapexSceneView : public CapexWindow
{

  TBOOL MessageProc( CWBMessage &Message );

  TBOOL Maximized;
  CRect MaximizedPosition, NormalPosition;
  TBOOL JustBeenMaximized;

public:

  CapexSceneView();
  CapexSceneView( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexSceneView();
  virtual APEXWINDOW GetWindowType() { return apEx_SceneView; }
  virtual void UpdateData();

  void ExportWindow( CXMLNode *node );
  virtual void ImportConfig( CXMLNode *node, CRect &Pos );
  void SetCameraMode( MODELVIEWCAMERAMODE m, CphxObject_Tool *Camera = NULL );
  void SelectUberTool( UBERTOOLTYPE Type );
  void SwitchObjSpaceMode();
  void SwitchWorldSpaceMode();

  void SetUbertoolSnap();
  void SetMaximizedPosition( CRect r ) { MaximizedPosition = r; }
};
