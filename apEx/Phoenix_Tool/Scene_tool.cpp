#include "BasePCH.h"
#include "Scene_tool.h"
#include "apxProject.h"

CArray<SceneObjCopyData> sceneCopyData;

CCoreBlendState *DefaultBlendState = NULL;
CCoreRasterizerState *DefaultRasterState = NULL;
CCoreDepthStencilState *DefaultDepthState = NULL;
CCoreVertexShader *DefaultVertexShader = NULL;
CCorePixelShader *DefaultPixelShader = NULL;

void SetStatusbarText( CString Text );

void InitDefaultStates( CCoreDevice *Device )
{
  DefaultBlendState = Device->CreateBlendState();
  DefaultRasterState = Device->CreateRasterizerState();
  DefaultDepthState = Device->CreateDepthStencilState();
  DefaultBlendState->Update();
  DefaultRasterState->Update();
  DefaultDepthState->Update();

  LPCSTR  shader = _T(
    "cbuffer b : register(b0){float4x4 viewmat;float4x4 projmat;}\n"
    "cbuffer c : register(b1){float4x4 worldmat;}\n"
    "struct VSIN\n"
    "{\n"
    "	float3 Position : POSITION0;\n"
    "	float3 Position2: POSITION1;\n"
    "	float3 Normal : NORMAL0;\n"
    "	float4 Color : COLOR0;\n"
    "	float4 UV : TEXCOORD0;\n"
    "	float4 UV2: TEXCOORD1;\n"
    "};\n\n"
    "struct VSOUT\n"
    "{\n"
    "	float4 Position : SV_POSITION;\n"
    " float4 Color:TEXCOORD0;\n"
    "};\n"
    "\n"
    "VSOUT v(VSIN x)\n"
    "{\n"
    "	VSOUT k;\n"
    "	k.Position = mul(projmat,mul(viewmat,mul(worldmat,float4(x.Position,1))));\n"
    "	k.Color=x.Color;\n"
    "	return k;\n"
    "}\n"
    "\n"
    "float4 p(VSOUT v) : SV_TARGET0\n"
    "{\n"
    "	return float4(1,0,0,1);\n"
    "}\n" );

  DefaultVertexShader = Device->CreateVertexShader( shader, strlen( shader ), "v", "vs_5_0" );
  DefaultPixelShader = Device->CreatePixelShader( shader, strlen( shader ), "p", "ps_5_0" );
}

void DeinitDefaultStates()
{
  SAFEDELETE( DefaultBlendState );
  SAFEDELETE( DefaultRasterState );
  SAFEDELETE( DefaultDepthState );
  SAFEDELETE( DefaultVertexShader );
  SAFEDELETE( DefaultPixelShader );
}


CphxScene_Tool::CphxScene_Tool()
{
  Name = _T( "New Scene" );
  Scene.RenderLayers = NULL;
  Scene.LayerCount = 0;
  Scene.ObjectCount = 0;
  Scene.Objects = NULL;
  UpdateLayers();
  ActiveClip = 0;
  //Timestamp = 0;
}

CphxScene_Tool::~CphxScene_Tool()
{
  for ( TS32 x = 0; x < Scene.LayerCount; x++ )
  {
#ifdef MEMORY_TRACKING
    memTracker.SetMissingIgnore( true );
#endif
    Scene.RenderLayers[ x ]->RenderInstances.FreeArray();
#ifdef MEMORY_TRACKING
    memTracker.SetMissingIgnore( false );
#endif
    SAFEDELETE( Scene.RenderLayers[ x ] );
  }
  SAFEDELETEA( Scene.RenderLayers );
  SAFEDELETEA( Scene.Objects );
  Clips.FreeArray();
  Objects.FreeArray();
}

void CphxScene_Tool::UpdateSceneGraph( int Clip, float t )
{
  Scene.UpdateSceneGraph( Clip, t );
}

void CphxScene_Tool::UpdateSceneGraph( int Clip, float t, D3DXMATRIX Root, CphxScene *RootScene )
{
  Scene.UpdateSceneGraph( Clip, t, Root, RootScene, NULL );
}

void CphxScene_Tool::Render()
{
  Scene.Render( true, true, 0 );
}

void CphxScene_Tool::KillInvalidBatches( bool wireframeview )
{
  TS32 WarnCount = 0;
  TS32 ErrCount = 0;

  for ( TS32 x = 0; x < Scene.LayerCount; x++ )
  {
    for ( TS32 y = 0; y < Scene.RenderLayers[ x ]->RenderInstances.NumItems(); y++ )
    {
      CphxRenderDataInstance *i = Scene.RenderLayers[ x ]->RenderInstances[ y ];

      if ( !i->VS )
      {
        i->VS = (ID3D11VertexShader*)DefaultVertexShader->GetHandle();
        WarnCount++;
      }
      if ( !i->PS )
      {
        i->PS = (ID3D11PixelShader*)DefaultPixelShader->GetHandle();
        WarnCount++;
      }

      if ( !i->BlendState )
      {
        i->BlendState = (ID3D11BlendState *)DefaultBlendState->GetHandle();
        WarnCount++;
      }
      if ( !i->DepthStencilState )
      {
        i->DepthStencilState = (ID3D11DepthStencilState *)DefaultDepthState->GetHandle();
        WarnCount++;
      }
      if ( !i->RasterizerState )
      {
        i->RasterizerState = (ID3D11RasterizerState *)DefaultRasterState->GetHandle();
        WarnCount++;
      }

      if ( ( i->Wireframe && !i->WireIndexCount ) || ( !i->Wireframe && !i->TriIndexCount ) || ( !i->VertexBuffer && !i->Indexed ) || ( wireframeview && Scene.RenderLayers[ x ]->Descriptor->VoxelizerLayer ) )
      {
        ErrCount++;
        Scene.RenderLayers[ x ]->RenderInstances.FreeByIndex( y );
        y--;
      }
    }
  }

  //if (WarnCount || ErrCount)
  //{
  //	LOG_DBG("[scene] Removed invalid batches: errors (%d) warnings (%d)", ErrCount, WarnCount);
  //}
}

void CphxScene_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );

  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Clip" ) );
    Clips[ x ]->Export( &n );
  }

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Object" ) );
    n.SetAttributeFromInteger( _T( "Type" ), Objects[ x ]->GetObjectType() );
    Objects[ x ]->Export( &n );
  }

}

void CphxScene_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Clip" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Clip" ), x );
    CphxSceneClip *Clip = AddClip();
    Clip->Import( &n );
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Object" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Object" ), x );
    PHXOBJECT type = Object_Dummy;
    if ( !n.HasAttribute( _T( "Type" ) ) ) continue;
    n.GetAttributeAsInteger( _T( "Type" ), (TS32*)&type );

    void *Data = NULL;

    switch ( type )
    {
    case Object_Model:
      if ( n.GetChildCount( _T( "model" ) ) )
      {
        CphxGUID g;
        g.SetString( n.GetChild( _T( "model" ) ).GetText().GetPointer() );
        Data = Project.GetModel( g );
      }
      break;
    case Object_Light:
      break;
    case Object_CamEye:
      break;
    case Object_Dummy:
      break;
    case Object_SubScene:
      if ( n.GetChildCount( _T( "scene" ) ) )
      {
        CphxGUID g;
        g.SetString( n.GetChild( _T( "scene" ) ).GetText().GetPointer() );
        Data = Project.GetScene( g );
      }
      break;
    case Object_ParticleEmitterCPU:
      if ( n.GetChildCount( _T( "EmitedObject" ) ) )
      {
        CphxGUID g;
        g.SetString( n.GetChild( _T( "EmitedObject" ) ).GetText().GetPointer() );
        Data = Project.GetModel( g );
      }
      if ( n.GetChildCount( _T( "EmitedScene" ) ) )
      {
        CphxGUID g;
        g.SetString( n.GetChild( _T( "EmitedScene" ) ).GetText().GetPointer() );
        Data = Project.GetScene( g );
        Data = (void*)( (TU32)( Data ) | 0x01 ); // OUCH
      }
      break;
    default:
      break;
    }

    CphxObject_Tool *t = AddObject( type, Data );
    t->Import( &n );
  }

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    CphxObject_Tool *o = GetObject( Objects[ x ]->TargetObjectGuid );
    if ( o ) Objects[ x ]->SetTarget( o );
    o = GetObject( Objects[ x ]->ParentObjectGuid );
    if ( o ) Objects[ x ]->SetParent( o, this );
  }

  if ( !Clips.NumItems() ) //one clip is always needed
    AddClip();

  RebuildMinimalData();

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->UpdateDependencies();
}

TBOOL CphxScene_Tool::GenerateResource( CCoreDevice *Dev )
{
  SwapParticleBuffers();
  ResetParticles();
  SwapParticleBuffers();
  ResetParticles();
  return true;
}

void CphxScene_Tool::UpdateLayers()
{
  SAFEDELETEA( Scene.RenderLayers );
  if ( !Project.GetRenderLayerCount() ) return;

  Scene.LayerCount = Project.GetRenderLayerCount();

  Scene.RenderLayers = new CphxRenderLayer*[ Scene.LayerCount ];

  for ( TS32 x = 0; x < Scene.LayerCount; x++ )
  {
    Scene.RenderLayers[ x ] = new CphxRenderLayer();
    Scene.RenderLayers[ x ]->Descriptor = &Project.GetRenderLayerByIndex( x )->RenderLayer;
  }
}

CphxObject_Tool * CphxScene_Tool::AddObject( PHXOBJECT p, void *Data )
{
  InvalidateUptoDateFlag();
  CphxObject_Tool *o = NULL;

  switch ( p )
  {
  case Object_Model:
    if ( !Data ) return NULL;
    o = new CphxObject_Model_Tool( (CphxModel_Tool *)Data );
    break;
  case Object_Light:
    o = new CphxObject_Light_Tool();
    break;
  case Object_CamEye:
  {
    o = new CphxObject_Tool();
    CphxObject *ob = new CphxObject();
    ob->ObjectType = p;
    o->SetObject( ob );
    o->SetName( CString( _T( "New Camera" ) ) );
    break;
  }
  case Object_Dummy:
  {
    o = new CphxObject_Tool();
    CphxObject *ob = new CphxObject();
    ob->ObjectType = p;
    o->SetObject( ob );
    o->SetName( CString( _T( "Dummy" ) ) );
    break;
  }
  case Object_SubScene:
    //if ( !Data ) return NULL;
    o = new CphxObject_SubScene_Tool();
    if ( Data )
      o->SetName( ( (CphxScene_Tool *)Data )->GetName() );
    break;
  case Object_ParticleEmitterCPU:
    o = new CphxObject_ParticleEmitter_CPU_Tool();
    {
      bool dataIsScene = (TU32)Data & 0x01;

      Data = (void*)( ( (TU32)Data ) & 0xfffffffc );

      ( (CphxObject_ParticleEmitter_CPU_Tool*)o )->SetEmittedModel( nullptr );
      ( (CphxObject_ParticleEmitter_CPU_Tool*)o )->SetEmittedScene( nullptr );

      if ( !dataIsScene )
        ( (CphxObject_ParticleEmitter_CPU_Tool*)o )->SetEmittedModel( (CphxModel_Tool *)Data );
      else
        ( (CphxObject_ParticleEmitter_CPU_Tool*)o )->SetEmittedScene( (CphxScene_Tool *)Data );
    }

    for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
      for ( TS32 y = 0; y < Project.GetMaterialByIndex( x )->Techniques.NumItems(); y++ )
        if ( Project.GetMaterialByIndex( x )->Techniques[ y ]->Tech->Type == TECH_PARTICLE )
        {
          CphxObject_ParticleEmitter_CPU_Tool *s = (CphxObject_ParticleEmitter_CPU_Tool*)o;
          s->SetMaterial( Project.GetMaterialByIndex( x ) );
          break;
        }
    break;
  case Object_ParticleGravity:
    o = new CphxObject_ParticleGravity_Tool();
    o->SetName( CString( _T( "New Gravity" ) ) );
    break;
  case Object_ParticleDrag:
    o = new CphxObject_ParticleDrag_Tool();
    o->SetName( CString( _T( "New Drag" ) ) );
    break;
  case Object_ParticleTurbulence:
    o = new CphxObject_ParticleTurbulence_Tool();
    o->SetName( CString( _T( "New Turbulence" ) ) );
    break;
  case Object_ParticleVortex:
    o = new CphxObject_ParticleVortex_Tool();
    o->SetName( CString( _T( "New Vortex" ) ) );
    break;
  case Object_LogicObject:
    {
      o = new CphxObject_Tool();
      CphxObject *ob = new CphxObject();
      ob->ObjectType = p;
      o->SetObject( ob );
      o->SetName( CString( _T( "Logic Object" ) ) );
    }
    break;
  default:
    break;
  }

  if ( !o ) return NULL;

  o->GetObject()->camCenterX = 0;
  o->GetObject()->camCenterY = 0;

  //create spline data for newly created object for all existing clips here

  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
  {
    o->AddClip( Clips[ x ], -1 );
    if ( p == Object_SubScene && Data )
    {
      o->GetClip( x )->SetSubSceneTarget( (CphxScene_Tool *)Data );
      o->GetClip( x )->SetTurbulenceFreq( 10 );
      o->GetClip( x )->SetRandSeed( 0 );
      o->AddParent( (CphxScene_Tool *)Data );
    }
  }

  o->GetObject()->Scene = &Scene;
  o->RebuildMinimalData();

  Objects.Add( o );
  AddParent( o );

  SimulateAddItem( Scene.Objects, Scene.ObjectCount, o->GetObject() );
  //Scene.Objects.Add(o->GetObject());
  o->GetObject()->ToolData = NULL;

  return o;
}

void CphxScene_Tool::SetKeyframerMode()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->SetKeyframerMode( Objects[ x ] );
}

void CphxScene_Tool::DeleteByIndex( TS32 x )
{
  if ( Objects[ x ]->Selected )
  {
    for ( TS32 y = 0; y < Objects.NumItems(); y++ )
    {
      if ( Objects[ y ]->ParentObjectGuid == Objects[ x ]->GetGUID() )
        Objects[ y ]->SetParent( Objects[ x ]->GetParentObject(), this );
      if ( Objects[ y ]->TargetObjectGuid == Objects[ x ]->GetGUID() )
        Objects[ y ]->SetTarget( NULL );
    }

    Objects[ x ]->SetParent( NULL, this );
    SimulateDeleteByIndex( Scene.Objects, Scene.ObjectCount, x );
    //Scene.Objects.DeleteByIndex(x);
    Objects.FreeByIndex( x );
  }
}

