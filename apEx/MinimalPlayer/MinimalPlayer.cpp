//#define MUSIC_IS_OGG

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define VC_EXTRALEAN
#include "SetupDialog.h"
#include "resource.h"
#include "..\LibCTiny\libcminimal.h"

#ifdef MUSIC_IS_OGG
#include "miniz.c"
#endif

//////////////////////////////////////////////////////////////////////////
//
//	Conspiracy Phoenix 64k engine
//
//	(c) Barna 'BoyC' Buza 2010-2023
//	http://www.conspiracy.hu
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
//	Music player settings can be configured via the Configuration Manager
//	Change the release build of the MinimalPlayer project between
//	Release 64k and Release Demo to switch between MVX/OGG
//
//////////////////////////////////////////////////////////////////////////

// CODE COVERAGE! http://msdn.microsoft.com/en-us/library/dd299398(v=vs.90).aspx

#include "MinimalPlayer.h"

#include "../Phoenix/phxEngine.h"
#include "../Phoenix/Project.h"
#include "../Phoenix/phxSpline.h"
#include "../MVX/mvx.h"

#include "MusicPlayer.h"

#include "WindowHandler.h"

#define BINARY_DECLARE(x) \
	extern "C" char x[];\
	extern "C" int x##_size;

BINARY_DECLARE( music );
BINARY_DECLARE( demo );
BINARY_DECLARE( precalc );


#define MSVCRT__EM_DENORMAL   0x00080000

//void testlibctiny()
//{
//	float f=acos(0.5f); // 1.04720
//	f = cos(0.5f); // 0.87
//	f = sin(0.5f); // 0.47
//	f = sqrt(0.5f); // 0.70710
//	f = atan2(0.5f,3.0f); // 0.165149...
//	f = atan(0.5f); // 0.463648...
//	f = pow(0.5f,3.0f); // 0.125
//	f = fmod(3.5f,2.0f); // 1.5
//	f=fabs(-0.5f); // 0.5
//	f=abs(-0.5f);
//	int k = 0.25;
//
//	int z = 0;
//}

//#ifdef MUSIC_IS_OGG
static char *Libraries[] =
{
  "d3d11.dll",
  "d3dx10_43.dll",
  "d3dx11_43.dll",
  "dsound.dll",
  "msacm32.dll",
  "glu32.dll",
  "d3dcompiler_47.dll"
};
//#endif

