#include "BasePCH.h"
#include "apExRoot.h"
#include "BuildInfo.h"
#include "Console.h"
#include "TimelineEditor.h"
#include "TimelinePreview.h"
#include "ModelView.h"
#include "SceneView.h"
#include "MinimalExport.h"
#include "VideoDumper.h"
#include <ShellAPI.h>
#include "Config.h"

extern CArray<CphxResource*> UpdateQueue;

enum APEXFILEMENU
{
  apEx_FileNew = 1,
  apEx_FileOpen,
  apEx_FileSave,
  apEx_FileSaveAs,
  apEx_ExportMinimal,
  apEx_ExportMinimalWithHeader,
  apEx_ExportTextureFilters,
  apEx_ExportMaterialLibrary,
  apEx_ExportTreeDescriptionLibrary,
  apEx_Exit,
  apEx_VideoDump,
  apEx_MergeProject,
  apEx_ImportMaterial,
  apEx_ImportModel,
  apEx_ImportSingleModel,
  apEx_ImportSingleMiniModel,
  apEx_MakeExe_64k_CNS,
  apEx_MakeExe_Demo_CNS,
  apEx_MakeExe_64k_External,
  apEx_MakeExe_Demo_External,
  apEx_Help_UsersGuide,
  apEx_Help_Overview,
  apEx_Help_UserInterface,
  apEx_Help_TextureGeneration,
  apEx_Help_Modelling,
  apEx_Help_Animation,
  apEx_Help_VideoEditing,
  apEx_Help_CreateTextureFilter,
  apEx_Help_CreateMaterial,
  apEx_Help_CreateTimelineEffect,
  apEx_Help_AdditionalFeatures,
  apEx_Help_CreatingARelease,
  apEx_Help_LastResort,

  apEx_SelectCSS = 0x1337,

  apEx_AddWBLayout = 0x25472,

  apEx_GoToTextureMenu = 0x31337,
  apEx_GoToModelMenu = 0x31337 + 0x1000,
  apEx_GoToSceneMenu = 0x31337 + 0x2000,
};

void CapexRoot::OnDraw( CWBDrawAPI *API )
{
  CWBSceneDisplay::ParticlesCalculatedInFrame = false;

  CWBLabel *l = (CWBLabel*)FindChildByID( "buildid", "label" );

  if ( l )
    l->SetText( apexBuild );

  l = (CWBLabel*)FindChildByID( "generatorstatus", "label" );
  if ( l )
  {
    if ( UpdateQueue.NumItems() )
      l->SetText( CString::Format( _T( "Stuff is being generated. %d items remain in the queue." ), UpdateQueue.NumItems() ) );
    else
      l->SetText( _T( "The Generator is Idle" ) );
  }

  l = (CWBLabel*)FindChildByID( "fpscounter", "label" );
  if ( l )
    l->SetText( CString::Format( _T( "%d FPS" ), (TS32)App->GetFrameRate() ) );
}

CapexRoot::CapexRoot( CWBItem *Parent, const CRect &Position ) : CWBItem( Parent, Position )
{
  {
    CString s = _T( "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/devenv.com" );
    CStreamReaderFile compiler;
    if ( compiler.Open( s.GetPointer() ) )
    {
      CompilerPath = s;
      LOG_NFO( "[apEx] Compiler located at %s", CompilerPath.GetPointer() );
    }
    else
    {
      CString s = _T("C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\Common7\\IDE\\devenv.com");
      CStreamReaderFile compiler;
      if (compiler.Open(s.GetPointer()))
      {
        CompilerPath = s;
        LOG_NFO("[apEx] Compiler located at %s", CompilerPath.GetPointer());
      }
    }
  }

  TrySetSolutionRoot( _T( "..\\" ) );

  CommandLogPos = 0;
  ///////////////////////////////////////////////////////////////////////////////////

  CString xmlname = CString( _T( "Data/UI/apEx" ) );
  App->LoadXMLLayoutFromFile( xmlname + ".xml" );
  App->LoadCSSFromFile( GetCSSPath() + _T( "apEx.css" ) );
  App->GenerateGUI( this, "apEx" );

  ToggleConsole( false );

  ///////////////////////////////////////////////////////////////////////////////////
  CRect r = GetParent()->GetClientRect();
  SetPosition( r );

  TS32 yp1 = (TS32)( r.y2*0.35 );
  TS32 yp2 = yp1 + App->GetDefaultFont()->GetLineHeight() + 3;

  NewWorkbenchButton = AddTaskbarButton( "+" );

  Tooltip = new CapexTooltip( this, r );

  WorkbenchRenamer = NULL;

  FullscreenPreview = NULL;
  CurrentWorkbench = NULL;
  FullscreenMouseSeek = false;
  FileMenuID = -1;
  ViewMenuID = -1;
  HelpMenuID = -1;

#ifdef PHOENIX_ENGINE_LINEAR_RENDER
  LPCSTR shader =
    "Texture2D GuiTexture:register(t0);"
    "SamplerState Sampler:register(s0);"
    "cbuffer resdata : register(b0)"
    "{							   "
    "		float4 resolution;	   "
    "}"
    "float gamma(float c)	{		float cs = saturate(c);		if (cs < 0.0031308) cs = 12.92*cs;		else cs = 1.055*pow(cs, 1 / 2.4) - 0.055;		return cs;	} "
    "float4 gamma(float4 g)	{		return float4(gamma(g.x), gamma(g.y), gamma(g.z), g.w);	}"
    "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
    "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
    "VSOUT vsmain(VSIN x) { VSOUT k; k.Position=float4(x.Position.x/resolution.x*2-1,-x.Position.y/resolution.y*2+1,0,1); k.UV=x.UV; k.Color=x.Color; return k; }"
    "float4 psmain(VSOUT x) : SV_TARGET0 { float4 res=gamma(x.Color)*gamma(GuiTexture.Sample(Sampler,x.UV)); if (resolution.z==0) return res; else return max(0.1,min(0.9,res)); }";
#else
  LPCSTR shader =
    "Texture2D GuiTexture:register(t0);"
    "SamplerState Sampler:register(s0);"
    "cbuffer resdata : register(b0)"
    "{							   "
    "		float4 resolution;	   "
    "}"
    "float gamma(float c)	{		float cs = saturate(c);		if (cs < 0.0031308) cs = 12.92*cs;		else cs = 1.055*pow(cs, 1 / 2.4) - 0.055;		return cs;	} "
    "float4 gamma(float4 g)	{		return float4(gamma(g.x), gamma(g.y), gamma(g.z), g.w);	}"
    "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
    "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
    "VSOUT vsmain(VSIN x) { VSOUT k; k.Position=float4(x.Position.x/resolution.x*2-1,-x.Position.y/resolution.y*2+1,0,1); k.UV=x.UV; k.Color=x.Color; return k; }"
    "float4 psmain(VSOUT x) : SV_TARGET0 { return x.Color*GuiTexture.Sample(Sampler,x.UV); }";
#endif

  GammaDisplayShader = App->GetDevice()->CreatePixelShader( shader, strlen( shader ), "psmain", "ps_5_0" );
  if ( !GammaDisplayShader )
    LOG( LOG_ERROR, _T( "[apex] Couldn't compile gamma display shader" ) );

  if ( !defaultLayouts.LoadFromFile( "Data/UI/DefaultLayouts.xml" ) )
    LOG_WARN( "[apEx] DefaultLayouts.xml not found" );

}

CapexRoot::~CapexRoot()
{
  SAFEDELETE( GammaDisplayShader );
}

void CapexRoot::SelectWorkBench( CWBButton *WBSelector )
{
  for ( TS32 y = 0; y < WorkBenchButtons.NumItems(); y++ )
    WorkBenchButtons[ y ]->Push( false );
  for ( TS32 y = 0; y < WorkBenches.NumItems(); y++ )
    WorkBenches[ y ]->Hide( true );

  WBSelector->Push( true );
  if ( WBSelector->GetData() )
    ( (CapexWorkBench*)WBSelector->GetData() )->Hide( false );

  CapexWorkBench *wb = (CapexWorkBench*)WBSelector->GetData();
  if ( wb )
    wb->UpdateWindows();

  Tooltip->SetTopmost();
}

void CapexRoot::SelectWorkBench( CapexWorkBench *WB )
{
  for ( TS32 y = 0; y < WorkBenchButtons.NumItems(); y++ )
    if ( WorkBenchButtons[ y ]->GetData() == WB )
      SelectWorkBench( WorkBenchButtons[ y ] );

  Tooltip->SetTopmost();
}

