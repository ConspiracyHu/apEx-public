#include "Project.h"
//#include "Event.h"
//#include <D3Dcompiler.h>
#ifndef D3DCOMPILE_OPTIMIZATION_LEVEL3
#define D3DCOMPILE_OPTIMIZATION_LEVEL3 (1<<15)
#endif

#ifndef D3DCOMPILE_OPTIMIZATION_LEVEL0
#define D3DCOMPILE_OPTIMIZATION_LEVEL0 (1<<14)
#endif

#include "phxEngine.h"
#include "..\LibCTiny\libcminimal.h"

/*

inline uint32 WireFormatLite::ZigZagEncode32(int32 n) {
// Note:  the right-shift must be arithmetic
return (n << 1) ^ (n >> 31);
}

inline int32 WireFormatLite::ZigZagDecode32(uint32 n) {
return (n >> 1) ^ -static_cast<int32>(n & 1);
}

void CodedOutputStream::WriteVarint32(uint32 value) {
if (buffer_size_ >= kMaxVarint32Bytes) {
// Fast path:  We have enough bytes left in the buffer to guarantee that
// this write won't cross the end, so we can skip the checks.
uint8* target = buffer_;
uint8* end = WriteVarint32FallbackToArrayInline(value, target);
int size = end - target;
Advance(size);
} else {
// Slow path:  This write might cross the end of the buffer, so we
// compose the bytes first then use WriteRaw().
uint8 bytes[kMaxVarint32Bytes];
int size = 0;
while (value > 0x7F) {
bytes[size++] = (static_cast<uint8>(value) & 0x7F) | 0x80;
value >>= 7;
}
bytes[size++] = static_cast<uint8>(value) & 0x7F;
WriteRaw(bytes, size);
}
}

inline const uint8* ReadVarint32FromArray(
const uint8* buffer, uint32* value) GOOGLE_ATTRIBUTE_ALWAYS_INLINE;
inline const uint8* ReadVarint32FromArray(const uint8* buffer, uint32* value) {
// Fast path:  We have enough bytes left in the buffer to guarantee that
// this read won't cross the end, so we can skip the checks.
const uint8* ptr = buffer;
uint32 b;
uint32 result;

b = *(ptr++); result  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
b = *(ptr++); result |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
b = *(ptr++); result |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
b = *(ptr++); result |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
b = *(ptr++); result |=  b         << 28; if (!(b & 0x80)) goto done;

// If the input is larger than 32 bits, we still need to read it all
// and discard the high-order bits.
for (int i = 0; i < kMaxVarintBytes - kMaxVarint32Bytes; i++) {
b = *(ptr++); if (!(b & 0x80)) goto done;
}

// We have overrun the maximum size of a varint (10 bytes).  Assume
// the data is corrupt.
return NULL;

done:
*value = result;
return ptr;
}

*/

void ImportSpline( CphxSpline *Spline );
void ImportClipSpline( CphxClipSpline *Spline );
void GenerateMesh( CphxModelObject *o, CphxModel *m );
void BuildPassState( CphxMaterialTechnique *tech, CphxMaterialRenderPass *Pass, CphxMaterialPassConstantState* State );
void CollectRenderState( CphxMaterialParameterBatch &Params, D3D11_RASTERIZER_DESC &RasterDesc, D3D11_DEPTH_STENCIL_DESC &DepthStencilDesc, D3D11_BLEND_DESC &BlendDesc, CphxMaterialPassConstantState *pass );
void ImportMaterialSplineBatch( CphxModel *m, CphxObjectClip *clip, int ClipID );
void ImportTechSplineBatch( CphxMaterialTechnique *m, void *group, CphxMaterialSplineBatch *clip, int ClipID );
void ImportMaterialParamSplineBatch( CphxMaterialParameterBatch *m, void *group, CphxMaterialSplineBatch *clip, int ClipID );
void ImportTechniqueData( CphxProject* proj, CphxMaterialTechnique* t, int& passid, CphxMaterialPassConstantState** state );

void ImportMaterialParams( CphxProject *proj, CphxMaterialParameterBatch &Params );
void ImportMaterialParamValue( CphxProject *proj, CphxMaterialParameter *Param );
void ImportVariableMaterialData( CphxProject *proj, CphxMaterial *&mat, CphxMaterialPassConstantState **&state );

static D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D, 0, 0 };
static D3D11_SHADER_RESOURCE_VIEW_DESC resdesc = { DXGI_FORMAT_R24_UNORM_X8_TYPELESS, D3D11_SRV_DIMENSION_TEXTURE2D, 0, 1 };
static D3D11_TEXTURE2D_DESC depthBufferDesc = { 0, 0, 1, 1, DXGI_FORMAT_R24G8_TYPELESS, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0 };
static D3D11_RENDER_TARGET_BLEND_DESC blenddesc = { true, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
static char *entrypoints[] = { "v", "p", "g" };// , "h", "d" };
static char *versions[] = { "vs_5_0", "ps_5_0", "gs_5_0" };// , "hs_5_0", "ds_5_0" };
static float one[] = { 1, 1, 1 };
#ifdef HAS_ARBARO
static TREESPECIESDESCRIPTOR *TreeSpecies;
#endif

//static const PHXSPLINETYPE DefaultSplines[] = { Spline_Position_x, Spline_Position_y, Spline_Position_z, Spline_Scale_x, Spline_Scale_y, Spline_Scale_z, Spline_Rotation };

static unsigned char MeshPrimitiveParameterCounts[] = //IF THESE CHANGE YOU NEED TO CHANGE THEM IN THE MINIMALEXPORT.CPP AS WELL
{
  0,//Mesh_Cube = 0,
  2,//Mesh_Plane,
  5,//Mesh_Sphere,
  3,//Mesh_Cylinder,
  4,//Mesh_Cone,
  3,//Mesh_Arc,
  1,//Mesh_Line,
  2,//Mesh_Spline,
  7,//Mesh_Loft,
  0,//Mesh_Clone,
  0,//Mesh_Copy,
  2,//Mesh_GeoSphere,
  14,//Mesh_Scatter,
  0,//Mesh_Stored,
  6,//Mesh_Tree,
  7,//Mesh_TreeLeaves,
  2,//Mesh_Text,
  0,//Mesh_Marched
  0,//Mesh_StoredMini
  0,//Mesh_Merge
};

static unsigned char MeshFilterParameterCounts[] = //IF THESE CHANGE YOU NEED TO CHANGE THEM IN THE MINIMALEXPORT.CPP AS WELL
{
  2,//ModelFilter_UVMap = 0,
  1,//ModelFilter_Bevel,
  0,//ModelFilter_MapXForm,
  2,//ModelFilter_MeshSmooth,
  1,//ModelFilter_SmoothGroup,
  2,//ModelFilter_TintMesh,
  3,//ModelFilter_TintMeshShape,
  1,//ModelFilter_Replicate,
  2,//ModelFilter_NormalDeform,
  2,//ModelFilter_CSG
  3,//ModelFilter_Greeble
};

static unsigned char MeshFilterExtraDataSize[] =
{
  20,//ModelFilter_UVMap = 0,
  0,//ModelFilter_Bevel,
  20,//ModelFilter_MapXForm,
  0,//ModelFilter_MeshSmooth,
  0,//ModelFilter_SmoothGroup,
  2,//ModelFilter_TintMesh,
  20,//ModelFilter_TintMeshShape,
  24,//ModelFilter_Replicate,
  0,//ModelFilter_NormalDeform,
  0,//ModelFilter_CSG
};

static unsigned char PixelFormats[] = //IF THESE CHANGE YOU NEED TO ADD THEM TO THE RENDERTARGETEDITOR.CPP STRING LIST AND apxProject.CPP AS WELL
{
  DXGI_FORMAT_R16G16B16A16_FLOAT,
  DXGI_FORMAT_R32_FLOAT,
};

extern IDXGISwapChain *SwapChain;
static int LastPrecalcTime;
static CphxProject *Precalc = nullptr;

static bool DoPrecalc( float Pos )
{
  if ( !Precalc ) return false;
  int tme = timeGetTime();
  if ( tme - LastPrecalcTime < 40 ) return false;
  LastPrecalcTime = tme;

  Precalc->Timeline.Render( Pos * 1000, false, false );

#ifdef DEBUGINTOFILE
  HRESULT res =
#endif
    SwapChain->Present( 0, 0 );

#ifdef DEBUGINTOFILE
  if ( res != S_OK )
    DEBUGLOG( "*** Failed precalc present at position %f", Pos );
#endif

  MSG msg;
  while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != 0 )
    DispatchMessage( &msg );

  return phxDone;
}

static void CreateRenderTarget( const DXGI_FORMAT Format, CphxRenderTarget& target )
{
  D3D11_TEXTURE2D_DESC tex = depthBufferDesc;
  tex.Width = target.XRes;
  tex.Height = target.YRes;
  tex.Format = Format;
  tex.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  tex.MipLevels = 0;
  tex.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
  D3D11_RENDER_TARGET_VIEW_DESC rt = { Format, D3D11_RTV_DIMENSION_TEXTURE2DMS, 0 };

  phxDev->CreateTexture2D( &tex, NULL, &target.Texture );
  phxDev->CreateShaderResourceView( target.Texture, NULL, &target.View );
  phxDev->CreateRenderTargetView( target.Texture, &rt, &target.RTView );

#ifdef DEBUGINTOFILE
  if ( !target.Texture || !target.View || !target.RTView )
    DEBUGLOG( "Failed to create rendertarget at size %d %d", target.XRes, target.YRes );
  else
    DEBUGLOG( "Rendertarget created at size %d %d", target.XRes, target.YRes );
#endif
}

static void CreateDepthBuffer( int XRes, int YRes )
{
  D3D11_TEXTURE2D_DESC dBD = depthBufferDesc;
  dBD.Width = XRes;
  dBD.Height = YRes;

  ID3D11Texture2D *DepthBuffer;
  phxDev->CreateTexture2D( &dBD, NULL, &DepthBuffer );
  phxDev->CreateDepthStencilView( DepthBuffer, &depthStencilViewDesc, &phxDepthBufferView );
  phxDev->CreateShaderResourceView( DepthBuffer, &resdesc, &phxDepthBufferShaderView );

#ifdef DEBUGINTOFILE
  if ( !DepthBuffer )
    DEBUGLOG( "Failed to create depth buffer at size %d %d", XRes, YRes );
  else
    DEBUGLOG( "Depth buffer created at size %d %d", XRes, YRes );
#endif
}

static unsigned char **datastreams;

