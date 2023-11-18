#include "BasePCH.h"
#include "Config.h"
#include "apExRoot.h"

namespace Config
{

TBOOL ConfirmExit = true;
TBOOL VSync = true;
TBOOL RightDoubleClickCameraReset = true;
int rendercount;
CString AutoloadMusic;
CString VideoDumpPath = _T( "VideoDump\\" );
TS32 TexgenMemPoolSize;
CString skinCSS = _T( "Default" );
TS32 MonitorAspectX = GetSystemMetrics( SM_CXSCREEN );
TS32 MonitorAspectY = GetSystemMetrics( SM_CYSCREEN );
TF32 UbertoolSnap = 0.5f;
TBOOL noConfig = false;

CArray<CString> RecentFiles;
CDictionary<CString, CString> DirectoryList;

void Load()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "apEx.cfg" ) )
  {
    noConfig = true;
    LOG_NFO( "[apEx] Config not found, using default settings." );
    return;
  }

  if ( !d.GetDocumentNode().GetChildCount( _T( "config" ) ) )
  {
    noConfig = true;
    LOG_ERR( "[apEx] config field missing from configuration file." );
    return;
  }

  CXMLNode n = d.GetDocumentNode().GetChild( _T( "config" ) );

  if ( n.GetChildCount( _T( "monitoraspectx" ) ) )
    n.GetChild( _T( "monitoraspectx" ) ).GetValue( Config::MonitorAspectX );
  else Config::MonitorAspectX = GetSystemMetrics( SM_CXSCREEN );

  if ( n.GetChildCount( _T( "monitoraspecty" ) ) )
    n.GetChild( _T( "monitoraspecty" ) ).GetValue( Config::MonitorAspectY );
  else Config::MonitorAspectY = GetSystemMetrics( SM_CYSCREEN );

  if ( n.GetChildCount( _T( "confirmexit" ) ) )
    n.GetChild( _T( "confirmexit" ) ).GetValue( Config::ConfirmExit );

  if ( n.GetChildCount( _T( "vsync" ) ) )
    n.GetChild( _T( "vsync" ) ).GetValue( Config::VSync );

  if ( n.GetChildCount( _T( "rightdoubleclickcamerareset" ) ) )
    n.GetChild( _T( "rightdoubleclickcamerareset" ) ).GetValue( Config::RightDoubleClickCameraReset );

  if ( n.GetChildCount( _T( "skincss" ) ) )
    Config::skinCSS = n.GetChild( _T( "skincss" ) ).GetText();

  if ( n.GetChildCount( _T( "snapvalue" ) ) )
    n.GetChild( _T( "snapvalue" ) ).GetValue( UbertoolSnap );

  if ( n.GetChildCount( _T( "musicautoload" ) ) )
    Config::AutoloadMusic = n.GetChild( _T( "musicautoload" ) ).GetText();

  if ( n.GetChildCount( _T( "videodumppath" ) ) )
    Config::VideoDumpPath = n.GetChild( _T( "videodumppath" ) ).GetText();

  for ( TS32 x = 0; x < n.GetChildCount( _T( "recentfile" ) ); x++ )
    Config::RecentFiles += n.GetChild( _T( "recentfile" ), x ).GetText();

  if ( n.GetChildCount( _T( "texgenmempool" ) ) )
    n.GetChild( _T( "texgenmempool" ) ).GetValue( Config::TexgenMemPoolSize );
  else Config::TexgenMemPoolSize = 512;

  for ( TS32 x = 0; x < n.GetChildCount( _T( "directory" ) ); x++ )
  {
    auto node = n.GetChild( _T( "directory" ), x );
    if ( node.GetChildCount( _T( "id" ) ) && node.GetChildCount( _T( "location" ) ) )
    {
      CString key = node.GetChild( _T( "id" ) ).GetText();
      CString data = node.GetChild( _T( "location" ) ).GetText();

      TU8 *dataout = NULL;
      TS32 size = 0;
      data.DecodeBase64( dataout, size );

      if ( dataout )
      {
        CString s = CString( (TS8*)dataout, size );
        SAFEDELETE( dataout );
        Config::DirectoryList[ key ] = s;
      }
      else
        Config::DirectoryList[ key ] = data;
    }
  }
}

