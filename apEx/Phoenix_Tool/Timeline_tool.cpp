#define _WINSOCKAPI_
#include "BasePCH.h"
#include "phxSplineExt.h"
#include "../Phoenix/Timeline.h"
#include "Timeline_tool.h"
#include "apxProject.h"
#include "../apEx/apExRoot.h"

CphxEvent_Tool::CphxEvent_Tool()
{
  Event = NULL;
  Pass = 0;
  Selected = false;
  Color = CColor::FromARGB( 0xff094167 );
  Type = EVENT_ENDDEMO;
  //TargetID=0;
  Time = new CphxSpline_Tool_float16();
}

CString CphxEvent_Tool::GetName()
{
  return Name;
}

CphxEvent_Tool::~CphxEvent_Tool()
{
  SAFEDELETE( Time );
  SAFEDELETE( Event );
}

TBOOL CphxEvent_Tool::Intersects( CRect &r )
{
  if ( !( Pass >= r.y1 && Pass <= r.y2 ) ) return false;
  return IntervalIntersection( Event->StartFrame, Event->EndFrame, r.x1, r.x2 );
}

void CphxEvent_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Type" ), false ).SetInt( Type );
  Node->AddChild( _T( "Pass" ), false ).SetInt( Pass );
  Node->AddChild( _T( "StartFrame" ), false ).SetInt( Event->StartFrame );
  Node->AddChild( _T( "EndFrame" ), false ).SetInt( Event->EndFrame );
  Node->AddChild( _T( "TargetRT" ), false ).SetText( TargetID.GetString() );
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  CXMLNode n = Node->AddChild( _T( "TimeSpline" ) );
  Time->ExportData( n );
  ExportEventData( Node );
}

TS32 RemovedDuplicateKeyCount = 0;

void CphxEvent_Tool::ImportData( CXMLNode *Node )
{
  TS32 x = Type;
  if ( Node->GetChildCount( _T( "Type" ) ) ) Node->GetChild( _T( "Type" ) ).GetValue( x );
  Type = (PHXEVENTTYPE)x;
  if ( Node->GetChildCount( _T( "Pass" ) ) ) Node->GetChild( _T( "Pass" ) ).GetValue( Pass );
  x = Event->StartFrame;
  if ( Node->GetChildCount( _T( "StartFrame" ) ) ) Node->GetChild( _T( "StartFrame" ) ).GetValue( x );
  Event->StartFrame = x;
  x = Event->EndFrame;
  if ( Node->GetChildCount( _T( "EndFrame" ) ) ) Node->GetChild( _T( "EndFrame" ) ).GetValue( x );
  Event->EndFrame = x;

  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  if ( Node->GetChildCount( _T( "TimeSpline" ) ) )
  {
    Time->Keys.FreeArray();
    Time->ImportData( Node->GetChild( _T( "TimeSpline" ) ) );

    //hack to kill duplicate keys
    //for (TS32 i = 0; i < Time->Keys.NumItems(); i++)
    //{
    //	for (TS32 j = i + 1; j < Time->Keys.NumItems(); j++)
    //	{
    //		if (Time->Keys[i]->Key.Value[0] == Time->Keys[j]->Key.Value[0] &&
    //			Time->Keys[i]->Key.t == Time->Keys[j]->Key.t)
    //		{
    //			RemovedDuplicateKeyCount++;
    //			Time->Keys.FreeByIndex(j);
    //			j--;
    //		}
    //	}
    //}

  }

  if ( Node->GetChildCount( _T( "TargetRT" ) ) ) TargetID.SetString( Node->GetChild( _T( "TargetRT" ), false ).GetText().GetPointer() );

  ImportEventData( Node );
}

TBOOL CphxEvent_Tool::GenerateResource( CCoreDevice *Dev )
{
  return true;
}

CphxSpline_Tool_float16 * CphxEvent_Tool::GetSpline( TS32 x, CString &SplineName, CColor &SplineColor )
{
  SplineName = _T( "Time Envelope" );
  SplineColor = 0xffffffff;
  return Time;
}

void CphxEvent_Tool::SetName( CString &s )
{
  Name = s;
}

CphxTimeline_Tool::CphxTimeline_Tool()
{
  Timeline = new CphxTimeline;
  Timeline->Target = NULL;
  Timeline->EventCount = 0;
  Timeline->Events = NULL;
  Timeline->FrameRate = 120;
  Timeline->AspectX = 16;
  Timeline->AspectY = 9;
  Timeline->RenderTargets = NULL;
  Timeline->RenderTargetCount = 0;
}

CphxTimeline_Tool::~CphxTimeline_Tool()
{
  SAFEDELETE( Timeline->RenderTargets );

  SAFEDELETEA( Timeline->Events );

  Events.FreeArray();
  SAFEDELETE( Timeline );
}

