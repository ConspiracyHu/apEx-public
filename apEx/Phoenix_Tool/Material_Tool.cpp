#define _WINSOCKAPI_
#include "BasePCH.h"
#include "../Phoenix/phxEngine.h"
#include "../Phoenix/phxSpline.h"
#include "Material_Tool.h"
#include "phxSplineExt.h"
#include "apxProject.h"
#include "RenderLayer.h"

CphxMaterial_Tool *DefaultMaterial = NULL;
CphxMaterialTechnique_Tool *DefaultTech = NULL;

CphxMaterialRenderPass_Tool::CphxMaterialRenderPass_Tool()
{
  Name = _T( "New Pass" );
  Pass = new CphxMaterialRenderPass;
  //Pass->Wireframe = false;
  Pass->VS = NULL;
  Pass->PS = NULL;
  Pass->GS = NULL;
  Pass->HS = NULL;
  Pass->DS = NULL;
  //Pass->FinalBlendState = NULL;
  //Pass->FinalDepthStencilState = NULL;
  //Pass->FinalRasterizerState = NULL;
  Pass->Parameters.ParamCount = 0;
  Pass->Parameters.Parameters = NULL;
  VS = NULL;
  PS = NULL;
  GS = NULL;
  HS = NULL;
  DS = NULL;
}

CphxMaterialRenderPass_Tool::~CphxMaterialRenderPass_Tool()
{
  SAFEDELETE( VS );
  SAFEDELETE( PS );
  SAFEDELETE( GS );
  SAFEDELETE( HS );
  SAFEDELETE( DS );
  SAFEDELETE( Pass->Parameters.Parameters );
  SAFEDELETE( Pass );
}

void CphxMaterialRenderPass_Tool::CompileShaders( CCoreDevice *Dev, CString &Error )
{
  if ( ShaderCode == LastCompiledCode )
  {
    Error = LastCompileError;
    return;
  }

  Pass->VS = NULL;
  Pass->PS = NULL;
  Pass->GS = NULL;
  Pass->HS = NULL;
  Pass->DS = NULL;

  SAFEDELETE( VS );
  SAFEDELETE( PS );
  SAFEDELETE( GS );
  SAFEDELETE( HS );
  SAFEDELETE( DS );

  Error = "";
  CString err = "";

  VS = Dev->CreateVertexShader( ShaderCode.GetPointer(), ShaderCode.Length(), "v", "vs_5_0", &err );
  if ( err.Find( _T( "error X3501" ) ) < 0 )
  {
    Error += _T( "Compiling Vertex Shader: " );
    Error += err.Length() ? _T( "\n" ) + err : _T( "Shader ('v') compiled successfully.\n" );
  }

  err = "";
  PS = Dev->CreatePixelShader( ShaderCode.GetPointer(), ShaderCode.Length(), "p", "ps_5_0", &err );
  Error += _T( "Compiling Pixel Shader: " );
  Error += err.Length() ? _T( "\n" ) + err : _T( "Shader ('p') compiled successfully.\n" );

  err = "";
  GS = Dev->CreateGeometryShader( ShaderCode.GetPointer(), ShaderCode.Length(), "g", "gs_5_0", &err );
  if ( err.Find( _T( "error X3501" ) ) < 0 )
  {
    Error += _T( "Compiling Geometry Shader: " );
    Error += err.Length() ? _T( "\n" ) + err : _T( "Shader ('g') compiled successfully.\n" );
  }

  err = "";
  HS = Dev->CreateHullShader( ShaderCode.GetPointer(), ShaderCode.Length(), "h", "hs_5_0", &err );
  if ( err.Find( _T( "error X3501" ) ) < 0 )
  {
    Error += _T( "Compiling Hull Shader: " );
    Error += err.Length() ? _T( "\n" ) + err : _T( "Shader ('h') compiled successfully.\n" );
  }

  err = "";
  DS = Dev->CreateDomainShader( ShaderCode.GetPointer(), ShaderCode.Length(), "d", "ds_5_0", &err );
  if ( err.Find( _T( "error X3501" ) ) < 0 )
  {
    Error += _T( "Compiling Domain Shader: " );
    Error += err.Length() ? _T( "\n" ) + err : _T( "Shader ('d') compiled successfully.\n" );
  }

  if ( VS ) Pass->VS = (ID3D11VertexShader*)VS->GetHandle();
  if ( PS ) Pass->PS = (ID3D11PixelShader*)PS->GetHandle();
  if ( GS ) Pass->GS = (ID3D11GeometryShader*)GS->GetHandle();
  if ( DS ) Pass->DS = (ID3D11DomainShader*)DS->GetHandle();
  if ( HS ) Pass->HS = (ID3D11HullShader*)HS->GetHandle();

  LastCompiledCode = ShaderCode;
  LastCompileError = Error;
}

void CphxMaterialRenderPass_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "Code" ), false ).SetText( ShaderCode.GetPointer() );
  Node->AddChild( _T( "Minifiable" ), false ).SetInt( Minifiable );
  PassParameters.Export( Node );
}

void CphxMaterialRenderPass_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) )
    Name = Node->GetChild( _T( "Name" ) ).GetText();
  if ( Node->GetChildCount( _T( "Code" ) ) )
    ShaderCode = Node->GetChild( _T( "Code" ) ).GetText();

  if ( Node->GetChildCount( _T( "Minifiable" ) ) )
  {
    TBOOL i = 0;
    if ( Node->GetChild( _T( "Minifiable" ) ).GetValue( i ) )
      Minifiable = i;
  }

  PassParameters.Import( Node );
}

CphxMaterialParameter_Tool * CphxMaterialRenderPass_Tool::CreateParam()
{
  //InvalidateUptoDateFlag();
  CphxMaterialParameter_Tool *p = PassParameters.CreateParam();
  Rebuild();
  return p;
}

void CphxMaterialRenderPass_Tool::DeleteParam( TS32 p )
{
  if ( p < 0 || p >= PassParameters.Parameters.NumItems() ) return;
  //InvalidateUptoDateFlag();
  PassParameters.Parameters.FreeByIndex( p );
  Rebuild();
}

