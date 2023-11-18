//////////////////////////////////////////////////////////////////////////
//
//	apEx - Another Programming Experiment
//
//	Editor tool for the Phoenix 64k engine
//
//	(c) Barna 'BoyC' Buza 2012 - 2023
//	www.conspiracy.hu
//
//////////////////////////////////////////////////////////////////////////

// NOTIFICATION ORANGE: #FFCC00

#include "BasePCH.h"
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"
#include "BuildInfo.h"
#include "Console.h"
#include "ProFont.h"
#include "apExRoot.h"
#include "resource.h"
#include <ShellAPI.h>

#include "..\Phoenix_Tool\apxPhoenix.h"

#include "ExtendedList.h"
#include "TextBox_HLSL.h"
#include "SplineEditor_Phx.h"
#include "TimelineEditor.h"
#include "TimelinePreview.h"
#include "ModelView.h"
#include "SceneView.h"
#include "VideoDumper.h"

#include "ConsoleCommands.h"
#include "NumPad.h"
#include "KKPViewer.h"

#include "Config.h"

extern CapexRoot *Root;

TBOOL InitGUI( CWBApplication *App )
{
  App->RegisterUIFactoryCallback( CString( _T( "textboxhlsl" ) ), CWBTextBox_HLSL::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "extendedlist" ) ), CapexList::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "splineeditorphx" ) ), CapexSplineEditor_phx::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "timelineeditorphx" ) ), CWBTimeline::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "demopreview" ) ), CWBDemoPreview::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "dumperpreview" ) ), CapexVideoDumperPreview::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "modeldisplay" ) ), CWBModelDisplay::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "scenedisplay" ) ), CWBSceneDisplay::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "console" ) ), CapexConsole::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "consoleinput" ) ), CapexConsoleInputLine::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "numpad" ) ), CWBNumPad::Factory );
  App->RegisterUIFactoryCallback( CString( _T( "kkpdisplay" ) ), CKKPDisplay::Factory );

  App->SetClearColor( CColor::FromARGB( 0xff0b0b0b ) );

  //CreateUniFont(App, "UniFontUpCase");
  //App->GetFont("UniFontUpCase")->ConvertToUppercase();

  CreateUniFont( App, "UniFont" );
  CreateProFont( App, "ProFont" );
  App->LoadSkinFromFile( _T( "Data/UI/Skin.wbs" ) );

  Config::Load();
  Root = new CapexRoot( App->GetRoot(), CRect( 0, 0, 0, 0 ) );
  Config::InitRoot( Root );

  InitializeConsoleCommands();

  return true;
}

void InitProject()
{
  Project.ImportLibraries();
  Root->UpdateWindowData( apEx_TexGenFilterEditor );
  Root->UpdateWindowData( apEx_MaterialEditor );
  Root->UpdateWindowData( apEx_RenderTargetEditor );
  Root->UpdateWindowData( apEx_TimelineEditor );
  Root->UpdateWindowData( apEx_TreeSpeciesEditor );
}

TBOOL CheckForUpdates()
{
  CSocket cns;
  if ( cns.Connect( "conspiracy.hu", 80 ) )
  {
    CString datastr = _T( "GET /apEx/version.txt HTTP/1.1\r\nHost: conspiracy.hu\r\nConnection: close\r\n\r\n" );
    cns.Write( datastr.GetPointer(), datastr.Length() );

    TS64 time = globalTimer.GetTime();

    while ( !cns.GetLength() )
      if ( globalTimer.GetTime() - time > 1000 ) break;

    TS32 len = (TS32)cns.GetLength();

    if ( len )
    {
      TU8 *data = new TU8[ len + 1 ];
      memset( data, 0, len + 1 );
      cns.ReadFull( data, len );
      CString versionreply = CString( (TS8*)data );
      CStringArray version = versionreply.Explode( _T( "\r\n\r\n" ) );

      if ( version.NumItems() > 1 )
      {
        TS32 uploadedbuild = 0;
        if ( version[ 1 ].Scan( _T( "%d" ), &uploadedbuild ) == 1 )
        {
          if ( uploadedbuild <= apexRelease )
          {
            LOG_NFO( "apEx is up to date." );
          }
          else
          {
            LOG_NFO( "A newer build of apEx is available." );
            CString Buildtext = CString::Format( _T( "A newer version of apEx is available.\nPress OK to download! (This will quit and open a browser.)" ) );
            if ( MessageBox( NULL, Buildtext.GetPointer(), "I'm feeling outdated", MB_OKCANCEL ) != IDCANCEL )
            {
              CString url = CString::Format( _T( "http://conspiracy.hu/private/boyc/apEx/apEx_%.4dr.zip" ), uploadedbuild );
              ShellExecute( NULL, "open", url.GetPointer(), NULL, NULL, SW_SHOW );
              return true;
            }
          }
        }
      }
      delete[] data;
    }

    cns.Close();
  }

  return false;
}

