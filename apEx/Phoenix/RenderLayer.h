#pragma once
#include "phxarray.h"
#include "RenderTarget.h"
#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")


class CphxRenderDataInstance
{

public:

  //class CphxModelObject_Mesh *Model;

  int RenderPriority;

  bool Wireframe;
  bool Indexed;

  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;
  ID3D11Buffer *WireBuffer;
  int TriIndexCount;
  int WireIndexCount;

  ID3D11VertexShader *VS;  //WARNING! DO NOT CHANGE THE ORDER OF THESE DUE TO A MEMCPY IN CreateRenderDataInstances() in Material.cpp!
  ID3D11PixelShader *PS;
  ID3D11GeometryShader *GS;
  ID3D11HullShader *HS;
  ID3D11DomainShader *DS;

  ID3D11BlendState *BlendState; //WARNING! DO NOT CHANGE THE ORDER OF THESE DUE TO A MEMCPY IN CreateRenderDataInstances() in Material.cpp!
  ID3D11RasterizerState *RasterizerState;
  ID3D11DepthStencilState *DepthStencilState;
  ID3D11ShaderResourceView *Textures[ 8 ];

  //ID3D11Buffer *Buffers[3]; //buffer 0 is the matrix data, 1 is the static material data, 2 is the dynamic material data	

  D3DXMATRIX Matrices[ 2 ];
  float MaterialData[ MATERIALDATASIZE / 4 ];

  void *ToolData;

  void Render();

  //CphxRenderDataInstance() { Buffers[0] = NULL; Buffers[1] = NULL; Buffers[2] = NULL; }
  //~CphxRenderDataInstance() { if (Buffers[0]) Buffers[0]->Release(); Buffers[0] = NULL; }

};

class CphxRenderLayerDescriptor
{

public:

  bool OmitDepthBuffer;
  bool VoxelizerLayer;
  bool clearRenderTargets;
  int TargetCount;
  CphxRenderTarget **Targets;

#ifndef PHX_MINIMAL_BUILD
  bool IgnoreHelperObjects;
#endif

  void SetEnvironment( bool ClearColor, bool ClearZ, int cubeResolution );
  void GenMipmaps();

};

class CphxRenderLayer
{
public:

  CphxRenderLayerDescriptor *Descriptor;
  CphxArray<CphxRenderDataInstance*> RenderInstances;

};


void __fastcall GetRenderTargetResolution( unsigned char ResData, int &XRes, int &YRes, int DemoResX, int DemoResY );
