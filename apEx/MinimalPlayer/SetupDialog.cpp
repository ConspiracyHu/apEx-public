#include "SetupDialog.h"
#include "resource.h"
#include "MinimalPlayer.h"
#ifdef PHX_MINIMAL_BUILD
#include "..\Phoenix\PhoenixConfig.h"
#else
#include "..\Phoenix\PhoenixConfig_Full.h"
#endif

#include "../Phoenix/phxEngine.h"

SETUPCFG setupcfg;

#ifndef PHX_FAKE_FARBRAUSCH_INTRO_BUILD

//disabling this saves about 100 bytes on a minimal exe IF it works with kkrunchy - need to remove minimalplayer.rc then
//#define SETUPDIALOG_SOURCE_IS_RESOURCE

#define IDOK2                           3
#define IDD_DIALOG1                     101
#define IDD_SETUP                       101
#define IDI_ICON2                       103
#define IDC_RESOLUTION                  1001
#define IDC_FULLSCREEN                  1002
#define IDC_MUSIC                       1003
#define IDC_TEXDETAIL                   1004
#define IDC_ASPECTRATIO                 1004
#define IDC_ONTOP                       1005
#define IDC_VSYNC                       1006
#define IDC_ALTERNATEASPECT             1007
#define IDC_VSYNC2                      1007
#define IDC_ANTIALIAS                   1007
#define IDC_CINEMATICFPS                1007
#define IDC_TEXXXXT                     1008
#define IDC_CINEMATIC                   1009
#define IDC_REFRESHRATE                 1010
#define IDC_SHARE                       1011
#define IDC_WWW                         1012
#define IDC_FACEBOOK                    1013
#define IDC_POUET                       1014
#define IDC_SHARE2                      1015
#define IDC_YOUTUBE                     1015
#define IDC_RELEASENAME                 1016
#define IDC_GROUPNAME                   1017

#define AspectCnt 10
/*
static const char *AspectRatioNames[ AspectCnt ] =
{
  "4:3", "5:4", "16:9", "21:9",
  "1:1", "16:10", "3:2", "17:9", "64:27"
};
*/

static const char AspectRatiosX[] = { 4, 5, 16, 21, 1, 16, 3, 17, 64 };
static const char AspectRatiosY[] = { 3, 4, 9,   9, 1, 10, 2,  9, 27 };

static char s[ 30 ];

int GCD( int x, int y )
{
  if ( y == 0 )
    return x;
  else
    return GCD( y, x%y );
}