CphxEvent_Tool * CphxTimeline_Tool::CreateEvent( PHXEVENTTYPE Type, TS32 StartFrame, TS32 EndFrame, TS32 Pass, CphxMaterialTechnique_Tool *Tech )
{
  CphxEvent_Tool *e = NULL;

  switch ( Type )
  {
  case EVENT_ENDDEMO:
    e = new CphxEvent_EndDemo_Tool;
    break;
  case EVENT_RENDERDEMO:
    e = new CphxEvent_RenderDemo_Tool;
    break;
  case EVENT_SHADERTOY:
  {
    CphxEvent_Shadertoy_Tool *h;
    h = new CphxEvent_Shadertoy_Tool();
    h->SetTech( Tech );
    h->UpdateMaterialState();
    e = h;
  }
  break;
  case EVENT_RENDERSCENE:
    e = new CphxEvent_RenderScene_Tool;
    break;
  case EVENT_PARTICLECALC:
    e = new CphxEvent_ParticleCalc_Tool;
    break;
  case EVENT_CAMERASHAKE:
    e = new CphxEvent_CameraShake_Tool;
    break;
  case EVENT_CAMERAOVERRIDE:
    e = new CphxEvent_CameraOverride_Tool;
    break;
  default:
    return NULL;
    break;
  }

  e->Pass = Pass;
  e->Event->StartFrame = StartFrame;
  e->Event->EndFrame = EndFrame;
  e->Event->Target = NULL;
  if ( Project.GetRenderTargetByIndex( 0 ) )
    e->TargetID = Project.GetRenderTargetByIndex( 0 )->GetGUID();

  e->Time->AddKey( 0, 0 );
  e->Time->AddKey( 1, 1 );

  Events += e;

  SAFEDELETEA( Timeline->Events );
  Timeline->EventCount = Events.NumItems();
  Timeline->Events = new CphxEvent *[ Timeline->EventCount ];

  for ( TS32 x = 0; x < Timeline->EventCount; x++ )
    Timeline->Events[ x ] = Events[ x ]->Event;

  //Project.ApplyRenderTargets();
  return e;
}

void CphxTimeline_Tool::DeleteEventByIndex( TS32 x )
{
  Events.FreeByIndex( x );

  //rebuild event list
  SAFEDELETEA( Timeline->Events );
  Timeline->EventCount = Events.NumItems();
  Timeline->Events = new CphxEvent *[ Timeline->EventCount ];
  for ( TS32 y = 0; y < Timeline->EventCount; y++ )
    Timeline->Events[ y ] = Events[ y ]->Event;
}

void CphxTimeline_Tool::Render( float Frame, TBOOL RenderAsSubroutine )
{
  if ( !Timeline ) return;

  TBOOL UptoDate = true;

  for ( TS32 x = 0; x < Events.NumItems(); x++ )
  {
    if ( Events[ x ]->Event->StartFrame <= Frame && Events[ x ]->Event->EndFrame >= Frame )
    {
      if ( !Events[ x ]->ContentReady() )
      {
        Events[ x ]->RequestContent();
        UptoDate = false;
      }

      if ( Events[ x ]->Type == EVENT_RENDERSCENE )
      {
        CphxScene_Tool *s = ( (CphxEvent_RenderScene_Tool*)Events[ x ] )->Scene;
        if ( !s ) continue;
        s->ForceUpdateContent();
      }


      if ( Events[ x ]->Type == EVENT_SHADERTOY )
      {
        CphxEvent_Shadertoy_Tool *s = (CphxEvent_Shadertoy_Tool*)Events[ x ];
        s->UpdateMaterialState();
      }

      if ( Events[ x ]->Type == EVENT_PARTICLECALC )
      {
        CphxEvent_ParticleCalc_Tool *p = (CphxEvent_ParticleCalc_Tool*)Events[ x ];
      }

      if ( Events[ x ]->Type == EVENT_CAMERAOVERRIDE )
      {
        CphxEvent_CameraOverride_Tool* p = (CphxEvent_CameraOverride_Tool*)Events[ x ];
      }

    }
  }

  TS32 oldeventcount = Timeline->EventCount;
  TS32 neweventcount = 0;

  if ( Project.LastPassToDraw >= 0 )
  {
    for ( TS32 x = 0; x < Events.NumItems(); x++ )
      if ( Events[ x ]->Pass <= Project.LastPassToDraw )
        neweventcount++;
  }
  else neweventcount = oldeventcount;

  Timeline->EventCount = neweventcount;


#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif
  if ( UptoDate )
    Timeline->Render( Frame, true, RenderAsSubroutine != 0 );
#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif

  Timeline->EventCount = oldeventcount;
}

