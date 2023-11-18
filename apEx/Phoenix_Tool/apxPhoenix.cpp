#include "BasePCH.h"
#include "apxPhoenix.h"
#include "Texgen_tool.h"

typedef HRESULT( __stdcall d3d_compile_func )( LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs );

void PhoenixInitTool( ID3D11Device *Dev, ID3D11DeviceContext *ctx )
{
  extern d3dcompilefunc *D3DCompileCall;
  extern d3d_compile_func *D3DCompileFunc;

  D3DCompileCall = (d3dcompilefunc*)D3DCompileFunc;

  phxDev = Dev;
  phxContext = ctx;
  InitializePhoenix();

  memset( EngineFontList, 0, sizeof( EngineFontList ) );
  char* fonts[] = {
  "Arial",
  "Arial Black",
  "Calibri",
  "Cambria",
  "Candara",
  "Comic Sans MS",
  "Consolas",
  "Constantia",
  "Corbel",
  "Courier New",
  "Franklin Gothic Medium",
  "Georgia",
  "Impact",
  "Lucida Console",
  "Lucida Sans Unicode",
  "Microsoft Sans Serif",
  "Palatino Linotype",
  "Tahoma",
  "Times New Roman",
  "Trebuchet MS",
  "Verdana",
  "Webdings",
  "Wingdings",
};
  for ( int x = 0; x < sizeof( fonts ) / sizeof( char* ); x++ )
    EngineFontList[ x ] = fonts[ x ];

  IGNOREFREEERRORS( true );
  SAFEDELETE( TexgenPool );
  IGNOREFREEERRORS( false );
  TexgenPool = new CphxTexturePool_Tool();
}

void PhoenixDeinitTool()
{
  DeinitTexgen();
  SceneDataBuffer->Release();
  ObjectMatrixBuffer->Release();
  TexgenVertexBuffer->Release();
  TexgenVertexShader->Release();
  RenderPixelShader->Release();
  TexgenVertexFormat->Release();
  RenderVertexFormat->Release();
#ifdef PHX_HAS_STANDARD_PARTICLES
  ParticleVertexFormat->Release();
#endif
  ( (CphxTexturePool_Tool*)TexgenPool )->Clear();
  delete TexgenPool;
}
