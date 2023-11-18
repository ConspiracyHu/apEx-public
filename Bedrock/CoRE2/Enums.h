#pragma once
#include "../BaseLib/BaseLib.h"

enum COREDEVICEAPI
{
	COREAPI_DX9,
	COREAPI_DX11,
	COREAPI_OPENGL,
	COREAPI_CORE1WRAPPER,
};

enum COREFORMAT
{
	COREFMT_UNKNOWN = 0,
	COREFMT_A8R8G8B8 = 1,
	COREFMT_A32B32G32R32F = 2,
	COREFMT_R32F = 3,
	COREFMT_G16R16F = 4,
	COREFMT_R16G16B16A16_FLOAT = 5,
};

enum COREVERTEXATTRIBUTE
{
	COREVXATTR_STOP = -1,
	COREVXATTR_POSITION3 = 0,
	COREVXATTR_POSITION4 = 1,
	COREVXATTR_NORMAL3 = 2,
	COREVXATTR_TEXCOORD2 = 3,
  COREVXATTR_COLOR4 = 4,
	COREVXATTR_COLOR16 = 5,
	COREVXATTR_POSITIONT4 = 6,
  COREVXATTR_TEXCOORD4 = 7,
};

enum CORECOMPARISONFUNCTION
{
	CORECMP_NEVER = 0,
	CORECMP_LESS = 1,
	CORECMP_EQUAL = 2,
	CORECMP_LEQUAL = 3,
	CORECMP_GREATER = 4,
	CORECMP_NOTEQUAL = 5,
	CORECMP_GEQUAL = 6,
	CORECMP_ALWAYS = 7,
};
extern EnumNamePair ComparisonFunctionNames[];


enum COREBLENDFACTOR
{
	COREBLEND_ZERO = 0,
	COREBLEND_ONE = 1,
	COREBLEND_SRCCOL = 2,
	COREBLEND_INVSRCCOL = 3,
	COREBLEND_SRCALPHA = 4,
	COREBLEND_INVSRCALPHA = 5,
	COREBLEND_DSTALPHA = 6,
	COREBLEND_INVDSTALPHA = 7,
	COREBLEND_DSTCOLOR = 8,
	COREBLEND_INVDSTCOLOR = 9,
	COREBLEND_SRCALPHASATURATE = 10,
	COREBLEND_BLENDFACTOR = 11,
	COREBLEND_INVBLENDFACTOR = 12,
};
extern EnumNamePair BlendFactorNames[];

enum COREBLENDOP
{
	COREBLENDOP_ADD = 0,
	COREBLENDOP_SUB = 1,
	COREBLENDOP_REVSUB = 2,
	COREBLENDOP_MIN = 3,
	COREBLENDOP_MAX = 4,
};
extern EnumNamePair BlendOpNames[];

enum CORECULLMODE
{
	CORECULL_NONE = 0,
	CORECULL_CW = 1,
	CORECULL_CCW = 2,
};
extern EnumNamePair CullModeNames[];


enum COREFILLMODE
{
	COREFILL_SOLID = 0,
	COREFILL_EDGES = 1,
	COREFILL_POINTS = 2,
};
extern EnumNamePair FillModeNames[];

enum CORETEXTUREADDRESSMODE
{
	CORETEXADDRESS_WRAP = 0,
	CORETEXADDRESS_MIRROR = 1,
	CORETEXADDRESS_CLAMP = 2,
	CORETEXADDRESS_BORDER = 3,
	CORETEXADDRESS_MIRRORONCE = 4,
};
extern EnumNamePair AddressModeNames[];

enum CORETEXTUREWRAPMODE
{
	CORETEXWRAP_NONE = 0,
	CORETEXWRAP_U = 1,
	CORETEXWRAP_V = 2,
	CORETEXWRAP_UV = 3,
	CORETEXWRAP_W = 4,
	CORETEXWRAP_UW = 5,
	CORETEXWRAP_VW = 6,
	CORETEXWRAP_UVW = 7,
};

enum CORERENDERSTATE
{
	CORERS_VERTEXSHADER = 0,
	CORERS_PIXELSHADER,
	CORERS_GEOMETRYSHADER,
	CORERS_HULLSHADER,
	CORERS_DOMAINSHADER,
	CORERS_TEXTURE,
	CORERS_VERTEXFORMAT,
	CORERS_INDEXBUFFER,
	CORERS_SAMPLERSTATE,
	CORERS_BLENDSTATE,
	CORERS_RASTERIZERSTATE,
	CORERS_DEPTHSTENCILSTATE,
	CORERS_COMPUTESHADER,
};

#define CORE_VS_SAMPLERCOUNT 4
#define CORE_GS_SAMPLERCOUNT 4
#define CORE_PS_SAMPLERCOUNT 16

enum CORESAMPLER
{
	CORESMP_PS0 = 0,
	CORESMP_PS1 = 1,
	CORESMP_PS2 = 2,
	CORESMP_PS3 = 3,
	CORESMP_PS4 = 4,
	CORESMP_PS5 = 5,
	CORESMP_PS6 = 6,
	CORESMP_PS7 = 7,
	CORESMP_PS8 = 8,
	CORESMP_PS9 = 9,
	CORESMP_PS10 = 10,
	CORESMP_PS11 = 11,
	CORESMP_PS12 = 12,
	CORESMP_PS13 = 13,
	CORESMP_PS14 = 14,
	CORESMP_PS15 = 15,
	CORESMP_VS0 = 100,
	CORESMP_VS1 = 101,
	CORESMP_VS2 = 102,
	CORESMP_VS3 = 103,
	CORESMP_GS0 = 200,
	CORESMP_GS1 = 201,
	CORESMP_GS2 = 202,
	CORESMP_GS3 = 203,
};
extern EnumNamePair SamplerNames[];

enum COREFILTER
{
	COREFILTER_MIN_MAG_MIP_POINT = 0,
	COREFILTER_MIN_MAG_POINT_MIP_LINEAR = 1,
	COREFILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 2,
	COREFILTER_MIN_POINT_MAG_MIP_LINEAR = 3,
	COREFILTER_MIN_LINEAR_MAG_MIP_POINT = 4,
	COREFILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 5,
	COREFILTER_MIN_MAG_LINEAR_MIP_POINT = 6,
	COREFILTER_MIN_MAG_MIP_LINEAR = 7,
	COREFILTER_ANISOTROPIC = 8,
	COREFILTER_COMPARISON_MIN_MAG_MIP_POINT = 9,
	COREFILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 10,
	COREFILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 11,
	COREFILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 12,
	COREFILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 13,
	COREFILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 14,
	COREFILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 15,
	COREFILTER_COMPARISON_MIN_MAG_MIP_LINEAR = 16,
	COREFILTER_COMPARISON_ANISOTROPIC = 17,
};
extern EnumNamePair FilterNames[];


#define CORELOCK_READONLY           0x00000010L
#define CORELOCK_DISCARD            0x00002000L
#define CORELOCK_NOOVERWRITE        0x00001000L
#define CORELOCK_NOSYSLOCK          0x00000800L
#define CORELOCK_DONOTWAIT          0x00004000L                  
