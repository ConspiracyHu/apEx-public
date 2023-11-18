#pragma once
#include "apExWindow.h"
#include "WorkBench.h"
#include "UberTool.h"

enum MODELVIEWCAMERAMODE
{
  CAMERA_LEFT = 20,
  CAMERA_RIGHT,
  CAMERA_TOP,
  CAMERA_BOTTOM,
  CAMERA_FRONT,
  CAMERA_BACK,
  CAMERA_NORMAL,
  CAMERA_CAMERAVIEW,
};

enum CAMERADRAGMODE
{
  CAMDRAGMODE_NONE,
  CAMDRAGMODE_ROTATE,
  CAMDRAGMODE_PAN,
  CAMDRAGMODE_UBERTOOL,
};

enum UBERTOOLTYPE
{
  UBERTOOL_MOVE,
  UBERTOOL_ROTATE,
  UBERTOOL_SCALE
};

struct CAMERADATA
{
  D3DXVECTOR3 Eye, Target, Up, OrthoX, OrthoY, OrthoZ;
  D3DXVECTOR3 _Eye, _Target, _Up;
};

class CWBModelDisplay : public CWBItem
{
  TBOOL MessageProc( CWBMessage &Message );

  virtual void OnDraw( CWBDrawAPI *API );

  MODELVIEWCAMERAMODE Mode;

  //D3DXVECTOR3 Eye, Target, Up, OrthoX, OrthoY, OrthoZ;
  //D3DXVECTOR3 _Eye, _Target, _Up;

  CphxRenderTarget_Tool *GetDisplayedRenderTarget();

  CDictionary<CphxGUID, CAMERADATA> CameraData;

  TS32 DragStartButton;

  TF32 OrthoZoom;
  UBERTOOLTYPE SelectedUberTool;

  CAMERADRAGMODE DragMode;

  TBOOL GridView;
  TBOOL TintView;
  TBOOL SolidView = false;

  void InitCameraGrid();
  void DrawOrthoGrid( CWBDrawAPI *API );

  class CphxRenderDataInstance *CameraGrid;

  CUberTool *UberTool;

  TBOOL UberToolVisible();
  void ApplyUberToolValues();
  void CalculateMatrices();

  CphxModelObject_Tool *GetEditedModelObject();
  CphxModelObject_Tool *Pick();

  CphxModelObject_Tool *MouseObject;
    
  void UpdateUberToolColor();
  void DisplayUberTool( CWBDrawAPI *API );

  CAMERADATA *GetCamData();

public:

  CapexRenderTargetCollection *RenderTargets;

  TBOOL ObjectSpaceTransform;
  TBOOL CenterTransform;
  CphxGUID RenderTarget;

  CWBModelDisplay();
  CWBModelDisplay( CWBItem *Parent, const CRect &Pos );

  virtual ~CWBModelDisplay();

  static CWBItem * Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

  WB_DECLARE_GUIITEM( _T( "modeldisplay" ), CWBItem );

  void SetCameraMode( MODELVIEWCAMERAMODE m );
  MODELVIEWCAMERAMODE GetMode() { return Mode; }

  void SelectUberTool( UBERTOOLTYPE Type );

  void SetGridView( TBOOL g ) { GridView = g; }
  void SetSolidView( TBOOL g ) { SolidView = g; }
  void SetTintView( TBOOL g ) { TintView = g; }

  void UpdateRendertargetCollection();
  void FlushCamData() { CameraData.Flush(); }
  void SetRenderTarget( CphxGUID guid );
};

class CapexModelView : public CapexWindow
{
  TBOOL MessageProc( CWBMessage &Message );

  TBOOL Maximized;
  CRect MaximizedPosition, NormalPosition;
  TBOOL JustBeenMaximized;

public:

  CapexModelView();
  CapexModelView( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexModelView();
  virtual APEXWINDOW GetWindowType() { return apEx_ModelView; }
  virtual void UpdateData();

  void ExportWindow( CXMLNode *node );
  virtual void ImportConfig( CXMLNode *node, CRect &Pos );
  void SetCameraMode( MODELVIEWCAMERAMODE m );
  void SelectUberTool( UBERTOOLTYPE Type );
  void SwitchObjSpaceMode();
  void SwitchWorldSpaceMode();

  void SetUbertoolSnap();
  void SetMaximizedPosition( CRect r ) { MaximizedPosition = r; }
};

void InitializeModelView( CCoreDevice *Device );
void DeinitializeModelView();
