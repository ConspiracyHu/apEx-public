#include "Scene.h"
#include "..\LibCTiny\libcminimal.h"

D3DXMATRIX phxViewMatrix;
D3DXMATRIX phxProjectionMatrix;
D3DXMATRIX phxPrevFrameViewMatrix;
D3DXMATRIX phxPrevFrameProjectionMatrix;
D3DXVECTOR4 phxCameraPos;
D3DXMATRIX phxIViewMatrix;
D3DXMATRIX phxIProjectionMatrix;

D3DXMATRIX phxWorldMatrix;
D3DXMATRIX phxITWorldMatrix;
D3DXVECTOR4 phxCameraDir; //for hive

int Pivot( CphxRenderDataInstance **Instances, int first, int last )
{
  int  p = first;
  int pivot = Instances[ first ]->RenderPriority; //moving this value out of the main loop makes this sort stable. I think.

  CphxRenderDataInstance *temp;

  for ( int i = first + 1; i <= last; i++ )
  {
    if ( Instances[ i ]->RenderPriority > pivot )
    {
      p++;
      temp = Instances[ i ];
      Instances[ i ] = Instances[ p ];
      Instances[ p ] = temp;
    }
  }

  temp = Instances[ p ];
  Instances[ p ] = Instances[ first ];
  Instances[ first ] = temp;

  return p;
}

void SortRenderLayer( CphxRenderDataInstance **Instaces, int first, int last )
{
  if ( first < last )
  {
    int pivot = Pivot( Instaces, first, last );
    SortRenderLayer( Instaces, first, pivot - 1 );
    SortRenderLayer( Instaces, pivot + 1, last );
  }
}

void CphxScene::UpdateSceneGraph( int Clip, float t )
{
  D3DXMATRIX Root;
  D3DXMatrixIdentity( &Root );
  for ( int x = 0; x < LayerCount; x++ )
    RenderLayers[ x ]->RenderInstances.FreeArray();

  LightCount = 0;
  UpdateSceneGraph( Clip, t, Root, this, NULL );

  //calculate target directions
  for ( int x = 0; x < ObjectCount; x++ )
  {
    CphxObject *o = Objects[ x ];
    if ( o->Target )
    {
      o->TargetDirection = o->Target->WorldPosition - o->WorldPosition;
      D3DXVec3Normalize( &o->TargetDirection, &o->TargetDirection );
      o->SplineResults[ Spot_Direction_X ] = o->TargetDirection.x;
      o->SplineResults[ Spot_Direction_Y ] = o->TargetDirection.y;
      o->SplineResults[ Spot_Direction_Z ] = o->TargetDirection.z;
    }
  }

  CollectLights( this );

  for ( int x = 0; x < LayerCount; x++ )
    SortRenderLayer( RenderLayers[ x ]->RenderInstances.Array, 0, RenderLayers[ x ]->RenderInstances.ItemCount - 1 );

  //calculate inverse matrices for particle affectors
  for ( int x = 0; x < ObjectCount; x++ )
    if ( Objects[ x ]->ObjectType == Object_ParticleGravity ||
         Objects[ x ]->ObjectType == Object_ParticleDrag ||
         Objects[ x ]->ObjectType == Object_ParticleTurbulence ||
         Objects[ x ]->ObjectType == Object_ParticleVortex )
    {
      D3DXMATRIX m = Objects[ x ]->GetWorldMatrix();
      D3DXMatrixInverse( &Objects[ x ]->inverse, NULL, &m );
    }
}

void CphxScene::UpdateSceneGraph( int Clip, float t, D3DXMATRIX Root, CphxScene *RootScene, void *SubSceneData )
{
  for ( int x = 0; x < ObjectCount; x++ )
    if ( Objects[ x ]->Parent == NULL )
      Objects[ x ]->TraverseSceneGraph( Clip, t, Root, RootScene, SubSceneData );
}

void SetSamplers();

void CphxScene::CollectLights( CphxScene* sceneToCollectFrom )
{
  for ( int x = 0; x < sceneToCollectFrom->ObjectCount; x++ )
  {
    if ( LightCount >= 8 )
      return;

    CphxObject* object = sceneToCollectFrom->Objects[ x ];

    if ( object->ObjectType == Object_Light )
    {
      memcpy( &Lights[ LightCount ], &object->SplineResults[ Spline_Position_x ], sizeof( LIGHTDATA ) );
      if ( object->SplineResults[ Spline_Position_w ] != 0 )
      {
        //D3DXMATRIX rot;
        //D3DXVECTOR3 lightDir;
        //D3DXVec3TransformNormal( &lightDir, &D3DXVECTOR3( 0, 0, 1 ), D3DXMatrixRotationQuaternion( &rot, &object->RotationResult ) );

        Lights[ LightCount ].Position.x = object->WorldPosition.x;
        Lights[ LightCount ].Position.y = object->WorldPosition.y;
        Lights[ LightCount ].Position.z = object->WorldPosition.z;
      }
      else
      {
        Lights[ LightCount ].SpotDirection.x = object->WorldPosition.x;
        Lights[ LightCount ].SpotDirection.y = object->WorldPosition.y;
        Lights[ LightCount ].SpotDirection.z = object->WorldPosition.z;
        Lights[ LightCount ].Ambient.w = object->SplineResults[ Spline_Light_OrthoX ];
        Lights[ LightCount ].Diffuse.w = object->SplineResults[ Spline_Light_OrthoY ];
      }
      LightCount++;
    }
  }
}