TS32 EventSorter( CphxEvent_Tool **a, CphxEvent_Tool **b )
{
  if ( !( a && b && *a && *b ) ) return 0;

  if ( ( *a )->Pass == ( *b )->Pass ) return ( *a )->Event->StartFrame - ( *b )->Event->StartFrame;

  return ( *a )->Pass - ( *b )->Pass;
}

void CphxTimeline_Tool::Sort()
{
  Events.Sort( EventSorter );

  SAFEDELETEA( Timeline->Events );
  Timeline->EventCount = Events.NumItems();
  Timeline->Events = new CphxEvent *[ Timeline->EventCount ];
  for ( TS32 x = 0; x < Timeline->EventCount; x++ )
    Timeline->Events[ x ] = Events[ x ]->Event;

}

CphxEvent_EndDemo_Tool::CphxEvent_EndDemo_Tool() : CphxEvent_Tool()
{
  Event = new CphxEvent_EndDemo();
  Event->Time = (CphxSpline_float16*)Time->Spline;
  Type = EVENT_ENDDEMO;
  //Color=CColor::FromARGB(0xfff2806a);
  Color = CColor::FromARGB( 0xff995143 );
}

CphxEvent_EndDemo_Tool::~CphxEvent_EndDemo_Tool()
{
}

CString CphxEvent_EndDemo_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  return _T( "End Demo" );
}

CphxEvent_RenderDemo_Tool::CphxEvent_RenderDemo_Tool() : CphxEvent_Tool()
{
  Event = new CphxEvent_RenderDemo();
  Event->Time = (CphxSpline_float16*)Time->Spline;
  Type = EVENT_RENDERDEMO;
  //Color=CColor::FromARGB(0xffb975b5);
  Color = CColor::FromARGB( 0xff754a72 );
}

CphxEvent_RenderDemo_Tool::~CphxEvent_RenderDemo_Tool()
{

}

CString CphxEvent_RenderDemo_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  return _T( "Render Demo" );
}

CphxEvent_Shadertoy_Tool::CphxEvent_Shadertoy_Tool() : CphxEvent_Tool()
{
  CphxEvent_Shadertoy *e = new CphxEvent_Shadertoy();
  //e->RSBatch = NULL;

  Event = e;
  //e->ShaderParams.LinkCount = 0;
  //e->ShaderParams.Links = NULL;
  Event->Time = (CphxSpline_float16*)Time->Spline;
  Type = EVENT_SHADERTOY;
  //Color=CColor::FromARGB(0xff7ac1ff);
  Color = CColor::FromARGB( 0xff4d7aa1 );
  Tech = NULL;
  MaterialStateSize = 0;
}

CphxEvent_Shadertoy_Tool::~CphxEvent_Shadertoy_Tool()
{
  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;
  //if (e) SAFEDELETE(e->ShaderParams.Links);

  FreeMaterialState();
  ClearTech();
}

CString CphxEvent_Shadertoy_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  if ( Tech ) return Tech->Name;
  return _T( "Shader Effect" );
}

void CphxEvent_Shadertoy_Tool::FreeMaterialState()
{
  if ( !Tech ) return;
  if ( !Event ) return;

  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;

  if ( !e->MaterialState ) return;
  if ( !MaterialStateSize ) return;

  for ( TS32 x = 0; x < MaterialStateSize; x++ )
  {
    //need to recast here because the minimal engine version of this class has no destructor - hence no virtual destructor call
    CphxMaterialPassConstantState_Tool *t = (CphxMaterialPassConstantState_Tool*)e->MaterialState[ x ];
    SAFEDELETE( t );
    e->MaterialState[ x ] = NULL;
  }

  SAFEDELETEA( e->MaterialState );
  MaterialStateSize = 0;
}

void CphxEvent_Shadertoy_Tool::AllocateMaterialState()
{
  FreeMaterialState();

  if ( !Tech ) return;
  if ( !Event ) return;

  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;

  MaterialStateSize = 0;
  e->MaterialState = NULL;

  MaterialStateSize += e->Tech->PassCount;

  if ( MaterialStateSize )
  {
    e->MaterialState = new CphxMaterialPassConstantState*[ MaterialStateSize ];
    for ( TS32 x = 0; x < MaterialStateSize; x++ )
      e->MaterialState[ x ] = new CphxMaterialPassConstantState_Tool();
  }

}

void CphxEvent_Shadertoy_Tool::UpdateMaterialState()
{
  AllocateMaterialState();

  if ( !Tech ) return;
  if ( !Event ) return;

  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;

  MaterialData.UpdateParams( Tech );
  UpdateMaterialTextures();

  //apply parameters to material instance

  TS32 passcnt = 0;
  Tech->CreateInstancedData( e->MaterialState, passcnt );
}

