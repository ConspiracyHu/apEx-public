#pragma once
#include "Core2_Config.h"
#include "Enums.h"

#ifdef CORE_API_DX9

extern D3DFORMAT DX9Formats[];
extern DWORD DX9CullModes[];
extern DWORD DX9FillModes[];
extern DWORD DX9ComparisonFunctions[];
extern DWORD DX9BlendFactors[];
extern DWORD DX9BlendOps[];
extern DWORD DX9TextureFilters[];
extern DWORD DX9TextureAddressModes[];
extern DWORD DX9TextureWrapModes[];
extern DWORD DX9Samplers[];
extern D3DRENDERSTATETYPE DX9WrapModes[];

COREFORMAT GetFormat(D3DFORMAT Format);

#endif