void CphxScene::Render( bool ClearColor, bool ClearZ, int cubeResolution )
{
  SetSamplers();

  //collect lights

  //calculate needed matrices
  D3DXMatrixInverse( &phxIViewMatrix, NULL, &phxViewMatrix );
  D3DXMatrixInverse( &phxIProjectionMatrix, NULL, &phxProjectionMatrix );

  //upload scene level data

  for ( int x = 0; x < LayerCount; x++ )
  {
    RenderLayers[ x ]->Descriptor->SetEnvironment( ClearColor, ClearZ, cubeResolution );

    D3D11_MAPPED_SUBRESOURCE map;
    phxContext->Map( SceneDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map );
    unsigned char* m = (unsigned char*)map.pData;

    float LightCountData[ 4 ];
    LightCountData[ 0 ] = (float)LightCount;

    memcpy( m, &phxViewMatrix, sizeof( phxViewMatrix ) ); m += sizeof( phxViewMatrix );
    memcpy( m, &phxProjectionMatrix, sizeof( phxProjectionMatrix ) ); m += sizeof( phxProjectionMatrix );
    memcpy( m, &phxCameraPos, sizeof( phxCameraPos ) ); m += sizeof( phxCameraPos );
    memcpy( m, &LightCountData, sizeof( LightCountData ) ); m += sizeof( LightCountData );
    memcpy( m, Lights, sizeof( LIGHTDATA )*MAX_LIGHT_COUNT ); m += sizeof( LIGHTDATA )*MAX_LIGHT_COUNT;

    float RTResolution[ 4 ];
    //memset(RTResolution, 16, 0);

    if ( RenderLayers[ x ]->Descriptor->TargetCount )
    {
      RTResolution[ 0 ] = (float)RenderLayers[ x ]->Descriptor->Targets[ 0 ]->XRes;
      RTResolution[ 1 ] = (float)RenderLayers[ x ]->Descriptor->Targets[ 0 ]->YRes;
      RTResolution[ 2 ] = 1 / RTResolution[ 0 ];
      RTResolution[ 3 ] = 1 / RTResolution[ 1 ];
    }

    memcpy( m, RTResolution, 16 ); m += 16;

    memcpy( m, &phxIViewMatrix, sizeof( phxIViewMatrix ) ); m += sizeof( phxIViewMatrix );
    memcpy( m, &phxIProjectionMatrix, sizeof( phxIProjectionMatrix ) ); m += sizeof( phxIProjectionMatrix );


    phxContext->Unmap( SceneDataBuffer, 0 );

    for ( int y = 0; y < 2; y++ )
    {
      ID3D11Buffer *Buffer = y ? ObjectMatrixBuffer : SceneDataBuffer;
      phxContext->VSSetConstantBuffers( y, 1, &Buffer );
      phxContext->GSSetConstantBuffers( y, 1, &Buffer );
      phxContext->PSSetConstantBuffers( y, 1, &Buffer );
    }


    for ( int y = 0; y < RenderLayers[ x ]->RenderInstances.NumItems(); y++ )
      RenderLayers[ x ]->RenderInstances[ y ]->Render();

#ifdef PHX_VOLUMETRIC_RENDERTARGETS
    if ( RenderLayers[ x ]->Descriptor->VoxelizerLayer )
          phxContext->GenerateMips( phxTexture3DResourceView );

#endif
    RenderLayers[ x ]->Descriptor->GenMipmaps();
  }
}

void CphxScene::AddRenderDataInstance( CphxRenderLayerDescriptor *Layer, CphxRenderDataInstance *RDI )
{
  for ( int x = 0; x < LayerCount; x++ )
    if ( RenderLayers[ x ]->Descriptor == Layer )
    {
      RenderLayers[ x ]->RenderInstances.Add( RDI );
      return;
    }
#ifndef PHX_MINIMAL_BUILD
  delete RDI;
#endif
}

//void CphxScene::SimulateParticles(float t, bool updatebuffer)
//{
//	for (int x = 0; x < Objects.NumItems(); x++)
//		if (Objects[x]->ObjectType == Object_ParticleEmitterCPU)
//		{
//			CphxObject_ParticleEmitter_CPU *p = (CphxObject_ParticleEmitter_CPU*)Objects[x];
//			if (p->VertexBuffer)
//				p->UpdateParticles(t, updatebuffer);
//		}
//}

void CphxObject::TraverseSceneGraph( int Clip, float t, D3DXMATRIX CurrentMatrix, CphxScene *RootScene, void *SubSceneData )
{
  D3DXMATRIX m = CurrentMatrix;
  CalculateAnimation( Clip, t );

  //#ifdef COMPILED_FOR_TOOL
  //	D3DXMatrixIdentity(&m);
  //	if (Clip >= 0 && Clip<Clips.ItemCount)
  //#endif
  //	{
  //		//update animation
  //#ifdef COMPILED_FOR_TOOL
  //		Clips[Clip]->StoreValues();
  //		if (SubSceneDepth>0)
  //#endif
  //			Clips[Clip]->CalculateValue(t);
  //
  //		D3DXMatrixMultiply(&m, &CurrentMatrix, &Clips[Clip]->GetTransformationMatrix());
  //	}

  D3DXMATRIX prs;
  D3DXMatrixTransformation( &prs, NULL, NULL, (D3DXVECTOR3*)&SplineResults[ Spline_Scale_x ], NULL, &RotationResult, (D3DXVECTOR3*)&SplineResults[ Spline_Position_x ] );
  D3DXMatrixMultiply( &m, &prs, &CurrentMatrix );

  prevMatrix = currMatrix;
  currMatrix = m;

  //move calculated position to world space
  D3DXVECTOR4 v;
  D3DXVec3Transform( &v, (D3DXVECTOR3*)&SplineResults[ Spline_Position_x ], &CurrentMatrix );
  WorldPosition = *(D3DXVECTOR3*)&v;

  //create render data instances for the object based on the current clip and position here, also handles subscenes correctly
  CreateRenderDataInstances( Clip, m, RootScene, SubSceneData ? SubSceneData : ToolData );

  //#ifdef COMPILED_FOR_TOOL
  //	if (Clip >= 0 && Clip < Clips.ItemCount)
  //		Clips[Clip]->RestoreValues();
  //#endif

  //go down the hierarchy
  for ( int x = 0; x < ChildCount; x++ )
    Children[ x ]->TraverseSceneGraph( Clip, t, m, RootScene, SubSceneData );
}