void CphxEvent_Shadertoy_Tool::UpdateMaterialTextures()
{
  if ( !Tech ) return;
  if ( !Event ) return;

  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;

  MaterialData.UpdateTextures( Tech );

  TS32 passcnt = 0;
  Tech->CreateInstancedData_Textures( e->MaterialState, passcnt );
}


void CphxEvent_Shadertoy_Tool::ExportEventData( CXMLNode *Node )
{
  if ( Tech )
  {
    Node->AddChild( _T( "Tech" ) ).SetText( Tech->GetGUID().GetString() );
    MaterialData.ExportData( &Node->AddChild( _T( "TechData" ) ) );
  }

  for ( TS32 x = 0; x < ShaderSplines.Splines.NumItems(); x++ )
  {
    if ( ShaderSplines.Splines[ x ]->Spline.Target->Scope == PARAM_ANIMATED )
    {
      CXMLNode n = Node->AddChild( _T( "ParamData" ) );
      n.AddChild( _T( "TargetGUID" ) ).SetText( ShaderSplines.Splines[ x ]->TargetParamGUID.GetString() );
      CXMLNode s = n.AddChild( _T( "Spline" ) );
      ShaderSplines.Splines[ x ]->Splines[ 0 ].ExportData( s );
      if ( ShaderSplines.Splines[ x ]->Spline.Target->Type == PARAM_COLOR )
      {
        for ( TS32 y = 1; y < 4; y++ )
        {
          s = n.AddChild( _T( "Spline" ) );
          ShaderSplines.Splines[ x ]->Splines[ y ].ExportData( s );
        }
      }
    }
  }

}

void CphxEvent_Shadertoy_Tool::ImportEventData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Tech" ) ) )
  {
    CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;
    CphxGUID g;
    CXMLNode t = Node->GetChild( _T( "Tech" ) );
    g.SetString( t.GetText().GetPointer() );
    CphxMaterialTechnique_Tool *te = Project.GetTech( g );

    SetTech( te );

    if ( Node->GetChildCount( _T( "TechData" ) ) )
      MaterialData.ImportData( &Node->GetChild( _T( "TechData" ) ) );

    for ( TS32 x = 0; x < Node->GetChildCount( _T( "ParamData" ) ); x++ )
    {
      CXMLNode n = Node->GetChild( _T( "ParamData" ), x );
      if ( !n.GetChildCount( _T( "TargetGUID" ) ) ) continue;

      CphxGUID Target;
      Target.SetString( n.GetChild( _T( "TargetGUID" ) ).GetText().GetPointer() );

      TBOOL Found = false;
      for ( TS32 y = 0; y < ShaderSplines.Splines.NumItems(); y++ )
      {
        if ( ShaderSplines.Splines[ y ]->TargetParamGUID == Target )
        {
          Found = true;

          for ( TS32 z = 0; z < n.GetChildCount( _T( "Spline" ) ); z++ )
            ShaderSplines.Splines[ y ]->Splines[ z ].ImportData( n.GetChild( _T( "Spline" ), z ) );

          break;
        }
      }

      if ( !Found )
        LOG_ERR( "[import] Event data import error: referenced parameter %s not found", Target.GetString() );

    }

    UpdateMaterialState();
  }
}

void CphxEvent_Shadertoy_Tool::SetTech( CphxMaterialTechnique_Tool *t, TBOOL Update )
{
  if ( t == Tech ) return;
  RemoveParent( Tech );

  InvalidateUptoDateFlag();

  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;

  ClearTech();

  Tech = t;
  if ( !t ) return;

  e->Tech = t->Tech;
  if ( t )
  {
    if ( !Update )
      MaterialData.SetDefaultValues( t );
    else
      MaterialData.SetMissingDefaultValues( t );
  }

  if ( t->Tech )
  {
    ShaderSplines.BackupSplines();
    ShaderSplines.BuildBatch( this );
    ShaderSplines.RestoreSplines();
  }

  //if (t->Tech)
  //{
  //	//e->RSBatch = t->Tech->CreateRSBatch();
  //	//ShaderParams.BackupParamValues();
  //	//ShaderParams.BuildBatch(Tech, &e->ShaderParams, true, true);
  //	//ShaderParams.RestoreParamValues();
  //	//ShaderParams.BuildLinkBatch(&e->ShaderParams);
  //}

  AddParent( t );
  //UpdateMaterialState();

}