TBOOL CapexRoot::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_CONTEXTMESSAGE:
    if ( Message.GetTarget() == GetGuid() )
    {
      switch ( Message.Data )
      {
      case apEx_FileNew:
        if ( MessageBox( (HWND)App->GetHandle(), _T( "Are you sure you want to clear the project?" ), _T( "Please confirm" ), MB_OKCANCEL ) != IDCANCEL )
        {
          ClearProjectData();
          Project.Reset();
          Project.LoadedFileName = _T( "" );
          App->SetWindowTitle( CString( _T( "apEx IntroTool" ) ) );
          Project.ImportLibraries();
          UpdateWindowData();
        }
        break;
      case apEx_FileOpen:
        OpenProject();
        break;
      case apEx_ImportMaterial:
        ImportMaterial();
        break;
      case apEx_MergeProject:
        MergeProject();
        break;
      case apEx_ImportModel:
        ImportModel( false, false );
        break;
      case apEx_ImportSingleModel:
        ImportModel( true, false );
        break;
      case apEx_ImportSingleMiniModel:
        ImportModel( true, true );
        break;
      case apEx_FileSave:
        if ( !Project.LoadedFileName.Length() )
          SaveProject();
        else
        {
          App->SelectMouseCursor( CM_WAIT );
          App->FinalizeMouseCursor();
          Project.Export( Project.LoadedFileName, true, false );
        }
        break;
      case apEx_FileSaveAs:
        SaveProject();
        break;
      case apEx_ExportMinimal:
        ExportMinimalFile(false);
        break;
      case apEx_ExportMinimalWithHeader:
        ExportMinimalFile(true);
        break;
      case apEx_ExportTextureFilters:
      {
        CXMLDocument d;
        CXMLNode n = d.GetDocumentNode().AddChild( _T( "apExProject" ) );
        Project.ExportTextureFilters( &n );
        d.SaveToFile( _T( "Data\\FilterLibrary.xml" ) );
      }
      break;
      case apEx_ExportMaterialLibrary:
      {
        CXMLDocument d;
        CXMLNode n = d.GetDocumentNode().AddChild( _T( "apExProject" ) );
        Project.ExportMaterials( &n );
        d.SaveToFile( _T( "Data\\MaterialLibrary.xml" ) );
      }
      break;
      case apEx_ExportTreeDescriptionLibrary:
      {
        CXMLDocument d;
        CXMLNode n = d.GetDocumentNode().AddChild( _T( "apExProject" ) );
        Project.ExportTreeSpecies( &n );
        d.SaveToFile( _T( "Data\\Arboretum.xml" ) );
      }
      break;
      case apEx_Exit:
        App->SetDone( true );
        break;

      case apEx_VideoDump:
        ToggleVideoDumper();
        break;

      case apEx_MakeExe_64k_CNS:
      case apEx_MakeExe_Demo_CNS:
      case apEx_MakeExe_64k_External:
      case apEx_MakeExe_Demo_External:
        CompileRelease( Message.Data );
        break;

      case apEx_Help_UsersGuide:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/7q37zG" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_Overview:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/iNFsi0" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_UserInterface:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/kmWjlf" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_TextureGeneration:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/8E4kgA" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_Modelling:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/hToqBX" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_Animation:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/iUqTXk" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_VideoEditing:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/w9upjy" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_CreateTextureFilter:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/fb42XL" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_CreateMaterial:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/bzh7lh" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_CreateTimelineEffect:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/ugiD1t" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_AdditionalFeatures:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/qA5IJM" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_CreatingARelease:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "http://goo.gl/t8EtWb" ), NULL, NULL, SW_SHOW );
        break;
      case apEx_Help_LastResort:
        ShellExecute( (HWND)App->GetHandle(), _T( "open" ), _T( "mailto:boyc@conspiracy.hu" ), NULL, NULL, SW_SHOW );
        break;

      default:
        break;
      }

      if ( Message.Data >= 1000 && Message.Data < apEx_AddWBLayout )
      {
        if ( Config::RecentFiles.NumItems() > Message.Data - 1000 )
        {
          ClearProjectData();
          Project.Import( Config::RecentFiles[ Message.Data - 1000 ], (HWND)App->GetHandle() );

          UpdateWindowData();
          SelectFirstTextureObjectScene();

          CStringArray r = Project.LoadedFileName.Explode( _T( "\\" ) );
          if ( r.NumItems() )
            GetApplication()->SetWindowTitle( CString::Format( _T( "%s - apEx IntroTool" ), r.Last().GetPointer() ) );

          CString s = Config::RecentFiles[ Message.Data - 1000 ];
          Config::RecentFiles.Delete( s );
          Config::RecentFiles.Add( s );
        }
      }

      if ( Message.Data >= apEx_AddWBLayout && Message.Data < apEx_GoToTextureMenu )
      {
        auto doc = defaultLayouts.GetDocumentNode();
        if ( doc.GetChildCount( "Layouts" ) )
        {
          auto layouts = doc.GetChild( "Layouts" );
          if ( layouts.GetChildCount( "workbench" ) > Message.Data - apEx_AddWBLayout )
          {
            auto wb = layouts.GetChild( "workbench", Message.Data - apEx_AddWBLayout );

            if ( wb.HasAttribute( "Name" ) )
            {
              CapexWorkBench *workBench = AddWorkBench( wb.GetAttributeAsString( "Name" ).GetPointer() );
              SelectWorkBench( workBench );
              workBench->ImportLayout( &wb, true );
            }
          }
        }
      }

      if ( Message.Data >= apEx_GoToTextureMenu && Message.Data < apEx_GoToModelMenu )
      {
        auto* texture = Project.GetTexgenOp( SearchedTexture );
        if ( texture )
        {
          SelectWorkBench( WorkBenches[ Message.Data - apEx_GoToTextureMenu ] );
          WorkBenches[ Message.Data - apEx_GoToTextureMenu ]->GoToTexture( texture );
        }
      }

      if ( Message.Data >= apEx_GoToModelMenu && Message.Data < apEx_GoToSceneMenu )
      {
        auto* model = Project.GetModel( SearchedModel );
        if ( model )
        {
          SelectWorkBench( WorkBenches[ Message.Data - apEx_GoToModelMenu ] );
          WorkBenches[ Message.Data - apEx_GoToModelMenu ]->SetEditedModel( model );
          WorkBenches[ Message.Data - apEx_GoToModelMenu ]->UpdateWindows( apEx_ModelList );
          WorkBenches[ Message.Data - apEx_GoToModelMenu ]->UpdateWindows( apEx_ModelView );
        }
      }

      if ( Message.Data >= apEx_GoToSceneMenu )
      {
        auto* scene = Project.GetScene( SearchedScene );
        if ( scene )
        {
          SelectWorkBench( WorkBenches[ Message.Data - apEx_GoToSceneMenu ] );
          WorkBenches[ Message.Data - apEx_GoToSceneMenu ]->SetEditedScene( scene );
          WorkBenches[ Message.Data - apEx_GoToSceneMenu ]->UpdateWindows( apEx_SceneList );
        }
      }

      return true;
    }

    break;
  case WBM_ROOTRESIZE:
  {
    CRect r = GetParent()->GetClientRect();
    ApplyPosition( r );

    CWBMessage me;
    BuildPositionMessage( r, me );
    App->SendMessage( me );

    Tooltip->SetPosition( r );

    return true;
  }
  break;
  case WBM_COMMAND:
  {
    if ( Message.GetTarget() == NewWorkbenchButton->GetGuid() )
    {
      bool workbenchMenuOpen = false;

      auto doc = defaultLayouts.GetDocumentNode();
      if ( doc.GetChildCount( "Layouts" ) )
      {
        auto layouts = doc.GetChild( "Layouts" );
        if ( layouts.GetChildCount( "workbench" ) )
        {
          auto ctx = OpenContextMenu( App->GetMousePos() );
          workbenchMenuOpen = true;

          for ( int x = 0; x < layouts.GetChildCount( "workbench" ); x++ )
          {
            auto wb = layouts.GetChild( "workbench", x );
            if ( wb.HasAttribute( "Name" ) )
              ctx->AddItem( wb.GetAttributeAsString( "Name" ), x + apEx_AddWBLayout );
          }
        }
      }

      if ( !workbenchMenuOpen )
        SelectWorkBench( AddWorkBench( CString::Format( "Workbench #%d", WorkBenches.NumItems() + 1 ).GetPointer() ) );
      return true;
    }

    CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );

    if ( consoleInput && Message.GetTarget() == consoleInput->GetGuid() )
    {
      ExecuteCommand( consoleInput->GetText().ExplodeByWhiteSpace() );
      if ( consoleInput->GetText().Length() )
      {
        CommandLog += consoleInput->GetText();
        CommandLogPos = CommandLog.NumItems();
      }
      consoleInput->SetText( _T( "" ) );
      return true;
    }
    break;
  }
  case WBM_LEFTBUTTONDOWN:

    if ( App->GetMouseItem() && App->GetMouseItem()->GetID() == "filemenu" )
    {
      OpenFileMenu();
      return true;
    }

    if ( App->GetMouseItem() && App->GetMouseItem()->GetID() == "viewmenu" /*ViewMenuButton && App->GetMouseItem() == ViewMenuButton*/ )
    {
      OpenViewMenu();
      return true;
    }

    if ( App->GetMouseItem() && App->GetMouseItem()->GetID() == "helpmenu" /*HelpMenuButton && App->GetMouseItem() == HelpMenuButton*/ )
    {
      OpenHelpMenu();
      return true;
    }

    for ( TS32 x = 0; x < WorkBenchButtons.NumItems(); x++ )
      if ( App->GetMouseItem() == WorkBenchButtons[ x ] )
      {
        SelectWorkBench( WorkBenchButtons[ x ] );
        return true;
      }

    break;

  case WBM_RIGHTBUTTONDOWN:
    if ( !FullscreenPreview ) return false;
    FullscreenMouseSeek = true;
    break;

  case WBM_MOUSEMOVE:

    if ( App->GetShiftState() )
    {
      UpdateColorPicker();
    }

    if ( FullscreenMouseSeek )
    {
      FSMouseSeek();
      return true;
    }
    break;

  case WBM_RIGHTBUTTONUP:
    FullscreenMouseSeek = false;
    break;

  case WBM_KEYDOWN:

  {
    CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
    CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );

    if ( Message.Key == VK_RETURN && console && !console->IsHidden() && consoleInput )
    {
      consoleInput->SetText( _T( "" ) );
      consoleInput->SetFocus();
      return true;
    }

    if ( console && !console->IsHidden() && consoleInput && consoleInput->InFocus() )
    {
      if ( Message.Key == VK_UP )
      {
        MoveInCommandLog( -1 );
        return true;
      }
      if ( Message.Key == VK_DOWN )
      {
        MoveInCommandLog( 1 );
        return true;
      }
    }

    if ( Message.Key == 'S' && App->GetCtrlState() )
    {
      if ( Project.LoadedFileName.Length() )
      {
        App->SelectMouseCursor( CM_WAIT );
        App->FinalizeMouseCursor();
        Project.Export( Project.LoadedFileName, true, false );
      }
      else
        SaveProject();
      return true;
    }
    if ( Message.Key == 'O' && App->GetCtrlState() )
    {
      OpenProject();
      return true;
    }

    if ( Message.Key == VK_F2 )
    {
      for ( TS32 x = 0; x < WorkBenchButtons.NumItems(); x++ )
      {
        if ( WorkBenchButtons[ x ]->InFocus() )
        {
          CWBButton *w = WorkBenchButtons[ x ];
          SAFEDELETE( WorkbenchRenamer );
          WorkbenchRenamer = new CWBTextBox( w, w->GetWindowRect() );
          WorkbenchRenamer->AddClass( "WorkbenchRenamer" );
          App->ApplyStyle( WorkbenchRenamer );
          //WorkbenchRenamer->ApplyStyleDeclarations( _T( "padding:2px;padding-top:1px;padding-left:6px;" ) );
          WorkbenchRenamer->SetText( w->GetText() );
          WorkbenchRenamer->SetFocus();
          WorkbenchRenamer->EnableHScrollbar( false, false );
          WorkbenchRenamer->EnableVScrollbar( false, false );
          WorkbenchRenamer->SetSelection( 0, WorkbenchRenamer->GetText().Length() );
          return true;
        }
      }
    }

    if ( Message.Key == VK_F11 )
    {
      ToggleFullscreenPreview();
      return true;
    }

    if ( Message.Key == VK_F4 &&  App->GetCtrlState() )
    {
      //close workbench

      TS32 wbid = GetSelectedWorkBench();
      if ( wbid >= 0 && wbid < WorkBenches.NumItems() )
      {
        WorkBenches.FreeByIndex( wbid );
        WorkBenchButtons.FreeByIndex( wbid );

        if ( WorkBenches.NumItems() )
          SelectWorkBench( WorkBenches[ max( 0, min( WorkBenches.NumItems() - 1, wbid ) ) ] );

        CWBBox* WorkbenchButtonBox = (CWBBox*)FindChildByID( "workbenchlist", "box" );
        if ( WorkbenchButtonBox )
          App->ApplyStyle( WorkbenchButtonBox );
      }


      return true;
    }

    if ( FullscreenPreview )
    {
      switch ( Message.Key )
      {
      case VK_HOME: Project.SeekToTime( 0 ); return true;
      case VK_LEFT: Project.SeekToTime( Project.GetTimepos() - 10000 ); return true;
      case VK_RIGHT: Project.SeekToTime( Project.GetTimepos() + 10000 ); return true;
      case VK_UP: Project.SeekToTime( Project.GetTimepos() + 60000 ); return true;
      case VK_DOWN: Project.SeekToTime( Project.GetTimepos() - 60000 ); return true;
      }
    }

    break;

  }
  case WBM_FOCUSLOST:
  {
    CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
    CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );

    if ( console && !console->IsHidden() && consoleInput && Message.GetTarget() == consoleInput->GetGuid() )
    {
      CommandLogPos = CommandLog.NumItems();
      return false;
    }

    if ( WorkbenchRenamer && Message.GetTarget() == WorkbenchRenamer->GetGuid() )
    {
      TS32 wbid = GetSelectedWorkBench();
      if ( wbid >= 0 && wbid < WorkBenches.NumItems() )
      {
        WorkBenches[ wbid ]->SetName( WorkbenchRenamer->GetText() );
        WorkBenchButtons[ wbid ]->SetText( WorkbenchRenamer->GetText() );
        //WorkBenchButtons[wbid]->ResizeToContentSize(true, false, 12, 0);
        SAFEDELETE( WorkbenchRenamer );
        CWBMessage me;

        CWBBox* WorkbenchButtonBox = (CWBBox*)FindChildByID( "workbenchlist", "box" );
        if ( WorkbenchButtonBox )
          App->ApplyStyle( WorkbenchButtonBox );
      }

      //->SetName(WorkbenchRenamer->GetText());
      return true;
    }
    break;
  }
  case WBM_CHAR:
  {
    CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
    CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );

    if ( console && !console->IsHidden() && consoleInput  && consoleInput->InFocus() )
    {
      if ( Message.Key == '\t' )
      {
        HandleConsoleTab();
        return true;
      }
    }

    if ( Message.Key == '`' )
    {
      CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
      CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );

      if ( console )
      {
        console->SetTopmost();
        console->Hide( !console->IsHidden() );
        if ( consoleInput )
          consoleInput->TabStringRoot = _T( "" );
      }
      return true;
    }
    if ( Message.Key == ' ' )
    {
      if ( FullscreenPreview )
      {
        Project.TogglePlaying();
        return true;
      }

      TS32 wbid = GetSelectedWorkBench();
      if ( wbid >= 0 && wbid < WorkBenches.NumItems() )
      {
        CapexTimelineEditor *e = (CapexTimelineEditor*)WorkBenches[ wbid ]->GetWindow( apEx_TimelineEditor );
        if ( e )
        {
          Project.TogglePlaying();
          return true;
        }
      }
      return false;
    }
    break;
  }
  }

  return CWBItem::MessageProc( Message );
}

