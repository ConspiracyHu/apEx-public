#include "Texgen.h"
#include "phxSpline.h"
#include "../LibCTiny/libcminimal.h"

//////////////////////////////////////////////////////////////////////////
// Texture pool

#define SCALEX(x) (((x)*XRes)>>8)
#define WRAPX(x) ((x)&(XRes-1))
#define SCALEY(y) (((y)*YRes)>>8)
#define WRAPY(y) ((y)&(YRes-1))

CphxTexturePool *TexgenPool;

static D3D11_RENDER_TARGET_VIEW_DESC texgenrt = { DXGI_FORMAT_R16G16B16A16_UNORM, D3D11_RTV_DIMENSION_TEXTURE2D, 0 };
static D3D11_TEXTURE2D_DESC tex = { 0, 0, 0, 1, DXGI_FORMAT_R16G16B16A16_UNORM, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS };

bool CphxTexturePoolTexture::Create( unsigned char res, bool _hdr )
{
  tex.Format = texgenrt.Format = hdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R16G16B16A16_UNORM;

  Resolution = res;
  Used = false;

  XRes = GETXRES( res );
  YRes = GETYRES( res );
  hdr = _hdr;

  tex.Width = XRes;
  tex.Height = YRes;

#ifdef DEBUGINTOFILE
  HRESULT result =
#endif
    phxDev->CreateTexture2D( &tex, NULL, &Texture );
#ifdef DEBUGINTOFILE
  if ( result != S_OK )
    DEBUGLOG( "*** Failed to create Texture pool texture2d of resolution (%x) %d %d", result, XRes, YRes );
#endif

#ifdef DEBUGINTOFILE
  result =
#endif
    phxDev->CreateShaderResourceView( Texture, NULL, &View );
#ifdef DEBUGINTOFILE
  if ( result != S_OK )
    DEBUGLOG( "*** Failed to create shader resource view of resolution (%x) %d %d", result, XRes, YRes );
#endif

#ifdef DEBUGINTOFILE
  result =
#endif
    phxDev->CreateRenderTargetView( Texture, &texgenrt, &RTView );
#ifdef DEBUGINTOFILE
  if ( result != S_OK )
    DEBUGLOG( "*** Failed to create rendertarget view of resolution (%x) %d %d", result, XRes, YRes );
#endif

#ifdef DEBUGINTOFILE
  if ( !Texture || !View || !RTView )
    DEBUGLOG( "*** Failed to create Texture pool texture of resolution %d %d", XRes, YRes );
#endif

  return true;
}

CphxTexturePoolTexture *CphxTexturePool::GetTexture( unsigned char Resolution, bool hdr )
{
  for ( int x = 0; x < poolSize; x++ )
  {
    CphxTexturePoolTexture* p = pool[ x ];
    if ( p->Resolution == Resolution && p->hdr == hdr && !p->Used && !p->Deleted )
    {
      p->Used = true;
      return p;
    }
  }

  //return NULL;
  CphxTexturePoolTexture *t = new CphxTexturePoolTexture;
  pool[ poolSize++ ] = t;
  t->Resolution = Resolution;
  t->Create( Resolution, hdr );
  t->Used = true;

  return t;
}

//////////////////////////////////////////////////////////////////////////
// Texture filters

void SetSamplers()
{
  phxContext->PSSetSamplers( 0, 1, &TexgenSampler );
  phxContext->PSSetSamplers( 1, 1, &TexgenSampler_NoWrap );
  phxContext->PSSetSamplers( 2, 1, &TexgenSampler_ShadowCompare );
}

void Prepare2dRender()
{
  phxContext->VSSetShader( TexgenVertexShader, NULL, 0 );

  SetSamplers();

  phxContext->RSSetState( NULL );
  phxContext->OMSetBlendState( NULL, NULL, 0xffffffff );
  phxContext->OMSetDepthStencilState( NULL, 0 );

  phxContext->IASetInputLayout( TexgenVertexFormat );
  unsigned int offset = 0;
  unsigned int stride = 6 * sizeof( float );
  phxContext->IASetVertexBuffers( 0, 1, &TexgenVertexBuffer, &stride, &offset );
  phxContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

  phxContext->GSSetShader( NULL, NULL, 0 );
  phxContext->HSSetShader( NULL, NULL, 0 );
  phxContext->DSSetShader( NULL, NULL, 0 );
}

