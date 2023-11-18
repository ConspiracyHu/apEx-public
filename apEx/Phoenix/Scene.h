#pragma once
#include "Material.h"
#include "phxarray.h"
#include "Model.h"
#include "phxSpline.h"

#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")

class CphxScene;
class CphxModel;

#define PARTICLEENGINE_FRAMERATE 25.0f

enum PHXSPLINETYPE //if you add one add it to the SplineNames[] and SplineColors[] arrays in the tool as well
{
  Spline_MaterialParam = 0,
  Spline_Scale_x = 1,
  Spline_Scale_y = 2,
  Spline_Scale_z = 3,
  Spline_Rotation = 4,

  Spline_SubScene_Clip = 5,
  Spline_SubScene_Time = 6,

  Padding0 = 7,

  Spline_Position_x = 8,
  Spline_Position_y = 9,
  Spline_Position_z = 10,
  Spline_Position_w = 11,

  Spline_Light_AmbientR = 12,
  Spline_Light_AmbientG = 13,
  Spline_Light_AmbientB = 14,
  Padding1 = 15,

  Spline_Light_DiffuseR = 16,
  Spline_Light_DiffuseG = 17,
  Spline_Light_DiffuseB = 18,
  Padding2 = 19,

  Spline_Light_SpecularR = 20,
  Spline_Light_SpecularG = 21,
  Spline_Light_SpecularB = 22,
  Padding3 = 23,

  Spot_Direction_X = 24,
  Spot_Direction_Y = 25,
  Spot_Direction_Z = 26,
  Padding4 = 27,

  Spline_Light_Exponent = 28,
  Spline_Light_Cutoff = 29,
  Spline_Light_Attenuation_Linear = 30,
  Spline_Light_Attenuation_Quadratic = 31,
  Padding7 = 32,

  Spline_Camera_FOV = 33,
  Spline_Camera_Roll = 34,
  Padding8 = 35,
  Padding9 = 36,

  Spline_Particle_Offset_x = 37,
  Spline_Particle_Offset_y = 38,
  Spline_Particle_Offset_z = 39,
  Spline_Particle_EmissionPerSecond = 40,
  Spline_Particle_EmissionTrigger = 41,
  Spline_Particle_EmissionVelocity = 42,
  Spline_Particle_Life = 43,
  Spline_Particle_EmissionRotation = 44,
  Spline_Particle_EmissionVelocityChaos = 45,
  Spline_Particle_EmissionRotationChaos = 46,
  Spline_Particle_LifeChaos = 47,

  Spline_Light_OrthoX = 48,
  Spline_Light_OrthoY = 49,

  Spline_AffectorPower = 50,

  Spline_Particle_Scale = 51,
  Spline_Particle_ScaleChaos = 52,
  Spline_Particle_Stretch_X = 53,
  Spline_Particle_Stretch_Y = 54,

  Spline_SubScene_RepeatCount = 55,
  Spline_SubScene_RepeatTimeOffset = 56,

  Spline_Count, //don't remove this
};


enum PHXOBJECT
{
  Object_Model = 0,
  Object_Light,
  Object_CamEye,
  Object_Dummy,
  Object_SubScene,
  Object_ParticleEmitter,
  Object_ParticleEmitterCPU,
  Object_ParticleDrag,
  Object_ParticleGravity,
  Object_ParticleTurbulence,
  Object_ParticleVortex,
  Object_LogicObject,

  Object_Count,
};

struct CphxClipSpline
{
  PHXSPLINETYPE Type;
  CphxSpline *Spline;
  CphxMaterialParameter *MaterialParam;
  void *GroupingData;
};

struct CphxObjectClip
{
  class CphxScene *SubSceneTarget;
  unsigned char RandSeed;
  unsigned char TurbulenceFrequency;
  int SplineCount;
  CphxClipSpline **Splines;
  CphxMaterialSplineBatch *MaterialSplines;
};

struct LIGHTDATA
{
  D3DXVECTOR4 Position;
  D3DXVECTOR4 Ambient, Diffuse, Specular;
  D3DXVECTOR4 SpotDirection;
  D3DXVECTOR4 SpotData; //exponent, cutoff, linear and quadratic attenuations
};

class CphxObject
{

public:

