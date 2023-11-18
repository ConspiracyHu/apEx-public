#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../Phoenix/Scene.h"
#include "phxResource.h"
#include "../Phoenix/Scene.h"
#include "../Phoenix_Tool/Model_Tool.h"
#include "Material_Tool.h"

class CphxSceneClip;
class CphxObject_Tool;
class CphxSpline_Tool;
class CphxScene_Tool;
class CphxMaterialParameter_Tool;
class CphxModelObject_Tool;

class CphxClipSpline_Tool
{

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxClipSpline MinimalSpline;
  CphxSpline_Tool *Spline;
  CphxMaterialParameter_Tool *Parameter;
  CphxModelObject_Tool *GroupingObject;

  CphxClipSpline_Tool( PHXSPLINETYPE Spline );
  virtual ~CphxClipSpline_Tool();

  void Update();

  TBOOL IsDefaultSpline();
};

class CphxObjectClip_Tool
{
  CphxGUID SubSceneTargetGUID;
  CphxScene_Tool *SubSceneTarget = nullptr;
  unsigned char RandSeed = 0;
  unsigned char TurbulenceFrequency = 10;
  CArray<CphxClipSpline_Tool*> Splines;
  CphxSceneClip *SceneClip = nullptr;
  CphxMaterialSplineBatch_Tool MaterialSplines;

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObjectClip_Tool( CphxSceneClip *Clip );
  virtual ~CphxObjectClip_Tool();

  void BuildSplines( CphxObject_Tool *Owner );
  void CreateSpline( PHXSPLINETYPE Spline );
  void RebuildMaterialSplines( CphxModel_Tool *m );
  void RebuildMaterialSplines( CphxObject_ParticleEmitter_CPU_Tool *m );

  void BuildMinimalData( CphxObjectClip *clip );
  void UpdateSplines();

  TS32 GetSplineCount() { return Splines.NumItems(); }
  TS32 GetMaterialSplineCount() { return MaterialSplines.Splines.NumItems(); }
  CphxClipSpline_Tool *GetSplineByIndex( TS32 idx ) { return Splines[ idx ]; }
  CphxMaterialSpline_Tool *GetMaterialSplineByIndex( TS32 idx ) { return MaterialSplines.Splines[ idx ]; }
  CphxMaterialSplineBatch_Tool *GetMaterialSplineBatch() { return &MaterialSplines; }
  CphxSpline_Tool * GetSpline( PHXSPLINETYPE Spline );
  CphxClipSpline_Tool * GetClipSpline( PHXSPLINETYPE Spline );
  void BackupSplineData();
  void ApplySplineTransformation( PHXSPLINETYPE Spline, TF32 Timestamp, TF32 Value, TBOOL AutoKey );
  void ApplySplineTransformation( PHXSPLINETYPE Spline, TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey );

  CphxGUID GetSceneClipGUID();
  CphxSceneClip *GetSceneClip() { return SceneClip; }
  void BuildMaterialSplines( CphxModel_Tool *m );
  void BuildMaterialSplines( CphxObject_ParticleEmitter_CPU_Tool *m );
  CphxScene_Tool* GetSubSceneTarget() { return SubSceneTarget; }
  void SetSubSceneTarget( CphxScene_Tool* target );
  unsigned char GetRandSeed() { return RandSeed;  }
  void SetRandSeed( unsigned char seed ) { RandSeed = seed; }
  unsigned char GetTurbulenceFreq() { return TurbulenceFrequency; }
  void SetTurbulenceFreq( unsigned char seed ) { TurbulenceFrequency = seed; }
};

class CphxObjectClipArray_Tool
{
  friend class CphxScene_Tool;
  friend CphxObject_Tool;
  CArray<CphxObjectClip_Tool*> Clips;
  CphxObject_Tool *Owner;
  int MinimalClipCount;

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObjectClipArray_Tool();
  virtual ~CphxObjectClipArray_Tool();

  void AddClip( CphxSceneClip *Clip, TS32 referenceClipId );
  void DeleteClip();

