#include "BasePCH.h"
#include "DX9Shader.h"
#ifdef CORE_API_DX9

//////////////////////////////////////////////////////////////////////////
// vertex shader

CCoreDX9VertexShader::CCoreDX9VertexShader(CCoreDX9Device *dev) : CCoreVertexShader(dev)
{
	Dev = dev->GetDevice();
	VertexShaderHandle = NULL;
}

CCoreDX9VertexShader::~CCoreDX9VertexShader()
{
	Release();
}

TBOOL CCoreDX9VertexShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);

	HRESULT res = Dev->CreateVertexShader((DWORD*)Binary, &VertexShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] VertexShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX9VertexShader::Release()
{
	if (VertexShaderHandle)
		VertexShaderHandle->Release();
	VertexShaderHandle = NULL;
}

TBOOL CCoreDX9VertexShader::Apply()
{
	HRESULT res = Dev->SetVertexShader(VertexShaderHandle) == S_OK;
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] Setting VertexShader failed (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

//////////////////////////////////////////////////////////////////////////
// Pixel shader

CCoreDX9PixelShader::CCoreDX9PixelShader(CCoreDX9Device *dev) : CCorePixelShader(dev)
{
	Dev = dev->GetDevice();
	PixelShaderHandle = NULL;
}

CCoreDX9PixelShader::~CCoreDX9PixelShader()
{
	Release();
}

TBOOL CCoreDX9PixelShader::Create(void *Binary, TS32 Length)
{
	if (!Binary || Length <= 0) return false;
	FetchBinary(Binary, Length);

	HRESULT res = Dev->CreatePixelShader((DWORD*)Binary, &PixelShaderHandle);
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] PixelShader Creation error (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

void CCoreDX9PixelShader::Release()
{
	if (PixelShaderHandle)
		PixelShaderHandle->Release();
	PixelShaderHandle = NULL;
}

TBOOL CCoreDX9PixelShader::Apply()
{
	HRESULT res = Dev->SetPixelShader(PixelShaderHandle) == S_OK;
	if (res != S_OK)
	{
		_com_error err(res);
		LOG(LOG_ERROR, _T("[core] Setting PixelShader failed (%s)"), err.ErrorMessage());
	}
	return res == S_OK;
}

#else
NoEmptyFile();
#endif