void CphxObject::CalculateAnimation( int Clip, float t )
{
  //need to set defaults somewhere, might as well do it here :)
  SplineResults[ Spline_Scale_x ] = 1;
  SplineResults[ Spline_Scale_y ] = 1;
  SplineResults[ Spline_Scale_z ] = 1;
  SplineResults[ Spline_Light_DiffuseR ] = 1;
  SplineResults[ Spline_Light_DiffuseG ] = 1;
  SplineResults[ Spline_Light_DiffuseB ] = 1;
  SplineResults[ Spline_Light_SpecularR ] = 1;
  SplineResults[ Spline_Light_SpecularG ] = 1;
  SplineResults[ Spline_Light_SpecularB ] = 1;
  SplineResults[ Spline_Camera_FOV ] = 1;
  SplineResults[ Spline_Particle_EmissionVelocity ] = 1;
  SplineResults[ Spline_AffectorPower ] = 1;
  SplineResults[ Spline_Particle_Scale ] = 1;
  SplineResults[ Spline_Particle_Stretch_X ] = 1;
  SplineResults[ Spline_Particle_Stretch_Y ] = 1;

  SplineResults[ Spline_Particle_EmissionPerSecond ] = 25;
  SplineResults[ Spline_Particle_Life ] = 10;

  SplineResults[ Spline_Light_OrthoX ] = 1;
  SplineResults[ Spline_Light_OrthoY ] = 1;

  for ( int x = 0; x < Clips[ Clip ]->SplineCount; x++ )
  {
    CphxClipSpline *s = Clips[ Clip ]->Splines[ x ];
    s->Spline->CalculateValue( t );
    SplineResults[ s->Type ] = s->Spline->Value[ 0 ];
    if ( s->Type == Spline_Rotation ) RotationResult = s->Spline->GetQuaternion();
  }

  Clips[ Clip ]->MaterialSplines->CalculateValues( t );
  SubSceneTarget = Clips[ Clip ]->SubSceneTarget;

  if ( ObjectType == Object_ParticleTurbulence )
  {
    RandSeed = Clips[ Clip ]->RandSeed;
    TurbulenceFrequency = Clips[ Clip ]->TurbulenceFrequency;
  }
}

D3DXMATRIX CphxObject::GetWorldMatrix()
{
  D3DXMATRIX prs;
  D3DXMatrixTransformation( &prs, NULL, NULL, (D3DXVECTOR3*)&SplineResults[ Spline_Scale_x ], NULL, &RotationResult, (D3DXVECTOR3*)&SplineResults[ Spline_Position_x ] );

#ifdef PHX_HAS_SCENE_OBJECT_HIERARCHIES
  if ( Parent )
    D3DXMatrixMultiply( &prs, &prs, &Parent->GetWorldMatrix() );
#endif

  return prs;
}

#ifdef PHX_OBJ_MODEL
void CphxObject_Model::CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData )
{
  Model->CreateRenderDataInstances( Clips[ Clip ], m, RootScene, SubSceneData ? SubSceneData : ToolData );
}
#endif

#ifdef PHX_OBJ_SUBSCENE
void CphxObject_SubScene::CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData )
{
  D3DXMATRIX mtx = m;
  D3DXMATRIX prs;
  D3DXMatrixTransformation( &prs, NULL, NULL, (D3DXVECTOR3*)&SplineResults[ Spline_Scale_x ], NULL, &RotationResult, (D3DXVECTOR3*)&SplineResults[ Spline_Position_x ] );

#ifndef PHX_MINIMAL_BUILD
  int clipId = max( 0, min( SubSceneTarget->ClipCount - 1, (int)SplineResults[ Spline_SubScene_Clip ] ) );
#endif

  int repeatCount = max( 1, (int)SplineResults[ Spline_SubScene_RepeatCount ] );
  float timeOffset = SplineResults[ Spline_SubScene_RepeatTimeOffset ];

  for ( int x = 0; x < repeatCount; x++ )
  {
    SubSceneTarget->UpdateSceneGraph(
#ifndef PHX_MINIMAL_BUILD
      clipId,
#else
      (int)SplineResults[ Spline_SubScene_Clip ],
#endif    
      fabs( fmod( SplineResults[ Spline_SubScene_Time ] + timeOffset * x, 1.0f ) ), mtx, RootScene, SubSceneData ? SubSceneData : ToolData );

    D3DXMatrixMultiply( &mtx, &prs, &mtx );
  }

  RootScene->CollectLights( SubSceneTarget );
}
#endif

//void CphxObject_ParticleEmitter::CreateRenderDataInstances(int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData)
//{
//
//}

