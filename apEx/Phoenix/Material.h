#pragma once
#include "phxEngine.h"
#include "RenderLayer.h"

enum MATERIALPARAMSCOPE
{
  PARAM_CONSTANT = 0,
  PARAM_VARIABLE = 1,
  PARAM_ANIMATED = 2,
};

enum COMPARISONFUNCTION
{
  COMPARE_LESSEQUAL = 0,
  COMPARE_NEVER = 1,
  COMPARE_LESS = 2,
  COMPARE_EQUAL = 3,
  COMPARE_GREATER = 4,
  COMPARE_NOTEQUAL = 5,
  COMPARE_GREATEREQUAL = 6,
  COMPARE_ALWAYS = 7,
};

enum MATERIALPARAMTYPE
{
  //data types added here need to be named in the MaterialTypeNames[] array
  PARAM_FLOAT = 0,
  PARAM_COLOR,
  PARAM_ZMODE,
  PARAM_ZFUNCTION,
  PARAM_FILLMODE,
  PARAM_CULLMODE,
  PARAM_RENDERPRIORITY,
  PARAM_TEXTURE0,
  PARAM_TEXTURE1,
  PARAM_TEXTURE2,
  PARAM_TEXTURE3,
  PARAM_TEXTURE4,
  PARAM_TEXTURE5,
  PARAM_TEXTURE6,
  PARAM_TEXTURE7,
  PARAM_BLENDMODE0,
  PARAM_BLENDMODE1,
  PARAM_BLENDMODE2,
  PARAM_BLENDMODE3,
  PARAM_BLENDMODE4,
  PARAM_BLENDMODE5,
  PARAM_BLENDMODE6,
  PARAM_BLENDMODE7,
  PARAM_RENDERTARGET, //only for postproc
  PARAM_PARTICLELIFEFLOAT,
  PARAM_DEPTHTEXTURE7,
  PARAM_3DTEXTURE6,
  PARAM_MESHDATA0,
  PARAM_MESHDATA1,
  PARAM_MESHDATA2,
  PARAM_MESHDATA3,
  PARAM_MESHDATA4,
  PARAM_MESHDATA5,
  PARAM_MESHDATA6,
  PARAM_MESHDATA7,
  PARAM_PARTICLELIFE,
  PARAM_LTC1,
  PARAM_LTC2,

  PARAM_COUNT, //leave this at the end!
};

#define phxSrcBlend_ZERO			0x00
#define phxSrcBlend_ONE				0x01
#define phxSrcBlend_SRCCOLOR		0x02
#define phxSrcBlend_INVSRCCOLOR		0x03
#define phxSrcBlend_SRCALPHA		0x04
#define phxSrcBlend_INVSRCALPHA		0x05
#define phxSrcBlend_DESTALPHA		0x06
#define phxSrcBlend_INVDESTALPHA	0x07
#define phxSrcBlend_DESTCOLOR		0x08
#define phxSrcBlend_INVDESTCOLOR	0x09
#define phxSrcBlend_SRCALPHASAT		0x0a
#define phxSrcBlend_BOTHSRCALPHA	0x0b
#define phxSrcBlend_BOTHINVSRCALPHA	0x0c
#define phxSrcBlend_BLENDFACTOR		0x0d
#define phxSrcBlend_INVBLENDFACTOR	0x0e

#define phxDstBlend_ZERO			0x00
#define phxDstBlend_ONE				0x10
#define phxDstBlend_SRCCOLOR		0x20
#define phxDstBlend_INVSRCCOLOR		0x30
#define phxDstBlend_SRCALPHA		0x40
#define phxDstBlend_INVSRCALPHA		0x50
#define phxDstBlend_DESTALPHA		0x60
#define phxDstBlend_INVDESTALPHA	0x70
#define phxDstBlend_DESTCOLOR		0x80
#define phxDstBlend_INVDESTCOLOR	0x90
#define phxDstBlend_SRCALPHASAT		0xa0
#define phxDstBlend_BOTHSRCALPHA	0xb0
#define phxDstBlend_BOTHINVSRCALPHA	0xc0
#define phxDstBlend_BLENDFACTOR		0xd0
#define phxDstBlend_INVBLENDFACTOR	0xe0