void PHXTEXTUREFILTER::Render( CphxTexturePoolTexture *&Target, CphxTexturePoolTexture *&SwapBuffer, CphxTexturePoolTexture *Inputs[ TEXGEN_MAX_PARENTS ], unsigned char RandSeed, unsigned char Parameters[ TEXGEN_MAX_PARAMS ], void *ExtraData, int ExtraDataSize )
{
  srand( RandSeed );
  float ShaderData[ TEXGEN_MAX_PARAMS + 4 ];

  phxContext->PSSetShader( PixelShader, NULL, 0 );
  Prepare2dRender();
  phxContext->PSSetConstantBuffers( 0, 1, &TexgenBufferPS );

  Target->SetViewport();

  D3D11_MAPPED_SUBRESOURCE map;
  ID3D11ShaderResourceView *Textures[ 5 ];

  for ( unsigned int x = 0; x < DataDescriptor.PassCount; x++ )
  {
    CphxTexturePoolTexture *Lookup = GetLookupTexture( Target->Resolution, ExtraData, ExtraDataSize );

    //swap targets
    CphxTexturePoolTexture *swapvar = SwapBuffer;
    SwapBuffer = Target;
    Target = swapvar;

    //clear warnings about rendertargets
    Textures[ 0 ] = Textures[ 1 ] = Textures[ 2 ] = Textures[ 3 ] = Textures[ 4 ] = NULL;
    phxContext->PSSetShaderResources( 0, 5, Textures );

    //set render target
    phxContext->OMSetRenderTargets( 1, &Target->RTView, NULL );

    //set shader data
    ShaderData[ 0 ] = (float)x; //pass count
    for ( int y = 0; y < 3; y++ ) ShaderData[ y + 1 ] = rand() / (float)RAND_MAX;
    for ( int y = 0; y < TEXGEN_MAX_PARAMS; y++ ) ShaderData[ y + 4 ] = Parameters[ y ] / 255.0f;

    //set shaders and uniforms
    phxContext->Map( TexgenBufferPS, 0, D3D11_MAP_WRITE_DISCARD, 0, &map );
    memcpy( map.pData, ShaderData, SHADERDATALENGTH );
    phxContext->Unmap( TexgenBufferPS, 0 );

    //set samplers

    int scnt = 0;
    if ( Inputs[ 0 ] || x ) Textures[ scnt++ ] = x ? SwapBuffer->View : Inputs[ 0 ]->View;
    if ( Inputs[ 1 ] ) Textures[ scnt++ ] = Inputs[ 1 ]->View;
    if ( Inputs[ 2 ] ) Textures[ scnt++ ] = Inputs[ 2 ]->View;
    if ( Lookup ) Textures[ scnt++ ] = Lookup->View;
    if ( Lookup ) Textures[ scnt++ ] = Lookup->View; //yeah, this is required to make sure the noise is loaded into a known sampler for multipass noise lookup filters

    phxContext->PSSetShaderResources( 0, 5, Textures );

    //render here
    phxContext->Draw( 6, 0 );

    phxContext->GenerateMips( Target->View );

    if ( Lookup )
    {
      if ( Lookup->View )
        Lookup->View->Release();
      if ( Lookup->Texture )
        Lookup->Texture->Release();
      delete Lookup;
    }
  }
}

HDC hdc = GetDC( NULL );

static unsigned long rndx = 123456789, rndy = 362436069, rndz = 521288629;

unsigned long xorshf96( void ) {          //period 2^96-1
  unsigned long t;
  rndx ^= rndx << 16;
  rndx ^= rndx >> 5;
  rndx ^= rndx << 1;

  t = rndx;
  rndx = rndy;
  rndy = rndz;
  rndz = t ^ rndx ^ rndy;

  return rndz;
}