BOOL CALLBACK DlgFunc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
  case WM_LBUTTONDOWN:
    SendMessage( hWnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL );
    break;
  case WM_INITDIALOG:
  {
    //target size: 8180 -> crushed to 8360 :)

    DEVMODE dev;
    int ModeCount = 0;
    int Curridx = 0;
    unsigned int highestbpp = 0;
    unsigned int highestfrequency = 0;

    EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &setupcfg.mode );

    while ( EnumDisplaySettings( NULL, ModeCount, &dev ) )
    {
      if ( dev.dmDisplayFixedOutput == DMDFO_DEFAULT && dev.dmBitsPerPel >= 32 )
      {
        sprintf( s, "%d x %d %dbpp @ %dhz", dev.dmPelsWidth, dev.dmPelsHeight, dev.dmBitsPerPel, dev.dmDisplayFrequency );
        SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_SETITEMDATA, SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)s ), ModeCount );

        if ( dev.dmPelsWidth == setupcfg.mode.dmPelsWidth && dev.dmPelsHeight == setupcfg.mode.dmPelsHeight && dev.dmBitsPerPel >= highestbpp && dev.dmDisplayFrequency >= highestfrequency )
        {
          highestbpp = dev.dmBitsPerPel;
          highestfrequency = dev.dmDisplayFrequency;
          SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_SETCURSEL, Curridx, 0 );
        }

        Curridx++;
      }
      ModeCount++;
    }

    int gcd = GCD( setupcfg.mode.dmPelsWidth, setupcfg.mode.dmPelsHeight );
    int xaspect = setupcfg.mode.dmPelsWidth / gcd;
    int yaspect = setupcfg.mode.dmPelsHeight / gcd;

    setupcfg.HardwareAspectRatio = xaspect / (float)yaspect;

    sprintf( s, "%d:%d (auto)", xaspect, yaspect );

    SendDlgItemMessage( hWnd, IDC_ASPECTRATIO, CB_ADDSTRING, 0, (LPARAM)s );
    SendDlgItemMessage( hWnd, IDC_ASPECTRATIO, CB_SETCURSEL, 0, 0 );

    for ( int x = 1; x < AspectCnt; x++ )
    {
      sprintf( s, "%d:%d", AspectRatiosX[ x - 1 ], AspectRatiosY[ x - 1 ] );
      SendDlgItemMessage( hWnd, IDC_ASPECTRATIO, CB_ADDSTRING, 0, (LPARAM)s );
    }

    SendDlgItemMessage( hWnd, IDC_FULLSCREEN, BM_SETCHECK, 1, 1 );
    SendDlgItemMessage( hWnd, IDC_VSYNC, BM_SETCHECK, 1, 1 );

    SendDlgItemMessage( hWnd, IDC_GROUPNAME, WM_SETTEXT, 0, (LPARAM)setupcfg.GroupName );
    SendDlgItemMessage( hWnd, IDC_RELEASENAME, WM_SETTEXT, 0, (LPARAM)setupcfg.ReleaseName );
  }
  break;
  case WM_COMMAND:
    switch ( LOWORD( wParam ) )
    {
    case IDCANCEL:
    case IDOK:
    {
      EnumDisplaySettings( 0, SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_GETITEMDATA, SendDlgItemMessage( hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0 ), 0 ), &setupcfg.mode );

      int ar = SendDlgItemMessage( hWnd, IDC_ASPECTRATIO, CB_GETCURSEL, 0, 0 );
      if ( ar )
        setupcfg.HardwareAspectRatio = AspectRatiosX[ ar - 1 ] / (float)AspectRatiosY[ ar - 1 ];
      setupcfg.fullscreen = (int)SendDlgItemMessage( hWnd, IDC_FULLSCREEN, BM_GETCHECK, 0, 0 );
      setupcfg.vsync = (int)SendDlgItemMessage( hWnd, IDC_VSYNC, BM_GETCHECK, 0, 0 );
      EndDialog( hWnd, wParam == IDOK );
    }
    break;

#ifdef SETUPBOX_HAS_SOCIAL
    case IDC_SHARE: //this relies on these four ID-s being sequential in this exact order!
    case IDC_WWW:
    case IDC_FACEBOOK:
    case IDC_POUET:
    case IDC_YOUTUBE:
      ShellExecute( hWnd, "open", setupcfg.urls[ LOWORD( wParam ) - IDC_SHARE ], NULL, NULL, SW_SHOW );
      break;
#endif
    }
    break;
  }
  return ( WM_INITDIALOG == uMsg ) ? TRUE : FALSE;
}

#ifndef SETUPDIALOG_SOURCE_IS_RESOURCE
//to generate this data use the DlgResToDlgTemplate tool to extract it from an exe that contains the resource version
//used by the resourceless version

#ifdef SETUPBOX_HAS_SOCIAL
#include "Setup_Dialog_With_Social.h"
#else
#include "Setup_Dialog_Without_Social.h"
#endif

/*
#define BINARY_DECLARE(x) \
	extern "C" char x[];\
	extern "C" int x##_size;

BINARY_DECLARE( setupdlg );
*/

#endif

int OpenSetupDialog( char *Strings[ 7 ] )
{
  setupcfg.ReleaseName = Strings[ 1 ];
  setupcfg.GroupName = Strings[ 0 ];
#ifdef SETUPBOX_HAS_SOCIAL
  setupcfg.urls[ 0 ] = Strings[ 6 ]; //share url
  setupcfg.urls[ 1 ] = Strings[ 2 ]; //www
  setupcfg.urls[ 2 ] = Strings[ 4 ]; //facebook
  setupcfg.urls[ 3 ] = Strings[ 3 ]; //pouet
  setupcfg.urls[ 4 ] = Strings[ 5 ]; //youtube
#endif

#ifdef SETUPDIALOG_SOURCE_IS_RESOURCE
  return (int)DialogBox( 0, MAKEINTRESOURCE( IDD_SETUP ), 0, DlgFunc );
#else
  //resourceless version, if you use this you can remove the .rc from the project, might not work with kkrunchy tho
  return ::DialogBoxIndirect( 0, (LPCDLGTEMPLATE)setupdlg, 0, (DLGPROC)DlgFunc );
#endif
}

#endif