void CphxMaterialRenderPass_Tool::Rebuild()
{
  PassParameters.Rebuild( &Pass->Parameters );
}

CphxMaterialTechnique_Tool::CphxMaterialTechnique_Tool()
{
  Tech = new CphxMaterialTechnique;
  Tech->Type = TECH_MATERIAL;
  Tech->RenderPasses = NULL;
  Tech->PassCount = 0;
  Tech->Parameters.ParamCount = 0;
  Tech->Parameters.Parameters = NULL;
  Tech->TargetLayer = NULL;
  TargetLayer.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  External = false;

  Name = _T( "New Technique" );
}

CphxMaterialTechnique_Tool::~CphxMaterialTechnique_Tool()
{
  //this is copypasted to the material update/import code as well!
  Passes.FreeArray();
  SAFEDELETE( Tech->Parameters.Parameters );
  SAFEDELETEA( Tech->RenderPasses );
  SAFEDELETE( Tech );
}

CphxMaterialRenderPass_Tool * CphxMaterialTechnique_Tool::CreatePass()
{
  CphxMaterialRenderPass_Tool *p = new CphxMaterialRenderPass_Tool();
  Passes += p;

  CphxMaterialRenderPass **old = Tech->RenderPasses;
  Tech->RenderPasses = new CphxMaterialRenderPass*[ Tech->PassCount + 1 ];
  for ( TS32 x = 0; x < Tech->PassCount; x++ )
    Tech->RenderPasses[ x ] = old[ x ];
  Tech->RenderPasses[ Tech->PassCount ] = p->Pass;
  Tech->PassCount++;
  SAFEDELETEA( old );

  return p;

}

CphxMaterialParameter_Tool * CphxMaterialTechnique_Tool::CreateParam()
{
  InvalidateUptoDateFlag();
  CphxMaterialParameter_Tool *p = TechParameters.CreateParam();
  Rebuild();
  return p;
}

void CphxMaterialTechnique_Tool::DeleteParam( TS32 p )
{
  if ( p < 0 || p >= TechParameters.Parameters.NumItems() ) return;
  InvalidateUptoDateFlag();
  TechParameters.Parameters.FreeByIndex( p );
  Rebuild();
}


void CphxMaterialTechnique_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "Type" ), false ).SetInt( Tech->Type );
  Node->AddChild( _T( "TargetLayer" ) ).SetText( TargetLayer.GetString() );
  TechParameters.Export( Node );
  for ( TS32 x = 0; x < Passes.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Pass" ) );
    Passes[ x ]->ExportData( &n );
  }
}

void CphxMaterialTechnique_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  if ( Node->GetChildCount( _T( "Type" ) ) ) Node->GetChild( _T( "Type" ) ).GetValue( (TS32&)Tech->Type );
  if ( Node->GetChildCount( _T( "TargetLayer" ) ) ) TargetLayer.SetString( Node->GetChild( _T( "TargetLayer" ) ).GetText().GetPointer() );

  TechParameters.Import( Node );
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Pass" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Pass" ), x );
    CphxMaterialRenderPass_Tool *p = CreatePass();
    p->ImportData( &n );
  }

  if ( Project.GetRenderLayer( TargetLayer ) )
    SetTargetLayer( Project.GetRenderLayer( TargetLayer ) );
  else
    LOG_ERR( "Error while importing material techinque %s: target layer not found in project!", Name.GetPointer() );

}

TBOOL CphxMaterialTechnique_Tool::GenerateResource( CCoreDevice *Dev )
{
  TechParameters.Rebuild( &Tech->Parameters );
  for ( TS32 x = 0; x < Passes.NumItems(); x++ )
    Passes[ x ]->PassParameters.Rebuild( &Passes[ x ]->Pass->Parameters );

  CString Err;
  for ( TS32 x = 0; x < Passes.NumItems(); x++ )
    Passes[ x ]->CompileShaders( Dev, Err );
  return true;
}

void CphxMaterialTechnique_Tool::Rebuild()
{
  TechParameters.Rebuild( &Tech->Parameters );
}

void CphxMaterialTechnique_Tool::SetTargetLayer( CphxRenderLayerDescriptor_Tool *t )
{
  CphxRenderLayerDescriptor_Tool *o = Project.GetRenderLayer( TargetLayer );
  if ( o ) RemoveParent( o );

  if ( !t )
  {
    TargetLayer.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );
    Tech->TargetLayer = NULL;
    return;
  }

  AddParent( t );
  TargetLayer = t->GetGUID();
  Tech->TargetLayer = &t->RenderLayer;
}

void CphxMaterialTechnique_Tool::CreateInstancedData( CphxMaterialPassConstantState **MaterialState, TS32 &passcnt )
{
  CphxRenderLayerDescriptor_Tool *o = Project.GetRenderLayer( TargetLayer );

  for ( TS32 x = 0; x < Passes.NumItems(); x++ )
  {
    CphxParameterDataCollector DataCollector;
    if ( o && o->RenderLayer.VoxelizerLayer )
      DataCollector.RasterDesc.DepthClipEnable = false;

    TechParameters.CollectData( DataCollector );
    Passes[ x ]->PassParameters.CollectData( DataCollector );

    DataCollector.Apply( MaterialState[ passcnt ] );

    passcnt++;
  }
}

void CphxMaterialTechnique_Tool::CreateInstancedData_Textures( CphxMaterialPassConstantState **MaterialState, TS32 &passcnt )
{
  CphxRenderLayerDescriptor_Tool *o = Project.GetRenderLayer( TargetLayer );

  for ( TS32 x = 0; x < Passes.NumItems(); x++ )
  {
    CphxParameterDataCollector DataCollector;
    if ( o && o->RenderLayer.VoxelizerLayer )
      DataCollector.RasterDesc.DepthClipEnable = false;

    TechParameters.CollectData( DataCollector );
    Passes[ x ]->PassParameters.CollectData( DataCollector );

    DataCollector.ApplyTextures( MaterialState[ passcnt ] );

    passcnt++;
  }
}