CWBButton *CapexRoot::AddTaskbarButton( const TCHAR *Name )
{
  CWBBox* WorkbenchButtonBox = (CWBBox*)FindChildByID( "workbenchlist", "box" );
  if ( !WorkbenchButtonBox )
    return NULL;

  CWBButton *b = new CWBButton( WorkbenchButtonBox, CRect( 0, 0, 0, 0 ), Name );

  //move new button to the end of the line
  if ( NewWorkbenchButton && WorkbenchButtonBox->NumChildren() > 1 )
  {
    SAFEDELETE( NewWorkbenchButton );
    NewWorkbenchButton = new CWBButton( WorkbenchButtonBox, CRect( 0, 0, 0, 0 ), "+" );
    NewWorkbenchButton->AddClass( "addworkbenchbutton" );
  }

  App->ApplyStyle( WorkbenchButtonBox );

  return b;
}

CapexWorkBench *CapexRoot::AddWorkBench( const TCHAR *Name )
{
  CWBButton *b = (CWBButton*)FindChildByID( "viewmenu", "button" );
  if ( b )
    b->Hide( false );

  b = (CWBButton*)FindChildByID( "helpmenu", "button" );
  if ( b )
    b->Hide( false );

  CWBButton *MenuButton = AddTaskbarButton( Name );
  CRect r = GetParent()->GetClientRect();

  CWBBox* wbbox = (CWBBox*)FindChildByID( "workbench", "box" );

  CapexWorkBench *WB = new CapexWorkBench( wbbox, wbbox->GetClientRect(), Name );
  WorkBenches.Add( WB );
  WB->Hide( true );
  App->ApplyStyle( WB );

  MenuButton->SetData( WB );
  WorkBenchButtons.Add( MenuButton );

  return WB;
}

void CapexRoot::OpenFileMenu()
{
  if ( App->FindItemByGuid( FileMenuID ) ) return;

  CWBButton *FileMenuButton = (CWBButton*)FindChildByID( "filemenu", "button" );
  if ( !FileMenuButton )
    return;

  CWBContextMenu *c = OpenContextMenu( FileMenuButton->GetScreenRect().BottomLeft() );
  FileMenuID = c->GetGuid();

  c->AddItem( "New", apEx_FileNew );
  c->AddSeparator();
  c->AddItem( "Open Project... (ctrl+o)", apEx_FileOpen );
  c->AddSeparator();
  c->AddItem( "Merge Project...", apEx_MergeProject );
  c->AddSeparator();
  c->AddItem( "Import/Update Material...", apEx_ImportMaterial );
  c->AddItem( "Import Scene...", apEx_ImportModel );
  c->AddItem( "Import Scene As Single Model...", apEx_ImportSingleModel );
  c->AddItem( "Import Scene As Single MiniModel...", apEx_ImportSingleMiniModel );
  c->AddSeparator();
  c->AddItem( "Save (ctrl+s)", apEx_FileSave );
  c->AddItem( "Save as...", apEx_FileSaveAs );
  c->AddSeparator();
  c->AddItem( "Export Minimal...", apEx_ExportMinimal );
  c->AddItem( "Export Minimal and C header...", apEx_ExportMinimalWithHeader );
  c->AddSeparator();
  c->AddItem( "Export Texture Filters", apEx_ExportTextureFilters );
  c->AddItem( "Export Material Library", apEx_ExportMaterialLibrary );
  c->AddItem( "Export Tree Descriptions", apEx_ExportTreeDescriptionLibrary );
  c->AddSeparator();
  CWBContextItem* skins = c->AddItem( "Select Skin", apEx_SelectCSS );

  CFileList fileList;
  fileList.ExpandSearch( _T( "*.*" ), _T( "Data/UI/" ), false, true );
  for ( int x = 0; x < fileList.Files.NumItems(); x++ )
  {
    if ( fileList.Files[ x ].isDirectory )
    {
      skins->AddItem( fileList.Files[ x ].FileName.GetPointer(), apEx_SelectCSS + x + 1 );
    }
  }

  c->AddSeparator();
  c->AddItem( "Dump Video...", apEx_VideoDump );
  if ( CompilerPath.Length() && SourceRootPath.Length() )
  {
    c->AddSeparator();
    CWBContextItem *compile = c->AddItem( "Compile Release", -1 );

    compile->AddItem( "Conspiracy 64k", apEx_MakeExe_64k_CNS );
    compile->AddItem( "Conspiracy Demo", apEx_MakeExe_Demo_CNS );
    compile->AddItem( "ThirdParty 64k", apEx_MakeExe_64k_External );
    compile->AddItem( "ThirdParty Demo", apEx_MakeExe_Demo_External );
    compile->AddSeparator();
    compile->AddItem( "!!!REMEMBER THE SHADER MINIFIER TOGGLE!!!", 0x25472 );
  }
  c->AddSeparator();
  if ( Config::RecentFiles.NumItems() )
  {
    //CWBContextItem *i=c->AddItem(_T("Recents"),-1);
    for ( TS32 x = Config::RecentFiles.NumItems() - 1; x >= 0; x-- )
    {
      CStringArray r = Config::RecentFiles[ x ].Explode( _T( "\\" ) );
      c->AddItem( r.Last().GetPointer(), 1000 + x, true );
    }
  }
  if ( Config::RecentFiles.NumItems() )
  {
    c->AddSeparator();
  }
  c->AddItem( "Exit (alt+f4)", apEx_Exit );
}

void CapexRoot::OpenViewMenu()
{
  if ( App->FindItemByGuid( ViewMenuID ) ) return;

  CWBButton *ViewMenuButton = (CWBButton*)FindChildByID( "viewmenu", "button" );
  if ( !ViewMenuButton || ViewMenuButton->IsHidden() )
    return;

  CapexWorkBench *wb = NULL;
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( !WorkBenches[ x ]->IsHidden() ) wb = WorkBenches[ x ];

  if ( !wb ) return;

  CWBContextMenu *c = wb->OpenContextMenu( ViewMenuButton->GetScreenRect().BottomLeft() );
  ViewMenuID = c->GetGuid();

  c->AddItem( "Texture Page List", apEx_TexGenPages );
  c->AddItem( "Texture Generator", apEx_TexGenMain );
  c->AddItem( "Texture Preview", apEx_TexGenPreview );
  c->AddItem( "Texture Operator Parameters", apEx_TextureOpParameters );
  c->AddItem( "Texture Filter Editor", apEx_TexGenFilterEditor );
  c->AddSeparator();

  //c->AddItem("Material List", apEx_MaterialList); //no need for this, integrated into the material editor.
  c->AddItem( "Material Editor", apEx_MaterialEditor );
  c->AddItem( "Material Shader", apEx_MaterialShaderEditor );
  c->AddItem( "Render Target Editor", apEx_RenderTargetEditor );
  c->AddSeparator();

  c->AddItem( "Tree Species Editor", apEx_TreeSpeciesEditor );
  c->AddSeparator();
  c->AddItem( "Model List", apEx_ModelList );
  c->AddItem( "Model Primitives", apEx_ModelPrimitives );
  c->AddItem( "Model Graph", apEx_ModelGraph );
  c->AddItem( "Model Preview", apEx_ModelView );
  c->AddItem( "Model Parameters", apEx_ModelParameters );
  c->AddItem( "Model Material", apEx_ModelMaterial );
  c->AddItem( "Model Matrix", apEx_ModelMatrix );
  c->AddSeparator();

  c->AddItem( "Scene List", apEx_SceneList );
  c->AddItem( "Scene Primitives", apEx_ScenePrimitives );
  c->AddItem( "Scene Graph", apEx_SceneGraph );
  c->AddItem( "Scene Preview", apEx_SceneView );
  c->AddItem( "Scene Clips", apEx_SceneClips );
  c->AddItem( "Scene Object Parameters", apEx_SceneObjectParameters );
  c->AddItem( "Scene Spline Editor", apEx_SceneSplineEditor );
  c->AddSeparator();

  c->AddItem( "TimeLine Editor", apEx_TimelineEditor );
  c->AddItem( "TimeLine Preview", apEx_TimelinePreview );
  c->AddItem( "TimeLine Event Parameters", apEx_TimelineEventParameters );
  c->AddItem( "TimeLine Event Splines", apEx_TimelineEventSplines );

  c->AddSeparator();
  c->AddItem( "UI XML Editor", apEx_XMLEditor );
  c->AddItem( "UI CSS Editor", apEx_CSSEditor );

  c->AddSeparator();
  c->AddItem( "Help", apEx_Help );
  c->AddItem( "Project Settings", apEx_ProjectSettings );
  c->AddItem( "Console Output", apEx_Console );
  c->AddItem( "KKrunchy Pack Info Viewer", apEx_KKPViewer );
}