void CphxEvent_Shadertoy_Tool::ClearTech()
{
  CphxEvent_Shadertoy *e = (CphxEvent_Shadertoy*)Event;

#ifdef MEMORY_TRACKING
  TBOOL old = memTracker.SetMissingIgnore( true );
#endif

  //if (e && e->RSBatch)
  //{
  //	if (e->RSBatch->TechLevelParams) e->RSBatch->TechLevelParams->Release();
  //	for (TS32 x = 0; x < e->Tech->PassCount; x++)
  //		if (e->RSBatch->PassParams[x]) e->RSBatch->PassParams[x]->Release();
  //	SAFEDELETE(e->RSBatch->PassParams);
  //	SAFEDELETE(e->RSBatch);
  //}

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( old );
#endif

  if ( e ) e->Tech = NULL;
  Tech = NULL;
}

TBOOL CphxEvent_Shadertoy_Tool::GenerateResource( CCoreDevice *Dev )
{
  SetTech( Tech );
  UpdateMaterialState();

  UpdateWindowData( apEx_TimelineEventSplines );

  return true;
}

TBOOL CphxEvent_Shadertoy_Tool::GenerateResource()
{
  UpdateMaterialState();
  return true;
}

TS32 CphxEvent_Shadertoy_Tool::GetSplineCount()
{
  TS32 sc = 1;
  for ( TS32 x = 0; x < ShaderSplines.Splines.NumItems(); x++ )
  {
    if ( ShaderSplines.Splines[ x ]->Spline.Target->Scope == PARAM_ANIMATED )
    {
      switch ( ShaderSplines.Splines[ x ]->Spline.Target->Type )
      {
      case PARAM_FLOAT:
        sc += 1;
        break;
      case PARAM_COLOR:
        sc += 4;
        break;
      default:
        break;
      }
    }
  }
  return sc;
}

CphxSpline_Tool_float16 * CphxEvent_Shadertoy_Tool::GetSpline( TS32 id, CString &SplineName, CColor &SplineColor )
{
  if ( id == 0 )
    return CphxEvent_Tool::GetSpline( id, SplineName, SplineColor );

  TS32 sc = 1;
  for ( TS32 x = 0; x < ShaderSplines.Splines.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool *p = Project.GetMaterialParameter( ShaderSplines.Splines[ x ]->TargetParamGUID );

    if ( ShaderSplines.Splines[ x ]->Spline.Target->Scope == PARAM_ANIMATED )
    {
      switch ( ShaderSplines.Splines[ x ]->Spline.Target->Type )
      {
      case PARAM_FLOAT:
        if ( sc == id )
        {
          SplineName = p ? p->Name : _T( "INVALID" );
          SplineColor = 0xffffffff;
          return &ShaderSplines.Splines[ x ]->Splines[ 0 ];
        }
        sc += 1;
        break;
      case PARAM_COLOR:
        if ( id - sc >= 0 && id - sc < 4 )
        {
          CString postfix;
          switch ( id - sc )
          {
          case 0: postfix = _T( " Red" ); break;
          case 1: postfix = _T( " Green" ); break;
          case 2: postfix = _T( " Blue" ); break;
          case 3: postfix = _T( " Alpha" ); break;
          }

          SplineName = ( p ? p->Name : _T( "INVALID" ) ) + postfix;
          SplineColor = 0xffffff00;
          return &ShaderSplines.Splines[ x ]->Splines[ id - sc ];
        }
        sc += 4;
        break;
      default:
        break;
      }
    }
  }

  return NULL;
}

TBOOL CphxEvent_Shadertoy_Tool::CanWriteToRenderTarget( CphxRenderTarget_Tool *RT )
{
  return true;
}

CphxEvent_RenderScene_Tool::CphxEvent_RenderScene_Tool() : CphxEvent_Tool()
{
  CphxEvent_RenderScene *e = new CphxEvent_RenderScene();
  e->Scene = NULL;
  e->Camera = NULL;
  e->Clip = 0;
  e->ClearColor = true;
  e->ClearZ = true;
  Event = e;
  Event->Time = (CphxSpline_float16*)Time->Spline;
  Type = EVENT_RENDERSCENE;
  //Color=CColor::FromARGB(0xff8dd28a);
  Color = CColor::FromARGB( 0xff598557 );
  Scene = NULL;
  Clip = NULL;
  Camera = NULL;
}

CphxEvent_RenderScene_Tool::~CphxEvent_RenderScene_Tool()
{

}

CString CphxEvent_RenderScene_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  if ( Scene ) return Scene->GetName();
  return _T( "3D Scene" );
}

TBOOL CphxEvent_RenderScene_Tool::CanWriteToRenderTarget( CphxRenderTarget_Tool *RT )
{
  return true;
}

void CphxEvent_RenderScene_Tool::SetScene( CphxScene_Tool *s )
{
  if ( Scene == s ) return;

  if ( Scene ) RemoveParent( Scene );
  CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)Event;
  Scene = s;
  if ( !s ) e->Scene = NULL;
  else e->Scene = &s->Scene;
  if ( Scene ) AddParent( Scene );

  if ( Scene->GetClipCount() )
    SetClip( Scene->GetClipByIndex( 0 ) );

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
      if ( Scene->GetObjectByIndex( x )->TargetObject )
      {
        SetCamera( Scene->GetObjectByIndex( x ) );
        return;
      }
  }

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
    {
      SetCamera( Scene->GetObjectByIndex( x ) );
      return;
    }
  }

}

