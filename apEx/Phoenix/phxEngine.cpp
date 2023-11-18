#include "phxEngine.h"
#include "Texgen.h"
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")
//d3dx10.lib; d3dx11.lib

/*

gamma-degamma

float gamma(float c)
{
float cs=saturate(c);

if (cs<0.0031308) cs=12.92*cs;
else cs=1.055*pow(cs,1/2.4)-0.055;

return cs;
}

float degamma(float c)
{
float cs;
if (c<0.04045) cs=c/12.92;
else cs=pow((c+0.055)/1.055,2.4);
return cs;
}

float4 gamma(float4 g)
{
return float4(gamma(g.x),gamma(g.y),gamma(g.z),gamma(g.w));
}

float4 degamma(float4 g)
{
return float4(degamma(g.x),degamma(g.y),degamma(g.z),degamma(g.w));
}


*/

ID3D11Device *phxDev;
ID3D11DeviceContext *phxContext;
ID3D11Buffer *TexgenBufferPS;
ID3D11Buffer *TexgenVertexBuffer;
ID3D11InputLayout *TexgenVertexFormat;
ID3D11InputLayout *RenderVertexFormat;
#ifdef PHX_HAS_STANDARD_PARTICLES
ID3D11InputLayout *ParticleVertexFormat;
#endif
ID3D11VertexShader *TexgenVertexShader;
ID3D11PixelShader *RenderPixelShader;
ID3D11SamplerState *TexgenSampler;
ID3D11SamplerState *TexgenSampler_NoWrap;
ID3D11SamplerState *TexgenSampler_ShadowCompare;
ID3D11Buffer *SceneDataBuffer;
ID3D11Buffer *ObjectMatrixBuffer;
ID3D11DepthStencilView* phxDepthBufferView
#ifndef PHX_MINIMAL_BUILD
= NULL
#endif
;
ID3D11ShaderResourceView* phxDepthBufferShaderView
#ifndef PHX_MINIMAL_BUILD
= NULL
#endif
;
ID3D11ShaderResourceView* phxMeshDataShaderView
#ifndef PHX_MINIMAL_BUILD
= NULL
#endif
;
ID3D11RenderTargetView* phxBackBufferView
#ifndef PHX_MINIMAL_BUILD
= NULL
#endif
;
IDXGISwapChain* SwapChain;

char* EngineFontList[ 32 ];

#ifdef PHX_VOLUMETRIC_RENDERTARGETS
ID3D11Texture3D *phxTexture3D;
ID3D11ShaderResourceView *phxTexture3DResourceView;
ID3D11UnorderedAccessView *phxTexture3DUAV;

ID3D11RenderTargetView *phxVoxelForceMSAARTView = NULL;
ID3D11Texture2D *phxVoxelForceMSAATexture;
#endif

#ifdef LTC1
#include "ltc_1.h"
ID3D11Texture2D* ltc1 = NULL;
ID3D11ShaderResourceView* ltc1view = NULL;
#endif

#ifdef LTC2
#include "ltc_2.h"
ID3D11Texture2D* ltc2 = NULL;
ID3D11ShaderResourceView* ltc2view = NULL;
#endif

void CreateRenderTarget( const int XRes, const int YRes, const DXGI_FORMAT Format, ID3D11ShaderResourceView *&View, ID3D11RenderTargetView *&RTView, ID3D11Texture2D *&Texture, bool voxelTarget );

bool phxDone = false;

CphxRenderTarget *phxInternalRenderTarget; //used for rendertarget copies

#ifndef PHX_MINIMAL_BUILD
fTimerFunction TimerFunction = timeGetTime;
#endif

#ifndef D3DCOMPILE_OPTIMIZATION_LEVEL3
#define D3DCOMPILE_OPTIMIZATION_LEVEL3 (1<<15)
#endif

