#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../Phoenix/Material.h"
#include "phxResource.h"
#include "phxSplineExt.h"

class CphxRenderLayerDescriptor_Tool;
class CphxMaterialParameterBatch_Tool;
class CphxMaterialTechnique_Tool;
class CphxModel_Tool;
class CphxEvent_Shadertoy_Tool;
class CphxObjectClip_Tool;
class CphxObject_ParticleEmitter_CPU_Tool;

//class CphxMaterialParameterValue_Tool
//{
//public:
//
//	CphxMaterialParameterValue Value;
//	class CphxSpline_Tool_float16 *Splines[4];
//
//	CphxMaterialParameterValue_Tool();
//	virtual ~CphxMaterialParameterValue_Tool();
//
//};
//
class CphxMaterialSpline_Tool
{
public:
  CphxGUID TargetParamGUID;
  CphxGUID TargetObjectGUID;
  CphxSpline_Tool_float16 Splines[ 4 ];
  CphxMaterialSpline Spline;
  //CphxMaterialParameterValue_Tool *Value;

  CphxMaterialSpline_Tool();
  virtual ~CphxMaterialSpline_Tool();
};

TU32 DictionaryHash( CphxGUID g );

class CphxMaterialSplineBatch_Tool
{
  void ImportParameters( CphxMaterialParameterBatch_Tool *Batch, void *GroupingData, CphxGUID &GroupingGUID );

public:

  CArray<CphxMaterialSpline*> MinimalSplineArray;
  CArray<CphxMaterialSpline_Tool*> Splines;
  CArray<CphxMaterialSpline_Tool*> SplineBackup;
  CphxMaterialSplineBatch *Batch;

  //CDictionary<CphxGUID, CphxMaterialSpline_Tool*> Storage;

  CphxMaterialSplineBatch_Tool();
  virtual ~CphxMaterialSplineBatch_Tool();

  void ImportTech( CphxMaterialTechnique_Tool *Tech, void *GroupingData, CphxGUID &GroupingGUID );

  void BuildBatch( CphxModel_Tool *Model );
  void BuildBatch( CphxEvent_Shadertoy_Tool *Model );
  void BuildBatch( CphxObject_ParticleEmitter_CPU_Tool *Model );
  void BackupSplines();
  void RestoreSplines();

  void Copy( CphxMaterialSplineBatch_Tool *Original );
  //void BuildLinkBatch(CphxMaterialSplineBatch *SplineBatch);

  CphxMaterialSpline_Tool *FindSpline( class CphxMaterialParameter_Tool *param, void *GroupingData );
};

class CphxMaterialParameter_Tool : public CphxResource
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  CString Name;

  CphxMaterialParameter Parameter;
  MATERIALVALUE DefaultValue;
  CphxGUID TextureGUID;

  CphxMaterialParameter_Tool();
  virtual ~CphxMaterialParameter_Tool();

  TBOOL GenerateResource( CCoreDevice *Dev ) { return true; }
  virtual PHXRESOURCETYPE GetType() { return PHX_MATERIALPARAM; }
  void SetTextureGUID( CphxGUID &g );

  bool IsDefault();
};

class CphxMaterialPassConstantState;
class CphxRenderTarget_Tool;

class CphxParameterDataCollector
{
public:

  CArray<TF32> StaticFloatData;
  CArray<TF32> DynamicFloatData;

  D3D11_RASTERIZER_DESC RasterDesc;
  D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
  D3D11_BLEND_DESC BlendDesc;
  ID3D11ShaderResourceView *Textures[ 8 ];
  CphxRenderTarget *RenderTarget;
  TBOOL Wireframe;
  TS32 RenderPriority;

  CphxParameterDataCollector();
  virtual ~CphxParameterDataCollector();
  void Apply( CphxMaterialPassConstantState *p );
  void ApplyTextures( CphxMaterialPassConstantState *p );
};

class CphxMaterialParameterBatch_Tool
{
public:

  virtual void Export( CXMLNode *Node );
  virtual void Import( CXMLNode *Node );

  CArray<CphxMaterialParameter_Tool*> Parameters;

  CphxMaterialParameterBatch_Tool();
  virtual ~CphxMaterialParameterBatch_Tool();

  CphxMaterialParameter_Tool *CreateParam();

  void Rebuild( CphxMaterialParameterBatch *Original );
  void CollectData( CphxParameterDataCollector &Collector );
};

class CphxMaterialRenderPass_Tool
{
  CString LastCompiledCode;
  CString LastCompileError;

public:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CString Name;
  CString ShaderCode;
  CString CodeBackup; // for shader minimization purposes
  CphxMaterialParameterBatch_Tool PassParameters;


  CCorePixelShader *PS;
  CCoreVertexShader *VS;
  CCoreGeometryShader *GS;
  CCoreDomainShader *DS;
  CCoreHullShader *HS;

  CphxMaterialRenderPass *Pass;

  TBOOL Minifiable = true;

  CphxMaterialRenderPass_Tool();
  virtual ~CphxMaterialRenderPass_Tool();

  void CompileShaders( CCoreDevice *Dev, CString &Error );

  CphxMaterialParameter_Tool *CreateParam();
  void DeleteParam( TS32 p );
  void Rebuild();

};

class CphxMaterialPassConstantState;

class CphxMaterialTechnique_Tool : public CphxResource
{

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  TBOOL External;

  CphxGUID TargetLayer;

  CString Name;
  CphxMaterialParameterBatch_Tool TechParameters;

  CphxMaterialTechnique *Tech;
  CArray<CphxMaterialRenderPass_Tool*> Passes;

  CphxMaterialTechnique_Tool();
  virtual ~CphxMaterialTechnique_Tool();

  CphxMaterialRenderPass_Tool *CreatePass();
  CphxMaterialParameter_Tool *CreateParam();
  void DeleteParam( TS32 p );

  TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType() { return PHX_MATERIALTECH; }

  void Rebuild();
  void SetTargetLayer( CphxRenderLayerDescriptor_Tool *t );
  void CreateInstancedData( CphxMaterialPassConstantState **MaterialState, TS32 &passcnt );
  void CreateInstancedData_Textures( CphxMaterialPassConstantState **MaterialState, TS32 &passcnt );

  CphxMaterialParameter_Tool *GetParameter( CphxGUID &g );

  void RebuildDependents();
};

class CphxMaterial_Tool : public CphxResource
{

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  CphxMaterial Material;

  CString Name;
  CArray<CphxMaterialTechnique_Tool*> Techniques;
  TBOOL External;

  CphxMaterial_Tool();
  virtual ~CphxMaterial_Tool();

  TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType() { return PHX_MATERIAL; }

  void AddTech( CphxMaterialTechnique_Tool *t );
  void ReplaceTech( CphxMaterialTechnique_Tool *original, CphxMaterialTechnique_Tool *updated );

  CphxMaterialParameter_Tool *GetParameter( CphxGUID &guid );

  void RebuildDependents();
};

extern CphxMaterial_Tool *DefaultMaterial;
extern CphxMaterialTechnique_Tool *DefaultTech;