void CphxEvent_RenderScene_Tool::SetClip( CphxSceneClip *c )
{
  CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)Event;
  if ( Clip ) RemoveParent( Clip );
  if ( !Scene ) return;
  Clip = c;
  AddParent( c );
  if ( !c ) e->Clip = 0;
  else
  {
    for ( TS32 x = 0; x < Scene->GetClipCount(); x++ )
      if ( Scene->GetClipByIndex( x ) == c )
      {
        e->Clip = x;
      }
  }
}

void CphxEvent_RenderScene_Tool::SetCamera( CphxObject_Tool *o )
{
  CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)Event;
  if ( !Scene ) return;
  Camera = o;
  if ( !o ) e->Camera = NULL;
  else e->Camera = o->GetObject();
}

void CphxEvent_RenderScene_Tool::ImportEventData( CXMLNode *Node )
{
  CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)Event;

  if ( Node->GetChildCount( _T( "scene" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "scene" ) ).GetText().GetPointer() );
    SetScene( Project.GetScene( g ) );
  }

  if ( Node->GetChildCount( _T( "camera" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "camera" ) ).GetText().GetPointer() );
    if ( Scene )
      SetCamera( Scene->GetObject( g ) );
  }

  if ( Node->GetChildCount( _T( "clip" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "clip" ) ).GetText().GetPointer() );
    if ( Scene )
      SetClip( Scene->GetClip( g ) );
  }

  if ( Node->GetChildCount( _T( "clearcolor" ) ) )
  {
    TBOOL b = true;
    Node->GetChild( _T( "clearcolor" ) ).GetValue( b );
    e->ClearColor = b != 0;
  }

  if ( Node->GetChildCount( _T( "clearz" ) ) )
  {
    TBOOL b = true;
    Node->GetChild( _T( "clearz" ) ).GetValue( b );
    e->ClearZ = b != 0;
  }
}

void CphxEvent_RenderScene_Tool::ExportEventData( CXMLNode *Node )
{
  if ( Scene )
    Node->AddChild( _T( "scene" ) ).SetText( Scene->GetGUID().GetString() );
  if ( Clip )
    Node->AddChild( _T( "clip" ) ).SetText( Clip->GetGUID().GetString() );
  if ( Camera )
    Node->AddChild( _T( "camera" ) ).SetText( Camera->GetGUID().GetString() );

  CphxEvent_RenderScene *e = (CphxEvent_RenderScene*)Event;
  Node->AddChild( _T( "clearcolor" ) ).SetInt( e->ClearColor );
  Node->AddChild( _T( "clearz" ) ).SetInt( e->ClearZ );
}

void CphxEvent_ParticleCalc_Tool::ExportEventData( CXMLNode *Node )
{
  if ( Scene )
    Node->AddChild( _T( "scene" ) ).SetText( Scene->GetGUID().GetString() );
  if ( Clip )
    Node->AddChild( _T( "clip" ) ).SetText( Clip->GetGUID().GetString() );
  if ( Camera )
    Node->AddChild( _T( "camera" ) ).SetText( Camera->GetGUID().GetString() );
}

void CphxEvent_ParticleCalc_Tool::ImportEventData( CXMLNode *Node )
{
  CphxEvent_ParticleCalc *e = (CphxEvent_ParticleCalc*)Event;

  if ( Node->GetChildCount( _T( "scene" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "scene" ) ).GetText().GetPointer() );
    SetScene( Project.GetScene( g ) );
  }

  if ( Node->GetChildCount( _T( "camera" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "camera" ) ).GetText().GetPointer() );
    if ( Scene )
      SetCamera( Scene->GetObject( g ) );
  }

  if ( Node->GetChildCount( _T( "clip" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "clip" ) ).GetText().GetPointer() );
    if ( Scene )
      SetClip( Scene->GetClip( g ) );
  }
}


CphxEvent_ParticleCalc_Tool::CphxEvent_ParticleCalc_Tool() : CphxEvent_Tool()
{
  CphxEvent_ParticleCalc *e = new CphxEvent_ParticleCalc();
  e->Scene = NULL;
  //e->Camera = NULL;
  e->Clip = 0;
  e->lasttime = 0;
  e->OnScreenLastFrame = false;

  Event = e;
  Event->Time = (CphxSpline_float16*)Time->Spline;


  Type = EVENT_PARTICLECALC;
  //Color=CColor::FromARGB(0xffff9b05);
  Color = CColor::FromARGB( 0xffa16203 );
  Scene = NULL;
  Clip = NULL;
  Camera = NULL;
}