void CapexRoot::LoadDefaultLayout()
{
  //force resize messages to be handlesd
  App->HandleMessages();

  CapexWorkBench *w;
  w = AddWorkBench( "Texture Generator" );
  CSize c = w->GetClientRect().Size();

  w->OpenWindow( apEx_TexGenPages )->SetPosition( CRect( 0, 0, 150, 256 ) );
  w->OpenWindow( apEx_ModelView )->SetPosition( CRect( 150, 0, 150 + 256, 256 ) );
  w->OpenWindow( apEx_ModelList )->SetPosition( CRect( 150 + 256, 0, 300 + 256, 256 ) );
  w->OpenWindow( apEx_Help )->SetPosition( CRect( 300 + 256, 0, (TS32)( c.x*0.7f ) - 256 * 2, 256 ) );
  w->OpenWindow( apEx_TexGenPreview )->SetPosition( CRect( (TS32)( c.x*0.7f ) - 256 * 2, 0, (TS32)( c.x*0.7f ) - 256, 256 ) );
  w->OpenWindow( apEx_TextureOpParameters )->SetPosition( CRect( (TS32)( c.x*0.7f ) - 256, 0, (TS32)( c.x*0.7f ), 256 ) );
  w->OpenWindow( apEx_TexGenFilterEditor )->SetPosition( CRect( CPoint( (TS32)( c.x*0.7f ), 0 ), c ) );
  w->OpenWindow( apEx_TexGenMain )->SetPosition( CRect( 0, 256, (TS32)( c.x*0.7f ), c.y ) );

  SelectWorkBench( w );

  w = AddWorkBench( "Material Editor" );

  CSize merec = c;
  w->OpenWindow( apEx_MaterialShaderEditor )->SetPosition( CRect( c.x - (TS32)( c.x / 3.0f ), 0, c.x, c.y ) );
  merec.x -= (TS32)( c.x / 3.0f );

  w->OpenWindow( apEx_ModelPrimitives )->SetPosition( CRect( 0, merec.y - 374, 166, merec.y ) );
  w->OpenWindow( apEx_ModelList )->SetPosition( CRect( 0, 0, 166, 85 ) );
  w->OpenWindow( apEx_ModelParameters )->SetPosition( CRect( 0, 85, 166, 166 ) );
  w->OpenWindow( apEx_ModelMaterial )->SetPosition( CRect( 0, 166, 166, merec.y - 374 ) );
  w->OpenWindow( apEx_RenderTargetEditor )->SetPosition( CRect( 166, merec.y - 250, 414, merec.y ) );
  w->OpenWindow( apEx_MaterialEditor )->SetPosition( CRect( 414, merec.y - 250, merec.x, merec.y ) );
  w->OpenWindow( apEx_SceneSplineEditor )->SetPosition( CRect( 166, merec.y - 400, merec.x - 115, merec.y - 250 ) );
  w->OpenWindow( apEx_SceneClips )->SetPosition( CRect( merec.x - 115, merec.y - 400, merec.x, merec.y - 250 ) );
  w->OpenWindow( apEx_SceneList )->SetPosition( CRect( merec.x - 115, 0, merec.x, 85 ) );
  w->OpenWindow( apEx_SceneObjectParameters )->SetPosition( CRect( merec.x - 115, 85, merec.x, merec.y - 706 ) );
  w->OpenWindow( apEx_ScenePrimitives )->SetPosition( CRect( merec.x - 115, merec.y - 706, merec.x, merec.y - 400 ) );
  merec.x -= 115 + 166;
  merec.y -= 400;
  w->OpenWindow( apEx_ModelView )->SetPosition( CRect( 166, 0, 166 + merec.x / 2, merec.y / 2 ) );
  w->OpenWindow( apEx_ModelView )->SetPosition( CRect( 166, merec.y / 2, 166 + merec.x / 2, merec.y ) );
  w->OpenWindow( apEx_SceneView )->SetPosition( CRect( 166 + merec.x / 2, 0, 166 + merec.x, merec.y / 2 ) );
  w->OpenWindow( apEx_SceneView )->SetPosition( CRect( 166 + merec.x / 2, merec.y / 2, 166 + merec.x, merec.y ) );

  w = AddWorkBench( "Modeler" );

  CapexModelView *m = (CapexModelView *)w->OpenWindow( apEx_ModelView ); m->SetPosition( CRect( 0, 0, (TS32)( c.x*0.4f ), (TS32)( c.y*0.5 ) ) );	m->SetCameraMode( CAMERA_TOP );
  m = (CapexModelView *)w->OpenWindow( apEx_ModelView ); m->SetPosition( CRect( (TS32)( c.x*0.4f ), 0, (TS32)( c.x*0.8f ), (TS32)( c.y*0.5 ) ) );		m->SetCameraMode( CAMERA_RIGHT );
  m = (CapexModelView *)w->OpenWindow( apEx_ModelView ); m->SetPosition( CRect( 0, (TS32)( c.y*0.5 ), (TS32)( c.x*0.4f ), c.y ) );					m->SetCameraMode( CAMERA_FRONT );
  m = (CapexModelView *)w->OpenWindow( apEx_ModelView ); m->SetPosition( CRect( (TS32)( c.x*0.4f ), (TS32)( c.y*0.5 ), (TS32)( c.x*0.8f ), c.y ) );	m->SetCameraMode( CAMERA_NORMAL );
  w->OpenWindow( apEx_ModelMaterial )->SetPosition( CRect( (TS32)( c.x*0.8f ), (TS32)( c.y*3.0 / 5.0f ), c.x, c.y ) );
  w->OpenWindow( apEx_ModelList )->SetPosition( CRect( (TS32)( c.x*0.8f ), 0, (TS32)( c.x*0.9f ), (TS32)( c.y / 5.0f ) ) );
  w->OpenWindow( apEx_ModelParameters )->SetPosition( CRect( (TS32)( c.x*0.8f ), (TS32)( c.y / 5.0f ), (TS32)( c.x*0.9f ), (TS32)( c.y*3.0 / 5.0f ) ) );
  w->OpenWindow( apEx_ModelPrimitives )->SetPosition( CRect( (TS32)( c.x*0.9f ), 0, c.x, (TS32)( c.y*2.0f / 5.0f ) ) );
  w->OpenWindow( apEx_ModelGraph )->SetPosition( CRect( (TS32)( c.x*0.9f ), (TS32)( c.y*2.0f / 5.0f ), c.x, (TS32)( c.y*3.0 / 5.0f ) ) );

  w = AddWorkBench( "Keyframer" );

  w->OpenWindow( apEx_SceneSplineEditor )->SetPosition( CRect( 0, (TS32)( c.y*0.75f ), c.x, c.y ) );
  CapexSceneView *s = (CapexSceneView *)w->OpenWindow( apEx_SceneView ); s->SetPosition( CRect( (TS32)( c.x*0.2f ), 0, (TS32)( c.x*0.6f ), (TS32)( c.y*0.5*0.75f ) ) ); s->SetCameraMode( CAMERA_TOP );
  s = (CapexSceneView *)w->OpenWindow( apEx_SceneView ); s->SetPosition( CRect( (TS32)( c.x*0.6f ), 0, c.x, (TS32)( c.y*0.5*0.75f ) ) );							  s->SetCameraMode( CAMERA_RIGHT );
  s = (CapexSceneView *)w->OpenWindow( apEx_SceneView ); s->SetPosition( CRect( (TS32)( c.x*0.2f ), (TS32)( c.y*0.5*0.75f ), (TS32)( c.x*0.6f ), (TS32)( c.y*0.75f ) ) ); s->SetCameraMode( CAMERA_FRONT );
  s = (CapexSceneView *)w->OpenWindow( apEx_SceneView ); s->SetPosition( CRect( (TS32)( c.x*0.6f ), (TS32)( c.y*0.5*0.75f ), c.x, (TS32)( c.y*0.75f ) ) );			  s->SetCameraMode( CAMERA_NORMAL );
  w->OpenWindow( apEx_SceneList )->SetPosition( CRect( 0, 0, (TS32)( c.x*0.1 ), (TS32)( c.y*0.2f ) ) );
  w->OpenWindow( apEx_SceneClips )->SetPosition( CRect( 0, (TS32)( c.y*0.2f ), (TS32)( c.x*0.1 ), (TS32)( c.y*0.4f ) ) );
  w->OpenWindow( apEx_ScenePrimitives )->SetPosition( CRect( 0, (TS32)( c.y*0.4f ), (TS32)( c.x*0.1 ), (TS32)( c.y*0.75f ) ) );
  w->OpenWindow( apEx_SceneGraph )->SetPosition( CRect( (TS32)( c.x*0.1 ), 0, (TS32)( c.x*0.2 ), (TS32)( c.y*0.25 ) ) );
  w->OpenWindow( apEx_SceneObjectParameters )->SetPosition( CRect( (TS32)( c.x*0.1 ), (TS32)( c.y*0.25 ), (TS32)( c.x*0.2 ), (TS32)( c.y*0.75f ) ) );

  w = AddWorkBench( "Video Editor" );

  TS32 x = (TS32)( c.x*0.8 );
  TS32 y = (TS32)( x * 9 / 16 );
  w->OpenWindow( apEx_TimelinePreview )->SetPosition( CRect( 0, 0, x, y ) );
  w->OpenWindow( apEx_TimelineEventParameters )->SetPosition( CRect( x, (TS32)( c.y / 3.0f ), c.x, y ) );
  w->OpenWindow( apEx_TimelineEditor )->SetPosition( CRect( 0, y, (TS32)( c.x*0.7f ), c.y ) );
  w->OpenWindow( apEx_TimelineEventSplines )->SetPosition( CRect( (TS32)( c.x*0.7f ), y, c.x, c.y ) );
  w->OpenWindow( apEx_ProjectSettings )->SetPosition( CRect( x, 0, c.x, (TS32)( c.y / 3.0f ) ) );

  w = AddWorkBench( "UI Editor" );

  w->OpenWindow( apEx_Console )->SetPosition( CRect( 0, (TS32)( c.y*0.7 ), c.x, c.y ) );
  w->OpenWindow( apEx_XMLEditor )->SetPosition( CRect( 0, 0, c.x / 4, (TS32)( c.y*0.7 ) ) );
  w->OpenWindow( apEx_CSSEditor )->SetPosition( CRect( c.x / 4, 0, c.x / 2, (TS32)( c.y*0.7 ) ) );

  App->HandleMessages(); //so all windows get resized

  for ( x = 0; x < WorkBenches.NumItems(); x++ )
  {
    CRect BoundingBox = CRect( 0, 0, 0, 0 );
    TBOOL HasElements = false;
    for ( y = 0; y < WorkBenches[ x ]->GetWindowCount(); y++ )
    {
      CapexWindow *win = WorkBenches[ x ]->GetWindowByIndex( y );
      if ( win->GetWindowType() == apEx_ModelView || win->GetWindowType() == apEx_SceneView )
      {
        if ( !HasElements ) BoundingBox = win->GetPosition();
        else BoundingBox = BoundingBox&win->GetPosition();
        HasElements = true;
      }
    }

    if ( HasElements )
    {
      for ( y = 0; y < WorkBenches[ x ]->GetWindowCount(); y++ )
      {
        CapexWindow *win = WorkBenches[ x ]->GetWindowByIndex( y );
        if ( win->GetWindowType() == apEx_ModelView )
        {
          CapexModelView *mv = (CapexModelView*)win;
          mv->SetMaximizedPosition( BoundingBox );
        }
        if ( win->GetWindowType() == apEx_SceneView )
        {
          CapexSceneView *mv = (CapexSceneView*)win;
          mv->SetMaximizedPosition( BoundingBox );
        }
      }
    }
  }
}