#ifdef PHX_OBJ_EMITTERCPU
void CphxObject_ParticleEmitter_CPU::SpawnParticle( float t, D3DXMATRIX &m, D3DXMATRIX &o, float mt )
{
  int idx = -1;
  int minlife = Particles[ 0 ].LifeLeft;
  int particlecount = 1 << BufferSize;
  for ( int x = 0; x < particlecount; x++ )
  {
    if ( Particles[ x ].LifeLeft <= 0 )
    {
      idx = x;
      break;
    }
    if ( Particles[ x ].LifeLeft < minlife )
    {
      idx = x;
      minlife = Particles[ x ].LifeLeft;
    }
  }

  if ( idx == -1 )
  {
    if ( Aging )
      idx = 0;
    else
      return;
  }

  Particles[ idx ].MaxLife = Particles[ idx ].LifeLeft = Aging ? (int)( ( SplineResults[ Spline_Particle_Life ] + ( rand() / (float)RAND_MAX )*SplineResults[ Spline_Particle_LifeChaos ] ) * PARTICLEENGINE_FRAMERATE ) : 1;

  do
  {
    Particles[ idx ].Position = D3DXVECTOR3( rand() / (float)RAND_MAX - 0.5f, rand() / (float)RAND_MAX - 0.5f, rand() / (float)RAND_MAX - 0.5f );
  } while ( !( EmitterType != 1 /*sphere*/ || D3DXVec3LengthSq( &Particles[ idx ].Position ) < 0.25 ) );

  Particles[ idx ].RotationAxis = D3DXVECTOR3( rand() / (float)RAND_MAX - 0.5f, rand() / (float)RAND_MAX - 0.5f, rand() / (float)RAND_MAX - 0.5f );
  D3DXVec3Normalize( &Particles[ idx ].RotationAxis, &Particles[ idx ].RotationAxis );

  //adjust for inner radius
  D3DXVECTOR3 outerboundarypos;
  D3DXVECTOR3 originalpos = Particles[ idx ].Position;

  float originallength = D3DXVec3Length( &originalpos );

  float poslength = max( max( fabs( originalpos.x ), fabs( originalpos.y ) ), fabs( originalpos.z ) ); //cube adjust ratio
  if ( EmitterType == 1 ) poslength = originallength; //sphere adjust ratio
  outerboundarypos = originalpos*0.5f / poslength;

  float outerlength = D3DXVec3Length( &outerboundarypos );
  float r = lerp( InnerRadius / 255.0f, 1, originallength / outerlength );

  Particles[ idx ].Position = outerboundarypos*r;

  //transform particle

  D3DXVECTOR4 va, vb, vc;
  D3DXVec3Transform( &va, &Particles[ idx ].Position, &m );
  D3DXVec3Transform( &vb, &Particles[ idx ].Position, &o );
  D3DXVec4Lerp( &vc, &vb, &va, 1 - mt );
  Particles[ idx ].Position = D3DXVECTOR3( vc );

  if ( !Target )
  {
    outerboundarypos = D3DXVECTOR3( rand() / (float)RAND_MAX - 0.5f, rand() / (float)RAND_MAX - 0.5f, rand() / (float)RAND_MAX - 0.5f );
    D3DXVec3Normalize( &TargetDirection, &outerboundarypos );
  }

  Particles[ idx ].Velocity = TargetDirection*0.01f*( SplineResults[ Spline_Particle_EmissionVelocity ] + ( rand() / (float)RAND_MAX )*SplineResults[ Spline_Particle_EmissionVelocityChaos ] );
  Particles[ idx ].RotationSpeed = SplineResults[ Spline_Particle_EmissionRotation ] + ( rand() / (float)RAND_MAX )*SplineResults[ Spline_Particle_EmissionRotationChaos ];
  if ( rand() > RAND_MAX / 2 && TwoDirRotate ) Particles[ idx ].RotationSpeed *= -1;
  Particles[ idx ].ScaleChaos = max(0, 1 + ( (rand() / (float)RAND_MAX) * 2 - 1 )*SplineResults[ Spline_Particle_ScaleChaos ] );
  Particles[ idx ].Scale = SplineResults[ Spline_Particle_Scale ] * Particles[ idx ].ScaleChaos;
  Particles[ idx ].StretchX = SplineResults[ Spline_Particle_Stretch_X ] * Particles[ idx ].Scale;
  Particles[ idx ].StretchY = SplineResults[ Spline_Particle_Stretch_Y ] * Particles[ idx ].Scale;

  Particles[ idx ].Position += Particles[ idx ].Velocity*t; //interpolate for smooth emission of particles
  Particles[ idx ].Rotation = Particles[ idx ].RotationSpeed*t;
  if ( RandRotate )
    Particles[ idx ].Rotation = ( rand() / (float)RAND_MAX )*360.0f;
  Particles[ idx ].Chaos = rand() / (float)RAND_MAX;
}

int _cdecl ParticleSorter( const void *a, const void *b )
{
  float d = ( (PHXPARTICLEDISTANCE*)a )->Dist - ( (PHXPARTICLEDISTANCE*)b )->Dist;
  return d < 0 ? -1 : 1;
}

static CphxObject_ParticleAffector *affectors[ 256 ];