void CphxScene_Tool::DeleteSelected()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    if ( Objects[ x ]->Selected )
    {
      for ( TS32 y = 0; y < Objects.NumItems(); y++ )
      {
        if ( Objects[ y ]->ParentObjectGuid == Objects[ x ]->GetGUID() )
          Objects[ y ]->SetParent( Objects[ x ]->GetParentObject(), this );
        if ( Objects[ y ]->TargetObjectGuid == Objects[ x ]->GetGUID() )
          Objects[ y ]->SetTarget( NULL );
      }

      Objects[ x ]->SetParent( NULL, this );
      SimulateDeleteByIndex( Scene.Objects, Scene.ObjectCount, x );
      //Scene.Objects.DeleteByIndex(x);
      Objects.FreeByIndex( x );
      x--;
    }
  }
}

void CphxScene_Tool::CopySelected()
{
  int count = Objects.NumItems();
  for ( int x = 0; x < count; x++ )
    if ( Objects[ x ]->Selected )
    {
      Objects[ x ]->Selected = false;

      CphxObject_Tool *o = CopyObject( Objects[ x ] );

      //switch (Objects[x]->GetObjectType()) //if you expand this expand it in CphxScene::PasteObjects and CphxScene_Tool::CopyFrom(CphxScene_Tool *Source) too
      //{
      //case Object_Model:
      //{
      //  CphxObject_Model_Tool *t = (CphxObject_Model_Tool*)Objects[x];
      //  o = AddObject(Objects[x]->GetObjectType(), t->Model);
      //}
      //break;
      //case Object_SubScene:
      //{
      //  CphxObject_SubScene_Tool *t = (CphxObject_SubScene_Tool*)Objects[x];
      //  o = AddObject(Objects[x]->GetObjectType(), t->Scene);
      //}
      //break;
      //case Object_ParticleEmitterCPU:
      //{
      //  CphxObject_ParticleEmitter_CPU_Tool *t = (CphxObject_ParticleEmitter_CPU_Tool*)Objects[x];
      //  o = AddObject(Objects[x]->GetObjectType(), NULL);
      //  CphxObject_ParticleEmitter_CPU_Tool *n = (CphxObject_ParticleEmitter_CPU_Tool *)o;
      //  n->CopyDataFrom(t);
      //}
      //break;
      //case Object_ParticleDrag:
      //{
      //  CphxObject_ParticleDrag_Tool *t = (CphxObject_ParticleDrag_Tool*)Objects[x];
      //  o = AddObject(Objects[x]->GetObjectType(), NULL);
      //  CphxObject_ParticleDrag *old = (CphxObject_ParticleDrag*)Objects[x]->Object;
      //  CphxObject_ParticleDrag *n = (CphxObject_ParticleDrag*)o->Object;
      //  n->AreaType = old->AreaType;
      //}
      //break;
      //case Object_ParticleGravity:
      //{
      //  CphxObject_ParticleGravity_Tool *t = (CphxObject_ParticleGravity_Tool*)Objects[x];
      //  o = AddObject(Objects[x]->GetObjectType(), NULL);
      //  CphxObject_ParticleGravity *old = (CphxObject_ParticleGravity*)Objects[x]->Object;
      //  CphxObject_ParticleGravity *n = (CphxObject_ParticleGravity*)o->Object;
      //  n->AreaType = old->AreaType;
      //  n->Directional = old->Directional;
      //}
      //break;
      //case Object_ParticleTurbulence:
      //{
      //  CphxObject_ParticleTurbulence_Tool *t = (CphxObject_ParticleTurbulence_Tool*)Objects[x];
      //  o = AddObject(Objects[x]->GetObjectType(), NULL);
      //  CphxObject_ParticleTurbulence *old = (CphxObject_ParticleTurbulence*)Objects[x]->Object;
      //  CphxObject_ParticleTurbulence *n = (CphxObject_ParticleTurbulence*)o->Object;
      //  n->AreaType = old->AreaType;
      //  n->Frequency = old->Frequency;
      //  n->RandSeed = old->RandSeed;
      //}
      //break;


      //default:
      //  o = AddObject(Objects[x]->GetObjectType(), NULL);
      //  break;
      //}

      //o->GetObject()->Position = Objects[x]->GetObject()->Position;
      //o->GetObject()->Rotation = Objects[x]->GetObject()->Rotation;
      //o->GetObject()->Scale = Objects[x]->GetObject()->Scale;
      o->Selected = true;
      o->CopySplinesFrom( Objects[ x ] );

    }
}

void CphxScene_Tool::MarkCopyPaste()
{
  sceneCopyData.Flush();
  for ( int x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->Selected )
    {
      SceneObjCopyData d;
      d.scene = this;
      d.obj = Objects[ x ];
      d.clip = ActiveClip;
      sceneCopyData += d;
    }
}

void CphxScene_Tool::PasteObjects()
{
  if ( sceneCopyData.NumItems() )
  {
    for ( int x = 0; x < Objects.NumItems(); x++ )
      Objects[ x ]->Selected = false;
  }

  for ( int x = 0; x < sceneCopyData.NumItems(); x++ )
  {
    auto d = sceneCopyData[ x ];
    if ( d.scene == this )
      continue;

    bool found = false;
    for ( int y = 0; y < Project.GetSceneCount(); y++ )
      if ( Project.GetSceneByIndex( y ) == d.scene )
        found = true;

    if ( !found )
      continue;

    found = false;
    for ( int y = 0; y < d.scene->GetObjectCount(); y++ )
      if ( d.scene->GetObjectByIndex( y ) == d.obj )
        found = true;

    if ( !found )
      continue;

    if ( d.clip < 0 || d.clip >= d.scene->GetClipCount() )
      continue;

    auto o = CopyObject( d.obj );

    o->Selected = true;
    o->CopySplinesFrom( d.obj, d.clip, this );

  }
}

void CphxScene_Tool::CopyFrom( CphxScene_Tool *Source )
{
  for ( TS32 x = 0; x < Source->Objects.NumItems(); x++ )
    Source->Objects[ x ]->SetResourceIndex( x );

  for ( TS32 x = 0; x < Source->Clips.NumItems(); x++ )
  {
    CphxSceneClip *Clip = NULL;
    if ( x < Clips.NumItems() ) Clip = Clips[ x ];
    else Clip = AddClip();

    Clip->SetName( Source->Clips[ x ]->GetName() );
  }

  int count = Source->Objects.NumItems();
  for ( int x = 0; x < count; x++ )
  {
    CphxObject_Tool *o = CopyObject( Source->Objects[ x ] );// NULL;

    //switch (Source->Objects[x]->GetObjectType()) //if you expand this expand it in CphxScene::PasteObjects and CphxScene_Tool::CopyFrom(CphxScene_Tool *Source) too
    //{
    //case Object_Model:
    //{
    //  CphxObject_Model_Tool *t = (CphxObject_Model_Tool*)Source->Objects[x];
    //  o = AddObject(Source->Objects[x]->GetObjectType(), t->Model);
    //}
    //break;
    //case Object_SubScene:
    //{
    //  CphxObject_SubScene_Tool *t = (CphxObject_SubScene_Tool*)Source->Objects[x];
    //  o = AddObject(Source->Objects[x]->GetObjectType(), t->Scene);
    //}
    //break;
    //case Object_ParticleEmitterCPU:
    //{
    //  CphxObject_ParticleEmitter_CPU_Tool *t = (CphxObject_ParticleEmitter_CPU_Tool*)Source->Objects[x];
    //  o = AddObject(Source->Objects[x]->GetObjectType(), NULL);
    //  CphxObject_ParticleEmitter_CPU_Tool *n = (CphxObject_ParticleEmitter_CPU_Tool *)o;
    //  n->CopyDataFrom(t);
    //}
    //break;
    //case Object_ParticleDrag:
    //{
    //  CphxObject_ParticleDrag_Tool *t = (CphxObject_ParticleDrag_Tool*)Source->Objects[x];
    //  o = AddObject(Source->Objects[x]->GetObjectType(), NULL);
    //  CphxObject_ParticleDrag *old = (CphxObject_ParticleDrag*)Source->Objects[x]->Object;
    //  CphxObject_ParticleDrag *n = (CphxObject_ParticleDrag*)o->Object;
    //  n->AreaType = old->AreaType;
    //}
    //break;
    //case Object_ParticleGravity:
    //{
    //  CphxObject_ParticleGravity_Tool *t = (CphxObject_ParticleGravity_Tool*)Source->Objects[x];
    //  o = AddObject(Source->Objects[x]->GetObjectType(), NULL);
    //  CphxObject_ParticleGravity *old = (CphxObject_ParticleGravity*)Source->Objects[x]->Object;
    //  CphxObject_ParticleGravity *n = (CphxObject_ParticleGravity*)o->Object;
    //  n->AreaType = old->AreaType;
    //  n->Directional = old->Directional;
    //}
    //break;
    //case Object_ParticleTurbulence:
    //{
    //  CphxObject_ParticleTurbulence_Tool *t = (CphxObject_ParticleTurbulence_Tool*)Source->Objects[x];
    //  o = AddObject(Source->Objects[x]->GetObjectType(), NULL);
    //  CphxObject_ParticleTurbulence *old = (CphxObject_ParticleTurbulence*)Source->Objects[x]->Object;
    //  CphxObject_ParticleTurbulence *n = (CphxObject_ParticleTurbulence*)o->Object;
    //  n->AreaType = old->AreaType;
    //  n->Frequency = old->Frequency;
    //  n->RandSeed = old->RandSeed;
    //}
    //break;

    //default:
    //  o = AddObject(Source->Objects[x]->GetObjectType(), NULL);
    //  break;
    //}

    //o->GetObject()->Position = Objects[x]->GetObject()->Position;
    //o->GetObject()->Rotation = Objects[x]->GetObject()->Rotation;
    //o->GetObject()->Scale = Objects[x]->GetObject()->Scale;
    o->Selected = Source->Objects[ x ]->Selected;
    o->Name = Source->Objects[ x ]->Name;

    o->Clips.FreeMinimalData();
    o->Clips.Clips.FreeArray();
    o->Object->Clips = NULL;

    for ( TS32 y = 0; y < Source->Objects[ x ]->Clips.Clips.NumItems(); y++ )
    {
      CphxObjectClip_Tool *oc = Source->Objects[ x ]->Clips.Clips[ y ];
      CphxObjectClip_Tool *nc = new CphxObjectClip_Tool( Clips[ y ] );
      nc->SetSubSceneTarget( oc->GetSubSceneTarget() );
      nc->SetRandSeed( oc->GetRandSeed() );
      nc->SetTurbulenceFreq( oc->GetTurbulenceFreq() );
      o->Clips.Clips += nc;

      for ( TS32 z = 0; z < oc->GetSplineCount(); z++ )
      {
        CphxClipSpline_Tool *os = oc->GetSplineByIndex( z );
        nc->CreateSpline( os->MinimalSpline.Type );
        CphxClipSpline_Tool *ns = nc->GetSplineByIndex( nc->GetSplineCount() - 1 );
        ns->MinimalSpline.GroupingData = os->MinimalSpline.GroupingData;
        ns->MinimalSpline.MaterialParam = os->MinimalSpline.MaterialParam;
        os->Spline->CopyTo( ns->Spline );
      }

      nc->GetMaterialSplineBatch()->Copy( oc->GetMaterialSplineBatch() );
    }

    o->RebuildMinimalData();

    //o->CopySplinesFrom(Source->Objects[x]);
  }

  for ( int x = 0; x < count; x++ )
  {
    CphxObject_Tool *o = Source->Objects[ x ];
    CphxObject_Tool *n = Objects[ x ];
    if ( o->TargetObject )
      n->SetTarget( Objects[ o->TargetObject->GetResourceIndex() ] );
    if ( o->GetParentObject() )
      n->SetParent( Objects[ o->GetParentObject()->GetResourceIndex() ], this );
  }

  Name = CString( "*" ) + Source->Name;
  Scene.ClipCount = Clips.NumItems();
}


CphxObject_Tool* CphxScene_Tool::CopyObject( CphxObject_Tool* source )
{
  CphxObject_Tool* o = nullptr;
  switch ( source->GetObjectType() )
  {
  case Object_Model:
  {
    CphxObject_Model_Tool *t = (CphxObject_Model_Tool*)source;
    o = AddObject( source->GetObjectType(), t->Model );
  }
  break;
  case Object_SubScene:
  {
    CphxObject_SubScene_Tool *t = (CphxObject_SubScene_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
  }
  break;
  case Object_Light:
  {
    CphxObject_Light_Tool *t = (CphxObject_Light_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
    CphxObject_Light_Tool *n = (CphxObject_Light_Tool *)o;
    n->SetPointLight( t->IsPointLight() );
  }
  break;
  case Object_ParticleEmitterCPU:
  {
    CphxObject_ParticleEmitter_CPU_Tool *t = (CphxObject_ParticleEmitter_CPU_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
    CphxObject_ParticleEmitter_CPU_Tool *n = (CphxObject_ParticleEmitter_CPU_Tool *)o;
    if ( t->EmitedObject )
      n->SetEmittedModel( t->EmitedObject );
    if ( t->EmitedScene )
      n->SetEmittedScene( t->EmitedScene );
    n->CopyDataFrom( t );
  }
  break;
  case Object_ParticleDrag:
  {
    CphxObject_ParticleDrag_Tool *t = (CphxObject_ParticleDrag_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
    CphxObject_ParticleDrag *old = (CphxObject_ParticleDrag*)source->Object;
    CphxObject_ParticleDrag *n = (CphxObject_ParticleDrag*)o->Object;
    n->AreaType = old->AreaType;
  }
  break;
  case Object_ParticleGravity:
  {
    CphxObject_ParticleGravity_Tool *t = (CphxObject_ParticleGravity_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
    CphxObject_ParticleGravity *old = (CphxObject_ParticleGravity*)source->Object;
    CphxObject_ParticleGravity *n = (CphxObject_ParticleGravity*)o->Object;
    n->AreaType = old->AreaType;
    n->Directional = old->Directional;
  }
  break;
  case Object_ParticleTurbulence:
  {
    CphxObject_ParticleTurbulence_Tool *t = (CphxObject_ParticleTurbulence_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
    CphxObject_ParticleTurbulence *old = (CphxObject_ParticleTurbulence*)source->Object;
    CphxObject_ParticleTurbulence *n = (CphxObject_ParticleTurbulence*)o->Object;
    n->AreaType = old->AreaType;
    n->TurbulenceFrequency = old->TurbulenceFrequency;
    n->RandSeed = old->RandSeed;
  }
  break;
  case Object_ParticleVortex:
  {
    CphxObject_ParticleVortex_Tool* t = (CphxObject_ParticleVortex_Tool*)source;
    o = AddObject( source->GetObjectType(), NULL );
    CphxObject_ParticleVortex* old = (CphxObject_ParticleVortex*)source->Object;
    CphxObject_ParticleVortex* n = (CphxObject_ParticleVortex*)o->Object;
    n->AreaType = old->AreaType;
  }
  break;

  default:
    o = AddObject( source->GetObjectType(), NULL );
    break;
  }

  return o;
}

CphxSceneClip * CphxScene_Tool::AddClip()
{
  CphxSceneClip *c = new CphxSceneClip();
  Clips += c;

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->AddClip( c, GetActiveClip() );

  return c;
}

CphxSceneClip * CphxScene_Tool::GetClip( CphxGUID &GUID )
{
  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
    if ( Clips[ x ]->GetGUID() == GUID ) return Clips[ x ];

  return NULL;
}

TS32 CphxScene_Tool::GetClipIndex( CphxGUID &GUID )
{
  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
    if ( Clips[ x ]->GetGUID() == GUID ) return x;

  return -1;
}

TS32 CphxScene_Tool::GetObjectIndex( CphxGUID &GUID )
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->GetGUID() == GUID ) return x;

  return -1;
}


void CphxScene_Tool::RebuildMinimalData()
{
  Scene.ClipCount = GetClipCount();

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->RebuildMinimalData();
}

CphxObject_Tool * CphxScene_Tool::GetEditedObject()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->Selected ) return Objects[ x ];
  return NULL;
}

CphxObject_Tool * CphxScene_Tool::GetObject( CphxGUID &g )
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->GetGUID() == g ) return Objects[ x ];
  return NULL;
}

