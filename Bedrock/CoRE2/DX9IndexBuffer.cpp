#include "BasePCH.h"
#include "DX9IndexBuffer.h"

#ifdef CORE_API_DX9

CCoreDX9IndexBuffer::CCoreDX9IndexBuffer(CCoreDX9Device *dev) : CCoreIndexBuffer(dev)
{
	Dev = dev->GetDevice();
	IndexBufferHandle = NULL;
	IndexCount = 0;
	IndexSize = 0;
}

CCoreDX9IndexBuffer::~CCoreDX9IndexBuffer()
{
	Release();
}

void CCoreDX9IndexBuffer::Release()
{
	if (IndexBufferHandle) IndexBufferHandle->Release();
	IndexBufferHandle = NULL;
}

TBOOL CCoreDX9IndexBuffer::Apply()
{
	if (!IndexBufferHandle) return false;
	return (Dev->SetIndices(IndexBufferHandle) == D3D_OK);
}

void CCoreDX9IndexBuffer::OnDeviceLost()
{
}

void CCoreDX9IndexBuffer::OnDeviceReset()
{
}

TBOOL CCoreDX9IndexBuffer::Create(const TU32 idxcount, const TU32 tc, const TU32 idxsize)
{
	if (idxcount <= 0 || idxsize <= 0) return false;
	if (idxsize != 2 && idxsize != 4) return false;

	Release();

	DWORD Usage = D3DUSAGE_WRITEONLY;
	D3DPOOL Pool = D3DPOOL_MANAGED;
	D3DFORMAT Format = D3DFMT_INDEX16;
	if (idxsize == 4) Format = D3DFMT_INDEX32;

	if (Dev->CreateIndexBuffer(idxcount*idxsize, Usage, Format, Pool, &IndexBufferHandle, NULL) != D3D_OK)
		return false;

	IndexCount = idxcount;
	IndexSize = idxsize;
	//TriCount = tc;

	return true;
}

TBOOL CCoreDX9IndexBuffer::Lock(void **Result, const TU32 idxoffset, const TS32 idxcount)
{
	if (!IndexBufferHandle || idxcount <= 0) return false;

	void *Data = NULL;

	if (IndexBufferHandle->Lock(idxoffset*IndexSize, idxcount*IndexSize, &Data, 0) != D3D_OK)
		return false;

	*Result = Data;
	return true;
}

TBOOL CCoreDX9IndexBuffer::Lock(void **Result)
{
	return Lock(Result, 0, IndexCount);
}

TBOOL CCoreDX9IndexBuffer::UnLock()
{
	if (!IndexBufferHandle) return false;
	return IndexBufferHandle->Unlock() == D3D_OK;
}

#else
NoEmptyFile();
#endif