void CphxObject_ParticleEmitter_CPU::UpdateParticles( float elapsedtime, bool updatebuffer )
{
  //D3DXMATRIX wm = GetWorldMatrix();

  int particlecount = 1 << BufferSize;
  Ticks += elapsedtime * PARTICLEENGINE_FRAMERATE;

  //update particle system

  int objectcount = 1;
  int affectorcount = 0;

#ifdef PHX_HAS_SCENE_OBJECT_HIERARCHIES
  for ( int x = 0; x < Scene->ObjectCount; x++ )
  {
    if ( Scene->Objects[ x ]->Parent == this )
      objectcount++;
    if ( Scene->Objects[ x ]->ObjectType == Object_ParticleGravity ||
         Scene->Objects[ x ]->ObjectType == Object_ParticleDrag ||
         Scene->Objects[ x ]->ObjectType == Object_ParticleTurbulence ||
         Scene->Objects[ x ]->ObjectType == Object_ParticleVortex )
      affectors[ affectorcount++ ] = (CphxObject_ParticleAffector*)Scene->Objects[ x ];
  }
#endif

  D3DXMATRIX *matrices = new D3DXMATRIX[ objectcount ];
  D3DXMATRIX *oldmatrices = new D3DXMATRIX[ objectcount ];
  matrices[ 0 ] = currMatrix;// GetWorldMatrix();
  oldmatrices[ 0 ] = prevMatrix;// GetWorldMatrix();

#ifdef PHX_HAS_SCENE_OBJECT_HIERARCHIES
  int cnt = 1;

  for ( int x = 0; x < Scene->ObjectCount; x++ )
    if ( Scene->Objects[ x ]->Parent == this )
    {
      matrices[ cnt ] = Scene->Objects[ x ]->currMatrix;// GetWorldMatrix();
      oldmatrices[ cnt++ ] = Scene->Objects[ x ]->prevMatrix;
    }
#endif

  while ( Ticks >= 1 )
  {
    //age and move particles
    for ( int y = 0; y < particlecount; y++ )
    {
      if ( Aging ) Particles[ y ].LifeLeft -= 1;
      if ( Particles[ y ].LifeLeft > 0 )
      {
        //update velocity based on affecting forces
        for ( int x = 0; x < affectorcount; x++ )
          if ( affectors[ x ]->ParticleInside( Particles[ y ].Position ) )
            Particles[ y ].Velocity += affectors[ x ]->GetForce( &Particles[ y ] );

        //update position and calculate collisions
        Particles[ y ].Position += Particles[ y ].Velocity;
        Particles[ y ].Rotation += Particles[ y ].RotationSpeed;
      }
    }

    //spawn new particles
    if ( SplineResults[ Spline_Particle_EmissionPerSecond ] > 0 )
    {
      int cnt = 1 + (int)( ( 1 - fmod( EmissionFraction, 1.0f ) ) / ( PARTICLEENGINE_FRAMERATE / ( SplineResults[ Spline_Particle_EmissionPerSecond ] * objectcount ) ) );
      int idx = 0;
      while ( EmissionFraction < 1 )
      {
        int id = ( objIdxMod++ ) % objectcount;
        SpawnParticle( EmissionFraction - (int)EmissionFraction, matrices[ id ], oldmatrices[ id ], idx / (float)cnt );
        EmissionFraction += PARTICLEENGINE_FRAMERATE / ( SplineResults[ Spline_Particle_EmissionPerSecond ] * objectcount );
        idx++;
      }
    }

    EmissionFraction--;
    Ticks--;
  }

  delete[] oldmatrices;
  delete[] matrices;

  if ( !updatebuffer ) return;

  //sort particles
  LiveCount = 0;

  D3DXVECTOR4 cd1, cd2;
  D3DXMATRIX mx;
  D3DXMatrixTranspose( &mx, &phxViewMatrix );
  D3DXVec3Transform( &cd1, &D3DXVECTOR3( 0, 0, 1 ), &mx );
  D3DXVec3Transform( &cd2, &D3DXVECTOR3( 0, 0, 0 ), &mx );
  D3DXVECTOR3 camdir = *( D3DXVECTOR3* )&( cd1 - cd2 );

  for ( int y = 0; y < particlecount; y++ )
  {
    if ( Particles[ y ].LifeLeft > 0 )
    {
      DistanceBuffer[ LiveCount ].Idx = y;
#ifdef PHX_HAS_PARTICLE_SORTING
      if ( Sort )
        DistanceBuffer[ LiveCount ].Dist = Particles[ y ].Position.x*camdir.x + Particles[ y ].Position.y*camdir.y + Particles[ y ].Position.z*camdir.z;
#endif
      LiveCount++;
    }
  }

#ifdef PHX_HAS_PARTICLE_SORTING
  if ( Sort )
    qsort( DistanceBuffer, LiveCount, sizeof( PHXPARTICLEDISTANCE ), ParticleSorter );
#endif

  //create display buffer

#ifdef PHX_HAS_STANDARD_PARTICLES
  if ( !VertexBuffer )
    return;
  D3D11_MAPPED_SUBRESOURCE ms;
  phxContext->Map( VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms );

  float *Data = (float*)ms.pData;

  for ( int y = 0; y < LiveCount; y++ )
  {
#ifdef PHX_HAS_PARTICLE_SORTING
    int idx = DistanceBuffer[ y ].Idx;
#else
    int idx = y;
#endif

    D3DXVECTOR3 v = Particles[ idx ].Position + Particles[ idx ].Velocity*Ticks; //interpolate to current frame
    Data[ 0 ] = v.x;
    Data[ 1 ] = v.y;
    Data[ 2 ] = v.z;
    Data[ 3 ] = 1;
    Data[ 4 ] = Aging ? ( Particles[ idx ].LifeLeft - Ticks ) / (float)Particles[ idx ].MaxLife : 1;
    Data[ 5 ] = Particles[ idx ].Rotation + Particles[ idx ].RotationSpeed*Ticks;
    Data[ 6 ] = Particles[ idx ].Chaos;
    Data[ 7 ] = 0;// y / (float)LiveCount;
    Data += 8;
  }

  phxContext->Unmap( VertexBuffer, NULL );
#endif
}