CphxObject_Tool * CphxScene_Tool::GetObjectByName( CString Name )
{
  Name.ToLower();
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    CString n = Objects[ x ]->GetName();
    n.ToLower();
    if ( Name == n ) return Objects[ x ];
  }
  return NULL;
}

void CphxScene_Tool::LinkSelectedTo( CphxObject_Tool *parent, TBOOL preservePosition )
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->Selected )
    {
      if ( parent && parent->FindParent( Objects[ x ] ) )
      {
        SetStatusbarText( _T( "Some of the objects were not linked to avoid a circular hierarchy." ) );
        continue;
      }

      if ( preservePosition )
      {
        D3DXMATRIX pm;
        D3DXMatrixIsIdentity( &pm );
        if ( parent )
          pm = parent->GetMatrix();
        D3DXMatrixInverse( &pm, NULL, &pm );

        D3DXMATRIX om = Objects[ x ]->GetMatrix();
        D3DXMatrixMultiply( &om, &om, &pm );

        D3DXVECTOR3 scale;
        D3DXQUATERNION rotation;
        D3DXVECTOR3 translation;
        D3DXMatrixDecompose( &scale, &rotation, &translation, &om );

        for ( int y = 0; y < 3; y++ )
        {
          auto *scaleX = Objects[ x ]->GetClip( 0 )->GetClipSpline( (PHXSPLINETYPE)( Spline_Scale_x + y ) );
          if ( scaleX ) scaleX->Spline->Spline->Value[ 0 ] = scale[ y ];

          auto *posX = Objects[ x ]->GetClip( 0 )->GetClipSpline( (PHXSPLINETYPE)( Spline_Position_x + y ) );
          if ( posX ) posX->Spline->Spline->Value[ 0 ] = translation[ y ];
        }

        auto *rot = Objects[ x ]->GetClip( 0 )->GetClipSpline( Spline_Rotation );
        if ( rot )
        {
          rot->Spline->Spline->Value[ 0 ] = rotation.x;
          rot->Spline->Spline->Value[ 1 ] = rotation.y;
          rot->Spline->Spline->Value[ 2 ] = rotation.z;
          rot->Spline->Spline->Value[ 3 ] = rotation.w;
        }

      }

      Objects[ x ]->SetParent( parent, this );
    }
}

TBOOL CphxScene_Tool::CanContain( CphxScene_Tool *sub )
{
  //determine if sub can be put into this scene as a subscene
  return !sub->FindInSubsceneTree( this );
}

TBOOL CphxScene_Tool::FindInSubsceneTree( CphxScene_Tool *scene )
{
  if ( this == scene ) return true;

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    if ( Objects[ x ]->GetObjectType() == Object_SubScene )
    {
      CphxObject_SubScene_Tool *s = (CphxObject_SubScene_Tool*)Objects[ x ];
      for ( int y = 0; y < Clips.NumItems(); y++ )
        if ( s->GetClip(y)->GetSubSceneTarget()->FindInSubsceneTree( scene ) ) return true;
    }
  }

  return false;
}

void CphxScene_Tool::ForceUpdateContent()
{
  RequestContent();
  for ( TS32 x = 0; x < GetObjectCount(); x++ )
  {
    GetObjectByIndex( x )->RequestContent();
    if ( GetObjectByIndex( x )->GetObjectType() == Object_Model )
    {
      CphxObject_Model_Tool* o = (CphxObject_Model_Tool*)GetObjectByIndex( x );
      o->UpdateModel();
    }
    if ( GetObjectByIndex( x )->GetObjectType() == Object_SubScene )
    {
      CphxObject_SubScene_Tool* o = (CphxObject_SubScene_Tool*)GetObjectByIndex( x );

      CArray< CphxScene_Tool* > scenes;
      for ( int y = 0; y < Clips.NumItems(); y++ )
        scenes.AddUnique( o->GetClip( y )->GetSubSceneTarget() );

      for ( int y = 0; y < scenes.NumItems(); y++ )
        scenes[ y ]->ForceUpdateContent();
    }
    if ( GetObjectByIndex( x )->GetObjectType() == Object_ParticleEmitterCPU )
    {
      CphxObject_ParticleEmitter_CPU_Tool* o = (CphxObject_ParticleEmitter_CPU_Tool*)GetObjectByIndex( x );
      o->UpdateMaterialState();
      o->UpdateMaterialTextures();
      if ( o->EmitedScene )
        o->EmitedScene->ForceUpdateContent();
    }
  }
  SetKeyframerMode();
}

TS32 CphxScene_Tool::GetTriCount()
{
  TS32 TriCount = 0;
  for ( TS32 x = 0; x < Scene.LayerCount; x++ )
  {
    for ( TS32 y = 0; y < Scene.RenderLayers[ x ]->RenderInstances.NumItems(); y++ )
    {
      CphxRenderDataInstance *r = Scene.RenderLayers[ x ]->RenderInstances[ y ];
      TriCount += r->Wireframe ? r->WireIndexCount / 2 : r->TriIndexCount / 3;
    }
  }
  return TriCount;
}

void CphxScene_Tool::SimulateParticles()
{
  TU32 t = globalTimer.GetTime();
  if ( LastParticleTime == 0 ) LastParticleTime = t;

  if ( t - LastParticleTime > 1000 ) LastParticleTime = t - 1000;

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->GetObjectType() == Object_ParticleEmitterCPU )
    {
      CphxObject_ParticleEmitter_CPU_Tool *p = (CphxObject_ParticleEmitter_CPU_Tool*)Objects[ x ];
      p->SimulateParticles( ( t - LastParticleTime ) / 1000.0f );
    }

  LastParticleTime = t;
}

void CphxScene_Tool::ResetParticles()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    switch ( Objects[ x ]->GetObjectType() )
    {
    case Object_ParticleEmitterCPU:
      ( (CphxObject_ParticleEmitter_CPU_Tool*)Objects[ x ] )->ResetParticles();
      break;
    default:
      break;
    }
}

void CphxScene_Tool::SwapParticleBuffers()
{
  int lastt = LastParticleTime;
  LastParticleTime = lastParticleTimeBackup;
  lastParticleTimeBackup = lastt;

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->GetObjectType() == Object_ParticleEmitterCPU )
    {
      CphxObject_ParticleEmitter_CPU_Tool *p = (CphxObject_ParticleEmitter_CPU_Tool*)Objects[ x ];
      p->SwapParticleBuffers();
    }
}

#ifndef _DEBUG
void __fastcall SortRenderLayer( CphxRenderDataInstance **Instaces, int first, int last );
#else
void SortRenderLayer( CphxRenderDataInstance **Instaces, int first, int last );
#endif

void CphxScene_Tool::SortRenderLayers()
{
  for ( int x = 0; x < Scene.LayerCount; x++ )
    SortRenderLayer( Scene.RenderLayers[ x ]->RenderInstances.Array, 0, Scene.RenderLayers[ x ]->RenderInstances.ItemCount - 1 );
}

void CphxScene_Tool::CopyClip( CphxGUID &GUID )
{
  CphxSceneClip *o = GetClip( GUID );
  if ( !o )
    return;

  CphxSceneClip *c = AddClip();
  c->SetName( CString( _T( "Copy of " ) ) + o->GetName() );

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    auto *obj = Objects[ x ];
    //obj->Clips.FreeMinimalData();
    //obj->Clips.Clips.FreeArray();
    //obj->Object->Clips = NULL;

    CphxObjectClip_Tool *oc = NULL;
    CphxObjectClip_Tool *nc = NULL;

    for ( TS32 x = 0; x < obj->Clips.Clips.NumItems(); x++ )
    {
      if ( obj->Clips.GetClip( x )->GetSceneClip() == o )
        oc = obj->Clips.GetClip( x );
      if ( obj->Clips.GetClip( x )->GetSceneClip() == c )
        nc = obj->Clips.GetClip( x );
    }

    if ( !oc || !nc )
      continue;

    if ( oc->GetSplineCount() != nc->GetSplineCount() )
      continue;

    nc->SetSubSceneTarget( oc->GetSubSceneTarget() );
    nc->SetRandSeed( oc->GetRandSeed() );
    nc->SetTurbulenceFreq( oc->GetTurbulenceFreq() );

    for ( TS32 y = 0; y < oc->GetSplineCount(); y++ )
    {
      CphxClipSpline_Tool *os = oc->GetSplineByIndex( y );
      CphxClipSpline_Tool *ns = nc->GetSplineByIndex( y );
      ns->MinimalSpline.GroupingData = os->MinimalSpline.GroupingData;
      ns->MinimalSpline.MaterialParam = os->MinimalSpline.MaterialParam;
      os->Spline->CopyTo( ns->Spline );
    }

    nc->GetMaterialSplineBatch()->Copy( oc->GetMaterialSplineBatch() );

    obj->RebuildMinimalData();
  }

}

#include "../apEx/apexRoot.h"
#include "../apEx/ModelView.h"
#include "../apEx/SceneView.h"

void CphxScene_Tool::ApplyCameraDataForFirstCam()
{
  bool updated = false;
  for ( int x = 0; x < Objects.NumItems(); x++ )
  {
    if ( Objects[ x ]->GetObjectType() == Object_CamEye )
    {      
      if ( !updated )
      {
        UpdateSceneGraph( 0, 0 );
        updated = true;
      }
      auto mat = Objects[ x ]->GetMatrix();
      D3DXVECTOR4 pos = D3DXVECTOR4( 0, 0, 0, 1 );
      D3DXVec4Transform( &pos, &pos, &mat );
      D3DXVECTOR4 trg = pos + D3DXVECTOR4( 0, 0, 1, 0 );
      if ( Objects[ x ]->TargetObject )
      {
        mat = Objects[ x ]->TargetObject->GetMatrix();
        trg = D3DXVECTOR4( 0, 0, 0, 1 );
        D3DXVec4Transform( &trg, &trg, &mat );
      }

      CAMERADATA c;
      c.OrthoY = c.Up = D3DXVECTOR3( 0, 1, 0 );

      c.Up = D3DXVECTOR3( 0, 1, 0 );

      c.Eye = D3DXVECTOR3( pos );
      c.Up = D3DXVECTOR3( 0, 1, 0 );
      c.Target = D3DXVECTOR3( trg );

      extern CapexRoot *Root;

      if (Root)
      for ( int y = 0; y < Root->GetWorkBenchCount(); y++ )
      {
        auto* wb = Root->GetWorkBench( y );
        int cnt = wb->GetWindowCount( apEx_SceneView );

        for ( int z = 0; z < cnt; z++ )
        {
          auto *win = (CapexSceneView*)wb->GetWindow( apEx_SceneView, z );
          CWBSceneDisplay *t = (CWBSceneDisplay*)win->FindChildByID( _T( "display" ), _T( "scenedisplay" ) );
          if ( t->GetMode() == CAMERA_NORMAL )
          {
            t->SetCameraData( GetGUID(), c );
          }
        }
      }
    }
  }
}

