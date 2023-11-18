#include "RenderLayer.h"

void __fastcall GetRenderTargetResolution( unsigned char ResData, int &XRes, int &YRes, int DemoResX, int DemoResY )
{
#ifdef PHX_HAS_NON_DEMO_RELATIVE_RENDERTARGETS
  bool DemoRelative = ( ResData & 0x80 ) != 0;
#endif

  int xfact = ( ResData & 0x38 ) >> 3;
  int yfact = ResData & 0x07;

#ifdef PHX_HAS_NON_DEMO_RELATIVE_RENDERTARGETS
  if ( !DemoRelative )
  {
    XRes = 1 << ( xfact + 5 );
    YRes = 1 << ( yfact + 5 );
    return;
  }
#endif

  XRes = DemoResX >> xfact;
  YRes = DemoResY >> yfact;
}

#include "Model.h"

void CphxRenderDataInstance::Render()
{
  if ( !VS || !PS ) return; //kill invalid batches - required to avoid some driver crashes in the tool
  if ( !VertexBuffer ) return;

  unsigned int offset = 0;
  unsigned int stride = PHXVERTEXFORMATSIZE;

#ifdef PHX_HAS_STANDARD_PARTICLES
  if ( Indexed )
#endif
    phxContext->IASetInputLayout( RenderVertexFormat );
#ifdef PHX_HAS_STANDARD_PARTICLES
  else
  {
    phxContext->IASetInputLayout( ParticleVertexFormat );
    stride = 4 * 2 * 4;
  }
#endif

  phxContext->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
  phxContext->IASetPrimitiveTopology( Wireframe ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

#ifdef PHX_HAS_STANDARD_PARTICLES
  if ( !Indexed )
    phxContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
#endif

  phxContext->VSSetShader( VS, NULL, 0 );
  phxContext->GSSetShader( GS, NULL, 0 );
  phxContext->HSSetShader( HS, NULL, 0 );
  phxContext->DSSetShader( DS, NULL, 0 );
  phxContext->PSSetShader( PS, NULL, 0 );
  phxContext->RSSetState( RasterizerState );
  phxContext->OMSetBlendState( BlendState, NULL, 0xffffffff );
  phxContext->OMSetDepthStencilState( DepthStencilState, 0 );

  D3D11_MAPPED_SUBRESOURCE map;
  phxContext->Map( ObjectMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map );
  memcpy( map.pData, Matrices, 16 * 2 * 4 );
  memcpy( ( (unsigned char*)map.pData ) + sizeof( Matrices ), MaterialData, MATERIALDATASIZE );
  phxContext->Unmap( ObjectMatrixBuffer, 0 );


  //phxContext->VSSetConstantBuffers(1, 3, Buffers);
  //phxContext->PSSetConstantBuffers(1, 3, Buffers);

  //int TextureCount = 0;
  //for (int x = 0; x < 8; x++)
  //{
  //	if (!Textures[x]) break;
  //	TextureCount++;
  //}

  //if (TextureCount)
  {
    phxContext->VSSetShaderResources( 0, 8, Textures );
    phxContext->GSSetShaderResources( 0, 8, Textures );
    phxContext->PSSetShaderResources( 0, 8, Textures );
  }

  phxContext->IASetIndexBuffer( Wireframe ? WireBuffer : IndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

#ifdef PHX_HAS_STANDARD_PARTICLES
  if ( Indexed )
  {
#endif
    if ( Wireframe ? WireIndexCount : TriIndexCount )
      phxContext->DrawIndexed( Wireframe ? WireIndexCount : TriIndexCount, 0, 0 );
#ifdef PHX_HAS_STANDARD_PARTICLES
  }
  else
    phxContext->Draw( TriIndexCount, 0 ); //particles are non indexed
#endif
}

static FLOAT col[ 4 ] = { 0, 0, 0, 0 };
static D3D11_VIEWPORT viewports[ D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ];
static ID3D11RenderTargetView *rendertargets[ D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT ];

void CphxRenderLayerDescriptor::SetEnvironment( bool ClearColor, bool ClearZ, int cubeResolution )
{
  for ( int x = 0; x < TargetCount; x++ )
  {
    viewports[ x ].TopLeftX = viewports[ x ].TopLeftY = 0;
    viewports[ x ].Width = !cubeResolution ? (float)Targets[ x ]->XRes : cubeResolution;
    viewports[ x ].Height = !cubeResolution ? (float)Targets[ x ]->YRes : cubeResolution;
    viewports[ x ].MinDepth = 0;
    viewports[ x ].MaxDepth = 1;
    rendertargets[ x ] = Targets[ x ]->RTView;
    if ( ClearColor && clearRenderTargets )
      phxContext->ClearRenderTargetView( Targets[ x ]->RTView, col );
  }

#ifdef PHX_VOLUMETRIC_RENDERTARGETS
  if ( !VoxelizerLayer )
    {
#endif
  phxContext->RSSetViewports( TargetCount, viewports );
#ifdef PHX_VOLUMETRIC_RENDERTARGETS
  phxContext->OMSetRenderTargetsAndUnorderedAccessViews( TargetCount, rendertargets, OmitDepthBuffer ? NULL : phxDepthBufferView, TargetCount, 0, NULL, NULL );
#else
  phxContext->OMSetRenderTargets( TargetCount, rendertargets, OmitDepthBuffer ? NULL : phxDepthBufferView );
#endif
  if ( ClearZ && clearRenderTargets )
    phxContext->ClearDepthStencilView( phxDepthBufferView, ( D3D10_CLEAR_DEPTH * !OmitDepthBuffer ) | D3D10_CLEAR_STENCIL, 1, 0 );
#ifdef PHX_VOLUMETRIC_RENDERTARGETS
  }
    else
    {
      phxContext->ClearUnorderedAccessViewFloat( phxTexture3DUAV, col );
      viewports[ 0 ].TopLeftX = viewports[ 0 ].TopLeftY = 0;
      viewports[ 0 ].Width = 256;
      viewports[ 0 ].Height = 256;
      viewports[ 0 ].MinDepth = 0;
      viewports[ 0 ].MaxDepth = 1;
      phxContext->RSSetViewports( 1, viewports );
      phxContext->OMSetRenderTargetsAndUnorderedAccessViews( 1, &phxVoxelForceMSAARTView, nullptr, 1, 1, &phxTexture3DUAV, nullptr );
    }
#endif
}

//#ifdef PHX_VOLUMETRIC_RENDERTARGETS

void CphxRenderLayerDescriptor::GenMipmaps()
{
  for ( int x = 0; x < TargetCount; x++ )
  {
    //if ( Targets[x]->DoMips )
      phxContext->GenerateMips( Targets[x]->View );
  }
}

//#endif