INT WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in INT nCmdShow )
{
  OPENDEBUGFILE();
  DEBUGLOG( "WinMain init." );

  DEBUGLOG( "Test 0" );
  DEBUGLOG( "Test %d", 1337 );
  DEBUGLOG( "Test2 %s", "test test" );

  //testlibctiny();

  //our kkrunchy build ignores loadlibrary errors so we can exit gracefully:
//#ifdef MUSIC_IS_OGG
  for ( int x = 0; x < 7; x++ )
  {
    DEBUGLOG( "Loading Library %s", Libraries[ x ] );

    if ( !LoadLibrary( Libraries[ x ] ) )
    {
      char meh[ 100 ];
      int y = 0;
      while ( Libraries[ x ][ y ] )
        meh[ y++ ] = Libraries[ x ][ y ];
      memcpy( meh + y, " failed to load\0", 17 );
      MessageBoxA( NULL, (LPCSTR)meh, NULL, MB_ICONERROR );
      return 0;
    }
    DEBUGLOG( "Library %s Loaded ok", Libraries[ x ] );
  }
  //#endif

  unsigned char *DemoData = (unsigned char*)demo;

#ifdef MUSIC_IS_OGG
  int datLen = ( (int*)DemoData )[ 0 ];
  int datLenunp = ( (int*)DemoData )[ 1 ];

  unsigned char* uncmp = new unsigned char[ datLenunp ];
  mz_ulong uncmpsiz = datLenunp;

  //uncompress( uncmp, &uncmpsiz, DemoData + 8, datLen );
  //DemoData = DemoData + 8;
#endif

  char *Strings[ 7 ];

  static CphxProject Precalc;
  static CphxProject Demo;

  DEBUGLOG( "Parsing Demo Data" );
  for ( int x = 0; x < 7; x++ )
  {
    Strings[ x ] = (char*)DemoData;
    while ( *( DemoData++ ) );
  }

  //#define HIGHFRAMERATECOMPO 1

#ifndef HIGHFRAMERATECOMPO
#ifndef PHX_FAKE_FARBRAUSCH_INTRO_BUILD
#ifndef _DEBUG
  DEBUGLOG( "Opening Setup Window" );
  if ( OpenSetupDialog( Strings ) )
  {
#else
    {
      setupcfg.mode.dmPelsWidth = 800;
      setupcfg.mode.dmPelsHeight = 450;
      setupcfg.mode.dmDisplayFrequency = 0;
      setupcfg.HardwareAspectRatio = 16 / 9.0f;
      setupcfg.vsync = true;
      setupcfg.fullscreen = false;
#endif
#else
      {
        setupcfg.mode.dmPelsWidth = 1920;
        setupcfg.mode.dmPelsHeight = 1080;
        setupcfg.mode.dmDisplayFrequency = 0;
        setupcfg.HardwareAspectRatio = 16 / 9.0f;
        setupcfg.vsync = true;
        setupcfg.fullscreen = true;
#endif
#else
        {
          bool vsync = true;

          LPSTR cmd = GetCommandLine();
          int xres = 1920;
          int yres = 1080;

          bool warningdialogs = true;

          if ( strstr( cmd, "nowarnings" ) != NULL )
            warningdialogs = false;

          if ( strstr( cmd, "720p" ) != NULL )
          {
            xres = 1280;
            yres = 720;
          }

          DEVMODE current;
          EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &current );

          if ( strstr( cmd, "current" ) != NULL )
          {
            xres = current.dmPelsWidth;
            yres = current.dmPelsHeight;
          }

          if ( strstr( cmd, "novsync" ) != NULL )
            vsync = false;

          IDXGIFactory* factory;
          IDXGIAdapter* adapter;
          IDXGIOutput* adapterOutput;
          unsigned int numModes;
          DXGI_MODE_DESC* displayModeList;

          auto result = CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**)&factory );
          if ( FAILED( result ) ) { if ( warningdialogs ) MessageBox( NULL, "Error during mode enumeration", NULL, MB_ICONERROR ); return 0; }
          result = factory->EnumAdapters( 0, &adapter );
          if ( FAILED( result ) ) { if ( warningdialogs ) MessageBox( NULL, "Error during mode enumeration", NULL, MB_ICONERROR ); return 0; }
          result = adapter->EnumOutputs( 0, &adapterOutput );
          if ( FAILED( result ) ) { if ( warningdialogs ) MessageBox( NULL, "Error during mode enumeration", NULL, MB_ICONERROR ); return 0; }
          result = adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL );
          if ( FAILED( result ) ) { if ( warningdialogs ) MessageBox( NULL, "Error during mode enumeration", NULL, MB_ICONERROR ); return 0; }
          displayModeList = new DXGI_MODE_DESC[ numModes ];
          result = adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList );
          if ( FAILED( result ) ) { if ( warningdialogs ) MessageBox( NULL, "Error during mode enumeration", NULL, MB_ICONERROR ); return 0; }

          DEVMODE selected = current;
          selected.dmPelsWidth = 0;
          selected.dmPelsHeight = 0;

          for ( unsigned int i = 0; i < numModes; i++ )
          {
            if ( displayModeList[ i ].Width == (unsigned int)xres )
            {
              if ( displayModeList[ i ].Height == (unsigned int)yres )
              {
                float displayfreq = displayModeList[ i ].RefreshRate.Numerator / (float)displayModeList[ i ].RefreshRate.Denominator;
                if ( abs( displayfreq - 120 ) < 1 || displayfreq >= selected.dmDisplayFrequency )
                {
                  selected.dmPelsWidth = xres;
                  selected.dmPelsHeight = yres;
                  selected.dmDisplayFrequency = displayModeList[ i ].RefreshRate.Numerator;
                  SetDenominator( displayModeList[ i ].RefreshRate.Denominator );
                  if ( abs( displayfreq - 120 ) < 1 ) break;
                }
              }
            }
          }

          if ( selected.dmPelsWidth != xres || selected.dmPelsHeight != yres )
          {
            if ( warningdialogs ) MessageBoxA( NULL, "Requested resolution not supported by device.\nAborting.", NULL, MB_ICONERROR );
            return 0;
          }

          if ( selected.dmDisplayFrequency != 120 )
            if ( warningdialogs ) MessageBoxA( NULL, "120hz not supported by display device.\nRunning on lower frequency.", NULL, MB_ICONWARNING );

          setupcfg.vsync = vsync;
          setupcfg.mode = selected;
          setupcfg.HardwareAspectRatio = current.dmPelsWidth / (float)current.dmPelsHeight;
          setupcfg.fullscreen = true;

#endif

          DEBUGLOG( "Initializing Window: Fullscreen (%d) Width (%d) Height (%d) Frequency (%d)", setupcfg.fullscreen, setupcfg.mode.dmPelsWidth, setupcfg.mode.dmPelsHeight, setupcfg.mode.dmDisplayFrequency );

          InitWindow( hInstance, setupcfg.mode.dmPelsWidth, setupcfg.mode.dmPelsHeight, setupcfg.mode.dmDisplayFrequency, setupcfg.fullscreen != 0, LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) ), 0 );

          DEBUGLOG( "Initializing Phoenix" );

          InitializePhoenix();

          DEBUGLOG( "--------------------------------- LOADING PRECALC PROJECT" );
          Precalc.LoadProject( (unsigned char*)precalc + 7, NULL, setupcfg );
          DEBUGLOG( "--------------------------------- LOADING DEMO PROJECT" );
          Demo.LoadProject( DemoData, &Precalc, setupcfg );

          DEBUGLOG( "Starting demo" );
          if ( !phxDone && MUSIC_INIT( hWnd, music, music_size ) )
          {
            DEBUGLOG( "Initializing Music" );
            MUSIC_PLAY();

            //int tme = timeGetTime();

            //main loop
            while ( !phxDone )
            {
              MSG msg;
              while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != 0 )
              {
                //TranslateMessage(&msg);
                DispatchMessage( &msg );
              }

              //render demo here

              DEBUGLOG( "Rendering Frame %d", MUSIC_GETSYNC() );
              Demo.Render( MUSIC_GETSYNC() );
              SwapChain->Present( setupcfg.vsync ? 1 : 0, 0 );

            }

            DEBUGLOG( "Stopping Music" );
            MUSIC_STOP();
            DEBUGLOG( "Sound Deinit" );
            MUSIC_DEINIT();
          }

          if ( setupcfg.fullscreen )
          {
            DEBUGLOG( "Switching fullscreen mode back" );
            SwapChain->SetFullscreenState( FALSE, NULL ); //at least return from fullscreen mode properly
          }

          //phxContext->Release();
          //phxDev->Release();
          //DestroyWindow(hWnd);

  }

        DEBUGLOG( "Drop everything" );

        return 0;
}