  void RebuildMinimalData();
  void FreeMinimalData();

  void UpdateSplines();
  int GetClipCount();
  CphxObjectClip_Tool* GetClip( TS32 Clip );
  void BackupSplineData();
  CphxSpline_Tool *GetSpline( TS32 Clip, PHXSPLINETYPE Spline );
  void ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, TF32 Value, TBOOL AutoKey );
  void ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey );
};

class CphxScene_Tool;

class CphxObject_Tool : public CphxResource
{
  friend CphxScene_Tool;
  CphxObject *Object;

protected:
  CphxObjectClipArray_Tool Clips;
  CString Name;

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObject_Tool *Parent;

public:

  CColor WireframeColor;

  D3DXVECTOR3 _Scale, _Position;
  D3DXQUATERNION _Rotation;

  TBOOL Selected;
  CphxObject_Tool *TargetObject;
  CphxGUID TargetObjectGuid;
  CphxGUID ParentObjectGuid;

  void SetName( CString &name ) { Name = name; }
  CString GetName() { return Name; }

  virtual TBOOL GenerateResource( CCoreDevice *Dev ) { return true; }
  virtual PHXRESOURCETYPE GetType() { return PHX_OBJECT; }

  CphxObject_Tool();
  virtual ~CphxObject_Tool();

  CphxObject *GetObject() { return Object; }
  virtual void SetKeyframerMode( void *Data ) {}

  virtual TBOOL Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t );

  D3DXMATRIX GetMatrix();
  D3DXMATRIX GetMatrix_();
  CphxObject_Tool *GetParentObject() { return Parent; }
  //void SetObjectType(PHXOBJECT o) { ObjectType = o; }
  PHXOBJECT GetObjectType() { return Object->ObjectType; }

  void SetObject( CphxObject *o );

  void AddClip( CphxSceneClip *Clip, TS32 referenceClipId );
  virtual void CreateSplines( CphxObjectClip_Tool *Clip );
  void RebuildMinimalData();

  CphxObjectClip_Tool* GetClip( TS32 clip );
  void BackupSplineData();

  CphxSpline_Tool *GetSpline( TS32 Clip, PHXSPLINETYPE Spline );
  void ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, TF32 Value, TBOOL AutoKey );
  void ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey );

  void SetTarget( CphxObject_Tool *o );
  TBOOL FindParent( CphxObject_Tool *p );
  void SetParent( CphxObject_Tool *p, CphxScene_Tool *Scene );
  void RebuildChildList( CphxScene_Tool *Scene );

  void SetPosition( CVector3 v );
  void SetScale( CVector3 v );
  void SetRotation( CQuaternion q );
  CVector3 GetPosition();
  void CopySplinesFrom( CphxObject_Tool *Source );
  void CopySplinesFrom( CphxObject_Tool *Source, int clip, CphxScene_Tool *parentScene );

  CphxObjectClipArray_Tool& GetClipArray() { return Clips; }
  CphxScene_Tool* GetParentScene();
};


class CphxObject_Model_Tool : public CphxObject_Tool
{
  CphxObject_Model *obj;

  virtual void ExportData( CXMLNode *Node );

public:
  CphxModel_Tool *Model;

  CphxObject_Model_Tool( CphxModel_Tool *Model );
  virtual ~CphxObject_Model_Tool();
  virtual void SetKeyframerMode( void *Data );
  void UpdateModel();

  virtual TBOOL Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t );
  virtual void CreateSplines( CphxObjectClip_Tool *Clip );
  void SwapModel( CphxModel_Tool *newScene );
};

class CphxObject_Light_Tool : public CphxObject_Tool
{
  TBOOL PointLight;
public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObject_Light_Tool();
  virtual ~CphxObject_Light_Tool();
  virtual void CreateSplines( CphxObjectClip_Tool *Clip );

  TBOOL IsPointLight() { return PointLight; }
  void SetPointLight( TBOOL p );
};

class CphxScene_Tool;