LONG WINAPI CrashOverride( struct _EXCEPTION_POINTERS * excpInfo )
{
  if ( IsDebuggerPresent() ) return EXCEPTION_CONTINUE_SEARCH;
  LONG res = baseCrashTracker( excpInfo );
  Project.DoCrashSave();
  return res;
}

void TestXMLLeak()
{
  if ( Root ) Root->ClearProjectData();

  Project.Import( CString( "Data/Projects/supermode_final.apx" ), NULL );

  if ( Root ) Root->UpdateWindowData();
  if ( Root ) Root->SelectFirstTextureObjectScene();

  CStringArray r = Project.LoadedFileName.Explode( _T( "\\" ) );
  if ( r.NumItems() )
    if ( Root ) Root->GetApplication()->SetWindowTitle( CString::Format( _T( "%s - apEx IntroTool" ), r.Last().GetPointer() ) );
}

void AutoLoadSong( HWND Winhandle )
{
  if ( Project.CanLoadSong() && Config::AutoloadMusic.Length() )
  {
    CStreamReaderFile f;
    if ( f.Open( Config::AutoloadMusic.GetPointer() ) )
      if ( f.GetLength() )
        Project.LoadSong( f, Winhandle );

  }
}

#include <ShellScalingAPI.h>
#pragma comment(lib,"Shcore.lib")

INT WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow )
{
  //SetProcessDpiAwareness( PROCESS_PER_MONITOR_DPI_AWARE );

  //CFPUAnsiCRounding roundmode;

  //initialize console
  Logger.AddOutput( new CapexConsoleLog() );
  Logger.AddOutput( new CLoggerOutput_File( _T( "apEx.log" ) ) );

  InitializeCrashTracker( apexBuild, CrashOverride );

  LOG_NFO( "apEx - Another Programming Experiment" );
  LOG_NFO( "Editor tool for the Phoenix 64k engine" );
  LOG_NFO( "Build %s - Build Date: %s", apexBuild.GetPointer(), buildDateTime.GetPointer() );
  LOG_NFO( "(c) Barna 'BoyC' Buza - Conspiracy 2012 - 2023" );
  LOG_NFO( "www.conspiracy.hu" );
  LOG_NFO( "" );

  InitWinsock();

  if ( !strstr( lpCmdLine, _T( "-neupdateldaklienstbazmeg" ) ) )
  {
#ifndef _DEBUG
    if ( CheckForUpdates() )
    {
      DeinitWinsock();
      return 0;
    }
#endif // _DEBUG
  }

  //initialize application
  CWBApplication *App = new CWBApplication();

  if ( !App->Initialize( CCoreWindowParameters( hInstance, false, 800, 600, "apEx IntroTool", LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) ), true ) ) )
  {
    SAFEDELETE( App );
    return false;
  }

  if ( !InitGUI( App ) )
  {
    SAFEDELETE( App );
    return false;
  }

  CphxSound::Init( (HWND)App->GetHandle() );

#ifdef MEMORY_TRACKING
  LOG_NFO( "[memory] Current alloc count: %d Total alloc count: %d", CurrentAllocCount, TotalAllocCount );
#endif

  App->SetScreenshotName( _T( "apEx" ) );

  InitDefaultStates( App->GetDevice() );

  CCoreDX11Device *d = (CCoreDX11Device*)App->GetDevice();
  PhoenixInitTool( d->GetDevice(), d->GetDeviceContext() );

  InitProject();

  App->GetDevice()->ForceStateReset();

  App->GetDevice()->InitializeDebugAPI();
  App->SetVSync( Config::VSync );

  AutoLoadSong( (HWND)App->GetHandle() );

  //main loop

  while ( !App->IsDone() )
  {
    while ( App->HandleMessages() )
    {
      if ( App->DeviceOK() )
      {
        if ( !App->IsMinimized() )
        {
          ProcessGeneratorQueue( 40, App->GetDevice() );
          App->Display();
        }

        //TestXMLLeak();

        Project.DoAutosave();
      }
      else Sleep( 10 );
    }

    if ( Config::ConfirmExit )
      if ( MessageBox( (HWND)App->GetHandle(), _T( "Are you sure you want to exit?" ), _T( "Please confirm" ), MB_OKCANCEL ) == IDCANCEL )
        App->SetDone( false );
  }

  DeinitDefaultStates();

  App->Destroy();

  Config::Save( Root );

  Project.DoAutosave( 30 );
  Project.Reset();

  CphxSound::Deinit();

  SAFEDELETE( DefaultTech );
  SAFEDELETE( DefaultMaterial );

  //cleanup
  DeinitWinsock();
  DeinitializeModelView();
  PhoenixDeinitTool();

  delete App;

  return true;
}