void CapexRoot::ExecuteCommand( CStringArray &Commandline )
{
  if ( !Commandline.NumItems() ) return;

  CString s = Commandline.Implode( _T( " " ) );
  LOG( LOG_INFO, _T( "%s" ), s.GetPointer() );

  CString n = Commandline[ 0 ];
  n.ToLower();

  if ( ConsoleCommands.HasKey( n ) )
    ConsoleCommands[ n ].Command( Commandline );
  else
    LOG_WARN( "Command  '%s' unknown.", Commandline[ 0 ].GetPointer() );

}

void CapexRoot::UpdateWindowData( APEXWINDOW WindowType )
{
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    for ( TS32 y = 0; y < WorkBenches[ x ]->GetWindowCount(); y++ )
      if ( WorkBenches[ x ]->GetWindowByIndex( y )->GetWindowType() == WindowType )
        WorkBenches[ x ]->GetWindowByIndex( y )->UpdateData();
}

void CapexRoot::ClearProjectData()
{
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    //for (TS32 y = 0; y < WorkBenches[x]->GetWindowCount(); y++)
  {
    WorkBenches[ x ]->ResetUIData();
    WorkBenches[ x ]->SetEditedModel( NULL );
    WorkBenches[ x ]->SetEditedScene( NULL );
    WorkBenches[ x ]->SetEditedModelObject( NULL );
    WorkBenches[ x ]->SetEditedModelFilter( NULL );
  }
}

void CapexRoot::UpdateWindowData()
{
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    for ( TS32 y = 0; y < WorkBenches[ x ]->GetWindowCount(); y++ )
      WorkBenches[ x ]->GetWindowByIndex( y )->UpdateData();
}

TS32 CapexRoot::GetSelectedWorkBench()
{
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( !WorkBenches[ x ]->IsHidden() ) return x;
  return 0;
}

#include <CommDlg.h>

void CapexRoot::OpenProject()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Project Files\0*.apx\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open Project";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "apx";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "openproject" );

  if ( GetOpenFileName( &opf ) )
  {
    StoreCurrentDirectory( "openproject" );
    SetCurrentDirectory( dir );

    ClearProjectData();
    Project.Import( CString( opf.lpstrFile ), (HWND)App->GetHandle() );
    UpdateWindowData();
    SelectFirstTextureObjectScene();

    CStringArray r = Project.LoadedFileName.Explode( _T( "\\" ) );
    if ( r.NumItems() )
      GetApplication()->SetWindowTitle( CString::Format( _T( "%s - apEx IntroTool" ), r.Last().GetPointer() ) );

    Config::RecentFiles.Delete( CString( opf.lpstrFile ) );
    Config::RecentFiles.Add( CString( opf.lpstrFile ) );
  }

  SetCurrentDirectory( dir );
}

#include "MeshImport.h"

void CapexRoot::ImportModel( bool singleModel, bool miniModel )
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "Model Files\0*.fbx;*.3ds;*.ase;*.lwo;*.obj;*.dae;*.blend;*.x\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Import Model";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "fbx";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "importmodel" );
  if ( GetOpenFileName( &opf ) )
  {
    StoreCurrentDirectory( "importmodel" );
    SetCurrentDirectory( dir );

    ClearProjectData();

    if ( !singleModel )
      ImportWithAssimp( CString( opf.lpstrFile ) );
    else
      ImportToSingleModel( CString( opf.lpstrFile ), miniModel );

    //ImportRIP( CString( opf.lpstrFile ) );

    //CFileList f( "mesh_*.rip", "E:\\ManhattanRip\\_NinjaRipper\\2016.03.22_21.48.16_firefox.exe\\2016.03.22_21.50.28_firefox.exe\\" );

    //for ( TS32 x = 0; x < f.Files.NumItems(); x++ )
    //  ImportRIP( f.Files[x].Path+f.Files[x].FileName );

    //Project.Import(CString(opf.lpstrFile), (HWND)App->GetHandle());
    UpdateWindowData();
    SelectFirstTextureObjectScene();
  }

  SetCurrentDirectory( dir );
}

#include "KasparovImport.h"

void CapexRoot::ImportKasparov()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "Model Files\0kasparov.dat\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Import Kasparov Datafile";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "dat";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "importmodel" );
  if ( GetOpenFileName( &opf ) )
  {
    StoreCurrentDirectory( "importmodel" );
    SetCurrentDirectory( dir );

    ClearProjectData();

    ImportKasparovDat( CString( opf.lpstrFile ) );

    UpdateWindowData();
    SelectFirstTextureObjectScene();
  }

  SetCurrentDirectory( dir );
}

CString CapexRoot::GetCSSPath()
{
  return CString( _T( "Data/UI/" ) ) + Config::skinCSS + _T( "/" );
}

TBOOL CapexRoot::TrySetSolutionRoot( const CString & Text )
{
  CString s = Text + _T( "apEx.sln" );
  CStreamReaderFile Project;
  if ( Project.Open( s.GetPointer() ) )
  {
    SourceRootPath = Text;
    LOG_NFO( "[apEx] Source root located at %s", SourceRootPath.GetPointer() );
    return true;
  }
  return false;
}

void CapexRoot::SaveProject()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Project Files\0*.apx\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save Project";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "apx";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "saveproject" );

  if ( GetSaveFileName( &opf ) )
  {
    StoreCurrentDirectory( "saveproject" );
    App->SelectMouseCursor( CM_WAIT );
    App->FinalizeMouseCursor();
    SetCurrentDirectory( dir );
    Project.LoadedFileName = opf.lpstrFile;
    Project.Export( CString( opf.lpstrFile ), true, false );

    CStringArray r = Project.LoadedFileName.Explode( _T( "\\" ) );
    if ( r.NumItems() )
      App->SetWindowTitle( CString::Format( _T( "%s - apEx IntroTool" ), r.Last().GetPointer() ) );

    Config::RecentFiles.Delete( CString( opf.lpstrFile ) );
    Config::RecentFiles.Add( CString( opf.lpstrFile ) );
  }
  SetCurrentDirectory( dir );
}

void CapexRoot::ExportMinimalFile( bool exportHeader )
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Minimal Files\0*.64k\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Export Minimal Project";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "64k";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "exportminimal" );
  if ( GetSaveFileName( &opf ) )
  {
    StoreCurrentDirectory( "exportminimal" );
    App->SelectMouseCursor( CM_WAIT );
    App->FinalizeMouseCursor();
    SetCurrentDirectory( dir );

    ExportMinimal( CString( opf.lpstrFile ), false, exportHeader );
  }
  SetCurrentDirectory( dir );
}

void CapexRoot::SetStatusbarText( CString Text )
{
  CWBLabel* label = (CWBLabel*)FindChildByID( "statusbartext", "label" );
  if ( label )
  {
    label->SetText( Text );
    label->ApplyStyle( CString( "font-color" ), CString( "#ffffff" ), CStringArray() );
  }
}

void CapexRoot::SetStatusbarError( CString Text )
{
  CWBLabel* label = (CWBLabel*)FindChildByID( "statusbartext", "label" );
  if ( label )
  {
    label->SetText( Text );
    label->ApplyStyle( CString( "font-color" ), CString( "#ff2020" ), CStringArray() );
  }
}

void CapexRoot::SetUbertoolSnap( TF32 snap )
{
  Config::UbertoolSnap = snap;

  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
  {
    TS32 cnt = WorkBenches[ x ]->GetWindowCount( apEx_ModelView );
    for ( TS32 y = 0; y < cnt; y++ )
    {
      CapexModelView *w = (CapexModelView*)WorkBenches[ x ]->GetWindow( apEx_ModelView, y );
      w->SetUbertoolSnap();
    }
    cnt = WorkBenches[ x ]->GetWindowCount( apEx_SceneView );
    for ( TS32 y = 0; y < cnt; y++ )
    {
      CapexSceneView *w = (CapexSceneView*)WorkBenches[ x ]->GetWindow( apEx_SceneView, y );
      w->SetUbertoolSnap();
    }
  }
}

void CapexRoot::ToggleFullscreenPreview()
{
  Project.ResetParticles();

  if ( FullscreenPreview )
  {
    FullscreenPreview->MarkForDeletion();
    FullscreenPreview = NULL;

    if ( CurrentWorkbench )
      CurrentWorkbench->Hide( false );

    Project.StopPlayback();

    return;
  }

  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
  {
    if ( !WorkBenches[ x ]->IsHidden() ) CurrentWorkbench = WorkBenches[ x ];
    WorkBenches[ x ]->Hide( true );
  }

  FullscreenPreview = new CWBDemoPreview( this, GetClientRect() );
}

DWORD __stdcall FixedStepTimer();
extern TS32 FixesStepTimePosition;

void CapexRoot::ToggleVideoDumper()
{
  Project.ResetParticles();
  if ( VideoDumper )
  {
    VideoDumper->MarkForDeletion();
    VideoDumper = NULL;
    TimerFunction = timeGetTime;
    //if (CurrentWorkbench)
    //	CurrentWorkbench->Hide(false);
    return;
  }
  VideoDumper = new CapexVideoDumper( this, GetClientRect() );
  VideoDumper->SetFocus();
  TimerFunction = FixedStepTimer;
  FixesStepTimePosition = 0;
}

#include "../Phoenix/Timeline.h"

void CapexRoot::FSMouseSeek()
{
  CPoint p = App->GetMousePos();
  TS32 maxeventend = 0;
  for ( TS32 x = 0; x < Project.GetEventCount(); x++ )
  {
    CphxEvent_Tool *e = Project.GetEventByIndex( x );
    maxeventend = max( e->Event->EndFrame, maxeventend );
  }

  if ( maxeventend <= 0 ) return;

  TS32 width = GetClientRect().Width();
  TF32 tpos = p.x / (TF32)width;

  Project.SeekToFrame( (TS32)( tpos*maxeventend ) );
}

