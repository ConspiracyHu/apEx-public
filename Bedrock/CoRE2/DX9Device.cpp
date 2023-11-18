#include "BasePCH.h"
#include "DX9Device.h"
#include "DX9Texture.h"
#include "DX9VertexBuffer.h"
#include "DX9IndexBuffer.h"
#include "DX9VertexFormat.h"
#include "DX9Shader.h"

#ifdef CORE_API_DX9

CCoreDX9Device::CCoreDX9Device()
{
	D3D = NULL;
	Device = NULL;
	BackBufferSurface = NULL;
	memset(&D3DPP, 0, sizeof(D3DPRESENT_PARAMETERS));
}

CCoreDX9Device::~CCoreDX9Device()
{
	if (BackBufferSurface) BackBufferSurface->Release();
	if (Device) Device->Release();
	if (D3D) D3D->Release();
}

void CCoreDX9Device::ResetPrivateResources()
{
	if (BackBufferSurface) BackBufferSurface->Release();
	BackBufferSurface = NULL;

	Device->Reset(&D3DPP);

	Device->GetRenderTarget(0, &BackBufferSurface);
}

#define BACKBUFFERFORMAT D3DFMT_X8R8G8B8

TBOOL CCoreDX9Device::InitAPI(const TU32 hWnd, const TBOOL FullScreen, const TS32 XRes, const TS32 YRes, const TS32 AALevel/* =0 */, const TS32 RefreshRate/* =60 */)
{
	//http://www.gamedev.net/topic/291706-how-to-handle-window-resizing/

	//this->setup = *pSetup;

	//// Set default settings
	//UINT AdapterToUse = D3DADAPTER_DEFAULT;
	//D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

	//HRESULT h;

	//if (setup.bEnableNVPerfhud)
	//{
	//	for (UINT Adapter=0; Adapter < D3D->GetAdapterCount(); Adapter++)
	//	{
	//		D3DADAPTER_IDENTIFIER9 Identifier;
	//		HRESULT Res = D3D->GetAdapterIdentifier( Adapter, 0, &Identifier );
	//		if (strstr(Identifier.Description,"PerfHUD") != NULL)
	//		{
	//			AdapterToUse = Adapter;
	//			DeviceType = D3DDEVTYPE_REF;
	//			break;
	//		}
	//	}
	//}

	//D3DPRESENT_PARAMETERS d3dpp;
	//ZeroMemory(&d3dpp,sizeof(d3dpp));

	//d3dpp.SwapEffect     = D3DSWAPEFFECT_DISCARD;
	//d3dpp.hDeviceWindow  = hWnd;
	//d3dpp.Windowed       = !pSetup->bFullscreen;
	//d3dpp.PresentationInterval =
	//	(pSetup->bVerticalSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);

	//d3dpp.BackBufferCount  = 1;

	//d3dpp.BackBufferWidth  = pSetup->nScreenWidth;
	//d3dpp.BackBufferHeight = pSetup->nScreenHeight;

	//D3DDISPLAYMODE d3ddm;
	//if( FAILED( D3D->GetAdapterDisplayMode( AdapterToUse, &d3ddm ) ) )
	//	return false;

	//D3DFORMAT pBackbufferFormats32[] = {
	//	D3DFMT_X8R8G8B8,
	//	D3DFMT_A8R8G8B8,
	//};
	//for (int i=0; i<sizeof(pBackbufferFormats32)/sizeof(D3DFORMAT); i++)
	//	if ( SUCCEEDED( h = D3D->CheckDeviceType( AdapterToUse, DeviceType, d3ddm.Format, pBackbufferFormats32[i], !pSetup->bFullscreen ) ) )
	//		d3dpp.BackBufferFormat = pBackbufferFormats32[i];

	//if (d3dpp.BackBufferFormat==D3DFMT_UNKNOWN) return false;

	//d3dpp.EnableAutoDepthStencil = TRUE;
	//d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;

	//D3DFORMAT pDepthFormats[] = {
	//	D3DFMT_D16,
	//	D3DFMT_D24X8,
	//	D3DFMT_D24S8,
	//	D3DFMT_D32,
	//};

	//for (int i=0; i<sizeof(pDepthFormats)/sizeof(D3DFORMAT); i++)
	//	if ( SUCCEEDED( h = D3D->CheckDeviceFormat( AdapterToUse, DeviceType, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, pDepthFormats[i] ) ) )
	//		d3dpp.AutoDepthStencilFormat = pDepthFormats[i];

	//if (d3dpp.AutoDepthStencilFormat==D3DFMT_UNKNOWN) return false;

	//d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	//if (setup.nMultiSamplingLevel)
	//{
	//	static D3DMULTISAMPLE_TYPE pMultisampleTypes[]={
	//		D3DMULTISAMPLE_2_SAMPLES,
	//		D3DMULTISAMPLE_4_SAMPLES,
	//		D3DMULTISAMPLE_6_SAMPLES,
	//		D3DMULTISAMPLE_8_SAMPLES,
	//		D3DMULTISAMPLE_NONE,
	//	};

	//	for (int i=0; pMultisampleTypes[i] != D3DMULTISAMPLE_NONE; i++)
	//	{
	//		if (D3D->CheckDeviceMultiSampleType(AdapterToUse,DeviceType,d3dpp.BackBufferFormat      ,d3dpp.Windowed,pMultisampleTypes[i],NULL) == D3D_OK &&
	//			D3D->CheckDeviceMultiSampleType(AdapterToUse,DeviceType,d3dpp.AutoDepthStencilFormat,d3dpp.Windowed,pMultisampleTypes[i],NULL) == D3D_OK)
	//		{
	//			d3dpp.MultiSampleType = pMultisampleTypes[i];
	//		}
	//	}
	//}


	//h = D3D->CreateDevice( AdapterToUse, DeviceType, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &Device);
	//if (h != D3D_OK)
	//{
	//	h = D3D->CreateDevice( AdapterToUse, DeviceType, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &Device);
	//	if(h != D3D_OK)
	//	{
	//		return false;
	//	}
	//}

	//D3DCAPS9 caps;
	//Device->GetDeviceCaps(&caps);

	//Device->SetRenderState(D3DRS_ZENABLE , D3DZB_TRUE);
	//Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//Device->SetRenderState(D3DRS_LIGHTING, FALSE);

	//int nAniso = min( setup.nAnistropicLevel, caps.MaxAnisotropy );
	//for (int x=0; x<8; x++)
	//{
	//	Device->SetSamplerState( x, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	//	Device->SetSamplerState( x, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	//	Device->SetSamplerState( x, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	//	Device->SetSamplerState( x, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//	Device->SetSamplerState( x, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//	Device->SetSamplerState( x, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	//	if ( nAniso ) {
	//		//Device->SetSamplerState( x, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
	//		Device->SetSamplerState( x, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	//		Device->SetSamplerState( x, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	//		Device->SetSamplerState( x, D3DSAMP_MAXANISOTROPY, nAniso );
	//	}
	//}

	////Device->GetRenderTarget(0,&srfBBColor);
	////Device->GetDepthStencilSurface(&srfBBDepth);
	//return true;

	/*	TODO!!!!!!!!!!!!!!!!!!!

	 D3DDISPLAYMODE d3ddm;
	 if( FAILED( D3D->GetAdapterDisplayMode( AdapterToUse, &d3ddm ) ) )
	 return false;

	 D3DFORMAT pBackbufferFormats32[] = {
	 D3DFMT_X8R8G8B8,
	 D3DFMT_A8R8G8B8,
	 };
	 for (int i=0; i<sizeof(pBackbufferFormats32)/sizeof(D3DFORMAT); i++)
	 if (SUCCEEDED(D3D->CheckDeviceType( AdapterToUse, DeviceType, d3ddm.Format, pBackbufferFormats32[i], !pSetup->bFullscreen )))
	 d3dpp.BackBufferFormat = pBackbufferFormats32[i];

	 if (d3dpp.BackBufferFormat==D3DFMT_UNKNOWN) return false;

	 d3dpp.EnableAutoDepthStencil = TRUE;
	 d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;

	 D3DFORMAT pDepthFormats[] = {
	 D3DFMT_D16,
	 D3DFMT_D24X8,
	 D3DFMT_D24S8,
	 D3DFMT_D32,
	 };

	 for (int i=0; i<sizeof(pDepthFormats)/sizeof(D3DFORMAT); i++)
	 if ( (h = D3D->CheckDeviceFormat( AdapterToUse, DeviceType, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, pDepthFormats[i] )) == D3D_OK )
	 d3dpp.AutoDepthStencilFormat = pDepthFormats[i];

	 if (d3dpp.AutoDepthStencilFormat==D3DFMT_UNKNOWN) return false;

	 */

	//////////////////////////////////////////////////////////////////////////
	//create d3d
	D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!D3D) return false;

	memset(&D3DPP, 0, sizeof(D3DPP));

	D3DPP.Windowed = !FullScreen;
	D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DPP.EnableAutoDepthStencil = TRUE;
	D3DPP.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	D3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	D3DPP.hDeviceWindow = (HWND)hWnd;

	//////////////////////////////////////////////////////////////////////////
	//get display mode
	if (!FullScreen)
	{
		D3DDISPLAYMODE d3ddm;
		if (FAILED(D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm))) return false;
		D3DPP.BackBufferFormat = d3ddm.Format;
	}
	else
	{
		D3DDISPLAYMODE d3ddm;
		TBOOL bDesiredAdaptorModeFound = false;

		TS32 nMaxAdaptorModes = D3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, BACKBUFFERFORMAT);

		for (TS32 nMode = 0; nMode < nMaxAdaptorModes; nMode++)
		{
			if (FAILED(D3D->EnumAdapterModes(D3DADAPTER_DEFAULT, BACKBUFFERFORMAT, nMode, &d3ddm))) return false;

			if (d3ddm.Width != XRes || d3ddm.Height != YRes) continue;
			if (d3ddm.Format != BACKBUFFERFORMAT) continue;
			if (d3ddm.RefreshRate != RefreshRate) continue;

			bDesiredAdaptorModeFound = true;
			break;
		}

		//screen mode not found
		if (bDesiredAdaptorModeFound == false) return false;
		if (FAILED(D3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, BACKBUFFERFORMAT, BACKBUFFERFORMAT, FALSE))) return false;

		D3DPP.BackBufferWidth = XRes;
		D3DPP.BackBufferHeight = YRes;
		D3DPP.BackBufferFormat = BACKBUFFERFORMAT;
	}

	//////////////////////////////////////////////////////////////////////////
	//get backbuffer mode
	D3DFORMAT pDepthFormats[] =
	{
		D3DFMT_D16,
		D3DFMT_D24S8,
		//D3DFMT_D32,
		D3DFMT_UNKNOWN,
	};

	for (TS32 i = 0; pDepthFormats[i] != D3DFMT_UNKNOWN; i++)
		if (SUCCEEDED(D3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DPP.BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, pDepthFormats[i])))
			D3DPP.AutoDepthStencilFormat = pDepthFormats[i];

	if (D3DPP.AutoDepthStencilFormat == D3DFMT_UNKNOWN) return false;

	//////////////////////////////////////////////////////////////////////////
	//get multisample level
	D3DMULTISAMPLE_TYPE pMultisampleTypes[] =
	{
		D3DMULTISAMPLE_NONE,
		D3DMULTISAMPLE_2_SAMPLES,
		D3DMULTISAMPLE_4_SAMPLES,
		D3DMULTISAMPLE_6_SAMPLES,
		D3DMULTISAMPLE_8_SAMPLES,
	};

	for (TS32 i = max(min(AALevel, 4), 0); i >= 0; i--)
	{
		if (SUCCEEDED(D3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DPP.BackBufferFormat, D3DPP.Windowed, pMultisampleTypes[i], NULL)) &&
			SUCCEEDED(D3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DPP.AutoDepthStencilFormat, D3DPP.Windowed, pMultisampleTypes[i], NULL)))
			D3DPP.MultiSampleType = pMultisampleTypes[i];
	}

	//////////////////////////////////////////////////////////////////////////
	//check caps
	D3DCAPS9 d3dCaps;
	if (FAILED(D3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps))) return false;

	DWORD dwBehaviorFlags = 0;

	if (d3dCaps.VertexProcessingCaps)
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//////////////////////////////////////////////////////////////////////////
	//create device

	HRESULT res = D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)hWnd, dwBehaviorFlags, &D3DPP, &Device);

	if (FAILED(res)) return false;

	Device->GetRenderTarget(0, &BackBufferSurface);

	LOG(LOG_INFO, _T("[core] DirectX9 Device initialization successful."));

	return true;
}