static const unsigned char readbyte( const PHXDATASTREAMS Stream ) { return datastreams[ Stream ]++[ 0 ]; }
static const unsigned int readdword( const PHXDATASTREAMS Stream ) { unsigned int s = ( (unsigned int*)datastreams[ Stream ] )[ 0 ]; datastreams[ Stream ] += 4; return s; }
#ifdef PHX_MESH_LOADSTOREDMESH
static const float readfloat32( const PHXDATASTREAMS Stream ) { float s = ( (float*)datastreams[ Stream ] )[ 0 ]; datastreams[ Stream ] += 4; return s; }
#endif
static const unsigned short readshort( const PHXDATASTREAMS Stream ) { unsigned short s = ( (unsigned short*)datastreams[ Stream ] )[ 0 ]; datastreams[ Stream ] += 2; return s; }
static unsigned char *skiptext( const PHXDATASTREAMS Stream ) { unsigned char *c = datastreams[ Stream ]; while ( *( datastreams[ Stream ]++ ) ); return c; }
static unsigned char *copyshader( unsigned char* target )
{
  do
  {
    *( target++ ) = *( datastreams[ Stream_Shaders ] );
  } while ( *datastreams[ Stream_Shaders ]++ );
  return target;
}

static D3DXFLOAT16 readfloat( PHXDATASTREAMS Stream )
{
  unsigned short s = readshort( Stream );
  return *( (D3DXFLOAT16*)&s );
}

