#pragma once
#include "DX9Device.h"
#include "IndexBuffer.h"

#ifdef CORE_API_DX9

class CCoreDX9IndexBuffer : public CCoreIndexBuffer
{
	LPDIRECT3DDEVICE9 Dev;
	LPDIRECT3DINDEXBUFFER9 IndexBufferHandle;

	TS32 IndexCount;
	TS32 IndexSize;

	virtual void Release();
	virtual TBOOL Apply();

public:

	CCoreDX9IndexBuffer(CCoreDX9Device *dev);
	virtual ~CCoreDX9IndexBuffer();

	virtual void OnDeviceLost();
	virtual void OnDeviceReset();

	virtual TBOOL Create(const TU32 IndexCount, const TU32 TriCount, const TU32 IndexSize=2);
	virtual TBOOL Lock(void **Result, const TU32 IndexOffset, const TS32 IndexCount);
	virtual TBOOL Lock(void **Result);
	virtual TBOOL UnLock();
};

#endif