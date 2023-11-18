#define _WINSOCKAPI_
#include "BasePCH.h"
#include "../Phoenix/Timeline.h"
#include "../Phoenix_Tool/Timeline_tool.h"
#include "TimelineEditor.h"
#define WINDOWNAME _T("Timeline Editor")
#define WINDOWXML _T("TimelineEditor")
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"
#include "TimelineEventSplines.h"
#include "TimelineEventParameters.h"
#include "TimelinePreview.h"

//#define WBM_SELECTEVENT 0x100001

CapexTimelineEditor::CapexTimelineEditor() : CapexWindow()
{
}

CapexTimelineEditor::CapexTimelineEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexTimelineEditor::~CapexTimelineEditor()
{
}

void CapexTimelineEditor::UpdateData()
{
  CWBTimeline *t = (CWBTimeline*)FindChildByID( _T( "timeline" ), _T( "timelineeditorphx" ) );
  if ( !t ) return;

  CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "bpminput" ), _T( "textbox" ) );
  if ( ib )	ib->SetText( CString::Format( _T( "%.1f" ), Project.Timeline->GridBPM ) );

  ib = (CWBTextBox*)FindChildByID( _T( "highlightpri" ), _T( "textbox" ) );
  if ( ib )	ib->SetText( CString::Format( _T( "%d" ), Project.Timeline->BPMPrimaryModulus ) );

  ib = (CWBTextBox*)FindChildByID( _T( "highlightsec" ), _T( "textbox" ) );
  if ( ib )	ib->SetText( CString::Format( _T( "%d" ), Project.Timeline->BPMSecondaryModulus ) );

  CWBItemSelector *EventTypes = (CWBItemSelector*)FindChildByID( _T( "eventselector" ), _T( "itemselector" ) );
  if ( EventTypes )
  {
    EventTypes->Flush();
    AddEventLink( EventTypes, EVENT_ENDDEMO, 0, _T( "End Demo" ) );
    AddEventLink( EventTypes, EVENT_RENDERDEMO, 0, _T( "Render Demo" ) );
    AddEventLink( EventTypes, EVENT_RENDERSCENE, 0, _T( "Render Scene" ) );
    AddEventLink( EventTypes, EVENT_CAMERASHAKE, 0, _T( "Camera Shake" ) );
    AddEventLink( EventTypes, EVENT_PARTICLECALC, 0, _T( "Particle Calc" ) );
    AddEventLink( EventTypes, EVENT_CAMERAOVERRIDE, 0, _T( "Camera Override" ) );

    for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
      if ( Project.GetTechByIndex( x )->Tech->Type == TECH_SHADERTOY )
        AddEventLink( EventTypes, EVENT_SHADERTOY, Project.GetTechByIndex( x ), Project.GetTechByIndex( x )->Name );
  }
}

TBOOL EventTimesIntersect( CphxEvent_Tool *a, CphxEvent_Tool *b )
{
  CphxEvent *c = a->Event;
  CphxEvent *d = b->Event;
  CRect r1 = CRect( c->StartFrame, 0, c->EndFrame, 1 );
  CRect r2 = CRect( d->StartFrame, 0, d->EndFrame, 1 );
  return r1.Intersects( r2 );
}


TBOOL CapexTimelineEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    //CWBItem *bi = FindChildByID(_T("eventselector"), _T("button"));
    //if (bi && bi->GetGuid() == Message.GetTarget())
    //{
    //	EventLinks.Flush();
    //	//CWBContextMenu *ctx = new CWBContextMenu(App->GetRoot(), CRect(bi->GetScreenRect().BottomLeft(), CPoint(0, 0)), GetGuid());
    //	CWBContextMenu *ctx = OpenContextMenu(bi->GetScreenRect().BottomLeft());
    //	AddEventLink(ctx, EVENT_ENDDEMO, 0, _T("End Demo"));
    //	AddEventLink(ctx, EVENT_RENDERDEMO, 0, _T("Render Demo"));
    //	AddEventLink(ctx, EVENT_RENDERSCENE, 0, _T("Render Scene"));
    //	AddEventLink(ctx, EVENT_CAMERASHAKE, 0, _T("Camera Shake"));
    //	AddEventLink(ctx, EVENT_PARTICLECALC, 0, _T("Particle Calc"));

    //	for (TS32 x = 0; x < Project.GetTechCount(); x++)
    //		if (Project.GetTechByIndex(x)->Tech->Type == TECH_SHADERTOY)
    //			AddEventLink(ctx, EVENT_SHADERTOY, Project.GetTechByIndex(x), Project.GetTechByIndex(x)->Name);
    //	return true;
    //}

    CWBItem *bi = FindChildByID( _T( "loadsong" ), _T( "button" ) );
    if ( bi && bi->GetGuid() == Message.GetTarget() )
    {
      OpenSongImport();
      return true;
    }

    bi = FindChildByID( _T( "playsong" ), _T( "button" ) );
    if ( bi && bi->GetGuid() == Message.GetTarget() )
    {
      Project.TogglePlaying();
      return true;
    }

    bi = FindChildByID( _T( "generateall" ), _T( "button" ) );
    if ( bi && bi->GetGuid() == Message.GetTarget() )
    {
      for ( int x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        Project.Timeline->Events[ x ]->RequestContent();
      return true;
    }
  }
  break;
  case WBM_KEYDOWN:
  {
    if ( !InFocus() ) break;
    if ( Message.KeyboardState&WB_KBSTATE_ALT && Message.KeyboardState&WB_KBSTATE_CTRL )
      break; //altgr

    switch ( Message.Key )
    {
    case VK_UP:
    {
      for ( int x = 0; x < Project.Timeline->Events.NumItems(); x++ )
      {
        CphxEvent_Tool *e = Project.Timeline->Events[ x ];
        if ( e->Selected )
        {
          int pass = e->Pass;
          bool ok = false;
          while ( !ok && pass > 0 )
          {
            pass--;
            ok = true;
            for ( int y = 0; y < Project.Timeline->Events.NumItems(); y++ )
              if ( Project.Timeline->Events[ y ]->Pass == pass )
                if ( EventTimesIntersect( Project.Timeline->Events[ y ], e ) )
                  ok = false;
          }
          if ( ok )
            e->Pass = pass;
        }
      }
      Project.Timeline->Sort();
    }
    break;
    case VK_DOWN:
    {
      for ( int x = Project.Timeline->Events.NumItems() - 1; x >= 0; x-- )
      {
        CphxEvent_Tool *e = Project.Timeline->Events[ x ];
        if ( e->Selected )
        {
          int pass = e->Pass;
          bool ok = false;
          while ( !ok )
          {
            pass++;
            ok = true;
            for ( int y = 0; y < Project.Timeline->Events.NumItems(); y++ )
              if ( Project.Timeline->Events[ y ]->Pass == pass )
                if ( EventTimesIntersect( Project.Timeline->Events[ y ], e ) )
                  ok = false;
          }
          e->Pass = pass;
        }
      }
      Project.Timeline->Sort();
    }
    break;
    }
    break;
  }
  break;
  case WBM_CHAR:
    if ( Message.Key == ' ' )
    {
      Project.TogglePlaying();
      return true;
    }
    if ( Message.Key == 'H' || Message.Key == 'h' )
    {
      CapexTimelinePreview *p = (CapexTimelinePreview *)WorkBench->GetWindowByIndex( apEx_TimelinePreview, 0 );
      if ( p ) p->ToggleHistogram();
      return true;
    }
    if ( Message.Key == '*' ) //place beat marker
    {
      if ( !Project.Timeline->BeatMarkerPlaced )
      {
        Project.Timeline->BeatMarkerPlaced = true;
        Project.Timeline->BeatMarkers.FlushFast();
      }

      Project.Timeline->BeatMarkers += Project.GetTimepos();

      return true;
    }
    if ( Message.Key == 'C' || Message.Key == 'c' )
    {
      CopySelected();
      return true;
    }
    break;
  case WBM_TEXTCHANGED:
  {
    CWBTimeline *t = (CWBTimeline*)FindChildByID( _T( "timeline" ), _T( "timelineeditorphx" ) );
    if ( !t ) break;

    CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "bpminput" ), _T( "textbox" ) );
    if ( ib && ib->GetGuid() == Message.GetTarget() ) ib->GetText().Scan( _T( "%f" ), &Project.Timeline->GridBPM );

    ib = (CWBTextBox*)FindChildByID( _T( "highlightpri" ), _T( "textbox" ) );
    if ( ib && ib->GetGuid() == Message.GetTarget() ) ib->GetText().Scan( _T( "%d" ), &Project.Timeline->BPMPrimaryModulus );

    ib = (CWBTextBox*)FindChildByID( _T( "highlightsec" ), _T( "textbox" ) );
    if ( ib && ib->GetGuid() == Message.GetTarget() ) ib->GetText().Scan( _T( "%d" ), &Project.Timeline->BPMSecondaryModulus );

  }
  break;
  case WBM_ITEMSELECTED:
  {
    if ( Message.GetTargetID() == _T( "eventselector" ) )
      if ( Message.Data >= 0 && Message.Data < EventLinks.NumItems() )
      {
        CWBTimeline *t = (CWBTimeline*)FindChildByID( _T( "timeline" ), _T( "timelineeditorphx" ) );
        if ( t )
        {
          t->EventToCreate = EventLinks[ Message.Data ].EventType;
          t->Tech = EventLinks[ Message.Data ].Tech;
          return true;
        }
      }
  }
  //case WBM_SELECTEVENT:
  //{
  //	SelectEvent((CphxEvent_Tool*)Message.Data);
  //	return true;
  //}
  break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTimelineEditor::AddEventLink( CWBItemSelector *m, PHXEVENTTYPE type, CphxMaterialTechnique_Tool *Tech, CString Text )
{
  TS32 id = EventLinks.NumItems();
  SELECTABLEID eid = m->AddItem( Text.GetPointer() );
  m->GetItem( eid )->SetID( id );

  EVENTCREATORLINK l;
  l.EventType = type;
  l.Tech = Tech;
  EventLinks += l;
}

