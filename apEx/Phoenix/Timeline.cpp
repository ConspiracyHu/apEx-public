#include "Timeline.h"
#include "phxMath.h"
#include "Material.h"

//////////////////////////////////////////////////////////////////////////
// event implementations

extern bool phxDone;
D3DXVECTOR3 EyeOffset;
D3DXVECTOR3 TargetOffset;
CphxObject* cameraOverride;

#ifdef PHX_EVENT_ENDDEMO
void CphxEvent_EndDemo::Render( float t, float prevt, float aspect, bool subroutine )
{
  phxDone = true;
}
#endif

#ifdef PHX_EVENT_RENDERDEMO
void CphxEvent_RenderDemo::Render( float t, float prevt, float aspect, bool subroutine )
{
  if ( subroutine ) return; //no recursion thank you
  //Timeline->Render(lerp(_start,_end,t),true);
}
#endif

static ID3D11ShaderResourceView *rv[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void Prepare2dRender();

#ifdef PHX_EVENT_SHADERTOY
void CphxEvent_Shadertoy::Render( float t, float prevt, float aspect, bool subroutine )
{
  if ( !Tech ) return;

  MaterialSplines->CalculateValues( t );
  MaterialSplines->ApplyToParameters( NULL );

  for ( int y = 0; y < Tech->PassCount; y++ )
    Tech->CollectAnimatedData( MaterialState[ y ], y );

  float ShaderData[ 4 ];
  ShaderData[ 0 ] = t;
  ShaderData[ 1 ] = aspect;

  Prepare2dRender();

  phxContext->PSSetConstantBuffers( 0, 1, &SceneDataBuffer );
  phxContext->PSSetConstantBuffers( 1, 1, &ObjectMatrixBuffer );

  extern CphxRenderTarget *phxInternalRenderTarget;

  for ( int x = 0; x < Tech->PassCount; x++ )
  {
    if ( MaterialState[ x ]->RenderTarget )
    {
      //copy rendertarget if needed
      bool NeedsRTCopy = false;
      for ( int y = 0; y < 7; y++ )
      {
        MaterialState[ x ]->TextureBackup[ y ] = MaterialState[ x ]->Textures[ y ];
        if ( MaterialState[ x ]->Textures[ y ] == MaterialState[ x ]->RenderTarget->View )
        {
          MaterialState[ x ]->Textures[ y ] = phxInternalRenderTarget->View;
          NeedsRTCopy = true;
        }
      }

      if ( NeedsRTCopy )
        phxContext->CopyResource( phxInternalRenderTarget->Texture, MaterialState[ x ]->RenderTarget->Texture );

      //done copying, go on...
      MaterialState[ x ]->RenderTarget->SetViewport();
      phxContext->OMSetRenderTargets( 1, &MaterialState[ x ]->RenderTarget->RTView, NULL );
    }

    Target = MaterialState[ x ]->RenderTarget;

    phxContext->PSSetShader( Tech->RenderPasses[ x ]->PS, NULL, 0 );

    phxContext->RSSetState( MaterialState[ x ]->RasterizerState );
    phxContext->OMSetBlendState( MaterialState[ x ]->BlendState, NULL, 0xffffffff );
    phxContext->OMSetDepthStencilState( MaterialState[ x ]->DepthStencilState, 0 );
    MaterialState[ x ]->Textures[ 7 ] = phxDepthBufferShaderView;						//DEPTH BUFFER SET TO TEXTURE7 HERE
    phxContext->PSSetShaderResources( 0, 8, MaterialState[ x ]->Textures );

    int constdatasize = MaterialState[ x ]->ConstantDataSize;
    int dyndatasize = MaterialState[ x ]->AnimatedDataSize;

    D3D11_MAPPED_SUBRESOURCE map;
    phxContext->Map( ObjectMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map );
    memcpy( ( (unsigned char*)map.pData ), MaterialState[ x ]->ConstantData, constdatasize );
    memcpy( ( (unsigned char*)map.pData ) + constdatasize, MaterialState[ x ]->AnimatedData, dyndatasize );
    phxContext->Unmap( ObjectMatrixBuffer, 0 );

    //set layer shader data
    phxContext->Map( SceneDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map );
    unsigned char* m = (unsigned char*)map.pData;

    if ( Target )
    {
      ShaderData[ 2 ] = (float)Target->XRes;
      ShaderData[ 3 ] = (float)Target->YRes;
    }

    memcpy( m, ShaderData, sizeof( ShaderData ) ); m += sizeof( ShaderData );
    memcpy( m, &phxViewMatrix, sizeof( phxViewMatrix ) ); m += sizeof( phxViewMatrix );
    memcpy( m, &phxProjectionMatrix, sizeof( phxProjectionMatrix ) ); m += sizeof( phxProjectionMatrix );
    memcpy( m, &phxCameraPos, sizeof( phxCameraPos ) ); m += sizeof( phxCameraPos );
    memcpy( m, &phxIViewMatrix, sizeof( phxIViewMatrix ) ); m += sizeof( phxIViewMatrix );
    memcpy( m, &phxIProjectionMatrix, sizeof( phxIProjectionMatrix ) ); m += sizeof( phxIProjectionMatrix );
    memcpy( m, &phxPrevFrameViewMatrix, sizeof( phxPrevFrameViewMatrix ) ); m += sizeof( phxPrevFrameViewMatrix );
    memcpy( m, &phxPrevFrameProjectionMatrix, sizeof( phxPrevFrameProjectionMatrix ) ); m += sizeof( phxPrevFrameProjectionMatrix );

    phxContext->Unmap( SceneDataBuffer, 0 );


    //ConstantBuffers[2] = RSBatch->PassParams[x];
    //phxContext->PSSetConstantBuffers(0, 3, ConstantBuffers);

    //render here
    phxContext->Draw( 6, 0 );

    //clean up
    //memset(&rv, 0, sizeof(rv));
    phxContext->PSSetShaderResources( 0, 8, rv );

    if ( Target )
      phxContext->GenerateMips(Target->View);

    for ( int y = 0; y < 7; y++ )
      MaterialState[ x ]->Textures[ y ] = MaterialState[ x ]->TextureBackup[ y ];

  }
}
#endif

static float up[] = { 0, 1, 0 };

#ifdef PHX_EVENT_CAMOVERRIDE
void CphxEvent_CameraOverride::Render( float t, float prevt, float aspect, bool subroutine )
{
  if ( !Scene || ( !Camera ) ) return;

  Scene->UpdateSceneGraph( Clip, t );
  cameraOverride = Camera;
}
#endif

#ifdef PHX_EVENT_RENDERSCENE
void CphxEvent_RenderScene::Render( float t, float prevt, float aspect, bool subroutine )
{
  if ( !Scene || ( !Camera && !cameraOverride ) ) return;

  CphxObject* actualCamera = cameraOverride ? cameraOverride : Camera;

  for ( int x = 0; x < 2; x++ )
  {
    phxPrevFrameViewMatrix = phxViewMatrix;
    phxPrevFrameProjectionMatrix = phxProjectionMatrix;

    Scene->UpdateSceneGraph( Clip, x ? t : prevt );
    actualCamera->WorldPosition += EyeOffset;
    D3DXVECTOR3 eye = actualCamera->WorldPosition;
    D3DXVECTOR3 dir = *( (D3DXVECTOR3*)&actualCamera->SplineResults[ Spot_Direction_X ] );
    phxCameraPos = D3DXVECTOR4( eye.x, eye.y, eye.z, 1 );

    //roll camera
    D3DXMATRIX RollMat;
    D3DXVECTOR4 rolledup;
    D3DXVec3Transform( &rolledup, (D3DXVECTOR3*)up, D3DXMatrixRotationAxis( &RollMat, &dir, actualCamera->SplineResults[ Spline_Camera_Roll ] * 3.14159265359f*2.0f ) );

    D3DXMatrixLookAtRH( &phxViewMatrix, &( eye ), &( eye + dir + TargetOffset ), (D3DXVECTOR3*)&rolledup );

    // off-center calculation

    float fovYper2 = ( actualCamera->SplineResults[ Spline_Camera_FOV ] * 3.14159265359f / 4.0f ) / 2.0f;
    float zn = 0.01f;
    float cotFov = cos( fovYper2 ) / sin( fovYper2 );
    float t = zn / cotFov;
    float r = zn * aspect / cotFov;
    float xOffset = actualCamera->camCenterX / 127.0f * r;
    float yOffset = actualCamera->camCenterY / 127.0f * t;

    D3DXMatrixPerspectiveOffCenterRH( &phxProjectionMatrix, -r + xOffset, r + xOffset, -t + yOffset, t + yOffset, zn, 2000.0f );

    //D3DXMatrixPerspectiveFovRH( &phxProjectionMatrix, Camera->SplineResults[ Spline_Camera_FOV ] * 3.14159265359f / 4.0f, aspect, 0.01f, 2000.0f );

#ifdef PHX_OBJ_EMITTERCPU
    //if (x == 1) //if you put this back the particle count will lag one frame behind and that will cause flickering
    {
      for ( int x = 0; x < Scene->ObjectCount; x++ )
        if ( Scene->Objects[ x ]->ObjectType == Object_ParticleEmitterCPU )
        {
          CphxObject_ParticleEmitter_CPU *p = (CphxObject_ParticleEmitter_CPU*)Scene->Objects[ x ];
          //if ( p->VertexBuffer )
            p->UpdateParticles( 0, true ); //sort particles
        }
    }
#endif
  }

  //these are calculated at the start of the render
  //D3DXMatrixInverse(&phxIViewMatrix, NULL, &phxViewMatrix);
  //D3DXMatrixInverse(&phxIProjectionMatrix, NULL, &phxProjectionMatrix);
/*
  if ( !actualCamera->cameraCubeMapTarget )
  {
*/
    Scene->Render( ClearColor, ClearZ, 0 );
/*
  }
  else
  {
    for ( int x = 0; x < 6; x++ )
    {
      // set cube face camera

      Scene->Render( ClearColor, ClearZ, 256 );

      // copy target to cube face
      //this->Target
    }

    // build cube mipmap
  }
*/
}
#endif

unsigned int TimelineFramerate;
int CurrentFrame;

#ifdef PHX_EVENT_PARTICLECALC
void CphxEvent_ParticleCalc::Render( float t, float prevt, float aspect, bool subroutine )
{
  if ( !Scene /*|| ( !Camera && !cameraOverride )*/ ) return;

/*
#ifndef PHX_MINIMAL_BUILD
  int tme = TimerFunction();
#else
*/
  int tme = ( ( EndFrame - StartFrame ) * t + StartFrame ) * 1000.0f / TimelineFramerate;
//#endif

  if ( !OnScreenLastFrame )
  {
    lastt = t;
    lasttime = tme;
    OnScreenLastFrame = true;
  }

  //for (int x = 0; x < 2; x++)
  Scene->UpdateSceneGraph( Clip, lastt );
  Scene->UpdateSceneGraph( Clip, t );

  for ( int x = 0; x < Scene->ObjectCount; x++ )
    if ( Scene->Objects[ x ]->ObjectType == Object_ParticleEmitterCPU )
    {
      CphxObject_ParticleEmitter_CPU *p = (CphxObject_ParticleEmitter_CPU*)Scene->Objects[ x ];
      //if ( p->VertexBuffer )
        p->UpdateParticles( ( tme - lasttime ) / 1000.0f, false );
    }

  lasttime = tme;
  lastt = t;
}
#endif

#ifdef PHX_EVENT_CAMSHAKE
static long aholdrand = 1L;

int __cdecl arand()
{
  return( ( ( aholdrand = aholdrand * 214013L + 2531011L ) >> 16 ) & 0x7fff );
}

void CphxEvent_CameraShake::Render( float t, float prevt, float aspect, bool subroutine )
{
  float dist = 1 / (float)ShakesPerSec;
  float currTime = CurrentFrame / (float)TimelineFramerate;
  float d1 = fmod( currTime, dist );
  float d = d1 / dist;
  int t1 = (int)( ( currTime - d1 )*TimelineFramerate );
  int t2 = (int)( ( currTime - d1 + dist )*TimelineFramerate );

  aholdrand = t1;

  D3DXVECTOR3 eo1, eo2, to1, to2;

  for ( int x = 0; x < 3; x++ )
  {
    eo1[ x ] = arand() / (float)RAND_MAX - 0.5f;
    to1[ x ] = arand() / (float)RAND_MAX - 0.5f;
  }

  aholdrand = t2;

  for ( int x = 0; x < 3; x++ )
  {
    eo2[ x ] = arand() / (float)RAND_MAX - 0.5f;
    to2[ x ] = arand() / (float)RAND_MAX - 0.5f;
  }

  EyeIntensity->CalculateValue( t );
  TargetIntensity->CalculateValue( t );

  EyeOffset = ( ( eo2 - eo1 )*d + eo1 )*EyeIntensity->Value[ 0 ];
  TargetOffset = ( ( to2 - to1 )*d + to1 )*TargetIntensity->Value[ 0 ];
}
#endif

//////////////////////////////////////////////////////////////////////////
// timeline renderer

void CphxTimeline::Render( float Frame, bool tool, bool subroutine )
{
  cameraOverride = nullptr;
  TimelineFramerate = FrameRate;
  CurrentFrame = (int)Frame;
  EyeOffset = TargetOffset = D3DXVECTOR3( 0, 0, 0 );

  if ( !subroutine )
  {
    Target = NULL;

    //clear rendertargets at the beginning of the frame
    //FLOAT col[4];
    //col[0] = col[1] = col[2] = col[3] = 0;
    if ( !tool )
      phxContext->ClearRenderTargetView( phxBackBufferView, (float*)rv ); //clean up after precalcbar

    for ( int x = 0; x < RenderTargetCount; x++ )
      phxContext->ClearRenderTargetView( RenderTargets[ x ]->RTView, (float*)rv );
    phxContext->ClearDepthStencilView( phxDepthBufferView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1, 0 );
  }

  for ( int x = 0; x < EventCount; x++ )
  {
    if ( Events[ x ]->StartFrame <= (int)Frame && (int)Frame < Events[ x ]->EndFrame )
    {
      float t = ( Frame - Events[ x ]->StartFrame ) / ( Events[ x ]->EndFrame - Events[ x ]->StartFrame ); //position in event
      float prevt = ( Frame - 1 - Events[ x ]->StartFrame ) / ( Events[ x ]->EndFrame - Events[ x ]->StartFrame ); //position in event (previous frame)

      if ( Events[ x ]->Time ) //if time spline is missing, use default 0..1
      {
        Events[ x ]->Time->CalculateValue( prevt );
        prevt = Events[ x ]->Time->Value[ 0 ];
        Events[ x ]->Time->CalculateValue( t ); //time adjusted by event's time spline
        t = Events[ x ]->Time->Value[ 0 ];
      }

      Events[ x ]->Render( t, prevt, AspectX / (float)AspectY, subroutine );// Events[x]->Time.Value[0],subroutine);

      Target = Events[ x ]->Target;
      Events[ x ]->OnScreenLastFrame = true;
    }
    else Events[ x ]->OnScreenLastFrame = false;
  }

  if ( Target && !tool )
  {
    //display frame
    Prepare2dRender();
    phxContext->PSSetShader( RenderPixelShader, NULL, 0 );

    D3D11_VIEWPORT v = { ( ScreenX - Target->XRes ) / 2.0f, ( ScreenY - Target->YRes ) / 2.0f, (float)Target->XRes, (float)Target->YRes, 0, 1 };
    phxContext->RSSetViewports( 1, &v );
    phxContext->OMSetRenderTargets( 1, &phxBackBufferView, NULL );
    phxContext->PSSetShaderResources( 0, 1, &Target->View );
    phxContext->Draw( 6, 0 );
    phxContext->PSSetShaderResources( 0, 1, rv );
  }
}


