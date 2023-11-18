#pragma once
#include "RenderTarget.h"
#include "phxSpline.h"
#include "Material.h"
#include "Scene.h"
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

enum PHXEVENTTYPE
{
  EVENT_ENDDEMO = 0,
  EVENT_RENDERDEMO = 1,
  EVENT_SHADERTOY = 2,
  EVENT_RENDERSCENE = 3,
  EVENT_PARTICLECALC = 4,
  EVENT_CAMERASHAKE = 5,
  EVENT_CAMERAOVERRIDE = 6,

  Event_Count
};

class CphxEvent
{
public:

  bool OnScreenLastFrame;
  CphxRenderTarget *Target;

  unsigned short StartFrame, EndFrame;
  CphxSpline_float16 *Time;

  virtual void Render( float t, float prevt, float aspect, bool subroutine ) = 0;

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent() {};
#endif
};

#ifdef PHX_EVENT_ENDDEMO
class CphxEvent_EndDemo : public CphxEvent
{
public:
  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_EndDemo() {};
#endif
};
#endif

#ifdef PHX_EVENT_RENDERDEMO
class CphxEvent_RenderDemo : public CphxEvent
{
public:

  class CphxTimeline *Timeline;
  unsigned short _start, _end;

  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_RenderDemo() {};
#endif
};
#endif

#ifdef PHX_EVENT_SHADERTOY
class CphxEvent_Shadertoy : public CphxEvent
{
public:

  //CphxMaterialParameterLinkBatch ShaderParams;
  //CphxTechRenderStateBatch *RSBatch;

  CphxMaterialSplineBatch *MaterialSplines;

  CphxMaterialPassConstantState **MaterialState; //contains per object constant material state for each material pass
  CphxMaterialTechnique *Tech;

  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_Shadertoy() {};
#endif
};
#endif

#ifdef PHX_EVENT_RENDERSCENE
class CphxEvent_RenderScene : public CphxEvent
{
public:

  CphxScene *Scene;
  CphxObject *Camera;
  int Clip;
  bool ClearColor, ClearZ;

  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_RenderScene() {};
#endif
};
#endif


#ifdef PHX_EVENT_PARTICLECALC
class CphxEvent_ParticleCalc : public CphxEvent
{
public:
  CphxScene *Scene;
  //CphxObject *Camera;
  int Clip;

  int lasttime;
  float lastt;

  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_ParticleCalc() {};
#endif
};
#endif

#ifdef PHX_EVENT_CAMSHAKE
class CphxEvent_CameraShake : public CphxEvent
{
public:

  unsigned char ShakesPerSec;
  CphxSpline_float16 *EyeIntensity;
  CphxSpline_float16 *TargetIntensity;

  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_CameraShake() {};
#endif
};
#endif

#ifdef PHX_EVENT_CAMOVERRIDE
class CphxEvent_CameraOverride : public CphxEvent
{
public:

  CphxScene* Scene;
  CphxObject* Camera;
  int Clip;

  virtual void Render( float t, float prevt, float aspect, bool subroutine );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxEvent_CameraOverride() {};
#endif
};
#endif

class CphxTimeline
{

public:

  // these 4 values are read as a single dword during project load
  unsigned char AspectX;
  unsigned char AspectY;
  unsigned char FrameRate;
  unsigned char RenderTargetCount;

  CphxRenderTarget **RenderTargets;

  CphxRenderTarget *Target;

  int ScreenX, ScreenY;

  unsigned short EventCount;
  CphxEvent **Events;

  void Render( float Frame, bool tool, bool subroutine = false );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxTimeline() {};
#endif
};