bool CphxScene_Tool::IsClipUsedInSubScene( int clipIdx )
{
  return false;
  int subSceneCount = GetChildCount( PHX_OBJECT );
  if ( subSceneCount )
  {
    for ( int y = 0; y < subSceneCount; y++ )
    {
      CphxObject_Tool* obj = (CphxObject_Tool*)GetChild( PHX_OBJECT, y );
      if ( !obj->IsRequired() )
        continue;
      if ( obj->GetObjectType() == Object_ParticleEmitterCPU && clipIdx == 0 )
        return true;

      if ( obj->GetObjectType() == Object_SubScene )
      {
        CphxObject_SubScene_Tool* sub = (CphxObject_SubScene_Tool*)obj;
        for ( int z = 0; z < sub->GetClipArray().GetClipCount(); z++ )
        {
          if ( sub->GetSubScene( z ) != this )
            continue;
          auto* spline = sub->GetClipArray().GetClip( z )->GetSpline( Spline_SubScene_Clip );
          if ( spline && spline->HasIntValue( clipIdx ) )
            return true;
        }
      }
    }
  }

  subSceneCount = GetWeakChildCount( PHX_OBJECT );
  if ( subSceneCount )
  {
    for ( int y = 0; y < subSceneCount; y++ )
    {
      CphxObject_Tool* obj = (CphxObject_Tool*)GetWeakChild( PHX_OBJECT, y );
      if ( !obj->IsRequired() )
        continue;

      if ( obj->GetObjectType() == Object_ParticleEmitterCPU && clipIdx == 0 )
        return true;

      if ( obj->GetObjectType() == Object_SubScene )
      {
        CphxObject_SubScene_Tool* sub = (CphxObject_SubScene_Tool*)obj;
        for ( int z = 0; z < sub->GetClipArray().GetClipCount(); z++ )
        {
          if ( sub->GetSubScene( z ) != this )
            continue;
          auto* spline = sub->GetClipArray().GetClip( z )->GetSpline( Spline_SubScene_Clip );
          if ( spline && spline->HasIntValue( clipIdx ) )
            return true;
        }
      }
    }
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////
// object implementations

CphxObject_Tool::CphxObject_Tool()
{
  Object = NULL;
  Selected = false;
  Name = _T( "New Object" );
  Parent = NULL;
  Clips.Owner = this;
  TargetObject = NULL;
  TargetObjectGuid.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );
  ParentObjectGuid.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  int r, g, b;
  r = ( rand() % 256 );
  g = ( rand() % 255 ) + 1;
  b = ( rand() % 255 ) + 1;

  int mx = max( max( r, g ), b );

  WireframeColor.R() = ( r * 255 ) / mx;
  WireframeColor.G() = ( g * 255 ) / mx;
  WireframeColor.B() = ( b * 255 ) / mx;
  WireframeColor.A() = 255;
}

CphxObject_Tool::~CphxObject_Tool()
{
  Clips.FreeMinimalData();
  if ( Object ) SAFEDELETEA( Object->Children );
  SAFEDELETE( Object );
}

void CphxObject_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Clips.ExportData( Node );
  if ( TargetObject )
    Node->AddChild( _T( "targetobject" ) ).SetText( TargetObject->GetGUID().GetString() );
  if ( Parent )
    Node->AddChild( _T( "parentobject" ) ).SetText( Parent->GetGUID().GetString() );

  if ( GetObjectA()->camCenterX != 0 )
    Node->AddChild( _T( "camcenterx" ) ).SetInt( GetObjectA()->camCenterX );
  if ( GetObjectA()->camCenterY != 0 )
    Node->AddChild( _T( "camcentery" ) ).SetInt( GetObjectA()->camCenterY );
}

void CphxObject_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  Clips.ImportData( Node );
  
  for ( int x = 0; x < Clips.GetClipCount(); x++ )
  {
    if ( Clips.GetClip( x )->GetSubSceneTarget() )
      AddParent( Clips.GetClip( x )->GetSubSceneTarget() );
  }

  if ( Node->GetChildCount( _T( "targetobject" ) ) )
    TargetObjectGuid.SetString( Node->GetChild( _T( "targetobject" ) ).GetText().GetPointer() );
  if ( Node->GetChildCount( _T( "parentobject" ) ) )
    ParentObjectGuid.SetString( Node->GetChild( _T( "parentobject" ) ).GetText().GetPointer() );

  if ( Node->GetChildCount( _T( "camcenterx" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "camcenterx" ) ).GetValue( i ) ) GetObject()->camCenterX = i; }
  if ( Node->GetChildCount( _T( "camcentery" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "camcentery" ) ).GetValue( i ) ) GetObject()->camCenterY = i; }
}

D3DXMATRIX CphxObject_Tool::GetMatrix()
{
  D3DXMATRIX m;
  D3DXMatrixTransformation( &m, NULL, NULL, &D3DXVECTOR3( &Object->SplineResults[ Spline_Scale_x ] ), NULL, &Object->RotationResult, &D3DXVECTOR3( &Object->SplineResults[ Spline_Position_x ] ) );
  if ( Parent )
    D3DXMatrixMultiply( &m, &m, &Parent->GetMatrix() );
  return m;
}

D3DXMATRIX CphxObject_Tool::GetMatrix_()
{
  D3DXMATRIX m;
  D3DXMatrixTransformation( &m, NULL, NULL, &_Scale, NULL, &_Rotation, &_Position );
  if ( Parent )
    D3DXMatrixMultiply( &m, &m, &Parent->GetMatrix_() );
  return m;
}

void CphxObject_Tool::SetObject( CphxObject *o )
{
  Object = o;
  Object->Clips = NULL;
  Object->Target = NULL;
  Object->Parent = NULL;
  Object->Children = NULL;
  Object->ChildCount = 0;
  Object->cameraCubeMapTarget = NULL;
  memset( Object->SplineResults, 0, sizeof( Object->SplineResults ) );
  Object->SplineResults[ Spline_Scale_x ] = 1;
  Object->SplineResults[ Spline_Scale_y ] = 1;
  Object->SplineResults[ Spline_Scale_z ] = 1;
  D3DXQuaternionIdentity( &Object->RotationResult );

}

void CphxObject_Tool::AddClip( CphxSceneClip *Clip, TS32 referenceClipId )
{
  Clips.AddClip( Clip, referenceClipId );
}

void CphxObject_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  Clip->CreateSpline( Spline_Position_x );
  Clip->CreateSpline( Spline_Position_y );
  Clip->CreateSpline( Spline_Position_z );

  if ( GetObjectType() == Object_Dummy || GetObjectType() == Object_ParticleTurbulence || GetObjectType() == Object_ParticleDrag || GetObjectType() == Object_LogicObject )
  {
    Clip->CreateSpline( Spline_Scale_x );
    Clip->CreateSpline( Spline_Scale_y );
    Clip->CreateSpline( Spline_Scale_z );
    Clip->CreateSpline( Spline_Rotation );
  }

  if ( GetObjectType() == Object_CamEye )
  {
    Clip->CreateSpline( Spline_Camera_FOV );
    Clip->CreateSpline( Spline_Camera_Roll );
  }
}

void CphxObject_Tool::RebuildMinimalData()
{
  Clips.RebuildMinimalData();
}

CphxObjectClip_Tool* CphxObject_Tool::GetClip( TS32 clip )
{
  return Clips.GetClip( clip );
}

void CphxObject_Tool::BackupSplineData()
{
  Clips.BackupSplineData();
}

CphxSpline_Tool * CphxObject_Tool::GetSpline( TS32 Clip, PHXSPLINETYPE Spline )
{
  return Clips.GetSpline( Clip, Spline );
}

void CphxObject_Tool::ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, TF32 Value, TBOOL AutoKey )
{
  Clips.ApplySplineTransformation( Clip, Spline, Timestamp, Value, AutoKey );
}

void CphxObject_Tool::ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey )
{
  Clips.ApplySplineTransformation( Clip, Spline, Timestamp, Value, AutoKey );
}

void CphxObject_Tool::SetTarget( CphxObject_Tool *o )
{
  TargetObject = o;
  if ( o )
    TargetObjectGuid = o->GetGUID();
  else
    TargetObjectGuid.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  Object->Target = o ? o->GetObject() : NULL;
  Object->SplineResults[ Spot_Direction_X ] = 0;
  Object->SplineResults[ Spot_Direction_Y ] = 0;
  Object->SplineResults[ Spot_Direction_Z ] = 0;
  Object->SplineResults[ Spot_Direction_Z + 1 ] = 0;
}

TBOOL CphxObject_Tool::FindParent( CphxObject_Tool *p )
{
  if ( p == this ) return true;
  if ( !Parent ) return false;
  return Parent->FindParent( p );
}

void CphxObject_Tool::SetParent( CphxObject_Tool *p, CphxScene_Tool *Scene )
{
  CphxObject_Tool *OldParent = Parent;
  Parent = p;
  Object->Parent = Parent ? Parent->GetObject() : NULL;

  if ( OldParent ) OldParent->RebuildChildList( Scene );
  if ( Parent ) Parent->RebuildChildList( Scene );

  if ( Parent ) ParentObjectGuid = Parent->GetGUID();
  else ParentObjectGuid.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );
}

void CphxObject_Tool::RebuildChildList( CphxScene_Tool *Scene )
{
  SAFEDELETEA( Object->Children );
  Object->ChildCount = 0;
  TS32 Count = 0;
  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    CphxObject_Tool *p = Scene->GetObjectByIndex( x )->GetParentObject();
    if ( p && p == this ) Count++;
  }

  if ( !Count ) return;

  Object->Children = new CphxObject*[ Count ];
  Count = 0;
  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
  {
    CphxObject_Tool *p = Scene->GetObjectByIndex( x )->GetParentObject();
    if ( p && p == this )
      Object->Children[ Count++ ] = Scene->GetObjectByIndex( x )->GetObject();
  }
  Object->ChildCount = Count;
}

TBOOL CphxObject_Tool::Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t )
{
  if ( !Object ) return false;

  D3D10_VIEWPORT vp;
  vp.TopLeftX = ClientRect.TopLeft().x;
  vp.TopLeftY = ClientRect.TopLeft().y;
  vp.Width = ClientRect.Width();
  vp.Height = ClientRect.Height();
  vp.MinDepth = 0;
  vp.MaxDepth = 1;

  D3DXVECTOR3 v1, v2, n;
  D3DXVec3Unproject( &v1, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 0 ), &vp, &ProjectionMatrix, &CameraMatrix, &TransformationMatrix );
  D3DXVec3Unproject( &v2, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 1 ), &vp, &ProjectionMatrix, &CameraMatrix, &TransformationMatrix );
  D3DXVec3Normalize( &n, &( v2 - v1 ) );

  CLine l = CLine( CVector3( v1.x, v1.y, v1.z ), CVector3( n.x, n.y, n.z ) );

  switch ( Object->ObjectType )
  {
  case Object_CamEye:
  {
    CVector3 f = CVector3( 0.5492f, 1, 1.6757f ) / 5.0f;
    CBBox b = CBBox( f*-0.5, f*0.5 );

    TF32 tmin, tmax;
    if ( b.Intersect( l, tmin, tmax ) )
    {
      if ( tmin > 0 ) t = tmin;
      else t = tmax;
      return true;
    }
    break;
  }
  case Object_Dummy:
  {
    CSphere s = CSphere( CVector3( 0, 0, 0 ), 0.125 );
    TF32 tmin, tmax;
    if ( s.Intersect( l, tmin, tmax ) )
    {
      if ( tmin > 0 ) t = tmin;
      else t = tmax;
      return true;
    }
    break;
  }
  case Object_Light:
  {
    if ( GetObject()->SplineResults[ Spline_Position_w ] == 0 )
    {
      //dirlight
      CVector3 f = CVector3( 0.5, 0.5, 0.5 );
      CBBox b = CBBox( f*-0.25, f*0.25 );
      TF32 tmin, tmax;
      if ( b.Intersect( l, tmin, tmax ) )
      {
        if ( tmin > 0 ) t = tmin;
        else t = tmax;
        return true;
      }
    }
    else
    {
      //pointlight
      CSphere s = CSphere( CVector3( 0, 0, 0 ), 0.125 );
      TF32 tmin, tmax;
      if ( s.Intersect( l, tmin, tmax ) )
      {
        if ( tmin > 0 ) t = tmin;
        else t = tmax;
        return true;
      }
      break;
    }
  }
  break;
  default:
    break;
  }

  return false;

}

void CphxObject_Tool::SetPosition( CVector3 v )
{
  GetSpline( 0, Spline_Position_x )->Spline->Value[ 0 ] = v.x;
  GetSpline( 0, Spline_Position_y )->Spline->Value[ 0 ] = v.y;
  GetSpline( 0, Spline_Position_z )->Spline->Value[ 0 ] = v.z;
}

void CphxObject_Tool::SetScale( CVector3 v )
{
  GetSpline( 0, Spline_Scale_x )->Spline->Value[ 0 ] = v.x;
  GetSpline( 0, Spline_Scale_y )->Spline->Value[ 0 ] = v.y;
  GetSpline( 0, Spline_Scale_z )->Spline->Value[ 0 ] = v.z;
}

void CphxObject_Tool::SetRotation( CQuaternion q )
{
  GetSpline( 0, Spline_Rotation )->Spline->Value[ 0 ] = q.x;
  GetSpline( 0, Spline_Rotation )->Spline->Value[ 1 ] = q.y;
  GetSpline( 0, Spline_Rotation )->Spline->Value[ 2 ] = q.z;
  GetSpline( 0, Spline_Rotation )->Spline->Value[ 3 ] = q.s;
}

CVector3 CphxObject_Tool::GetPosition()
{
  D3DXMATRIX m = GetMatrix();
  D3DXVECTOR4 v = D3DXVECTOR4( 0, 0, 0, 1 );
  D3DXVec3Transform( &v, &D3DXVECTOR3( 0, 0, 0 ), &m );

  return CVector3( v.x, v.y, v.z );
}

void CphxObject_Tool::CopySplinesFrom( CphxObject_Tool *Source )
{
  Clips.FreeMinimalData();
  Clips.Clips.FreeArray();
  Object->Clips = NULL;

  for ( TS32 x = 0; x < Source->Clips.Clips.NumItems(); x++ )
  {
    CphxObjectClip_Tool *oc = Source->Clips.Clips[ x ];
    CphxObjectClip_Tool *nc = new CphxObjectClip_Tool( oc->GetSceneClip() );
    Clips.Clips += nc;

    for ( TS32 y = 0; y < oc->GetSplineCount(); y++ )
    {
      CphxClipSpline_Tool *os = oc->GetSplineByIndex( y );
      nc->CreateSpline( os->MinimalSpline.Type );
      CphxClipSpline_Tool *ns = nc->GetSplineByIndex( nc->GetSplineCount() - 1 );
      ns->MinimalSpline.GroupingData = os->MinimalSpline.GroupingData;
      ns->MinimalSpline.MaterialParam = os->MinimalSpline.MaterialParam;
      os->Spline->CopyTo( ns->Spline );
    }

    nc->GetMaterialSplineBatch()->Copy( oc->GetMaterialSplineBatch() );

    if ( GetObjectType() == PHXOBJECT::Object_SubScene && Source->GetObjectType() == PHXOBJECT::Object_SubScene )
    {
      CphxObject_SubScene_Tool* src = (CphxObject_SubScene_Tool*)Source;
      CphxObject_SubScene_Tool* dst = (CphxObject_SubScene_Tool*)this;
      dst->GetClip( x )->SetSubSceneTarget( src->GetClip( x )->GetSubSceneTarget() );
    }

    GetClip( x )->SetRandSeed( Source->GetClip( x )->GetRandSeed() );
    GetClip( x )->SetTurbulenceFreq( Source->GetClip( x )->GetTurbulenceFreq() );
  }

  RebuildMinimalData();
}

