#pragma once
#include "RenderTarget.h"
#include "phxarray.h"
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif
#pragma pack(push,1)

#define TEXGEN_MAX_PARENTS 3
#define TEXGEN_MAX_PARAMS 16
#define SHADERDATALENGTH ((4+TEXGEN_MAX_PARAMS)*sizeof(float))

//#define TEXTUREFORMAT DXGI_FORMAT_R16G16B16A16_UNORM
//#define TEXTUREFORMAT DXGI_FORMAT_R16G16B16A16_FLOAT

#define TEXTUREFORMATSIZE 8

//////////////////////////////////////////////////////////////////////////
// texture pool

class CphxTexturePoolTexture : public CphxRenderTarget
{
public:
  unsigned char Resolution;
  ID3D11Texture2D *Texture;
  bool Used;
  bool Deleted;
  bool hdr;

  virtual bool Create( unsigned char res, bool hdr );
};

class CphxTexturePool
{

public:

  //CphxArray<CphxTexturePoolTexture *> Pool;
  CphxTexturePoolTexture **pool;
  int poolSize;

  virtual CphxTexturePoolTexture *GetTexture( unsigned char Resolution, bool hdr );

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxTexturePool() {};
#endif
};

//////////////////////////////////////////////////////////////////////////
// texture filter

struct PHXTEXTDATA //this is written out as fwrite(&data,1,5,f) -> the last 3 bytes are ignored, the 5 is also used in the CphxTextureOperator_Tool::ParametersMatchWith function!
{
  unsigned char Size;
  unsigned char XPos;
  unsigned char YPos;
  unsigned char CharSpace;
  unsigned int Bold : 1;
  unsigned int Italic : 1;
  unsigned int Font : 6;
  //char *Text;
};

struct PHXFILTERDATADESCRIPTOR
{
  unsigned int NeedsRandSeed : 1;
  unsigned int InputCount : 2;
  unsigned int ParameterCount : 5; //32 bytes max
  unsigned int PassCount : 4; //16 iterations
  unsigned int LookupType : 4; //only 3 bits used
};

class PHXTEXTUREFILTER
{
#ifdef PHX_TEXGEN_IMAGE
  void LoadMetafile( unsigned char *Output, int XRes, int YRes, void *ImageData, int ImageSize );
#endif

public:
  PHXFILTERDATADESCRIPTOR DataDescriptor;
  //unsigned char PassCount;
  //unsigned char Shader; //used to access the appropriate shader in the minimal project. not needed during runtime.

  ID3D11PixelShader *PixelShader;

  //void ReadFromStream(unsigned char *&Stream);
  void Render( CphxTexturePoolTexture *&Target, CphxTexturePoolTexture *&SwapBuffer, CphxTexturePoolTexture *Inputs[ TEXGEN_MAX_PARENTS ], unsigned char RandSeed, unsigned char Parameters[ TEXGEN_MAX_PARAMS ], void *ExtraData, int ExtraDataSize );

  virtual CphxTexturePoolTexture *GetLookupTexture( unsigned char Res, void *ExtraData, int ExtraDataSize );
};

//////////////////////////////////////////////////////////////////////////
// texture operator

#define GETXRES(x) (1<<(x>>4))
#define GETYRES(y) (1<<(y&0x0f))

#define FILTER_SUBROUTINECALL	(255)
#define FILTER_IMAGELOAD		(254)
#define FILTER_TEXTDISPLAY		(253)
#define FILTER_SPLINE			(252)

struct PHXTEXTUREOPERATOR
{
  unsigned char Resolution;
  unsigned char Filter; // top bit is hdr

  unsigned char RandSeed;
  short Parents[ TEXGEN_MAX_PARENTS ];
  unsigned char Parameters[ TEXGEN_MAX_PARAMS ];

  bool NeedsRender;
  CphxTexturePoolTexture *CachedResult;

  int minimportData2; //used for data size and as the text pointer
  void *minimportData;
  //PHXTEXTDATA *minimportTextData;
  //class CphxSpline_float16 *minimportSplines[4];

  //void ReadFromStream(unsigned char *&Stream, PHXTEXTUREFILTER *Filters);
  CphxTexturePoolTexture *Generate( PHXTEXTUREFILTER *Filters, PHXTEXTUREOPERATOR *Operators );
};

//////////////////////////////////////////////////////////////////////////
// texture subroutine

struct PHXTEXTUREPARAMETEROVERRIDE
{
  unsigned char TargetOperator;
  unsigned char TargetParameter;
};

struct PHXTEXTURESUBROUTINE
{
public:
  PHXFILTERDATADESCRIPTOR DataDescriptor;

  PHXTEXTUREOPERATOR Operators[ 256 ];
  unsigned char Output;
  unsigned char Inputs[ TEXGEN_MAX_PARENTS ];

  unsigned char DynamicParameterCount;
  PHXTEXTUREPARAMETEROVERRIDE DynamicParameters[ TEXGEN_MAX_PARAMS ];

  //void ReadFromStream(unsigned char *&Stream);
  CphxTexturePoolTexture *Generate( PHXTEXTUREFILTER *Filters, PHXTEXTUREOPERATOR *Operators, unsigned short *Parents, unsigned char *Parameters, unsigned char Resolution );//CphxTextureOperator *ops, unsigned char Parameters[TEXGEN_MAX_PARAMS], CphxTextureOperator *Inputs[TEXGEN_MAX_PARENTS], int xres, int yres);
};

//extern CphxTexturePool TexgenPool;

extern PHXTEXTUREOPERATOR *TextureOperators;
extern CphxTexturePool *TexgenPool;

#pragma pack(pop)