void CphxProject::LoadProject( unsigned char *DATA, CphxProject *PrecalcProject, SETUPCFG &setup )
{
  DEBUGLOG( "Starting Project Load" );

  Precalc = PrecalcProject;
  LastPrecalcTime = timeGetTime();

  int Count;

  // stream setup
  datastreams = (unsigned char**)DATA;
  for ( int x = 0; x < StreamCount; x++ )
    datastreams[ x ] += (int)DATA;

  //////////////////////////////////////////////////////////////////////////
  // aspect ratio and framerate info, needed for rendertargets

  *( (unsigned int*)&Timeline.AspectX ) = readdword( Stream_Main ); // read x and y aspects, framerate and rendertarget count

  //////////////////////////////////////////////////////////////////////////
  // load rendertargets

  //calculate demo base resolution

  int XRes = Timeline.ScreenX = setup.mode.dmPelsWidth;
  int YRes = Timeline.ScreenY = setup.mode.dmPelsHeight;

  float demoResolutionAspect = XRes / (float)YRes;
  float monitorResolutionAspect = setup.fullscreen ? demoResolutionAspect : ( GetSystemMetrics( SM_CXSCREEN ) / (float)GetSystemMetrics( SM_CYSCREEN ) );
  float aspect = Timeline.AspectX / (float)Timeline.AspectY / setup.HardwareAspectRatio * monitorResolutionAspect;

  if ( aspect > demoResolutionAspect )
    YRes = (int)( XRes / aspect + 0.5f ); //+0.5f added for rounding precision
  else
    XRes = (int)( YRes * aspect + 0.5f ); //+0.5f added for rounding precision

  CreateDepthBuffer( XRes, YRes );

  //create internal rendertarget
  extern CphxRenderTarget *phxInternalRenderTarget;
  phxInternalRenderTarget = new CphxRenderTarget;
  phxInternalRenderTarget->XRes = XRes;
  phxInternalRenderTarget->YRes = YRes;
  CreateRenderTarget( DXGI_FORMAT_R16G16B16A16_FLOAT, *phxInternalRenderTarget );

  Timeline.RenderTargets = new CphxRenderTarget*[ Timeline.RenderTargetCount ];
  for ( int x = 0; x < Timeline.RenderTargetCount; x++ )
  {
    Timeline.RenderTargets[ x ] = new CphxRenderTarget;
    GetRenderTargetResolution( readbyte( Stream_Main ), Timeline.RenderTargets[ x ]->XRes, Timeline.RenderTargets[ x ]->YRes, XRes, YRes );

    unsigned char pFormat = readbyte( Stream_Main );
    Timeline.RenderTargets[ x ]->cubeMap = ( pFormat & 0xf0 ) != 0;

    CreateRenderTarget( (DXGI_FORMAT)PixelFormats[ pFormat & 0x0f ], *Timeline.RenderTargets[ x ] );
  }

  //////////////////////////////////////////////////////////////////////////
  // load render layers

  DEBUGLOG( "Loading RenderLayers" );

  int RenderLayerCount = readbyte( Stream_Main ); //layer count
  RenderLayers = new CphxRenderLayerDescriptor[ RenderLayerCount ];
  for ( int x = 0; x < RenderLayerCount; x++ )
  {
    unsigned char nfo = readbyte( Stream_Main );
    RenderLayers[ x ].OmitDepthBuffer = ( ( nfo >> 6 ) & 1 ) != 0;
    RenderLayers[ x ].clearRenderTargets = ( nfo >> 7 ) != 0;
    RenderLayers[ x ].TargetCount = nfo & ( ( 1 << 6 ) - 1 ); //rt count
    RenderLayers[ x ].Targets = new CphxRenderTarget*[ RenderLayers[ x ].TargetCount ];
    for ( int y = 0; y < RenderLayers[ x ].TargetCount; y++ )
      RenderLayers[ x ].Targets[ y ] = Timeline.RenderTargets[ readbyte( Stream_Main ) ];
  }

  //////////////////////////////////////////////////////////////////////////
  // load common shader code

  unsigned char* shaderInclude = skiptext( Stream_Shaders );
  unsigned char* shaderTarget = datastreams[ Stream_Shaders ] - 1;

  //////////////////////////////////////////////////////////////////////////
  // load texture filters

  DEBUGLOG( "Loading Texture Filters" );

  Count = readbyte( Stream_Main ); //filter count
  TextureFilters = new PHXTEXTUREFILTER[ Count ];

  //UnFixFloatingPoint(); //this affects the shader code created!!!

  for ( int x = 0; x < Count; x++ )
  {
    *( (unsigned short*)&TextureFilters[ x ].DataDescriptor ) = readshort( Stream_Main );
    //unsigned char *Shader = skiptext( Stream_Shaders );
    //compile texture filter

    unsigned char* endpos = copyshader( shaderTarget );

    ID3D10Blob *PS;

#ifdef _DEBUG
    ID3D10Blob *Error;
    D3DCompileCall( (char*)shaderInclude, endpos - shaderInclude, NULL, NULL, NULL, "p", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, &PS, &Error );
    char *ErrorText = NULL;
    if ( Error )
      ErrorText = (char*)Error->GetBufferPointer();
#else
    D3DCompileCall( (char*)shaderInclude, endpos - shaderInclude, NULL, NULL, NULL, "p", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, &PS, NULL );
#endif
    phxDev->CreatePixelShader( PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &TextureFilters[ x ].PixelShader );

#ifdef DEBUGINTOFILE
    if ( !TextureFilters[ x ].PixelShader )
      DEBUGLOG( "Compiling Texture Filter %d/%d failed.", x, Count );
    else
      DEBUGLOG( "Compiling Texture Filter %d/%d OK.", x, Count );
#endif

    if ( DoPrecalc( 0.15f*x / (float)Count ) ) return;
  }

  //FixFloatingPoint();

  //CphxTexturePoolTexture *t = TexgenPool->GetTexture(0);
  //TextureFilters[0].Render(t, t, NULL, 0, NULL, NULL, 0);

  //////////////////////////////////////////////////////////////////////////
  // load texture subroutines

  //skipped for now
  //Count = readbyte(); //subroutine count
  //TextureSubroutines = new PHXTEXTURESUBROUTINE[Count];
  //for (int x = 0; x < Count; x++)	TextureSubroutines[x].ReadFromStream(data);

  //////////////////////////////////////////////////////////////////////////
  // load font list
  Count = readbyte( Stream_Main );
  for ( int x = 0; x < Count; x++ )
    EngineFontList[ x ] = (char*)skiptext( Stream_ASCIIZ );

  //////////////////////////////////////////////////////////////////////////
  // load texture operators

  DEBUGLOG( "Loading Texture Operators" );

  Count = readshort( Stream_Main ); //texture operator count
  TextureOperators = new PHXTEXTUREOPERATOR[ Count ];
  int renderedcount = readshort( Stream_Main );
  Count = readbyte( Stream_Main ); //page count

  int texopcnt = 0;

  for ( int x = 0; x < Count; x++ )
  {
    unsigned char PageResolution = readbyte( Stream_Main ); //page resolution
    unsigned int pageopcnt = readshort( Stream_Main ); //texture operator count in page

    for ( unsigned int y = 0; y < pageopcnt; y++ )
    {
      PHXTEXTUREOPERATOR &o = TextureOperators[ texopcnt ];
      o.Resolution = PageResolution;
      o.Filter = readbyte( Stream_MaterialParam );
      o.NeedsRender = texopcnt++ < renderedcount;// (o.Filter & 0x80) != 0;
      o.Filter &= 0x7F;
      PHXFILTERDATADESCRIPTOR &f = TextureFilters[ o.Filter ].DataDescriptor;
      memcpy( o.Parameters, datastreams[ Stream_Main ], f.ParameterCount );										//THIS MIGHT BE CHANGED TO SIMPLY REFERENCE THE DATA, NEED TO LOOK INTO IT
      datastreams[ Stream_Main ] += f.ParameterCount; //parameters
      if ( f.NeedsRandSeed ) o.RandSeed = readbyte( Stream_Main ); //randseed

      for ( unsigned int z = 0; z < TEXGEN_MAX_PARENTS; z++ ) //parent ops
      {
        if ( z < f.InputCount )
          o.Parents[ z ] = readshort( Stream_TextureOperatorReference );
        else
          o.Parents[ z ] = -1;
      }

      switch ( f.LookupType ) //skip for now
      {
#ifdef PHX_TEXGEN_IMAGE
      case 1: //image load
        o.minimportData2 = ( (unsigned int*)datastreams[ Stream_Main ] )[ 0 ];
        datastreams[ Stream_Main ] += 4;
        o.minimportData = datastreams[ Stream_Main ];
        datastreams[ Stream_Main ] += o.minimportData2;
        break;
#endif
      case 2: //text data
      {
        o.minimportData = datastreams[ Stream_Main ];// dta;
        datastreams[ Stream_Main ] += 5;
        o.minimportData2 = (int)skiptext( Stream_ASCIIZ );
      }
      break;
      case 3: //spline data
        o.minimportData = new CphxSpline_float16*[ 4 ];
        for ( int z = 0; z < 4; z++ )
        {
          ( (CphxSpline_float16**)o.minimportData )[ z ] = new CphxSpline_float16;
          ImportSpline( ( (CphxSpline_float16**)o.minimportData )[ z ] );
        }
        break;
      }
    }
  }

  //allocate texture rendertargets
  TexgenPool->pool = new CphxTexturePoolTexture*[ texopcnt ];
  TexgenPool->poolSize = 0;
  //TexgenPool->PoolSize = texopcnt;
  //for (int x = 0; x < texopcnt; x++)
  //{
  //	CphxTexturePoolTexture *t = new CphxTexturePoolTexture;
  //	TexgenPool->Pool[x] = t;
  //	t->Create(TextureOperators[x].Resolution);		
  //}

  DEBUGLOG( "Rendering Textures" );

  //render textures \o/
  for ( int x = 0; x < renderedcount; x++ )
    //if (TextureOperators[x].NeedsRender)
  {
    DEBUGLOG( "Generating Operator: %d", x );
    ( TextureOperators[ x ].CachedResult = TextureOperators[ x ].Generate( TextureFilters, TextureOperators ) )->Deleted = true;
    //TexgenPool->Pool.Delete(TextureOperators[x].CachedResult);
    if ( DoPrecalc( 0.15f + 0.35f*x / (float)renderedcount ) ) return;
    DEBUGLOG( "Rendered Texture %d/%d - resolution %d %d", x, renderedcount, GETXRES( TextureOperators[ x ].Resolution ), GETYRES( TextureOperators[ x ].Resolution ) );
  }

  //TextureSubroutines[0].Generate(TextureFilters, TextureOperators, NULL, NULL, 0);
  //TextureOperators[0].Generate(TextureFilters, TextureOperators);

  //////////////////////////////////////////////////////////////////////////
  // generate textures

  //load all texture operators and generate all textures one by one as needed. don't store temporary images, it's precalc dammit.

  //////////////////////////////////////////////////////////////////////////
  // load material techniques

  DEBUGLOG( "Loading Material Techniques" );

  Count = readbyte( Stream_Main ); //tech count
  Techniques = new CphxMaterialTechnique[ Count ];
  for ( int x = 0; x < Count; x++ )
  {
    DEBUGLOG( "Loading Material Technique %d", x );

    Techniques[ x ].TargetLayer = &RenderLayers[ readbyte( Stream_Main ) ];
    ImportMaterialParams( this, Techniques[ x ].Parameters );

    Techniques[ x ].PassCount = readbyte( Stream_Main );
    Techniques[ x ].RenderPasses = new CphxMaterialRenderPass*[ Techniques[ x ].PassCount ];

    for ( int y = 0; y < Techniques[ x ].PassCount; y++ )
    {
      CphxMaterialRenderPass *p = new CphxMaterialRenderPass;
      Techniques[ x ].RenderPasses[ y ] = p;
      ImportMaterialParams( this, p->Parameters );
      //char *Shader = (char*)skiptext( Stream_Shaders );
#ifdef _DEBUG
      unsigned char* shaderPos = datastreams[ Stream_Shaders ];
#endif
      unsigned char* endpos = copyshader( shaderTarget );

      ID3D10Blob *Blobs[ 5 ];

#ifdef DEBUGINTOFILE
      ID3D10Blob *Errors[ 5 ];
      //DEBUGLOG( "%s", Shader );
#endif

      //compile materials
      for ( int z = 0; z < 3; z++ )
      {
#ifdef _DEBUG
        ID3D10Blob *Error;
        D3DCompileCall( (char*)shaderInclude, endpos - shaderInclude, NULL, NULL, NULL, entrypoints[ z ], versions[ z ], D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &Blobs[ z ], &Error );
        char *ErrorText = NULL;
        if ( Error )
          ErrorText = (char*)Error->GetBufferPointer();
        int xxxxexxxx = 0;
        p->shaderText = new char[ endpos - shaderInclude ];
        memcpy( p->shaderText, shaderInclude, endpos - shaderInclude );
#else
        D3DCompileCall( (char*)shaderInclude, endpos - shaderInclude, NULL, NULL, NULL, entrypoints[ z ], versions[ z ], D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &Blobs[ z ], NULL );
#endif
      }

      //typedef bool(*TEXTEDITCALLBACKW)(CInputFieldW *i, int Chr);

      ID3D10Blob **Blobs_ = Blobs;
      if ( *Blobs_ ) phxDev->CreateVertexShader( ( *Blobs_ )->GetBufferPointer(), ( *Blobs_ )->GetBufferSize(), NULL, &p->VS ); Blobs_++;
#ifdef DEBUGINTOFILE
      if ( !p->VS )
      {
        DEBUGLOG( "Compiling Vertex   Shader %d/%d in Technique %d/%d failed, or shader not required.", y, Techniques[ x ].PassCount, x, Count );
        if ( Errors[ 0 ] && Errors[ 0 ]->GetBufferPointer() )
          DEBUGLOG( "%s\n", Errors[ 0 ]->GetBufferPointer() );
        else DEBUGLOG( "ERROR BLOB IS NULL\n" );
      }
      else
        DEBUGLOG( "Compiling Vertex   Shader %d/%d in Technique %d/%d OK.", y, Techniques[ x ].PassCount, x, Count );
#endif
      if ( *Blobs_ ) phxDev->CreatePixelShader( ( *Blobs_ )->GetBufferPointer(), ( *Blobs_ )->GetBufferSize(), NULL, &p->PS ); Blobs_++;
#ifdef DEBUGINTOFILE
      if ( !p->PS )
      {
        DEBUGLOG( "Compiling Pixel    Shader %d/%d in Technique %d/%d failed, or shader not required.", y, Techniques[ x ].PassCount, x, Count );
        if ( Errors[ 1 ] && Errors[ 1 ]->GetBufferPointer() )
          DEBUGLOG( "%s\n", Errors[ 1 ]->GetBufferPointer() );
        else DEBUGLOG( "ERROR BLOB IS NULL\n" );
      }
      else
        DEBUGLOG( "Compiling Pixel    Shader %d/%d in Technique %d/%d OK.", y, Techniques[ x ].PassCount, x, Count );
#endif
      if ( *Blobs_ ) phxDev->CreateGeometryShader( ( *Blobs_ )->GetBufferPointer(), ( *Blobs_ )->GetBufferSize(), NULL, &p->GS ); Blobs_++;
#ifdef DEBUGINTOFILE
      if ( !p->GS )
      {
        DEBUGLOG( "Compiling Geometry Shader %d/%d in Technique %d/%d failed, or shader not required.", y, Techniques[ x ].PassCount, x, Count );
        if ( Errors[ 2 ] && Errors[ 2 ]->GetBufferPointer() )
          DEBUGLOG( "%s\n", Errors[ 2 ]->GetBufferPointer() );
        else DEBUGLOG( "ERROR BLOB IS NULL\n" );
      }
      else
        DEBUGLOG( "Compiling Geometry Shader %d/%d in Technique %d/%d OK.", y, Techniques[ x ].PassCount, x, Count );
#endif
      //      if ( *Blobs_ ) phxDev->CreateHullShader( ( *Blobs_ )->GetBufferPointer(), ( *Blobs_ )->GetBufferSize(), NULL, &p->HS ); Blobs_++;
      //#ifdef DEBUGINTOFILE
      //      if ( !p->HS )
      //      {
      //        DEBUGLOG( "Compiling Hull     Shader %d/%d in Technique %d/%d failed, or shader not required.", y, Techniques[ x ].PassCount, x, Count );
      //        if ( Errors[ 3 ] && Errors[ 3 ]->GetBufferPointer() )
      //          DEBUGLOG( "%s\n", Errors[ 3 ]->GetBufferPointer() );
      //        else DEBUGLOG( "ERROR BLOB IS NULL\n" );
      //      }
      //      else
      //        DEBUGLOG( "Compiling Hull     Shader %d/%d in Technique %d/%d OK.", y, Techniques[ x ].PassCount, x, Count );
      //#endif
      //      if ( *Blobs_ ) phxDev->CreateDomainShader( ( *Blobs_ )->GetBufferPointer(), ( *Blobs_ )->GetBufferSize(), NULL, &p->DS );
      //#ifdef DEBUGINTOFILE
      //      if ( !p->DS )
      //      {
      //        DEBUGLOG( "Compiling Domain   Shader %d/%d in Technique %d/%d failed, or shader not required.", y, Techniques[ x ].PassCount, x, Count );
      //        if ( Errors[ 4 ] && Errors[ 4 ]->GetBufferPointer() )
      //          DEBUGLOG( "%s\n", Errors[ 4 ]->GetBufferPointer() );
      //        else DEBUGLOG( "ERROR BLOB IS NULL\n" );
      //      }
      //      else
      //        DEBUGLOG( "Compiling Domain   Shader %d/%d in Technique %d/%d OK.", y, Techniques[ x ].PassCount, x, Count );
      //#endif

  }

    if ( DoPrecalc( 0.5f + 0.15f*x / (float)Count ) ) return;
}

  DEBUGLOG( "Loading Materials" );

  //////////////////////////////////////////////////////////////////////////
  // load materials
  Count = readbyte( Stream_Main ); //material count
  Materials = new CphxMaterial[ Count ];

  for ( int x = 0; x < Count; x++ )
  {
    //Materials[x].PassCount = 0; //zero already due to new[]
    Materials[ x ].TechCount = readbyte( Stream_Main );
    Materials[ x ].Techniques = new CphxMaterialTechnique*[ Materials[ x ].TechCount ];
    for ( int y = 0; y < Materials[ x ].TechCount; y++ )
    {
      Materials[ x ].Techniques[ y ] = &Techniques[ readbyte( Stream_Main ) ];
      Materials[ x ].PassCount += Materials[ x ].Techniques[ y ]->PassCount;
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // load treespecies

  Count = readbyte( Stream_Main ); //treespecies count

#ifdef HAS_ARBARO
  TreeSpecies = new TREESPECIESDESCRIPTOR[ Count ];

  for ( int x = 0; x < Count; x++ )
  {
    TREESPECIESDESCRIPTOR &s = TreeSpecies[ x ];
    memcpy( &s, datastreams[ Stream_Main ], sizeof( s ) );
    datastreams[ Stream_Main ] += sizeof( s );
  }
#endif

  //////////////////////////////////////////////////////////////////////////
  // load models
  DEBUGLOG( "Loading MiniMeshes" );
  Count = readbyte( Stream_ModelParameters ); //model count

#ifdef PHX_MESH_LOADSTOREDMINIMESH
  struct MiniMesh
  {
    int vxDataSize;
    int triDataSize;
    unsigned char* vxData;
    unsigned char* triData;
  };

  MiniMesh miniMeshes[ 16 ];

  for ( int x = 0; x < Count; x++ )
  {
    miniMeshes[ x ].vxDataSize = (unsigned short)readshort( Stream_ModelParameters );
    miniMeshes[ x ].triDataSize = (unsigned short)readshort( Stream_ModelParameters );
    miniMeshes[ x ].vxData = datastreams[ Stream_ModelParameters ];
    datastreams[ Stream_ModelParameters ] += miniMeshes[ x ].vxDataSize;
    miniMeshes[ x ].triData = datastreams[ Stream_ModelParameters ];
    datastreams[ Stream_ModelParameters ] += miniMeshes[ x ].triDataSize;
  }
#endif

  DEBUGLOG( "Loading Models" );

  Count = readbyte( Stream_Main ); //model count
  Models = new CphxModel[ Count ];
  for ( int x = 0; x < Count; x++ )
  {
    unsigned char objectcount = readbyte( Stream_Main );
    Models[ x ].Objects.Array = new CphxModelObject*[ objectcount ];
    Models[ x ].Objects.ItemCount = objectcount;
    for ( int y = 0; y < objectcount; y++ )
    {
      PHXMESHPRIMITIVE primitive = (PHXMESHPRIMITIVE)( readbyte( Stream_ModelPrimitives ) );

      CphxModelObject *o;
      CphxModelObject_Mesh *m;
      CphxModelObject_Clone *c;

      if ( primitive == Mesh_Clone )
        o = c = new CphxModelObject_Clone;
      else
        o = m = new CphxModelObject_Mesh;

      o->Primitive = primitive;
      Models[ x ].Objects[ y ] = o;

      //unsigned char matrixdata = readbyte(Stream_ModelParameters);

      //set identity
      //o->TransformationF16[0] = o->TransformationF16[4] = o->TransformationF16[8] = 1;

      //if (matrixdata & 0x01) //read orientation
      //{
      //	memcpy(o->TransformationF16, datastreams[Stream_ModelTransformation], 18); //orientation
      //	datastreams[Stream_ModelTransformation] += 18;
      //}

      //if (matrixdata >> 0x01) //read position
      //{
      //	memcpy(o->TransformationF16+9, datastreams[Stream_ModelTransformation], 6); //position
      //	datastreams[Stream_ModelTransformation] += 6;
      //}

      memcpy( o->TransformationF16, datastreams[ Stream_ModelTransformation ], 24 ); //transformation
      datastreams[ Stream_ModelTransformation ] += 24;

      if ( primitive != Mesh_Clone ) //mesh parameters
      {
        memcpy( m->MeshParameters, datastreams[ Stream_ModelParameters ], MeshPrimitiveParameterCounts[ primitive ] );
        datastreams[ Stream_ModelParameters ] += MeshPrimitiveParameterCounts[ primitive ];
      }

      switch ( primitive )
      {
#ifdef PHX_MESH_CREATETEXT
      case Mesh_Text:
        m->Text = (char*)skiptext( Stream_ASCIIZ );
        break;
#endif
#ifdef PHX_MESH_LOADSTOREDMESH
      case Mesh_Stored:
      {
        int vxCnt = readdword( Stream_ModelParameters );
        m->StoredVertexCount = vxCnt;
        m->StoredVertices = new CphxVertex[ vxCnt ];
        for ( int z = 0; z < vxCnt; z++ )
        {
          m->StoredVertices[ z ].Position.x = readfloat32( Stream_ModelParameters );
          m->StoredVertices[ z ].Position.y = readfloat32( Stream_ModelParameters );
          m->StoredVertices[ z ].Position.z = readfloat32( Stream_ModelParameters );
          m->StoredVertices[ z ].Normal.x = readfloat32( Stream_ModelParameters );
          m->StoredVertices[ z ].Normal.y = readfloat32( Stream_ModelParameters );
          m->StoredVertices[ z ].Normal.z = readfloat32( Stream_ModelParameters );
        }
        int pCnt = readdword( Stream_ModelParameters );
        m->StoredPolyCount = pCnt;
        m->StoredPolygons = new CphxPolygon[ pCnt ];
        for ( int z = 0; z < pCnt; z++ )
        {
          unsigned char cnt = m->StoredPolygons[ z ].VertexCount = readbyte( Stream_ModelParameters );
          for ( int i = 0; i < cnt; i++ )
          {
            m->StoredPolygons[ z ].VertexIDs[ i ] = readdword( Stream_ModelParameters );
            m->StoredPolygons[ z ].Normals[ i ].x = readfloat32( Stream_ModelParameters );
            m->StoredPolygons[ z ].Normals[ i ].y = readfloat32( Stream_ModelParameters );
            m->StoredPolygons[ z ].Normals[ i ].z = readfloat32( Stream_ModelParameters );
            m->StoredPolygons[ z ].Texcoords[ i ][ 0 ].x = readfloat32( Stream_ModelParameters );
            m->StoredPolygons[ z ].Texcoords[ i ][ 0 ].y = readfloat32( Stream_ModelParameters );
          }
        }
      }
      break;
#endif
#ifdef PHX_MESH_LOADSTOREDMINIMESH
      case Mesh_StoredMini:
      {
        unsigned char miniMeshIndex = readbyte( Stream_ModelParameters );
        m->StoredVertexCount = miniMeshes[ miniMeshIndex ].vxDataSize;
        m->StoredPolyCount = miniMeshes[ miniMeshIndex ].triDataSize;
        m->StoredVertices = (CphxVertex*)miniMeshes[ miniMeshIndex ].vxData; // this is a hack to not need another variable
        m->StoredPolygons = (CphxPolygon*)miniMeshes[ miniMeshIndex ].triData; // this is a hack to not need another variable
      }
      break;
#endif
#ifdef PHX_MESH_COPY
      case Mesh_Copy:
        o->parentobjcount = 1;
        break;
#endif
#ifdef PHX_MESH_SCATTER
      case Mesh_Scatter:
#endif
#ifdef PHX_MESH_LOFT
      case Mesh_Loft:
#endif
#if defined(PHX_MESH_SCATTER) || defined(PHX_MESH_LOFT)
        o->parentobjcount = 2;
        break;
#endif
      case Mesh_Clone:
#ifdef PHX_MESH_MERGE
      case Mesh_Merge:
#endif
        o->parentobjcount = readbyte( Stream_ModelReferences );
        break;
      }

      o->parentobjects = datastreams[ Stream_ModelReferences ];
      datastreams[ Stream_ModelReferences ] += o->parentobjcount;

      if ( !(
#ifdef PHX_MESH_LINE
        primitive == Mesh_Line ||
#endif
#ifdef PHX_MESH_ARC
        primitive == Mesh_Arc ||
#endif
#ifdef PHX_MESH_SPLINE
        primitive == Mesh_Spline ||
#endif				  
        primitive == Mesh_Clone ) ) //these have no material data
      {
        ImportVariableMaterialData( this, m->Material, m->MaterialState );
      }

      int fcount = readbyte( Stream_ModelFilterCount );

      if ( fcount )
      {
        m->FilterCount = fcount;

        m->FilterData = new PHXMESHFILTERDATA[ m->FilterCount ];

        for ( int z = 0; z < m->FilterCount; z++ )
        {
          m->FilterData[ z ].Type = (PHXMESHFILTER)( readbyte( Stream_ModelFilterType ) );
          m->FilterData[ z ].FilterParams = datastreams[ Stream_ModelFilterData ];
          datastreams[ Stream_ModelFilterData ] += MeshFilterParameterCounts[ m->FilterData[ z ].Type ];

          PHXDATASTREAMS SrcStream = Stream_ModelTransformation;

#ifdef PHX_MESH_CALCULATETINT
          if ( m->FilterData[ z ].Type == ModelFilter_TintMesh )
            SrcStream = Stream_TextureOperatorReference;
#endif

          m->FilterData[ z ].filtertransform = (D3DXFLOAT16*)datastreams[ SrcStream ];
          datastreams[ SrcStream ] += MeshFilterExtraDataSize[ m->FilterData[ z ].Type ];
        }
      }
    }

    bool done = false;

    DEBUGLOG( "--- Generating Meshes for Model %d/%d", x, Count );

    //mesh generation - no import done during this
    while ( !done )
    {
      done = true;
      for ( int y = 0; y < objectcount; y++ )
      {
        CphxModelObject *o = Models[ x ].Objects.Array[ y ];
        if ( o->minimalGenerated ) continue;

        if ( o->Primitive == Mesh_Clone )
        {
          CphxModelObject_Clone *c = (CphxModelObject_Clone*)o;
          for ( int z = 0; z < c->parentobjcount; z++ )
            c->ClonedObjects.Add( Models[ x ].Objects.Array[ c->parentobjects[ z ] ] );

          o->minimalGenerated = true;
          continue;
        }

        done = false;

        bool parentsok = true;
        for ( int z = 0; z < o->parentobjcount; z++ )
          parentsok &= Models[ x ].Objects.Array[ o->parentobjects[ z ] ]->minimalGenerated;

        CphxModelObject_Mesh *m = (CphxModelObject_Mesh*)o;

#ifdef PHX_MESH_CSG
        for ( int z = 0; z < m->FilterCount; z++ )
          if ( m->FilterData[ z ].Type == ModelFilter_CSG )
            parentsok &= Models[ x ].Objects[ m->FilterData[ z ].FilterParams[ 0 ] ]->minimalGenerated;
#endif

        if ( !parentsok ) continue;

        o->minimalGenerated = true;

        DEBUGLOG( "Generating Mesh %d/%d", y, objectcount );

        GenerateMesh( o, &Models[ x ] );

        //apply filters

        for ( int z = 0; z < m->FilterCount; z++ )
        {
          unsigned char *filterparams = m->FilterData[ z ].FilterParams;
          D3DXFLOAT16 *filtertransform = m->FilterData[ z ].filtertransform;

          switch ( m->FilterData[ z ].Type )
          {
          case ModelFilter_UVMap:
            m->Mesh.CalculateTextureCoordinates( (PHXTEXTUREMAPTYPE)filterparams[ 0 ], filterparams[ 1 ] & 0x0f, ( filterparams[ 1 ] & 0xf0 ) != 0,
                                                 D3DXVECTOR3( filtertransform ) + *(D3DXVECTOR3*)one,
                                                 D3DXQUATERNION( filtertransform + 3 ),
                                                 D3DXVECTOR3( filtertransform + 7 ) );
            //data += 20;
            break;
#ifdef PHX_MESH_MAPXFORM
          case ModelFilter_MapXForm:
            m->Mesh.MapXForm( D3DXVECTOR3( filtertransform ),
                              D3DXQUATERNION( filtertransform + 3 ),
                              D3DXVECTOR3( filtertransform + 7 ) );
            //data += 20;
            break;
#endif
#ifdef PHX_MESH_SMOOTH
          case ModelFilter_MeshSmooth:
            m->Mesh.CatmullClark( filterparams[ 0 ] != 0, filterparams[ 1 ] );
            break;
#endif
          case ModelFilter_SmoothGroup:
            m->Mesh.SmoothGroupSeparation = filterparams[ 0 ] / 255.0f*2.0f;
            m->Mesh.SkipNormalCalculation = false;
            break;
#ifdef PHX_MESH_CALCULATETINT
          case ModelFilter_TintMesh:
            //textureop = ((unsigned short*)data)[0]; //texture operator count
            //data += 2;
            m->Mesh.CalculateTint( filterparams[ 0 ], TextureOperators[ ( (unsigned short*)filtertransform )[ 0 ] ].CachedResult->Texture, filterparams[ 1 ] );
            break;
#endif
#ifdef PHX_MESH_CALCULATETINTSHAPE
          case ModelFilter_TintMeshShape:
            m->Mesh.CalculateTintShape( filterparams[ 0 ], filterparams[ 1 ], filterparams[ 2 ],
                                        D3DXVECTOR3( filtertransform ),
                                        D3DXQUATERNION( filtertransform + 3 ),
                                        D3DXVECTOR3( filtertransform + 7 ) );
            //data += 20;
            break;
#endif
#ifdef PHX_MESH_BEVEL
          case ModelFilter_Bevel:
            m->Mesh.Bevel( filterparams[ 0 ] / 255.0f );
            break;
#endif
#ifdef PHX_MESH_REPLICATE
          case ModelFilter_Replicate:
            m->Mesh.Replicate( filterparams[ 0 ], filtertransform );
            //data += 24;
            break;
#endif
#ifdef PHX_MESH_NORMALDEFORM
          case ModelFilter_NormalDeform:
            m->Mesh.NormalDeform( filterparams[ 0 ] / 255.0f*( filterparams[ 1 ] - 127 ) );
            //data += 24;
            break;
#endif
#ifdef PHX_MESH_CSG
          case ModelFilter_CSG:
          {
            D3DXMATRIX currObjMatInv;
            D3DXMatrixInverse( &currObjMatInv, nullptr, &m->GetMatrix() );

            D3DXMATRIX result;
            D3DXMatrixMultiply( &result, &Models[ x ].Objects[ filterparams[ 0 ] ]->GetMatrix(), &currObjMatInv );
            m->Mesh.CSG( &( (CphxModelObject_Mesh*)Models[ x ].Objects[ filterparams[ 0 ] ] )->Mesh, &result, filterparams[ 1 ] );
          }
            //data += 24;
            break;
#endif
#ifdef PHX_MESH_GREEBLE
          case ModelFilter_Greeble:
            m->Mesh.Greeble( filterparams[ 0 ], filterparams[ 1 ] / 255.0f, filterparams[ 2 ] / 255.0f );
            //data += 24;
            break;
#endif
          }

        }

        //build mesh
        PHXMESHPRIMITIVE prim = o->Primitive;

        if ( prim != Mesh_Clone
#ifdef PHX_MESH_ARC
             && prim != Mesh_Arc
#endif
#ifdef PHX_MESH_LINE
             && prim != Mesh_Line
#endif
#ifdef PHX_MESH_SPLINE
             && prim != Mesh_Spline
#endif
             )
          DEBUGLOG( "Building Mesh %d/%d", y, objectcount );

        m->Mesh.BuildMesh( m->VxBuffer, m->IndexBuffer, m->WireBuffer, m->VxCount, m->TriCount, m->EdgeCount );
        m->dataBuffer = m->Mesh.dataBuffer;
        m->dataBufferView = m->Mesh.dataBufferView;
      }
    }

    DEBUGLOG( "--- Mesh generation for Model %d/%d finished", x, Count );

    if ( DoPrecalc( 0.65f + 0.35f*x / (float)Count ) ) return;
  }

  //////////////////////////////////////////////////////////////////////////
  // load scenes

  DEBUGLOG( "Loading Scenes" );

  Count = readbyte( Stream_Main ); //scene count
  Scenes = new CphxScene[ Count ];
  for ( int x = 0; x < Count; x++ )
  {
    DEBUGLOG( "----Loading Scene %d/%d", x, Count );
    Scenes[ x ].LayerCount = RenderLayerCount;
    Scenes[ x ].RenderLayers = new CphxRenderLayer*[ RenderLayerCount ];
    for ( int y = 0; y < RenderLayerCount; y++ )
    {
      CphxRenderLayer *r = new CphxRenderLayer;
      Scenes[ x ].RenderLayers[ y ] = r;
      r->Descriptor = &RenderLayers[ y ];
    }

    unsigned char objectcount = readbyte( Stream_Main );
    unsigned char clipcount = readbyte( Stream_Main );

    Scenes[ x ].Objects = new CphxObject*[ objectcount ];
    Scenes[ x ].ObjectCount = objectcount;

    for ( int y = 0; y < objectcount; y++ )
    {
      DEBUGLOG( "Loading Object %d/%d", y, objectcount );
      CphxObject *o;
#ifdef PHX_OBJ_MODEL
      CphxObject_Model *m;
#endif
#ifdef PHX_OBJ_SUBSCENE
      CphxObject_SubScene *s;
#endif
#ifdef PHX_OBJ_EMITTERCPU
      CphxObject_ParticleEmitter_CPU *p;
#endif
#ifdef PHX_OBJ_PARTICLEGRAVITY
      CphxObject_ParticleGravity *g;
#endif
#ifdef PHX_OBJ_PARTICLEVORTEX
      CphxObject_ParticleVortex* v;
#endif
#ifdef PHX_OBJ_PARTICLEDRAG
      CphxObject_ParticleDrag *d;
#endif
#ifdef PHX_OBJ_PARTICLETURBULENCE
      CphxObject_ParticleTurbulence *t;
#endif
      o = new CphxObject;
      o->TargetID = -1;

      PHXOBJECT primitive = (PHXOBJECT)readbyte( Stream_Main );

      int parentid = readbyte( Stream_Main );

      switch ( primitive )
      {
      case Object_Model:
        DEBUGLOG( "Object is a model." );
#ifdef PHX_OBJ_MODEL
        o = m = new CphxObject_Model;
        o->TargetID = -1;
        m->Model = &Models[ readbyte( Stream_Main ) ];
#endif
        break;
      case Object_Light:
      {
        DEBUGLOG( "Object is a light." );
        bool pointlight = ( readbyte( Stream_Main ) ) != 0;

        if ( pointlight )
        {
          o->SplineResults[ Spline_Position_w ] = 1;
          o->TargetID = (char)( readbyte( Stream_Main ) ); //target object - CAREFUL FOR VALUES OF 255!!!!
        }
      }
      break;
      case Object_CamEye:
        DEBUGLOG( "Object is a camera." );
        o->TargetID = (unsigned char)readbyte( Stream_Main ); //target object
        break;
        //case Object_Dummy:
        //	break;
#ifdef PHX_OBJ_SUBSCENE
      case Object_SubScene:
        DEBUGLOG( "Object is a subscene." );
        o = s = new CphxObject_SubScene;
        //s->Scene = &Scenes[ (char)readbyte( Stream_Main ) ];
        break;
#endif

#ifdef PHX_OBJ_EMITTERCPU
      case Object_ParticleEmitterCPU:
      {
        DEBUGLOG( "Object is a particle emitter (cpu)." );
        o = p = new CphxObject_ParticleEmitter_CPU;
        unsigned char flags = readbyte( Stream_Main );
        p->Aging = flags & 1;
        p->RandRotate = ( flags >> 1 ) & 1;
        p->TwoDirRotate = ( flags >> 2 ) & 1;
        p->RotateToDirection = ( flags >> 3 ) & 1;
#ifdef PHX_HAS_PARTICLE_SORTING
        p->Sort = ( flags >> 4 ) & 1;
#endif
        p->BufferSize = readbyte( Stream_Main );
        p->EmitterType = readbyte( Stream_Main );
        p->InnerRadius = readbyte( Stream_Main );
        p->StartCount = readbyte( Stream_Main );
        p->RandSeed = readbyte( Stream_Main );
        o->TargetID = (char)readbyte( Stream_Main ); // BEWARE OF NULL TARGETS - THEY ARE REPRESENTED BY 255!

        char emittedobj = readbyte( Stream_Main );
        if ( emittedobj != -1 )
        {
#ifdef PHX_HAS_SUBSCENE_PARTICLES
          if ( ( flags >> 5 ) & 1 )
            p->SceneToEmit = &Scenes[ emittedobj ];
          else
#endif
#ifdef PHX_HAS_MESH_PARTICLES
            p->ObjectToEmit = &Models[ emittedobj ];
#endif
        }

        //import variable material params

        //p->Material = &Materials[readbyte(Stream_MaterialID)];
        //p->MaterialState = new CphxMaterialPassConstantState*[p->Material->PassCount];
        //for (int z = 0; z < p->Material->PassCount; z++)
        //	p->MaterialState[z] = new CphxMaterialPassConstantState;

#ifdef PHX_HAS_STANDARD_PARTICLES
        if ( emittedobj == -1 )
        {
          DEBUGLOG( "Importing material parameters." );
          ImportVariableMaterialData( this, p->Material, p->MaterialState );
        }
#endif

        //init emitter here...
        DEBUGLOG( "Initializing particle buffers for %d particles.", 1 << p->BufferSize );
        p->Particles = new PHXPARTICLE[ 1 << p->BufferSize ];
        p->DistanceBuffer = new PHXPARTICLEDISTANCE[ 1 << p->BufferSize ];

#ifdef PHX_HAS_STANDARD_PARTICLES
        D3D11_BUFFER_DESC bd;
        memset( &bd, 0, sizeof( bd ) );

        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = PARTICLESIZE*( 1 << p->BufferSize );
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.StructureByteStride = PARTICLESIZE;

        //HRESULT res = 
        DEBUGLOG( "Creating Particle Buffer of size %d", bd.ByteWidth );
#ifdef DEBUGINTOFILE
        HRESULT res =
#endif

          phxDev->CreateBuffer( &bd, NULL, &p->VertexBuffer );
#ifdef DEBUGINTOFILE
        if ( !p->VertexBuffer )
        {
          DEBUGLOG( "Failed to create particle buffer (HRESULT %x)", res );
          if ( res == DXGI_ERROR_DEVICE_REMOVED )
          {
            DEBUGLOG( "Error: Device removed. Reason: %x", phxDev->GetDeviceRemovedReason() );
          }
        }
        else DEBUGLOG( "Particle buffer created OK." );
#endif
#endif
      }
      break;
#endif // PHX_OBJ_EMITTERCPU
#ifdef PHX_OBJ_PARTICLEGRAVITY
      case Object_ParticleGravity:
        o = g = new CphxObject_ParticleGravity;
        g->AreaType = readbyte( Stream_Main );
        g->Directional = readbyte( Stream_Main ) != 0;
        break;
#endif
#ifdef PHX_OBJ_PARTICLEDRAG
      case Object_ParticleDrag:
        o = d = new CphxObject_ParticleDrag;
        d->AreaType = readbyte( Stream_Main );
        break;
#endif
#ifdef PHX_OBJ_PARTICLETURBULENCE
      case Object_ParticleTurbulence:
        o = t = new CphxObject_ParticleTurbulence;
        t->AreaType = readbyte( Stream_Main );
        //t->RandSeed = readbyte( Stream_Main );
        //t->Frequency = readfloat( Stream_Main );
        t->RandSeed = 0;
        t->calculatedKernelSeed = 1; // force calculation, randseed is 0
        t->InitKernel();
        break;
#endif
#ifdef PHX_OBJ_LOGICOBJECT
      case Object_LogicObject:
        o->camCenterX = readbyte( Stream_Main );
        o->camCenterY = readbyte( Stream_Main );
        break;
#endif
#ifdef PHX_OBJ_PARTICLEVORTEX
      case Object_ParticleVortex:
        o = v = new CphxObject_ParticleVortex;
        v->AreaType = readbyte( Stream_Main );
        break;
#endif
      default:
        DEBUGLOG( "Object is a dummy." );
        break;
      }

      o->Scene = &Scenes[ x ];
      o->ObjectType = primitive;
      o->minimportParentID = parentid;

      Scenes[ x ].Objects[ y ] = o;
      o->Clips = new CphxObjectClip*[ clipcount ];

      for ( int z = 0; z < clipcount; z++ )
      {
        o->Clips[ z ] = new CphxObjectClip;
#ifdef PHX_OBJ_SUBSCENE
        if ( primitive == Object_SubScene )
          s->Clips[ z ]->SubSceneTarget = &Scenes[ (unsigned char)readbyte( Stream_Main ) ];
#endif
#ifdef PHX_OBJ_PARTICLETURBULENCE
        if ( primitive == Object_ParticleTurbulence )
        {
          t->Clips[ z ]->RandSeed = (unsigned char)readbyte( Stream_Main );
          t->Clips[ z ]->TurbulenceFrequency = (unsigned char)readbyte( Stream_Main );
        }
#endif
      }

      DEBUGLOG( "Loading Clip Data" );

      for ( int z = 0; z < clipcount; z++ )
      {
        //o->Clips[z]->SplineCount = readbyte(Stream_Main)+7;
        o->Clips[ z ]->SplineCount = readbyte( Stream_Main );
        o->Clips[ z ]->Splines = new CphxClipSpline*[ o->Clips[ z ]->SplineCount ];
        o->Clips[ z ]->MaterialSplines = new CphxMaterialSplineBatch;

        //create default splines, the later imported splines should override their outputs during animation
        //for (int i = 0; i < 7; i++)
        //{
        //	CphxClipSpline *s = o->Clips[z]->Splines[i] = new CphxClipSpline;
        //	s->Spline = new CphxSpline_float16;
        //	if (i == 6)
        //	{
        //		s->Spline = new CphxSpline_Quaternion16;
        //		s->Spline->Value[3] = 1;
        //	}
        //	else
        //		if (i>3) s->Spline->Value[0] = 1;

        //	s->Type = DefaultSplines[i];
        //}

        //load stored splines
        //for (int i = 7; i < o->Clips[z]->SplineCount; i++)
        for ( int i = 0; i < o->Clips[ z ]->SplineCount; i++ )
        {
          o->Clips[ z ]->Splines[ i ] = new CphxClipSpline;
          ImportClipSpline( o->Clips[ z ]->Splines[ i ] );
        }

#ifdef PHX_OBJ_MODEL
        if ( primitive == Object_Model )
        {	//szoporoller
          ImportMaterialSplineBatch( m->Model, o->Clips[ z ], z );
        }
#endif

        if ( primitive == Object_ParticleEmitterCPU )
        {
#ifdef PHX_HAS_STANDARD_PARTICLES

#if defined PHX_HAS_MESH_PARTICLES || defined PHX_HAS_SUBSCENE_PARTICLES
          //szoporoller
          if (
#ifdef PHX_HAS_MESH_PARTICLES
            !p->ObjectToEmit
#endif
#ifdef PHX_HAS_SUBSCENE_PARTICLES
#ifdef PHX_HAS_MESH_PARTICLES
            &&
#endif
            !p->SceneToEmit
#endif
            )
#endif
            for ( int i = 0; i < p->Material->TechCount; i++ )
              ImportTechSplineBatch( p->Material->Techniques[ i ], NULL, o->Clips[ z ]->MaterialSplines, z );
#endif
#ifdef PHX_HAS_MESH_PARTICLES
          if ( p->ObjectToEmit )
            ImportMaterialSplineBatch( p->ObjectToEmit, o->Clips[ z ], z );
#endif
        }
      }

#ifdef PHX_HAS_STANDARD_PARTICLES
      if ( primitive == Object_ParticleEmitterCPU )
      {
        DEBUGLOG( "Updating Spline Texture Data" );
        p->UpdateSplineTexture();
        DEBUGLOG( "Spline Texture Data Updated." );
      }
#endif
    }

    //apply target objects and parents
    DEBUGLOG( "Apply Object Hierarchy" );
    for ( int y = 0; y < objectcount; y++ )
    {
      if ( Scenes[ x ].Objects[ y ]->minimportParentID != 0xff )
      {
        CphxObject *prnt = Scenes[ x ].Objects[ Scenes[ x ].Objects[ y ]->minimportParentID ];
        Scenes[ x ].Objects[ y ]->Parent = prnt;
        Scenes[ x ].Objects[ y ]->Scene = &Scenes[ x ];
        CphxObject **c = new CphxObject*[ prnt->ChildCount + 1 ];
        memcpy( c, prnt->Children, prnt->ChildCount * sizeof( CphxObject* ) );
        c[ prnt->ChildCount ] = Scenes[ x ].Objects[ y ];
        prnt->ChildCount++;
        prnt->Children = c;
      }

      if ( Scenes[ x ].Objects[ y ]->TargetID >= 0 )
        Scenes[ x ].Objects[ y ]->Target = Scenes[ x ].Objects[ Scenes[ x ].Objects[ y ]->TargetID ];
    }

    Scenes[ x ].UpdateSceneGraph( 0, 0 );
#ifdef PHX_OBJ_EMITTERCPU
    for ( int y = 0; y < objectcount; y++ )
    {
      if ( Scenes[ x ].Objects[ y ]->ObjectType == Object_ParticleEmitterCPU )
      {
        DEBUGLOG( "Reseting Particles for Scene %d Object %d", x, y );
        ( (CphxObject_ParticleEmitter_CPU*)Scenes[ x ].Objects[ y ] )->ResetParticles();
        DEBUGLOG( "Reset done" );
      }
    }
#endif
  }

  //////////////////////////////////////////////////////////////////////////
  // load timeline

  DEBUGLOG( "Loading Timeline" );

  Timeline.EventCount = readshort( Stream_Main ); //event count
  Timeline.Events = new CphxEvent*[ Timeline.EventCount ];

  for ( int x = 0; x < Timeline.EventCount; x++ )
  {
    PHXEVENTTYPE type = (PHXEVENTTYPE)( readbyte( Stream_EventType ) );

    int StartFrame = readshort( Stream_EventPosition );
    int EndFrame = readshort( Stream_EventPosition );
    CphxSpline_float16 *time = 0;

    if ( ( type >> 7 ) != 0 )
      ImportSpline( time = new CphxSpline_float16() );

    CphxEvent *e;

    switch ( type & 0x7f )
    {
#ifdef PHX_EVENT_ENDDEMO
    case EVENT_ENDDEMO:
      e = new CphxEvent_EndDemo;
      break;
#endif
#ifdef PHX_EVENT_RENDERSCENE
    case EVENT_RENDERSCENE:
    {
      CphxEvent_RenderScene *rs = new CphxEvent_RenderScene;
      e = rs;
      rs->Scene = &Scenes[ readbyte( Stream_Main ) ];
      char camID = readbyte( Stream_Main );
      rs->Camera = camID == -1 ? NULL : rs->Scene->Objects[ camID ];
      rs->Clip = readbyte( Stream_Main );
      rs->ClearColor = ( ( datastreams[ Stream_Main ][ 0 ] ) >> 1 ) != 0;
      rs->ClearZ = ( readbyte( Stream_Main ) ) & 0x1;
    }
    break;
#endif
#ifdef PHX_EVENT_PARTICLECALC
    case EVENT_PARTICLECALC:
    {
      CphxEvent_ParticleCalc *pc = new CphxEvent_ParticleCalc;
      e = pc;
      pc->Scene = &Scenes[ readbyte( Stream_Main ) ];
      //pc->Camera = pc->Scene->Objects[ readbyte( Stream_Main ) ];
      pc->Clip = readbyte( Stream_Main );
    }
    break;
#endif
#ifdef PHX_EVENT_SHADERTOY
    case EVENT_SHADERTOY:
    {
      CphxEvent_Shadertoy *st = new CphxEvent_Shadertoy;
      e = st;

      st->Tech = &Techniques[ readbyte( Stream_Main ) ];

      st->MaterialSplines = new CphxMaterialSplineBatch;
      st->MaterialState = new CphxMaterialPassConstantState*[ st->Tech->PassCount ];
      for ( int z = 0; z < st->Tech->PassCount; z++ )
        st->MaterialState[ z ] = new CphxMaterialPassConstantState;

      int passid = 0;
      ImportTechniqueData( this, st->Tech, passid, st->MaterialState );
      ImportTechSplineBatch( st->Tech, NULL, st->MaterialSplines, 0 );
    }
    break;
#endif
#ifdef PHX_EVENT_CAMSHAKE
    case EVENT_CAMERASHAKE:
    {
      CphxEvent_CameraShake *st = new CphxEvent_CameraShake;
      e = st;
      st->ShakesPerSec = readbyte( Stream_Main );
      ImportSpline( st->EyeIntensity = new CphxSpline_float16() );
      ImportSpline( st->TargetIntensity = new CphxSpline_float16() );
    }
    break;
#endif
#ifdef PHX_EVENT_CAMOVERRIDE
    case EVENT_CAMERAOVERRIDE:
    {
      CphxEvent_CameraOverride* pc = new CphxEvent_CameraOverride;
      e = pc;
      pc->Scene = &Scenes[ readbyte( Stream_Main ) ];
      pc->Camera = pc->Scene->Objects[ readbyte( Stream_Main ) ];
      pc->Clip = readbyte( Stream_Main );
    }
    break;
#endif
    }

    Timeline.Events[ x ] = e;

    e->StartFrame = StartFrame;
    e->EndFrame = EndFrame;

    //int targetid = readbyte(Stream_EventTargets);
    //if (targetid >= 0)
    e->Target = Timeline.RenderTargets[ 0 ];
    e->Time = time;
  }

  DEBUGLOG( "Project Load Finished" );
}

void CphxProject::Render( unsigned int Frame )
{
  Timeline.Render( Frame / 1000.0f*Timeline.FrameRate, false, false );
}

void ImportMaterialParams( CphxProject *proj, CphxMaterialParameterBatch &Params )
{
  Params.ParamCount = readbyte( Stream_Main );
  Params.Parameters = new CphxMaterialParameter*[ Params.ParamCount ];

  for ( int x = 0; x < Params.ParamCount; x++ )
  {
    CphxMaterialParameter *p = new CphxMaterialParameter;
    Params.Parameters[ x ] = p;
    p->Scope = (MATERIALPARAMSCOPE)( readbyte( Stream_Main ) );
    p->Type = (MATERIALPARAMTYPE)( readbyte( Stream_Main ) );
    if ( p->Scope == PARAM_CONSTANT ) 
      ImportMaterialParamValue( proj, p );
  }
}

void ImportMaterialParamValue( CphxProject *proj, CphxMaterialParameter *Param )
{
  switch ( Param->Type )
  {
  case PARAM_FLOAT:
    Param->Value.Float = ( readbyte( Stream_MaterialParam ) ) / 255.0f;
    break;
  case PARAM_COLOR:
    for ( int x = 0; x < 4; x++ )
      Param->Value.Color[ x ] = ( readbyte( Stream_MaterialParam ) ) / 255.0f;
    break;
  case PARAM_TEXTURE0:
  case PARAM_TEXTURE1:
  case PARAM_TEXTURE2:
  case PARAM_TEXTURE3:
  case PARAM_TEXTURE4:
  case PARAM_TEXTURE5:
  case PARAM_TEXTURE6:
  case PARAM_TEXTURE7:
  {
    int textureop = readshort( Stream_TextureOperatorReference );

    Param->Value.TextureView = NULL; //can't optimize this out as the param is a reused resource
    if ( textureop == 0xffff ) break;

    if ( textureop & 0x8000 )
      Param->Value.TextureView = proj->Timeline.RenderTargets[ textureop & 0x7fff ]->View;
    else
      Param->Value.TextureView = proj->TextureOperators[ textureop ].CachedResult->View;

    break;
  }
  case PARAM_RENDERTARGET:
  {
    int textureop = readshort( Stream_TextureOperatorReference );

    Param->Value.RenderTarget = NULL; //can't optimize this out as the param is a reused resource
    if ( textureop == 0xffff ) break;

    if ( textureop & 0x8000 )
      Param->Value.RenderTarget = proj->Timeline.RenderTargets[ textureop & 0x7fff ];
    break;
  }
  default:
    Param->Value.ZMode = readbyte( Stream_MaterialParam );
  case PARAM_DEPTHTEXTURE7:
  case PARAM_LTC1:
  case PARAM_LTC2:
#ifdef PHX_VOLUMETRIC_RENDERTARGETS
  case PARAM_3DTEXTURE6:
#endif
    break;
  }
}

void ImportTechniqueData( CphxProject* proj, CphxMaterialTechnique* t, int& passid, CphxMaterialPassConstantState** state )
{
  for ( int y = 0; y < t->Parameters.ParamCount; y++ )
    if ( t->Parameters.Parameters[ y ]->Scope == PARAM_VARIABLE )
      ImportMaterialParamValue( proj, t->Parameters.Parameters[ y ] );

  for ( int y = 0; y < t->PassCount; y++ )
  {
    CphxMaterialRenderPass* p = t->RenderPasses[ y ];
    for ( int z = 0; z < p->Parameters.ParamCount; z++ )
      if ( p->Parameters.Parameters[ z ]->Scope == PARAM_VARIABLE )
        ImportMaterialParamValue( proj, p->Parameters.Parameters[ z ] );

    BuildPassState( t, p, state[ passid++ ] );
  }
}

void ImportVariableMaterialData( CphxProject *proj, CphxMaterial *&mat, CphxMaterialPassConstantState **&state )
{
  mat = &( proj->Materials[ readbyte( Stream_MaterialID ) ] );
  state = new CphxMaterialPassConstantState*[ mat->PassCount ];
  for ( int z = 0; z < mat->PassCount; z++ )
    state[ z ] = new CphxMaterialPassConstantState;

  int passid = 0;
  for ( int x = 0; x < mat->TechCount; x++ )
    ImportTechniqueData( proj, mat->Techniques[ x ], passid, state );
}

void ImportClipSpline( CphxClipSpline *Spline )
{
  Spline->Type = (PHXSPLINETYPE)( readbyte( Stream_Main ) );

  //if (Spline->Type == Spline_MaterialParam)
  //{
  //	//todo: import material parameter link data
  //}

  CphxSpline *s;

  if ( Spline->Type == Spline_Rotation )
    s = new CphxSpline_Quaternion16;
  else
    s = new CphxSpline_float16;

  Spline->Spline = s;
  ImportSpline( s );
}

//void ImportSpline( CphxSpline_Quaternion16 *Spline )
//{
//  unsigned char c = readbyte( Stream_SplineDescriptors );
//
//  Spline->Loop = ( c >> 2 ) & 1; //Spline->Loop = (readbyte(Stream_SplineDescriptors)) != 0;
//  Spline->Interpolation = (SPLINEINTERPOLATION)( c & 0x03 );//Spline->Interpolation = (SPLINEINTERPOLATION)(readbyte(Stream_SplineDescriptors));
//
//  //Spline->Loop = (readbyte(Stream_SplineDescriptors)) != 0;
//  //Spline->Interpolation = (SPLINEINTERPOLATION)(readbyte(Stream_SplineDescriptors));
//
//  Spline->KeyCount = readbyte( Stream_SplineKeyCounts );
//
//  if ( !Spline->KeyCount )
//  {
//    for ( int x = 0; x < 4; x++ )
//      Spline->Value[ x ] = readfloat( Stream_SplineKeyValues );
//    return;
//  }
//
//  Spline->Keys = new CphxSplineKey*[ Spline->KeyCount ];
//
//  for ( int x = 0; x < Spline->KeyCount; x++ )
//  {
//    Spline->Keys[ x ] = new CphxSplineKey;
//    Spline->Keys[ x ]->t = readbyte( Stream_SplineKeyTimes );
//    for ( int y = 0; y < 4; y++ )
//      Spline->Keys[ x ]->Value[ y ] = readfloat( Stream_SplineKeyValues );
//  }
//}
//
void ImportSpline( CphxSpline *Spline ) // reads both float16 and quaternion splines
{
  unsigned char c = readbyte( Stream_SplineDescriptors );

  DEFAULTSPLINETYPE d = (DEFAULTSPLINETYPE)( c & 0x03 );

  if ( d != DEFAULTSPLINE_NOT_DEFAULT )
  {
    if ( d == DEFAULTSPLINE_ZERO )
      Spline->Value[ 3 ] = 1; // quaternion fix, the rest is zero

    if ( d == DEFAULTSPLINE_ONE )
      Spline->Value[ 0 ] = 1; // doesn't apply to quaternions

    if ( d == DEFAULTSPLINE_CONSTANTVALUE )
      for ( int x = 0; x < Spline->GetKeyFloatCount(); x++ )
        Spline->Value[ x ] = readfloat( Stream_SplineKeyValues );

    return;
  }

  Spline->Loop = ( c & ( 1 << 4 ) ) != 0;
  Spline->Interpolation = (SPLINEINTERPOLATION)( ( c >> 2 ) & 0x03 );

#if defined SPLINE_WAVEFORM_SIN || defined SPLINE_WAVEFORM_SQUARE || defined SPLINE_WAVEFORM_TRIANGLE || defined SPLINE_WAVEFORM_SAWTOOTH || defined SPLINE_WAVEFORM_NOISE
  Spline->Waveform = (SPLINEWAVEFORM)( ( c >> 5 ) & 0x07 );
  if ( Spline->Waveform != WAVEFORM_NONE )
  {
    Spline->MultiplicativeWaveform = ( readbyte( Stream_SplineDescriptors ) ) != 0;
    *(unsigned int*)&Spline->WaveformAmplitude = readdword( Stream_SplineDescriptors ); // reads frequency as well
    //Spline->WaveformFrequency = readfloat( Stream_SplineDescriptors );
    if ( Spline->Waveform == WAVEFORM_NOISE )
      Spline->RandSeed = readbyte( Stream_SplineDescriptors );
  }
#endif

  Spline->KeyCount = readbyte( Stream_SplineKeyCounts );

#ifdef SPLINE_HASZEROKEYEXPORT
  if ( !Spline->KeyCount )
  {
    for ( int x = 0; x < Spline->GetKeyFloatCount(); x++ )
      Spline->Value[ x ] = readfloat( Stream_SplineKeyValues );
    return;
  }
#endif

  Spline->Keys = new CphxSplineKey*[ Spline->KeyCount ];

  for ( int x = 0; x < Spline->KeyCount; x++ )
  {
    Spline->Keys[ x ] = new CphxSplineKey;
    Spline->Keys[ x ]->t = readbyte( Stream_SplineKeyTimes );

    for ( int y = 0; y < Spline->GetKeyFloatCount(); y++ )
      Spline->Keys[ x ]->Value[ y ] = readfloat( Stream_SplineKeyValues );

    if ( Spline->Interpolation == INTERPOLATION_BEZIER )
    {
      *(unsigned short*)Spline->Keys[ x ]->controlpositions = readshort( Stream_SplineKeyTimes );
      *(unsigned int*)Spline->Keys[ x ]->controlvalues = readdword( Stream_SplineKeyValues );
    }
  }

  Spline->CalculateValue( 0 ); //to force noise calc
}

void GenerateMesh( CphxModelObject *o, CphxModel *model )
{
  CphxModelObject_Mesh *m = (CphxModelObject_Mesh*)o;
  unsigned char *Data = m->MeshParameters;
  m->Mesh.SkipNormalCalculation = false;

  static CphxModelObject_Mesh *p[ 255 ];
  for ( int x = 0; x < o->parentobjcount; x++ )
    p[ x ] = (CphxModelObject_Mesh*)model->Objects[ o->parentobjects[ x ] ];
  //if ( o->parentobjcount > 0 ) p[ 0 ] = (CphxModelObject_Mesh *)model->Objects[ o->parentobjects[ 0 ] ];
  //if ( o->parentobjcount > 1 ) p[ 1 ] = (CphxModelObject_Mesh *)model->Objects[ o->parentobjects[ 1 ] ];

  switch ( o->Primitive )
  {
#ifdef PHX_MESH_CUBE
  case Mesh_Cube:
    m->Mesh.CreateCube();
    break;
#endif
#ifdef PHX_MESH_CREATEGEOSPHERE
  case Mesh_GeoSphere:
    m->Mesh.CreateGeoSphere( Data[ 0 ] );
    break;
#endif
#ifdef PHX_MESH_PLANE
  case Mesh_Plane:
    m->Mesh.CreatePlane( Data[ 0 ], Data[ 1 ] );
    break;
#endif
#ifdef PHX_MESH_SPHERE
  case Mesh_Sphere:
    m->Mesh.CreateSphere( Data[ 0 ], Data[ 1 ], ( Data[ 2 ] + 1 ) / 256.0f, Data[ 3 ] / 256.0f, Data[ 4 ] != 0 );
    break;
#endif
#ifdef PHX_MESH_CYLINDER
  case Mesh_Cylinder:
    m->Mesh.CreateCylinder( Data[ 0 ], Data[ 1 ], Data[ 2 ] != 0 );
    break;
#endif
#ifdef PHX_MESH_CONE
  case Mesh_Cone:
    m->Mesh.CreateCone( Data[ 0 ], Data[ 1 ], ( Data[ 2 ] + 1 ) / 256.0f, Data[ 3 ] != 0 );
    break;
#endif
#ifdef PHX_MESH_ARC
  case Mesh_Arc:
    m->Mesh.CreateArc( Data[ 0 ], ( Data[ 1 ] ) / 255.0f, Data[ 2 ] != 0 );
    break;
#endif
#ifdef PHX_MESH_LINE
  case Mesh_Line:
    m->Mesh.CreateLine( Data[ 0 ] );
    break;
#endif
#ifdef PHX_MESH_SPLINE
  case Mesh_Spline:
    //m->Mesh.Spline(NULL, 0, Data[0], false);
    break;
#endif
#ifdef PHX_MESH_LOFT
  case Mesh_Loft:
    m->Mesh.Loft( &p[ 0 ]->Mesh,
                  &p[ 0 ]->GetMatrix(),
                  &p[ 1 ]->Mesh,
                  &p[ 1 ]->GetMatrix(),
                  Data[ 2 ] != 0,
                  Data[ 3 ] != 0,
                  Data[ 4 ] ? ( Data[ 4 ] + 1 ) / 32.0f : 0,
                  Data[ 5 ] / 255.0f,
                  Data[ 6 ] / 255.0f );
    break;
#endif
#ifdef PHX_MESH_SCATTER
  case Mesh_Scatter:
    m->Mesh.Scatter( &p[ 0 ]->Mesh,
                     &p[ 0 ]->GetMatrix(),
                     &p[ 1 ]->Mesh,
                     &p[ 1 ]->GetMatrix(),
                     Data[ 2 ],
                     Data[ 3 ] / 255.0f,
                     Data[ 4 ] / 255.0f,
                     Data[ 5 ] / 255.0f,
                     Data[ 6 ],
                     Data[ 7 ],
                     (PHXSCATTERORIENTATIONTYPE)Data[ 8 ],
                     Data[ 9 ] / 255.0f,
                     Data[ 10 ] != 0,
                     *( (D3DXFLOAT16*)( &Data[ 12 ] ) ),//FloatParameter,
                     Data[ 11 ] );
    break;
#endif
#ifdef PHX_MESH_LOADSTOREDMESH
  case Mesh_Stored:
    m->Mesh.LoadStoredMesh( m->StoredVertices, m->StoredVertexCount, m->StoredPolygons, m->StoredPolyCount );
    break;
#endif
#ifdef PHX_MESH_LOADSTOREDMINIMESH
  case Mesh_StoredMini:
    m->Mesh.LoadStoredMiniMesh( (unsigned char*)m->StoredVertices, m->StoredVertexCount / 3, (unsigned char*)m->StoredPolygons, m->StoredPolyCount / 3 );
    break;
#endif
#ifdef PHX_MESH_CREATETREE
  case Mesh_Tree:
  {
    m->Mesh.CreateTree( Data[ 1 ], Data + 2, TreeSpecies[ Data[ 0 ] ] );
    break;
  }
#endif
#ifdef PHX_MESH_CREATETREELEAVES
  case Mesh_TreeLeaves:
  {
    m->Mesh.CreateTreeLeaves( Data[ 1 ], Data + 2, TreeSpecies[ Data[ 0 ] ] );
    break;
  }
#endif
#ifdef PHX_MESH_CREATETEXT
  case Mesh_Text:
  {
    m->Mesh.CreateText( Data[ 0 ], m->Text, Data[ 1 ] );
    break;
  }
#endif
#ifdef PHX_MESH_COPY
  case Mesh_Copy:
    m->Mesh.Copy( &p[ 0 ]->Mesh );
    break;
#endif
#ifdef PHX_MESH_MERGE
  case Mesh_Merge:
    m->Mesh.Merge( p, o->parentobjcount );
    break;
#endif
  default:
    break;
  }
}

void BuildPassState( CphxMaterialTechnique *Tech, CphxMaterialRenderPass *Pass, CphxMaterialPassConstantState* p )
{
  D3D11_RASTERIZER_DESC RasterDesc;
  D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
  D3D11_BLEND_DESC BlendDesc;
  memset( &RasterDesc, 0, sizeof( RasterDesc ) );
  memset( &DepthStencilDesc, 0, sizeof( DepthStencilDesc ) );
  memset( &BlendDesc, 0, sizeof( BlendDesc ) );
  RasterDesc.FillMode = D3D11_FILL_SOLID;
  RasterDesc.CullMode = D3D11_CULL_BACK;
  RasterDesc.DepthClipEnable = true;
  RasterDesc.AntialiasedLineEnable = true;

  if ( Tech->TargetLayer->VoxelizerLayer )
  {
    RasterDesc.DepthClipEnable = false;
  }

  DepthStencilDesc.DepthEnable = true;
  DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  for ( int z = 0; z < 8; z++ ) BlendDesc.RenderTarget[ z ] = blenddesc;
  p->RenderPriority = 127;

  CollectRenderState( Tech->Parameters, RasterDesc, DepthStencilDesc, BlendDesc, p );
  CollectRenderState( Pass->Parameters, RasterDesc, DepthStencilDesc, BlendDesc, p );

  phxDev->CreateBlendState( &BlendDesc, &p->BlendState );
  phxDev->CreateRasterizerState( &RasterDesc, &p->RasterizerState );
  phxDev->CreateDepthStencilState( &DepthStencilDesc, &p->DepthStencilState );
  p->ConstantDataSize *= 4; //hack to convert float count to data size
}

void CollectRenderState( CphxMaterialParameterBatch &Params, D3D11_RASTERIZER_DESC &RasterDesc, D3D11_DEPTH_STENCIL_DESC &DepthStencilDesc, D3D11_BLEND_DESC &BlendDesc, CphxMaterialPassConstantState *pass )
{
  for ( int x = 0; x < Params.ParamCount; x++ )
  {
    MATERIALVALUE &v = Params.Parameters[ x ]->Value;
    switch ( Params.Parameters[ x ]->Type )
    {
    case PARAM_FLOAT:
      if ( Params.Parameters[ x ]->Scope != PARAM_ANIMATED )
        pass->ConstantData[ pass->ConstantDataSize++ ] = v.Float;
      break;
    case PARAM_COLOR:
      if ( Params.Parameters[ x ]->Scope != PARAM_ANIMATED )
        for ( int y = 0; y < 4; y++ )
          pass->ConstantData[ pass->ConstantDataSize++ ] = v.Color[ y ];
      break;
    case PARAM_ZMODE:
      DepthStencilDesc.DepthEnable = !( v.ZMode & 1 ); //default zero means z enabled
      DepthStencilDesc.DepthWriteMask = !( ( v.ZMode >> 1 ) & 1 ) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO; //default zero means z write enabled
      break;
    case PARAM_ZFUNCTION:
      DepthStencilDesc.DepthFunc = v.ZFunction;
      break;
    case PARAM_TEXTURE0:
    case PARAM_TEXTURE1:
    case PARAM_TEXTURE2:
    case PARAM_TEXTURE3:
    case PARAM_TEXTURE4:
    case PARAM_TEXTURE5:
    case PARAM_TEXTURE6:
    case PARAM_TEXTURE7:
      pass->Textures[ Params.Parameters[ x ]->Type - PARAM_TEXTURE0 ] = v.TextureView;
      break;
      /*
          case PARAM_MESHDATA0:
          case PARAM_MESHDATA1:
          case PARAM_MESHDATA2:
          case PARAM_MESHDATA3:
          case PARAM_MESHDATA4:
          case PARAM_MESHDATA5:
          case PARAM_MESHDATA6:
          case PARAM_MESHDATA7:
            pass->Textures[ Params.Parameters[ x ]->Type - PARAM_MESHDATA0 ] = v.TextureView;
            break;
      */
    case PARAM_RENDERTARGET:
      pass->RenderTarget = v.RenderTarget;
      break;
    case PARAM_DEPTHTEXTURE7:
      pass->Textures[ 7 ] = phxDepthBufferShaderView;
      break;
#ifdef LTC1
    case PARAM_LTC1:
      pass->Textures[ 4 ] = ltc1view;
      break;
#endif
#ifdef LTC2
    case PARAM_LTC2:
      pass->Textures[ 5 ] = ltc2view;
      break;
#endif
#ifdef PHX_VOLUMETRIC_RENDERTARGETS
    case PARAM_3DTEXTURE6:
      pass->Textures[ 6 ] = phxTexture3DResourceView;
      break;
#endif
    case PARAM_BLENDMODE0:
    case PARAM_BLENDMODE1:
    case PARAM_BLENDMODE2:
    case PARAM_BLENDMODE3:
    case PARAM_BLENDMODE4:
    case PARAM_BLENDMODE5:
    case PARAM_BLENDMODE6:
    case PARAM_BLENDMODE7:
    {
      int z = Params.Parameters[ x ]->Type - PARAM_BLENDMODE0;
      BlendDesc.RenderTarget[ z ].BlendEnable = true;
      BlendDesc.RenderTarget[ z ].SrcBlend = (D3D11_BLEND)( ( v.BlendMode & 15 ) );
      BlendDesc.RenderTarget[ z ].DestBlend = (D3D11_BLEND)( ( v.BlendMode >> 4 ) );
    }
    break;
    case PARAM_CULLMODE:
      RasterDesc.CullMode = v.CullMode;
      break;
    case PARAM_FILLMODE:
      pass->Wireframe = v.Wireframe;
      break;
    case PARAM_RENDERPRIORITY:
      pass->RenderPriority = v.RenderPriority;
      break;

    default:
      break;
    }
  }
}

void ImportMaterialSplineBatch( CphxModel *m, CphxObjectClip *clip, int ClipID )
{
  for ( int x = 0; x < m->Objects.NumItems(); x++ )
  {
    CphxModelObject *o = m->Objects[ x ];
    if ( o->Primitive == Mesh_Clone ) continue;
    CphxMaterial *mat = ( (CphxModelObject_Mesh*)o )->Material;
    if ( mat )
      for ( int y = 0; y < mat->TechCount; y++ )
        ImportTechSplineBatch( mat->Techniques[ y ], o, clip->MaterialSplines, ClipID );
  }
}

void ImportTechSplineBatch( CphxMaterialTechnique *t, void *group, CphxMaterialSplineBatch *batch, int ClipID )
{
  ImportMaterialParamSplineBatch( &t->Parameters, group, batch, ClipID );
  for ( int x = 0; x < t->PassCount; x++ )
    ImportMaterialParamSplineBatch( &t->RenderPasses[ x ]->Parameters, group, batch, ClipID );
}

void ImportMaterialParamSplineBatch( CphxMaterialParameterBatch *b, void *group, CphxMaterialSplineBatch *batch, int ClipID )
{
  for ( int x = 0; x < b->ParamCount; x++ )
  {
    CphxMaterialParameter *p = b->Parameters[ x ];
    if ( p->Scope == PARAM_ANIMATED )
    {
      if ( p->Type == PARAM_FLOAT || p->Type == PARAM_COLOR || ( p->Type == PARAM_PARTICLELIFEFLOAT && !ClipID ) )
      {
        CphxMaterialSpline *s = new CphxMaterialSpline();
        s->GroupingData = group;
        s->Target = p;
        for ( int y = 0; y < 4; y++ )
          s->Splines[ y ] = new CphxSpline_float16;

        ImportSpline( s->Splines[ 0 ] );
        if ( p->Type == PARAM_COLOR )
          for ( int y = 1; y < 4; y++ )
            ImportSpline( s->Splines[ y ] );

        //HACK HACK HACK FOLLOWS, LEAKY DYNAMIC ARRAY INSTEAD OF PROPER SOLUTION OF KNOWING THE ARRAY SIZE BEFOREHAND

        CphxMaterialSpline **sp = new CphxMaterialSpline *[ batch->SplineCount + 1 ];
        for ( int y = 0; y < batch->SplineCount; y++ )
          sp[ y ] = batch->Splines[ y ];
        sp[ batch->SplineCount ] = s;
        batch->Splines = sp;
        batch->SplineCount++;
      }
    }
  }
}