CphxMaterialParameter_Tool * CphxMaterialTechnique_Tool::GetParameter( CphxGUID &guid )
{
  for ( TS32 y = 0; y < TechParameters.Parameters.NumItems(); y++ )
    if ( TechParameters.Parameters[ y ]->GetGUID() == guid ) return TechParameters.Parameters[ y ];

  for ( TS32 y = 0; y < Passes.NumItems(); y++ )
  {
    CphxMaterialRenderPass_Tool *p = Passes[ y ];
    for ( TS32 z = 0; z < p->PassParameters.Parameters.NumItems(); z++ )
      if ( p->PassParameters.Parameters[ z ]->GetGUID() == guid ) return p->PassParameters.Parameters[ z ];
  }

  return NULL;
}

void CphxMaterialTechnique_Tool::RebuildDependents()
{
  CArray<CphxMaterial_Tool*> MaterialsToUpdate;
  InvalidateUptoDateFlag();

  for ( TS32 x = GetChildCount() - 1; x >= 0; x-- ) //we'll be removing children so it's better to go backwards
  {
    switch ( GetChild( x )->GetType() )
    {
    case PHX_MATERIAL:
    {
      CphxMaterial_Tool *m = (CphxMaterial_Tool*)GetChild( x );
      for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
        if ( m->Techniques[ y ]->GetGUID() == GetGUID() )
        {
          m->ReplaceTech( this, this );
          m->InvalidateUptoDateFlag();
        }
      MaterialsToUpdate.AddUnique( m );
      break;
    }
    case PHX_EVENT:
    {
      CphxEvent_Tool *e = (CphxEvent_Tool *)GetChild( x );
      switch ( e->GetEventType() )
      {
      case EVENT_SHADERTOY:
      {
        auto *st = (CphxEvent_Shadertoy_Tool *)e;
        st->SetTech( NULL, true );
        st->SetTech( this, true );
      }
      break;
      default:
        LOG_ERR( "[apex] Unhandled Technique Event Target (%d) During Update!", e->GetEventType() );
        break;
      }
      int z = 0;
    }
    break;
    default:
      LOG_ERR( "[apex] Unhandled Technique Target (%d) During Update!", GetChild( x )->GetType() );

    }
  }

  for ( TS32 x = 0; x < MaterialsToUpdate.NumItems(); x++ )
    MaterialsToUpdate[ x ]->RebuildDependents();
}


CphxMaterialParameter_Tool::CphxMaterialParameter_Tool()
{
  Parameter.Scope = PARAM_CONSTANT;
  Parameter.Type = PARAM_FLOAT;
  Parameter.Value.Float = 0;
  Parameter.Value.Color[ 0 ] = 0;
  Parameter.Value.Color[ 1 ] = 0;
  Parameter.Value.Color[ 2 ] = 0;
  Parameter.Value.Color[ 3 ] = 0;
  DefaultValue = Parameter.Value;
}

CphxMaterialParameter_Tool::~CphxMaterialParameter_Tool()
{
}

bool MaterialParamHasNonGUIDData( MATERIALPARAMTYPE p )
{
  switch ( p )
  {
  case PARAM_FLOAT:
  case PARAM_COLOR:
  case PARAM_ZMODE:
  case PARAM_ZFUNCTION:
  case PARAM_FILLMODE:
  case PARAM_CULLMODE:
  case PARAM_RENDERPRIORITY:
  case PARAM_BLENDMODE0:
  case PARAM_BLENDMODE1:
  case PARAM_BLENDMODE2:
  case PARAM_BLENDMODE3:
  case PARAM_BLENDMODE4:
  case PARAM_BLENDMODE5:
  case PARAM_BLENDMODE6:
  case PARAM_BLENDMODE7:
  case PARAM_PARTICLELIFEFLOAT:
  case PARAM_PARTICLELIFE:
    return true;
  case PARAM_TEXTURE0:
  case PARAM_TEXTURE1:
  case PARAM_TEXTURE2:
  case PARAM_TEXTURE3:
  case PARAM_TEXTURE4:
  case PARAM_TEXTURE5:
  case PARAM_TEXTURE6:
  case PARAM_TEXTURE7:
  case PARAM_RENDERTARGET:
  case PARAM_DEPTHTEXTURE7:
  case PARAM_3DTEXTURE6:
  case PARAM_MESHDATA0:
  case PARAM_MESHDATA1:
  case PARAM_MESHDATA2:
  case PARAM_MESHDATA3:
  case PARAM_MESHDATA4:
  case PARAM_MESHDATA5:
  case PARAM_MESHDATA6:
  case PARAM_MESHDATA7:
  case PARAM_LTC1:
  case PARAM_LTC2:
    return false;
  default:
    return true;
  }
}

void CphxMaterialParameter_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ) ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "Scope" ) ).SetInt( Parameter.Scope );
  Node->AddChild( _T( "Type" ) ).SetInt( Parameter.Type );

  if ( MaterialParamHasNonGUIDData( Parameter.Type ) )
  {
    CString s = CString::EncodeToBase64( (TU8*)&Parameter.Value, sizeof( Parameter.Value ) );
    if ( Parameter.Scope == PARAM_CONSTANT )
      Node->AddChild( _T( "Value" ) ).SetText( s.GetPointer() );

    s = CString::EncodeToBase64( (TU8*)&DefaultValue, sizeof( DefaultValue ) );
    Node->AddChild( _T( "DefaultValue" ) ).SetText( s.GetPointer() );
  }

  if ( ( ( Parameter.Type >= PARAM_TEXTURE0 && Parameter.Type <= PARAM_TEXTURE7 ) || Parameter.Type == PARAM_RENDERTARGET ) && Parameter.Scope == PARAM_CONSTANT )
    Node->AddChild( _T( "TextureGUID" ) ).SetText( TextureGUID.GetString() );
}