TBOOL CCoreDX9Device::Initialize(CCoreWindowHandler *window, const TS32 AALevel)
{
	Window = window;

	if (!InitAPI(Window->GetHandle(), Window->GetInitParameters().FullScreen, Window->GetXRes(), Window->GetYRes(), AALevel, 60)) return false;

	ShowWindow((HWND)Window->GetHandle(), Window->GetInitParameters().Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	SetForegroundWindow((HWND)Window->GetHandle());
	SetFocus((HWND)Window->GetHandle());
	return true;
}

TBOOL CCoreDX9Device::IsWindowed()
{
	return D3DPP.Windowed != 0;
}

void CCoreDX9Device::Resize(const TS32 xr, const TS32 yr)
{
	if (IsWindowed())
	{
		D3DPP.BackBufferWidth = 0;
		D3DPP.BackBufferHeight = 0;
	}
	else
	{
		D3DPP.BackBufferWidth = xr;
		D3DPP.BackBufferHeight = yr;
	}
	D3DPP.Windowed = IsWindowed();
	ResetDevice();
}

void CCoreDX9Device::SetFullScreenMode(const TBOOL FullScreen, const TS32 xr, const TS32 yr)
{
	if (!FullScreen)
	{
		D3DPP.BackBufferWidth = 0;
		D3DPP.BackBufferHeight = 0;
	}
	else
	{
		D3DPP.BackBufferWidth = xr;
		D3DPP.BackBufferHeight = yr;
	}
	D3DPP.Windowed = !FullScreen;
	ResetDevice();
}

TBOOL CCoreDX9Device::DeviceOk()
{
	HRESULT h = Device->TestCooperativeLevel();
	if (h == D3D_OK) return true;

	if (h == D3DERR_DEVICENOTRESET)
		ResetDevice();

	return false;
}

//////////////////////////////////////////////////////////////////////////
// texture functions

CCoreTexture2D *CCoreDX9Device::CreateTexture2D(const TS32 XRes, const TS32 YRes, const TU8 *Data, const TS8 BytesPerPixel, const COREFORMAT Format/* =COREFMT_A8R8G8B8 */, const TBOOL RenderTarget/* =false */)
{
	CCoreTexture2D *Result = new CCoreDX9Texture2D(this);
	if (!Result->Create(XRes, YRes, Data, BytesPerPixel, Format, RenderTarget))
		SAFEDELETE(Result);
	return Result;
}

CCoreTexture2D *CCoreDX9Device::CreateTexture2D(const TU8 *Data, const TS32 Size)
{
	CCoreTexture2D *Result = new CCoreDX9Texture2D(this);
	if (!Result->Create(Data, Size))
		SAFEDELETE(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////////
// vertexbuffer functions

CCoreVertexBuffer *CCoreDX9Device::CreateVertexBuffer(const TU8 *Data, const TS32 Size)
{
	CCoreDX9VertexBuffer *Result = new CCoreDX9VertexBuffer(this);
	if (!Result->Create(Data, Size))
		SAFEDELETE(Result);
	return Result;
}

CCoreVertexBuffer *CCoreDX9Device::CreateVertexBufferDynamic(const TS32 Size)
{
	CCoreDX9VertexBuffer *Result = new CCoreDX9VertexBuffer(this);
	if (!Result->CreateDynamic(Size))
		SAFEDELETE(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////////
// indexbuffer functions

CCoreIndexBuffer *CCoreDX9Device::CreateIndexBuffer(const TS32 IndexCount, const TS32 TriCount, const TS32 IndexSize)
{
	CCoreDX9IndexBuffer *Result = new CCoreDX9IndexBuffer(this);
	if (!Result->Create(IndexCount, TriCount, IndexSize))
		SAFEDELETE(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////////
// vertexformat functions

CCoreVertexFormat *CCoreDX9Device::CreateVertexFormat(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs)
{
	CCoreDX9VertexFormat *Result = new CCoreDX9VertexFormat(this);
	if (!Result->Create(Attributes))
		SAFEDELETE(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////////
// shader functions

CCoreVertexShader *CCoreDX9Device::CreateVertexShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion)
{
#ifndef CORE_API_D3DX
	LOG(LOG_ERROR, _T("[core] DX9 CoRE linked without D3DX, vertexshader compilation failed."));
	return NULL;
#else
	if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) return NULL;

	LPD3DXBUFFER VS;
	LPD3DXBUFFER Err;

	if (D3DXCompileShader(Code, CodeSize, NULL, NULL, EntryFunction, ShaderVersion, 0, &VS, &Err, 0) != S_OK)
	{
		LOG(LOG_ERROR, _T("[core] VertexShader compilation error: %s"), CString((char*)Err->GetBufferPointer()).GetPointer());
		return NULL;
	}

	CCoreDX9VertexShader *s = new CCoreDX9VertexShader(this);

	if (!s->Create(VS->GetBufferPointer(), VS->GetBufferSize()))
	{
		SAFEDELETE(s);
		VS->Release();
		return NULL;
	}

	VS->Release();
	return s;
#endif
}

unsigned char *RemoveShaderComment(unsigned char *shader, int shadersize, int &newshadersize)
{
	newshadersize = 0;
	if (!shader) return NULL;
	int commentsize = ((unsigned short*)(shader + 6))[0] * 4 + 4;
	unsigned char *smallshader = new unsigned char[shadersize - commentsize];
	memcpy(smallshader, shader, 4);
	newshadersize = shadersize - commentsize;
	memcpy(smallshader + 4, shader + commentsize + 4, shadersize - commentsize - 4);
	return smallshader;
}

CCorePixelShader *CCoreDX9Device::CreatePixelShader(LPCSTR Code, TS32 CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion)
{
#ifndef CORE_API_D3DX
	LOG(LOG_ERROR, _T("[core] DX9 CoRE linked without D3DX, pixelshader compilation failed."));
	return NULL;
#else
	if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) return NULL;

	LPD3DXBUFFER VS;
	LPD3DXBUFFER Err;

	if (D3DXCompileShader(Code, CodeSize, NULL, NULL, EntryFunction, ShaderVersion, 0, &VS, &Err, 0) != S_OK)
	{
		LOG(LOG_ERROR, _T("[core] PixelShader compilation error: %s"), CString((char*)Err->GetBufferPointer()).GetPointer());
		return NULL;
	}

	CCoreDX9PixelShader *s = new CCoreDX9PixelShader(this);

	if (!s->Create(VS->GetBufferPointer(), VS->GetBufferSize()))
	{
		SAFEDELETE(s);
		VS->Release();
		return NULL;
	}

	VS->Release();

	return s;
#endif
}


//////////////////////////////////////////////////////////////////////////
// renderstate

TBOOL CCoreDX9Device::ApplyRenderState(const CORESAMPLER Sampler, const CORERENDERSTATE RenderState, const CORERENDERSTATEVALUE Value)
{
	switch (RenderState)
	{
		case CORERS_CULLMODE: return Device->SetRenderState(D3DRS_CULLMODE, DX9CullModes[Value.CullMode]) == D3D_OK;
		case CORERS_FILLMODE: return Device->SetRenderState(D3DRS_FILLMODE, DX9FillModes[Value.FillMode]) == D3D_OK;
		case CORERS_ZENABLE: return Device->SetRenderState(D3DRS_ZENABLE, Value.Int) == D3D_OK;
		case CORERS_ZWRITEENABLE: return Device->SetRenderState(D3DRS_ZWRITEENABLE, Value.Int) == D3D_OK;
		case CORERS_ZFUNCT: return Device->SetRenderState(D3DRS_ZFUNC, DX9ComparisonFunctions[Value.ComparisonFunct]) == D3D_OK;
		case CORERS_ALPHATESTENABLE: return Device->SetRenderState(D3DRS_ALPHATESTENABLE, Value.Int) == D3D_OK;
		case CORERS_ALPHAREFERENCE: return Device->SetRenderState(D3DRS_ALPHAREF, Value.Int) == D3D_OK;
		case CORERS_ALPHAFUNCT: return Device->SetRenderState(D3DRS_ALPHAFUNC, DX9ComparisonFunctions[Value.ComparisonFunct]) == D3D_OK;
		case CORERS_BLENDENABLE: return Device->SetRenderState(D3DRS_ALPHABLENDENABLE, Value.Int) == D3D_OK;
		case CORERS_SRCBLEND: return Device->SetRenderState(D3DRS_SRCBLEND, DX9BlendFactors[Value.BlendFactor]) == D3D_OK;
		case CORERS_DSTBLEND: return Device->SetRenderState(D3DRS_DESTBLEND, DX9BlendFactors[Value.BlendFactor]) == D3D_OK;
		case CORERS_BLENDOP: return Device->SetRenderState(D3DRS_BLENDOP, DX9BlendOps[Value.BlendOp]) == D3D_OK;
		case CORERS_STENCILENABLE: return Device->SetRenderState(D3DRS_STENCILENABLE, Value.Int) == D3D_OK;
		case CORERS_TWOSIDEDSTENCILENABLE: return Device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, Value.Int) == D3D_OK;
		case CORERS_SCISSORENABLE: return Device->SetRenderState(D3DRS_SCISSORTESTENABLE, Value.Int) == D3D_OK;
		case CORERS_AALINESENABLE: return Device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, Value.Int) == D3D_OK;
		case CORERS_VERTEXSHADER:
		{
			if (!Value.VertexShader) return Device->SetVertexShader(NULL) == D3D_OK;
			return ApplyVertexShader(Value.VertexShader);

		}
		case CORERS_PIXELSHADER:
		{
			if (!Value.PixelShader) return Device->SetPixelShader(NULL) == D3D_OK;
			return ApplyPixelShader(Value.PixelShader);
		}
		case CORERS_TEXTURE:
		{
			if (!Value.Texture) return Device->SetTexture(DX9Samplers[Sampler], 0) == D3D_OK;
			return ApplyTextureToSampler(Sampler, Value.Texture);
		}
		case CORERS_MINFILTER: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_MINFILTER, DX9TextureFilters[Value.TextureFilter]) == D3D_OK;
		case CORERS_MAGFILTER: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_MAGFILTER, DX9TextureFilters[Value.TextureFilter]) == D3D_OK;
		case CORERS_MIPFILTER: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_MIPFILTER, DX9TextureFilters[Value.TextureFilter]) == D3D_OK;
		case CORERS_ADDRESSU: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_ADDRESSU, DX9TextureAddressModes[Value.TextureAddressMode]) == D3D_OK;
		case CORERS_ADDRESSV: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_ADDRESSV, DX9TextureAddressModes[Value.TextureAddressMode]) == D3D_OK;
		case CORERS_ADDRESSW: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_ADDRESSW, DX9TextureAddressModes[Value.TextureAddressMode]) == D3D_OK;
		case CORERS_MAXANISOTROPY: return Device->SetSamplerState(DX9Samplers[Sampler], D3DSAMP_MAXANISOTROPY, Value.Int) == D3D_OK;
		case CORERS_WRAPMODE: return Device->SetRenderState(DX9WrapModes[Sampler], DX9TextureWrapModes[Value.TextureWrapMode]) == D3D_OK;
		case CORERS_VERTEXFORMAT:
		{
			if (!Value.VertexFormat)
			{
				CurrentVertexFormatSize = 0;
				return Device->SetVertexDeclaration(NULL) == D3D_OK;
			}

			CurrentVertexFormatSize = Value.VertexFormat->GetSize();
			return ApplyVertexFormat(Value.VertexFormat);
		}
		case CORERS_INDEXBUFFER:
		{
			if (!Value.IndexBuffer) return Device->SetIndices(NULL) == D3D_OK;
			return ApplyIndexBuffer(Value.IndexBuffer);
		}
		default: return false;
	}
}