void CphxObject_ParticleEmitter_CPU::CreateRenderDataInstances( int Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *SubSceneData )
{
  Clips[ Clip ]->MaterialSplines->ApplyToParameters( this );

  //update dynamic material variables

  int passid = 0;

#ifdef PHX_HAS_STANDARD_PARTICLES
  if ( Material )
  {
    for ( int x = 0; x < Material->TechCount; x++ )
      for ( int y = 0; y < Material->Techniques[ x ]->PassCount; y++ )
        Material->Techniques[ x ]->CollectAnimatedData( MaterialState[ passid++ ], y );
  }

  for ( int x = 0; x < passid; x++ )
    MaterialState[ x ]->Textures[ 0 ] = SplineTextureView;

#if defined PHX_HAS_MESH_PARTICLES || defined PHX_HAS_SUBSCENE_PARTICLES
  if (
#ifdef PHX_HAS_MESH_PARTICLES
    !ObjectToEmit
#endif
#ifdef PHX_HAS_SUBSCENE_PARTICLES
#ifdef PHX_HAS_MESH_PARTICLES
    &&
#endif
    !SceneToEmit
#endif
       )
#endif
  {
    Material->CreateRenderDataInstances( MaterialState, RootScene, VertexBuffer, LiveCount );
  }
#if defined PHX_HAS_MESH_PARTICLES || defined PHX_HAS_SUBSCENE_PARTICLES
  else
#endif
#endif
#if defined PHX_HAS_MESH_PARTICLES || defined PHX_HAS_SUBSCENE_PARTICLES
  {
    int particlecount = 1 << BufferSize;
    for ( int y = 0; y < particlecount; y++ )
    {
      if ( Particles[ y ].LifeLeft > 0 )
      {
        D3DXMATRIX m;
        D3DXQUATERNION q;

        if ( !RotateToDirection )
        {
          D3DXQuaternionRotationAxis( &q, &Particles[ y ].RotationAxis, ( Particles[ y ].Rotation + Particles[ y ].RotationSpeed * Ticks ) * 3.1415f / 180.0f );
        }
        else
        {
          D3DXVECTOR3 xd = Particles[ y ].Velocity;
          D3DXVECTOR3 yd, zd;
          D3DXVec3Normalize( &xd, &xd );
          D3DXVECTOR3 up( 0, 1, 0 );
          D3DXVec3Normalize( &zd, D3DXVec3Cross( &zd, &xd, &up ) );
          D3DXVec3Normalize( &yd, D3DXVec3Cross( &yd, &zd, &xd ) );
          D3DXMATRIX mx;
          D3DXMatrixIdentity( &mx );
          *( (D3DXVECTOR3*)mx.m[ 0 ] ) = xd;
          *( (D3DXVECTOR3*)mx.m[ 1 ] ) = yd;
          *( (D3DXVECTOR3*)mx.m[ 2 ] ) = zd;
          D3DXQuaternionRotationMatrix( &q, &mx );
          D3DXQUATERNION q2;
          D3DXQuaternionRotationAxis( &q2, &D3DXVECTOR3( 1, 0, 0 ), ( Particles[ y ].Rotation + Particles[ y ].RotationSpeed * Ticks ) * 3.1415f / 180.0f );
          D3DXQuaternionMultiply( &q, &q2, &q );
        }

        float life = Aging ? ( 1 - ( ( Particles[ y ].LifeLeft - Ticks ) / (float)Particles[ y ].MaxLife ) ) : 0.0f;

        float scale = 1;
        float scalex = 1;
        float scaley = 1;
        for ( int x = 0; x < Clips[ Clip ]->SplineCount; x++ )
        {
          if ( Clips[ Clip ]->Splines[ x ]->Type == Spline_Particle_Scale )
          {
            Clips[ Clip ]->Splines[ x ]->Spline->CalculateValue( life );
            scale = max(0, Particles[ y ].ScaleChaos) * Clips[ Clip ]->Splines[ x ]->Spline->Value[0];
          }
          if ( Clips[ Clip ]->Splines[ x ]->Type == Spline_Particle_Stretch_X )
          {
            Clips[ Clip ]->Splines[ x ]->Spline->CalculateValue( life );
            scalex = Clips[ Clip ]->Splines[ x ]->Spline->Value[ 0 ];
          }
          if ( Clips[ Clip ]->Splines[ x ]->Type == Spline_Particle_Stretch_Y )
          {
            Clips[ Clip ]->Splines[ x ]->Spline->CalculateValue( life );
            scaley = Clips[ Clip ]->Splines[ x ]->Spline->Value[ 0 ];
          }
        }

        D3DXVECTOR3 interpolatedPosition = Particles[ y ].Position + Particles[ y ].Velocity * Ticks;

        D3DXMatrixTransformation( &m, nullptr, nullptr, &D3DXVECTOR3( scale * scalex, scale * scaley, scale ), nullptr, &q, &interpolatedPosition );
#ifdef PHX_HAS_SUBSCENE_PARTICLES
        if ( SceneToEmit )
        {
          SceneToEmit->UpdateSceneGraph(0, life, m, RootScene, SubSceneData ? SubSceneData : ToolData );
        }
        else
#endif
#ifdef PHX_HAS_MESH_PARTICLES
        {
          Clips[ Clip ]->MaterialSplines->CalculateValues( life );
          ObjectToEmit->CreateRenderDataInstances( Clips[ Clip ], m, RootScene, SubSceneData ? SubSceneData : ToolData );
        }
#endif
      }
    }
  }
#endif
}