void InitRoot( CapexRoot* root )
{
  root->SetUbertoolSnap( UbertoolSnap );
  if ( noConfig )
  {
    root->LoadDefaultLayout();
    return;
  }

  CXMLDocument d;
  if ( !d.LoadFromFile( "apEx.cfg" ) )
    return;

  if ( !d.GetDocumentNode().GetChildCount( _T( "config" ) ) )
    return;

  CXMLNode n = d.GetDocumentNode().GetChild( _T( "config" ) );
  if ( !n.GetChildCount( _T( "guilayout" ) ) )
  {
    LOG_ERR( "[apEx] guilayout field missing from configuration file." );
    return;
  }

  CXMLNode l = n.GetChild( _T( "guilayout" ) );

  TS32 swb = 0;
  if ( l.HasAttribute( _T( "selectedworkbench" ) ) ) l.GetAttributeAsInteger( _T( "selectedworkbench" ), &swb );

  for ( TS32 x = 0; x < l.GetChildCount( _T( "workbench" ) ); x++ )
  {
    CXMLNode w = l.GetChild( _T( "workbench" ), x );
    CString name = _T( "missing name" );
    if ( w.HasAttribute( _T( "Name" ) ) )
      name = w.GetAttributeAsString( _T( "Name" ) );
    CapexWorkBench *wb = Root->AddWorkBench( name.GetPointer() );
    wb->ImportLayout( &w );

    if ( swb == x )
      Root->SelectWorkBench( wb );
  }
}

void Save( CapexRoot* root )
{
  CXMLDocument d;
  CXMLNode cfg = d.GetDocumentNode().AddChild( _T( "config" ) );
  cfg.AddChild( _T( "confirmexit" ) ).SetInt( Config::ConfirmExit );
  cfg.AddChild( _T( "vsync" ) ).SetInt( Config::VSync );
  cfg.AddChild( _T( "rightdoubleclickcamerareset" ) ).SetInt( Config::RightDoubleClickCameraReset );
  cfg.AddChild( _T( "skincss" ) ).SetText( Config::skinCSS );
  cfg.AddChild( _T( "snapvalue" ) ).SetFloat( root->GetUbertoolSnap() );
  cfg.AddChild( _T( "monitoraspectx" ) ).SetInt( Config::MonitorAspectX );
  cfg.AddChild( _T( "monitoraspecty" ) ).SetInt( Config::MonitorAspectY );
  cfg.AddChild( _T( "musicautoload" ) ).SetText( Config::AutoloadMusic );
  cfg.AddChild( _T( "videodumppath" ) ).SetText( Config::VideoDumpPath );
  CXMLNode layout = cfg.AddChild( _T( "guilayout" ) );
  layout.SetAttributeFromInteger( _T( "selectedworkbench" ), root->GetSelectedWorkBench() );
  for ( TS32 x = 0; x < root->GetWorkBenchCount(); x++ )
  {
    CapexWorkBench *w = root->GetWorkBench( x );
    CXMLNode wb = layout.AddChild( _T( "workbench" ) );
    w->ExportLayout( &wb );
  }

  for ( TS32 x = max( 0, Config::RecentFiles.NumItems() - 10 ); x < Config::RecentFiles.NumItems(); x++ )
    cfg.AddChild( _T( "recentfile" ) ).SetText( Config::RecentFiles[ x ].GetPointer() );

  cfg.AddChild( _T( "texgenmempool" ) ).SetInt( Config::TexgenMemPoolSize );

  for ( TS32 x = 0; x < Config::DirectoryList.NumItems(); x++ )
  {
    CXMLNode n = cfg.AddChild( _T( "directory" ) );
    auto *kdp = Config::DirectoryList.GetKDPair( x );
    n.AddChild( _T( "id" ) ).SetText( kdp->Key );

    CString loc = CString::EncodeToBase64( (TU8*)kdp->Data.GetPointer(), kdp->Data.Length() );

    n.AddChild( _T( "location" ) ).SetText( loc );
  }

  d.SaveToFile( _T( "apEx.cfg" ) );
}
}
