#include "BasePCH.h"
#include "VideoDumper.h"
#include "../Phoenix/Timeline.h"
#include "apExRoot.h"
#define WINDOWNAME _T("VideoDumper")
#define WINDOWXML _T("VideoDumper")
#include "../Phoenix_Tool/Timeline_tool.h"
#include "../Phoenix_Tool/apxProject.h"
#include "TimelineEditor.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include "Config.h"

//#include <windows.h>
//#include <shlobj.h>
//#include <iostream>
//#include <sstream>

extern CapexRoot *Root;

extern CCoreRasterizerState *RenderRasterState;
extern CCoreSamplerState *RenderSamplerState;
extern CCoreDepthStencilState *RenderDepthState_Normal;
extern CCoreBlendState *DisableBlend;

TS32 FixesStepTimePosition = 0;
DWORD __stdcall FixedStepTimer()
{
  return FixesStepTimePosition;
}

void CapexVideoDumperPreview::OnDraw( CWBDrawAPI *API )
{
  //RenderTargets->UpdateCollection(GetClientRect().Width(), GetClientRect().Height(), true);
  API->DrawRect( GetClientRect(), CColor::FromARGB( 0xff000000 ) );
  Project.ApplyRenderTargets( RenderTargets );

  TS32 tax = Project.Timeline->Timeline->AspectX;
  TS32 tay = Project.Timeline->Timeline->AspectY;

  TS32 max = Config::MonitorAspectX;
  TS32 may = Config::MonitorAspectY;

  TS32 sax = GetSystemMetrics( SM_CXSCREEN );
  TS32 say = GetSystemMetrics( SM_CYSCREEN );

  TF32 aspect = ( tax*max*say ) / (TF32)( tay*may*sax );
  //TF32 aspect=tax/(TF32)tay*(max/(TF32)sax)/(may/(TF32)say);

  int xs = GetClientRect().Width();
  int ys = GetClientRect().Height();
  if ( aspect > xs / (TF32)ys )
    ys = (TS32)( xs / aspect );
  else xs = (TS32)( ys*aspect );

  int x1 = ( GetClientRect().Width() - xs ) / 2;
  int y1 = ( GetClientRect().Height() - ys ) / 2;

  CRect DisplayRect = CRect( x1, y1, x1 + xs, y1 + ys );

  API->FlushDrawBuffer();
  API->SetRenderView( ClientToScreen( GetClientRect() ) );

  RenderDepthState_Normal->Apply();
  RenderRasterState->Apply();
  RenderSamplerState->Apply( CORESMP_PS0 );
  RenderSamplerState->Apply( CORESMP_VS0 );
  //App->GetDevice()->SetRenderState(RenderDepthState_Normal);
  //App->GetDevice()->SetRenderState(RenderRasterState);
  //App->GetDevice()->SetSamplerState(CORESMP_PS0, RenderSamplerState);
  //App->GetDevice()->SetSamplerState(CORESMP_VS0, RenderSamplerState);

  TS32 frame = Project.GetFrameToRender();
  //Project.Timeline->Render((TF32)frame);

  API->GetDevice()->ForceStateReset();
  API->SetUIRenderState();

  //for (TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++)
  //{
  //	CphxEvent_Tool *e = Project.Timeline->Events[x];
  //	if (e->Event->StartFrame <= frame && e->Event->EndFrame >= frame)
  //		e->RequestContent();
  //}

  if ( Project.Timeline->Timeline->Target )
  {
    CCoreDX11Texture2D *dummy = new CCoreDX11Texture2D( (CCoreDX11Device*)API->GetDevice() );
    dummy->SetView( Project.Timeline->Timeline->Target->View );

    API->FlushDrawBuffer();
    API->GetDevice()->SetRenderState( DisableBlend );
    API->GetDevice()->SetTexture( CORESMP_PS0, dummy );
    API->DrawRect( DisplayRect, 0, 0, 1, 1 );

    API->FlushDrawBuffer();
    API->SetUIRenderState();

    dummy->SetTextureHandle( NULL );
    dummy->SetView( NULL );
    SAFEDELETE( dummy );
  }
  //else
  //	API->DrawRectBorder(DisplayRect, CColor::FromARGB(0xff808080));

}

TBOOL CapexVideoDumperPreview::MessageProc( CWBMessage &Message )
{
  return CWBItem::MessageProc( Message );
}