#include <CommDlg.h>
#include <shlwapi.h>
#include "apExRoot.h"

void CapexTimelineEditor::OpenSongImport()
{
  if ( !Project.CanLoadSong() ) return;

  TCHAR dir[ 1024 ];
  GetCurrentDirectory( 1024, dir );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "Music Files (MVM, Ogg, V2, Wavesabre)\0*.mvm;*.ogg;*.v2m;*.wavesabre\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open Song";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = NULL;
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  extern CapexRoot *Root;
  opf.lpstrInitialDir = Root->GetTargetDirectory( "opensong" );

  if ( GetOpenFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "opensong" );
    SetCurrentDirectory( dir );
    CStreamReaderFile f;
    if ( f.Open( opf.lpstrFile ) )
    {
      if ( f.GetLength() )
      {

        Project.LoadSong( f, (HWND)App->GetHandle() );

        Project.SongFile = opf.lpstrFile;
        if ( Project.LoadedFileName.Length() )
        {
          TCHAR path[ MAX_PATH ];
          if ( PathRelativePathToA( path, Project.LoadedFileName.GetPointer(), 0, opf.lpstrFile, 0 ) )
            Project.SongFile = CString( path );
        }

        //if (Project.MusicData.MVXLoaded)
        //{
        //	if (Project.MusicData.MVXPlaying)
        //		mvxSystemWav_Stop();
        //	mvxSystemWav_DeInit();
        //	Project.MusicData.MVXLoaded = false;
        //}

        //SAFEDELETEA(Project.MusicData.MusicData);
        //Project.MusicData.MusicData = new TU8[(TS32)f.GetLength()];
        //f.Read(Project.MusicData.MusicData, (TS32)f.GetLength());
        //Project.MusicData.MusicDataSize = (TS32)f.GetLength();

        ////if (MvxInited)
        ////{
        ////	//mvxSystem_DeInit();
        ////	MvxInited=false;
        ////}

        //SAFEDELETE(Project.MusicData.Wav);
        //SAFEDELETE(Project.MusicData.TickData);

        //Project.MusicData.AppHandle = (HWND)App->GetHandle();

        //HANDLE h = CreateThread(NULL, 0, MVXRenderThread, NULL, 0, NULL);
        //SetThreadPriority(h, THREAD_PRIORITY_TIME_CRITICAL);

        ////mvxSystem_Init((HWND)App->GetHandle(),(char*)MusicData,MusicDataSize);
        ////mvxSystem_Play();
        ////MvxInited=true;

      }
    }
  }

  SetCurrentDirectory( dir );

}

void CapexTimelineEditor::SelectEvent( CphxEvent_Tool *ev )
{
  //if (!ev) return;

  CWBTimeline *t = (CWBTimeline*)FindChildByID( _T( "timeline" ), _T( "timelineeditorphx" ) );
  if ( t ) t->SelectEvent( ev );

}

void CapexTimelineEditor::OnDraw( CWBDrawAPI *API )
{
  CWBLabel *timestamp = (CWBLabel*)FindChildByID( _T( "timestamp" ), _T( "label" ) );
  if ( timestamp )
  {
    TS32 cf = Project.MusicData.CurrentFrame;
    TF32 cs = cf / (TF32)Project.Timeline->Timeline->FrameRate;
    TS32 mins = ( (TS32)cs ) / 60;
    TF32 secs = cs - mins * 60;
    timestamp->SetText( CString::Format( _T( "%d:%.2f:%d" ), mins, secs, cf ) );
  }

  CapexWindow::OnDraw( API );
}

void CapexTimelineEditor::CopySelected()
{
  TS32 maxframe = 0;
  TS32 minframe = 100000000;
  TS32 maxpass = 0;
  TS32 minpass = 100000000;
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    if ( Project.Timeline->Events[ x ]->Selected )
    {
      minframe = min( minframe, Project.Timeline->Events[ x ]->Event->StartFrame );
      maxframe = max( maxframe, Project.Timeline->Events[ x ]->Event->EndFrame );
      minpass = min( minpass, Project.Timeline->Events[ x ]->Pass );
      maxpass = max( maxpass, Project.Timeline->Events[ x ]->Pass );
    }
  bool ok = false;
  TS32 startpass = maxpass;
  while ( !ok )
  {
    startpass++;
    ok = true;
    for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    {
      CphxEvent_Tool *e = Project.Timeline->Events[ x ];
      if ( e->Pass >= startpass && e->Pass <= startpass + maxpass - minpass )
        if ( e->Event->StartFrame <= maxframe && e->Event->EndFrame >= minframe )
        {
          ok = false;
          break;
        }
    }
  }

  CphxEvent_Tool *e = NULL;

  for ( TS32 x = Project.Timeline->Events.NumItems() - 1; x >= 0; x-- )
    if ( Project.Timeline->Events[ x ]->Selected )
    {
      CopyEvent( Project.Timeline->Events[ x ], startpass + Project.Timeline->Events[ x ]->Pass - minpass );
      if ( !e ) e = Project.Timeline->Events.Last();
    }

  Project.Timeline->Sort();
  SelectEvent( e );
}