CphxEvent_ParticleCalc_Tool::~CphxEvent_ParticleCalc_Tool()
{

}

void CphxEvent_ParticleCalc_Tool::SetScene( CphxScene_Tool *s )
{
  if ( Scene == s ) return;

  if ( Scene ) RemoveParent( Scene );
  CphxEvent_ParticleCalc *e = (CphxEvent_ParticleCalc*)Event;
  Scene = s;
  if ( !s ) e->Scene = NULL;
  else e->Scene = &s->Scene;
  if ( Scene ) AddParent( Scene );

  if ( Scene->GetClipCount() )
    SetClip( Scene->GetClipByIndex( 0 ) );

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
      if ( Scene->GetObjectByIndex( x )->TargetObject )
      {
        SetCamera( Scene->GetObjectByIndex( x ) );
        return;
      }
  }

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
    {
      SetCamera( Scene->GetObjectByIndex( x ) );
      return;
    }
  }

}

void CphxEvent_ParticleCalc_Tool::SetClip( CphxSceneClip *c )
{
  CphxEvent_ParticleCalc *e = (CphxEvent_ParticleCalc*)Event;
  if ( Clip ) RemoveParent( Clip );
  if ( !Scene ) return;
  Clip = c;
  AddParent( c );
  if ( !c ) e->Clip = 0;
  else
  {
    for ( TS32 x = 0; x < Scene->GetClipCount(); x++ )
      if ( Scene->GetClipByIndex( x ) == c )
      {
        e->Clip = x;
      }
  }
}

void CphxEvent_ParticleCalc_Tool::SetCamera( CphxObject_Tool *o )
{
  CphxEvent_ParticleCalc *e = (CphxEvent_ParticleCalc*)Event;
  if ( !Scene ) return;
  Camera = o;
/*
  if ( !o ) e->Camera = NULL;
  else e->Camera = o->GetObject();
*/
}


CString CphxEvent_ParticleCalc_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  return _T( "Particle Calc" );
}

void CphxEvent_CameraShake_Tool::ExportEventData( CXMLNode *Node )
{
  CXMLNode n = Node->AddChild( _T( "ShakesPerSecond" ) );
  n.SetAttributeFromInteger( _T( "Value" ), ( (CphxEvent_CameraShake*)Event )->ShakesPerSec );

  n = Node->AddChild( _T( "EyeIntensitySpline" ) );
  EyeIntensity->ExportData( n );

  n = Node->AddChild( _T( "TargetIntensitySpline" ) );
  TargetIntensity->ExportData( n );
}

void CphxEvent_CameraShake_Tool::ImportEventData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "ShakesPerSecond" ) ) )
  {
    int x = 1;
    Node->GetChild( _T( "ShakesPerSecond" ) ).GetAttributeAsInteger( _T( "Value" ), &x );
    ( (CphxEvent_CameraShake*)Event )->ShakesPerSec = x;
  }

  if ( Node->GetChildCount( _T( "EyeIntensitySpline" ) ) )
  {
    EyeIntensity->Keys.FreeArray();
    EyeIntensity->ImportData( Node->GetChild( _T( "EyeIntensitySpline" ) ) );
  }

  if ( Node->GetChildCount( _T( "TargetIntensitySpline" ) ) )
  {
    TargetIntensity->Keys.FreeArray();
    TargetIntensity->ImportData( Node->GetChild( _T( "TargetIntensitySpline" ) ) );
  }
}

CphxEvent_CameraShake_Tool::CphxEvent_CameraShake_Tool() : CphxEvent_Tool()
{
  EyeIntensity = new CphxSpline_Tool_float16();
  TargetIntensity = new CphxSpline_Tool_float16();
  EyeIntensity->AddKey( 0, 0.1f );
  TargetIntensity->AddKey( 0, 0 );

  CphxEvent_CameraShake *camShake = new CphxEvent_CameraShake();
  camShake->EyeIntensity = (CphxSpline_float16*)EyeIntensity->Spline;
  camShake->TargetIntensity = (CphxSpline_float16*)TargetIntensity->Spline;
  camShake->ShakesPerSec = 20;

  Event = camShake;
  Event->Time = (CphxSpline_float16*)Time->Spline;
  Type = EVENT_CAMERASHAKE;
  //Color=CColor::FromARGB(0xffbe8534);
  Color = CColor::FromARGB( 0xff785421 );
}

CphxEvent_CameraShake_Tool::~CphxEvent_CameraShake_Tool()
{
  SAFEDELETE( EyeIntensity );
  SAFEDELETE( TargetIntensity );
}