#ifdef PHX_HAS_STANDARD_PARTICLES
float *texturedata = new float[ 2048 * 64 ];

void CphxObject_ParticleEmitter_CPU::UpdateSplineTexture()
{
  //free texture
  if ( SplineTextureView ) SplineTextureView->Release();
  if ( SplineTexture ) SplineTexture->Release();

  int splinecnt = 0;
  for ( int x = 0; x < Clips[ 0 ]->MaterialSplines->SplineCount; x++ )
  {
    CphxMaterialSpline *s = Clips[ 0 ]->MaterialSplines->Splines[ x ];
    if ( s->Target->Type == PARAM_PARTICLELIFEFLOAT ) splinecnt++;
  }

  //create texture data
  //float *texturedata = new float[2048 * splinecnt];
  splinecnt = 0;
  for ( int x = 0; x < Clips[ 0 ]->MaterialSplines->SplineCount; x++ )
  {
    CphxMaterialSpline *s = Clips[ 0 ]->MaterialSplines->Splines[ x ];
    if ( s->Target->Type == PARAM_PARTICLELIFEFLOAT )
    {
      for ( int z = 0; z < 2048; z++ )
      {
        s->Splines[ 0 ]->CalculateValue( z / 2048.0f );
        texturedata[ z + splinecnt * 2048 ] = s->Splines[ 0 ]->Value[ 0 ];
      }
      splinecnt++;
    }
  }

  //create texture

  D3D11_TEXTURE2D_DESC tex = { 2048,splinecnt,1,1,DXGI_FORMAT_R32_FLOAT,1,0,D3D11_USAGE_DEFAULT,D3D11_BIND_SHADER_RESOURCE,0,0 };
  D3D11_SUBRESOURCE_DATA data = { texturedata, 2048 * 4, 0 };

  phxDev->CreateTexture2D( &tex, &data, &SplineTexture );
  phxDev->CreateShaderResourceView( SplineTexture, NULL, &SplineTextureView );

#ifdef DEBUGINTOFILE
  if ( !SplineTexture || !SplineTextureView )
    DEBUGLOG( "*** Failed to create Spline texture" );
#endif

  //cleanup
  //delete[] texturedata;
}
#endif

void CphxObject_ParticleEmitter_CPU::ResetParticles()
{
  objIdxMod = 0;
  D3DXMATRIX wm = GetWorldMatrix();
  currMatrix = prevMatrix = wm;

  memset( Particles, 0, sizeof( PHXPARTICLE )*( 1 << BufferSize ) );
  int pcount = ( ( 1 << BufferSize )*StartCount ) / 255;
  srand( RandSeed );

  DEBUGLOG( "Spawning %d particles", pcount );

  int objectcount = 1;
#ifdef PHX_HAS_SCENE_OBJECT_HIERARCHIES
  for ( int x = 0; x < Scene->ObjectCount; x++ )
    if ( Scene->Objects[ x ]->Parent == this )
      objectcount++;
#endif

  D3DXMATRIX *matrices = new D3DXMATRIX[ objectcount ];
  D3DXMATRIX *oldmatrices = new D3DXMATRIX[ objectcount ];
  matrices[ 0 ] = currMatrix;// GetWorldMatrix();
  oldmatrices[ 0 ] = prevMatrix;// GetWorldMatrix();

#ifdef PHX_HAS_SCENE_OBJECT_HIERARCHIES
  int cnt = 1;

  for ( int x = 0; x < Scene->ObjectCount; x++ )
    if ( Scene->Objects[ x ]->Parent == this )
    {
      matrices[ cnt ] = Scene->Objects[ x ]->currMatrix;// GetWorldMatrix();
      oldmatrices[ cnt++ ] = Scene->Objects[ x ]->prevMatrix;
    }
#endif

  for ( int x = 0; x < pcount; x++ )
  {
    int id = ( objIdxMod++ ) % objectcount;
    SpawnParticle( 0, matrices[ id ], oldmatrices[ id ], 0 );
  }

  delete[] oldmatrices;
  delete[] matrices;

  Ticks = 0;

  UpdateParticles( 0 );
}
#endif

/////////////////////////
// particle affectors
////////////////////////

#if defined PHX_OBJ_PARTICLEDRAG || defined PHX_OBJ_PARTICLEGRAVITY || defined PHX_OBJ_PARTICLETURBULENCE || defined PHX_OBJ_PARTICLEVORTEX || defined PHX_OBJ_EMITTERCPU
bool CphxObject_ParticleAffector::ParticleInside( D3DXVECTOR3 v )
{
  D3DXVECTOR4 pos;
  D3DXVec3Transform( &pos, &v, &inverse );

  if ( AreaType )
    return pos.x<0.5f && pos.x>-0.5f && pos.y<0.5f && pos.y>-0.5f && pos.z<0.5f && pos.z>-0.5f;

  return true;

  /*
    if ( AreaType == 0 ) return 1;
    if ( AreaType == 1 ) return ( pos.x*pos.x + pos.y*pos.y + pos.z*pos.z )<0.25f;
    if ( AreaType == 2 ) return pos.x<0.5f && pos.x>-0.5f && pos.y<0.5f && pos.y>-0.5f && pos.z<0.5f && pos.z>-0.5f;
    return pos.y<0.5f && pos.y>-0.5f && pos.x*pos.x + pos.z*pos.z<0.25f;
  */
}
#endif