union MATERIALVALUE
{
  float Float;
  float Color[ 4 ];
  unsigned char ZMode;
  unsigned char BlendMode;
  D3D11_COMPARISON_FUNC ZFunction;
  bool Wireframe;
  D3D11_CULL_MODE CullMode;
  int RenderPriority;
  ID3D11ShaderResourceView *TextureView;
  CphxRenderTarget *RenderTarget;
};

struct CphxMaterialParameter
{
  MATERIALPARAMTYPE Type;
  MATERIALPARAMSCOPE Scope;
  MATERIALVALUE Value;
};

struct CphxMaterialParameterBatch //stores parameter data for a tech or a pass
{
  int ParamCount;
  CphxMaterialParameter **Parameters;
  float CollectedData[ MATERIALDATASIZE / 4 ];
  int CollectAnimatedData();
};

struct CphxMaterialSpline
{
  CphxMaterialParameter *Target;
  void *GroupingData;
  class CphxSpline_float16 *Splines[ 4 ];
  MATERIALVALUE GetValue();
  void CalculateValue( float t );
};

struct CphxMaterialSplineBatch //stores instanced parameter data for anything that uses a material
{
  int SplineCount;
  CphxMaterialSpline **Splines;

  void CalculateValues( float t );
  void ApplyToParameters( void *GroupingData );
};

enum SHADERTYPE
{
  SHADER_VERTEX = 0,
  SHADER_PIXEL = 1,
  SHADER_GEOMETRY = 2,
  SHADER_HULL = 3,
  SHADER_DOMAIN = 4,

  SHADER_TYPE_COUNT,
};

struct CphxMaterialRenderPass
{
  CphxMaterialParameterBatch Parameters;

  //bool Wireframe;

  ID3D11VertexShader *VS; //WARNING! DO NOT CHANGE THE ORDER OF THESE DUE TO A MEMCPY IN CreateRenderDataInstances()
  ID3D11PixelShader *PS;
  ID3D11GeometryShader *GS;
  ID3D11HullShader *HS;
  ID3D11DomainShader *DS;

#ifdef _DEBUG
  char* shaderText;
#endif

  //D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
  //D3D11_RASTERIZER_DESC RasterizerDesc;
  //D3D11_BLEND_DESC BlendDesc;

  //ID3D11BlendState *FinalBlendState;
  //ID3D11RasterizerState *FinalRasterizerState;
  //ID3D11DepthStencilState *FinalDepthStencilState;
};

struct CphxTechRenderStateBatch
{
  ID3D11Buffer *TechLevelParams;
  ID3D11Buffer **PassParams;
};

enum TECHNIQUETYPE
{
  TECH_MATERIAL = 0,
  TECH_POSTPROCESS = 1,
  TECH_SHADERTOY = 2,
  TECH_PARTICLE = 3,
};

class CphxModelObject_Mesh;
class CphxScene;
class CphxMaterialPassConstantState;

struct CphxMaterialTechnique
{
  CphxMaterialParameterBatch Parameters;

  TECHNIQUETYPE Type;
  CphxRenderLayerDescriptor *TargetLayer;

  int PassCount;
  CphxMaterialRenderPass **RenderPasses;

  //void BuildTechRS(CphxTechRenderStateBatch *Target);
  //CphxTechRenderStateBatch *CreateRSBatch();

  //void CreateRenderDataInstances(CphxModelObject_Mesh *Model, CphxScene *RootScene, int &passid, void *ToolData);
  void CreateRenderDataInstances( CphxMaterialPassConstantState **MaterialState, int &passid, CphxScene *RootScene, ID3D11Buffer *VertexBuffer, ID3D11Buffer *IndexBuffer, ID3D11Buffer *WireBuffer, int VertexCount, int IndexCount, void *ToolData, bool Indexed );
  void CollectAnimatedData( CphxMaterialPassConstantState *State, int Pass );
};

struct CphxMaterial
{
  int TechCount;
  CphxMaterialTechnique **Techniques;

  int PassCount; //calculated summary pass count used by the minimalengine

  void CreateRenderDataInstances( CphxModelObject_Mesh *Model, CphxScene *RootScene, void *ToolData );
  void CreateRenderDataInstances( CphxMaterialPassConstantState **State, CphxScene *RootScene, ID3D11Buffer *VertexBuffer, int VertexCount );
};