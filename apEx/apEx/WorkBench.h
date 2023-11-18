#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/apxProject.h"
#include "../Phoenix_Tool/Scene_tool.h"
#include "Docker.h"

struct WindowDockDisplayInfo
{
  CPoint displayCenter;
  DockPosition dockTarget;
  CWBItem* windowToDock = nullptr;
  CapexDocker* targetDockItem = nullptr;
  CapexWindow* targetWindowItem = nullptr;
  TBOOL isSet = true;
  CRect highlightArea;

  WindowDockDisplayInfo() = default;

  WindowDockDisplayInfo( CPoint displayCenter, CRect highlightArea, DockPosition dockTarget, CWBItem* windowToDock, CapexDocker* targetDockItem, CapexWindow* targetWindowItem )
    : displayCenter( displayCenter ),
    highlightArea( highlightArea ),
    dockTarget( dockTarget ),
    windowToDock( windowToDock ),
    targetDockItem( targetDockItem ),
    targetWindowItem( targetWindowItem )
  {
  }

  bool operator==( WindowDockDisplayInfo& r )
  {
    return displayCenter == r.displayCenter &&
      dockTarget == r.dockTarget &&
      windowToDock == r.windowToDock &&
      targetDockItem == r.targetDockItem &&
      targetWindowItem == r.targetWindowItem &&
      highlightArea == r.highlightArea &&
      isSet == r.isSet;
  }
};

class CapexWorkBench : public CWBItem
{
  friend class CapexWindow;

  TBOOL MessageProc( CWBMessage &Message );

  CString Name;
  TS32 SnapDistance;

  APEXPAGEID EditedTexturePage;

  CArray<CapexWindow*> Windows;

  void SnapRect( CRect &Original, CRect Reference, TBOOL Move, TBOOL Sides[ 4 ] );
  TBOOL IsUniqueWindow( APEXWINDOW w );
  void WindowBeingDeleted( CapexWindow *w );
  virtual void OnDraw( CWBDrawAPI *API );
  virtual void OnPostDraw( CWBDrawAPI* API );

  CphxObject_Model *PreviewObject;
  CphxObject *PreviewLight;
  CphxScene_Tool *ModelPreviewScene;
  CphxModel_Tool *EditedModel;
  CphxScene_Tool *EditedScene;
  CphxModelObject_Tool *EditedObj;
  CphxMeshFilter_Tool *EditedFilter;

  CapexDocker *DockRoot = nullptr;

  void DockWindow( CWBItem* item, DockPosition dockPosition );
  void UndockWindow( CapexWindow* wnd );

public:

  class CapexTexGenPreview *lastTexgenPreview = nullptr;

  CArray<WindowDockDisplayInfo> dockerItems;
  WindowDockDisplayInfo activeItem;

  CapexWorkBench( CWBItem *Parent, const CRect &Position, const TCHAR *Name );
  virtual ~CapexWorkBench();

  WB_DECLARE_GUIITEM( _T( "workbench" ), CWBItem );

  CapexWindow *OpenWindow( APEXWINDOW w );
  CapexWindow *OpenWindow( CXMLNode *w );
  CapexWindow *GetWindow( APEXWINDOW w, TS32 id = 0 );
  TS32 GetWindowCount() { return Windows.NumItems(); }
  CapexWindow *GetWindowByIndex( TS32 x ) { return Windows[ x ]; }

  TS32 GetWindowCount( APEXWINDOW w );
  CapexWindow *GetWindowByIndex( APEXWINDOW w, TS32 x );

  void SetEditedPage( APEXPAGEID ID );
  APEXPAGEID GetEditedPage() { return EditedTexturePage; }

  void ExportLayout( CXMLNode *node );
  void ExportDocker( CXMLNode *node, CapexDocker* docker );
  void ImportLayout( CXMLNode *node, bool resize = false );
  void ImportDocker( CXMLNode *node, CapexDocker* docker );

  void SetEditedModel( CphxModel_Tool *Model );
  CphxModel_Tool *GetEditedModel();
  CphxScene_Tool *GetModelPreviewScene();
  CphxModelObject_Tool *GetEditedModelObject();
  CphxMeshFilter_Tool *GetEditedModelFilter();

  void ResetUIData();

  void SetEditedScene( CphxScene_Tool *Model );
  CphxScene_Tool *GetEditedScene();

  void SetEditedModelObject( CphxModelObject_Tool *EditedObj );
  void SetEditedModelFilter( CphxMeshFilter_Tool *EditedFilter );
  PHXMESHFILTER GetEditedModelFilterType();

  void UpdateWindows( APEXWINDOW w );
  void UpdateWindows();

  void GoToTexture( CphxTextureOperator_Tool *Save );

  void SetName( CString &name ) { Name = name; }
  CString GetName() { return Name; }

  void SetLastTexgenPreview( CapexTexGenPreview* preview );

  //void OpenGotoTextureMenu( CphxGUID texture );
  //void OpenGotoModelMenu( CphxGUID model );
  //void OpenGotoSceneMenu( CphxGUID scene );
};

CapexWorkBench *GetActiveWorkBench();