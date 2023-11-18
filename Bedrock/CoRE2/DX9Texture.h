#pragma once
#include "DX9Device.h"
#include "Texture.h"

#ifdef CORE_API_DX9

class CCoreDX9Texture2D : public CCoreTexture2D
{
	LPDIRECT3DDEVICE9 Dev;

	LPDIRECT3DTEXTURE9 TextureHandle;
	LPDIRECT3DSURFACE9 SurfaceHandle;

	TBOOL RenderTarget;

	virtual void Release();
	virtual TBOOL SetToSampler(const CORESAMPLER Sampler);

public:

	CCoreDX9Texture2D(CCoreDX9Device *Device);
	virtual ~CCoreDX9Texture2D();

	virtual void OnDeviceLost();
	virtual void OnDeviceReset();

	virtual TBOOL Create(const TS32 XRes, const TS32 YRes, const TU8 *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false);
	virtual TBOOL Create(const TU8 *Data, const TS32 Size);
	virtual TBOOL CreateDepthBuffer(const TS32 XRes, const TS32 YRes);
	virtual TBOOL Lock(void **Result, TS32 &pitch);
	virtual TBOOL UnLock();

	virtual TBOOL Update(const TU8 *Data, const TS32 XRes, const TS32 YRes, const TS8 BytesPerPixel = 4);

	virtual CCoreTexture2D *Copy();
	virtual void ExportToImage(CString &Filename, TBOOL ClearAlpha, EXPORTIMAGEFORMAT Format);
};

class CCoreDX9Texture3D : public CCoreTexture3D
{
	LPDIRECT3DDEVICE9 Dev;

public:

	CCoreDX9Texture3D(CCoreDX9Device *Device);

};

class CCoreDX9TextureCube : public CCoreTextureCube
{
	LPDIRECT3DDEVICE9 Dev;

public:

	CCoreDX9TextureCube(CCoreDX9Device *Device);

};

#endif