  class CphxScene *Scene;
  class CphxScene *SubSceneTarget;

  CphxObject *Parent;
  int ChildCount;
  CphxObject **Children;

  //constant data, spline results are stored here
  float SplineResults[ Spline_Count ];
  D3DXQUATERNION RotationResult;
  D3DXVECTOR3 TargetDirection; //calculated from target

  PHXOBJECT ObjectType;

  //clip data
  CphxObjectClip **Clips;

  D3DXVECTOR3 WorldPosition;

  //LIGHTDATA LightData;

  //int Clip; //subscene clip
  //float ClipTime; //subscene clip time
  //float FOV, Roll; //camera data

  //float orthoX, orthoY, orthoRoll; //light shadow ortho data

  char camCenterX; // also used for logic object type
  char camCenterY; // also used for logic object data
  CphxRenderTarget* cameraCubeMapTarget;

  //data for different object types
  CphxObject *Target;
  int TargetID; //minimal import data!
  int minimportParentID;

  D3DXMATRIX currMatrix;
  D3DXMATRIX prevMatrix;
  D3DXMATRIX inverse; // for particle affector stuff

  virtual void CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData ) {};
  void TraverseSceneGraph( int Clip, float t, D3DXMATRIX CurrentMatrix, CphxScene *RootScene, void *SubSceneData );
  void CalculateAnimation( int Clip, float t );

  unsigned char RandSeed;
  unsigned char TurbulenceFrequency;

  void *ToolData;
  D3DXMATRIX GetWorldMatrix();

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject() {};
#endif
};

#ifdef PHX_OBJ_MODEL
class CphxObject_Model : public CphxObject
{
public:

  CphxModel *Model;

  virtual void CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_Model() {};
#endif

};
#endif

#ifdef PHX_OBJ_SUBSCENE
class CphxObject_SubScene : public CphxObject
{
public:

  //CphxScene *Scene;
  //CphxScene* ClipScenes[ 256 ];
  virtual void CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_SubScene() {};
#endif

};
#endif

//particle data: 4 floats position, 4 floats misc (life, rotation speed, rotation offset, reserved)
#define PARTICLESIZE (4*(4+4))

struct PHXPARTICLE
{
  D3DXVECTOR3 Position;
  D3DXVECTOR3 Velocity;
  float Rotation, RotationSpeed, Chaos, Scale, ScaleChaos, StretchX, StretchY;
  int MaxLife, LifeLeft; //particle alive if lifeleft > 0
  float RandSeed;
  D3DXVECTOR3 RotationAxis;
};

struct PHXPARTICLEDISTANCE
{
  int Idx;
  float Dist;
};

#ifdef PHX_OBJ_EMITTERCPU
class CphxObject_ParticleEmitter_CPU : public CphxObject
{
public:

  int objIdxMod;

  int LiveCount;
  PHXPARTICLE *Particles; //allocated/deallocated by creating object
  PHXPARTICLEDISTANCE *DistanceBuffer;

  float EmissionFraction;
  float Ticks;
  bool Aging;
  bool RandRotate;
  bool TwoDirRotate;
#ifdef PHX_HAS_PARTICLE_SORTING
  bool Sort;
#endif
  bool RotateToDirection;
  unsigned char BufferSize; //2^BufferSize particles

  unsigned char EmitterType; //box, sphere
  unsigned char InnerRadius;
  unsigned char StartCount, RandSeed;

#ifdef PHX_HAS_STANDARD_PARTICLES
  ID3D11Texture2D *SplineTexture;
  ID3D11ShaderResourceView *SplineTextureView;

  ID3D11Buffer* VertexBuffer;
  CphxMaterialPassConstantState **MaterialState; //contains per object constant material state for each material pass
  CphxMaterial *Material;
#endif

#ifdef PHX_HAS_MESH_PARTICLES
  CphxModel *ObjectToEmit;
#endif
#ifdef PHX_HAS_SUBSCENE_PARTICLES
  CphxScene *SceneToEmit;
#endif

  virtual void CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData );
  virtual void UpdateParticles( float elapsedtime, bool updatebuffer = true );
  void SpawnParticle( float t, D3DXMATRIX &mat, D3DXMATRIX &o, float mt );
#ifdef PHX_HAS_STANDARD_PARTICLES
  void UpdateSplineTexture();