void CapexTimelineEditor::CopyEvent( CphxEvent_Tool *e, TS32 TargetPass )
{
  e->Selected = false;

  CphxMaterialTechnique_Tool *tech = NULL;
  if ( e->Type == EVENT_SHADERTOY )
    tech = ( (CphxEvent_Shadertoy_Tool*)e )->GetTech();

  CphxEvent_Tool *n = Project.Timeline->CreateEvent( e->Type, e->Event->StartFrame, e->Event->EndFrame, TargetPass, tech );
  SAFEDELETE( n->Time );
  n->Time = (CphxSpline_Tool_float16*)e->Time->Copy();
  n->TargetID = e->TargetID;
  n->Event->Time = (CphxSpline_float16*)n->Time->Spline;
  if ( e->Name.Length() )
    n->SetName( e->Name );

  switch ( e->Type )
  {
  case EVENT_ENDDEMO:
    break;
  case EVENT_RENDERDEMO:
    break;
  case EVENT_SHADERTOY:
  {
    CphxEvent_Shadertoy_Tool *o = (CphxEvent_Shadertoy_Tool*)e;
    CphxEvent_Shadertoy_Tool *c = (CphxEvent_Shadertoy_Tool*)n;
    CphxEvent_Shadertoy *oe = (CphxEvent_Shadertoy*)e->Event;
    CphxEvent_Shadertoy *ce = (CphxEvent_Shadertoy*)n->Event;

    c->MaterialData.Copy( &o->MaterialData );
    c->ShaderSplines.Copy( &o->ShaderSplines );
  }
  break;
  case EVENT_RENDERSCENE:
  {
    CphxEvent_RenderScene_Tool *o = (CphxEvent_RenderScene_Tool*)e;
    CphxEvent_RenderScene_Tool *c = (CphxEvent_RenderScene_Tool*)n;
    c->SetScene( o->Scene );
    c->SetCamera( o->Camera );
    c->SetClip( o->Clip );
    CphxEvent_RenderScene *oe = (CphxEvent_RenderScene*)e->Event;
    CphxEvent_RenderScene *ce = (CphxEvent_RenderScene*)n->Event;
    ce->ClearColor = oe->ClearColor;
    ce->ClearZ = oe->ClearZ;
  }
  break;
  case EVENT_PARTICLECALC:
  {
    CphxEvent_ParticleCalc_Tool* o = (CphxEvent_ParticleCalc_Tool*)e;
    CphxEvent_ParticleCalc_Tool* c = (CphxEvent_ParticleCalc_Tool*)n;
    c->SetScene( o->Scene );
    c->SetCamera( o->Camera );
    c->SetClip( o->Clip );
  }
  break;
  case EVENT_CAMERAOVERRIDE:
  {
    CphxEvent_CameraOverride_Tool* o = (CphxEvent_CameraOverride_Tool*)e;
    CphxEvent_CameraOverride_Tool* c = (CphxEvent_CameraOverride_Tool*)n;
    c->SetScene( o->Scene );
    c->SetCamera( o->Camera );
    c->SetClip( o->Clip );
  }
  break;
  case EVENT_CAMERASHAKE:
  {
    CphxEvent_CameraShake_Tool *o = (CphxEvent_CameraShake_Tool*)e;
    CphxEvent_CameraShake_Tool *c = (CphxEvent_CameraShake_Tool*)n;
    c->TargetIntensity = (CphxSpline_Tool_float16*)o->TargetIntensity->Copy();
    c->EyeIntensity = (CphxSpline_Tool_float16*)o->EyeIntensity->Copy();

    CphxEvent_CameraShake *oe = (CphxEvent_CameraShake *)e->Event;
    CphxEvent_CameraShake *ce = (CphxEvent_CameraShake *)n->Event;
    ce->ShakesPerSec = oe->ShakesPerSec;
    ce->EyeIntensity = (CphxSpline_float16*)c->EyeIntensity->Spline;
    ce->TargetIntensity = (CphxSpline_float16*)c->TargetIntensity->Spline;
  }
  break;
  default:
    break;
  }

}

CWBTimeline::CWBTimeline( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  LineHeight = 10;
  Zoom = 10;
  Offset = 0;
  GridRect = CRect( 0, -15, 0, 0 );
  Dragmode = DRAGMODE_NONE;
  //GridBPM = 120.0f;
  OffsetPass = 0;
  RemainingFrames = RemainingPasses = 0;
  EventToCreate = (PHXEVENTTYPE)-1;
  //BPMPrimaryModulus = 32;
  //BPMSecondaryModulus = 8;
  EditBox = NULL;
}

CWBTimeline::~CWBTimeline()
{

}

CWBItem * CWBTimeline::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBTimeline *timeline = new CWBTimeline( Root, Pos );

  return timeline;
}

