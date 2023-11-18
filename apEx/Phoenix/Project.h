#pragma once
#include "phxEngine.h"
#include "Texgen.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "Scene.h"
#include "Timeline.h"
#include "../MinimalPlayer/SetupDialog.h"

enum PHXDATASTREAMS
{
  Stream_Main = 0,
  Stream_ASCIIZ,						//  1,
  Stream_Shaders,						//  2,
  Stream_SplineDescriptors,			//  3,
  Stream_SplineKeyCounts,				//  4,
  Stream_SplineKeyTimes,				//  5,
  Stream_SplineKeyValues,				//  6,
  Stream_TextureOperatorReference,	//  7,
  Stream_MaterialParam,				//  8, // unsigned char misc data
  Stream_ModelPrimitives,				//  9,
  Stream_ModelTransformation,			//  10,
  Stream_ModelParameters,				//  11,
  Stream_ModelReferences,				//  12, // - for clones, scatters etc
  Stream_MaterialID,					//  13,
  Stream_ModelFilterCount,			//  14,
  Stream_ModelFilterType,				//  15,
  Stream_ModelFilterData,				//  16,
  Stream_EventType,					//  17,
  Stream_EventPosition,				//  18, //start and end positions
  //Stream_MaterialParamDefault,					//  19
  //Stream_SplineStorageData,			//  20,
  //Stream_FloatSignExponent,			//  21,
  //Stream_FloatMantissa,				//  22,

  StreamCount	//leave this at the end, used as an array size index
};

class CphxProject
{

  //void ImportMaterialParams(CphxMaterialParameterBatch &Params);
  //void ImportMaterialParamValue(CphxMaterialParameter *Param);
  //void ImportVariableMaterialData(CphxMaterial *&mat, CphxMaterialPassConstantState **&state);
  //void ImportClipSpline(CphxClipSpline *Spline);
  //void ImportSpline(CphxSpline_Quaternion16 *Spline);
  //void ImportSpline(CphxSpline_float16 *Spline);
  //void GenerateMesh(CphxModelObject *o, CphxModel *m);
  //void BuildMaterialState(CphxMaterial *mat, CphxMaterialPassConstantState** Passes);
  //void BuildPassState(CphxMaterialTechnique *tech, CphxMaterialRenderPass *Pass, CphxMaterialPassConstantState* State);
  //void CollectRenderState(CphxMaterialParameterBatch &Params, D3D11_RASTERIZER_DESC &RasterDesc, D3D11_DEPTH_STENCIL_DESC &DepthStencilDesc, D3D11_BLEND_DESC &BlendDesc, CphxMaterialPassConstantState *pass);
  //void ImportMaterialSplineBatch(CphxModel *m, CphxObjectClip *clip, int ClipID);
  //void ImportTechSplineBatch(CphxMaterialTechnique *m, void *group, CphxMaterialSplineBatch *clip, int ClipID);
  //void ImportMaterialParamSplineBatch(CphxMaterialParameterBatch *m, void *group, CphxMaterialSplineBatch *clip, int ClipID);

  //int LastPrecalcTime;
  //bool DoPrecalc(CphxProject *Precalc, float Pos);

public:

  //////////////////////////////////////////////////////////////////////////
  // Shader Pool

  ID3D11VertexShader *VertexShaders;
  ID3D11PixelShader *PixelShaders;

  //////////////////////////////////////////////////////////////////////////
  // Texture generator

  PHXTEXTUREFILTER *TextureFilters;
  PHXTEXTURESUBROUTINE *TextureSubroutines;
  PHXTEXTUREOPERATOR *TextureOperators;

  ID3D11Texture2D *Textures;

  //CphxRenderTarget *RenderTargets;
  CphxRenderLayerDescriptor *RenderLayers;

  //////////////////////////////////////////////////////////////////////////
  // Mesh generator

  CphxModel *Models;

  //////////////////////////////////////////////////////////////////////////
  // Materials

  CphxMaterialTechnique *Techniques;
  CphxMaterial *Materials;

  //////////////////////////////////////////////////////////////////////////
  // Scenes

  CphxScene *Scenes;

  //////////////////////////////////////////////////////////////////////////
  // Time-line

  CphxTimeline Timeline;

  //////////////////////////////////////////////////////////////////////////
  // Functions

  void LoadProject( unsigned char *Data, CphxProject *Precalc, SETUPCFG &setup );
  void Render( unsigned int Frame );
};