void CphxObject_Tool::CopySplinesFrom( CphxObject_Tool *Source, int clip, CphxScene_Tool *parentScene )
{
  int clipCount = Clips.Clips.NumItems();
  Clips.FreeMinimalData();
  Clips.Clips.FreeArray();
  Object->Clips = NULL;

  for ( TS32 x = 0; x < clipCount; x++ )
  {
    CphxObjectClip_Tool *oc = Source->Clips.Clips[ clip ];
    CphxObjectClip_Tool *nc = new CphxObjectClip_Tool( parentScene->GetClipByIndex( x ) );
    Clips.Clips += nc;

    for ( TS32 y = 0; y < oc->GetSplineCount(); y++ )
    {
      CphxClipSpline_Tool *os = oc->GetSplineByIndex( y );
      nc->CreateSpline( os->MinimalSpline.Type );
      CphxClipSpline_Tool *ns = nc->GetSplineByIndex( nc->GetSplineCount() - 1 );
      ns->MinimalSpline.GroupingData = os->MinimalSpline.GroupingData;
      ns->MinimalSpline.MaterialParam = os->MinimalSpline.MaterialParam;
      os->Spline->CopyTo( ns->Spline );
    }

    nc->GetMaterialSplineBatch()->Copy( oc->GetMaterialSplineBatch() );
  }

  RebuildMinimalData();
}

CphxScene_Tool* CphxObject_Tool::GetParentScene()
{
  for ( int x = 0; x < Project.GetSceneCount(); x++ )
    for ( int y = 0; y < Project.GetSceneByIndex( x )->GetObjectCount(); y++ )
      if ( Project.GetSceneByIndex( x )->GetObjectByIndex( y ) == this )
        return Project.GetSceneByIndex( x );

  return nullptr;
}

CphxObject_Model_Tool::CphxObject_Model_Tool( CphxModel_Tool *model )
{
  Model = model;
  obj = new CphxObject_Model;
  obj->ObjectType = Object_Model;
  obj->Model = Model->GetModel();
  SetObject( obj );
  D3DXQuaternionIdentity( &obj->RotationResult );
  SetName( Model->GetName() );
  if ( Model )
    AddParent( Model );
}

CphxObject_Model_Tool::~CphxObject_Model_Tool()
{

}

void CphxObject_Model_Tool::SetKeyframerMode( void *Data )
{
  for ( TS32 x = 0; x < obj->Model->Objects.NumItems(); x++ )
    obj->ToolData = Data;
}

TBOOL CphxObject_Model_Tool::Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &tt )
{
  if ( !Model ) return false;

  float mt;
  CphxModelObject_Tool *picked = NULL;

  for ( int x = 0; x < Model->GetObjectCount(); x++ )
  {
    float t = 10000;
    D3DXMATRIX mx;
    D3DXMatrixIdentity( &mx );

    if ( Model->GetObjectByIndex( x )->Pick( ClientRect, MousePos, ProjectionMatrix, CameraMatrix, TransformationMatrix, t ) )
      if ( !picked || t < mt )
      {
        mt = t;
        picked = Model->GetObjectByIndex( x );
      }
  }

  if ( picked )
    tt = mt;

  return picked != NULL;
}

void CphxObject_Model_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );
  if ( Model )
    Node->AddChild( _T( "model" ) ).SetText( Model->GetGUID().GetString() );
}

void CphxObject_Model_Tool::UpdateModel()
{
  if ( !Model ) return;
  Model->RequestContent();
  Model->UpdateMaterialStates();
}

void CphxObject_Model_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_Scale_x );
  Clip->CreateSpline( Spline_Scale_y );
  Clip->CreateSpline( Spline_Scale_z );
  Clip->CreateSpline( Spline_Rotation );

  Clip->BuildMaterialSplines( Model );
}

void CphxObject_Model_Tool::SwapModel( CphxModel_Tool *model )
{
  RemoveParent( Model );
  Model = model;
  obj->Model = Model->GetModel();
  if ( Model )
    AddParent( Model );

  for ( int x = 0; x < Clips.GetClipCount(); x++ )
    Clips.GetClip( x )->RebuildMaterialSplines( model );
}

void CphxSceneClip::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
}

void CphxSceneClip::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
}

CphxSceneClip::CphxSceneClip()
{
  Name = _T( "New Clip" );
}

CphxSceneClip::~CphxSceneClip()
{

}

CphxObjectClipArray_Tool::CphxObjectClipArray_Tool()
{
  MinimalClipCount = 0;
}

CphxObjectClipArray_Tool::~CphxObjectClipArray_Tool()
{
  FreeMinimalData();
  Clips.FreeArray();
}

void CphxObjectClipArray_Tool::AddClip( CphxSceneClip *Clip, TS32 referenceClipId )
{
  CphxObjectClip_Tool *nc = new CphxObjectClip_Tool( Clip );

  if ( referenceClipId >= 0 )
  {
    CphxObjectClip_Tool *rc = Clips[ referenceClipId ];
    nc->SetSubSceneTarget( rc->GetSubSceneTarget() );
    nc->SetRandSeed( 0 );
    nc->SetTurbulenceFreq( 10 );
  }

  Clips += nc;
  nc->BuildSplines( Owner );
}

void CphxObjectClipArray_Tool::DeleteClip()
{

}

void CphxObjectClipArray_Tool::RebuildMinimalData()
{
  if ( !Owner || !Owner->GetObject() ) return;
  FreeMinimalData();

  if ( !Clips.NumItems() ) return;

  Owner->GetObject()->Clips = new CphxObjectClip *[ Clips.NumItems() ];

  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
  {
    CphxObjectClip *nc = new CphxObjectClip;
    Owner->GetObject()->Clips[ x ] = nc;
    Clips[ x ]->BuildMinimalData( nc );
  }
  MinimalClipCount = Clips.NumItems();
}

void CphxObjectClipArray_Tool::FreeMinimalData()
{
  if ( !Owner || !Owner->GetObject() ) return;
  CphxObjectClip **minimalClips = Owner->GetObject()->Clips;

  for ( TS32 x = 0; x < MinimalClipCount; x++ )
  {
    SAFEDELETE( minimalClips[ x ]->Splines );
    SAFEDELETEA( minimalClips[ x ] );
  }
  SAFEDELETEA( minimalClips );
  MinimalClipCount = 0;
}

void CphxObjectClipArray_Tool::UpdateSplines()
{
  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
    Clips[ x ]->UpdateSplines();
}

int CphxObjectClipArray_Tool::GetClipCount()
{
  return Clips.NumItems();
}

CphxObjectClip_Tool* CphxObjectClipArray_Tool::GetClip( TS32 Clip )
{
  return Clips[ Clip ];
}

void CphxObjectClipArray_Tool::BackupSplineData()
{
  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
    Clips[ x ]->BackupSplineData();
}

CphxSpline_Tool * CphxObjectClipArray_Tool::GetSpline( TS32 Clip, PHXSPLINETYPE Spline )
{
  return Clips[ Clip ]->GetSpline( Spline );
}

void CphxObjectClipArray_Tool::ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, TF32 Value, TBOOL AutoKey )
{
  Clips[ Clip ]->ApplySplineTransformation( Spline, Timestamp, Value, AutoKey );
}

void CphxObjectClipArray_Tool::ApplySplineTransformation( TS32 Clip, PHXSPLINETYPE Spline, TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey )
{
  Clips[ Clip ]->ApplySplineTransformation( Spline, Timestamp, Value, AutoKey );
}

void CphxObjectClipArray_Tool::ExportData( CXMLNode *Node )
{
  for ( TS32 x = 0; x < Clips.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "clipdata" ) );
    n.SetAttribute( _T( "targetclip" ), Clips[ x ]->GetSceneClipGUID().GetString() );
    Clips[ x ]->ExportData( &n );
  }
}

void CphxObjectClipArray_Tool::ImportData( CXMLNode *Node )
{
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "clipdata" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "clipdata" ), x );
    CphxGUID g;
    if ( n.HasAttribute( _T( "targetclip" ) ) ) g.SetString( n.GetAttributeAsString( _T( "targetclip" ) ).GetPointer() );

    TBOOL Found = false;
    for ( TS32 y = 0; y < Clips.NumItems() && !Found; y++ )
    {
      if ( Clips[ y ]->GetSceneClipGUID() == g )
      {
        Clips[ y ]->ImportData( &n );
        Found = true;
      }
    }

    if ( !Found )
      LOG_ERR( "[phx] Failed to identify clip %s during import", g.GetString() );

  }
}

CphxObjectClip_Tool::CphxObjectClip_Tool( CphxSceneClip *clip )
{
  SceneClip = clip;
}

CphxObjectClip_Tool::~CphxObjectClip_Tool()
{
  Splines.FreeArray();
}

void CphxObjectClip_Tool::BuildSplines( CphxObject_Tool *Owner )
{
  Owner->CreateSplines( this );
}

void CphxObjectClip_Tool::CreateSpline( PHXSPLINETYPE Spline )
{
  CphxClipSpline_Tool *t = new CphxClipSpline_Tool( Spline );

  if ( Spline == Spline_Scale_x || Spline == Spline_Scale_y || Spline == Spline_Scale_z )
    t->Spline->Spline->Value[ 0 ] = 1;

  if ( Spline == Spline_SubScene_Clip )
  {
    t->Spline->Spline->Interpolation = INTERPOLATION_CONSTANT;
  }

  if ( Spline == Spline_Rotation )
  {
    t->Spline->Spline->Value[ 0 ] = 0;
    t->Spline->Spline->Value[ 1 ] = 0;
    t->Spline->Spline->Value[ 2 ] = 0;
    t->Spline->Spline->Value[ 3 ] = 1;
  }

  if ( Spline == Spline_Camera_FOV )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  if ( Spline == Spline_Light_DiffuseR || Spline == Spline_Light_DiffuseG || Spline == Spline_Light_DiffuseB ||
       Spline == Spline_Light_SpecularR || Spline == Spline_Light_SpecularG || Spline == Spline_Light_SpecularB )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  if ( Spline == Spline_Particle_Life )
  {
    t->Spline->Spline->Value[ 0 ] = 10;
  }

  if ( Spline == Spline_Particle_EmissionPerSecond )
  {
    t->Spline->Spline->Value[ 0 ] = PARTICLEENGINE_FRAMERATE;
  }

  if ( Spline == Spline_Particle_EmissionVelocity )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  if ( Spline == Spline_Particle_EmissionRotation )
  {
    t->Spline->Spline->Value[ 0 ] = 0;
  }

  if ( Spline == Spline_AffectorPower )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  if ( Spline == Spline_Particle_Scale )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  if ( Spline == Spline_Particle_Stretch_X )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  if ( Spline == Spline_Particle_Stretch_Y )
  {
    t->Spline->Spline->Value[ 0 ] = 1;
  }

  Splines += t;
}

void CphxObjectClip_Tool::RebuildMaterialSplines( CphxModel_Tool *m )
{
  MaterialSplines.BuildBatch( m );
}

void CphxObjectClip_Tool::RebuildMaterialSplines( CphxObject_ParticleEmitter_CPU_Tool *m )
{
  MaterialSplines.BuildBatch( m );
}

void CphxObjectClip_Tool::BuildMinimalData( CphxObjectClip *clip )
{
  if ( !clip ) return;

  SubSceneTarget = Project.GetScene( SubSceneTargetGUID );
  if ( SubSceneTarget )
    clip->SubSceneTarget = &SubSceneTarget->Scene;
  else
    clip->SubSceneTarget = nullptr;

  clip->RandSeed = RandSeed;
  clip->TurbulenceFrequency = TurbulenceFrequency;

  clip->SplineCount = Splines.NumItems();
  clip->Splines = NULL;
  clip->MaterialSplines = MaterialSplines.Batch;

  if ( !clip->SplineCount ) return;

  clip->Splines = new CphxClipSpline*[ clip->SplineCount ];
  for ( TS32 x = 0; x < clip->SplineCount; x++ )
    clip->Splines[ x ] = &Splines[ x ]->MinimalSpline;
}

void CphxObjectClip_Tool::UpdateSplines()
{
  SubSceneTarget = Project.GetScene( SubSceneTargetGUID );

  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
    Splines[ x ]->Update();
}

void CphxObjectClip_Tool::BackupSplineData()
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
    Splines[ x ]->Spline->Backup();
}

CphxSpline_Tool * CphxObjectClip_Tool::GetSpline( PHXSPLINETYPE Spline )
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
    if ( Splines[ x ]->MinimalSpline.Type == Spline )
      return Splines[ x ]->Spline;
  return NULL;
}

CphxClipSpline_Tool * CphxObjectClip_Tool::GetClipSpline( PHXSPLINETYPE Spline )
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
    if ( Splines[ x ]->MinimalSpline.Type == Spline )
      return Splines[ x ];
  return NULL;
}

void CphxObjectClip_Tool::ApplySplineTransformation( PHXSPLINETYPE Spline, TF32 Timestamp, TF32 Value, TBOOL AutoKey )
{
  CphxSpline_Tool *s = GetSpline( Spline );
  if ( !s ) return;
  s->ApplySplineTransformation( Timestamp, Value, AutoKey );
}

void CphxObjectClip_Tool::ApplySplineTransformation( PHXSPLINETYPE Spline, TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey )
{
  CphxSpline_Tool *s = GetSpline( Spline );
  if ( !s ) return;
  s->ApplySplineTransformation( Timestamp, Value, AutoKey );
}

