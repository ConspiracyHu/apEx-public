#include "BasePCH.h"
#include "Core2_Config.h"
#include "Enums.h"
#include "../BaseLib/BaseLib.h"

#ifdef CORE_API_DX9

D3DFORMAT DX9Formats[] =
{
	D3DFMT_UNKNOWN,
	D3DFMT_A8R8G8B8,
	D3DFMT_A32B32G32R32F,
	D3DFMT_R32F,
	D3DFMT_G16R16F,
};

DWORD DX9CullModes[] =
{
	D3DCULL_NONE,
	D3DCULL_CW,
	D3DCULL_CCW,
};

DWORD DX9FillModes[] =
{
	D3DFILL_SOLID,
	D3DFILL_WIREFRAME,
	D3DFILL_POINT,
};

DWORD DX9ComparisonFunctions[] =
{
	D3DCMP_NEVER,
	D3DCMP_LESS,
	D3DCMP_EQUAL,
	D3DCMP_LESSEQUAL,
	D3DCMP_GREATER,
	D3DCMP_NOTEQUAL,
	D3DCMP_GREATEREQUAL,
	D3DCMP_ALWAYS,
};

DWORD DX9BlendFactors[] =
{
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHASAT,
	D3DBLEND_BLENDFACTOR,
	D3DBLEND_INVBLENDFACTOR,
};

DWORD DX9BlendOps[] =
{
	D3DBLENDOP_ADD,
	D3DBLENDOP_SUBTRACT,
	D3DBLENDOP_REVSUBTRACT,
	D3DBLENDOP_MIN,
	D3DBLENDOP_MAX,
};

DWORD DX9TextureFilters[] =
{
	D3DTEXF_NONE,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_ANISOTROPIC,
};

DWORD DX9TextureAddressModes[] =
{
	D3DTADDRESS_WRAP,
	D3DTADDRESS_MIRROR,
	D3DTADDRESS_CLAMP,
	D3DTADDRESS_BORDER,
	D3DTADDRESS_MIRRORONCE,
};

DWORD DX9TextureWrapModes[] =
{
	0,
	D3DWRAP_U,
	D3DWRAP_V,
	D3DWRAP_V | D3DWRAP_U,
	D3DWRAP_W,
	D3DWRAP_W | D3DWRAP_U,
	D3DWRAP_W | D3DWRAP_V,
	D3DWRAP_W | D3DWRAP_V | D3DWRAP_U,
};

DWORD DX9Samplers[] =
{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	D3DVERTEXTEXTURESAMPLER0,
	D3DVERTEXTEXTURESAMPLER1,
	D3DVERTEXTEXTURESAMPLER2,
	D3DVERTEXTEXTURESAMPLER3,
};

D3DRENDERSTATETYPE DX9WrapModes[] =
{
	D3DRS_WRAP0,
	D3DRS_WRAP1,
	D3DRS_WRAP2,
	D3DRS_WRAP3,
	D3DRS_WRAP4,
	D3DRS_WRAP5,
	D3DRS_WRAP6,
	D3DRS_WRAP7,
	D3DRS_WRAP8,
	D3DRS_WRAP9,
	D3DRS_WRAP10,
	D3DRS_WRAP11,
	D3DRS_WRAP12,
	D3DRS_WRAP13,
	D3DRS_WRAP14,
	D3DRS_WRAP15,
	(D3DRENDERSTATETYPE)0,
	(D3DRENDERSTATETYPE)0,
	(D3DRENDERSTATETYPE)0,
	(D3DRENDERSTATETYPE)0,
};

COREFORMAT GetFormat(D3DFORMAT Format)
{
	switch (Format)
	{
		case D3DFMT_A8R8G8B8: return COREFMT_A8R8G8B8;
		case D3DFMT_A32B32G32R32F: return COREFMT_A32B32G32R32F;
		case D3DFMT_R32F: return COREFMT_R32F;
		case D3DFMT_G16R16F: return COREFMT_G16R16F;
		default: return COREFMT_UNKNOWN;
	}
}

#else
NoEmptyFile();
#endif