#include "TexGenPages.h"
#include "ModelList.h"
#include "SceneList.h"
#include "TexGenFilterEditor.h"

void CapexRoot::SelectFirstTextureObjectScene()
{
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
  {
    if ( WorkBenches[ x ]->GetWindow( apEx_TexGenPages ) )
      WorkBenches[ x ]->SetEditedPage( 0 );
    //( (CapexTexGenPages*)WorkBenches[ x ]->GetWindow( apEx_TexGenPages ) )->SetEditedPage( 0 );

    if ( WorkBenches[ x ]->GetWindow( apEx_ModelList ) )
      ( (CapexModelList*)WorkBenches[ x ]->GetWindow( apEx_ModelList ) )->SelectModel( 0 );

    if ( WorkBenches[ x ]->GetWindow( apEx_SceneList ) )
      ( (CapexSceneList*)WorkBenches[ x ]->GetWindow( apEx_SceneList ) )->SelectScene( 0 );

    if ( WorkBenches[ x ]->GetWindow( apEx_TexGenFilterEditor ) )
      ( (CapexTexGenFilterEditor*)WorkBenches[ x ]->GetWindow( apEx_TexGenFilterEditor ) )->SetEditedFilter( NULL );
  }
}

void CapexRoot::RegisterConsoleCommand( TCHAR *Name, TCHAR *Description, APEXCONSOLECOMMAND Callback )
{
  CString n = Name;
  n.ToLower();

  APEXCONSOLECOMMANDDESC d;
  d.Command = Callback;
  d.Description = Description;
  ConsoleCommands[ n ] = d;
}

int AlphabeticSortCallback( CString *a, CString *b )
{
  CString _a = *a;
  CString _b = *b;
  _a.ToLower();
  _b.ToLower();
  if ( _a > _b )
    return 1;

  return _a == _b ? 0 : -1;
}

void CapexRoot::ListCommands()
{
  LOG_NFO( "" );
  LOG_NFO( "Available console commands are the following:" );
  LOG_NFO( "" );

  CStringArray Commands;
  for ( TS32 x = 0; x < ConsoleCommands.NumItems(); x++ )
  {
    Commands += ConsoleCommands.GetKDPair( x )->Key;
  }

  Commands.Sort( AlphabeticSortCallback );

  for ( TS32 x = 0; x < Commands.NumItems(); x++ )
    LOG_NFO( "%s - %s", Commands[ x ].GetPointer(), ConsoleCommands[ Commands[ x ] ].Description.GetPointer() );

  LOG_NFO( "" );
}

void CapexRoot::MoveInCommandLog( TS32 step )
{
  CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
  CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );
  if ( !console || !consoleInput )
    return;

  if ( consoleInput->IsHidden() ) return;
  consoleInput->TabStringRoot = _T( "" );

  if ( CommandLogPos + step >= CommandLog.NumItems() )
  {
    consoleInput->SetText( _T( "" ) );
    CommandLogPos = CommandLog.NumItems();
    return;
  }

  if ( step >= 0 && CommandLogPos == CommandLog.NumItems() ) return;

  TS32 CLP = CommandLogPos;
  CommandLogPos = max( 0, min( CommandLog.NumItems() - 1, CommandLogPos + step ) );

  if ( CLP != CommandLogPos )
  {
    if ( CLP == CommandLog.NumItems() && consoleInput->GetText().Length() )
      CommandLog += consoleInput->GetText();

    consoleInput->SetText( CommandLog[ CommandLogPos ] );
  }
}

void CapexRoot::HandleConsoleTab()
{
  CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
  CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );
  if ( !console || !consoleInput )
    return;

  if ( console->IsHidden() ) return;
  if ( !consoleInput->InFocus() ) return;

  CString cmd = consoleInput->GetText();
  if ( !cmd.Length() )
  {
    consoleInput->SetText( _T( "list" ) );
    return;
  }

  CStringArray command = cmd.ExplodeByWhiteSpace();
  if ( command.NumItems() > 1 ) return;

  if ( !consoleInput->TabStringRoot.Length() )
  {
    consoleInput->TabStringRoot = command[ 0 ];
    consoleInput->TabStringRoot.ToLower();
  }

  CStringArray ApplicableCommands;

  for ( TS32 x = 0; x < ConsoleCommands.NumItems(); x++ )
  {
    CString cm = ConsoleCommands.GetKDPair( x )->Key;
    cm.ToLower();
    if ( cm.Find( consoleInput->TabStringRoot ) == 0 )
      ApplicableCommands += cm;
  }

  ApplicableCommands.Sort( AlphabeticSortCallback );

  if ( !ApplicableCommands.NumItems() ) return;
  TS32 currentcommandindex = -1;
  CString currcomm = command[ 0 ];
  currcomm.ToLower();

  for ( TS32 x = 0; x < ApplicableCommands.NumItems(); x++ )
    if ( currcomm == ApplicableCommands[ x ] ) currentcommandindex = x;

  currentcommandindex = max( 0, currentcommandindex + 1 );
  currentcommandindex = currentcommandindex%ApplicableCommands.NumItems();
  consoleInput->SetText( ApplicableCommands[ currentcommandindex ] );
}

#include "ExternalTools.h"

void CapexRoot::ToggleConsole( TBOOL Show )
{
  CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
  CapexConsoleInputLine* consoleInput = (CapexConsoleInputLine*)FindChildByID( "consoleinput", "consoleinput" );
  if ( !console || !consoleInput )
    return;

  console->SetTopmost();
  console->ApplyStyleDeclarations( Show ? "visibility:visible;" : "visibility:hidden;" );
  console->Hide( !Show );
  consoleInput->TabStringRoot = _T( "" );
}

void CapexRoot::UpdateColorPicker()
{
  CPoint p = App->GetMousePos();
  POINT pp;
  pp.x = p.x;
  pp.y = p.y;

  //::ClientToScreen( (HWND)App->GetHandle(), &pp );
  COLORREF color = GetPixel( GetDC( (HWND)App->GetHandle() ), pp.x, pp.y );

  CColor col( color );
  col.A() = 0xff;

  CWBItem* quad = FindChildByID( "colorpickquad" );
  CWBLabel* text = FindChildByID<CWBLabel>( "colorpicktext" );

  if ( quad )
  {
    quad->ApplyStyleDeclarations( CString::Format( "background:#%.2x%.2x%.2x;", col.B(), col.G(), col.R() ) );
  }

  if ( text )
  {
    TS32 red = (TS32)( DeGamma( col.B() / 255.0f )*255.0f );
    TS32 gre = (TS32)( DeGamma( col.G() / 255.0f )*255.0f );
    TS32 blu = (TS32)( DeGamma( col.R() / 255.0f )*255.0f );

    text->SetText( CString::Format( "R: %.3d G: %.3d B: %.3d - Rl: %.3d Gl: %.3d Bl: %.3d", col.B(), col.G(), col.R(), red, gre, blu ) );
  }

}

char* mvxMachineDefines[] =
{
  "MVX_COMPILE_MASTER",
  "MVX_COMPILE_DRUM",
  "MVX_COMPILE_DISTO",
  "MVX_COMPILE_SWEEP",
  "MVX_COMPILE_FILTER",
  "MVX_COMPILE_DELAY",
  "MVX_COMPILE_COMP",
  "MVX_COMPILE_SWEEP_B",
  "MVX_COMPILE_FLANGE",
  "MVX_COMPILE_EQ",
  "MVX_COMPILE_FORMANT",
  "MVX_COMPILE_PIANO",
  "MVX_COMPILE_GUITAR",
  "MVX_COMPILE_CHORUS_IQ",
  "MVX_COMPILE_FLUTE",
  "MVX_COMPILE_SHAPER",
  "MVX_COMPILE_REVERB",
  "MVX_COMPILE_BANDSAW",
  "MVX_COMPILE_SAMPLER",
  "MVX_COMPILE_GLITCH",
  "MVX_COMPILE_MONSTA",
  "MVX_COMPILE_GRAIN",
  "MVX_COMPILE_FREEVERB",
  "MVX_COMPILE_GRAINDLS",
  "MVX_COMPILE_CHORUS",
  "MVX_COMPILE_COMB",
};

#define MVX_NUMMACHINES 26
static_assert( sizeof( mvxMachineDefines ) / sizeof( char* ) == (int)MVX_NUMMACHINES, "MVX MACHINE LIST FOR MVXDEFINES IS NOT UP TO DATE!" );

bool mvxMachinesUsed[ MVX_NUMMACHINES ];

CString GetMVXDefines( unsigned char* mvx, int mvxSize )
{
  memset( mvxMachinesUsed, 0, sizeof( mvxMachinesUsed ) );
  if ( !mvx )
    return CString();

  if ( ( *(unsigned int*)mvx ) != 'MVM' )
    return CString();

  unsigned char machineCount = mvx[ 10 ];
  for ( int x = 0; x < machineCount; x++ )
    mvxMachinesUsed[ mvx[ x + 11 ] ] = true;

  CString result;

  for ( int x = 0; x < MVX_NUMMACHINES; x++ )
    result += CString::Format( "%sefine %s\n", mvxMachinesUsed[ x ] ? "#d" : "//", mvxMachineDefines[ x ] );

  return result;
}

#include <shlwapi.h>
#include <pathcch.h>