TBOOL CCoreDX9Device::SetNoVertexBuffer()
{
	return (Device->SetStreamSource(0, NULL, 0, 0) == D3D_OK);
}

TBOOL CCoreDX9Device::CommitRenderStates()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// display functions

TBOOL CCoreDX9Device::BeginScene()
{
	return Device->BeginScene() == D3D_OK;
}

TBOOL CCoreDX9Device::EndScene()
{
	return Device->EndScene() == D3D_OK;
}

TBOOL CCoreDX9Device::Clear(const TBOOL clearPixels, const TBOOL clearDepth, const CColor &Color, const TF32 Depth, const TS32 Stencil)
{
	TS32 Flags = 0;
	if (clearPixels) Flags |= D3DCLEAR_TARGET;
	if (clearDepth) Flags |= D3DCLEAR_ZBUFFER;
	return Device->Clear(0, NULL, Flags, Color, Depth, Stencil) == D3D_OK;
}

TBOOL CCoreDX9Device::Flip()
{
	RECT r;
	r.top = r.left = 0;
	r.right = D3DPP.BackBufferWidth;
	r.bottom = D3DPP.BackBufferHeight;

	if (D3DPP.Windowed)
		return Device->Present(&r, NULL, NULL, NULL) == D3D_OK;
	else
		return Device->Present(NULL, NULL, NULL, NULL) == D3D_OK;
}

TBOOL CCoreDX9Device::DrawIndexedTriangles(TS32 Count, TS32 NumVertices)
{
	return ApplyRequestedRenderState() && Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, NumVertices, 0, Count) == D3D_OK;
}

TBOOL CCoreDX9Device::DrawTriangles(TS32 Count)
{
	return ApplyRequestedRenderState() && Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, Count) == D3D_OK;
}

#else
NoEmptyFile();
#endif