void CphxMaterialParameter_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  if ( Node->GetChildCount( _T( "Scope" ) ) ) Node->GetChild( _T( "Scope" ) ).GetValue( (TS32&)Parameter.Scope );
  if ( Node->GetChildCount( _T( "Type" ) ) ) Node->GetChild( _T( "Type" ) ).GetValue( (TS32&)Parameter.Type );

  memset( &Parameter.Value, 0, sizeof( MATERIALVALUE ) );
  if ( Node->GetChildCount( _T( "Value" ) ) )
  {
    CString t = Node->GetChild( _T( "Value" ) ).GetText();
    TU8 *Data;
    TS32 DataSize = 0;

    t.DecodeBase64( Data, DataSize );

    if ( DataSize == sizeof( MATERIALVALUE ) )
      memcpy( &Parameter.Value, Data, DataSize );

    SAFEDELETEA( Data );
  }

  if ( Node->GetChildCount( _T( "DefaultValue" ) ) )
  {
    CString t = Node->GetChild( _T( "DefaultValue" ) ).GetText();
    TU8 *Data;
    TS32 DataSize = 0;

    t.DecodeBase64( Data, DataSize );

    if ( DataSize == sizeof( MATERIALVALUE ) )
      memcpy( &DefaultValue, Data, DataSize );

    if ( Parameter.Scope == PARAM_ANIMATED && DataSize == sizeof( MATERIALVALUE ) )
      memcpy( &Parameter.Value, Data, DataSize );

    SAFEDELETEA( Data );
  }

  if ( Node->GetChildCount( _T( "TextureGUID" ) ) )
  {
    if ( Node->GetChild( _T( "TextureGUID" ) ).GetText().Length() == 32 )
      TextureGUID.SetString( Node->GetChild( _T( "TextureGUID" ) ).GetText().GetPointer() );
    else
      TextureGUID.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

    SetTextureGUID( TextureGUID );
  }
}

void CphxMaterialParameter_Tool::SetTextureGUID( CphxGUID &g )
{
  CphxResource *p = Project.GetResource( TextureGUID );
  RemoveParent( p );

  TextureGUID = g;

  p = Project.GetResource( TextureGUID );
  if ( p ) AddParent( p );

  Parameter.Value.RenderTarget = NULL;
  DefaultValue.RenderTarget = NULL;

  if ( Project.GetRenderTarget( TextureGUID ) )
    DefaultValue.RenderTarget = Parameter.Value.RenderTarget = &Project.GetRenderTarget( TextureGUID )->rt;

  InvalidateUptoDateFlag();
}

bool CphxMaterialParameter_Tool::IsDefault()
{
  switch ( Parameter.Type )
  {
      case PARAM_FLOAT:
        return Parameter.Value.Float == DefaultValue.Float;
      case PARAM_COLOR:
        return Parameter.Value.Color[ 0 ] == DefaultValue.Color[ 0 ] &&
          Parameter.Value.Color[ 1 ] == DefaultValue.Color[ 1 ] &&
          Parameter.Value.Color[ 2 ] == DefaultValue.Color[ 2 ] &&
          Parameter.Value.Color[ 3 ] == DefaultValue.Color[ 3 ];
      case PARAM_ZMODE:
        return Parameter.Value.ZMode == DefaultValue.ZMode;
      case PARAM_ZFUNCTION:
        return Parameter.Value.ZFunction == DefaultValue.ZFunction;
      case PARAM_FILLMODE:
        return Parameter.Value.Wireframe == DefaultValue.Wireframe;
      case PARAM_CULLMODE:
        return Parameter.Value.CullMode == DefaultValue.CullMode;
      case PARAM_RENDERPRIORITY:
        return Parameter.Value.RenderPriority == DefaultValue.RenderPriority;
      case PARAM_TEXTURE0:
      case PARAM_TEXTURE1:
      case PARAM_TEXTURE2:
      case PARAM_TEXTURE3:
      case PARAM_TEXTURE4:
      case PARAM_TEXTURE5:
      case PARAM_TEXTURE6:
      case PARAM_TEXTURE7:
        return false;
      case PARAM_BLENDMODE0:
      case PARAM_BLENDMODE1:
      case PARAM_BLENDMODE2:
      case PARAM_BLENDMODE3:
      case PARAM_BLENDMODE4:
      case PARAM_BLENDMODE5:
      case PARAM_BLENDMODE6:
      case PARAM_BLENDMODE7:
        return Parameter.Value.BlendMode == DefaultValue.BlendMode;
      case PARAM_RENDERTARGET: //only for postproc
        return false;
        return Parameter.Value.RenderTarget == DefaultValue.RenderTarget;
      case PARAM_PARTICLELIFEFLOAT:
        return false;
        //return Parameter.Value. == DefaultValue.RenderTarget;
      case PARAM_DEPTHTEXTURE7:
      case PARAM_3DTEXTURE6:
      case PARAM_MESHDATA0:
      case PARAM_MESHDATA1:
      case PARAM_MESHDATA2:
      case PARAM_MESHDATA3:
      case PARAM_MESHDATA4:
      case PARAM_MESHDATA5:
      case PARAM_MESHDATA6:
      case PARAM_MESHDATA7:
      case PARAM_PARTICLELIFE:
      case PARAM_LTC1:
      case PARAM_LTC2:
        return false;
      break;
  }
  return false;
}

CphxMaterialParameterBatch_Tool::CphxMaterialParameterBatch_Tool()
{
}

CphxMaterialParameterBatch_Tool::~CphxMaterialParameterBatch_Tool()
{
  Parameters.FreeArray();
}

CphxMaterialParameter_Tool * CphxMaterialParameterBatch_Tool::CreateParam()
{
  CphxMaterialParameter_Tool *p = new CphxMaterialParameter_Tool();
  p->Name = _T( "New Parameter" );
  Parameters += p;
  return p;
}

void CphxMaterialParameterBatch_Tool::Rebuild( CphxMaterialParameterBatch *Original )
{
  if ( !Original ) return;

  SAFEDELETE( Original->Parameters );
  Original->ParamCount = Parameters.NumItems();

  if ( !Parameters.NumItems() ) return;

  Original->Parameters = new CphxMaterialParameter *[ Original->ParamCount ];

  for ( TS32 x = 0; x < Original->ParamCount; x++ )
    Original->Parameters[ x ] = &Parameters[ x ]->Parameter;
}

void CphxMaterialParameterBatch_Tool::Export( CXMLNode *Node )
{
  for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Parameter" ) );
    Parameters[ x ]->Export( &n );
  }
}