class CphxObject_SubScene_Tool : public CphxObject_Tool
{
  CphxObject_SubScene *obj;

  virtual void ExportData( CXMLNode *Node );

public:
  //CphxScene_Tool *Scene;

  CphxObject_SubScene_Tool();
  virtual ~CphxObject_SubScene_Tool();
  virtual void SetKeyframerMode( void *Data );
  void UpdateModel();

  virtual TBOOL Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t );
  virtual void CreateSplines( CphxObjectClip_Tool *Clip );
  void SwapScene( CphxScene_Tool *newScene, TS32 Clip );
  CphxScene_Tool* GetSubScene(int clipidx);

  virtual void MarkSpecialRequired() override;
};

//class CphxObject_ParticleEmitter_Tool : public CphxObject_Tool
//{
//
//	CphxObject_ParticleEmitter *obj;
//
//public:
//
//	CphxObject_ParticleEmitter_Tool();
//	virtual ~CphxObject_ParticleEmitter_Tool();
//	virtual void SetKeyframerMode(void *Data);
//
//	void UpdateParticleResources();
//
//};

class CphxObject_ParticleEmitter_CPU_Tool : public CphxObject_Tool
{

  CphxObject_ParticleEmitter_CPU *obj;
  CCoreVertexBuffer *VertexBuffer;

  PHXPARTICLE *backupBuffer = nullptr;
  PHXPARTICLE *tempBuffer = nullptr;

  int backupLiveCount = 0;
  float backupEmissionFraction = 0;
  float backupTicks = 0;

  void FreeMaterialState();
  void AllocateMaterialState();

  TS32 MaterialStateSize;
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  CphxMaterial_Tool *Material;
  CphxMaterialDataStorage_Tool MaterialData;
  CphxModel_Tool *EmitedObject;
  CphxScene_Tool *EmitedScene;

  CphxObject_ParticleEmitter_CPU_Tool();
  virtual ~CphxObject_ParticleEmitter_CPU_Tool();
  virtual void SetKeyframerMode( void *Data );

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  void UpdateParticleResources( CCoreDevice *Dev );

  virtual void SetMaterial( CphxMaterial_Tool *m, TBOOL Update = false );
  virtual void SetEmittedModel( CphxModel_Tool *m );
  virtual void SetEmittedScene( CphxScene_Tool *m );
  virtual void CopyDataFrom( CphxObject_ParticleEmitter_CPU_Tool* data );
  virtual CphxMaterial_Tool *GetMaterial() { return Material; }
  virtual void UpdateMaterialState();
  virtual void UpdateMaterialTextures();

  virtual void SimulateParticles( TF32 ElapsedTime );
  virtual void CreateSplines( CphxObjectClip_Tool *Clip );

  virtual void ResetParticles();

  virtual void SwapParticleBuffers();
  virtual void MarkSpecialRequired() override;
};

class CphxObject_ParticleGravity_Tool : public CphxObject_Tool
{

  CphxObject_ParticleGravity *affector;

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObject_ParticleGravity_Tool();
  virtual ~CphxObject_ParticleGravity_Tool();

  virtual void CreateSplines( CphxObjectClip_Tool *Clip );
};

class CphxObject_ParticleDrag_Tool : public CphxObject_Tool
{

  CphxObject_ParticleDrag *affector;

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObject_ParticleDrag_Tool();
  virtual ~CphxObject_ParticleDrag_Tool();

  virtual void CreateSplines( CphxObjectClip_Tool *Clip );
};

class CphxObject_ParticleTurbulence_Tool : public CphxObject_Tool
{

  CphxObject_ParticleTurbulence *affector;

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CphxObject_ParticleTurbulence_Tool();
  virtual ~CphxObject_ParticleTurbulence_Tool();

  virtual void CreateSplines( CphxObjectClip_Tool *Clip );
};

class CphxObject_ParticleVortex_Tool : public CphxObject_Tool
{

  CphxObject_ParticleVortex* affector;

public:

  virtual void ExportData( CXMLNode* Node );
  virtual void ImportData( CXMLNode* Node );

