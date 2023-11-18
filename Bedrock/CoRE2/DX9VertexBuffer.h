#pragma once
#include "DX9Device.h"
#include "VertexBuffer.h"

#ifdef CORE_API_DX9

class CCoreDX9VertexBuffer : public CCoreVertexBuffer
{
	LPDIRECT3DDEVICE9 Dev;
	TS32 Size;
	TBOOL Dynamic;

	LPDIRECT3DVERTEXBUFFER9 VertexBufferHandle;

	virtual void Release();
	virtual TBOOL Apply(const TU32 Offset);

public:

	CCoreDX9VertexBuffer(CCoreDX9Device *Device);
	virtual ~CCoreDX9VertexBuffer();

	virtual void OnDeviceLost();
	virtual void OnDeviceReset();

	virtual TBOOL Create(const TU8 *Data, const TU32 Size);
	virtual TBOOL CreateDynamic(const TU32 Size);
	virtual TBOOL Update(const TS32 Offset, const TU8 *Data, const TU32 Size);
	virtual TBOOL Lock(void **Result);
	virtual TBOOL Lock(void **Result, const TU32 Offset, const TS32 size, const TS32 Flags = 0);
	virtual TBOOL UnLock();
};

#endif