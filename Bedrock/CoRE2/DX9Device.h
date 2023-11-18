#pragma once
#include "Core2_Config.h"
#include "../BaseLib/BaseLib.h"
#include "Device.h"

#ifdef CORE_API_DX9

#include "DX9Enums.h"

class CCoreDX9Device : public CCoreDevice
{
	LPDIRECT3D9 D3D;
	LPDIRECT3DDEVICE9 Device;
	D3DPRESENT_PARAMETERS D3DPP;

	LPDIRECT3DSURFACE9 BackBufferSurface;

	virtual void ResetPrivateResources();
	virtual TBOOL InitAPI(const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel = 0, const TS32 RefreshRate = 60);
	virtual TBOOL ApplyRenderState(const CORESAMPLER Sampler, const CORERENDERSTATE RenderState, const CORERENDERSTATEVALUE Value);
	virtual TBOOL SetNoVertexBuffer();
	virtual TBOOL CommitRenderStates();

public:

	CCoreDX9Device();
	virtual ~CCoreDX9Device();
	INLINE LPDIRECT3DDEVICE9 GetDevice() { return Device; }
	virtual COREDEVICEAPI GetAPIType() { return COREAPI_DX9; }

	//this initializer will change to accommodate multiple platforms at once once we get to that point:
	TBOOL Initialize(CCoreWindowHandler *Window, const TS32 AALevel = 0);

	virtual TBOOL DeviceOk();
	virtual TBOOL IsWindowed();
	virtual void Resize(const TS32 xr, const TS32 yr);
	virtual void SetFullScreenMode(const TBOOL FullScreen, const TS32 xr, const TS32 yr);

	//////////////////////////////////////////////////////////////////////////
	// texture functions

	virtual CCoreTexture2D *CreateTexture2D(const TS32 XRes, const TS32 YRes, const TU8 *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false);
	virtual CCoreTexture2D *CreateTexture2D(const TU8 *Data, const TS32 Size);
	virtual CCoreTexture2D *CopyTexture(CCoreTexture2D *Texture);

	//////////////////////////////////////////////////////////////////////////
	// vertexbuffer functions

	virtual CCoreVertexBuffer *CreateVertexBuffer(const TU8 *Data, const TS32 Size);
	virtual CCoreVertexBuffer *CreateVertexBufferDynamic(const TS32 Size);

	//////////////////////////////////////////////////////////////////////////
	// indexbuffer functions

	virtual CCoreIndexBuffer *CreateIndexBuffer(const TS32 IndexCount, const TS32 IndexSize = 2);

	//////////////////////////////////////////////////////////////////////////
	// vertexformat functions

	virtual CCoreVertexFormat *CreateVertexFormat(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs = NULL);

	//////////////////////////////////////////////////////////////////////////
	// shader functions

	virtual CCoreVertexShader *CreateVertexShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err);
	virtual CCorePixelShader *CreatePixelShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err);

	//////////////////////////////////////////////////////////////////////////
	// display functions

	virtual TBOOL BeginScene();
	virtual TBOOL EndScene();
	virtual TBOOL Clear(const TBOOL clearPixels = true, const TBOOL clearDepth = true, const CColor &Color = CColor((TU32)0), const TF32 Depth = 1, const TS32 Stencil = 0);
	virtual TBOOL Flip();
	virtual TBOOL DrawIndexedTriangles(TS32 Count, TS32 NumVertices);
	virtual TBOOL DrawTriangles(TS32 Count);
	virtual TBOOL DrawLines(TS32 Count);

	virtual void TakeScreenShot(CString Filename) {};

	virtual TF32 GetUVOffset() { return 0.5f; }

	virtual TBOOL SetRenderTarget(CCoreTexture2D *RT);
	virtual TBOOL SetViewport(CRect Viewport);

	virtual CCoreGeometryShader *CreateGeometryShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreDomainShader *CreateDomainShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreHullShader *CreateHullShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreComputeShader *CreateComputeShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL);
	virtual CCoreVertexShader *CreateVertexShader();
	virtual CCorePixelShader *CreatePixelShader();
	virtual CCoreGeometryShader *CreateGeometryShader();
	virtual CCoreDomainShader *CreateDomainShader();
	virtual CCoreHullShader *CreateHullShader();
	virtual CCoreComputeShader *CreateComputeShader();
	virtual void SetShaderConstants(TS32 Slot, TS32 Count, CCoreConstantBuffer **Buffers);
	virtual CCoreConstantBuffer *CreateConstantBuffer();

	virtual CCoreBlendState *CreateBlendState();
	virtual CCoreDepthStencilState *CreateDepthStencilState();
	virtual CCoreRasterizerState *CreateRasterizerState();
	virtual CCoreSamplerState *CreateSamplerState();

};

#endif