void CWBTimeline::OnDraw( CWBDrawAPI *API )
{
  if ( Dragmode != DRAGMODE_TIMESEEK ) Project.UpdateCurrentFrameFromPlayback();

  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );

  DrawBackground( API );
  API->SetCropToClient( this );

  CRect p = GetClientRect() + GridRect;

  CColor GridColor = CSSProperties.DisplayDescriptor.GetColor( GetState(), WB_ITEM_BORDERCOLOR );

  //seek bar
  API->DrawRect( CRect( 0, 0, p.x2, p.y1 ), GridColor );
  API->DrawRect( CRect( 0, 0, (TS32)( p.x2*Project.MusicData.MVXRenderStatus ), p.y1 ), CColor::FromARGB( 0xff598557 ) );

  if ( Dragmode == DRAGMODE_TIMESEEK || Dragmode == DRAGMODE_RESIZELEFT || Dragmode == DRAGMODE_RESIZERIGHT || ( MouseOver() && CRect( 0, 0, p.x2, p.y1 ).Contains( ScreenToClient( App->GetMousePos() ) ) ) )
    App->SelectMouseCursor( CM_SIZEWE );

  //grid
  API->DrawRectBorder( p, GridColor );

  for ( TS32 x = p.y1; x < p.y2; x += LineHeight )
    API->DrawRect( CRect( CPoint( p.x1, x ), CPoint( p.x2, x + 1 ) ), GridColor );

  //tick behind text
  for ( TS32 x = 0; x < 6; x++ )
  {
    TS32 pc = (TS32)( p.Width()*x / 5.0f ) + p.x1;
    //tick behind text
    API->DrawRect( CRect( pc, p.y1 - 3, pc + 1, p.y1 + 1 ), 0x60ffffff );
  }

  //Current frame on seek bar
  TS32 fr = FrameToClient( Project.MusicData.CurrentFrame );
  API->DrawRect( CRect( fr, GetClientRect().y1, fr + 1, p.y1 ), 0xffffffff );

  for ( TS32 x = 0; x < 6; x++ )
  {
    TS32 pc = (TS32)( p.Width()*x / 5.0f ) + p.x1;
    TS32 frame = (TS32)( pc*Zoom + Offset );
    TF32 timesec = frame / (TF32)Project.Timeline->Timeline->FrameRate;

    CString txt = CString::Format( _T( "%d:%2.2f (%d)" ), (TS32)( timesec / 60 ), fmod( timesec, 60.0f ), frame );
    TS32 len = Font->GetWidth( txt, false );

    CRect r = CRect( pc, 0, pc, -GridRect.y1 );

    CPoint fontpos = Font->GetCenter( txt, r );
    if ( fontpos.x < p.x1 + 2 ) fontpos.x = p.x1 + 2;
    if ( fontpos.x + len > p.x2 - 2 ) fontpos.x = p.x2 - 2 - len;
    fontpos.y -= 1;

    WriteWithOutline( API, Font, txt, fontpos );
    //Font->Write(API, txt, fontpos);
  }

  //bpm grid

  if ( !Project.MusicData.TickData.NumItems() )
  {
    TF32 TickLength = 60.0f / Project.Timeline->GridBPM*(TF32)Project.Timeline->Timeline->FrameRate / Zoom;
    TS32 TicksTooSmall = (TS32)( 16 / TickLength );
    TS32 skip = 1;
    for ( ; TicksTooSmall; TicksTooSmall = TicksTooSmall >> 1 )
      skip <<= 1;

    TF32 bpmoff = Offset / (TF32)Project.Timeline->Timeline->FrameRate / 60.0f;
    bpmoff = bpmoff - fmod( bpmoff, 1 / Project.Timeline->GridBPM );

    TS32 bpmstart = (TS32)( bpmoff*Project.Timeline->GridBPM );

    TF32 endframe = ( Offset + p.Width()*Zoom ) / (TF32)Project.Timeline->Timeline->FrameRate / 60.0f;

    if ( Project.MusicData.OfflineLength > 0 )
      endframe = min( endframe, TF32( Project.MusicData.OfflineLength / 1000.0 / 60.0 - 1.0 / Project.Timeline->GridBPM ) );

    TS32 bpmend = (TS32)( endframe*Project.Timeline->GridBPM ) + 1;


    for ( TS32 x = bpmstart; x <= bpmend; x++ )
    {
      if ( x%skip ) continue;

      TS32 f = (TS32)( x / Project.Timeline->GridBPM * 60 * Project.Timeline->Timeline->FrameRate / Zoom - Offset / Zoom );
      CColor Color = GridColor;

      if ( !( x%Project.Timeline->BPMSecondaryModulus ) ) Color = CColor::FromARGB( 0xff598557 );//0xff9e9e9e;
      if ( !( x%Project.Timeline->BPMPrimaryModulus ) ) Color = 0xffadadad;// CColor::FromARGB(0xff007acc);

      API->DrawRect( CRect( CPoint( f, p.y1 + 1 ), CPoint( f + 1, p.y2 ) ), Color );
    }
  }
  else
  { //use tick data

    TU32 startoff = Offset;
    TU32 endoff = ClientToFrame( p.x2 );

    TF32 TickLength = Project.MusicData.TickData.Last() / (TF32)Project.MusicData.TickData.NumItems() / 1000.0f*(TF32)Project.Timeline->Timeline->FrameRate / Zoom;
    if ( TickLength )
    {
      TS32 TicksTooSmall = (TS32)( 16 / TickLength );
      TS32 skip = 1;
      for ( ; TicksTooSmall; TicksTooSmall = TicksTooSmall >> 1 )
        skip <<= 1;

      for ( TS32 x = 0; x < Project.MusicData.TickData.NumItems(); x++ )
      {
        if ( Project.MusicData.TickData[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate >= startoff )
        {
          if ( x%skip ) continue;

          TS32 f = FrameToClient( (TS32)( Project.MusicData.TickData[ x ] / 1000.0f*(TF32)Project.Timeline->Timeline->FrameRate ) );
          CColor Color = GridColor;

          if ( !( x%Project.Timeline->BPMSecondaryModulus ) ) Color = CColor::FromARGB( 0xff598557 );//0xff9e9e9e;
          if ( !( x%Project.Timeline->BPMPrimaryModulus ) ) Color = 0xffadadad;// CColor::FromARGB(0xff007acc);

          API->DrawRect( CRect( CPoint( f, p.y1 ), CPoint( f + 1, p.y2 ) ), Color );
        }

        if ( Project.MusicData.TickData[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate > endoff ) break;
      }
    }

  }

  for ( TS32 x = 0; x < Project.Timeline->BeatMarkers.NumItems(); x++ )
  {
    fr = FrameToClient( (TS32)( Project.Timeline->BeatMarkers[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate ) );
    API->DrawRect( CRect( fr, p.y1, fr + 1, GetClientRect().y2 ), CColor::FromARGB( 0xffff0000 ) );
  }

  //Current frame

  fr = FrameToClient( Project.MusicData.CurrentFrame );
  API->DrawRect( CRect( fr, p.y1, fr + 1, GetClientRect().y2 ), 0xffffffff );

  //dragarea
  TS32 sp = ClientToFrame( ScreenToClient( App->GetMousePos() ).x );
  TS32 x1 = max( 0, min( ClickedFrame, sp ) );
  TS32 x2 = max( 0, max( ClickedFrame, sp ) );

  TS32 cp = ( ScreenToClient( App->GetMousePos() ) - p.TopLeft() ).y / LineHeight + OffsetPass;
  TS32 y1 = max( OffsetPass, min( ClickedPass, cp ) );
  TS32 y2 = max( OffsetPass, max( ClickedPass, cp ) );

  CRect dragrect = CRect( x1, y1, x2, y2 );

  //events
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    if ( Project.Timeline->Events[ x ]->Pass >= OffsetPass )
      if ( !Project.Timeline->Events[ x ]->Selected || ( !Project.Timeline->Events[ x ]->Intersects( dragrect ) && Dragmode == DRAGMODE_SELECT ) )
        DrawEvent( API, Project.Timeline->Events[ x ], false );

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    if ( Project.Timeline->Events[ x ]->Pass >= OffsetPass )
      if ( Project.Timeline->Events[ x ]->Selected || ( Project.Timeline->Events[ x ]->Intersects( dragrect ) && Dragmode == DRAGMODE_SELECT ) )
        DrawEvent( API, Project.Timeline->Events[ x ], true );

  //overtime
  
  if ( Project.MusicData.OfflineLength > 0 )
  {
    int endFrame = int( Project.MusicData.OfflineLength*Project.Timeline->Timeline->FrameRate / 1000.0f );
    int endPos = FrameToClient( endFrame );
    API->DrawRect( CRect( endPos, p.y1, p.x2, p.y2 ), 0x80000000 );
  }

  //last pass to draw
  if ( Project.LastPassToDraw >= 0 )
  {
    TS32 lpd = max( Project.LastPassToDraw, OffsetPass - 1 );
    CRect p = GetClientRect() + GridRect;
    API->DrawRect( CRect( p.x1, ( lpd - OffsetPass + 1 )*LineHeight, p.x2, ( lpd - OffsetPass + 1 )*LineHeight + 1 ) + p.TopLeft(), 0xffffffff );
  }

  //event creation
  if ( Dragmode == DRAGMODE_NEWEVENT )
  {
    TS32 spos = FrameToClient( ClickedFrame );
    TS32 x1p = min( spos, ScreenToClient( App->GetMousePos() ).x );
    TS32 x2p = max( spos, ScreenToClient( App->GetMousePos() ).x );
    CRect r = GetEventPosition( x1p, x2p, ClickedPass );
    API->DrawRect( r, CColor::FromARGB( 0x80ffffff ) );
    API->DrawRectBorder( r, CColor::FromARGB( 0xa0ffffff ) );
  }

  if ( Dragmode == DRAGMODE_SELECT && App->GetMousePos() != App->GetLeftDownPos() )
  {
    TS32 nx1 = FrameToClient( x1 );
    TS32 nx2 = FrameToClient( x2 );

    CRect r1 = GetEventPosition( nx1, nx2, y1 );
    CRect r2 = GetEventPosition( nx1, nx2, y2 );

    API->DrawRect( r1&r2, CColor::FromARGB( 0x30ffffff ) );
    API->DrawRectBorder( r1&r2, CColor::FromARGB( 0xffffffff ) );
  }

  DrawBorder( API );
}

TBOOL CWBTimeline::MessageProc( CWBMessage &Message )
{
  CRect p = GetClientRect() + GridRect;

  CPoint mousePos = App->GetMousePos();
  if ( Dragmode != DRAGMODE_NONE && Dragmode != DRAGMODE_NEWEVENT && Dragmode != DRAGMODE_TIMESEEK && App->GetRightButtonState() )
    mousePos = App->GetLeftDownPos();

  switch ( Message.GetMessage() )
  {
  case WBM_RIGHTBUTTONDOWN:

    App->SendMessage( CWBMessage( App, WBM_MOUSEMOVE, GetGuid() ) );

    if ( MouseOver() && Dragmode == DRAGMODE_NONE )
    {
      if ( App->GetShiftState() )
      {
        Project.LastPassToDraw = -1;
        return true;
      }

      if ( ClientToScreen( p ).Contains( mousePos ) )
      {
        if ( App->GetCtrlState() )
        {
          if ( EventToCreate >= 0 )
          {
            App->SetCapture( this );
            Dragmode = DRAGMODE_NEWEVENT;
            ClickedFrame = ClientToFrame( ScreenToClient( mousePos ).x );
            ClickedPass = ( ScreenToClient( mousePos ) - p.TopLeft() ).y / LineHeight + OffsetPass;
          }
        }
        else
        {
          App->SetCapture( this );
          Dragmode = DRAGMODE_TIMESEEK;
          Project.MusicData.TimelineDragged = true;
          Project.MusicData.CurrentFrame = max( 0, ClientToFrame( ScreenToClient( mousePos ).x ) );
        }
      }
      return true;
    }
    break;
  case WBM_MIDDLEBUTTONDOWN:
    if ( MouseOver() )
    {
      App->SetCapture( this );
      Dragmode = DRAGMODE_DRAG;
      StoredOffset = Offset;
      StoredOffsetPass = OffsetPass;
      return true;
    }
    break;
  case WBM_LEFTBUTTONDOWN:

    if ( MouseOver() )
    {
      App->SetCapture( this );
      LastMousePos = ScreenToClient( mousePos );
      StoredOffset = Offset;
      StoredOffsetPass = OffsetPass;
      RemainingPasses = 0;
      RemainingFrames = 0;
      ClickedFrame = ClientToFrame( ScreenToClient( mousePos ).x );
      ClickedPass = ( ScreenToClient( mousePos ) - p.TopLeft() ).y / LineHeight + OffsetPass;

      if ( App->GetShiftState() )
      {
        Project.LastPassToDraw = ClickedPass;
        Dragmode = DRAGMODE_DRAGLASTDRAWNPASS;
        return true;
      }

      CRect pr = GetClientRect() + GridRect;

      if ( pr.Contains( ScreenToClient( mousePos ) ) ) //clicked on event area
      {
        for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        {
          CRect r = GetEventPosition( Project.Timeline->Events[ x ] );
          CPoint mp = ScreenToClient( mousePos );

          if ( r.Contains( mp ) )
          {

            if ( !App->GetCtrlState() && !Project.Timeline->Events[ x ]->Selected )
              for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
                Project.Timeline->Events[ y ]->Selected = false;

            Project.Timeline->Events[ x ]->Selected = true;

            TS32 resizewidth = min( 4, max( 0, r.Width() - 1 ) / 2 );
            if ( resizewidth )
            {
              CRect leftresize = CRect( r.TopLeft(), r.BottomLeft() + CPoint( resizewidth, 0 ) );
              CRect rightresize = CRect( r.TopRight() - CPoint( resizewidth, 0 ), r.BottomRight() );

              for ( int y = 0; y < Project.Timeline->Events.NumItems(); y++ )
              {
                Project.Timeline->Events[ y ]->originalEndFrame = Project.Timeline->Events[ y ]->Event->EndFrame;
                Project.Timeline->Events[ y ]->originalStartFrame = Project.Timeline->Events[ y ]->Event->StartFrame;
              }

              if ( leftresize.Contains( mp ) )
              {
                ResizedEvent = Project.Timeline->Events[ x ];
                Dragmode = DRAGMODE_RESIZELEFT;
                return true;
              }
              if ( rightresize.Contains( mp ) )
              {
                ResizedEvent = Project.Timeline->Events[ x ];
                Dragmode = DRAGMODE_RESIZERIGHT;
                return true;
              }

            }

            Dragmode = DRAGMODE_MOVE;

            //select event here
            SelectEvent( Project.Timeline->Events[ x ] );

            return true;
          }
        }

        if ( !App->GetCtrlState() )
          for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
            Project.Timeline->Events[ y ]->Selected = false;

        Dragmode = DRAGMODE_SELECT;
        return true;
      }

      //clicked on timer area

      Dragmode = DRAGMODE_TIMESEEK;
      Project.MusicData.TimelineDragged = true;
      Project.MusicData.CurrentFrame = max( 0, ClientToFrame( ScreenToClient( mousePos ).x ) );

      return true;
    }
    break;
  case WBM_MOUSEMOVE:
    switch ( Dragmode )
    {
    case DRAGMODE_SELECT:
    case DRAGMODE_NONE:
      break;
    case DRAGMODE_DRAG:
    {
      Offset = (TS32)max( 0, StoredOffset + ( App->GetMidDownPos().x - mousePos.x )*Zoom );
      OffsetPass = max( 0, StoredOffsetPass + ( App->GetMidDownPos().y - mousePos.y ) / LineHeight );
      return true;
    }
    break;
    case DRAGMODE_DRAGLASTDRAWNPASS:
      Project.LastPassToDraw = max( 0, ( ScreenToClient( mousePos ) - p.TopLeft() ).y / LineHeight + OffsetPass );
      return true;
      break;
    case DRAGMODE_MOVE:
      DoEventDragging( mousePos );
      break;
    case DRAGMODE_TIMESEEK:
      Project.MusicData.CurrentFrame = max( 0, ClientToFrame( ScreenToClient( mousePos ).x ) );
      break;
    case DRAGMODE_RESIZELEFT:
    {
      TS32 frame = max( 0, min( ResizedEvent->Event->EndFrame - 1, ClientToFrame( ScreenToClient( mousePos ).x ) ) );

      //snap here
      if ( !App->GetShiftState() )
        frame = max( 0, min( ResizedEvent->Event->EndFrame - 1, GetCursorSnap( frame, ResizedEvent, mousePos ) ) );

      int frameDelta = frame - ResizedEvent->originalStartFrame;

      for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        if ( Project.Timeline->Events[ x ] != ResizedEvent && Project.Timeline->Events[ x ]->Pass == ResizedEvent->Pass )
        {
          CphxEvent_Tool *e = Project.Timeline->Events[ x ];
          if ( e->Event->EndFrame > ResizedEvent->originalStartFrame + frameDelta && e->Event->EndFrame < ResizedEvent->Event->EndFrame )
            frameDelta = max( frameDelta, e->Event->EndFrame - ResizedEvent->originalStartFrame );
        }

      frameDelta = min( frameDelta, ResizedEvent->originalEndFrame - ResizedEvent->originalStartFrame - 1 );

      //if (App->GetAltState())
      {
        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          if ( Project.Timeline->Events[ y ]->Selected )
          {
            for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
              if ( Project.Timeline->Events[ x ] != Project.Timeline->Events[ y ] && Project.Timeline->Events[ x ]->Pass == Project.Timeline->Events[ y ]->Pass )
              {
                CphxEvent_Tool *e = Project.Timeline->Events[ x ];
                if ( e->Event->EndFrame > Project.Timeline->Events[ y ]->originalStartFrame + frameDelta && e->Event->EndFrame < Project.Timeline->Events[ y ]->Event->EndFrame )
                  frameDelta = max( frameDelta, e->Event->EndFrame - Project.Timeline->Events[ y ]->originalStartFrame );
              }
          }

        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          if ( Project.Timeline->Events[ y ]->Selected )
            frameDelta = min( frameDelta, Project.Timeline->Events[ y ]->originalEndFrame - Project.Timeline->Events[ y ]->originalStartFrame - 1 );

        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          if ( Project.Timeline->Events[ y ]->Selected )
          {
            CphxEvent_Tool *e = Project.Timeline->Events[ y ];
            e->Event->StartFrame = e->originalStartFrame + frameDelta;
          }
      }

      ResizedEvent->Event->StartFrame = ResizedEvent->originalStartFrame + frameDelta;

    }
    break;
    case DRAGMODE_RESIZERIGHT:
    {
      TS32 frame = max( 0, max( ResizedEvent->Event->StartFrame + 1, ClientToFrame( ScreenToClient( mousePos ).x ) ) );

      //snap here
      if ( !App->GetShiftState() )
        frame = max( 0, max( ResizedEvent->Event->StartFrame + 1, GetCursorSnap( frame, ResizedEvent, mousePos ) ) );

      int frameDelta = frame - ResizedEvent->originalEndFrame;

      TS32 oldpos = ResizedEvent->Event->StartFrame;
      for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        if ( Project.Timeline->Events[ x ] != ResizedEvent && Project.Timeline->Events[ x ]->Pass == ResizedEvent->Pass )
        {
          CphxEvent_Tool *e = Project.Timeline->Events[ x ];
          if ( e->Event->StartFrame<frame && e->Event->StartFrame>ResizedEvent->Event->StartFrame )
            frameDelta = min( frameDelta, e->Event->StartFrame - ResizedEvent->originalEndFrame );// e->Event->StartFrame;
        }

      frameDelta = max( frameDelta, ResizedEvent->originalStartFrame - ResizedEvent->originalEndFrame + 1 );

      //if (App->GetAltState())
      {
        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          if ( Project.Timeline->Events[ y ]->Selected )
          {
            for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
              if ( Project.Timeline->Events[ x ] != Project.Timeline->Events[ y ] && Project.Timeline->Events[ x ]->Pass == Project.Timeline->Events[ y ]->Pass )
              {
                CphxEvent_Tool *e = Project.Timeline->Events[ x ];
                if ( e->Event->StartFrame<Project.Timeline->Events[ y ]->originalEndFrame + frameDelta && e->Event->StartFrame>Project.Timeline->Events[ y ]->Event->StartFrame )
                  frameDelta = min( frameDelta, e->Event->StartFrame - Project.Timeline->Events[ y ]->originalEndFrame );
              }
          }

        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          if ( Project.Timeline->Events[ y ]->Selected )
            frameDelta = max( frameDelta, Project.Timeline->Events[ y ]->originalStartFrame - Project.Timeline->Events[ y ]->originalEndFrame + 1 );

        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          if ( Project.Timeline->Events[ y ]->Selected )
          {
            CphxEvent_Tool *e = Project.Timeline->Events[ y ];
            e->Event->EndFrame = e->originalEndFrame + frameDelta;
          }
      }

      ResizedEvent->Event->EndFrame = ResizedEvent->originalEndFrame + frameDelta;
    }
    break;
    default:
      break;
    }
    break;
  case WBM_MOUSEWHEEL:
    if ( MouseOver() )
    {
      float oldzoom = Zoom;
      Zoom = max( 0.005f, Zoom*( 1 - Message.Data / 10.0f ) );

      CRect r = GetClientRect() + GridRect;

      int pp = ScreenToClient( mousePos ).x - r.x1;

      float dz = Zoom / (float)oldzoom;
      Offset += (TS32)( pp*oldzoom*( 1 - dz ) );
      Offset = max( 0, Offset );

    }
    break;
  case WBM_RIGHTBUTTONUP:
    App->SendMessage( CWBMessage( App, WBM_MOUSEMOVE, GetGuid() ) );
    App->ReleaseCapture();
    if ( Dragmode == DRAGMODE_NEWEVENT )
    {
      TS32 releaseFrame = ClientToFrame( ScreenToClient( mousePos ).x );
      TS32 xs = max( 0, min( releaseFrame, ClickedFrame ) );
      TS32 xe = max( 0, max( releaseFrame, ClickedFrame ) );

      for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        if ( Project.Timeline->Events[ x ]->Pass == ClickedPass )
        {
          CphxEvent *e = Project.Timeline->Events[ x ]->Event;
          if ( e->StartFrame <= xs && e->EndFrame >= xe ) //positions inside an event
          {
            xe = xs - 1;
            break;
          }
          if ( e->StartFrame == xs ) xs = e->EndFrame;
          if ( e->EndFrame == xe ) xe = e->StartFrame;

          if ( e->StartFrame > xs && e->EndFrame < xe ) //crop with event
            if ( xs == ClickedFrame ) xe = e->StartFrame;
            else xs = e->EndFrame;
        }

      for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        if ( Project.Timeline->Events[ x ]->Pass == ClickedPass )
        {
          CphxEvent *e = Project.Timeline->Events[ x ]->Event;
          if ( e->StartFrame >= xs && e->StartFrame <= xe )	xe = e->StartFrame;
          if ( e->EndFrame >= xs && e->EndFrame <= xe )	xs = e->EndFrame;
        }

      if ( xs < xe )
      {
        for ( TS32 y = 0; y < Project.Timeline->Events.NumItems(); y++ )
          Project.Timeline->Events[ y ]->Selected = false;

        CphxEvent_Tool *t = Project.Timeline->CreateEvent( EventToCreate, xs, xe, ClickedPass, Tech );
        Project.Timeline->Sort();
        SelectEvent( t );
        t->Selected = true;
      }

    }
    Project.MusicData.TimelineDragged = false;

    if ( Dragmode == DRAGMODE_NEWEVENT || Dragmode == DRAGMODE_TIMESEEK )
      Dragmode = DRAGMODE_NONE;
    return true;
    break;
  case WBM_MIDDLEBUTTONUP:
    App->ReleaseCapture();
    Dragmode = DRAGMODE_NONE;
    return true;
    break;
  case WBM_LEFTBUTTONUP:
    if ( Dragmode == DRAGMODE_SELECT )
    {
      TS32 sp = ClientToFrame( ScreenToClient( mousePos ).x );
      TS32 x1 = max( 0, min( ClickedFrame, sp ) );
      TS32 x2 = max( 0, max( ClickedFrame, sp ) );

      TS32 cp = ( ScreenToClient( mousePos ) - p.TopLeft() ).y / LineHeight + OffsetPass;
      TS32 y1 = max( OffsetPass, min( ClickedPass, cp ) );
      TS32 y2 = max( OffsetPass, max( ClickedPass, cp ) );

      CRect dragrect = CRect( x1, y1, x2, y2 );

      for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
        if ( Project.Timeline->Events[ x ]->Intersects( dragrect ) )
          Project.Timeline->Events[ x ]->Selected = true;

    }

    Project.MusicData.TimelineDragged = false;
    App->ReleaseCapture();
    Dragmode = DRAGMODE_NONE;
    return true;
    break;

  case WBM_LEFTBUTTONDBLCLK:
  {
    CphxEvent_Tool *event = Project.GetEvent( SelectedEvent );
    if ( event && event->GetEventType() == EVENT_RENDERSCENE )
    {
      CphxEvent_RenderScene_Tool* scn = (CphxEvent_RenderScene_Tool*)event;
      if ( scn->Scene )
        Root->GoToScene( scn->Scene, !App->GetCtrlState() );
    }
  }
  break;

  case WBM_COMMAND:
    if ( EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( true );
      return true;
    }
    break;

  case WBM_FOCUSLOST:
    if ( EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( false );
      return true;
    }
    break;

  case WBM_KEYDOWN:
    if ( !InFocus() ) break;
    if ( Message.KeyboardState&WB_KBSTATE_ALT && Message.KeyboardState&WB_KBSTATE_CTRL )
      break; //altgr

    if ( Message.Key == VK_ESCAPE && EditBox )
    {
      StopRenaming( false );
    }

    switch ( Message.Key )
    {
    case VK_F2:
      StartRenaming();
      break;
    case VK_DELETE:
      DeleteSelectedEvents();
      return true;
      break;
    }


    break;

  default:
    break;
  }

  return CWBItem::MessageProc( Message );
}

CRect CWBTimeline::GetEventPosition( CphxEvent_Tool *Event )
{
  //CRect p=GetClientRect()+GridRect;

  //CRect out;
  //out.y1=Event->Pass*LineHeight;
  //out.y2=(Event->Pass+1)*LineHeight+1;
  //out.x1=FrameToClient(Event->Event->StartFrame);
  //out.x2=FrameToClient(Event->Event->EndFrame);

  //out+=p.TopLeft();

  return GetEventPosition( FrameToClient( Event->Event->StartFrame ), FrameToClient( Event->Event->EndFrame ), Event->Pass );
}

CRect CWBTimeline::GetEventPosition( TS32 x1, TS32 x2, TS32 pass )
{
  CRect p = GetClientRect() + GridRect;
  return CRect( x1, ( pass - OffsetPass )*LineHeight, x2 + 1, ( pass - OffsetPass + 1 )*LineHeight + 1 ) + p.TopLeft();
}

void CWBTimeline::DrawEvent( CWBDrawAPI *API, CphxEvent_Tool *Event, TBOOL ForceSelect )
{
  CRect pos = GetEventPosition( Event );
  if ( !pos.Intersects( GetClientRect() ) ) return; //early exit

  CRect cr = API->GetCropRect();

  API->DrawRect( pos, Event->Color );

  CPoint mp = ScreenToClient( App->GetMousePos() );

  CColor border = ( Event->Selected || ForceSelect ) ? CColor::FromARGB( 0xfffcd400 ) : CColor::FromARGB( 0xff1e1e00 );
  if ( Event->exportProblem )
    border = CColor::FromARGB( 0xffff0000 );

  API->DrawRectBorder( pos, border );

  API->SetCropRect( cr | ( ClientToScreen( pos ) + CRect( -1, -1, -1, -1 ) ) );

  CWBFont *f = GetFont( GetState() );

  CString N = Event->GetName();

  pos = pos | GetClientRect();
  CPoint p = f->GetCenter( N, pos, WBTT_UPPERCASE );
  p.x = pos.x1 + 4;

  f->Write( API, N, p, 0xffffffff, WBTT_UPPERCASE );

  API->SetCropRect( cr );

  //resize cursor

  if ( Dragmode == DRAGMODE_NONE && MouseOver() && !App->GetShiftState() )
  {
    if ( pos.Contains( mp ) )
    {
      TS32 resizewidth = min( 4, max( 0, pos.Width() - 1 ) / 2 );
      if ( resizewidth )
      {
        CRect leftresize = CRect( pos.TopLeft(), pos.BottomLeft() + CPoint( resizewidth, 0 ) );
        CRect rightresize = CRect( pos.TopRight() - CPoint( resizewidth, 0 ), pos.BottomRight() );

        if ( leftresize.Contains( mp ) || rightresize.Contains( mp ) )
          App->SelectMouseCursor( CM_SIZEWE );

      }
    }
  }

}

TS32 CWBTimeline::FrameToClient( TS32 c )
{
  return (TS32)( ( c - Offset ) / Zoom );
}

TS32 CWBTimeline::ClientToFrame( TS32 c )
{
  return (TS32)( c*Zoom + Offset );
}

void CWBTimeline::DoEventDragging( CPoint mousePos )
{
  CArray<int> snapGrid;

  if ( App->GetAltState() )
  {
    //TS32 cp = ClientToFrame(mx);
    CRect p = GetClientRect() + GridRect;

    if ( !Project.MusicData.TickData.NumItems() )
    {
      TF32 TickLength = 60.0f / Project.Timeline->GridBPM*(TF32)Project.Timeline->Timeline->FrameRate / Zoom;
      TS32 TicksTooSmall = (TS32)( 16 / TickLength );
      TS32 skip = 1;
      for ( ; TicksTooSmall; TicksTooSmall = TicksTooSmall >> 1 )
        skip <<= 1;

      TF32 bpmoff = Offset / (TF32)Project.Timeline->Timeline->FrameRate / 60.0f;
      bpmoff = bpmoff - fmod( bpmoff, 1 / Project.Timeline->GridBPM );

      TS32 bpmstart = (TS32)( bpmoff*Project.Timeline->GridBPM );

      TF32 endframe = ( Offset + p.Width()*Zoom ) / (TF32)Project.Timeline->Timeline->FrameRate / 60.0f;
      TS32 bpmend = (TS32)( endframe*Project.Timeline->GridBPM ) + 1;

      for ( TS32 x = bpmstart; x <= bpmend; x++ )
      {
        if ( x%skip ) continue;
        TS32 f = (TS32)( x / Project.Timeline->GridBPM * 60 * Project.Timeline->Timeline->FrameRate + 0.5 );
        snapGrid += f;
      }
    }
    else
    { //use tick data

      TU32 startoff = Offset;
      TU32 endoff = ClientToFrame( p.x2 );

      TF32 TickLength = Project.MusicData.TickData.Last() / (TF32)Project.MusicData.TickData.NumItems() / 1000.0f*(TF32)Project.Timeline->Timeline->FrameRate / Zoom;
      TS32 TicksTooSmall = (TS32)( 16 / TickLength );
      TS32 skip = 1;
      for ( ; TicksTooSmall; TicksTooSmall = TicksTooSmall >> 1 )
        skip <<= 1;

      for ( TS32 x = 0; x < Project.MusicData.TickData.NumItems(); x++ )
      {
        if ( Project.MusicData.TickData[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate >= startoff )
        {
          if ( x%skip ) continue;
          TS32 f = (TS32)( Project.MusicData.TickData[ x ] / 1000.0f*(TF32)Project.Timeline->Timeline->FrameRate );
          snapGrid += f;
        }

        if ( Project.MusicData.TickData[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate > endoff ) break;
      }

    }

  }

  CPoint mp = ScreenToClient( mousePos );

  TS32 p1 = ( LastMousePos.y ) / LineHeight + OffsetPass;
  TS32 p2 = ( mp.y ) / LineHeight + OffsetPass;
  TS32 f1 = ClientToFrame( LastMousePos.x );
  TS32 f2 = ClientToFrame( mp.x );

  //if (!wh->RightButton && Project.Timeline->RightUndo)
  //{
  //	Project.Timeline->RightUndo=false;
  //	p1=(wh->lby-r.y1)/LineHeight+OffsetPass;
  //	p2=(mp.y-r.y1)/LineHeight+OffsetPass;
  //	f1=(TS32)((wh->lbx-r.x1)/Zoom+OffFrame);
  //	f2=(TS32)(p/Zoom+OffFrame);
  //}

  TS32 passdelta = p2 - p1 + RemainingPasses;
  TS32 framedelta = f2 - f1 + RemainingFrames;
  TS32 passd = 0;
  if ( passdelta != 0 ) passd = passdelta / abs( passdelta );
  TS32 framed = 0;
  if ( framedelta != 0 ) framed = framedelta / abs( framedelta );
  TS32 resultframedelta = 0, resultframepass = 0;
  TS32 x = passdelta + passd;

  //LOG.PrintF("%d %d %d %d",passdelta,passd,framedelta,framed);

  TBOOL SolutionFound = false;
  do
  {
    x -= passd;

    for ( TS32 z = 0; z < Project.Timeline->Events.NumItems(); z++ )
      if ( Project.Timeline->Events[ z ]->Selected )
      {
        TS32 p = Project.Timeline->Events[ z ]->Pass + x;
        if ( p < 0 ) x = -Project.Timeline->Events[ z ]->Pass;
      }

    TS32 y = framedelta + framed;
    do
    {
      y -= framed;

      int snapDeviation = 0;

      if ( App->GetAltState() )
      {
        int snapDistance = 200;
        int snapState = 0;
        int snapFrame = 0;
        CphxEvent_Tool *snapper1 = nullptr;
        CphxEvent_Tool *snapper2 = nullptr;

        for ( TS32 z = 0; z < Project.Timeline->Events.NumItems(); z++ )
          if ( Project.Timeline->Events[ z ]->Selected )
          {
            CphxEvent_Tool *e1 = Project.Timeline->Events[ z ];

            for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
              if ( !Project.Timeline->Events[ x ]->Selected )
              {
                CphxEvent_Tool *e2 = Project.Timeline->Events[ x ];

                int oldSnapDist = snapDistance;
                snapDistance = min( snapDistance, abs( e1->Event->StartFrame + y - e2->Event->StartFrame ) );
                snapDistance = min( snapDistance, abs( e1->Event->EndFrame + y - e2->Event->EndFrame ) );
                snapDistance = min( snapDistance, abs( e1->Event->StartFrame + y - e2->Event->EndFrame ) );
                snapDistance = min( snapDistance, abs( e1->Event->EndFrame + y - e2->Event->StartFrame ) );

                if ( snapDistance != oldSnapDist )
                {
                  snapper1 = e1;
                  snapper2 = e2;
                }
              }
          }

        for ( TS32 z = 0; z < Project.Timeline->Events.NumItems(); z++ )
          if ( Project.Timeline->Events[ z ]->Selected )
          {
            CphxEvent_Tool *e1 = Project.Timeline->Events[ z ];

            for ( TS32 x = 0; x < snapGrid.NumItems(); x++ )
            {
              if ( abs( e1->Event->StartFrame + y - snapGrid[ x ] ) < snapDistance )
              {
                snapper1 = e1;
                snapper2 = nullptr;
                snapDistance = abs( e1->Event->StartFrame + y - snapGrid[ x ] );
                snapFrame = snapGrid[ x ];
              }
              if ( abs( e1->Event->EndFrame + y - snapGrid[ x ] ) < snapDistance )
              {
                snapper1 = e1;
                snapper2 = nullptr;
                snapDistance = abs( e1->Event->EndFrame + y - snapGrid[ x ] );
                snapFrame = snapGrid[ x ];
              }
            }
          }

        if ( (int)( snapDistance / Zoom ) <= 5 && snapper1 && snapper2 )
        {
          if ( abs( snapper1->Event->StartFrame + y - snapper2->Event->StartFrame ) == snapDistance )
            snapDeviation = snapper1->Event->StartFrame + y - snapper2->Event->StartFrame;
          else
            if ( abs( snapper1->Event->EndFrame + y - snapper2->Event->EndFrame ) == snapDistance )
              snapDeviation = snapper1->Event->EndFrame + y - snapper2->Event->EndFrame;
            else
              if ( abs( snapper1->Event->StartFrame + y - snapper2->Event->EndFrame ) == snapDistance )
                snapDeviation = snapper1->Event->StartFrame + y - snapper2->Event->EndFrame;
              else
                if ( abs( snapper1->Event->EndFrame + y - snapper2->Event->StartFrame ) == snapDistance )
                  snapDeviation = snapper1->Event->EndFrame + y - snapper2->Event->StartFrame;
        }

        if ( (int)( snapDistance / Zoom ) <= 5 && snapper1 && !snapper2 )
        {
          if ( abs( snapper1->Event->StartFrame + y - snapFrame ) == snapDistance )
            snapDeviation = snapper1->Event->StartFrame + y - snapFrame;
          else
            if ( abs( snapper1->Event->EndFrame + y - snapFrame ) == snapDistance )
              snapDeviation = snapper1->Event->EndFrame + y - snapFrame;
        }
      }

      y -= snapDeviation;

      TBOOL Allfit = true;
      for ( TS32 z = 0; z < Project.Timeline->Events.NumItems() && Allfit; z++ )
        if ( Project.Timeline->Events[ z ]->Selected )
        {
          TS32 sf, ef, p;
          sf = Project.Timeline->Events[ z ]->Event->StartFrame + y;
          ef = Project.Timeline->Events[ z ]->Event->EndFrame + y;
          p = Project.Timeline->Events[ z ]->Pass + x;

          if ( p < 0 || sf < 0 ) Allfit = false;

          for ( TS32 w = 0; w < Project.Timeline->Events.NumItems() && Allfit; w++ )
            if ( !Project.Timeline->Events[ w ]->Selected && p == Project.Timeline->Events[ w ]->Pass )
            {
              TS32 s = Project.Timeline->Events[ w ]->Event->StartFrame;
              TS32 e = Project.Timeline->Events[ w ]->Event->EndFrame;
              if ( ( sf >= s && sf < e ) || ( ef > s && ef <= e ) || ( sf <= s && ef >= e ) ) Allfit = false; //collision testing
            }
        }
      if ( Allfit )
      {
        resultframedelta = y;
        resultframepass = x;
        RemainingFrames = framedelta - resultframedelta;
        RemainingPasses = passdelta - resultframepass;
        SolutionFound = true;
      }

      y += snapDeviation;
    } while ( !( y == 0 || SolutionFound ) );
  } while ( !( x == 0 || SolutionFound ) );

  //if (resultframedelta!=0 || resultframepass!=0)
  for ( TS32 z = 0; z < Project.Timeline->Events.NumItems(); z++ )
    if ( Project.Timeline->Events[ z ]->Selected )
    {
      //if (!wh->RightButton)
      //{
      Project.Timeline->Events[ z ]->Event->StartFrame += resultframedelta;
      Project.Timeline->Events[ z ]->Event->EndFrame += resultframedelta;
      Project.Timeline->Events[ z ]->Pass += resultframepass;
      //}
      //else
      //{
      //	Project.Timeline->Events[z]->Event->StartFrame=Project.Timeline->Events[z]->OldStartframe;
      //	Project.Timeline->Events[z]->Event->EndFrame=Project.Timeline->Events[z]->OldEndframe;
      //	Project.Timeline->Events[z]->Pass=Project.Timeline->Events[z]->OldPass;           
      //}
    }

  //if (wh->RightButton) Project.Timeline->RightUndo=true;
  LastMousePos = mp;

  Project.Timeline->Sort();

}

TS32 CWBTimeline::GetCursorSnap( TS32 original, CphxEvent_Tool *Skipped, CPoint mousePos )
{
  TS32 mx = ScreenToClient( mousePos ).x;
  TS32 cp = ClientToFrame( mx );
  CRect p = GetClientRect() + GridRect;

  TS32 dist = 100000;
  TS32 mindistframe = cp;

  //snap to grid

  if ( !Project.MusicData.TickData.NumItems() )
  {
    TF32 TickLength = 60.0f / Project.Timeline->GridBPM*(TF32)Project.Timeline->Timeline->FrameRate / Zoom;
    TS32 TicksTooSmall = (TS32)( 16 / TickLength );
    TS32 skip = 1;
    for ( ; TicksTooSmall; TicksTooSmall = TicksTooSmall >> 1 )
      skip <<= 1;

    TF32 bpmoff = Offset / (TF32)Project.Timeline->Timeline->FrameRate / 60.0f;
    bpmoff = bpmoff - fmod( bpmoff, 1 / Project.Timeline->GridBPM );

    TS32 bpmstart = (TS32)( bpmoff*Project.Timeline->GridBPM );

    TF32 endframe = ( Offset + p.Width()*Zoom ) / (TF32)Project.Timeline->Timeline->FrameRate / 60.0f;
    TS32 bpmend = (TS32)( endframe*Project.Timeline->GridBPM ) + 1;

    for ( TS32 x = bpmstart; x <= bpmend; x++ )
    {
      if ( x%skip ) continue;
      TS32 f = (TS32)( x / Project.Timeline->GridBPM * 60 * Project.Timeline->Timeline->FrameRate + 0.5 );
      //TS32 f=(TS32)(x/GridBPM*60*Project.Timeline->Timeline->FrameRate/Zoom-Offset/Zoom);
      //return (TS32)((c-Offset)/Zoom);
      if ( abs( cp - f ) < dist )
      {
        mindistframe = f;
        dist = abs( cp - f );
      }
    }
  }
  else
  { //use tick data

    TU32 startoff = Offset;
    TU32 endoff = ClientToFrame( p.x2 );

    TF32 TickLength = Project.MusicData.TickData.Last() / (TF32)Project.MusicData.TickData.NumItems() / 1000.0f*(TF32)Project.Timeline->Timeline->FrameRate / Zoom;
    TS32 TicksTooSmall = (TS32)( 16 / TickLength );
    TS32 skip = 1;
    for ( ; TicksTooSmall; TicksTooSmall = TicksTooSmall >> 1 )
      skip <<= 1;

    for ( TS32 x = 0; x < Project.MusicData.TickData.NumItems(); x++ )
    {
      if ( Project.MusicData.TickData[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate >= startoff )
      {
        if ( x%skip ) continue;
        TS32 f = (TS32)( Project.MusicData.TickData[ x ] / 1000.0f*(TF32)Project.Timeline->Timeline->FrameRate );

        if ( abs( cp - f ) < dist )
        {
          mindistframe = f;
          dist = abs( cp - f );
        }
      }

      if ( Project.MusicData.TickData[ x ] / 1000.0f*Project.Timeline->Timeline->FrameRate > endoff ) break;
    }

  }

  //snap to events

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    if ( Project.Timeline->Events[ x ] != Skipped )
    {
      TS32 s = Project.Timeline->Events[ x ]->Event->StartFrame;
      TS32 e = Project.Timeline->Events[ x ]->Event->EndFrame;

      if ( abs( cp - s ) < dist )
      {
        mindistframe = s;
        dist = abs( cp - s );
      }

      if ( abs( cp - e ) < dist )
      {
        mindistframe = e;
        dist = abs( cp - e );
      }
    }

  if ( abs( FrameToClient( mindistframe ) - mx ) <= 5 )
    return mindistframe;

  return original;
}

void CWBTimeline::SelectEvent( CphxEvent_Tool *ev )
{
  CapexTimelineEventSplines *s = (CapexTimelineEventSplines *)GetActiveWorkBench()->GetWindow( apEx_TimelineEventSplines );
  if ( s ) s->UpdateSplines( ev );

  CapexTimelineEventParameters *p = (CapexTimelineEventParameters*)GetActiveWorkBench()->GetWindow( apEx_TimelineEventParameters );
  if ( p ) p->SetEditedEvent( ev );


  SelectedEvent.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  if ( ev ) SelectedEvent = ev->GetGUID();

  //App->SendMessage(CWBMessage(App, (WBMESSAGE)WBM_SELECTEVENT, GetGuid(), (TS32)ev));
}

void CWBTimeline::WriteWithOutline( CWBDrawAPI *API, CWBFont *Font, CString &text, CPoint Pos )
{
  for ( TS32 x = -1; x < 2; x++ )
    for ( TS32 y = -1; y < 2; y++ )
    {
      if ( x != 0 || y != 0 )
        Font->Write( API, text, Pos + CPoint( x, y ), 0xff000000, WBTT_UPPERCASE );
    }

  Font->Write( API, text, Pos, 0xffffffff, WBTT_UPPERCASE );
}

void CWBTimeline::DeleteSelectedEvents()
{
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    if ( Project.Timeline->Events[ x ]->Selected )
    {
      Project.Timeline->DeleteEventByIndex( x );
      x--;
    }
  SelectEvent( NULL );
}

CphxEvent_Tool * CWBTimeline::GetSelectedEvent()
{
  return Project.GetEvent( SelectedEvent );
}

void CWBTimeline::StartRenaming()
{
  CWBTimeline *t = (CWBTimeline*)FindChildByID( _T( "timeline" ), _T( "timelineeditorphx" ) );
  if ( !t ) return;

  if ( !GetSelectedEvent() ) return;

  CphxEvent_Tool *Op = GetSelectedEvent();

  CRect dr = t->GetEventPosition( Op ) | GetClientRect();
  if ( !dr.Intersects( GetClientRect() ) ) return; //early exit
  if ( EditBox ) SAFEDELETE( EditBox );

  //TS32 ItemHeight=FixItemHeight;
  //CPoint Offset=-CPoint(GetHScrollbarPos(),GetVScrollbarPos());
  //CPoint p=GetClientRect().TopLeft()+Offset;
  //p+=CPoint(0,ItemHeight*CursorPosition);
  //CRect r=CRect(p,CPoint((GetClientRect()+Offset).x2,p.y+ItemHeight));

  CWBTextBox *b = new CWBTextBox( this, dr );
  b->ApplyStyleDeclarations( _T( "border:1px; padding-left:3px; padding-top:-2px; top:0px; border-color:#fffcd400; background:#ff414141;" ) );
  App->ApplyStyle( b );

  b->SetText( Op->GetName() );
  b->SetFocus();
  b->EnableHScrollbar( false, false );
  b->EnableVScrollbar( false, false );
  //b->SetBackgroundColor(rgbBackgroundSelectedFocus);
  b->SetSelection( 0, b->GetText().Length() );

  EditBox = b;
}

void CWBTimeline::StopRenaming( TBOOL ApplyChanges )
{
  if ( !GetSelectedEvent() ) return;
  if ( !EditBox ) return;

  CphxEvent_Tool *EditedEvent = GetSelectedEvent();

  if ( ApplyChanges )
  {
    CWBTextBox *ib = (CWBTextBox*)EditBox;
    EditedEvent->SetName( ib->GetText() );
  }

  SAFEDELETE( EditBox );
}