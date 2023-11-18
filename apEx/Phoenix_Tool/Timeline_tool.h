#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "RenderTargets.h"
#include "Material_Tool.h"
#include "../Phoenix_Tool/phxSplineExt.h"
#include "../Phoenix_Tool/Scene_tool.h"
#include "../Phoenix/Timeline.h"

//enum PHXEVENTTYPE;

class CphxEvent_Tool : public CphxResource
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  virtual void ExportEventData( CXMLNode *Node ) = 0;
  virtual void ImportEventData( CXMLNode *Node ) = 0;

protected:

public:
  CString Name;

  PHXEVENTTYPE Type;
  CphxGUID TargetID;

  class CphxEvent *Event;
  TS32 Pass;
  TBOOL Selected;
  CColor Color;
  TBOOL exportProblem = false;

  unsigned short originalStartFrame, originalEndFrame;

  CphxSpline_Tool_float16 *Time;

  void SetName( CString &s );
  virtual CString GetName();
  TBOOL Intersects( CRect &r );

  CphxEvent_Tool();
  virtual ~CphxEvent_Tool();

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType() { return PHX_EVENT; }

  virtual TS32 GetSplineCount() { return 1; }
  virtual CphxSpline_Tool_float16 *GetSpline( TS32 x, CString &SplineName, CColor &SplineColor );

  virtual TBOOL DoesScreenWrite() = 0;
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT ) = 0;

  virtual PHXEVENTTYPE GetEventType() = 0;
};

class CphxEvent_EndDemo_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode *Node ) {};
  virtual void ImportEventData( CXMLNode *Node ) {};

public:
  CphxEvent_EndDemo_Tool();
  virtual ~CphxEvent_EndDemo_Tool();

  virtual CString GetName();

  virtual TBOOL DoesScreenWrite() { return false; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT ) { return false; }

  virtual PHXEVENTTYPE GetEventType() { return EVENT_ENDDEMO; }
};

class CphxEvent_RenderDemo_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode *Node ) {};
  virtual void ImportEventData( CXMLNode *Node ) {};

public:
  CphxEvent_RenderDemo_Tool();
  virtual ~CphxEvent_RenderDemo_Tool();

  virtual CString GetName();

  virtual TBOOL DoesScreenWrite() { return false; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT ) { return false; }

  virtual PHXEVENTTYPE GetEventType() { return EVENT_RENDERDEMO; }
};

class CphxEvent_Shadertoy_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode *Node );
  virtual void ImportEventData( CXMLNode *Node );

  //CphxMaterialParameterLinkBatch_Tool ShaderParams;

  CphxMaterialTechnique_Tool *Tech;
  void ClearTech();

  TS32 MaterialStateSize;

  virtual TBOOL GenerateResource();

public:
  CphxMaterialDataStorage_Tool MaterialData;
  CphxMaterialSplineBatch_Tool ShaderSplines;

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  void SetTech( CphxMaterialTechnique_Tool *Tech, TBOOL Update = false );
  CphxMaterialTechnique_Tool *GetTech() { return Tech; }

  CphxEvent_Shadertoy_Tool();
  virtual ~CphxEvent_Shadertoy_Tool();

  virtual CString GetName();

  virtual TS32 GetSplineCount();
  virtual CphxSpline_Tool_float16 *GetSpline( TS32 x, CString &SplineName, CColor &SplineColor );

  virtual TBOOL DoesScreenWrite() { return true; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT );

  virtual void FreeMaterialState();
  virtual void AllocateMaterialState();
  virtual void UpdateMaterialState();
  virtual void UpdateMaterialTextures();

  virtual PHXEVENTTYPE GetEventType() { return EVENT_SHADERTOY; }
};

class CphxEvent_RenderScene_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode *Node );
  virtual void ImportEventData( CXMLNode *Node );

public:
  CphxEvent_RenderScene_Tool();
  virtual ~CphxEvent_RenderScene_Tool();

  CphxScene_Tool *Scene;
  CphxSceneClip *Clip;
  CphxObject_Tool *Camera;

  virtual CString GetName();
  void SetScene( CphxScene_Tool *s );
  void SetClip( CphxSceneClip *c );
  void SetCamera( CphxObject_Tool *o );

  virtual TBOOL DoesScreenWrite() { return true; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT );

  virtual PHXEVENTTYPE GetEventType() { return EVENT_RENDERSCENE; }
};

class CphxEvent_ParticleCalc_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode *Node );
  virtual void ImportEventData( CXMLNode *Node );

public:

  CphxScene_Tool *Scene;
  CphxSceneClip *Clip;
  CphxObject_Tool *Camera;

  CphxEvent_ParticleCalc_Tool();
  virtual ~CphxEvent_ParticleCalc_Tool();

  virtual CString GetName();
  void SetScene( CphxScene_Tool *s );
  void SetClip( CphxSceneClip *c );
  void SetCamera( CphxObject_Tool *o );

  virtual TBOOL DoesScreenWrite() { return false; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT ) { return false; }

  virtual PHXEVENTTYPE GetEventType() { return EVENT_PARTICLECALC; }
};

class CphxEvent_CameraShake_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode *Node );
  virtual void ImportEventData( CXMLNode *Node );

public:
  CphxSpline_Tool_float16 *EyeIntensity;
  CphxSpline_Tool_float16 *TargetIntensity;

  CphxEvent_CameraShake_Tool();
  virtual ~CphxEvent_CameraShake_Tool();

  virtual CString GetName();
  virtual TS32 GetSplineCount() { return 3; }
  virtual CphxSpline_Tool_float16 *GetSpline( TS32 x, CString &SplineName, CColor &SplineColor );

  virtual TBOOL DoesScreenWrite() { return false; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool *RT ) { return false; }

  virtual PHXEVENTTYPE GetEventType() { return EVENT_CAMERASHAKE; }
};

class CphxEvent_CameraOverride_Tool : public CphxEvent_Tool
{
  virtual void ExportEventData( CXMLNode* Node );
  virtual void ImportEventData( CXMLNode* Node );

public:

  CphxScene_Tool* Scene;
  CphxSceneClip* Clip;
  CphxObject_Tool* Camera;

  CphxEvent_CameraOverride_Tool();
  virtual ~CphxEvent_CameraOverride_Tool();

  virtual CString GetName();
  void SetScene( CphxScene_Tool* s );
  void SetClip( CphxSceneClip* c );
  void SetCamera( CphxObject_Tool* o );

  virtual TBOOL DoesScreenWrite() { return false; }
  virtual TBOOL CanWriteToRenderTarget( CphxRenderTarget_Tool* RT ) { return false; }

  virtual PHXEVENTTYPE GetEventType() { return EVENT_CAMERAOVERRIDE; }
};

class CphxTimeline_Tool
{

public:

  TF32 GridBPM = 120;
  TS32 BPMPrimaryModulus = 32;
  TS32 BPMSecondaryModulus = 8;


  TBOOL BeatMarkerPlaced = false;
  CArray<TS32> BeatMarkers;

  CArray<CphxEvent_Tool*> Events;
  class CphxTimeline *Timeline;

  CphxTimeline_Tool();
  virtual ~CphxTimeline_Tool();

  CphxEvent_Tool *CreateEvent( PHXEVENTTYPE Type, TS32 StartFrame, TS32 EndFrame, TS32 Pass, CphxMaterialTechnique_Tool *Tech );
  void DeleteEventByIndex( TS32 x );

  virtual void Render( float Frame, TBOOL RenderAsSubroutine = false );

  void Sort();
};