  CphxObject_ParticleVortex_Tool();
  virtual ~CphxObject_ParticleVortex_Tool();

  virtual void CreateSplines( CphxObjectClip_Tool* Clip );
};

class CphxSceneClip : public CphxResource
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CString Name;

public:

  void SetName( CString &name ) { Name = name; }
  CString GetName() { return Name; }

  virtual TBOOL GenerateResource( CCoreDevice *Dev ) { return true; }
  virtual PHXRESOURCETYPE GetType() { return PHX_CLIP; }

  CphxSceneClip();
  virtual ~CphxSceneClip();
};

class CphxScene_Tool : public CphxResource
{

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CString Name;

  CArray<CphxObject_Tool*> Objects;
  CArray<CphxSceneClip*> Clips;

  TS32 ActiveClip;
  //TF32 Timestamp;

  TU32 LastParticleTime = 0;
  TU32 lastParticleTimeBackup = 0;

public:

  void SetName( CString &name ) { Name = name; }
  CString GetName() { return Name; }

  CphxScene Scene;

  CphxScene_Tool();
  virtual ~CphxScene_Tool();

  void UpdateSceneGraph( int Clip, float t );
  void UpdateSceneGraph( int Clip, float t, D3DXMATRIX Root, CphxScene *RootScene );
  void Render();

  void KillInvalidBatches( bool wireframeview );

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType() { return PHX_SCENE; }

  void UpdateLayers();

  CphxObject_Tool *AddObject( PHXOBJECT p, void *Data );

  TS32 GetObjectCount() { return Objects.NumItems(); }
  CphxObject_Tool *GetObjectByIndex( TS32 x ) { if ( x >= 0 && x < Objects.NumItems() ) return Objects[ x ]; return NULL; }
  CphxObject_Tool *GetObject( CphxGUID &g );
  CphxObject_Tool *GetObjectByName( CString Name );

  void SetKeyframerMode();
  void DeleteByIndex( TS32 idx );
  void DeleteSelected();
  void CopySelected();
  void MarkCopyPaste();
  void PasteObjects();
  void CopyFrom( CphxScene_Tool *Source );
  CphxObject_Tool* CopyObject( CphxObject_Tool* source );

  CphxSceneClip *AddClip();
  CphxSceneClip *GetClip( CphxGUID &GUID );
  TS32 GetClipIndex( CphxGUID &GUID );
  void RebuildMinimalData();

  TS32 GetClipCount() { return Clips.NumItems(); }
  CphxSceneClip *GetClipByIndex( TS32 x ) { if ( x < 0 || x >= Clips.NumItems() ) return NULL; return Clips[ x ]; }

  TS32 GetActiveClip() { return ActiveClip; }
  void SetActiveClip( TS32 ac ) { ActiveClip = ac; }

  //TF32 GetTimestamp() { return Timestamp; }
  //TF32 SetTimestamp(TF32 ts) { Timestamp = ts; }

  TS32 GetObjectIndex( CphxObject_Tool *o ) { return Objects.Find( o ); }
  TS32 GetObjectIndex( CphxGUID &GUID );
  CphxObject_Tool *GetEditedObject();
  void LinkSelectedTo( CphxObject_Tool *parent, TBOOL preservePosition );

  TBOOL CanContain( CphxScene_Tool *sub );
  TBOOL FindInSubsceneTree( CphxScene_Tool *scene );

  void ForceUpdateContent();
  TS32 GetTriCount();

  virtual void SimulateParticles();
  virtual void ResetParticles();

  virtual void SwapParticleBuffers();

  void SortRenderLayers();
  void CopyClip( CphxGUID &GUID );
  void ApplyCameraDataForFirstCam();

  bool IsClipUsedInSubScene( int clipIdx );
};

void InitDefaultStates( CCoreDevice *Device );
void DeinitDefaultStates();

struct SceneObjCopyData
{
  CphxScene_Tool* scene = nullptr;
  CphxObject_Tool* obj = nullptr;
  int clip = 0;
};