CapexVideoDumperPreview::CapexVideoDumperPreview( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  RenderTargets = Project.SpawnRenderTargetCollection( App->GetDevice(), Pos.Width(), Pos.Height(), true );
}

CapexVideoDumperPreview::~CapexVideoDumperPreview()
{
  SAFEDELETE( RenderTargets );
}

CWBItem * CapexVideoDumperPreview::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new CapexVideoDumperPreview( Root, Pos );
}

void CapexVideoDumperPreview::PrepareFrame()
{
  Project.ApplyRenderTargets( RenderTargets );
  RenderDepthState_Normal->Apply();
  RenderRasterState->Apply();
  RenderSamplerState->Apply( CORESMP_PS0 );
  RenderSamplerState->Apply( CORESMP_VS0 );
  TS32 frame = Project.GetFrameToRender();
  Project.Timeline->Render( (TF32)frame );
}

CapexVideoDumper::CapexVideoDumper() : CWBItem()
{
}

//static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
//{
//
//	if (uMsg == BFFM_INITIALIZED)
//	{
//		//std::string tmp = (const char *)lpData;
//		//std::cout << "path: " << tmp << std::endl;
//		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
//	}
//
//	return 0;
//}
//
//CString BrowseFolder(CString saved_path)
//{
//	TCHAR path[MAX_PATH];
//
//	const char * path_param = saved_path.GetPointer();
//
//	BROWSEINFO bi = { 0 };
//	bi.lpszTitle = ("Browse for folder...");
//	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
//	bi.lpfn = BrowseCallbackProc;
//	bi.lParam = (LPARAM)path_param;
//
//	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
//
//	if (pidl != 0)
//	{
//		//get the name of the folder and put it in path
//		SHGetPathFromIDList(pidl, path);
//
//		//free memory used
//		IMalloc * imalloc = 0;
//		if (SUCCEEDED(SHGetMalloc(&imalloc)))
//		{
//			imalloc->Free(pidl);
//			imalloc->Release();
//		}
//
//		return path;
//	}
//
//	return "";
//}

#include "../Phoenix/Timeline.h"
#include "TimelinePreview.h"

CapexVideoDumper::CapexVideoDumper( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  CRect p = Pos;
  CRect cp = CRect( 0, 0, 640, 480 ) + p.Center() - CPoint( 320, 240 );
  //ApplyStyleDeclarations(_T("margin:0px;"));

  CenterBox = new CWBBox( this, cp );
  CenterBox->ApplyStyleDeclarations( _T( "border:3px;background:#252527;" ) );

  //don't use reloadlayout here as it calls the virtual updatedata()!
  CString xmlname = CString( _T( "Data/UI/" ) ) + WINDOWXML;
  App->LoadXMLLayoutFromFile( xmlname + ".xml" );
  App->LoadCSSFromFile( Root->GetCSSPath() + _T( "apEx.css" ) );
  App->LoadCSSFromFile( Root->GetCSSPath() + WINDOWXML + ".css", false );
  App->GenerateGUI( CenterBox, WINDOWXML );
  App->LoadCSSFromFile( Root->GetCSSPath() + _T( "apEx.css" ) );

  CWBTextBox *t = (CWBTextBox *)FindChildByID( _T( "Path" ), _T( "textbox" ) );
  if ( t ) t->SetText( Config::VideoDumpPath );

  CWBButton *b = (CWBButton*)FindChildByID( _T( "ChangePath" ), _T( "button" ) );
  if ( b ) b->Hide( true );

  XRes = GetSystemMetrics( SM_CXSCREEN );
  YRes = GetSystemMetrics( SM_CYSCREEN );
  StartFrame = 0;
  EndFrame = -1;
  Framerate = 25;

  TS32 SmallestEndEvent = -1;

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    if ( Project.Timeline->Events[ x ]->Type == EVENT_ENDDEMO )
    {
      TS32 t = Project.Timeline->Events[ x ]->Event->StartFrame;
      if ( SmallestEndEvent == -1 ) SmallestEndEvent = t;
      SmallestEndEvent = min( SmallestEndEvent, t );
    }

  if ( SmallestEndEvent != -1 ) EndFrame = SmallestEndEvent;
  else //no end demo event was found
  {
    for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
      if ( Project.Timeline->Events[ x ]->Type == EVENT_ENDDEMO )
        EndFrame = max( EndFrame, Project.Timeline->Events[ x ]->Event->EndFrame );
  }

  t = (CWBTextBox *)FindChildByID( _T( "XRes" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), XRes ) );
  t = (CWBTextBox *)FindChildByID( _T( "YRes" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), YRes ) );
  t = (CWBTextBox *)FindChildByID( _T( "Framerate" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%g" ), Framerate ) );
  t = (CWBTextBox *)FindChildByID( _T( "StartFrame" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), StartFrame ) );
  t = (CWBTextBox *)FindChildByID( _T( "EndFrame" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), EndFrame ) );

  CapexVideoDumperPreview *d = (CapexVideoDumperPreview*)FindChildByID( _T( "DisplayBox" ), _T( "dumperpreview" ) );
  if ( d ) d->RenderTargets->UpdateCollection( XRes, YRes, true );
}

