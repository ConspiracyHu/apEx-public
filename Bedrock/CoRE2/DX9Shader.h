#pragma once
#include "DX9Device.h"
#include "Shader.h"

#ifdef CORE_API_DX9

class CCoreDX9VertexShader : public CCoreVertexShader
{
	LPDIRECT3DDEVICE9 Dev;
	LPDIRECT3DVERTEXSHADER9 VertexShaderHandle;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX9VertexShader(CCoreDX9Device *Device);
	virtual ~CCoreDX9VertexShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
};

class CCoreDX9PixelShader : public CCorePixelShader
{
	LPDIRECT3DDEVICE9 Dev;
	LPDIRECT3DPIXELSHADER9 PixelShaderHandle;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX9PixelShader(CCoreDX9Device *Device);
	virtual ~CCoreDX9PixelShader();

	virtual TBOOL Create(void *Binary, TS32 Length);
};

#endif