void CphxMaterialParameterBatch_Tool::Import( CXMLNode *Node )
{
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Parameter" ) ); x++ )
  {
    CphxMaterialParameter_Tool *p = new CphxMaterialParameter_Tool();
    p->Import( &Node->GetChild( _T( "Parameter" ), x ) );
    Parameters += p;
  }
}

void CphxMaterialParameterBatch_Tool::CollectData( CphxParameterDataCollector &Collector )
{
  for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
  {
    switch ( Parameters[ x ]->Parameter.Type )
    {
    case PARAM_FLOAT:
      if ( Parameters[ x ]->Parameter.Scope == PARAM_ANIMATED )
        Collector.DynamicFloatData += Parameters[ x ]->Parameter.Value.Float;
      else
        Collector.StaticFloatData += Parameters[ x ]->Parameter.Value.Float;
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
      CphxResource *r = Project.GetResource( Parameters[ x ]->TextureGUID );
      if ( r ) r->RequestContent();

      if ( Parameters[ x ]->Parameter.Scope == PARAM_CONSTANT )
      {
        Collector.Textures[ Parameters[ x ]->Parameter.Type - PARAM_TEXTURE0 ] = Project.GetTextureView( Parameters[ x ]->TextureGUID );
      }

      if ( Parameters[ x ]->Parameter.Scope == PARAM_VARIABLE )
        Collector.Textures[ Parameters[ x ]->Parameter.Type - PARAM_TEXTURE0 ] = Parameters[ x ]->Parameter.Value.TextureView;
    }
    break;
    case PARAM_MESHDATA0:
    case PARAM_MESHDATA1:
    case PARAM_MESHDATA2:
    case PARAM_MESHDATA3:
    case PARAM_MESHDATA4:
    case PARAM_MESHDATA5:
    case PARAM_MESHDATA6:
    case PARAM_MESHDATA7:
      Collector.Textures[ Parameters[ x ]->Parameter.Type - PARAM_MESHDATA0 ] = phxMeshDataShaderView;
      break;
    case PARAM_DEPTHTEXTURE7:
      Collector.Textures[ 7 ] = phxDepthBufferShaderView;
      break;
    case PARAM_LTC1:
      Collector.Textures[ 4 ] = ltc1view;
      break;
    case PARAM_LTC2:
      Collector.Textures[ 5 ] = ltc2view;
      break;
#ifdef PHX_VOLUMETRIC_RENDERTARGETS
    case PARAM_3DTEXTURE6:
            Collector.Textures[ 6 ] = phxTexture3DResourceView;
            break;
#endif

    case PARAM_RENDERTARGET:
    {
      CphxResource *r = Project.GetResource( Parameters[ x ]->TextureGUID );
      if ( r ) r->RequestContent();

      if ( Parameters[ x ]->Parameter.Scope == PARAM_CONSTANT )
      {
        CphxRenderTarget_Tool *t = Project.GetRenderTarget( Parameters[ x ]->TextureGUID );
        if ( t )
          Collector.RenderTarget = &( t->rt );
        else
          LOG_ERR( "[material] Referenced rendertarget %s not found!", Parameters[ x ]->TextureGUID.GetString() );
      }

      if ( Parameters[ x ]->Parameter.Scope == PARAM_VARIABLE )
        Collector.RenderTarget = Parameters[ x ]->Parameter.Value.RenderTarget;

    }
    break;
    case PARAM_COLOR:
      for ( TS32 y = 0; y < 4; y++ )
        if ( Parameters[ x ]->Parameter.Scope == PARAM_ANIMATED )
          Collector.DynamicFloatData += Parameters[ x ]->Parameter.Value.Color[ y ];
        else
          Collector.StaticFloatData += Parameters[ x ]->Parameter.Value.Color[ y ];
      break;
    case PARAM_ZMODE:
      Collector.DepthStencilDesc.DepthEnable = !( Parameters[ x ]->Parameter.Value.ZMode & 1 ); //default zero means z enabled
      Collector.DepthStencilDesc.DepthWriteMask = !( ( Parameters[ x ]->Parameter.Value.ZMode >> 1 ) & 1 ) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO; //default zero means z write enabled
      break;
    case PARAM_ZFUNCTION:
      Collector.DepthStencilDesc.DepthFunc = Parameters[ x ]->Parameter.Value.ZFunction;
      break;
    case PARAM_BLENDMODE0:
    case PARAM_BLENDMODE1:
    case PARAM_BLENDMODE2:
    case PARAM_BLENDMODE3:
    case PARAM_BLENDMODE4:
    case PARAM_BLENDMODE5:
    case PARAM_BLENDMODE6:
    case PARAM_BLENDMODE7:
    {
      TS32 z = Parameters[ x ]->Parameter.Type - PARAM_BLENDMODE0;
      Collector.BlendDesc.RenderTarget[ z ].BlendEnable = true;
      Collector.BlendDesc.RenderTarget[ z ].SrcBlend = (D3D11_BLEND)( ( Parameters[ x ]->Parameter.Value.BlendMode + 0x11 ) & 15 );
      Collector.BlendDesc.RenderTarget[ z ].DestBlend = (D3D11_BLEND)( ( Parameters[ x ]->Parameter.Value.BlendMode + 0x11 ) >> 4 );
    }
    break;
    case PARAM_FILLMODE:
      Collector.Wireframe = Parameters[ x ]->Parameter.Value.Wireframe;
      break;
    case PARAM_CULLMODE:
      Collector.RasterDesc.CullMode = Parameters[ x ]->Parameter.Value.CullMode;
      break;
    case PARAM_RENDERPRIORITY:
      Collector.RenderPriority = Parameters[ x ]->Parameter.Value.RenderPriority;
      break;
    default:
      break;
    }
  }
}

CphxMaterialSplineBatch_Tool::CphxMaterialSplineBatch_Tool()
{
  Batch = new CphxMaterialSplineBatch;
  Batch->SplineCount = 0;
  Batch->Splines = NULL;
}

CphxMaterialSplineBatch_Tool::~CphxMaterialSplineBatch_Tool()
{
  SAFEDELETE( Batch );
  Splines.FreeArray();
  SplineBackup.FreeArray();
}