void CphxObjectClip_Tool::ExportData( CXMLNode *Node )
{
  if ( SubSceneTarget )
  {
    Node->AddChild( _T( "subscenetarget" ) ).SetText( SubSceneTarget->GetGUID().GetString() );
  }

  Node->AddChild( _T( "randseed" ) ).SetInt( RandSeed );
  Node->AddChild( _T( "turbulencefreq" ) ).SetInt( TurbulenceFrequency );

  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "clipspline" ) );
    Splines[ x ]->ExportData( &n );
  }

  for ( TS32 x = 0; x < MaterialSplines.Splines.NumItems(); x++ )
  {
    if ( MaterialSplines.Splines[ x ]->Spline.Target->Scope == PARAM_ANIMATED )
    {
      CXMLNode n = Node->AddChild( _T( "ParamData" ) );
      n.AddChild( _T( "TargetGUID" ) ).SetText( MaterialSplines.Splines[ x ]->TargetParamGUID.GetString() );
      n.AddChild( _T( "GroupingGUID" ) ).SetText( MaterialSplines.Splines[ x ]->TargetObjectGUID.GetString() );
      CXMLNode s = n.AddChild( _T( "Spline" ) );
      MaterialSplines.Splines[ x ]->Splines[ 0 ].ExportData( s );
      if ( MaterialSplines.Splines[ x ]->Spline.Target->Type == PARAM_COLOR )
      {
        for ( TS32 y = 1; y < 4; y++ )
        {
          s = n.AddChild( _T( "Spline" ) );
          MaterialSplines.Splines[ x ]->Splines[ y ].ExportData( s );
        }
      }
    }
  }
}

void CphxObjectClip_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "subscenetarget" ) ) )
  {
    CXMLNode n = Node->GetChild( _T( "subscenetarget" ) );
    SubSceneTargetGUID.SetString( n.GetText().GetPointer() );
    SubSceneTarget = Project.GetScene( SubSceneTargetGUID );
  }

  if ( Node->GetChildCount( _T( "randseed" ) ) )
  {
    int seed = 0;
    Node->GetChild( _T( "randseed" ) ).GetValue( seed );
    RandSeed = seed;
  }

  if ( Node->GetChildCount( _T( "turbulencefreq" ) ) )
  {
    int seed = 0;
    Node->GetChild( _T( "turbulencefreq" ) ).GetValue( seed );
    TurbulenceFrequency = seed;
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "clipspline" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "clipspline" ), x );
    if ( !n.HasAttribute( _T( "type" ) ) ) continue;
    PHXSPLINETYPE type = Spline_Position_x;
    n.GetAttributeAsInteger( _T( "type" ), (TS32*)&type );

    CphxClipSpline_Tool *s = new CphxClipSpline_Tool( type );
    s->ImportData( &n );

    TBOOL Found = false;

    for ( TS32 y = 0; y < Splines.NumItems(); y++ )
    {
      if ( Splines[ y ]->MinimalSpline.Type == s->MinimalSpline.Type &&
           Splines[ y ]->Parameter == s->Parameter &&
           Splines[ y ]->GroupingObject == s->GroupingObject ) //duplicate found
      {
        if ( Found )
        {
          LOG_ERR( "[phx] Multiple copies of the same spline found in an object!" );
          continue;
        }

        //replace spline
        CphxClipSpline_Tool *old = Splines[ y ];
        Splines[ y ] = s;
        SAFEDELETE( old );

        Found = true;
      }
    }

    if ( !Found ) Splines += s;
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "ParamData" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "ParamData" ), x );
    if ( !n.GetChildCount( _T( "TargetGUID" ) ) ) continue;
    if ( !n.GetChildCount( _T( "GroupingGUID" ) ) ) continue;

    CphxGUID Target;
    Target.SetString( n.GetChild( _T( "TargetGUID" ) ).GetText().GetPointer() );

    CphxGUID Group;
    Group.SetString( n.GetChild( _T( "GroupingGUID" ) ).GetText().GetPointer() );

    CphxMaterialParameter_Tool *param = Project.GetMaterialParameter( Target );

    TBOOL Found = false;
    for ( TS32 y = 0; y < MaterialSplines.Splines.NumItems(); y++ )
    {
      if ( MaterialSplines.Splines[ y ]->TargetParamGUID == Target && ( MaterialSplines.Splines[ y ]->TargetObjectGUID == Group || ( param && param->Parameter.Type == PARAM_PARTICLELIFEFLOAT ) ) )
      {
        Found = true;
        if ( param && param->Parameter.Type == PARAM_PARTICLELIFEFLOAT )
          MaterialSplines.Splines[ y ]->TargetObjectGUID = Group;

        for ( TS32 z = 0; z < n.GetChildCount( _T( "Spline" ) ); z++ )
          MaterialSplines.Splines[ y ]->Splines[ z ].ImportData( n.GetChild( _T( "Spline" ), z ) );

        break;
      }
    }

    if ( !Found )
      LOG_ERR( "[import] Event data import error: referenced parameter %s not found", Target.GetString() );

  }


}

CphxGUID CphxObjectClip_Tool::GetSceneClipGUID()
{
  return SceneClip->GetGUID();
}

void CphxObjectClip_Tool::BuildMaterialSplines( CphxModel_Tool *m )
{
  MaterialSplines.BackupSplines();
  MaterialSplines.BuildBatch( m );
  MaterialSplines.RestoreSplines();
}

void CphxObjectClip_Tool::BuildMaterialSplines( CphxObject_ParticleEmitter_CPU_Tool *m )
{
  MaterialSplines.BackupSplines();
  MaterialSplines.BuildBatch( m );
  MaterialSplines.RestoreSplines();
}

void CphxObjectClip_Tool::SetSubSceneTarget( CphxScene_Tool* target )
{
  SubSceneTarget = target;
  if ( target )
  {
    SubSceneTargetGUID = target->GetGUID();
  }
  else
    SubSceneTargetGUID = CphxGUID();
}

CphxClipSpline_Tool::CphxClipSpline_Tool( PHXSPLINETYPE spline )
{
  Parameter = NULL;
  GroupingObject = NULL;

  MinimalSpline.GroupingData = NULL;
  MinimalSpline.MaterialParam = NULL;
  MinimalSpline.Type = spline;

  switch ( spline )
  {
  case Spline_Rotation:
    Spline = new CphxSpline_Tool_Quaternion16();
    MinimalSpline.Spline = Spline->Spline;
    break;
  default:
    Spline = new CphxSpline_Tool_float16();
    MinimalSpline.Spline = Spline->Spline;
    break;
  }
}

CphxClipSpline_Tool::~CphxClipSpline_Tool()
{
  SAFEDELETE( Spline );
}

void CphxClipSpline_Tool::Update()
{
  Spline->Sort();
  Spline->UpdateSplineKeys();
}

void CphxClipSpline_Tool::ExportData( CXMLNode *Node )
{
  Node->SetAttributeFromInteger( _T( "type" ), MinimalSpline.Type );
  if ( Parameter ) Node->SetAttribute( _T( "materialparam" ), Parameter->GetGUID().GetString() );
  if ( GroupingObject ) Node->SetAttribute( _T( "groupingobject" ), GroupingObject->GetGUID().GetString() );
  CXMLNode n = Node->AddChild( _T( "spline" ) );
  Spline->ExportData( n );
}

void CphxClipSpline_Tool::ImportData( CXMLNode *Node )
{
  TS32 t = 0;
  if ( Node->HasAttribute( _T( "type" ) ) ) Node->GetAttributeAsInteger( _T( "type" ), &t );
  MinimalSpline.Type = (PHXSPLINETYPE)t;
  if ( Node->HasAttribute( _T( "materialparam" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetAttributeAsString( _T( "materialparam" ) ).GetPointer() );
    Parameter = Project.GetMaterialParameter( g );
  }

  if ( Node->HasAttribute( _T( "groupingobject" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetAttributeAsString( _T( "groupingobject" ) ).GetPointer() );
    GroupingObject = Project.GetModelObject( g );
  }

  if ( Node->GetChildCount( _T( "spline" ) ) )
  {
    CXMLNode n = Node->GetChild( _T( "spline" ) );
    Spline->ImportData( n );
  }

  //fov fix

  //if (t == Spline_Camera_FOV)
  //{
  //	for (TS32 x = 0; x < Spline->Keys.NumItems(); x++)
  //	{
  //		for (TS32 y = 0; y < 4; y++)
  //		{
  //			Spline->Keys[x]->Key.Value[y] = Spline->Keys[x]->Key.Value[y] / (3.14159265359f / 4.0f / 45.0f);
  //			Spline->Keys[x]->Stored.Value[y] = Spline->Keys[x]->Stored.Value[y] / (3.14159265359f / 4.0f / 45.0f);
  //		}
  //	}
  //	for (TS32 y = 0; y < 4; y++)
  //	{
  //		Spline->ValueBackup[y] /= 3.14159265359f / 4.0f / 45.0f;
  //		Spline->Spline->Value[y] /= 3.14159265359f / 4.0f / 45.0f;
  //	}
  //	Spline->UpdateSplineKeys();
  //}
}

TBOOL CphxClipSpline_Tool::IsDefaultSpline()
{
  D3DXFLOAT16 constant[ 4 ];
  if ( !Spline->IsConstant( constant ) )
    return false;

  switch ( MinimalSpline.Type )
  {
    case Spline_Scale_x:
    case Spline_Scale_y:
    case Spline_Scale_z:
    case Spline_Light_DiffuseR:
    case Spline_Light_DiffuseG:
    case Spline_Light_DiffuseB:
    case Spline_Light_SpecularR:
    case Spline_Light_SpecularG:
    case Spline_Light_SpecularB:
    case Spline_Camera_FOV:
    case Spline_Particle_EmissionVelocity:
    case Spline_AffectorPower:
    case Spline_Particle_Scale:
    case Spline_Particle_Stretch_X:
    case Spline_Particle_Stretch_Y:
      return constant[ 0 ] == D3DXFLOAT16( 1.0f );
    case Spline_Rotation:
      return constant[ 0 ] == D3DXFLOAT16( 0.0f ) && constant[ 1 ] == D3DXFLOAT16( 0.0f ) && constant[ 2 ] == D3DXFLOAT16( 0.0f ) && constant[ 3 ] == D3DXFLOAT16( 1.0f );
    case Spline_Light_AmbientR:
    case Spline_Light_AmbientG:
    case Spline_Light_AmbientB:
    case Spline_SubScene_Clip:
    case Spline_SubScene_Time:
    case Spline_Position_x:
    case Spline_Position_y:
    case Spline_Position_z:
    case Spline_Position_w:
    case Spline_Light_Exponent:
    case Spline_Light_Cutoff:
    case Spline_Light_Attenuation_Linear:
    case Spline_Light_Attenuation_Quadratic:
    case Spline_Camera_Roll:
    case Spline_Particle_EmissionRotation:
    case Spline_Particle_EmissionRotationChaos:
    case Spline_Particle_EmissionVelocityChaos:
    case Spline_Particle_LifeChaos:
    case Spline_Light_OrthoX:
    case Spline_Light_OrthoY:
    case Spline_Particle_ScaleChaos:
    case Spline_SubScene_RepeatCount:
    case Spline_SubScene_RepeatTimeOffset:
      return constant[ 0 ] == D3DXFLOAT16( 0.0f );
    case Spline_Particle_EmissionPerSecond:
      return constant[ 0 ] == D3DXFLOAT16( 25.0f );
    case Spline_Particle_Life:
      return constant[ 0 ] == D3DXFLOAT16( 10.0f );

    case Spot_Direction_X:
    case Spot_Direction_Y:
    case Spot_Direction_Z:
    case Spline_Particle_Offset_x:
    case Spline_Particle_Offset_y:
    case Spline_Particle_Offset_z:
    case Spline_Particle_EmissionTrigger:
      return false;
  default:
      return false;
    //break;
  }


  if ( MinimalSpline.Type != Spline_Position_x &&
       MinimalSpline.Type != Spline_Position_y &&
       MinimalSpline.Type != Spline_Position_z &&
       MinimalSpline.Type != Spline_Scale_x &&
       MinimalSpline.Type != Spline_Scale_y &&
       MinimalSpline.Type != Spline_Scale_z &&
       MinimalSpline.Type != Spline_Rotation ) return false;

  if ( MinimalSpline.Spline->KeyCount > 1 ) return false;
  if ( MinimalSpline.Spline->Interpolation != INTERPOLATION_LINEAR ) return false;
  if ( MinimalSpline.Spline->Loop ) return false;

  if ( Spline->GetSplineType() == Spline_F16 )
  {
    auto *s = (CphxSpline_float16 *)Spline->Spline;
    if ( s->Waveform != WAVEFORM_NONE ) return false;
  }

  if ( MinimalSpline.Spline->KeyCount == 0 )
  {
    if ( MinimalSpline.Type == Spline_Position_x && MinimalSpline.Spline->Value[ 0 ] == 0 ) return true;
    if ( MinimalSpline.Type == Spline_Position_y && MinimalSpline.Spline->Value[ 0 ] == 0 ) return true;
    if ( MinimalSpline.Type == Spline_Position_z && MinimalSpline.Spline->Value[ 0 ] == 0 ) return true;

    if ( MinimalSpline.Type == Spline_Scale_x && MinimalSpline.Spline->Value[ 0 ] == 1 ) return true;
    if ( MinimalSpline.Type == Spline_Scale_y && MinimalSpline.Spline->Value[ 0 ] == 1 ) return true;
    if ( MinimalSpline.Type == Spline_Scale_z && MinimalSpline.Spline->Value[ 0 ] == 1 ) return true;

    if ( MinimalSpline.Type == Spline_Rotation &&
         MinimalSpline.Spline->Value[ 0 ] == 0 &&
         MinimalSpline.Spline->Value[ 1 ] == 0 &&
         MinimalSpline.Spline->Value[ 2 ] == 0 &&
         MinimalSpline.Spline->Value[ 3 ] == 1 ) return true;
  }

  if ( MinimalSpline.Spline->KeyCount == 1 )
  {
    if ( MinimalSpline.Type == Spline_Position_x && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 0 ) ) return true;
    if ( MinimalSpline.Type == Spline_Position_y && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 0 ) ) return true;
    if ( MinimalSpline.Type == Spline_Position_z && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 0 ) ) return true;

    if ( MinimalSpline.Type == Spline_Scale_x && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 1 ) ) return true;
    if ( MinimalSpline.Type == Spline_Scale_y && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 1 ) ) return true;
    if ( MinimalSpline.Type == Spline_Scale_z && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 1 ) ) return true;

    if ( MinimalSpline.Type == Spline_Rotation
         && MinimalSpline.Spline->Keys[ 0 ]->Value[ 0 ] == D3DXFLOAT16( 0 )
         && MinimalSpline.Spline->Keys[ 0 ]->Value[ 1 ] == D3DXFLOAT16( 0 )
         && MinimalSpline.Spline->Keys[ 0 ]->Value[ 2 ] == D3DXFLOAT16( 0 )
         && MinimalSpline.Spline->Keys[ 0 ]->Value[ 3 ] == D3DXFLOAT16( 1 ) ) return true;
  }

  return false;
}