#endif
  void ResetParticles();

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_ParticleEmitter_CPU() {};
#endif

};
#endif

#if defined PHX_OBJ_PARTICLEDRAG || defined PHX_OBJ_PARTICLEGRAVITY || defined PHX_OBJ_PARTICLETURBULENCE || defined PHX_OBJ_PARTICLEVORTEX  || defined PHX_OBJ_EMITTERCPU
class CphxObject_ParticleAffector : public CphxObject
{
public:
  unsigned char AreaType;
  bool ParticleInside( D3DXVECTOR3 v );
  virtual D3DXVECTOR3 GetForce( PHXPARTICLE *p ) = 0;

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_ParticleAffector() {};
#endif

};
#endif

#ifdef PHX_OBJ_PARTICLEDRAG
class CphxObject_ParticleDrag : public CphxObject_ParticleAffector
{
public:
  D3DXVECTOR3 GetForce( PHXPARTICLE *p );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_ParticleDrag() {};
#endif

};
#endif

#ifdef PHX_OBJ_PARTICLEGRAVITY
class CphxObject_ParticleGravity : public CphxObject_ParticleAffector
{
public:
  bool Directional;

  D3DXVECTOR3 GetForce( PHXPARTICLE *p );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_ParticleGravity() {};
#endif

};
#endif

#ifdef PHX_OBJ_PARTICLETURBULENCE
class CphxObject_ParticleTurbulence : public CphxObject_ParticleAffector
{
  D3DXVECTOR3 SampleKernel( const D3DXVECTOR4& Pos );
public:
  D3DXVECTOR3 Kernel[ 32 ][ 32 ][ 32 ];
  //D3DXFLOAT16 Frequency;
  //unsigned char RandSeed;
  unsigned char calculatedKernelSeed;

  void InitKernel();
  D3DXVECTOR3 GetForce( PHXPARTICLE *p );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_ParticleTurbulence() {};
#endif

};
#endif

#ifdef PHX_OBJ_PARTICLEVORTEX
class CphxObject_ParticleVortex : public CphxObject_ParticleAffector
{
public:
  D3DXVECTOR3 GetForce( PHXPARTICLE* p );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxObject_ParticleVortex() {};
#endif

};
#endif

//class CphxObject_ParticleEmitter : public CphxObject
//{
//public:
//
//	bool Sort;
//	unsigned char Resolution; //same resolution ticker as for textures
//
//	ID3D11Resource *Particles;
//	ID3D11UnorderedAccessView *ParticleView;
//	ID3D11ComputeShader *UpdateShader;
//	ID3D11ComputeShader *SpawnShader;
//
//	//virtual void CreateRenderDataInstances(int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData);
//};

class CphxScene
{
public:

  int LightCount;
  LIGHTDATA Lights[ MAX_LIGHT_COUNT ];

  int ObjectCount;
  CphxObject **Objects;

  int LayerCount;
  CphxRenderLayer **RenderLayers;
  CphxObject Root;

#ifndef PHX_MINIMAL_BUILD
  int ClipCount;
#endif

  void CollectLights( CphxScene* sceneToCollectFrom );
  void UpdateSceneGraph( int Clip, float t );
  void UpdateSceneGraph( int Clip, float t, D3DXMATRIX Root, CphxScene *RootScene, void *SubSceneData );
  void Render( bool ClearColor, bool ClearZ, int cubeResolution );

  void AddRenderDataInstance( CphxRenderLayerDescriptor *Layer, CphxRenderDataInstance *RDI );
  //void SimulateParticles(float t, bool updatebuffer);

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxScene() {};
#endif

};

extern D3DXMATRIX phxViewMatrix;
extern D3DXMATRIX phxIViewMatrix;
extern D3DXMATRIX phxProjectionMatrix;
extern D3DXMATRIX phxIProjectionMatrix;
extern D3DXMATRIX phxWorldMatrix;
extern D3DXMATRIX phxITWorldMatrix;
extern D3DXVECTOR4 phxCameraPos;
extern D3DXVECTOR4 phxCameraDir; //for hive
extern D3DXMATRIX phxPrevFrameViewMatrix;
extern D3DXMATRIX phxPrevFrameProjectionMatrix;
