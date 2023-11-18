#pragma once
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"
#include "WorkBench.h"
#include "Console.h"
#include "Config.h"

class CapexTooltip :public CWBItem
{

  virtual TBOOL MessageProc( CWBMessage &Message );
  void OnDraw( CWBDrawAPI *API );
  void WriteText( CWBDrawAPI *API, CWBFont *font, CString &Text, CPoint Position, CColor Color );

  CString Tooltip;

public:
  TS32 FadeoutStart; //when to start fading the splash screen

  WB_DECLARE_GUIITEM( _T( "apextooltip" ), CWBItem );

  CapexTooltip( CWBItem *Parent, const CRect &Position );
  virtual ~CapexTooltip();

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

  void SetTooltip( CString Text ) { Tooltip = Text; }

};

typedef void( __cdecl *APEXCONSOLECOMMAND )( CStringArray &Parameters );

struct APEXCONSOLECOMMANDDESC
{
  APEXCONSOLECOMMAND Command;
  CString Description;
};

class CapexConsoleInputLine : public CWBTextBox
{

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CString TabStringRoot;

  static CWBItem * Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

  WB_DECLARE_GUIITEM( _T( "consoleinput" ), CWBTextBox );

  CapexConsoleInputLine( CWBItem *Parent, const CRect &Position );
  virtual ~CapexConsoleInputLine();
};

class CapexVideoDumper;

class CapexRoot : public CWBItem
{
  TBOOL MessageProc( CWBMessage &Message );

  //CWBBox *TaskBar;
 // CWBBox *WorkbenchButtonBox;
  //CWBBox *StatusBar;

  CapexTooltip *Tooltip;

  CArray<CapexWorkBench*> WorkBenches;
  CArray<CWBButton*> WorkBenchButtons;
  //CapexConsole *Console;
  //CWBItem *ConsoleBackground;
  //CapexConsoleInputLine *ConsoleInput;

  //CWBButton *FileMenuButton;
  //CWBButton *ViewMenuButton;
  //CWBButton *HelpMenuButton = NULL;
  CWBButton *NewWorkbenchButton = NULL;

  CWBTextBox *WorkbenchRenamer;

  CWBButton *AddTaskbarButton( const TCHAR *Name );
  //CWBButton *AddMenuButton( const TCHAR *Name );
  void SelectWorkBench( CWBButton *WBSelector );

  WBGUID FileMenuID;
  WBGUID ViewMenuID;
  WBGUID HelpMenuID;

  virtual void OnDraw( CWBDrawAPI *API );
  void OpenFileMenu();
  void OpenViewMenu();
  void OpenHelpMenu();

  void OpenProject();
  void SaveProject();
  void ImportMaterial();
  void MergeProject();
  void ImportModel( bool singleModel, bool miniModel );
  void ExportMinimalFile( bool exportHeader );

  void ToggleFullscreenPreview();

  CWBItem *FullscreenPreview;
  CapexWorkBench *CurrentWorkbench;
  TBOOL FullscreenMouseSeek;

  void FSMouseSeek();

  CDictionary<CString, APEXCONSOLECOMMANDDESC> ConsoleCommands;
  CArray<CString> CommandLog;
  TS32 CommandLogPos;

  CapexVideoDumper *VideoDumper = NULL;

  void MoveInCommandLog( TS32 step );
  void HandleConsoleTab();
  void CompileRelease( TS32 Type );
  void ToggleConsole( TBOOL Show );

  CString SourceRootPath;
  CString CompilerPath;

  void UpdateColorPicker();

  CXMLDocument defaultLayouts;

  CphxGUID SearchedTexture;
  CphxGUID SearchedModel;
  CphxGUID SearchedScene;

public:

  CapexRoot( CWBItem *Parent, const CRect &Position );
  virtual ~CapexRoot();

  WB_DECLARE_GUIITEM( _T( "apexroot" ), CWBItem );

  CapexWorkBench *AddWorkBench( const TCHAR *Name );

  void LoadDefaultLayout();

  void ExecuteCommand( CStringArray &Commandline );
  void UpdateWindowData( APEXWINDOW WindowType );
  void UpdateWindowData();
  void ClearProjectData();

  TS32 GetWorkBenchCount() { return WorkBenches.NumItems(); }
  CapexWorkBench *GetWorkBench( TS32 x ) { return WorkBenches[ x ]; }
  TS32 GetSelectedWorkBench();

  void SelectWorkBench( CapexWorkBench *WB );
  //void ApplyStatusbarStyle(CString sbs);
  void SetStatusbarText( CString Text );
  void SetStatusbarError( CString Text );
  void SetTooltip( CString Text ) { Tooltip->SetTooltip( Text ); }

  void SetUbertoolSnap( TF32 snap );
  TF32 GetUbertoolSnap() { return Config::UbertoolSnap; }

  void SelectFirstTextureObjectScene();

  void RegisterConsoleCommand( TCHAR *Name, TCHAR *Description, APEXCONSOLECOMMAND Callback );
  void ListCommands();

  void ToggleVideoDumper();
  TCHAR *GetTargetDirectory( CString key );
  void StoreCurrentDirectory( CString key );

  void GoToTexture( CphxTextureOperator_Tool *Save, bool enableContextMenu );
  void GoToModel( CphxModel_Tool *model, bool enableContextMenu );
  void GoToScene( CphxScene_Tool*scene, bool enableContextMenu );

  CCorePixelShader *GammaDisplayShader = NULL;

  void ImportKasparov();

  CString GetCSSPath();

  TBOOL TrySetSolutionRoot( const CString & Text );
};

void SetStatusbarText( CString Text );
void SetStatusbarError( CString Text );
void SetTooltip( CString Text );

extern CapexRoot *Root;