CapexVideoDumper::~CapexVideoDumper()
{

}

void CapexVideoDumper::UpdateData()
{

}

void CapexVideoDumper::OnDraw( CWBDrawAPI *API )
{
  API->DrawRect( GetClientRect(), CColor::FromARGB( 0x80ffffff ) );
  if ( !CenterBox ) return;

  CRect cp = CenterBox->GetPosition();
  CWBFont *f = App->GetDefaultFont();

  CWBLabel *progresstext = (CWBLabel*)FindChildByID( _T( "Progress" ), _T( "label" ) );

  if ( !Recording )
  {
    if ( progresstext ) progresstext->SetText( _T( "Currently not recording." ) );
    return;
  }

  //recording started

  extern CArray<CphxResource*> UpdateQueue;

  if ( Recording && UpdateQueue.NumItems() )
  {
    if ( progresstext ) progresstext->SetText( CString::Format( _T( "Generating... %d items remaining." ), UpdateQueue.NumItems() ) );
    return;
  }

  //generating done

  TF32 frame = ( CurrentFrame*Project.Timeline->Timeline->FrameRate ) / Framerate;
  TS32 FrameToRender = (TS32)( frame + 0.5f ); //round to nearest

  if ( frame < StartFrame )
  {
    if ( progresstext ) progresstext->SetText( CString::Format( _T( "Seeking... Frame %d" ), FrameToRender ) );
  }
  else
  {
    if ( progresstext ) progresstext->SetText( CString::Format( _T( "Dumping image %d (frame %d)" ), CurrentFrame, FrameToRender ) );
  }

  CapexVideoDumperPreview *d = (CapexVideoDumperPreview*)FindChildByID( _T( "DisplayBox" ), _T( "dumperpreview" ) );
  if ( d )
  {
    API->FlushDrawBuffer();
    API->GetDevice()->ForceStateReset();
    API->SetRenderView( ClientToScreen( GetClientRect() ) );
    d->PrepareFrame();
    API->GetDevice()->ForceStateReset();
    API->SetUIRenderState();

    FixesStepTimePosition = (TS32)( frame / (TF32)Project.Timeline->Timeline->FrameRate * 1000 + 0.5f );
    Project.MusicData.CurrentFrame = FrameToRender;

    if ( frame >= StartFrame )
    {
      //dump
      if ( Project.Timeline->Timeline->Target )
      {
        CCoreDX11Texture2D *RT = d->RenderTargets->GetRenderTarget( Project.Timeline->Timeline->Target );
        if ( !RT )
        {
          //emtpy image
          LOG_ERR( "RT NOT FOUND" );
        }
        else
        {
          CWBButton *pngbutt = (CWBButton*)FindChildByID( _T( "pngtoggle" ), _T( "button" ) );
          if ( pngbutt && pngbutt->IsPushed() )
          {
            CString Filename = Config::VideoDumpPath + Project.Title + CString::Format( _T( "%.5d.png" ), CurrentFrame );
            RT->ExportToImage( Filename, true, CORE_PNG, true );
          }
          else
          {
            CString Filename = Config::VideoDumpPath + Project.Title + CString::Format( _T( "%.5d.tga" ), CurrentFrame );
            RT->ExportToImage( Filename, true, CORE_TGA, true );
          }
        }
      }
      Sleep( 100 );
    }
  }

  if ( frame >= EndFrame ) //stop recording
  {
    Recording = false;
    CWBButton *b = (CWBButton*)FindChildByID( _T( "Start" ), _T( "button" ) );
    if ( b ) b->SetText( _T( "GO!" ) );
  }

  CurrentFrame++;
}