CphxTexturePoolTexture * PHXTEXTUREFILTER::GetLookupTexture( unsigned char Res, void *ExtraData, int ExtraDataSize )
{
  if ( !DataDescriptor.LookupType || ( !ExtraData && DataDescriptor.LookupType != 4 ) ) return NULL;

  int XRes = GETXRES( Res );
  int YRes = GETYRES( Res );

  CphxTexturePoolTexture *out = new CphxTexturePoolTexture();
  unsigned char *ImageData = NULL;

  D3D11_TEXTURE2D_DESC tex = { XRes, YRes, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
  D3D11_SUBRESOURCE_DATA data = { 0, XRes * 4, 0 };

  switch ( DataDescriptor.LookupType )
  {
#ifdef PHX_TEXGEN_IMAGE
  case 1:	//image load
  {
    //D3DX11_IMAGE_LOAD_INFO nfo;
    //nfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    if ( D3DX11CreateTextureFromMemory( phxDev, ExtraData, ExtraDataSize, NULL, NULL, (ID3D11Resource**)&out->Texture, NULL ) == S_OK )
    {
      //FixFloatingPoint();
      phxDev->CreateShaderResourceView( out->Texture, NULL, &out->View );
      //delete[] ImageData;
      return out;
    }

    //failed to load with d3dx, try with gdi
    ImageData = new unsigned char[ XRes*YRes * 4 ];
    LoadMetafile( ImageData, XRes, YRes, ExtraData, ExtraDataSize );
  }
  break;
#endif
  case 2:	//text display
  {
    tex.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    ImageData = new unsigned char[ XRes*YRes * 4 ];
    PHXTEXTDATA *t = (PHXTEXTDATA*)ExtraData;

#ifndef PHX_MINIMAL_BUILD
    if ( !ExtraDataSize )//if (!t->Text)
    {
      delete[] ImageData;
      delete out;
      return NULL;
    }
#endif

    unsigned char *ImageData2 = new unsigned char[ XRes*YRes * 4 ];

    HDC mdc = CreateCompatibleDC( hdc );
    HBITMAP bm = CreateCompatibleBitmap( hdc, XRes, YRes );
    BITMAPINFO bmi;

    HFONT hf = CreateFontA( ( t->Size*XRes ) >> 8, 0, 0, 0, t->Bold ? FW_BOLD : FW_NORMAL,
                            t->Italic, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, DEFAULT_PITCH, EngineFontList[ t->Font ] );

    RECT r = { 0, 0, XRes, YRes };

    HGDIOBJ bmOld = SelectObject( mdc, bm );
    //FillRect( mdc, &r, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

    HGDIOBJ fnOld = SelectObject( mdc, hf );
    SetBkMode( mdc, TRANSPARENT );
    SetTextColor( mdc, 0xFFFFFF );
    SetTextCharacterExtra( mdc, SCALEX( t->CharSpace - 64 ) );
    int len = 0;
    while ( ( (char*)ExtraDataSize )[ len ] ) len++;//while (t->Text[len]) len++;
    TextOut( mdc, 0, 0, (char*)ExtraDataSize, len );//TextOut(mdc, 0, 0, t->Text, len);

    bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
    bmi.bmiHeader.biWidth = XRes;
    bmi.bmiHeader.biHeight = YRes;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    GetDIBits( mdc, bm, 0, YRes, ImageData2, &bmi, DIB_RGB_COLORS );

    for ( int zx = 0; zx < XRes; zx++ )
      for ( int zy = 0; zy < YRes; zy++ )
        ( (unsigned int*)( &ImageData[ ( WRAPX( zx + SCALEX( t->XPos ) ) + WRAPY( ( YRes - zy - 1 ) + SCALEY( t->YPos ) )*XRes ) * 4 ] ) )[ 0 ] = ImageData2[ ( zx + zy*XRes ) * 4 ] * 0x01010101;

    delete[] ImageData2;

    SelectObject( mdc, fnOld );
    DeleteObject( hf );
    SelectObject( mdc, bmOld );
    DeleteObject( bm );
    DeleteDC( mdc );
  }
  break;
  case 3:	//spline
  {
    XRes = 4096;
    ImageData = new unsigned char[ XRes * 4 * 2 ];
    CphxSpline_float16 **splines = (CphxSpline_float16 **)ExtraData;
    tex.Width = XRes;
    tex.Height = 1;
    tex.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
    data.SysMemPitch = XRes * 8;

    for ( int x = 0; x < XRes; x++ )
    {
      float t = x / (float)( XRes - 1 );
      for ( int i = 0; i < 4; i++ )
      {
        splines[ i ]->CalculateValue( t );
        ( (unsigned short*)ImageData )[ x * 4 + i ] = (unsigned short)( max( 0, min( 1, splines[ i ]->Value[ 0 ] ) ) * 65535 );
      }
    }

  }
  break;
  case 4: //noise
  {
    //srand(0); //random seed has already been set by the calling function! MUHHHAHAHAHAHAHAHAHA :D			
    tex.Width = tex.Height = XRes = YRes = 256;
    data.SysMemPitch = XRes * 4;
    ImageData = new unsigned char[ XRes*YRes * 4 ];
    rndx = rand();
    rndy = 362436069;
    rndz = 521288629;
    for ( int x = 0; x < XRes*YRes * 4; x++ ) ImageData[ x ] = (unsigned char)xorshf96();
  }
  break;
  default:
    break;
  }

  data.pSysMem = ImageData;

  HRESULT res = phxDev->CreateTexture2D( &tex, &data, &out->Texture );
  res = phxDev->CreateShaderResourceView( out->Texture, NULL, &out->View );

  if ( ImageData ) delete[] ImageData;
  return out;
}

#ifdef PHX_TEXGEN_IMAGE
#include <olectl.h>
#include <Shlwapi.h>

void PHXTEXTUREFILTER::LoadMetafile( unsigned char *Output, int XRes, int YRes, void *ImageData, int ImageSize )
{
  memset( Output, 0, XRes*YRes * 4 );
  if ( !ImageData || !ImageSize ) return;

  LPPICTURE gpPicture = NULL;

  LPSTREAM pstm = SHCreateMemStream( (unsigned char*)ImageData, ImageSize );
  OleLoadPictureEx( pstm, ImageSize, FALSE, IID_IPicture, 300, 300, LP_DEFAULT, (LPVOID *)&gpPicture );
  pstm->Release();

  if ( !gpPicture ) return;

  long hmWidth, hmHeight;
  gpPicture->get_Width( &hmWidth );
  gpPicture->get_Height( &hmHeight );

  int XSize = XRes;
  int YSize = (int)( hmHeight / (float)hmWidth*(float)XSize );
  if ( YSize > YRes )
  {
    YSize = YRes;
    XSize = (int)( hmWidth / (float)hmHeight*(float)YSize );
  }

  int AAlevel = 3;

  int XOff = ( XRes - XSize ) / 2;
  int YOff = ( YRes - YSize ) / 2;

  HDC mdc = CreateCompatibleDC( hdc );
  HBITMAP bm = CreateCompatibleBitmap( hdc, XSize*AAlevel, YSize*AAlevel );
  BITMAPINFO bmi = { sizeof( bmi.bmiHeader ), XSize*AAlevel, YSize*AAlevel, 1, 32, BI_RGB, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  HGDIOBJ oldbm = SelectObject( mdc, bm );
  RECT r = { 0, 0, XSize*AAlevel, YSize*AAlevel };

  SetBkMode( mdc, TRANSPARENT );

  int bufsize = XSize*YSize*AAlevel*AAlevel * 4;

  unsigned char *alphabuffer = new unsigned char[ bufsize * 2 ]; //black+white buffers
  unsigned char *wbuf = alphabuffer + bufsize;

  FillRect( mdc, &r, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
  gpPicture->Render( mdc, 0, 0, XSize*AAlevel, YSize*AAlevel, 0, hmHeight, hmWidth, -hmHeight, &r );
  GetDIBits( mdc, bm, 0, YSize*AAlevel, alphabuffer, &bmi, DIB_RGB_COLORS );

  FillRect( mdc, &r, (HBRUSH)GetStockObject( WHITE_BRUSH ) );
  gpPicture->Render( mdc, 0, 0, XSize*AAlevel, YSize*AAlevel, 0, hmHeight, hmWidth, -hmHeight, &r );
  GetDIBits( mdc, bm, 0, YSize*AAlevel, wbuf, &bmi, DIB_RGB_COLORS );

  SelectObject( mdc, oldbm );
  DeleteDC( mdc );
  DeleteObject( bm );

  gpPicture->Release();

  unsigned char *bbuf = alphabuffer;
  for ( int y = 0; y < YSize*AAlevel; y++ )
    for ( int x = 0; x < XSize*AAlevel; x++ )
    {
      //calculating alpha from the difference of the black background and white background rendered images
      bbuf[ 3 ] = 255 - ( wbuf[ 0 ] + wbuf[ 1 ] + wbuf[ 2 ] - ( bbuf[ 0 ] + bbuf[ 1 ] + bbuf[ 2 ] ) ) / 3;
      bbuf += 4;
      wbuf += 4;
    }

  int aad2 = AAlevel*AAlevel;

  //supersampling
  int l = ( YSize - 1 )*XSize * 4 * aad2;
  int a = YOff*XRes;
  for ( int y = 0; y < YSize; y++ )
  {
    int p = l;
    for ( int x = 0; x < XSize; x++ )
    {
      int cols[ 4 ];

      int jj = cols[ 0 ] = cols[ 1 ] = cols[ 2 ] = cols[ 3 ] = 0;
      for ( int j = 0; j < AAlevel; j++ )
      {
        int i4 = 0;
        for ( int i = 0; i < AAlevel; i++ )
        {
          int z = p + i4 + jj;
          for ( int k = 0; k < 3; k++ ) cols[ k ] += alphabuffer[ z + k ] * alphabuffer[ z + 3 ];
          cols[ 3 ] += alphabuffer[ z + 3 ];
          i4 += 4;
        }
        jj += XSize*AAlevel * 4;
      }

      for ( int j = 0; j < 3; j++ )
        Output[ ( a + x + XOff ) * 4 + j ] = cols[ 3 ] ? cols[ j ] / cols[ 3 ] : 0;

      Output[ ( a + x + XOff ) * 4 + 3 ] = cols[ 3 ] / aad2;

      p += AAlevel * 4;
    }
    a += XRes;
    l -= XSize * 4 * aad2;
  }

  delete[] alphabuffer;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Texture operators

CphxTexturePoolTexture *PHXTEXTUREOPERATOR::Generate( PHXTEXTUREFILTER *Filters, PHXTEXTUREOPERATOR *Operators )
{
  if ( CachedResult ) return CachedResult;

  CphxTexturePoolTexture *ParentResults[ TEXGEN_MAX_PARENTS ];

  //generate parents
  for ( int x = 0; x < TEXGEN_MAX_PARENTS; x++ )
  {
    ParentResults[ x ] = NULL;
    if ( Parents[ x ] >= 0 )
    {
      DEBUGLOG( " Generating Parent OP: %d", Parents[ x ] );
      ParentResults[ x ] = Operators[ Parents[ x ] ].Generate( Filters, Operators );
    }
  }

  //allocate memory
  CphxTexturePoolTexture *Result = TexgenPool->GetTexture( Resolution, ( Filter >> 7 ) != 0 );
  CphxTexturePoolTexture *BackBuffer = TexgenPool->GetTexture( Resolution, ( Filter >> 7 ) != 0 );

  //render
  Filters[ Filter & 0x7f ].Render( Result, BackBuffer, ParentResults, RandSeed, Parameters, minimportData, minimportData2 );

  BackBuffer->Used = false;

  //free parent results
  for ( int x = 0; x < TEXGEN_MAX_PARENTS; x++ )
    if ( ParentResults[ x ] && !Operators[ Parents[ x ] ].NeedsRender )
      ParentResults[ x ]->Used = false;

  //return result
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// Texture subroutines

CphxTexturePoolTexture *PHXTEXTURESUBROUTINE::Generate( PHXTEXTUREFILTER *Filters, PHXTEXTUREOPERATOR *CallerOperators, unsigned short *Parents, unsigned char *Parameters, unsigned char Resolution )
{
  //generate inputs
  for ( unsigned int x = 0; x < DataDescriptor.InputCount; x++ )
    Operators[ Inputs[ x ] ].CachedResult = CallerOperators[ Parents[ x ] ].Generate( Filters, Operators );

  //set resolution
  for ( int x = 0; x < 256; x++ )
    Operators[ x ].Resolution = Resolution;

  //set parameters
  for ( int x = 0; x < DynamicParameterCount; x++ )
    Operators[ DynamicParameters[ x ].TargetOperator ].Parameters[ DynamicParameters[ x ].TargetParameter ] = Parameters[ x ];

  //Generate
  CphxTexturePoolTexture *Result = Operators[ Output ].Generate( Filters, Operators );

  //free inputs
  for ( unsigned int x = 0; x < DataDescriptor.InputCount; x++ )
  {
    Operators[ Inputs[ x ] ].CachedResult->Used = false;
    Operators[ Inputs[ x ] ].CachedResult = 0;
  }

  return Result;
}
