#pragma once
#include "phxEngine.h"
#include "Mesh.h"
#include "Material.h"

#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")

class CphxScene;
struct CphxObjectClip;

class CphxModelObject
{
public:

  unsigned char parentobjcount;
  unsigned char *parentobjects;

  void *ToolObject;
  virtual void CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *CloneData ) = 0;

  D3DXFLOAT16 TransformationF16[ 12 ];

  //D3DXMATRIX Matrix;
  D3DXMATRIX GetMatrix();

  PHXMESHPRIMITIVE Primitive;
  bool minimalGenerated;

  // for clones only

  CphxArray<CphxModelObject*> ClonedObjects;

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxModelObject() {};
#endif

};

class CphxMaterialPassConstantState
{
public:

  ID3D11BlendState *BlendState; //WARNING! DO NOT CHANGE THE ORDER OF THESE DUE TO A MEMCPY IN CreateRenderDataInstances() in Material.cpp!
  ID3D11RasterizerState *RasterizerState;
  ID3D11DepthStencilState *DepthStencilState;
  ID3D11ShaderResourceView *Textures[ 8 ];

  bool Wireframe;
  unsigned char RenderPriority;
  CphxRenderTarget *RenderTarget;
  float ConstantData[ MATERIALDATASIZE / 4 ];
  float AnimatedData[ MATERIALDATASIZE / 4 ];
  int ConstantDataSize;
  int AnimatedDataSize;

  ID3D11ShaderResourceView *TextureBackup[ 8 ]; //used by the minimalengine to store original texture references when the internal temp buffer is copied to
};

class CphxModelObject_Mesh : public CphxModelObject
{
public:
  unsigned char MeshParameters[ 14 ];
  char *Text; // also used as minimesh pointer
  CphxVertex *StoredVertices;
  int StoredVertexCount;
  CphxPolygon *StoredPolygons;
  int StoredPolyCount;

  ID3D11Buffer *VxBuffer;
  ID3D11Buffer *IndexBuffer;
  ID3D11Buffer *WireBuffer;
  int VxCount, TriCount, EdgeCount;

  ID3D11Buffer *dataBuffer;
  ID3D11ShaderResourceView *dataBufferView;

  CphxMaterialPassConstantState **MaterialState; //contains per object constant material state for each material pass

  CphxMaterial *Material;
  CphxMesh Mesh;

  int FilterCount;
  PHXMESHFILTERDATA *FilterData;

  void CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *CloneData );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxModelObject_Mesh() {};
#endif

};

class CphxModelObject_Clone : public CphxModelObject
{
public:

  void CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *CloneData );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxModelObject_Clone() {};
#endif

};

class CphxModel
{
public:

  CphxArray<CphxModelObject*> Objects;

  void CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *ToolData );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxModel() {};
#endif


};