CphxObject_Light_Tool::CphxObject_Light_Tool()
{
  Name = _T( "New Light" );
  CphxObject *obj = new CphxObject;
  obj->ObjectType = Object_Light;
  SetObject( obj );
  memset( obj->SplineResults, 0, sizeof( obj->SplineResults ) );
  D3DXQuaternionIdentity( &obj->RotationResult );
  SetPointLight( false );
}

CphxObject_Light_Tool::~CphxObject_Light_Tool()
{

}

void CphxObject_Light_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_Light_AmbientR );
  Clip->CreateSpline( Spline_Light_AmbientG );
  Clip->CreateSpline( Spline_Light_AmbientB );
  Clip->CreateSpline( Spline_Light_DiffuseR );
  Clip->CreateSpline( Spline_Light_DiffuseG );
  Clip->CreateSpline( Spline_Light_DiffuseB );
  Clip->CreateSpline( Spline_Light_SpecularR );
  Clip->CreateSpline( Spline_Light_SpecularG );
  Clip->CreateSpline( Spline_Light_SpecularB );
  Clip->CreateSpline( Spline_Light_Exponent );
  Clip->CreateSpline( Spline_Light_Cutoff );
  Clip->CreateSpline( Spline_Light_Attenuation_Linear );
  Clip->CreateSpline( Spline_Light_Attenuation_Quadratic );
  Clip->CreateSpline( Spline_Light_OrthoX );
  Clip->CreateSpline( Spline_Light_OrthoY );
}

void CphxObject_Light_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );
  Node->AddChild( _T( "pointlight" ) ).SetInt( PointLight );
}

void CphxObject_Light_Tool::SetPointLight( TBOOL p )
{
  PointLight = p;
  if ( GetObject() )
    GetObject()->SplineResults[ Spline_Position_w ] = PointLight ? 1.0f : 0;
}

void CphxObject_Light_Tool::ImportData( CXMLNode *Node )
{
  CphxObject_Tool::ImportData( Node );
  if ( Node->GetChildCount( _T( "pointlight" ) ) )
  {
    TS32 i = 0;
    if ( Node->GetChild( _T( "pointlight" ) ).GetValue( i ) )
      SetPointLight( i != 0 );
  }
}

void CphxObject_SubScene_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );
  //if ( Scene )
  //  Node->AddChild( _T( "scene" ) ).SetText( Scene->GetGUID().GetString() );
}

CphxObject_SubScene_Tool::CphxObject_SubScene_Tool()
{
  //Scene = scene;
  obj = new CphxObject_SubScene;
  obj->ObjectType = Object_SubScene;
  //obj->Scene = &Scene->Scene;
  obj->SubSceneTarget = nullptr;
  SetObject( obj );
  D3DXQuaternionIdentity( &obj->RotationResult );
  //SetName( scene->GetName() );

  //for ( int x = 0; x < Clips.GetClipCount(); x++ )
  //  Clips.GetClip( x )->SetSubSceneTarget( scene );
}

CphxObject_SubScene_Tool::~CphxObject_SubScene_Tool()
{

}

void CphxObject_SubScene_Tool::SetKeyframerMode( void *Data )
{
  for ( int x = 0; x < Clips.GetClipCount(); x++ )
  {
    auto* Scene = Clips.GetClip( x )->GetSubSceneTarget();
    if ( Scene )
      Scene->SetKeyframerMode();
  }

}

void CphxObject_SubScene_Tool::UpdateModel()
{
  for ( int x = 0; x < Clips.GetClipCount(); x++ )
  {
    auto* Scene = Clips.GetClip( x )->GetSubSceneTarget();
    if ( Scene )
      Scene->RequestContent();
  }
}

#include "../apEx/WorkBench.h"

TBOOL CphxObject_SubScene_Tool::Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t )
{
  auto* editedScene = GetActiveWorkBench()->GetEditedScene();
  if ( !editedScene )
    return false;

  TS32 clipID = editedScene->GetActiveClip();

  auto* Scene = GetClip( clipID )->GetSubSceneTarget();

  if ( !Scene ) return false;

  TBOOL Hit = false;

  D3DXMATRIX mx;
  D3DXMatrixMultiply( &mx, &TransformationMatrix, &GetMatrix() );

  for ( TS32 x = 0; x < Scene->GetObjectCount(); x++ )
    Hit |= Scene->GetObjectByIndex( x )->Pick( ClientRect, MousePos, ProjectionMatrix, CameraMatrix, mx, t );

  return Hit;
}

void CphxObject_SubScene_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_Scale_x );
  Clip->CreateSpline( Spline_Scale_y );
  Clip->CreateSpline( Spline_Scale_z );
  Clip->CreateSpline( Spline_Rotation );
  Clip->CreateSpline( Spline_SubScene_Clip );
  Clip->CreateSpline( Spline_SubScene_Time );
  Clip->CreateSpline( Spline_SubScene_RepeatCount );
  Clip->CreateSpline( Spline_SubScene_RepeatTimeOffset );
}

void CphxObject_SubScene_Tool::SwapScene( CphxScene_Tool *newScene, TS32 Clip )
{
  for ( int x = 0; x < Clips.GetClipCount(); x++ )
    RemoveParent( Clips.GetClip( x )->GetSubSceneTarget() );

  auto* clipData = GetClip( Clip );
  clipData->SetSubSceneTarget( newScene );

  for ( int x = 0; x < Clips.GetClipCount(); x++ )
    AddParent( Clips.GetClip( x )->GetSubSceneTarget() );

  RebuildMinimalData();
}

CphxScene_Tool* CphxObject_SubScene_Tool::GetSubScene( TS32 clip )
{
  return GetClip( clip )->GetSubSceneTarget();
}

void CphxObject_SubScene_Tool::MarkSpecialRequired()
{
  for ( int x = 0; x < Clips.GetClipCount(); x++ )
  {
    if ( !Clips.GetClip( x )->GetSceneClip()->IsRequired() )
      continue;

    auto* subScene = GetSubScene( x );
    subScene->MarkAsRequired();

    auto* spline = GetClip( x )->GetClipSpline( Spline_SubScene_Clip );
    if ( !spline )
    {
      subScene->GetClipByIndex( 0 )->MarkAsRequired();
      continue;
    }

    float result[ 4 ];
    spline->Spline->GetValue( 0, result );
    subScene->GetClipByIndex( (int)result[ 0 ] )->MarkAsRequired();

    for ( int y = 0; y < spline->Spline->Keys.NumItems(); y++ )
      subScene->GetClipByIndex( (int)spline->Spline->Keys[ y ]->Key.Value[ 0 ] )->MarkAsRequired();
  }
}

//CphxObject_ParticleEmitter_Tool::CphxObject_ParticleEmitter_Tool()
//{
//	obj = new CphxObject_ParticleEmitter;
//	obj->ObjectType = Object_ParticleEmitter;
//	obj->Particles = NULL;
//	obj->ParticleView = NULL;
//	obj->UpdateShader = NULL;
//	obj->SpawnShader = NULL;
//	SetObject(obj);
//	D3DXQuaternionIdentity(&obj->RotationResult);
//	SetName(CString(_T("New Emitter")));
//}

//CphxObject_ParticleEmitter_Tool::~CphxObject_ParticleEmitter_Tool()
//{
//
//}
//
//void CphxObject_ParticleEmitter_Tool::SetKeyframerMode(void *Data)
//{
//
//}
//
//void CphxObject_ParticleEmitter_Tool::UpdateParticleResources()
//{
//
//}

CphxObject_ParticleEmitter_CPU_Tool::CphxObject_ParticleEmitter_CPU_Tool()
{
  obj = new CphxObject_ParticleEmitter_CPU;
  obj->ObjectType = Object_ParticleEmitterCPU;
  obj->BufferSize = 10;
  obj->Particles = new PHXPARTICLE[ 1 << obj->BufferSize ];
  memset( obj->Particles, 0, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
  obj->DistanceBuffer = new PHXPARTICLEDISTANCE[ 1 << obj->BufferSize ];
  memset( obj->DistanceBuffer, 0, sizeof( PHXPARTICLEDISTANCE )*( 1 << obj->BufferSize ) );
  backupBuffer = new PHXPARTICLE[ 1 << obj->BufferSize ];
  memset( backupBuffer, 0, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
  tempBuffer = new PHXPARTICLE[ 1 << obj->BufferSize ];
  memset( tempBuffer, 0, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
  obj->VertexBuffer = NULL;
  obj->SplineTexture = NULL;
  obj->SplineTextureView = NULL;
  obj->EmissionFraction = 0;
  obj->Ticks = 0;
  obj->Aging = true;
  obj->RandRotate = false;
  obj->TwoDirRotate = true;
  obj->TwoDirRotate = false;
  obj->EmitterType = 0;
  obj->InnerRadius = 0;
  obj->RandSeed = 0;
  obj->StartCount = 0;
  obj->Sort = false;
  obj->TargetDirection = D3DXVECTOR3( 0, 0, 0 );
  obj->LiveCount = 0;
  VertexBuffer = NULL;

  Material = DefaultMaterial;
  EmitedObject = nullptr;
  EmitedScene = nullptr;
  MaterialStateSize = 0;
  obj->MaterialState = NULL;
  obj->Material = &DefaultMaterial->Material; //default display material should be here

  SetObject( obj );
  D3DXQuaternionIdentity( &obj->RotationResult );
  SetName( CString( _T( "New Emitter (CPU)" ) ) );

  UpdateMaterialState();
}

CphxObject_ParticleEmitter_CPU_Tool::~CphxObject_ParticleEmitter_CPU_Tool()
{
  if ( obj->SplineTextureView ) obj->SplineTextureView->Release();
  if ( obj->SplineTexture ) obj->SplineTexture->Release();
  FreeMaterialState();
  SAFEDELETE( VertexBuffer );
  SAFEDELETEA( obj->Particles );
  SAFEDELETEA( obj->DistanceBuffer );
  SAFEDELETEA( backupBuffer );
  SAFEDELETEA( tempBuffer );
}

void CphxObject_ParticleEmitter_CPU_Tool::SetKeyframerMode( void *Data )
{
  if ( EmitedObject )
  {
    obj->ToolData = Data;
  }
}

TBOOL CphxObject_ParticleEmitter_CPU_Tool::GenerateResource( CCoreDevice *Dev )
{
  UpdateParticleResources( Dev );
  return true;
}

void CphxObject_ParticleEmitter_CPU_Tool::SetMaterial( CphxMaterial_Tool *m, TBOOL Update )
{
  CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)obj;

  RemoveParent( Material );
  Material = m;
  AddParent( Material );
  o->Material = &m->Material;
  InvalidateUptoDateFlag();

  if ( m )
  {
    if ( !Update )
      MaterialData.SetDefaultValues( m );
    else
      MaterialData.SetMissingDefaultValues( m );
  }

  UpdateMaterialState();
}

void CphxObject_ParticleEmitter_CPU_Tool::SetEmittedModel( CphxModel_Tool *m )
{
  if ( EmitedScene )
    SetEmittedScene( nullptr );

  RemoveParent( EmitedObject );
  EmitedObject = m;
  AddParent( EmitedObject, true );
  if ( m )
    obj->ObjectToEmit = EmitedObject->GetModel();
  else
    obj->ObjectToEmit = nullptr;

  for ( int x = 0; x < Clips.GetClipCount(); x++ )
    Clips.GetClip( x )->RebuildMaterialSplines( this );

  if ( m )
    m->InvalidateUptoDateFlag();
}

void CphxObject_ParticleEmitter_CPU_Tool::SetEmittedScene( CphxScene_Tool *m )
{
  if ( EmitedObject )
    SetEmittedModel( nullptr );

  RemoveParent( EmitedScene );
  EmitedScene = m;
  AddParent( EmitedScene, true );
  if ( m )
    obj->SceneToEmit = &EmitedScene->Scene;
  else
    obj->SceneToEmit = nullptr;

  for ( int x = 0; x < Clips.GetClipCount(); x++ )
    Clips.GetClip( x )->RebuildMaterialSplines( this );

  if ( m )
    m->InvalidateUptoDateFlag();
}

void CphxObject_ParticleEmitter_CPU_Tool::CopyDataFrom( CphxObject_ParticleEmitter_CPU_Tool* data )
{
  SetTarget( data->TargetObject );
  CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)obj;
  CphxObject_ParticleEmitter_CPU *d = (CphxObject_ParticleEmitter_CPU*)data->obj;

  o->Aging = d->Aging;
  o->RandRotate = d->RandRotate;
  o->TwoDirRotate = d->TwoDirRotate;
  o->RotateToDirection = d->RotateToDirection;
  o->Sort = d->Sort;
  o->BufferSize = d->BufferSize;
  o->EmitterType = d->EmitterType;
  o->InnerRadius = d->InnerRadius;
  o->StartCount = d->StartCount;
  o->RandSeed = d->RandSeed;
  o->ObjectToEmit = d->ObjectToEmit;

  MaterialData.Copy( &data->MaterialData );
  UpdateMaterialState();
}

void CphxObject_ParticleEmitter_CPU_Tool::UpdateMaterialState()
{
  AllocateMaterialState();

  if ( !Material ) return;
  CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)obj;
  
  if ( EmitedObject )
  {
    EmitedObject->RequestContent();
    EmitedObject->UpdateMaterialStates();
  }

  if ( EmitedScene )
  {
    EmitedScene->RequestContent();
  }

  MaterialData.UpdateParams( Material );
  UpdateMaterialTextures();

  //apply parameters to material instance

  TS32 passcnt = 0;
  for ( TS32 x = 0; x < Material->Techniques.NumItems(); x++ )
    Material->Techniques[ x ]->CreateInstancedData( o->MaterialState, passcnt );

}

void CphxObject_ParticleEmitter_CPU_Tool::UpdateMaterialTextures()
{
  if ( !Material ) return;
  CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)obj;

  MaterialData.UpdateTextures( Material );

  TS32 passcnt = 0;
  for ( TS32 x = 0; x < Material->Techniques.NumItems(); x++ )
    Material->Techniques[ x ]->CreateInstancedData_Textures( o->MaterialState, passcnt );
}