void CapexRoot::CompileRelease( TS32 Type )
{
  if ( !SourceRootPath.Length() || !CompilerPath.Length() ) return;

  CString SourcePath = SourceRootPath + _T( "apEx.sln" );

  CWBBox* console = (CWBBox*)FindChildByID( "console", "box" );
  bool ConsoleHidden = console && console->IsHidden();

  if ( ConsoleHidden )
    ToggleConsole( ConsoleHidden );

  //create minimal file here

  ExportMinimal( ( SourceRootPath + _T( "MinimalPlayer\\Data\\release.64k" ) ).GetPointer(), Type == apEx_MakeExe_Demo_CNS || Type == apEx_MakeExe_Demo_External, false );

  if ( Type == apEx_MakeExe_Demo_CNS || Type == apEx_MakeExe_64k_CNS )
  {
    //CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalcphoenix.64k" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalc.64k" ) ).GetPointer(), 0 );
    if ( !Project.EnableFarbrauschPrecalc )
    {
      if ( !Project.EnableMinimalPrecalc )
        CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalcphoenix_shader.64k" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalc.64k" ) ).GetPointer(), 0 );
      else
        CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalcminimal.64k" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalc.64k" ) ).GetPointer(), 0 );
    }
    else
      CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalcfarbrausch.64k" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalc.64k" ) ).GetPointer(), 0 );
  }
  else
  {
    CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalcapex.64k" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\precalc.64k" ) ).GetPointer(), 0 );
  }

  if ( !MinimalExportSuccessful )
  {
    if ( ConsoleHidden ) ToggleConsole( !ConsoleHidden );
    LOG_ERR( "[apex] Minimal export failed. Halting compilation." );
    return;
  }

  //copy song

  if ( Project.SongFile.Length() )
    CopyFile( Project.SongFile.GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\release.mus" ) ).GetPointer(), 0 );


  bool mvxDefinesReplaced = false;

  {
    CStreamReaderMemory mvm;

    bool open = mvm.Open( Project.SongFile.GetPointer() );
    if ( !open )
    {
      TCHAR name[ MAX_PATH ];
      LPSTR* fileName = nullptr;
      if ( GetFullPathName( Project.LoadedFileName.GetPointer(), MAX_PATH, name, fileName ) )
      {
        PathRemoveFileSpecA( name );

        TCHAR relative[ MAX_PATH ];
        PathCombineA( relative, name, Project.SongFile.GetPointer() );
        open = mvm.Open( relative );
      }
    }

    CString mvxDefines = GetMVXDefines( mvm.GetData(), (int)mvm.GetLength() );

    if ( mvxDefines.Length() )
    {
      CopyFile( ( SourceRootPath + _T( "..\\MVX\\Library\\mvxDefines.h" ) ).GetPointer(), ( SourceRootPath + _T( "..\\MVX\\Library\\mvxDefines.bak" ) ).GetPointer(), 0 );
      CStreamWriterFile writer;
      writer.Open( ( SourceRootPath + _T( "..\\MVX\\Library\\mvxDefines.h" ) ).GetPointer() );
      writer.Write( mvxDefines.GetPointer(), mvxDefines.Length() );
      mvxDefinesReplaced = true;
    }
  }

  //write song related headers
  if ( Project.IsSongLoaded() && Project.GetSynthConfig().Length() != 0 )
  {
    FILE *f = NULL;
    fopen_s( &f, "..\\MinimalPlayer\\SynthConfig.inc", "w+t" );
    fprintf_s( f, "%s", Project.GetSynthConfig().GetPointer() );
    fclose( f );
  }

  //create minimal config here

  FILE *f = NULL;
  fopen_s( &f, "..\\LibCTiny\\helperfunctions.cpp", "w+t" );
  if ( f )
  {
    if ( Type == apEx_MakeExe_64k_CNS || Type == apEx_MakeExe_64k_External )
      fprintf_s( f,
                 //"extern \"C\"\n"
                 //"				{\n"
                 //"	__declspec(naked) int rounding_common()\n"
                 //"	{									   \n"
                 //"		__asm							   \n"
                 //"		{								   \n"
                 //"			fnstcw[ESP - 8]				   \n"
                 //"				fldcw[ESP]				   \n"
                 //"				fistp dword ptr[ESP]	   \n"
                 //"				pop EAX					   \n"
                 //"				fldcw[ESP - 12]			   \n"
                 //"				ret						   \n"
                 //"		}								   \n"
                 //"	}									   \n"
                 //"										   \n"
                 //"	int __declspec(naked) floor(float x)   \n"
                 //"	{									   \n"
                 //"		__asm							   \n"
                 //"		{								   \n"
                 //"			fld dword ptr[ESP + 4]		   \n"
                 //"				push 0x73f				   \n"
                 //"				jmp rounding_common		   \n"
                 //"		}								   \n"
                 //"	}									   \n"
                 //"	}									   \n"			
                 "#include <math.h>\n"
                 "//double floor(double x) { return x - fmod(x, 1.0); }"
      );
    fclose( f );
  }

  //move proper resource file to minimal

  if ( Type == apEx_MakeExe_64k_CNS || Type == apEx_MakeExe_Demo_CNS )
  {
    CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\MinimalPlayer_CNS.rc" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\MinimalPlayer.rc" ) ).GetPointer(), false );
    CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\Setup_Dialog_CNS.bin" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\setupdialog.bin" ) ).GetPointer(), false );
  }

  if ( Type == apEx_MakeExe_64k_External || Type == apEx_MakeExe_Demo_External )
  {
    CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\MinimalPlayer_NoCNS.rc" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\MinimalPlayer.rc" ) ).GetPointer(), false );
    CopyFile( ( SourceRootPath + _T( "MinimalPlayer\\Data\\Setup_Dialog_Non_CNS.bin" ) ).GetPointer(), ( SourceRootPath + _T( "MinimalPlayer\\Data\\setupdialog.bin" ) ).GetPointer(), false );
  }

  //compile

  //start with the project data
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );

  SetCurrentDirectory( ( SourceRootPath + _T( "MinimalPlayer\\Data" ) ).GetPointer() );

  Execute( _T( "nasmw.exe" ), _T( "-f win32 release.asm -o release.obj" ) );

  SetCurrentDirectory( dir );

  CString TargetFile;

  //create minimal engine config
  CopyFile( ( SourceRootPath + _T( "Phoenix\\PhoenixConfig.h" ) ).GetPointer(), ( SourceRootPath + _T( "Phoenix\\PhoenixConfig.bak" ) ).GetPointer(), 0 );
  CreateMinimalConfig( ( SourceRootPath + _T( "Phoenix\\PhoenixConfig.h" ) ).GetPointer() );

  if ( Type == apEx_MakeExe_Demo_CNS || Type == apEx_MakeExe_Demo_External )
  {
    TargetFile = SourceRootPath + _T( "MinimalPlayer\\Release Demo\\MinimalPlayer.exe" );
    remove( TargetFile.GetPointer() );
    ExecuteWithConsoleLog( NULL, ( _T( "\"" ) + CompilerPath + _T( "\" " ) + SourcePath + _T( " /rebuild \"Release Demo\" /Project MinimalPlayer" ) ).GetPointer() );
  }

  if ( Type == apEx_MakeExe_64k_CNS || Type == apEx_MakeExe_64k_External )
  {
    TargetFile = SourceRootPath + _T( "MinimalPlayer\\Release 64k\\MinimalPlayer.exe" );
    CString TargetFileRenamed = SourceRootPath + _T( "MinimalPlayer\\Release 64k\\MinimalPlayer.unpacked.exe" );
    remove( TargetFile.GetPointer() );
    ExecuteWithConsoleLog( NULL, ( _T( "\"" ) + CompilerPath + _T( "\" " ) + SourcePath + _T( " /rebuild Release /Project MinimalPlayer" ) ).GetPointer() );
    CopyFile( TargetFile.GetPointer(), TargetFileRenamed.GetPointer(), false );
    
    //ExecuteWithConsoleLog(CompilerPath.GetPointer(), "/?");
    //kkrunch!

    extern TBOOL UseNewkkrunchy;

    if ( !UseNewkkrunchy )
      ExecuteWithConsoleLog( NULL, ( _T( "Tools/kkrunchy_023_2014_importhack.exe \"" ) + TargetFile + _T( "\"" ) + _T( " --best --refsize 64 --out " ) + _T( "\"" ) + TargetFile + _T( ".kkrunched\"" ) ).GetPointer() );
    else
      ExecuteWithConsoleLog( NULL, ( _T( "Tools/rekkrunchy.exe \"" ) + TargetFile + _T( "\"" ) + _T( " --best --refsize 64 --out " ) + _T( "\"" ) + TargetFile + _T( ".kkrunched\"" ) ).GetPointer() );
  }

  //undo minimal engine config changes
  CopyFile( ( SourceRootPath + _T( "Phoenix\\PhoenixConfig.bak" ) ).GetPointer(), ( SourceRootPath + _T( "Phoenix\\PhoenixConfig.h" ) ).GetPointer(), 0 );

  if ( mvxDefinesReplaced )
    CopyFile( ( SourceRootPath + _T( "..\\MVX\\Library\\mvxDefines.bak" ) ).GetPointer(), ( SourceRootPath + _T( "..\\MVX\\Library\\mvxDefines.h" ) ).GetPointer(), 0 );
  //move result

  CString resultFileName = TargetFile;
  if ( Type == apEx_MakeExe_64k_CNS || Type == apEx_MakeExe_64k_External )
    resultFileName = TargetFile + ".kkrunched";

  CStreamReaderMemory Result;
  if ( Result.Open( resultFileName.GetPointer() ) )
  {
    //file already read, let's clean up before we open the window
    ExecuteWithConsoleLog( NULL, ( _T( "\"" ) + CompilerPath + _T( "\" " ) + SourcePath + _T( " /rebuild Release /Project Phoenix" ) ).GetPointer() );

    TCHAR dir[ 1024 ];
    if ( !GetCurrentDirectory( 1024, dir ) )
      memset( dir, 0, sizeof( TCHAR ) * 1024 );
    char Filestring[ 256 ];

    OPENFILENAME opf;
    opf.hwndOwner = 0;
    opf.lpstrFilter = "Executables\0*.exe\0\0";
    opf.lpstrCustomFilter = 0;
    opf.nMaxCustFilter = 0L;
    opf.nFilterIndex = 1L;
    opf.lpstrFile = Filestring;
    opf.lpstrFile[ 0 ] = '\0';
    opf.nMaxFile = 256;
    opf.lpstrFileTitle = 0;
    opf.nMaxFileTitle = 50;
    opf.lpstrInitialDir = "Data";
    opf.lpstrTitle = "Make Exe";
    opf.nFileOffset = 0;
    opf.nFileExtension = 0;
    opf.lpstrDefExt = "exe";
    opf.lpfnHook = NULL;
    opf.lCustData = 0;
    opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
    opf.lStructSize = sizeof( OPENFILENAME );

    opf.hInstance = GetModuleHandle( 0 );
    opf.pvReserved = NULL;
    opf.dwReserved = 0;
    opf.FlagsEx = 0;

    opf.lpstrInitialDir = GetTargetDirectory( "saveexe" );
    if ( GetSaveFileName( &opf ) )
    {
      StoreCurrentDirectory( "saveexe" );
      SetCurrentDirectory( dir );
      CStreamWriterFile Output;
      if ( Output.Open( opf.lpstrFile ) )
      {
        Output.Write( Result.GetData(), (TU32)Result.GetLength() );
      }
      else
        LOG_ERR( "[apex] Error writing executable at target location" );
    }
    SetCurrentDirectory( dir );
  }
  else
    LOG_ERR( "[apex] Executable could not be opened! Compilation error?" );

  if ( ConsoleHidden ) ToggleConsole( !ConsoleHidden );
}

