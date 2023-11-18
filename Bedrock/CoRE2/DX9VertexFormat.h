#pragma once
#include "DX9Device.h"
#include "VertexFormat.h"

#ifdef CORE_API_DX9

class CCoreDX9VertexFormat : public CCoreVertexFormat
{
	LPDIRECT3DDEVICE9 Dev;
	LPDIRECT3DVERTEXDECLARATION9 VertexFormatHandle;

	TS32 Size;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX9VertexFormat(CCoreDX9Device *dev);
	virtual ~CCoreDX9VertexFormat();

	virtual void OnDeviceLost();
	virtual void OnDeviceReset();

	virtual TBOOL Create(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs=NULL);
	virtual TS32 GetSize();
};

#endif