#include "BasePCH.h"
#include "DX9VertexBuffer.h"

#ifdef CORE_API_DX9

CCoreDX9VertexBuffer::CCoreDX9VertexBuffer(CCoreDX9Device *dev) : CCoreVertexBuffer(dev)
{
	Dev = dev->GetDevice();
	VertexBufferHandle = NULL;
	Size = 0;
	Dynamic = false;
}

CCoreDX9VertexBuffer::~CCoreDX9VertexBuffer()
{
	Release();
}

void CCoreDX9VertexBuffer::Release()
{
	if (VertexBufferHandle) VertexBufferHandle->Release();
	VertexBufferHandle = NULL;
}

TBOOL CCoreDX9VertexBuffer::Apply(const TU32 Offset)
{
	if (!VertexBufferHandle) return false;
	return (Dev->SetStreamSource(0, VertexBufferHandle, Offset, Device->GetVertexFormatSize()) == D3D_OK);
}

void CCoreDX9VertexBuffer::OnDeviceLost()
{
	if (Dynamic)
		Release();
}

void CCoreDX9VertexBuffer::OnDeviceReset()
{
	if (Dynamic)
		BASEASSERT(CreateDynamic(Size));
}

TBOOL CCoreDX9VertexBuffer::CreateDynamic(const TU32 size)
{
	if (size <= 0) return false;
	Release();

	if (Dev->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &VertexBufferHandle, NULL) != D3D_OK)
		return false;

	Size = size;
	Dynamic = true;

	return true;
}

TBOOL CCoreDX9VertexBuffer::Create(const TU8 *Data, const TU32 size)
{
	if (!Data) return false;
	if (size <= 0) return false;
	Release();

	if (Dev->CreateVertexBuffer(Size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &VertexBufferHandle, NULL) != D3D_OK)
		return false;

	Size = size;

	void *Buffer = NULL;

	if (!Lock(&Buffer))
		return false;

	memcpy(Buffer, Data, Size);

	if (!UnLock())
		return false;

	return true;
}

TBOOL CCoreDX9VertexBuffer::Update(const TS32 Offset, const TU8 *Data, const TU32 Size)
{
	void *dta;
	if (!Lock(&dta, Offset, Size, CORELOCK_NOOVERWRITE)) return false;
	memcpy(dta, Data, Size);
	if (!UnLock()) return false;
	return true;
}

TBOOL CCoreDX9VertexBuffer::Lock(void **Result, const TU32 Offset, const TS32 size, const TS32 Flags)
{
	if (!VertexBufferHandle || size <= 0) return false;

	void *Data = NULL;

	if (VertexBufferHandle->Lock(Offset, size, &Data, Flags) != D3D_OK)
		return false;

	*Result = Data;
	return true;
}

TBOOL CCoreDX9VertexBuffer::Lock(void **Result)
{
	return Lock(Result, 0, Size);
}

TBOOL CCoreDX9VertexBuffer::UnLock()
{
	if (!VertexBufferHandle) return false;
	return VertexBufferHandle->Unlock() == D3D_OK;
}

#else
NoEmptyFile();
#endif