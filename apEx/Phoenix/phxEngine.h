#pragma once

#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

#include <d3d11.h>
#include <D3DX11.h>
#include "../MVX/mvx.h"

#define MAX_LIGHT_COUNT 8
#define PHOENIX_ENGINE_LINEAR_RENDER 1

extern ID3D11Device *phxDev;
extern ID3D11DeviceContext *phxContext;
extern ID3D11Buffer *TexgenBufferPS;
extern ID3D11Buffer *TexgenVertexBuffer;
extern ID3D11InputLayout *TexgenVertexFormat;
extern ID3D11InputLayout *RenderVertexFormat;
#ifdef PHX_HAS_STANDARD_PARTICLES
extern ID3D11InputLayout *ParticleVertexFormat;
#endif
extern ID3D11VertexShader *TexgenVertexShader;
extern ID3D11SamplerState *TexgenSampler;
extern ID3D11SamplerState *TexgenSampler_NoWrap;
extern ID3D11SamplerState *TexgenSampler_ShadowCompare;
extern ID3D11Buffer *SceneDataBuffer;
extern ID3D11Buffer *ObjectMatrixBuffer;
extern ID3D11DepthStencilView *phxDepthBufferView;
extern ID3D11ShaderResourceView *phxDepthBufferShaderView;
extern ID3D11ShaderResourceView *phxMeshDataShaderView;
extern ID3D11RenderTargetView *phxBackBufferView;
extern ID3D11PixelShader *RenderPixelShader;
extern ID3D11ShaderResourceView *ltc1view, *ltc2view;

#ifdef PHX_VOLUMETRIC_RENDERTARGETS
extern ID3D11Texture3D *phxTexture3D;
extern ID3D11ShaderResourceView *phxTexture3DResourceView;
extern ID3D11UnorderedAccessView *phxTexture3DUAV;
extern ID3D11RenderTargetView *phxVoxelForceMSAARTView;
extern ID3D11Texture2D *phxVoxelForceMSAATexture;
#endif

extern bool phxDone;

//scene data: view+projection matrices
#define SCENEDATASIZE (16*4*2+4*4*2+MAX_LIGHT_COUNT*(6*4*4)+16+16*4+16*4+16*4*2)
#define MATERIALDATASIZE (64*sizeof(float))

//#define PHXVERTEXFORMAT (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX4 | D3DFVF_DIFFUSE)
#define PHXVERTEXFORMATSIZE ((3+3+3+2*4)*sizeof(float)+sizeof(unsigned int))

typedef HRESULT( __stdcall d3dcompilefunc )( LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs );
extern d3dcompilefunc *D3DCompileCall;

typedef DWORD( __stdcall *fTimerFunction )( );
extern fTimerFunction TimerFunction;

void __fastcall InitializePhoenix();
void DeinitializePhoenix();

extern char* EngineFontList[ 32 ];
//void FixFloatingPoint();
//void UnFixFloatingPoint();