#ifdef PHX_OBJ_PARTICLETURBULENCE
void CphxObject_ParticleTurbulence::InitKernel()
{
  if ( RandSeed == calculatedKernelSeed )
    return;

  srand( RandSeed );
  calculatedKernelSeed = RandSeed;
  for ( int x = 0; x < 32; x++ )
    for ( int y = 0; y < 32; y++ )
      for ( int z = 0; z < 32; z++ )
      {
        for ( int i = 0; i < 3; i++ )
          Kernel[ x ][ y ][ z ][ i ] = (float)( rand() / (float)RAND_MAX ) - 0.5f;
        D3DXVec3Normalize( &Kernel[ x ][ y ][ z ], &Kernel[ x ][ y ][ z ] );
      }
}

D3DXVECTOR3 Lerp( const D3DXVECTOR3& a, const D3DXVECTOR3& b, float t )
{
  return a + ( b - a )*t;
}

D3DXVECTOR3 CphxObject_ParticleTurbulence::SampleKernel( const D3DXVECTOR4& Pos )
{
  int v[ 3 ];
  D3DXVECTOR3 f;
  D3DXVECTOR3 area[ 2 ][ 2 ][ 2 ];

  for ( int x = 0; x < 3; x++ )
  {
    v[ x ] = (int)Pos[ x ];
    if ( Pos[ x ] < 0 )
      v[ x ] -= 1;
    f[ x ] = ( Pos[ x ] - v[ x ] );
  }

  for ( int x = 0; x < 2; x++ )
    for ( int y = 0; y < 2; y++ )
      for ( int z = 0; z < 2; z++ )
        area[ x ][ y ][ z ] = Kernel[ ( v[ 0 ] + x ) & 31 ][ ( v[ 1 ] + y ) & 31 ][ ( v[ 2 ] + z ) & 31 ];

  D3DXVECTOR3 v1 = Lerp( area[ 0 ][ 0 ][ 0 ], area[ 1 ][ 0 ][ 0 ], f.x );
  D3DXVECTOR3 v2 = Lerp( area[ 0 ][ 1 ][ 0 ], area[ 1 ][ 1 ][ 0 ], f.x );

  D3DXVECTOR3 v3 = Lerp( area[ 0 ][ 0 ][ 1 ], area[ 1 ][ 0 ][ 1 ], f.x );
  D3DXVECTOR3 v4 = Lerp( area[ 0 ][ 1 ][ 1 ], area[ 1 ][ 1 ][ 1 ], f.x );

  D3DXVECTOR3 v5 = Lerp( v1, v2, f.y );
  D3DXVECTOR3 v6 = Lerp( v3, v4, f.y );

  D3DXVECTOR3 res = Lerp( v5, v6, f.z );
  D3DXVec3Normalize( &res, &res );

  return res;
}


D3DXVECTOR3 CphxObject_ParticleTurbulence::GetForce( PHXPARTICLE *p )
{
  InitKernel();
  D3DXVECTOR4 Pos;
  D3DXVec3Transform( &Pos, &p->Position, &inverse );
  D3DXVECTOR3 v3 = SampleKernel( Pos*TurbulenceFrequency ) + SampleKernel( Pos*( TurbulenceFrequency *2.0f ) )*( 1 / 2.0f ) + SampleKernel( Pos*( TurbulenceFrequency *4.0f ) )*( 1 / 4.0f );
  D3DXVec3Normalize( &v3, &v3 );
  return v3*( SplineResults[ Spline_AffectorPower ] / 100.0f );
}
#endif

#ifdef PHX_OBJ_PARTICLEDRAG
D3DXVECTOR3 CphxObject_ParticleDrag::GetForce( PHXPARTICLE *p )
{
  return p->Velocity*( -SplineResults[ Spline_AffectorPower ] );
}
#endif

#ifdef PHX_OBJ_PARTICLEGRAVITY
D3DXVECTOR3 CphxObject_ParticleGravity::GetForce( PHXPARTICLE *p )
{
  D3DXVECTOR3 pos = WorldPosition;

  if ( Directional )
  {
    D3DXVec3Normalize( &pos, &pos );
    return pos*( SplineResults[ Spline_AffectorPower ] / 100.0f );
  }

  D3DXVECTOR3 v = pos - p->Position;
  float l = D3DXVec3Length( &v );
  return v*( SplineResults[ Spline_AffectorPower ] / ( l * l * l ) / 100.0f ); //third /l actually normalizes the vector
}
#endif

#ifdef PHX_OBJ_PARTICLEVORTEX
D3DXVECTOR3 CphxObject_ParticleVortex::GetForce( PHXPARTICLE* p )
{
  float pwr = SplineResults[ Spline_AffectorPower ];

  D3DXVECTOR3 pos = WorldPosition;
  D3DXVECTOR3 v = pos - p->Position;
  D3DXVECTOR4 axis;
  D3DXVECTOR3 force;
  D3DXVec3Transform( &axis, &D3DXVECTOR3( 0, 1, 0 ), &GetWorldMatrix() );
  D3DXVec3Normalize( (D3DXVECTOR3*)&axis, (D3DXVECTOR3*)&axis );
  D3DXVec3Normalize( &v, &v );
  D3DXVec3Cross( &force, (D3DXVECTOR3*)&axis, &v );
  return force * pwr;
}
#endif
