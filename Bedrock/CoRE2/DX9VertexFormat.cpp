#include "BasePCH.h"
#include "DX9VertexFormat.h"

#ifdef CORE_API_DX9

CCoreDX9VertexFormat::CCoreDX9VertexFormat(CCoreDX9Device *dev) : CCoreVertexFormat(dev)
{
	Dev = dev->GetDevice();
	VertexFormatHandle = NULL;
	Size = 0;
}

CCoreDX9VertexFormat::~CCoreDX9VertexFormat()
{
	Release();
}

void CCoreDX9VertexFormat::Release()
{
	if (VertexFormatHandle) VertexFormatHandle->Release();
	VertexFormatHandle = NULL;
}

TBOOL CCoreDX9VertexFormat::Apply()
{
	if (!VertexFormatHandle) return false;
	return (Dev->SetVertexDeclaration(VertexFormatHandle) == D3D_OK);
}

void CCoreDX9VertexFormat::OnDeviceLost()
{
}

void CCoreDX9VertexFormat::OnDeviceReset()
{
}

TBOOL CCoreDX9VertexFormat::Create(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs)
{
	if (!Attributes.NumItems()) return false;
	Release();

	TS32 PosUsages = 0;
	TS32 NormUsages = 0;
	TS32 UVUsages = 0;
	TS32 ColUsages = 0;

	TS32 offset = 0;

	D3DVERTEXELEMENT9 *vxdecl = new D3DVERTEXELEMENT9[Attributes.NumItems() + 1];

	for (TS32 x = 0; x < Attributes.NumItems(); x++)
	{
		vxdecl[x].Stream = 0;
		vxdecl[x].Offset = offset;
		vxdecl[x].Method = D3DDECLMETHOD_DEFAULT;

		switch (Attributes[x])
		{
			case COREVXATTR_POSITION3:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_POSITION;
				vxdecl[x].Type = D3DDECLTYPE_FLOAT3;
				vxdecl[x].UsageIndex = PosUsages++;
				offset += 12;
			}
				break;
			case COREVXATTR_POSITION4:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_POSITION;
				vxdecl[x].Type = D3DDECLTYPE_FLOAT4;
				vxdecl[x].UsageIndex = PosUsages++;
				offset += 16;
			}
				break;
			case COREVXATTR_NORMAL3:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_NORMAL;
				vxdecl[x].Type = D3DDECLTYPE_FLOAT3;
				vxdecl[x].UsageIndex = NormUsages++;
				offset += 12;
			}
				break;
			case COREVXATTR_TEXCOORD2:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_TEXCOORD;
				vxdecl[x].Type = D3DDECLTYPE_FLOAT2;
				vxdecl[x].UsageIndex = UVUsages++;
				offset += 8;
			}
				break;
			case COREVXATTR_COLOR4:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_COLOR;
				vxdecl[x].Type = D3DDECLTYPE_D3DCOLOR;
				vxdecl[x].UsageIndex = ColUsages++;
				offset += 4;
			}
				break;
			case COREVXATTR_COLOR16:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_COLOR;
				vxdecl[x].Type = D3DDECLTYPE_FLOAT4;
				vxdecl[x].UsageIndex = ColUsages++;
				offset += 16;
			}
				break;
			case COREVXATTR_POSITIONT4:
			{
				vxdecl[x].Usage = D3DDECLUSAGE_POSITIONT;
				vxdecl[x].Type = D3DDECLTYPE_FLOAT4;
				vxdecl[x].UsageIndex = PosUsages++;
				offset += 16;
			}
				break;

			default:
			{
				//unhandled format
				SAFEDELETEA(vxdecl);
				return false;
			}
		}
	}

	vxdecl[Attributes.NumItems()].Stream = 0xff;
	vxdecl[Attributes.NumItems()].Type = D3DDECLTYPE_UNUSED;
	vxdecl[Attributes.NumItems()].Offset = 0;
	vxdecl[Attributes.NumItems()].Usage = 0;
	vxdecl[Attributes.NumItems()].Method = 0;
	vxdecl[Attributes.NumItems()].UsageIndex = 0;

	if (Dev->CreateVertexDeclaration(vxdecl, &VertexFormatHandle) != D3D_OK)
	{
		SAFEDELETEA(vxdecl);
		return false;
	}

	Size = offset;
	SAFEDELETEA(vxdecl);
	return true;
}

TS32 CCoreDX9VertexFormat::GetSize()
{
	return Size;
}

#else
NoEmptyFile();
#endif