void CphxObject_ParticleEmitter_CPU_Tool::FreeMaterialState()
{
  CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)obj;
  if ( !o->MaterialState ) return;
  if ( !MaterialStateSize ) return;

  for ( TS32 x = 0; x < MaterialStateSize; x++ )
  {
    //need to recast here because the minimal engine version of this class has no destructor - hence no virtual destructor call
    CphxMaterialPassConstantState_Tool *t = (CphxMaterialPassConstantState_Tool*)o->MaterialState[ x ];
    SAFEDELETE( t );
    o->MaterialState[ x ] = NULL;
  }

  SAFEDELETEA( o->MaterialState );
  MaterialStateSize = 0;
}

void CphxObject_ParticleEmitter_CPU_Tool::AllocateMaterialState()
{
  FreeMaterialState();
  CphxObject_ParticleEmitter_CPU *o = (CphxObject_ParticleEmitter_CPU*)obj;
  if ( !Material ) return;

  MaterialStateSize = 0;
  o->MaterialState = NULL;

  for ( TS32 x = 0; x < o->Material->TechCount; x++ )
    MaterialStateSize += o->Material->Techniques[ x ]->PassCount;

  if ( MaterialStateSize )
  {
    o->MaterialState = new CphxMaterialPassConstantState*[ MaterialStateSize ];
    for ( TS32 x = 0; x < MaterialStateSize; x++ )
      o->MaterialState[ x ] = new CphxMaterialPassConstantState_Tool();
  }
}

void CphxObject_ParticleEmitter_CPU_Tool::SimulateParticles( TF32 ElapsedTime )
{
  if ( obj->VertexBuffer )
    obj->UpdateParticles( ElapsedTime );
}

void CphxObject_ParticleEmitter_CPU_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_Scale_x );
  Clip->CreateSpline( Spline_Scale_y );
  Clip->CreateSpline( Spline_Scale_z );
  Clip->CreateSpline( Spline_Rotation );

  Clip->CreateSpline( Spline_Particle_EmissionPerSecond );
  Clip->CreateSpline( Spline_Particle_Life );
  Clip->CreateSpline( Spline_Particle_LifeChaos );
  Clip->CreateSpline( Spline_Particle_EmissionVelocity );
  Clip->CreateSpline( Spline_Particle_EmissionVelocityChaos );
  Clip->CreateSpline( Spline_Particle_EmissionRotation );
  Clip->CreateSpline( Spline_Particle_EmissionRotationChaos );

  Clip->CreateSpline( Spline_Particle_Scale );
  Clip->CreateSpline( Spline_Particle_ScaleChaos );
  Clip->CreateSpline( Spline_Particle_Stretch_X );
  Clip->CreateSpline( Spline_Particle_Stretch_Y );

  Clip->BuildMaterialSplines( this );
}

void CphxObject_ParticleEmitter_CPU_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );

  Node->AddChild( _T( "shape" ) ).SetInt( obj->EmitterType );
  Node->AddChild( _T( "buffersize" ) ).SetInt( obj->BufferSize );
  Node->AddChild( _T( "innerradius" ) ).SetInt( obj->InnerRadius );
  Node->AddChild( _T( "startcount" ) ).SetInt( obj->StartCount );
  Node->AddChild( _T( "aging" ) ).SetInt( obj->Aging );
  Node->AddChild( _T( "randrotate" ) ).SetInt( obj->RandRotate );
  Node->AddChild( _T( "twodirrot" ) ).SetInt( obj->TwoDirRotate );
  Node->AddChild( _T( "rotatetodir" ) ).SetInt( obj->RotateToDirection );
  Node->AddChild( _T( "randseed" ) ).SetInt( obj->RandSeed );
  Node->AddChild( _T( "sort" ) ).SetInt( obj->Sort );

  if ( Material )
  {
    CXMLNode m = Node->AddChild( _T( "Material" ), false );
    m.SetAttribute( _T( "GUID" ), Material->GetGUID().GetString() );
    MaterialData.ExportData( Node );
  }

  if ( EmitedObject )
    Node->AddChild( _T( "EmitedObject" ) ).SetText( EmitedObject->GetGUID().GetString() );

  if ( EmitedScene )
    Node->AddChild( _T( "EmitedScene" ) ).SetText( EmitedScene->GetGUID().GetString() );
}

void CphxObject_ParticleEmitter_CPU_Tool::ImportData( CXMLNode *Node )
{
  CphxObject_Tool::ImportData( Node );

  if ( Node->GetChildCount( _T( "shape" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "shape" ) ).GetValue( i ) ) obj->EmitterType = i; }
  if ( Node->GetChildCount( _T( "buffersize" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "buffersize" ) ).GetValue( i ) ) obj->BufferSize = i; }
  if ( Node->GetChildCount( _T( "innerradius" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "innerradius" ) ).GetValue( i ) ) obj->InnerRadius = i; }
  if ( Node->GetChildCount( _T( "startcount" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "startcount" ) ).GetValue( i ) ) obj->StartCount = i; }
  if ( Node->GetChildCount( _T( "aging" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "aging" ) ).GetValue( i ) ) obj->Aging = i != 0; }
  if ( Node->GetChildCount( _T( "randrotate" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "randrotate" ) ).GetValue( i ) ) obj->RandRotate = i != 0; }
  if ( Node->GetChildCount( _T( "twodirrot" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "twodirrot" ) ).GetValue( i ) ) obj->TwoDirRotate = i != 0; }
  if ( Node->GetChildCount( _T( "rotatetodir" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "rotatetodir" ) ).GetValue( i ) ) obj->RotateToDirection = i != 0; }
  if ( Node->GetChildCount( _T( "randseed" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "randseed" ) ).GetValue( i ) ) obj->RandSeed = i; }
  if ( Node->GetChildCount( _T( "sort" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "sort" ) ).GetValue( i ) ) obj->Sort = i != 0; }

  //if ( Node->GetChildCount( _T( "EmitedObject" ) ) )
  //{
  //  CXMLNode m = Node->GetChild( _T( "EmitedObject" ) );
  //  if ( m.HasAttribute( _T( "GUID" ) ) )
  //  {
  //    CphxGUID g;
  //    g.SetString( m.GetAttributeAsString( _T( "GUID" ) ).GetPointer() );
  //    SetEmittedModel( Project.GetModel( g ) );
  //  }
  //}

  if ( Node->GetChildCount( _T( "Material" ) ) )
  {
    CXMLNode m = Node->GetChild( _T( "Material" ) );
    if ( m.HasAttribute( _T( "GUID" ) ) )
    {
      CphxGUID g;
      g.SetString( m.GetAttributeAsString( _T( "GUID" ) ).GetPointer() );
      CphxMaterial_Tool *mat = Project.GetMaterial( g );
      SetMaterial( mat );
    }
    MaterialData.ImportData( Node );
    UpdateMaterialState();
  }

}

void CphxObject_ParticleEmitter_CPU_Tool::UpdateParticleResources( CCoreDevice *Dev )
{
  //reallocate particle buffer
  SAFEDELETEA( obj->Particles );
  SAFEDELETEA( obj->DistanceBuffer );
  SAFEDELETEA( backupBuffer );
  SAFEDELETEA( tempBuffer );
  SAFEDELETE( VertexBuffer );
  obj->VertexBuffer = NULL;
  obj->LiveCount = 0;

  obj->Particles = new PHXPARTICLE[ 1 << obj->BufferSize ];
  memset( obj->Particles, 0, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
  obj->DistanceBuffer = new PHXPARTICLEDISTANCE[ 1 << obj->BufferSize ];
  memset( obj->DistanceBuffer, 0, sizeof( PHXPARTICLEDISTANCE )*( 1 << obj->BufferSize ) );
  backupBuffer = new PHXPARTICLE[ 1 << obj->BufferSize ];
  memset( backupBuffer, 0, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
  tempBuffer = new PHXPARTICLE[ 1 << obj->BufferSize ];
  memset( tempBuffer, 0, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );

  //reallocate vertexbuffer
  VertexBuffer = Dev->CreateVertexBufferDynamic( PARTICLESIZE*( 1 << obj->BufferSize ) );
  obj->VertexBuffer = (ID3D11Buffer *)VertexBuffer->GetHandle();

#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  obj->UpdateSplineTexture();
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif

  obj->CalculateAnimation( 0, 0 );
  ResetParticles();
}

void CphxObject_ParticleEmitter_CPU_Tool::ResetParticles()
{
  //if ( ContentReady() )
  if ( ContentReady() && obj && obj->Particles && backupBuffer && tempBuffer )
  {
    obj->CalculateAnimation( 0, 0 );
    obj->ResetParticles();
    SwapParticleBuffers();
    obj->CalculateAnimation( 0, 0 );
    obj->ResetParticles();
    SwapParticleBuffers();
  }
}

void CphxObject_ParticleEmitter_CPU_Tool::SwapParticleBuffers()
{
  if ( ContentReady() && obj && obj->Particles && backupBuffer && tempBuffer )
  {
    int lct = backupLiveCount;
    backupLiveCount = obj->LiveCount;
    obj->LiveCount = lct;

    float f = backupEmissionFraction;
    backupEmissionFraction = obj->EmissionFraction;
    obj->EmissionFraction = f;

    f = backupTicks;
    backupTicks = obj->Ticks;
    obj->Ticks = f;

    memcpy( tempBuffer, backupBuffer, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
    memcpy( backupBuffer, obj->Particles, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
    memcpy( obj->Particles, tempBuffer, sizeof( PHXPARTICLE )*( 1 << obj->BufferSize ) );
  }
}

void CphxObject_ParticleEmitter_CPU_Tool::MarkSpecialRequired()
{
  if ( EmitedScene )
    EmitedScene->GetClipByIndex( 0 )->MarkAsRequired();
}

void CphxObject_ParticleGravity_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );

  Node->AddChild( _T( "shape" ) ).SetInt( affector->AreaType );
  Node->AddChild( _T( "directional" ) ).SetInt( affector->Directional );
}

void CphxObject_ParticleGravity_Tool::ImportData( CXMLNode *Node )
{
  CphxObject_Tool::ImportData( Node );

  if ( Node->GetChildCount( _T( "shape" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "shape" ) ).GetValue( i ) ) affector->AreaType = i; }
  if ( Node->GetChildCount( _T( "directional" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "directional" ) ).GetValue( i ) ) affector->Directional = i != 0; }
}

CphxObject_ParticleGravity_Tool::CphxObject_ParticleGravity_Tool()
{
  affector = new CphxObject_ParticleGravity;
  affector->ObjectType = Object_ParticleGravity;
  affector->AreaType = 0;
  affector->Directional = true;
  SetObject( affector );
}

CphxObject_ParticleGravity_Tool::~CphxObject_ParticleGravity_Tool()
{
}

void CphxObject_ParticleGravity_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_AffectorPower );
}

void CphxObject_ParticleDrag_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );

  Node->AddChild( _T( "shape" ) ).SetInt( affector->AreaType );
}

void CphxObject_ParticleDrag_Tool::ImportData( CXMLNode *Node )
{
  CphxObject_Tool::ImportData( Node );

  if ( Node->GetChildCount( _T( "shape" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "shape" ) ).GetValue( i ) ) affector->AreaType = i; }
}

CphxObject_ParticleDrag_Tool::CphxObject_ParticleDrag_Tool()
{
  affector = new CphxObject_ParticleDrag;
  affector->ObjectType = Object_ParticleDrag;
  affector->AreaType = 0;
  SetObject( affector );
}

CphxObject_ParticleDrag_Tool::~CphxObject_ParticleDrag_Tool()
{
}

void CphxObject_ParticleDrag_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_AffectorPower );
}

void CphxObject_ParticleTurbulence_Tool::ExportData( CXMLNode *Node )
{
  CphxObject_Tool::ExportData( Node );

  Node->AddChild( _T( "shape" ) ).SetInt( affector->AreaType );
/*
  Node->AddChild( _T( "randseed" ) ).SetInt( affector->RandSeed );
  Node->AddChild( _T( "frequency" ) ).SetInt( affector->TurbulenceFrequency );
*/
}

void CphxObject_ParticleTurbulence_Tool::ImportData( CXMLNode *Node )
{
  CphxObject_Tool::ImportData( Node );

  if ( Node->GetChildCount( _T( "shape" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "shape" ) ).GetValue( i ) ) affector->AreaType = i; }
  if ( Node->GetChildCount( _T( "randseed" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "randseed" ) ).GetValue( i ) ) affector->RandSeed = i; }
  if ( Node->GetChildCount( _T( "frequency" ) ) )
  {
    TS32 i = 0;
    if ( Node->GetChild( _T( "frequency" ) ).GetValue( i ) )
    {
      TU16 v = i;
      affector->TurbulenceFrequency = *( (D3DXFLOAT16*)&v );
    }
  }

  affector->InitKernel();
}

CphxObject_ParticleTurbulence_Tool::CphxObject_ParticleTurbulence_Tool()
{
  affector = new CphxObject_ParticleTurbulence;
  affector->ObjectType = Object_ParticleTurbulence;
  affector->AreaType = 1;
  affector->RandSeed = 0;
  affector->InitKernel();
  SetObject( affector );
}

CphxObject_ParticleTurbulence_Tool::~CphxObject_ParticleTurbulence_Tool()
{
}

void CphxObject_ParticleTurbulence_Tool::CreateSplines( CphxObjectClip_Tool *Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_AffectorPower );
}

void CphxObject_ParticleVortex_Tool::ExportData( CXMLNode* Node )
{
  CphxObject_Tool::ExportData( Node );

  Node->AddChild( _T( "shape" ) ).SetInt( affector->AreaType );
}

void CphxObject_ParticleVortex_Tool::ImportData( CXMLNode* Node )
{
  CphxObject_Tool::ImportData( Node );

  if ( Node->GetChildCount( _T( "shape" ) ) ) { TS32 i = 0; if ( Node->GetChild( _T( "shape" ) ).GetValue( i ) ) affector->AreaType = i; }
}

CphxObject_ParticleVortex_Tool::CphxObject_ParticleVortex_Tool()
{
  affector = new CphxObject_ParticleVortex;
  affector->ObjectType = Object_ParticleVortex;
  affector->AreaType = 0;
  SetObject( affector );
}

CphxObject_ParticleVortex_Tool::~CphxObject_ParticleVortex_Tool()
{
}

void CphxObject_ParticleVortex_Tool::CreateSplines( CphxObjectClip_Tool* Clip )
{
  CphxObject_Tool::CreateSplines( Clip );
  Clip->CreateSpline( Spline_AffectorPower );
}

