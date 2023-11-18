#include "BasePCH.h"
#include "DX9Texture.h"

#ifdef CORE_API_DX9

CCoreDX9Texture2D::CCoreDX9Texture2D(CCoreDX9Device *dev) : CCoreTexture2D(dev)
{
	Dev = dev->GetDevice();
	TextureHandle = NULL;
	SurfaceHandle = NULL;
	RenderTarget = false;
};

CCoreDX9Texture2D::~CCoreDX9Texture2D()
{
	Release();
}

void CCoreDX9Texture2D::Release()
{
	if (TextureHandle) TextureHandle->Release();
	if (SurfaceHandle) SurfaceHandle->Release();
	TextureHandle = NULL;
	SurfaceHandle = NULL;
}

TBOOL CCoreDX9Texture2D::SetToSampler(const CORESAMPLER smp)
{
	return Dev->SetTexture(DX9Samplers[smp], TextureHandle) == D3D_OK;
}

TBOOL CCoreDX9Texture2D::Create(const TS32 xres, const TS32 yres, const TU8 *Data, const TS8 BytesPerPixel, const COREFORMAT format, const TBOOL rendertarget)
{
	if (xres <= 0 || yres <= 0 || format == COREFMT_UNKNOWN) return false;
	Release();

	DWORD Usage = D3DUSAGE_AUTOGENMIPMAP;//|D3DUSAGE_WRITEONLY;
	D3DFORMAT txFormat = DX9Formats[format];
	D3DPOOL Pool = D3DPOOL_MANAGED;

	if (rendertarget)
	{
		Usage |= D3DUSAGE_RENDERTARGET;
		Pool = D3DPOOL_DEFAULT;
	}

	if (Dev->CreateTexture(xres, yres, 0, Usage, txFormat, Pool, &TextureHandle, NULL) != D3D_OK) return false;

	if (TextureHandle->GetSurfaceLevel(0, &SurfaceHandle) != D3D_OK)
	{
		Release();
		return false;
	}

	if (!Update(Data, xres, yres, BytesPerPixel))
	{
		Release();
		return false;
	}

	XRes = xres;
	YRes = yres;
	Format = format;
	RenderTarget = rendertarget;
	return true;
}

TBOOL CCoreDX9Texture2D::Create(const TU8 *Data, const TS32 Size)
{
	if (!Data || Size <= 0) return false;
	Release();

#ifdef CORE_API_D3DX
	if (D3DXCreateTextureFromFileInMemory(Dev, Data, Size, &TextureHandle) != D3D_OK)
		return false;
#else
#ifdef CORE_VERBOSE_LOG

	//LOG("",aksdf,"",...)

	LOG(LOG_ERROR, _T("[core] Texture Creation From Image failed: d3dx not linked"));
#endif
	return false;
#endif

	if (TextureHandle->GetSurfaceLevel(0, &SurfaceHandle) != D3D_OK)
	{
		Release();
		return false;
	}

	D3DSURFACE_DESC texturedesc;
	TextureHandle->GetLevelDesc(0, &texturedesc);
	XRes = texturedesc.Width;
	YRes = texturedesc.Height;
	Format = GetFormat(texturedesc.Format);
	RenderTarget = false;

	return true;
}

TBOOL CCoreDX9Texture2D::Lock(void **Result, TS32 &pitch)
{
	if (!TextureHandle) return false;

	D3DLOCKED_RECT Rect;

	if (TextureHandle->LockRect(0, &Rect, NULL,/*D3DLOCK_DISCARD*/0) != D3D_OK)
		return false;

	*Result = Rect.pBits;
	pitch = Rect.Pitch;
	return true;
}

TBOOL CCoreDX9Texture2D::UnLock()
{
	if (!TextureHandle) return false;

	return TextureHandle->UnlockRect(0) == D3D_OK;
}

void CCoreDX9Texture2D::OnDeviceLost()
{
	if (RenderTarget) Release();
}

void CCoreDX9Texture2D::OnDeviceReset()
{
	if (RenderTarget && XRes > 0 && YRes > 0 && Format != COREFMT_UNKNOWN)
		BASEASSERT(Create(XRes, YRes, NULL, 0, Format, RenderTarget));
}

TBOOL CCoreDX9Texture2D::Update(const TU8 *Data, const TS32 XRes, const TS32 YRes, const TS8 BytesPerPixel)
{
	void *d = NULL;
	TS32 Pitch = 0;
	if (!Lock(&d, Pitch)) return false;

	TS8 *p = (TS8*)d;

	TS32 LineWidth = XRes*BytesPerPixel;

	for (TS32 y = 0; y < YRes; y++)
	{
		memcpy(p, Data, LineWidth);
		p += Pitch;
		Data += LineWidth;
	}

	if (!UnLock()) return false;
	return true;
}

CCoreTexture2D * CCoreDX9Texture2D::Copy()
{
	BASEASSERT(false);
}

void CCoreDX9Texture2D::ExportToImage(CString &Filename, TBOOL ClearAlpha, EXPORTIMAGEFORMAT Format)
{
	BASEASSERT(false);
}

CCoreDX9Texture3D::CCoreDX9Texture3D(CCoreDX9Device *dev) : CCoreTexture3D(dev)
{
	Dev = dev->GetDevice();
};


CCoreDX9TextureCube::CCoreDX9TextureCube(CCoreDX9Device *dev) : CCoreTextureCube(dev)
{
	Dev = dev->GetDevice();
};

#else
NoEmptyFile();
#endif