void CphxMaterialSplineBatch_Tool::BuildBatch( CphxModel_Tool *Model )
{
  Splines.FreeArray();
  Batch->Splines = NULL;
  MinimalSplineArray.FlushFast();

  if ( !Model ) return;

  for ( TS32 x = 0; x < Model->GetObjectCount(); x++ )
  {
    CphxModelObject_Tool *o = Model->GetObjectByIndex( x );
    if ( o->GetPrimitive() == Mesh_Clone ) continue;

    CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)o;
    if ( !m->GetMaterial() ) continue;

    CphxMaterial_Tool *mat = m->GetMaterial();

    for ( int y = 0; y < mat->Techniques.NumItems(); y++ )
      ImportTech( mat->Techniques[ y ], m->GetObject(), m->GetGUID() );
  }

  Batch->SplineCount = MinimalSplineArray.NumItems();
  Batch->Splines = MinimalSplineArray.GetPointer( 0 );

}

void CphxMaterialSplineBatch_Tool::BuildBatch( CphxObject_ParticleEmitter_CPU_Tool *Obj )
{
  Splines.FreeArray();
  Batch->Splines = NULL;
  MinimalSplineArray.FlushFast();

  if ( !Obj || !Obj->Material ) return;

  if ( !Obj->EmitedObject )
  {
    for ( int y = 0; y < Obj->Material->Techniques.NumItems(); y++ )
      ImportTech( Obj->Material->Techniques[ y ], Obj->GetObject(), Obj->GetGUID() );
  }
  else
  {
    for ( TS32 x = 0; x < Obj->EmitedObject->GetObjectCount(); x++ )
    {
      CphxModelObject_Tool *o = Obj->EmitedObject->GetObjectByIndex( x );
      if ( o->GetPrimitive() == Mesh_Clone ) continue;

      CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)o;
      if ( !m->GetMaterial() ) continue;

      CphxMaterial_Tool *mat = m->GetMaterial();

      for ( int y = 0; y < mat->Techniques.NumItems(); y++ )
        ImportTech( mat->Techniques[ y ], m->GetObject(), m->GetGUID() );
    }
  }

  Batch->SplineCount = MinimalSplineArray.NumItems();
  Batch->Splines = MinimalSplineArray.GetPointer( 0 );
}

#include "../Phoenix/Timeline.h"

void CphxMaterialSplineBatch_Tool::BuildBatch( CphxEvent_Shadertoy_Tool *st )
{
  Splines.FreeArray();
  Batch->Splines = NULL;
  MinimalSplineArray.FlushFast();

  if ( !st ) return;

  CphxMaterialTechnique_Tool *tech = st->GetTech();
  if ( !tech ) return;

  ImportTech( tech, st->Event, st->GetGUID() );

  Batch->SplineCount = MinimalSplineArray.NumItems();
  Batch->Splines = MinimalSplineArray.GetPointer( 0 );

  CphxEvent_Shadertoy *s = (CphxEvent_Shadertoy*)st->Event;
  s->MaterialSplines = Batch;

  //(CphxEvent_ShaderToy*)st->Event
}

void CphxMaterialSplineBatch_Tool::ImportTech( CphxMaterialTechnique_Tool *Tech, void *GroupingData, CphxGUID &GroupingGUID )
{
  if ( !Tech ) return;

  ImportParameters( &Tech->TechParameters, GroupingData, GroupingGUID );
  for ( TS32 x = 0; x < Tech->Passes.NumItems(); x++ )
    ImportParameters( &Tech->Passes[ x ]->PassParameters, GroupingData, GroupingGUID );
}

void CphxMaterialSplineBatch_Tool::ImportParameters( CphxMaterialParameterBatch_Tool *Batch, void *GroupingData, CphxGUID &GroupingGUID )
{
  for ( TS32 x = 0; x < Batch->Parameters.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool *p = Batch->Parameters[ x ];
    if ( p->Parameter.Scope != PARAM_ANIMATED ) continue;
    if ( p->Parameter.Type != PARAM_FLOAT && p->Parameter.Type != PARAM_COLOR && p->Parameter.Type != PARAM_PARTICLELIFEFLOAT ) continue;

    CphxMaterialSpline_Tool *s = NULL;

    //this loop is the "restore from spline backup" part
    for ( TS32 y = 0; y < SplineBackup.NumItems(); y++ )
    {
      if ( SplineBackup[ y ]->TargetParamGUID == p->GetGUID() && SplineBackup[ y ]->TargetObjectGUID == GroupingGUID && SplineBackup[ y ]->Spline.GroupingData == GroupingData )
      {
        s = SplineBackup[ y ];
        SplineBackup.DeleteByIndex( y );
        break;
      }
    }

    if ( !s )
    {
      s = new CphxMaterialSpline_Tool();
      for ( TS32 y = 0; y < 4; y++ )
      {
        s->Splines[ y ].Spline->Value[ 0 ] = p->DefaultValue.Color[ y ];
        s->Spline.Splines[ y ] = (CphxSpline_float16*)s->Splines[ y ].Spline;
      }
    }

    s->TargetParamGUID = p->GetGUID();
    s->TargetObjectGUID = GroupingGUID;
    s->Spline.GroupingData = GroupingData;
    s->Spline.Target = &p->Parameter;

    Splines += s;
    MinimalSplineArray.Add( &s->Spline );
  }
}

void CphxMaterialSplineBatch_Tool::BackupSplines()
{
  SplineBackup.FreeArray();

  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
  {
    CphxMaterialSpline_Tool *o = Splines[ x ];

    CphxMaterialSpline_Tool *s = new CphxMaterialSpline_Tool();
    s->TargetParamGUID = o->TargetParamGUID;
    s->TargetObjectGUID = o->TargetObjectGUID;
    s->Spline.GroupingData = o->Spline.GroupingData;
    s->Spline.Target = o->Spline.Target;
    for ( TS32 y = 0; y < 4; y++ )
    {
      o->Splines[ y ].CopyTo( &( s->Splines[ y ] ) );
      s->Spline.Splines[ y ] = (CphxSpline_float16*)s->Splines[ y ].Spline;
    }
    SplineBackup += s;
  }
}

void CphxMaterialSplineBatch_Tool::RestoreSplines()
{
  //splines already restored in build, just need to flush the rest
  SplineBackup.FreeArray();
}