void CapexRoot::OpenHelpMenu()
{
  if ( App->FindItemByGuid( HelpMenuID ) ) return;

  CWBButton *HelpMenuButton = (CWBButton*)FindChildByID( "helpmenu", "button" );
  if ( !HelpMenuButton || HelpMenuButton->IsHidden() )
    return;

  CWBContextMenu *c = OpenContextMenu( HelpMenuButton->GetScreenRect().BottomLeft() );
  HelpMenuID = c->GetGuid();

  c->AddItem( "Open User's Guide", apEx_Help_UsersGuide );
  c->AddSeparator();

  c->AddItem( "Overview", apEx_Help_Overview );
  c->AddItem( "User Interface", apEx_Help_UserInterface );
  c->AddItem( "Texture Generation", apEx_Help_TextureGeneration );
  c->AddItem( "Modelling", apEx_Help_Modelling );
  c->AddItem( "Animation", apEx_Help_Animation );
  c->AddItem( "Video Editing", apEx_Help_VideoEditing );
  c->AddItem( "Creating a New Texture Filter", apEx_Help_CreateTextureFilter );
  c->AddItem( "Creating a New Material", apEx_Help_CreateMaterial );
  c->AddItem( "Creating a New Timeline Effect", apEx_Help_CreateTimelineEffect );
  c->AddItem( "Additional Features", apEx_Help_AdditionalFeatures );
  c->AddItem( "Creating a Release", apEx_Help_CreatingARelease );

  c->AddSeparator();
  c->AddItem( "Last resort (email)", apEx_Help_LastResort );
}

void CapexRoot::ImportMaterial()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Materials\0*.apxmat\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open Material";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "apxmat";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "importmaterial" );

  if ( GetOpenFileName( &opf ) )
  {
    StoreCurrentDirectory( "importmaterial" );
    SetCurrentDirectory( dir );

    ClearProjectData();
    Project.ImportMaterial( CString( opf.lpstrFile ), (HWND)App->GetHandle() );
    UpdateWindowData();
  }

  SetCurrentDirectory( dir );
}

void CapexRoot::MergeProject()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Project Files\0*.apx\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open Material";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "apx";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = GetTargetDirectory( "openproject" );

  if ( GetOpenFileName( &opf ) )
  {
    StoreCurrentDirectory( "openproject" );
    SetCurrentDirectory( dir );

    ClearProjectData();
    Project.MergeProject( CString( opf.lpstrFile ), (HWND)App->GetHandle() );
    UpdateWindowData();
  }

  SetCurrentDirectory( dir );
}

TCHAR *CapexRoot::GetTargetDirectory( CString key )
{
  if ( !Config::DirectoryList.HasKey( key ) ) return NULL;
  return Config::DirectoryList[ key ].GetPointer();
}

void CapexRoot::StoreCurrentDirectory( CString key )
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    return;
  Config::DirectoryList[ key ] = CString( dir );
}

CapexWorkBench * GetActiveWorkBench();

void CapexRoot::GoToTexture( CphxTextureOperator_Tool *Save, bool enableContextMenu )
{
  if ( !Save )
    return;

  SearchedTexture = Save->GetGUID();

  auto wb = ::GetActiveWorkBench();

  if ( wb->GetWindow( apEx_TexGenMain ) && !enableContextMenu )
  {
    wb->GoToTexture( Save );
    return;
  }

  TS32 cnt = 0;
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( WorkBenches[ x ]->GetWindow( apEx_TexGenMain ) )
      cnt++;

  if ( !cnt )
    return;

  if ( cnt == 1 || !enableContextMenu )
    for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
      if ( WorkBenches[ x ]->GetWindow( apEx_TexGenMain ) )
      {
        SelectWorkBench( WorkBenches[ x ] );
        WorkBenches[ x ]->GoToTexture( Save );
        return;
      }

  CWBContextMenu* ctx = OpenContextMenu( App->GetMousePos() );
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( WorkBenches[ x ]->GetWindow( apEx_TexGenMain ) )
      ctx->AddItem( WorkBenches[ x ]->GetName(), x + apEx_GoToTextureMenu );
}

void CapexRoot::GoToModel( CphxModel_Tool *model, bool enableContextMenu )
{
  if ( !model )
    return;

  SearchedModel = model->GetGUID();

  auto wb = ::GetActiveWorkBench();

  if ( wb->GetWindow( apEx_ModelView ) && !enableContextMenu )
  {
    wb->SetEditedModel( model );
    wb->UpdateWindows( apEx_ModelList );
    wb->UpdateWindows( apEx_ModelView );
    return;
  }

  TS32 cnt = 0;
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( WorkBenches[ x ]->GetWindow( apEx_ModelView ) )
      cnt++;

  if ( !cnt )
    return;

  if ( cnt == 1 || !enableContextMenu )
    for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
      if ( WorkBenches[ x ]->GetWindow( apEx_ModelView ) )
      {
        SelectWorkBench( WorkBenches[ x ] );
        WorkBenches[ x ]->SetEditedModel( model );
        WorkBenches[ x ]->UpdateWindows( apEx_ModelList );
        WorkBenches[ x ]->UpdateWindows( apEx_ModelView );
        return;
      }

  CWBContextMenu* ctx = OpenContextMenu( App->GetMousePos() );
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( WorkBenches[ x ]->GetWindow( apEx_ModelView ) )
      ctx->AddItem( WorkBenches[ x ]->GetName(), x + apEx_GoToModelMenu );
}

void CapexRoot::GoToScene( CphxScene_Tool* scene, bool enableContextMenu )
{
  if ( !scene )
    return;

  SearchedScene = scene->GetGUID();

  auto wb = ::GetActiveWorkBench();

  if ( wb->GetWindow( apEx_SceneView ) && !enableContextMenu )
  {
    wb->SetEditedScene( scene );
    wb->UpdateWindows( apEx_SceneList );
    return;
  }

  TS32 cnt = 0;
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( WorkBenches[ x ]->GetWindow( apEx_SceneView ) )
      cnt++;

  if ( !cnt )
    return;

  if ( cnt == 1 || !enableContextMenu )
    for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
      if ( WorkBenches[ x ]->GetWindow( apEx_SceneView ) )
      {
        SelectWorkBench( WorkBenches[ x ] );
        WorkBenches[ x ]->SetEditedScene( scene );
        WorkBenches[ x ]->UpdateWindows( apEx_SceneList );
        return;
      }

  CWBContextMenu* ctx = OpenContextMenu( App->GetMousePos() );
  for ( TS32 x = 0; x < WorkBenches.NumItems(); x++ )
    if ( WorkBenches[ x ]->GetWindow( apEx_SceneView ) )
      ctx->AddItem( WorkBenches[ x ]->GetName(), x + apEx_GoToSceneMenu );
}

CapexRoot *Root = NULL;

void UpdateWindowData( APEXWINDOW w )
{
  Root->UpdateWindowData( w );
}

void UpdateWindowData()
{
  Root->UpdateWindowData();
}

void SetStatusbarText( CString Text )
{
  Root->SetStatusbarText( Text );
}

void SetStatusbarError( CString Text )
{
  Root->SetStatusbarError( Text );
}

void SetTooltip( CString Text )
{
  Root->SetTooltip( Text );
}

TBOOL CapexTooltip::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
    if ( FadeoutStart < 0 )
    {
      FadeoutStart = globalTimer.GetTime();
      return true;
    }
    break;
  case WBM_REPOSITION:
    return CWBItem::MessageProc( Message );
  default:
    break;
  }
  return false;
}

CapexTooltip::CapexTooltip( CWBItem *Parent, const CRect &Position ) : CWBItem( Parent, Position )
{
  FadeoutStart = -1;
}

CapexTooltip::~CapexTooltip()
{

}

void CapexTooltip::OnDraw( CWBDrawAPI *API )
{
  CPoint p = App->GetMousePos() + CPoint( 16, 8 );
  CWBFont *f = App->GetDefaultFont();

  CString Text = Tooltip;

  for ( TS32 x = 0; x < 3; x++ )
    for ( TS32 y = 0; y < 3; y++ )
      WriteText( API, f, Tooltip, p + CPoint( x, y ), CColor::FromARGB( 0xff000000 ) );

  WriteText( API, f, Tooltip, p + CPoint( 1, 1 ), CColor::FromARGB( 0xffffffff ) );

  Tooltip = _T( "" ); //clear tooltip at end of frame

  //////////////////////////////////////////////////////////////////////////
  // Draw apex logo on startup

  TF32 Opacity = 1;

  if ( FadeoutStart >= 0 ) Opacity = min( 1, 1 - ( globalTimer.GetTime() - FadeoutStart ) / 1000.0f );
  if ( Opacity < 0 ) return;

  Opacity = 3 * Opacity*Opacity - 2 * Opacity*Opacity*Opacity;

  API->SetOpacity( (TS32)( Opacity * 255 ) );

  API->DrawRect( GetClientRect(), CColor::FromARGB( 0xa0202020 ) );
  App->GetSkin()->RenderElement( API, _T( "logowithborders" ), GetClientRect() );
  CWBFont *Font = App->GetFont( _T( "UniFont" ) );
  if ( !Font ) return;
  CString Creds[] = { _T( "apEx Demo Editor - (c) Barna 'BoyC' Buza 2012 - 2023" ),
    _T( "Build " ) + apexBuild,
    _T( "Build Date: " ) + buildDateTime };

  TS32 ypos = GetClientRect().Height() / 2 + 56;

  for ( TS32 x = 0; x < 3; x++ )
  {
    TS32 xpos = Font->GetCenterWidth( 0, GetClientRect().Width(), Creds[ x ], WBTT_UPPERCASE );
    Font->Write( API, Creds[ x ], CPoint( xpos, ypos ), CColor::FromARGB( 0xffa9a9a9 ), WBTT_UPPERCASE );
    ypos += Font->GetLineHeight();
  }

  App->SelectMouseCursor( CM_ARROW );
}

void CapexTooltip::WriteText( CWBDrawAPI *API, CWBFont *Font, CString &Text, CPoint Position, CColor Color )
{
  CString s = Text;
  TS32 l = Text.Length();
  TS32 lastlinestart = 0;
  TS32 ypos = 0;

  for ( TS32 x = 0; x < l; x++ )
  {
    if ( x == l - 1 || s[ x ] == _T( '\n' ) )
    {
      if ( s[ x ] == _T( '\n' ) ) s[ x ] = 0;
      Font->Write( API, s.GetPointer() + lastlinestart, Position + CPoint( 0, ypos ), Color, WBTT_UPPERCASE );
      lastlinestart = x + 1;
      ypos += Font->GetLineHeight();
    }
  }
}

TBOOL CapexTooltip::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return FadeoutStart >= 0;
}

TBOOL CapexConsoleInputLine::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
    switch ( Message.Key )
    {
    case VK_DOWN:
    case VK_UP:
      return false;
    case VK_LEFT:
    case VK_RIGHT:
      break;
    case VK_BACK:
      TabStringRoot = _T( "" );
      break;
    }
    break;
  case WBM_CHAR:
    if ( Message.Key == '\t' ) return false;

    //if (_istalnum(Message.Key))
    TabStringRoot = _T( "" );

    break;
  default:
    break;
  }

  return CWBTextBox::MessageProc( Message );
}

CWBItem * CapexConsoleInputLine::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new CapexConsoleInputLine( Root, Pos );
}

CapexConsoleInputLine::CapexConsoleInputLine( CWBItem *Parent, const CRect &Position ) : CWBTextBox( Parent, Position )
{

}

CapexConsoleInputLine::~CapexConsoleInputLine()
{

}