static const float VxBuffer[] = { -1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, -1, 0, 1, 1, 1, -1, 1, 0, 1, 0, 0, 1, -1, 0, 1, 1, 1, -1, -1, 0, 1, 0, 1, };
static const D3D11_INPUT_ELEMENT_DESC vxdecl[ 2 ] =
{
  { "PositionT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof( float ) * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static const D3D11_INPUT_ELEMENT_DESC vxdecl3[ 2 ] =
{
  { "Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Texcoord", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof( float ) * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static const D3D11_INPUT_ELEMENT_DESC vxdecl2[ 6 ] =
{
  { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Position", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof( float ) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof( float ) * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof( float ) * 9, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Texcoord", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof( float ) * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "Texcoord", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof( float ) * 14, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static const CHAR shader[] =
"SamplerState o:register(s1);"
"Texture2D r:register(t0);"
"struct l{float4 o:positiont;float2 r:texcoord0;};"
"struct i{float4 o:position,r:texcoord0;};"
"struct n{float3 o:position0,r:position1,u:normal0;float4 c:color0,s:texcoord0,a:texcoord1;};"
"struct t{float2 o:texcoord0;float4 r:sv_position;};"
"t u(l c){t s;s.r=c.o;s.o=c.r;return s;}"
"t c(n s){t a;a.r=0;a.o=0;return a;}"
"t s(i a){t e;e.r=0;e.o=0;return e;}"
"float4 a(t e):sv_target0{return r.Sample(o,e.o);}";

static const char vs5[] = "vs_5_0";
static const char ps5[] = "ps_5_0";

static D3D11_BUFFER_DESC desc = { 0, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
static const D3D11_BUFFER_DESC bd = { 6 * 6 * sizeof( float ), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
static const D3D11_SUBRESOURCE_DATA vxData = { VxBuffer, 0, 0 };
static D3D11_SAMPLER_DESC smpData = { D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0, 16, D3D11_COMPARISON_NEVER, 0, 0, 0, 0, -D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX };

d3dcompilefunc *D3DCompileCall = NULL;

void __fastcall InitializePhoenix()
{
  if ( !D3DCompileCall ) //this is here so the tool can set a more properly found compile call so that people unwilling to install the latest directx don't crash the tool :P (hi, Val!)
  {
    HMODULE dll = LoadLibraryA( "d3dcompiler_47.dll" );
    D3DCompileCall = (d3dcompilefunc *)GetProcAddress( dll, "D3DCompile" );
  }

  desc.ByteWidth = SHADERDATALENGTH;
  phxDev->CreateBuffer( &desc, NULL, &TexgenBufferPS );

  desc.ByteWidth = SCENEDATASIZE;
  phxDev->CreateBuffer( &desc, NULL, &SceneDataBuffer );

  desc.ByteWidth = 16 * 4 * 2 + MATERIALDATASIZE;
  phxDev->CreateBuffer( &desc, NULL, &ObjectMatrixBuffer );

  ID3D10Blob *VS;
  D3DCompileCall( shader, sizeof( shader ), NULL, NULL, NULL, "a", ps5, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, NULL );
  phxDev->CreatePixelShader( VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &RenderPixelShader );
  //VS->Release();
  D3DCompileCall( shader, sizeof( shader ), NULL, NULL, NULL, "c", vs5, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, NULL );
  phxDev->CreateInputLayout( vxdecl2, 6, VS->GetBufferPointer(), VS->GetBufferSize(), &RenderVertexFormat );
  //VS->Release();
#ifdef PHX_HAS_STANDARD_PARTICLES
  D3DCompileCall( shader, sizeof( shader ), NULL, NULL, NULL, "s", vs5, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, NULL );
  phxDev->CreateInputLayout( vxdecl3, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &ParticleVertexFormat );
#endif
  //VS->Release();
  D3DCompileCall( shader, sizeof( shader ), NULL, NULL, NULL, "u", vs5, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &VS, NULL );
  phxDev->CreateInputLayout( vxdecl, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &TexgenVertexFormat );
  phxDev->CreateVertexShader( VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &TexgenVertexShader );
  //VS->Release();

  phxDev->CreateBuffer( &bd, &vxData, &TexgenVertexBuffer );

  phxDev->CreateSamplerState( &smpData, &TexgenSampler );
  smpData.AddressU = smpData.AddressV = smpData.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  phxDev->CreateSamplerState( &smpData, &TexgenSampler_NoWrap );
  smpData.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
  //smpData.MaxAnisotropy = 16;
  smpData.ComparisonFunc = D3D11_COMPARISON_LESS;
  phxDev->CreateSamplerState( &smpData, &TexgenSampler_ShadowCompare );

  phxContext->VSSetSamplers( 0, 1, &TexgenSampler );
  phxContext->PSSetSamplers( 0, 1, &TexgenSampler );
  phxContext->VSSetSamplers( 1, 1, &TexgenSampler_NoWrap );
  phxContext->PSSetSamplers( 1, 1, &TexgenSampler_NoWrap );
  phxContext->VSSetSamplers( 2, 1, &TexgenSampler_ShadowCompare );
  phxContext->PSSetSamplers( 2, 1, &TexgenSampler_ShadowCompare );

  TexgenPool = new CphxTexturePool();

#ifdef PHX_VOLUMETRIC_RENDERTARGETS

#define Texture3DSize 8

  D3D11_TEXTURE3D_DESC textureDesc;
  memset( &textureDesc, 0, sizeof textureDesc );
  textureDesc.Width = Texture3DSize;
  textureDesc.Height = Texture3DSize;
  textureDesc.Depth = Texture3DSize;
  textureDesc.MipLevels = 0;
  textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;// usage_ == TEXTURE_DYNAMIC ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;// usage_ == TEXTURE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
  //if ( usage_ == TEXTURE_UAV )
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
  textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

  //if ( mips_ != 1 && mips_ != 0 )
  //  textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

  phxDev->CreateTexture3D( &textureDesc, 0, &phxTexture3D );

  D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
  memset( &resourceViewDesc, 0, sizeof resourceViewDesc );
  resourceViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
  resourceViewDesc.Texture3D.MipLevels = -1;
  resourceViewDesc.Texture3D.MostDetailedMip = 0;

  phxDev->CreateShaderResourceView( phxTexture3D, &resourceViewDesc, &phxTexture3DResourceView );
  //if ( FAILED( hr ) )
  //{
  //  D3D_SAFE_RELEASE( shaderResourceView_ );
  //  YUMELOG_ERROR( "Failed to create shader resource view for texture", hr );
  //  return false;
  //}
  //else
  //{
  //  YumeString srvName = GetName();
  //  srvName.append( "_SRV" );

  //  ( (ID3D11ShaderResourceView*)shaderResourceView_ )->SetPrivateData( WKPDID_D3DDebugObjectName, srvName.length(), srvName.c_str() );
  //}
  D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
  ZeroMemory( &uav_desc, sizeof( uav_desc ) );

  uav_desc.Format = textureDesc.Format;
  uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
  uav_desc.Texture3D.FirstWSlice = 0;
  uav_desc.Texture3D.MipSlice = 0;
  uav_desc.Texture3D.WSize = Texture3DSize; //width

  phxDev->CreateUnorderedAccessView( phxTexture3D, &uav_desc, &phxTexture3DUAV );

  static const D3D11_TEXTURE2D_DESC tex = { Texture3DSize, Texture3DSize, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 8, D3D11_STANDARD_MULTISAMPLE_PATTERN, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, 0 };
  static const D3D11_RENDER_TARGET_VIEW_DESC rt = { DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2DMS, 0 };

  phxDev->CreateTexture2D( &tex, NULL, &phxVoxelForceMSAATexture );
  phxDev->CreateRenderTargetView( phxVoxelForceMSAATexture, &rt, &phxVoxelForceMSAARTView );

  //if ( FAILED( hr ) )
  //{
  //  D3D_SAFE_RELEASE( unorderedAccessView_ );
  //  YUMELOG_ERROR( "Failed to create UAV for texture " << hr );
  //  return false;
  //}
  //else
  //{
  //  YumeString srvName = GetName();
  //  srvName.append( "_UAV" );

  //  ( (ID3D11UnorderedAccessView*)unorderedAccessView_ )->SetPrivateData( WKPDID_D3DDebugObjectName, srvName.length(), srvName.c_str() );
  //}

#endif

#ifdef LTC1
  static const D3D11_TEXTURE2D_DESC ltctex16 = { 16, 16, 1, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
  static const D3D11_SHADER_RESOURCE_VIEW_DESC ltcview16 = { DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_SRV_DIMENSION_TEXTURE2D, 0, 1 };
#endif
#ifdef LTC2
  static const D3D11_TEXTURE2D_DESC ltctex8 = { 16, 16, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
  static const D3D11_SHADER_RESOURCE_VIEW_DESC ltcview8 = { DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_SRV_DIMENSION_TEXTURE2D, 0, 1 };
#endif

#if defined LTC1 || defined LTC2
  unsigned short ltcdata[ 16 * 16 * 4 ];
  D3D11_SUBRESOURCE_DATA subData = { ltcdata, 16 * 4 * 2, 0 };
#endif

#ifdef LTC1
  for ( int x = 0; x < 16 * 16; x++ )
  {
    for ( int y = 0; y < 4; y++ )
      ltcdata[ x * 4 + y ] = ( (unsigned short*)( raw_ltc_1 ) )[ x + 16 * 16 * y ];
  }
  phxDev->CreateTexture2D( &ltctex16, &subData, &ltc1 );
  phxDev->CreateShaderResourceView( ltc1, &ltcview16, &ltc1view );
#endif

#ifdef LTC2
  for ( int x = 0; x < 16 * 16; x++ )
  {
    for ( int y = 0; y < 2; y++ )
      ( (unsigned char*)ltcdata )[ x * 4 + y ] = ( (unsigned char*)( raw_ltc_2 ) )[ x + 16 * 16 * ( y ) ];
  }

  subData.SysMemPitch = 16 * 4;
  phxDev->CreateTexture2D( &ltctex8, &subData, &ltc2 );
  phxDev->CreateShaderResourceView( ltc2, &ltcview8, &ltc2view );
#endif

  //ID3D11Texture2D *ltc1 = NULL, *ltc2 = NULL;
  //ID3D11ShaderResourceView *ltc1view = NULL, *ltc2view = NULL;

}

//void FixFloatingPoint()
//{
//	//set up fpu rounding mode
//
//	//static unsigned short	CW;
//	//__asm
//	//{
//	//	fstcw	CW							// store fpu control word  
//	//	mov		dx, word ptr[CW]
//	//	or		dx, 0x0C00                  // rounding: truncate (default)
//	//	mov		CW, dx
//	//	fldcw	CW							// load modfied control word  
//	//}
//}
//
//void UnFixFloatingPoint()
//{
//	//set up fpu rounding mode
//
//	//static unsigned short	CW;
//	//__asm
//	//{
//	//	fstcw	CW							// store fpu control word  
//	//	mov		dx, word ptr[CW]
//	//	and		dx, 0xFFFFF3FF                  // rounding: truncate (default)
//	//	mov		CW, dx
//	//	fldcw	CW							// load modfied control word  
//	//}
//}