CString CphxEvent_CameraShake_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  return _T( "Camera Shake" );
}

CphxSpline_Tool_float16 * CphxEvent_CameraShake_Tool::GetSpline( TS32 x, CString &SplineName, CColor &SplineColor )
{
  if ( x == 0 )
  {
    SplineName = _T( "Time Envelope" );
    SplineColor = 0xffffffff;
    return Time;
  }

  if ( x == 1 )
  {
    SplineName = _T( "Eye Shake Intensity" );
    SplineColor = 0xffff00ff;
    return EyeIntensity;
  }

  if ( x == 2 )
  {
    SplineName = _T( "Target Shake Intensity" );
    SplineColor = 0xff00ffff;
    return TargetIntensity;
  }

  return NULL;
}

void CphxEvent_CameraOverride_Tool::ExportEventData( CXMLNode* Node )
{
  if ( Scene )
    Node->AddChild( _T( "scene" ) ).SetText( Scene->GetGUID().GetString() );
  if ( Clip )
    Node->AddChild( _T( "clip" ) ).SetText( Clip->GetGUID().GetString() );
  if ( Camera )
    Node->AddChild( _T( "camera" ) ).SetText( Camera->GetGUID().GetString() );
}

void CphxEvent_CameraOverride_Tool::ImportEventData( CXMLNode* Node )
{
  CphxEvent_CameraOverride* e = (CphxEvent_CameraOverride*)Event;

  if ( Node->GetChildCount( _T( "scene" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "scene" ) ).GetText().GetPointer() );
    SetScene( Project.GetScene( g ) );
  }

  if ( Node->GetChildCount( _T( "camera" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "camera" ) ).GetText().GetPointer() );
    if ( Scene )
      SetCamera( Scene->GetObject( g ) );
  }

  if ( Node->GetChildCount( _T( "clip" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "clip" ) ).GetText().GetPointer() );
    if ( Scene )
      SetClip( Scene->GetClip( g ) );
  }
}


CphxEvent_CameraOverride_Tool::CphxEvent_CameraOverride_Tool() : CphxEvent_Tool()
{
  CphxEvent_CameraOverride* e = new CphxEvent_CameraOverride();
  e->Scene = NULL;
  e->Camera = NULL;
  e->Clip = 0;
  e->OnScreenLastFrame = false;

  Event = e;
  Event->Time = (CphxSpline_float16*)Time->Spline;


  Type = EVENT_CAMERAOVERRIDE;
  //Color=CColor::FromARGB(0xffff9b05);
  Color = CColor::FromARGB( 0xff0362a1 );
  Scene = NULL;
  Clip = NULL;
  Camera = NULL;
}

CphxEvent_CameraOverride_Tool::~CphxEvent_CameraOverride_Tool()
{

}

void CphxEvent_CameraOverride_Tool::SetScene( CphxScene_Tool* s )
{
  if ( Scene == s ) return;

  if ( Scene ) RemoveParent( Scene );
  CphxEvent_CameraOverride* e = (CphxEvent_CameraOverride*)Event;
  Scene = s;
  if ( !s ) e->Scene = NULL;
  else e->Scene = &s->Scene;
  if ( Scene ) AddParent( Scene );

  if ( Scene->GetClipCount() )
    SetClip( Scene->GetClipByIndex( 0 ) );

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
      if ( Scene->GetObjectByIndex( x )->TargetObject )
      {
        SetCamera( Scene->GetObjectByIndex( x ) );
        return;
      }
  }

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    if ( Scene->GetObjectByIndex( x )->GetObjectType() == Object_CamEye )
    {
      SetCamera( Scene->GetObjectByIndex( x ) );
      return;
    }
  }

}

void CphxEvent_CameraOverride_Tool::SetClip( CphxSceneClip* c )
{
  CphxEvent_CameraOverride* e = (CphxEvent_CameraOverride*)Event;
  if ( Clip ) RemoveParent( Clip );
  if ( !Scene ) return;
  Clip = c;
  AddParent( c );
  if ( !c ) e->Clip = 0;
  else
  {
    for ( TS32 x = 0; x < Scene->GetClipCount(); x++ )
      if ( Scene->GetClipByIndex( x ) == c )
      {
        e->Clip = x;
      }
  }
}

void CphxEvent_CameraOverride_Tool::SetCamera( CphxObject_Tool* o )
{
  CphxEvent_CameraOverride* e = (CphxEvent_CameraOverride*)Event;
  if ( !Scene ) return;
  Camera = o;
  if ( !o ) e->Camera = NULL;
  else e->Camera = o->GetObject();
}


CString CphxEvent_CameraOverride_Tool::GetName()
{
  if ( Name.Length() ) return _T( "\"" ) + Name + _T( "\"" );
  return _T( "Camera Override" );
}