void CphxMaterialSplineBatch_Tool::Copy( CphxMaterialSplineBatch_Tool *Original )
{
  Splines.FreeArray();
  Batch->Splines = NULL;
  MinimalSplineArray.FlushFast();

  for ( TS32 x = 0; x < Original->Splines.NumItems(); x++ )
  {
    CphxMaterialSpline_Tool *o = Original->Splines[ x ];

    CphxMaterialSpline_Tool *s = new CphxMaterialSpline_Tool();
    s->TargetParamGUID = o->TargetParamGUID;
    s->TargetObjectGUID = o->TargetObjectGUID;
    s->Spline.GroupingData = o->Spline.GroupingData;
    s->Spline.Target = o->Spline.Target;
    for ( TS32 y = 0; y < 4; y++ )
    {
      o->Splines[ y ].CopyTo( &( s->Splines[ y ] ) );
      s->Spline.Splines[ y ] = (CphxSpline_float16*)s->Splines[ y ].Spline;
    }
    Splines += s;

    MinimalSplineArray.Add( &s->Spline );
  }

  Batch->SplineCount = MinimalSplineArray.NumItems();
  Batch->Splines = MinimalSplineArray.GetPointer( 0 );
}

CphxMaterialSpline_Tool * CphxMaterialSplineBatch_Tool::FindSpline( CphxMaterialParameter_Tool *param, void *GroupingData )
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
  {
    CphxMaterialSpline_Tool *s = Splines[ x ];
    if ( s->TargetParamGUID != param->GetGUID() ) continue;

    if ( GroupingData == NULL ) return s;
    CphxResource *r = Project.GetResource( s->TargetObjectGUID );
    if ( r == GroupingData ) return s;
  }
  return NULL;
}


//void CphxMaterialSplineBatch_Tool::ImportParameters(CphxMaterialParameterBatch_Tool *b, TBOOL VariableParams, TBOOL AnimatedParams)
//{
//	if (!b) return;
//	for (TS32 x = 0; x < b->Parameters.NumItems(); x++)
//	{
//		if ((VariableParams && b->Parameters[x]->Parameter.Scope == PARAM_VARIABLE) ||
//			(AnimatedParams && b->Parameters[x]->Parameter.Scope == PARAM_ANIMATED))
//		{
//			CphxMaterialSpline_Tool *l = new CphxMaterialSpline_Tool();
//			l->TargetParamGUID = b->Parameters[x]->GetGUID();
//			l->Spline.Target = &b->Parameters[x]->Parameter;
//			l->Spline.Value.Value = b->Parameters[x]->DefaultValue;
//			Splines += l;
//		}
//	}
//
//}

TU32 DictionaryHash( CphxGUID g )
{
  CString s = g.GetString();
  return s.GetHash();
}

//void CphxMaterialSplineBatch_Tool::BackupParamValues()
//{
//	Storage.FreeAll();
//	for (TS32 x = 0; x < Splines.NumItems(); x++)
//	{
//		Storage[Splines[x]->TargetParamGUID] = Splines[x]->Value;
//		Splines[x]->Value = NULL;
//		//for (TS32 y = 0; y < 4; y++)
//		//	Links[x]->Link.Value.Spline[y]=NULL;
//	}
//}
//
//void CphxMaterialSplineBatch_Tool::RestoreParamValues()
//{
//	for (TS32 x = 0; x < Splines.NumItems(); x++)
//		if (Storage.HasKey(Splines[x]->TargetParamGUID))
//		{
//			SAFEDELETE(Splines[x]->Value);
//			//for (TS32 y = 0; y < 4; y++)
//			//	SAFEDELETE(Links[x]->Link.Value.Spline[y]);
//			Splines[x]->Value = Storage[Splines[x]->TargetParamGUID];
//			Splines[x]->Spline.Value = Splines[x]->Value->Value;
//
//			Storage[Splines[x]->TargetParamGUID] = NULL;
//
//			//for (TS32 y = 0; y < 4; y++)
//			//	Storage[Links[x]->TargetParamGUID].Spline[y] = NULL;
//		}
//
//	for (TS32 x = 0; x < Storage.NumItems(); x++)
//		SAFEDELETE(Storage.GetByIndex(x));
//}
//
//void CphxMaterialSplineBatch_Tool::BuildLinkBatch(CphxMaterialSplineBatch *LinkBatch)
//{
//	LinkBatch->SplineCount = Splines.NumItems();
//	SAFEDELETE(LinkBatch->Splines);
//	LinkBatch->Splines = new CphxMaterialSpline[Splines.NumItems()];
//	for (TS32 x = 0; x < Splines.NumItems(); x++)
//		LinkBatch->Splines[x] = Splines[x]->Spline;
//}

CphxMaterialSpline_Tool::CphxMaterialSpline_Tool()
{
  //Value = new CphxMaterialParameterValue_Tool();
  //for (TS32 x = 0; x < 4; x++)
  //	Spline.Value.Spline[x] = Value->Value.Spline[x];
}

CphxMaterialSpline_Tool::~CphxMaterialSpline_Tool()
{
  //SAFEDELETE(Value);
}

//CphxMaterialParameterValue_Tool::CphxMaterialParameterValue_Tool()
//{
//	for (TS32 x = 0; x < 4; x++)
//	{
//		Splines[x] = new CphxSpline_Tool_float16();
//		Value.Spline[x] = (CphxSpline_float16*)Splines[x]->Spline;
//		Value.Value.Color[0] = 0;
//		Value.Value.Color[1] = 0;
//		Value.Value.Color[2] = 0;
//		Value.Value.Color[3] = 0;
//	}
//}
//
//CphxMaterialParameterValue_Tool::~CphxMaterialParameterValue_Tool()
//{
//	for (TS32 x = 0; x < 4; x++)
//	{
//		SAFEDELETE(Splines[x]);
//	}
//}

void CphxMaterial_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  for ( TS32 x = 0; x < Techniques.NumItems(); x++ )
    Node->AddChild( _T( "Tech" ), false ).SetText( Techniques[ x ]->GetGUID().GetString() );
}