TBOOL CapexVideoDumper::MessageProc( CWBMessage &Message )
{

  switch ( Message.GetMessage() )
  {
  case WBM_FOCUSLOST:
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    CWBTextBox *t = (CWBTextBox *)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( b && Message.GetMessage() == WBM_COMMAND )
    {
      if ( b->GetID() == _T( "Exit" ) )
      {
        Root->ToggleVideoDumper();
        return true;
      }
      if ( b->GetID() == _T( "Start" ) )
      {
        if ( !Recording )
        {
          Project.SeekToFrame( 0 );
          Project.ResetParticles();
          FixesStepTimePosition = 0;
          CurrentFrame = 0;
          for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
            Project.Timeline->Events[ x ]->RequestContent();

          t = (CWBTextBox *)FindChildByID( _T( "Path" ), _T( "textbox" ) ); if ( t ) t->SetText( Config::VideoDumpPath );
          t = (CWBTextBox *)FindChildByID( _T( "XRes" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), XRes ) );
          t = (CWBTextBox *)FindChildByID( _T( "YRes" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), YRes ) );
          t = (CWBTextBox *)FindChildByID( _T( "Framerate" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%g" ), Framerate ) );
          t = (CWBTextBox *)FindChildByID( _T( "StartFrame" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), StartFrame ) );
          t = (CWBTextBox *)FindChildByID( _T( "EndFrame" ), _T( "textbox" ) ); if ( t ) t->SetText( CString::Format( _T( "%d" ), EndFrame ) );

          CapexVideoDumperPreview *d = (CapexVideoDumperPreview*)FindChildByID( _T( "DisplayBox" ), _T( "dumperpreview" ) );
          if ( d ) d->RenderTargets->UpdateCollection( XRes, YRes, true );

          b->SetText( _T( "Stop" ) );
        }
        else
        {
          b->SetText( _T( "GO!" ) );
        }

        Recording = !Recording;

        return true;
      }

      if ( b->GetID() == _T( "pngtoggle" ) )
      {
        if ( !Recording )
          b->Push( !b->IsPushed() );
        return true;
      }
    }

    if ( t )
    {
      if ( Recording ) return true;

      if ( t->GetID() == _T( "Path" ) )
      {
        Config::VideoDumpPath = t->GetText();
        if ( Config::VideoDumpPath[ Config::VideoDumpPath.Length() - 1 ] != '\\' && Config::VideoDumpPath[ Config::VideoDumpPath.Length() - 1 ] != '/' ) Config::VideoDumpPath += _T( "\\" );
        t->SetText( Config::VideoDumpPath );
        return true;
      }

      if ( t->GetID() == _T( "XRes" ) )
      {
        t->GetText().Scan( _T( "%d" ), &XRes );
        t->SetText( CString::Format( _T( "%d" ), XRes ) );
        return true;
      }

      if ( t->GetID() == _T( "YRes" ) )
      {
        t->GetText().Scan( _T( "%d" ), &YRes );
        t->SetText( CString::Format( _T( "%d" ), YRes ) );
        return true;
      }

      if ( t->GetID() == _T( "Framerate" ) )
      {
        t->GetText().Scan( _T( "%g" ), &Framerate );
        t->SetText( CString::Format( _T( "%g" ), Framerate ) );
        return true;
      }

      if ( t->GetID() == _T( "StartFrame" ) )
      {
        t->GetText().Scan( _T( "%d" ), &StartFrame );
        t->SetText( CString::Format( _T( "%d" ), StartFrame ) );
        return true;
      }

      if ( t->GetID() == _T( "EndFrame" ) )
      {
        t->GetText().Scan( _T( "%d" ), &EndFrame );
        t->SetText( CString::Format( _T( "%d" ), EndFrame ) );
        return true;
      }

    }
  }
  break;

  case WBM_KEYDOWN:
    if ( Message.Key == VK_ESCAPE )
    {
      Root->ToggleVideoDumper();
      return true;
    }
    break;

  default:
    break;
  }

  return CWBItem::MessageProc( Message );
}