void CphxMaterial_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Tech" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Tech" ), x );
    CphxGUID guid;
    guid.SetString( n.GetText().GetPointer() );
    CphxMaterialTechnique_Tool *t = Project.GetTech( guid );
    if ( t ) AddTech( t );
  }

}

CphxMaterial_Tool::CphxMaterial_Tool()
{
  Name = _T( "New Material" );
  Material.TechCount = 0;
  Material.Techniques = NULL;
  External = false;
}

CphxMaterial_Tool::~CphxMaterial_Tool()
{
  SAFEDELETEA( Material.Techniques );
}

TBOOL CphxMaterial_Tool::GenerateResource( CCoreDevice *Dev )
{
  return true;
}

void CphxMaterial_Tool::AddTech( CphxMaterialTechnique_Tool *t )
{
  if ( Techniques.Find( t ) >= 0 ) return;

  Techniques += t;
  AddParent( t );

  CphxMaterialTechnique **tl = Material.Techniques;
  Material.TechCount++;
  Material.Techniques = new CphxMaterialTechnique*[ Material.TechCount ];
  for ( TS32 x = 0; x < Material.TechCount - 1; x++ )
    Material.Techniques[ x ] = tl[ x ];
  Material.Techniques[ Material.TechCount - 1 ] = t->Tech;
  SAFEDELETEA( tl );
}

void CphxMaterial_Tool::ReplaceTech( CphxMaterialTechnique_Tool *original, CphxMaterialTechnique_Tool *updated )
{
  TS32 idx = Techniques.Find( original );
  if ( idx < 0 ) return;

  Techniques[ idx ] = updated;
  RemoveParent( original );
  AddParent( updated );

  CphxMaterialTechnique **tl = Material.Techniques;
  tl[ idx ] = updated->Tech;
}


CphxMaterialParameter_Tool * CphxMaterial_Tool::GetParameter( CphxGUID &guid )
{
  for ( TS32 x = 0; x < Techniques.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool *p = Techniques[ x ]->GetParameter( guid );
    if ( p ) return p;
  }

  return NULL;
}

void CphxMaterial_Tool::RebuildDependents()
{
  InvalidateUptoDateFlag();

  for ( TS32 x = GetChildCount() - 1; x >= 0; x-- )
  {
    switch ( GetChild( x )->GetType() )
    {
    case PHX_MODELOBJECT:
    {
      auto *o = (CphxModelObject_Tool *)GetChild( x );
      if ( o->GetMaterial() == this )
        o->SetMaterial( this, true );
      else
        LOG_ERR( "[apex] Material update discrepancy!" );
    }
    break;
    case PHX_OBJECT:
    {
      auto *o = (CphxObject_Tool*)GetChild( x );
      switch ( o->GetObjectType() )
      {
      case Object_ParticleEmitterCPU:
      {
        auto *p = (CphxObject_ParticleEmitter_CPU_Tool*)o;
        if ( p->GetMaterial() == this )
          p->SetMaterial( this, true );
        else
          LOG_ERR( "[apex] Material update discrepancy!" );
      }
      break;
      default:
        LOG_ERR( "[apex] Unhandled Material Object Type (%d) During Update!", o->GetObjectType() );
      }
    }
    break;
    default:
      LOG_ERR( "[apex] Unhandled Material Target (%d) During Update!", GetChild( x )->GetType() );
      break;
    }
  }
}

CphxParameterDataCollector::CphxParameterDataCollector()
{
  RenderPriority = 128;
  Wireframe = false;

  RasterDesc.AntialiasedLineEnable;

  RasterDesc.FillMode = D3D11_FILL_SOLID;
  RasterDesc.CullMode = D3D11_CULL_BACK;
  RasterDesc.FrontCounterClockwise = false;
  RasterDesc.DepthBias = 0;
  RasterDesc.DepthBiasClamp = 0;
  RasterDesc.SlopeScaledDepthBias = 0;
  RasterDesc.DepthClipEnable = true;
  RasterDesc.ScissorEnable = false;
  RasterDesc.MultisampleEnable = false;
  RasterDesc.AntialiasedLineEnable = true;

  DepthStencilDesc.DepthEnable = true;
  DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
  DepthStencilDesc.StencilEnable = false;

  BlendDesc.AlphaToCoverageEnable = false;
  BlendDesc.IndependentBlendEnable = false;

  for ( TS32 x = 0; x < 8; x++ )
  {
    BlendDesc.RenderTarget[ x ].BlendEnable = false;
    BlendDesc.RenderTarget[ x ].SrcBlend = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[ x ].DestBlend = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[ x ].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[ x ].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[ x ].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[ x ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[ x ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  }

  Wireframe = false;

  for ( TS32 x = 0; x < 8; x++ )
    Textures[ x ] = NULL;

  RenderTarget = NULL;
}

CphxParameterDataCollector::~CphxParameterDataCollector()
{

}

void CphxParameterDataCollector::Apply( CphxMaterialPassConstantState *p )
{
  p->RenderPriority = RenderPriority;
  p->Wireframe = Wireframe != 0;
  if ( p->BlendState ) p->BlendState->Release();
  if ( p->DepthStencilState ) p->DepthStencilState->Release();
  if ( p->RasterizerState ) p->RasterizerState->Release();

  phxDev->CreateBlendState( &BlendDesc, &p->BlendState );
  phxDev->CreateRasterizerState( &RasterDesc, &p->RasterizerState );
  phxDev->CreateDepthStencilState( &DepthStencilDesc, &p->DepthStencilState );
  ApplyTextures( p );
  for ( TS32 x = 0; x < StaticFloatData.NumItems() && x < MATERIALDATASIZE / sizeof( float ); x++ )
    p->ConstantData[ x ] = StaticFloatData[ x ];
  p->ConstantDataSize = min( StaticFloatData.NumItems() * sizeof( float ), MATERIALDATASIZE / sizeof( float ) );
  p->RenderTarget = RenderTarget;
}

void CphxParameterDataCollector::ApplyTextures( CphxMaterialPassConstantState *p )
{
  for ( TS32 x = 0; x < 8; x++ )
    p->Textures[ x ] = Textures[ x ];
  p->RenderTarget = RenderTarget;
}
