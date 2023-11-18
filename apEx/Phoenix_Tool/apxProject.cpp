#define _WINSOCKAPI_
#include "BasePCH.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include "../Phoenix/Timeline.h"
#include "Timeline_tool.h"
#include "apxProject.h"
#include "../apEx/apExRoot.h"
#include "../Phoenix_Tool/Material_Tool.h"
#include "../apEx/TimelinePreview.h"
#include "../Phoenix/Material.h"
#include "../Phoenix/Timeline.h"
#include "../apEx/Config.h"

CapexProject Project;

COREFORMAT pixelFormats[] = //IF THESE CHANGE YOU NEED TO ADD THEM TO THE PROJECT.CPP AS WELL, make sure that the core formats here mean the same as the dxgi formats there
{
  COREFMT_R16G16B16A16_FLOAT,
  COREFMT_R32F,
};

CapexProject::CapexProject()
{
  Group = _T( "CONSPIRACY" );
  Title = _T( "[INSERT TITLE]" );
  Urls[ 0 ] = _T( "http://conspiracy.hu" );
  Urls[ 1 ] = _T( "http://conspiracy.hu/link/___***TITLEHERE***___/pouet/" );
  Urls[ 2 ] = _T( "http://facebook.com/conspiracy.hu" );
  Urls[ 3 ] = _T( "http://youtube.com/user/conspiracyhu" );
  Urls[ 4 ] = _T( "http://www.addthis.com/bookmark.php?url=http://conspiracy.hu" );

  LastSaveTime = globalTimer.GetTime();
  LoadedFileName = _T( "" );
  Timeline = new CphxTimeline_Tool();
  DemoResolutionX = 1;
  DemoResolutionY = 1;
  Song = NULL;

  MusicData.AppHandle = NULL;
  MusicData.MVXRenderStatus = 0;
  MusicData.Playing = false;
  MusicData.PlaybackStart = 0;
  MusicData.CurrentFrame = 0;
  MusicData.TimelineDragged = false;

  DefaultMaterial = new CphxMaterial_Tool();
  DefaultMaterial->GetGUID().SetString( _T( "DEFAULTMATERIAL!DEFAULTMATERIAL!" ) );

  DefaultTech = new CphxMaterialTechnique_Tool();
  DefaultTech->GetGUID().SetString( _T( "DEFAULTTECHNIQUEDEFAULTTECHNIQUE" ) );

  CphxMaterialRenderPass_Tool *pass = DefaultTech->CreatePass();
  CphxMaterialParameter_Tool *p;

  p = pass->CreateParam();
  p->Parameter.Scope = PARAM_CONSTANT;
  p->Parameter.Type = PARAM_CULLMODE;
  p->Parameter.Value.CullMode = D3D11_CULL_NONE;

  pass->ShaderCode = _T(
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
    "   float3 Normal: TEXCOORD1;\n"
    " float2 uv:TEXCOORD0;\n"
    "};\n"
    "\n"
    "VSOUT v(VSIN x)\n"
    "{\n"
    "	VSOUT k;\n"
    "	k.Position = mul(projmat,mul(viewmat,mul(worldmat,float4(x.Position,1))));\n"
    "	k.uv=x.UV.xy;\n"
    "   k.Normal=x.Normal;\n"
    "	return k;\n"
    "}\n"
    "\n"
    "float4 p(VSOUT v) : SV_TARGET0\n"
    "{\n float c=saturate(dot(normalize(float3(1,1,1)),normalize(v.Normal))*0.8)+0.2;"
    "	return float4(c,c,c,1);\n"
    "}\n" );

  //pass = DefaultTech->CreatePass();
  //
  ////pass->Pass->Wireframe = true;

  //pass->ShaderCode = _T(
  //	"cbuffer b : register(b0){float4x4 viewmat;float4x4 projmat;}\n"
  //	"cbuffer c : register(b1){float4x4 worldmat;}\n"
  //	"struct VSIN\n"
  //	"{\n"
  //	"	float3 Position : POSITION0;\n"
  //	"	float3 Position2: POSITION1;\n"
  //	"	float3 Normal : NORMAL0;\n"
  //	"	float4 Color : COLOR0;\n"
  //	"	float4 UV : TEXCOORD0;\n"
  //	"	float4 UV2: TEXCOORD1;\n"
  //	"};\n\n"
  //	"struct VSOUT\n"
  //	"{\n"
  //	"	float4 Position : SV_POSITION;\n"
  //	"   float3 Normal: TEXCOORD1;\n"
  //	" float2 uv:TEXCOORD0;\n"
  //	"};\n"
  //	"\n"
  //	"VSOUT v(VSIN x)\n"
  //	"{\n"
  //	"	VSOUT k;\n"
  //	"	k.Position = mul(projmat,mul(viewmat,mul(worldmat,float4(x.Position,1))));\n"
  //	"	k.uv=x.UV.xy;\n"
  //	"   k.Normal=x.Normal;\n"
  //	"	return k;\n"
  //	"}\n"
  //	"\n"
  //	"float4 p(VSOUT v) : SV_TARGET0\n"
  //	"{\n"
  //	"	return float4(140,140,140,255)/255.0f;\n"
  //	"}\n");

  //p = pass->CreateParam();
  //p->Parameter.Scope = PARAM_CONSTANT;
  //p->Parameter.Type = PARAM_FILLMODE;
  //p->Parameter.Value.Wireframe = true;

  //p = pass->CreateParam();
  //p->Parameter.Scope = PARAM_CONSTANT;
  //p->Parameter.Type = PARAM_ZFUNCTION;
  //p->Parameter.Value.BlendMode = D3D11_COMPARISON_LESS_EQUAL;

  DefaultMaterial->AddTech( DefaultTech );
}

CapexProject::~CapexProject()
{
  SAFEDELETE( DefaultTech );
  SAFEDELETE( DefaultMaterial );
  Reset();
  SAFEDELETE( Timeline );
}

extern CapexRoot *Root;

void CapexProject::Reset()
{

  Group = _T( "CONSPIRACY" );
  Title = _T( "[INSERT TITLE]" );
  Urls[ 0 ] = _T( "http://conspiracy.hu" );
  Urls[ 1 ] = _T( "http://conspiracy.hu/link/___***TITLEHERE***___/pouet/" );
  Urls[ 2 ] = _T( "http://facebook.com/conspiracy.hu" );
  Urls[ 3 ] = _T( "http://youtube.com/user/conspiracyhu" );
  Urls[ 4 ] = _T( "http://www.addthis.com/bookmark.php?url=http://conspiracy.hu" );

  if ( Song )
    Song->Stop();
  SAFEDELETE( Song );

  MusicData.TickData.Flush();
  MusicData.OfflineLength = 0;

  MusicData.AppHandle = NULL;
  MusicData.MVXRenderStatus = 0;
  MusicData.Playing = false;
  MusicData.PlaybackStart = 0;
  MusicData.CurrentFrame = 0;
  MusicData.TimelineDragged = false;

  DemoResolutionX = 1;
  DemoResolutionY = 1;
  SAFEDELETE( Timeline );
  Timeline = new CphxTimeline_Tool();

  Models.FreeArray();

  RenderLayers.FreeArray();
  RenderTargets.FreeArray();
  Filters.FreeArray();
  TexgenPages.FreeArray();
  Techniques.FreeArray();
  Materials.FreeArray();
  Scenes.FreeArray();
  TreeSpecies.FreeArray();

  UpdateRTCollections();
}

CapexTexGenPage *CapexProject::CreateTexgenPage()
{
  APEXPAGEID id = 0;
  if ( TexgenPages.NumItems() ) id = TexgenPages.Last()->GetID() + 1;

  CapexTexGenPage *p = new CapexTexGenPage( id );
  TexgenPages += p;

  return p;
}

TBOOL CapexProject::DeleteTexgenPage( APEXPAGEID ID )
{
  for ( int x = 0; x < TexgenPages.NumItems(); x++ )
    if ( TexgenPages[ x ]->GetID() == ID )
    {
      TexgenPages.FreeByIndex( x );
      return true;
    }

  return false;
}

CapexTexGenPage *CapexProject::GetTexgenPage( APEXPAGEID ID )
{
  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
    if ( TexgenPages[ x ]->GetID() == ID )
      return TexgenPages[ x ];
  return NULL;
}

CphxTextureFilter_Tool * CapexProject::CreateTextureFilter()
{
  CphxTextureFilter_Tool *f = new CphxTextureFilter_Tool();

  Filters += f;

  return f;
}

TBOOL CapexProject::DeleteTextureFilter( CphxGUID &GUID )
{
  for ( TS32 x = 0; x < Filters.NumItems(); x++ )
    if ( Filters[ x ]->GetGUID() == GUID )
    {
      if ( Filters[ x ]->HasDependants() ) return false;
      Filters.FreeByIndex( x );
      return true;
    }
  return true;
}

CphxTextureFilter_Tool * CapexProject::GetTextureFilter( CphxGUID &GUID )
{
  for ( TS32 x = 0; x < Filters.NumItems(); x++ )
    if ( Filters[ x ]->GetGUID() == GUID ) return Filters[ x ];
  return NULL;
}

TS32 CapexProject::GetTextureFilterIndex( CphxGUID &GUID )
{
  for ( TS32 x = 0; x < Filters.NumItems(); x++ )
    if ( Filters[ x ]->GetGUID() == GUID ) return x;
  return -1;
}

void CapexProject::ExportTextureFilters( CXMLNode *RootNode )
{
  if ( !RootNode ) return;
  for ( TS32 x = 0; x < Filters.NumItems(); x++ )
  {
    CXMLNode n = RootNode->AddChild( _T( "texturefilter" ) );
    Filters[ x ]->Export( &n );
  }
}

TS32 FilterNameSorter( CphxTextureFilter_Tool **a, CphxTextureFilter_Tool **b )
{
  return CString::CompareNoCase( ( *a )->Name, ( *b )->Name );
}

TS32 MaterialNameSorter( CphxMaterial_Tool **a, CphxMaterial_Tool **b )
{
  return CString::CompareNoCase( ( *a )->Name, ( *b )->Name );
}

TS32 TechNameSorter( CphxMaterialTechnique_Tool **a, CphxMaterialTechnique_Tool **b )
{
  return CString::CompareNoCase( ( *a )->Name, ( *b )->Name );
}

void CapexProject::ImportTextureFilters( CXMLNode *RootNode, TBOOL External )
{
  if ( !RootNode ) return;

  //Filters.Flush();

  for ( TS32 x = 0; x < RootNode->GetChildCount( _T( "texturefilter" ) ); x++ )
  {
    CphxTextureFilter_Tool *f = new CphxTextureFilter_Tool();
    f->Import( &RootNode->GetChild( _T( "texturefilter" ), x ) );
    f->External = External;

    if ( GetTextureFilter( f->GetGUID() ) )
      delete f;
    else
      Filters += f;
  }

  Filters.Sort( FilterNameSorter );
}

void CapexProject::ExportMaterials( CXMLNode *RootNode )
{
  if ( !RootNode ) return;

  for ( TS32 x = 0; x < RenderTargets.NumItems(); x++ )
  {
    CXMLNode n = RootNode->AddChild( _T( "rendertarget" ) );
    RenderTargets[ x ]->Export( &n );
  }

  for ( TS32 x = 0; x < RenderLayers.NumItems(); x++ )
  {
    CXMLNode n = RootNode->AddChild( _T( "renderlayer" ) );
    RenderLayers[ x ]->Export( &n );
  }

  for ( TS32 x = 0; x < Techniques.NumItems(); x++ )
  {
    CXMLNode n = RootNode->AddChild( _T( "rendertechnique" ) );
    Techniques[ x ]->Export( &n );
  }

  for ( TS32 x = 0; x < Materials.NumItems(); x++ )
  {
    CXMLNode n = RootNode->AddChild( _T( "material" ) );
    Materials[ x ]->Export( &n );
  }
}

void CapexProject::ImportMaterials( CXMLNode *RootNode, TBOOL External )
{
  if ( !RootNode ) return;

  //Filters.Flush();

  for ( TS32 x = 0; x < RootNode->GetChildCount( _T( "rendertarget" ) ); x++ )
  {
    CphxRenderTarget_Tool *f = new CphxRenderTarget_Tool();
    f->Import( &RootNode->GetChild( _T( "rendertarget" ), x ) );
    f->External = External;

    if ( GetRenderTarget( f->GetGUID() ) )
      delete f;
    else
      RenderTargets += f;
  }

  for ( TS32 x = 0; x < RootNode->GetChildCount( _T( "renderlayer" ) ); x++ )
  {
    CphxRenderLayerDescriptor_Tool *f = new CphxRenderLayerDescriptor_Tool();
    f->Import( &RootNode->GetChild( _T( "renderlayer" ), x ) );
    f->External = External;

    if ( GetRenderLayer( f->GetGUID() ) )
      delete f;
    else
      RenderLayers += f;
  }

  //set first layer to pickable
  TBOOL Found = false;
  for ( TS32 x = 0; x < RenderLayers.NumItems(); x++ )
    if ( RenderLayers[ x ]->HasPicking ) Found = true;

  if ( !Found && RenderLayers.NumItems() ) RenderLayers[ 0 ]->HasPicking = true;

  for ( TS32 x = 0; x < RootNode->GetChildCount( _T( "rendertechnique" ) ); x++ )
  {
    CphxMaterialTechnique_Tool *f = new CphxMaterialTechnique_Tool();
    f->Import( &RootNode->GetChild( _T( "rendertechnique" ), x ) );
    f->External = External;

    if ( GetTech( f->GetGUID() ) )
      delete f;
    else
      Techniques += f;
  }

  for ( TS32 x = 0; x < RootNode->GetChildCount( _T( "material" ) ); x++ )
  {
    CphxMaterial_Tool *f = new CphxMaterial_Tool();
    f->Import( &RootNode->GetChild( _T( "material" ), x ) );
    f->External = External;

    if ( GetMaterial( f->GetGUID() ) )
      delete f;
    else
      Materials += f;
  }

  Techniques.Sort( TechNameSorter );
  Materials.Sort( MaterialNameSorter );
}


CphxTextureOperator_Tool * CapexProject::GetTexgenOp( APEXOPID ID )
{
  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
    for ( TS32 y = 0; y < TexgenPages[ x ]->GetOpCount(); y++ )
      if ( TexgenPages[ x ]->GetOp( y )->ID == ID ) return TexgenPages[ x ]->GetOp( y );
  return NULL;
}

CphxTextureOperator_Tool * CapexProject::GetTexgenOp( CphxGUID &ID )
{
  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
    for ( TS32 y = 0; y < TexgenPages[ x ]->GetOpCount(); y++ )
      if ( TexgenPages[ x ]->GetOp( y )->GetGUID() == ID ) return TexgenPages[ x ]->GetOp( y );
  return NULL;
}

void CapexProject::ImportMain( CXMLDocument &d, bool reset )
{
  if ( reset )
    Reset();

  CXMLNode RootNode = d.GetDocumentNode().GetChild( "apExProject" );
  ImportTreeSpecies( &RootNode, false );
  ImportTextureFilters( &RootNode, false );

  for ( TS32 x = 0; x < RootNode.GetChildCount( _T( "texturepage" ) ); x++ )
  {
    CXMLNode n = RootNode.GetChild( _T( "texturepage" ), x );
    CapexTexGenPage *p = CreateTexgenPage();
    p->Import( &n );
  }

  ImportMaterials( &RootNode, false );

  //import models
  for ( TS32 x = 0; x < RootNode.GetChildCount( _T( "model" ) ); x++ )
  {
    CXMLNode n = RootNode.GetChild( _T( "model" ), x );
    CphxModel_Tool *t = CreateModel();
    t->Import( &n );
  }

  //import scenes - only need the list first - this is so subscenes have something to reference
  for ( TS32 x = 0; x < RootNode.GetChildCount( _T( "scene" ) ); x++ )
  {
    CXMLNode n = RootNode.GetChild( _T( "scene" ), x );
    CphxScene_Tool *t = CreateScene();
    t->ImportGUID( &n );
  }

  for ( TS32 x = 0; x < RootNode.GetChildCount( _T( "scene" ) ); x++ )
  {
    CXMLNode n = RootNode.GetChild( _T( "scene" ), x );
    CphxScene_Tool *t = Scenes[ x ];
    t->Import( &n );
    t->ApplyCameraDataForFirstCam();
  }

  //import events

  TS32 x = Timeline->Timeline->AspectX;
  if ( RootNode.GetChildCount( _T( "AspectX" ) ) ) RootNode.GetChild( _T( "AspectX" ) ).GetValue( x );
  Timeline->Timeline->AspectX = x;
  x = Timeline->Timeline->AspectY;
  if ( RootNode.GetChildCount( _T( "AspectY" ) ) ) RootNode.GetChild( _T( "AspectY" ) ).GetValue( x );
  Timeline->Timeline->AspectY = x;

  x = Timeline->Timeline->FrameRate;
  if ( RootNode.GetChildCount( _T( "Framerate" ) ) ) RootNode.GetChild( _T( "Framerate" ) ).GetValue( x );
  Timeline->Timeline->FrameRate = x;

  if ( RootNode.GetChildCount( _T( "Title" ) ) ) Title = RootNode.GetChild( _T( "Title" ) ).GetText();
  if ( RootNode.GetChildCount( _T( "Group" ) ) ) Group = RootNode.GetChild( _T( "Group" ) ).GetText();

  if ( RootNode.GetChildCount( _T( "www" ) ) ) Urls[ 0 ] = RootNode.GetChild( _T( "www" ) ).GetText();
  if ( RootNode.GetChildCount( _T( "pouet" ) ) ) Urls[ 1 ] = RootNode.GetChild( _T( "pouet" ) ).GetText();
  if ( RootNode.GetChildCount( _T( "facebook" ) ) ) Urls[ 2 ] = RootNode.GetChild( _T( "facebook" ) ).GetText();
  if ( RootNode.GetChildCount( _T( "youtube" ) ) ) Urls[ 3 ] = RootNode.GetChild( _T( "youtube" ) ).GetText();
  if ( RootNode.GetChildCount( _T( "share" ) ) ) Urls[ 4 ] = RootNode.GetChild( _T( "share" ) ).GetText();

  if ( RootNode.GetChildCount( _T( "shaderminifierenabled" ) ) )       RootNode.GetChild(_T( "shaderminifierenabled" ) ).GetValue( EnableShaderMinifier );
  if ( RootNode.GetChildCount( _T( "globalshaderminifierenabled" ) ) ) RootNode.GetChild(_T( "globalshaderminifierenabled" ) ).GetValue( EnableGlobalShaderMinifier );
  if ( RootNode.GetChildCount( _T( "minimalprecalc" ) ) )              RootNode.GetChild(_T( "minimalprecalc" ) ).GetValue( EnableMinimalPrecalc );
  if ( RootNode.GetChildCount( _T( "farbrauschprecalc" ) ) )           RootNode.GetChild(_T( "farbrauschprecalc" ) ).GetValue( EnableFarbrauschPrecalc );
  if ( RootNode.GetChildCount( _T( "setuphassocial" ) ) )              RootNode.GetChild(_T( "setuphassocial" ) ).GetValue( EnableSetupHasSocial );

  if ( RootNode.GetChildCount( _T( "gridbpm" ) ) ) RootNode.GetChild( _T( "gridbpm" ) ).GetValue( Timeline->GridBPM );
  if ( RootNode.GetChildCount( _T( "gridprimaryhighlight" ) ) ) RootNode.GetChild( _T( "gridprimaryhighlight" ) ).GetValue( Timeline->BPMPrimaryModulus );
  if ( RootNode.GetChildCount( _T( "gridsecondaryhighlight" ) ) ) RootNode.GetChild( _T( "gridsecondaryhighlight" ) ).GetValue( Timeline->BPMSecondaryModulus );
  if ( RootNode.GetChildCount( _T( "songfile" ) ) ) SongFile = RootNode.GetChild( _T( "songfile" ) ).GetText();


  for ( x = 0; x < RootNode.GetChildCount( _T( "event" ) ); x++ )
  {
    CXMLNode n = RootNode.GetChild( _T( "event" ), x );
    if ( !n.GetChildCount( _T( "Type" ) ) ) continue;
    PHXEVENTTYPE EventType;
    if ( !n.GetChild( _T( "Type" ) ).GetValue( (TS32&)EventType ) ) continue;

    CphxEvent_Tool *e = Timeline->CreateEvent( EventType, 0, 0, 0, NULL );
    e->Import( &n );
  }

  Timeline->Sort();

  //load library data
  ImportLibraries();

  MakeTextureOperatorGUIDConnections();
  for ( x = 0; x < TexgenPages.NumItems(); x++ )
    TexgenPages[ x ]->BuildOperatorConnections();

  //due to historic reasons the shadertoy event parent connections need to be created here
  for ( TS32 x = 0; x < GetEventCount(); x++ )
  {
    auto *e = GetEventByIndex( x );
    if ( e->GetEventType() == EVENT_SHADERTOY )
    {
      auto *st = (CphxEvent_Shadertoy_Tool *)e;
      for ( TS32 y = 0; y < st->MaterialData.MaterialTextures.NumItems(); y++ )
      {
        st->AddParent( Project.GetResource( st->MaterialData.MaterialTextures.GetByIndex( y ) ) );
      }
    }
  }
}

void CapexProject::ImportMaterialMain( CXMLDocument &d )
{
  CXMLNode RootNode = d.GetDocumentNode().GetChild( "apExMaterial" );
  CArray<CphxMaterial_Tool*> MaterialsToUpdate;

  for ( TS32 x = 0; x < RootNode.GetChildCount( _T( "rendertechnique" ) ); x++ )
  {
    CphxMaterialTechnique_Tool *f = new CphxMaterialTechnique_Tool();
    f->Import( &RootNode.GetChild( _T( "rendertechnique" ), x ) );
    f->External = false;

    CphxMaterialTechnique_Tool *t = GetTech( f->GetGUID() );

    if ( t )
    {
      for ( TS32 x = t->GetChildCount() - 1; x >= 0; x-- ) //we'll be removing children so it's better to go backwards
      {
        switch ( t->GetChild( x )->GetType() )
        {
        case PHX_MATERIAL:
        {
          CphxMaterial_Tool *m = (CphxMaterial_Tool*)t->GetChild( x );
          for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
            if ( m->Techniques[ y ]->GetGUID() == f->GetGUID() )
            {
              m->ReplaceTech( t, f );
              m->InvalidateUptoDateFlag();
            }
          MaterialsToUpdate.AddUnique( m );
          break;
        }
        case PHX_EVENT:
        {
          CphxEvent_Tool *e = (CphxEvent_Tool *)t->GetChild( x );
          switch ( e->GetEventType() )
          {
          case EVENT_SHADERTOY:
          {
            auto *st = (CphxEvent_Shadertoy_Tool *)e;
            st->SetTech( f, true );
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
          LOG_ERR( "[apex] Unhandled Technique Target (%d) During Update!", t->GetChild( x )->GetType() );

        }
      }

      //SetStatusbarText("Material Updating Not Supported Properly Yet, THIS WILL CAUSE CRASHES UNLESS YOU SAVE AND RELOAD IMMEDIATELY!");
      TS32 idx = Techniques.Find( t );
      Techniques[ idx ] = f;
      delete t;
    }
    else
      Techniques += f;
  }

  for ( TS32 x = 0; x < RootNode.GetChildCount( _T( "material" ) ); x++ )
  {
    CphxMaterial_Tool *f = new CphxMaterial_Tool();
    f->Import( &RootNode.GetChild( _T( "material" ), x ) );
    f->External = false;

    CphxMaterial_Tool *m = GetMaterial( f->GetGUID() );
    if ( m )
    {
      m->Techniques.FlushFast();
      m->Import( &RootNode.GetChild( _T( "material" ), x ) );
      m->InvalidateUptoDateFlag();
      MaterialsToUpdate.AddUnique( m );
      delete f;
    }
    else
      Materials += f;
  }

  for ( TS32 x = 0; x < MaterialsToUpdate.NumItems(); x++ )
    MaterialsToUpdate[ x ]->RebuildDependents();

  Techniques.Sort( TechNameSorter );
  Materials.Sort( MaterialNameSorter );
}

#include <shlwapi.h>
#include <pathcch.h>

void CapexProject::Import( CString &FileName, HWND hwnd )
{
  CXMLDocument d;

  if ( !d.LoadFromFile( FileName.GetPointer() ) )
  {
    LOG_ERR( "[project] Error importing document!" );
    return;
  }

  if ( !d.GetDocumentNode().GetChildCount( "apExProject" ) )
  {
    LOG_ERR( "[project] Document missing root node." );
    return;
  }

  ImportMain( d );

  LoadedFileName = FileName;

  TCHAR name[ MAX_PATH ];
  if ( GetFullPathName( FileName.GetPointer(), MAX_PATH, name, nullptr ) )
    LoadedFileName = CString( name );

  LastSaveTime = globalTimer.GetTime();
  UpdateRTCollections();

  //import music
  if ( SongFile.Length() && CanLoadSong() )
  {
    CStreamReaderFile f;
    bool open = f.Open( SongFile.GetPointer() );
    if ( !open )
    {
      TCHAR name[ MAX_PATH ];
      LPSTR* fileName = nullptr;
      if ( GetFullPathName( FileName.GetPointer(), MAX_PATH, name, fileName ) )
      {
        PathRemoveFileSpecA( name );

        TCHAR relative[ MAX_PATH ];
        PathCombineA( relative, name, SongFile.GetPointer() );
        open = f.Open( relative );
      }
    }

    if ( open )
      if ( f.GetLength() )
        LoadSong( f, hwnd );
  }
}

void CapexProject::ImportMaterial( CString &FileName, HWND hwnd )
{
  CXMLDocument d;

  if ( !d.LoadFromFile( FileName.GetPointer() ) )
  {
    LOG_ERR( "[project] Error importing document!" );
    return;
  }

  if ( !d.GetDocumentNode().GetChildCount( "apExMaterial" ) )
  {
    LOG_ERR( "[project] Document missing root node." );
    return;
  }

  ImportMaterialMain( d );

  UpdateRTCollections();
}


void CapexProject::MergeProject( CString &FileName, HWND hwnd )
{
  CXMLDocument d;

  if ( !d.LoadFromFile( FileName.GetPointer() ) )
  {
    LOG_ERR( "[project] Error importing document!" );
    return;
  }

  if ( !d.GetDocumentNode().GetChildCount( "apExProject" ) )
  {
    LOG_ERR( "[project] Document missing root node." );
    return;
  }

  ImportMain( d, false );

  UpdateRTCollections();
}

extern CString apexBuild;

CString CapexProject::ExportToString( TBOOL saveClean )
{
  CXMLDocument d;
  CXMLNode n = d.GetDocumentNode();
  n = n.AddChild( "apExProject" );
  n.SetAttribute( _T( "build" ), apexBuild.GetPointer() );

  n.AddChild( _T( "AspectX" ) ).SetInt( Timeline->Timeline->AspectX );
  n.AddChild( _T( "AspectY" ) ).SetInt( Timeline->Timeline->AspectY );
  n.AddChild( _T( "Framerate" ) ).SetInt( Timeline->Timeline->FrameRate );
  n.AddChild( _T( "Title" ) ).SetText( Title.GetPointer() );
  n.AddChild( _T( "Group" ) ).SetText( Group.GetPointer() );

  n.AddChild( _T( "www" ) ).SetText( Urls[ 0 ].GetPointer() );
  n.AddChild( _T( "pouet" ) ).SetText( Urls[ 1 ].GetPointer() );
  n.AddChild( _T( "facebook" ) ).SetText( Urls[ 2 ].GetPointer() );
  n.AddChild( _T( "youtube" ) ).SetText( Urls[ 3 ].GetPointer() );
  n.AddChild( _T( "share" ) ).SetText( Urls[ 4 ].GetPointer() );

  n.AddChild( _T( "shaderminifierenabled" ) ).SetInt( EnableShaderMinifier );
  n.AddChild( _T( "globalshaderminifierenabled" ) ).SetInt( EnableGlobalShaderMinifier );
  n.AddChild( _T( "minimalprecalc" ) ).SetInt( EnableMinimalPrecalc );
  n.AddChild( _T( "farbrauschprecalc" ) ).SetInt( EnableFarbrauschPrecalc );
  n.AddChild( _T( "setuphassocial" ) ).SetInt( EnableSetupHasSocial );

  n.AddChild( _T( "gridbpm" ) ).SetFloat( Timeline->GridBPM );
  n.AddChild( _T( "gridprimaryhighlight" ) ).SetInt( Timeline->BPMPrimaryModulus );
  n.AddChild( _T( "gridsecondaryhighlight" ) ).SetInt( Timeline->BPMSecondaryModulus );
  if ( SongFile.Length() ) n.AddChild( _T( "songfile" ) ).SetText( SongFile );

  //export tree species
  for ( TS32 x = 0; x < TreeSpecies.NumItems(); x++ )
    if ( !TreeSpecies[ x ]->External || TreeSpecies[ x ]->HasDependants() ) //only export used and those filters that weren't loaded from the filter library
    {
      CXMLNode f = n.AddChild( _T( "treespecies" ) );
      TreeSpecies[ x ]->Export( &f );
    }

  //ExportTextureFilters(&n);

  //export texture filters
  for ( TS32 x = 0; x < Filters.NumItems(); x++ )
    if ( !Filters[ x ]->External || Filters[ x ]->HasDependants() ) //only export used and those filters that weren't loaded from the filter library
    {
      CXMLNode f = n.AddChild( _T( "texturefilter" ) );
      Filters[ x ]->Export( &f );
    }

  //export texture pages
  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
  {
    CXMLNode p = n.AddChild( _T( "texturepage" ) );
    TexgenPages[ x ]->Export( &p );
  }

  //ExportMaterials(&n);

  TBOOL CleanSave = false;

  for ( TS32 x = 0; x < RenderTargets.NumItems(); x++ )
    if ( ( !RenderTargets[ x ]->External && !CleanSave ) || RenderTargets[ x ]->HasDependants() ) //only export used and those filters that weren't loaded from the filter library
    {
      CXMLNode t = n.AddChild( _T( "rendertarget" ) );
      RenderTargets[ x ]->Export( &t );
    }

  for ( TS32 x = 0; x < RenderLayers.NumItems(); x++ )
    //if ( ( !RenderLayers[ x ]->External && !CleanSave ) /*|| RenderLayers[ x ]->HasDependants()*/ ) //only export those filters that weren't loaded from the filter library - EXPORT NON USED FILTERS TOO!
    {
      CXMLNode t = n.AddChild( _T( "renderlayer" ) );
      RenderLayers[ x ]->Export( &t );
    }

  for ( TS32 x = 0; x < Techniques.NumItems(); x++ )
    if ( ( !Techniques[ x ]->External && !CleanSave ) || Techniques[ x ]->HasDependants() ) //only export used and those filters that weren't loaded from the filter library
    {
      CXMLNode t = n.AddChild( _T( "rendertechnique" ) );
      Techniques[ x ]->Export( &t );
    }

  for ( TS32 x = 0; x < Materials.NumItems(); x++ )
    if ( ( !Materials[ x ]->External && !CleanSave ) || Materials[ x ]->HasDependants() ) //only export used and those filters that weren't loaded from the filter library
    {
      CXMLNode t = n.AddChild( _T( "material" ) );
      Materials[ x ]->Export( &t );
    }


  ////export rendertargets
  //for (TS32 x = 0; x < RenderTargets.NumItems(); x++)
  //{
  //	CXMLNode t = n.AddChild(_T("rendertarget"));
  //	RenderTargets[x]->Export(&t);
  //}

  ////export material techniques
  //for (TS32 x = 0; x < Techniques.NumItems(); x++)
  //{
  //	CXMLNode t = n.AddChild(_T("rendertechnique"));
  //	Techniques[x]->Export(&t);
  //}

  //export models
  for ( TS32 x = 0; x < Models.NumItems(); x++ )
  {
    CXMLNode m = n.AddChild( _T( "model" ) );
    Models[ x ]->Export( &m );
  }

  //export scenes
  for ( TS32 x = 0; x < Scenes.NumItems(); x++ )
  {
    CXMLNode m = n.AddChild( _T( "scene" ) );
    Scenes[ x ]->Export( &m );
  }

  //export events

  for ( TS32 x = 0; x < Timeline->Events.NumItems(); x++ )
  {
    CXMLNode t = n.AddChild( _T( "event" ) );
    Timeline->Events[ x ]->Export( &t );
  }

  return d.SaveToString();
}

void CapexProject::Export( CString &FileName, TBOOL BackupExisting, TBOOL saveClean )
{
  CString XML = ExportToString( saveClean );

  LastSaveTime = globalTimer.GetTime();

  if ( BackupExisting )
    DoBackupSave( FileName );

  HANDLE h = CreateFile( FileName.GetPointer(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL );
  if ( h == INVALID_HANDLE_VALUE )
  {
    LOG_ERR( "[project] Error exporting document!" );
    return;
  }

  //TS32 memalloc = memTracker.GetAllocatedMemorySize();

  /*char * sz8 = new char[XML.Length() * 3];
  XML.WriteAsMultiByte(sz8, XML.Length() * 3);*/
  DWORD b;
  //WriteFile(h, sz8, strlen(sz8), &b, NULL);
  WriteFile( h, XML.GetPointer(), XML.Length() * sizeof( TCHAR ), &b, NULL );
  CloseHandle( h );

  //delete[] sz8;

  SetStatusbarText( CString::Format( _T( "Project successfully saved to %s" ), FileName.GetPointer() ) );
}

void CapexProject::ImportLibraries()
{
  CXMLDocument d;

  if ( !d.LoadFromFile( "Data//Arboretum.xml" ) )
    LOG_WARN( "[apex] Error importing arboretum: library file not found." );
  else
    if ( d.GetDocumentNode().GetChildCount( _T( "apExProject" ) ) == 0 )
      LOG_ERR( "[apEx] Error importing arboretum: apExProject node not found." );
    else
      Project.ImportTreeSpecies( &d.GetDocumentNode().GetChild( _T( "apExProject" ) ), true );

  if ( !d.LoadFromFile( "Data//FilterLibrary.xml" ) )
    LOG_WARN( "[apex] Error importing texture filter library: library file not found." );
  else
    if ( d.GetDocumentNode().GetChildCount( _T( "apExProject" ) ) == 0 )
      LOG_ERR( "[apEx] Error importing texture filter library: apExProject node not found." );
    else
      Project.ImportTextureFilters( &d.GetDocumentNode().GetChild( _T( "apExProject" ) ), true );

  if ( !d.LoadFromFile( "Data//MaterialLibrary.xml" ) )
    LOG_WARN( "[apex] Error importing material library: library file not found." );
  else
    if ( d.GetDocumentNode().GetChildCount( _T( "apExProject" ) ) == 0 )
      LOG_ERR( "[apEx] Error importing material library: apExProject node not found." );
    else
      Project.ImportMaterials( &d.GetDocumentNode().GetChild( _T( "apExProject" ) ), true );

}

void CapexProject::MakeTextureOperatorGUIDConnections()
{
  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
    for ( TS32 y = 0; y < TexgenPages[ x ]->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *tx = TexgenPages[ x ]->GetOp( y );

      switch ( tx->GetOpType() )
      {
      case TEXGEN_OP_LOAD:
      {
        CphxTextureOperator_Load *l = (CphxTextureOperator_Load*)tx;
        CphxTextureOperator_Tool *to = GetTexgenOp( l->LoadTime_TargetOpGUID );
        l->LoadedOp = to;
      }
      break;
      case TEXGEN_OP_SUBROUTINECALL:
      {
        CphxTextureOperator_SubroutineCall *l = (CphxTextureOperator_SubroutineCall*)tx;
        CphxTextureOperator_Tool *to = GetTexgenOp( l->Loadtime_SubroutineGUID );
        if ( to->GetOpType() == TEXGEN_OP_SUBROUTINE )
          l->Subroutine = (CphxTextureOperator_Subroutine*)to;
        else
          LOG_ERR( "[import] Subroutine call %s referencing non subroutine operator!", l->GetGUID().GetString() );
      }
      break;
      case TEXGEN_OP_SUBROUTINE:
      {
        CphxTextureOperator_Subroutine *l = (CphxTextureOperator_Subroutine*)tx;
        for ( TS32 i = 0; i < l->Parameters.NumItems(); i++ )
          for ( TS32 j = 0; j < l->Parameters[ i ]->Targets.NumItems(); j++ )
          {
            CphxTextureOperator_Tool *to = GetTexgenOp( l->Parameters[ i ]->Targets[ j ]->TargetGUID );
            if ( !to )
              LOG_ERR( "[import] Subroutine %s referencing missing operator %s!", l->GetGUID().GetString(), l->Parameters[ i ]->Targets[ j ]->TargetGUID.GetString() );
            else
              l->Parameters[ i ]->Targets[ j ]->TargetID = to->ID;
          }
      }
      break;
      default:
        break;
      }

    }
}

void CapexProject::KillTextureLoops()
{
  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
    for ( TS32 y = 0; y < TexgenPages[ x ]->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *t = TexgenPages[ x ]->GetOp( y );
      if ( t->GetOpType() == TEXGEN_OP_LOAD || t->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
      {
        for ( TS32 a = 0; a < TexgenPages.NumItems(); a++ )
          for ( TS32 b = 0; b < TexgenPages[ a ]->GetOpCount(); b++ )
            TexgenPages[ a ]->GetOp( b )->TouchedByLoop = false;
        t->KillLoops();
      }
    }
}

TS32 ResourceAccessSorter( CphxResource **a, CphxResource **b )
{
  return ( *a )->GetLastTouchTime() - ( *b )->GetLastTouchTime();
}

void CapexProject::FreeTextureMem( TS32 TextureOpCount )
{
  CArray<CphxResource*> Operators;

  //for (TS32 x=0; x<TexgenPages.NumItems(); x++)
  //	for (TS32 y=0; y<TexgenPages[x]->GetOpCount(); y++)
  //		TexgenPages[x]->GetOp(y)->SubroutineTagged=false;

  //for (TS32 x=0; x<TexgenPages.NumItems(); x++)
  //	for (TS32 y=0; y<TexgenPages[x]->GetOpCount(); y++)
  //		if (TexgenPages[x]->GetOp(y)->GetOpType()==TEXGEN_OP_SAVE)
  //		{
  //			CphxTextureOperator_Save *s=(CphxTextureOperator_Save*)TexgenPages[x]->GetOp(y);
  //			if (s->GetContentOp())
  //				s->GetContentOp()->SubroutineTagged=true;
  //		}

  for ( TS32 x = 0; x < TexgenPages.NumItems(); x++ )
    for ( TS32 y = 0; y < TexgenPages[ x ]->GetOpCount(); y++ )
      if ( !TexgenPages[ x ]->GetOp( y )->IsRequiredForGenerator() && !TexgenPages[ x ]->GetOp( y )->UsedByMaterial()/* && !TexgenPages[x]->GetOp(y)->SubroutineTagged*/ )
      {
        if ( TexgenPages[ x ]->GetOp( y )->SubroutineRoot )
        {
          TexgenPages[ x ]->GetOp( y )->Allocate( false );
          TextureOpCount--;
          if ( !TextureOpCount ) return;
        }

        if ( TexgenPages[ x ]->GetOp( y )->IsAllocated() )
          Operators += TexgenPages[ x ]->GetOp( y );
      }

  Operators.Sort( ResourceAccessSorter );

  for ( TS32 x = 0; x < min( TextureOpCount, Operators.NumItems() ); x++ )
  {
    //LOG_DBG("[texgen] Deallocating texture %s",((CphxTextureOperator_Tool*)Operators[x])->GetName().GetPointer());
    Operators[ x ]->Allocate( false );
  }
}

void CapexProject::DoAutosave( TS32 Seconds )
{
  if ( Root->GetApplication()->GetLeftButtonState() || Root->GetApplication()->GetRightButtonState() || Root->GetApplication()->GetMiddleButtonState() )
  {
    LastClickedTime = globalTimer.GetTime();
    return;
  }

  if ( Project.MusicData.Playing )
  {
    LastClickedTime = globalTimer.GetTime();
    return;
  }

  if ( LoadedFileName.Length() <= 0 ) return;

  int timeLeft = max( (TS32)( 1000 * Seconds ) - ( (TS32)globalTimer.GetTime() - LastSaveTime ), (TS32)( 1000 * 3 ) - ( (TS32)globalTimer.GetTime() - LastClickedTime ) );
  if ( timeLeft <= 5000 )
    SetStatusbarText( CString::Format( _T( "Autosaving in %d..." ), (int)( timeLeft / 1000 ) + 1 ) );

  if ( ( globalTimer.GetTime() - LastSaveTime > (TU32)( 1000 * Seconds ) ) &&
       ( globalTimer.GetTime() - LastClickedTime > (TU32)( 1000 * 3 ) ) )
  {
    CreateDirectory( _T( "Data" ), 0 );
    CreateDirectory( _T( "Data\\Autosave" ), 0 );

    CStringArray a = LoadedFileName.Explode( _T( ".apx" ) );
    if ( a.NumItems() )
    {
      CStringArray b = a[ 0 ].Explode( _T( "\\" ) );
      if ( b.NumItems() )
      {
        CString out = _T( "Data\\Autosave\\" ) + b.Last() + _T( "_autosave.apx" );
        Export( out, false, false );
      }
    }
  }
}

void CapexProject::DoBackupSave( CString &Filename )
{
  TS32 MaxBackupCount = 5;

  if ( !exists( Filename ) ) return;

  CreateDirectory( _T( "Data" ), 0 );
  CreateDirectory( _T( "Data\\Autosave" ), 0 );

  CStringArray a = Filename.Explode( _T( ".apx" ) );
  if ( a.NumItems() )
  {
    CStringArray b = a[ 0 ].Explode( _T( "\\" ) );
    if ( b.NumItems() )
    {
      CString fn = b.Last();

      for ( TS32 x = MaxBackupCount - 1; x >= 0; x-- )
      {
        CString bckname = _T( "Data\\Autosave\\" ) + CString::Format( _T( "%s.%d.apx" ), fn.GetPointer(), x );
        CString newname = _T( "Data\\Autosave\\" ) + CString::Format( _T( "%s.%d.apx" ), fn.GetPointer(), x + 1 );
        if ( exists( bckname ) )
        {
          if ( x == MaxBackupCount - 1 )
            remove( bckname.GetPointer() );
          else
          {
            if ( rename( bckname.GetPointer(), newname.GetPointer() ) )
              LOG_ERR( "[apex] Failed to rename %s to %s", bckname.GetPointer(), newname.GetPointer() );
          }
        }
      }

      CString backupname = _T( "Data\\Autosave\\" ) + CString::Format( _T( "%s.%d.apx" ), fn.GetPointer(), 0 );
      Export( backupname, false, false );
    }
  }

}

#include <time.h>

void CapexProject::DoCrashSave()
{
  CreateDirectory( _T( "Data" ), 0 );
  CreateDirectory( _T( "Data\\Crashes" ), 0 );

  CString fname = _T( "crash.apx" );

  CStringArray a;
  if ( LoadedFileName.Length() )
    a = LoadedFileName.Explode( _T( ".apx" ) );
  else a = fname.Explode( _T( ".apx" ) );

  if ( a.NumItems() )
  {
    time_t rawtime;
    struct tm timeinfo;
    time( &rawtime );
    localtime_s( &timeinfo, &rawtime );

    CStringArray b = a[ 0 ].Explode( _T( "\\" ) );
    CString out = _T( "Data\\Crashes\\" ) + b.Last() + CString::Format( _T( "_%d_%.2d_%.2d_%.2d_%.2d_%.2d_crash.apx" ), timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec );
    Export( out, false, false );
  }

}

CphxMaterialTechnique_Tool * CapexProject::CreateTech()
{
  CphxMaterialTechnique_Tool *t = new CphxMaterialTechnique_Tool();
  Techniques += t;
  return t;
}

void CapexProject::DeleteRenderTarget( CphxRenderTarget_Tool *t )
{
  RenderTargets.Free( t );
}

CphxRenderTarget_Tool * CapexProject::CreateRenderTarget()
{
  CphxRenderTarget_Tool *t = new CphxRenderTarget_Tool();
  RenderTargets += t;
  UpdateRTCollections();
  return t;
}

void CapexProject::ApplyRenderTargets( CapexRenderTargetCollection *Collection )
{
  extern CphxRenderTarget *phxInternalRenderTarget;
  phxInternalRenderTarget = &InternalRenderTarget.rt;
  InternalRenderTarget.rt.RTView = Collection->Internal->GetRenderTargetView();
  InternalRenderTarget.rt.View = Collection->Internal->GetShaderResourceView();
  InternalRenderTarget.rt.Texture = Collection->Internal->GetTextureHandle();
  InternalRenderTarget.rt.XRes = Collection->Internal->GetXRes();
  InternalRenderTarget.rt.YRes = Collection->Internal->GetYRes();

  for ( TS32 x = 0; x < Collection->RenderTargets.NumItems(); x++ )
  {
    RenderTargets[ x ]->rt.RTView = Collection->RenderTargets[ x ]->GetRenderTargetView();
    RenderTargets[ x ]->rt.View = Collection->RenderTargets[ x ]->GetShaderResourceView();
    RenderTargets[ x ]->rt.Texture = Collection->RenderTargets[ x ]->GetTextureHandle();
    RenderTargets[ x ]->rt.XRes = Collection->RenderTargets[ x ]->GetXRes();
    RenderTargets[ x ]->rt.YRes = Collection->RenderTargets[ x ]->GetYRes();
  }

  SAFEDELETE( Timeline->Timeline->RenderTargets );
  Timeline->Timeline->RenderTargetCount = RenderTargets.NumItems();

  if ( Collection->RenderTargets.NumItems() )
  {
    Timeline->Timeline->RenderTargets = new CphxRenderTarget*[ Timeline->Timeline->RenderTargetCount ];
    for ( TS32 x = 0; x < Timeline->Timeline->RenderTargetCount; x++ )
      Timeline->Timeline->RenderTargets[ x ] = &RenderTargets[ x ]->rt;
  }

  for ( TS32 x = 0; x < Timeline->Events.NumItems(); x++ )
  {
    CphxRenderTarget_Tool *t = Project.GetRenderTarget( Timeline->Events[ x ]->TargetID );
    Timeline->Events[ x ]->Event->Target = NULL;
    if ( t ) Timeline->Events[ x ]->Event->Target = &t->rt;
  }

  //for (TS32 x = 0; x < RenderLayers.NumItems(); x++)
  //{
  //	for (TS32 y = 0; y < RenderLayers[x]->RenderTargets.NumItems(); y++)
  //	{
  //		RenderLayers[x]->RenderLayer.Targets[y] = &RenderLayers[x]->RenderTargets[y]->rt;
  //	}
  //}

  phxDepthBufferView = Collection->DepthBuffer->GetDepthView();
  phxDepthBufferShaderView = Collection->DepthBuffer->GetShaderResourceView();
}

//void CapexProject::ChangeDemoResolution(TS32 XRes, TS32 YRes, CCoreDevice *dev)
//{
//	TS32 odx = DemoResolutionX;
//	TS32 ody = DemoResolutionY;
//
//	TS32 tax = Project.Timeline->Timeline->AspectX;
//	TS32 tay = Project.Timeline->Timeline->AspectY;
//
//	TS32 max = MonitorAspectX;
//	TS32 may = MonitorAspectY;
//
//	TS32 sax = GetSystemMetrics(SM_CXSCREEN);
//	TS32 say = GetSystemMetrics(SM_CYSCREEN);
//
//	TF32 aspect = (tax*max*say) / (TF32)(tay*may*sax);
//	//TF32 aspect=tax/(TF32)tay*(max/(TF32)sax)/(may/(TF32)say);
//
//	DemoResolutionX = XRes;
//	DemoResolutionY = YRes;
//
//	if (aspect > XRes / (TF32)YRes)
//		DemoResolutionY = (TS32)(DemoResolutionX / aspect);
//	else DemoResolutionX = (TS32)(DemoResolutionY*aspect);
//
//	if (DemoResolutionX == odx && DemoResolutionY == ody) return; //no change
//
//	for (TS32 x = 0; x < RenderTargets.NumItems(); x++)
//		if (RenderTargets[x]->ResolutionDescriptor & 0x80) //demo relative resolution
//			RenderTargets[x]->Reallocate(dev);
//
//	SAFEDELETE(DepthBuffer);
//	DepthBuffer = new CCoreDX11Texture2D((CCoreDX11Device*)dev);
//	DepthBuffer->CreateDepthBuffer(DemoResolutionX, DemoResolutionY);
//
//	ApplyRenderTargets();
//}
//
//void CapexProject::ForceDemoResolution(TS32 XRes, TS32 YRes, CCoreDevice *dev)
//{
//	TS32 odx = DemoResolutionX;
//	TS32 ody = DemoResolutionY;
//
//	DemoResolutionX = XRes;
//	DemoResolutionY = YRes;
//
//	if (DemoResolutionX == odx && DemoResolutionY == ody) return; //no change
//
//	for (TS32 x = 0; x < RenderTargets.NumItems(); x++)
//		if (RenderTargets[x]->ResolutionDescriptor & 0x80) //demo relative resolution
//			RenderTargets[x]->Reallocate(dev);
//
//	SAFEDELETE(DepthBuffer);
//	DepthBuffer = new CCoreDX11Texture2D((CCoreDX11Device*)dev);
//	DepthBuffer->CreateDepthBuffer(DemoResolutionX, DemoResolutionY);
//
//	ApplyRenderTargets();
//}

CphxRenderTarget_Tool * CapexProject::GetRenderTarget( CphxGUID &ID )
{
  for ( TS32 x = 0; x < RenderTargets.NumItems(); x++ )
    if ( RenderTargets[ x ]->GetGUID() == ID ) return RenderTargets[ x ];
  return NULL;
}

TS32 CapexProject::GetRenderTargetIndex( CphxGUID &ID )
{
  for ( TS32 x = 0; x < RenderTargets.NumItems(); x++ )
    if ( RenderTargets[ x ]->GetGUID() == ID ) return x;
  return -1;
}

CphxMaterialTechnique_Tool * CapexProject::GetTech( CphxGUID &ID )
{
  for ( TS32 x = 0; x < Techniques.NumItems(); x++ )
    if ( Techniques[ x ]->GetGUID() == ID ) return Techniques[ x ];

  if ( ID == DefaultTech->GetGUID() ) return DefaultTech;

  return NULL;
}

CphxMaterialParameter_Tool * CapexProject::GetMaterialParameter( CphxGUID &ID )
{
  for ( TS32 x = 0; x < Techniques.NumItems(); x++ )
  {
    for ( TS32 y = 0; y < Techniques[ x ]->TechParameters.Parameters.NumItems(); y++ )
      if ( Techniques[ x ]->TechParameters.Parameters[ y ]->GetGUID() == ID )
        return Techniques[ x ]->TechParameters.Parameters[ y ];

    for ( TS32 y = 0; y < Techniques[ x ]->Passes.NumItems(); y++ )
      for ( TS32 z = 0; z < Techniques[ x ]->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ )
        if ( Techniques[ x ]->Passes[ y ]->PassParameters.Parameters[ z ]->GetGUID() == ID )
          return Techniques[ x ]->Passes[ y ]->PassParameters.Parameters[ z ];
  }

  return NULL;
}

TS32 CapexProject::GetFrameToRender()
{
  if ( !MusicData.TimelineDragged )
  {
    if ( MusicData.Playing )
      if ( Song )
        return max( 0, (TS32)( Song->GetPlayPosition() / 1000.0f*Project.Timeline->Timeline->FrameRate ) );
      else
        return max( 0, (TS32)( ( globalTimer.GetTime() - MusicData.PlaybackStart ) / 1000.0f*Project.Timeline->Timeline->FrameRate ) );
  }

  return MusicData.CurrentFrame;
}

TBOOL __fastcall MvxProgress( void * pUserData, TU32 nCurrentProgress, TU32 nTotalProgress )
{
  Project.MusicData.MVXRenderStatus = nCurrentProgress / (TF32)nTotalProgress;

  if ( nCurrentProgress == nTotalProgress )
  {
    Project.MusicData.Playing = false;
    LOG_NFO( "[timeline] Music successfully rendered and loaded." );
    Project.AcquireTickData();
  }

  return true;
}

void CapexProject::LoadSong( CStreamReader &r, HWND AppHandle )
{
  if ( Song )
    Song->Stop();
  SAFEDELETE( Song );
  Song = new CphxSound();
  if ( !Song->Load( r, MvxProgress, NULL ) )
  {
    LOG_ERR( "[mvx] Failed to load MVX song" );
    SAFEDELETE( Song );
    return;
  }

}

CString CapexProject::GetSongType()
{
  return Song ? Song->GetFileTypeMacro() : CString();
}

CString CapexProject::GetSynthConfig()
{
  return Song ? Song->GetSynthConfig() : CString();
}

void CapexProject::TogglePlaying()
{
  if ( MusicData.Playing )
  {
    UpdateCurrentFrameFromPlayback();

    if ( Song )
      Song->Stop();

    MusicData.Playing = false;
    return;
  }

  if ( Song )
  {
    LOG_DBG( "[mvx] Starting playback at %d", (TS32)( MusicData.CurrentFrame / (TF32)Timeline->Timeline->FrameRate*1000.0f ) );
    Song->Play( (TS32)( MusicData.CurrentFrame / (TF32)Timeline->Timeline->FrameRate*1000.0f ) );
  }
  else
    MusicData.PlaybackStart = globalTimer.GetTime() - (TS32)( MusicData.CurrentFrame / (TF32)Timeline->Timeline->FrameRate*1000.0f );

  MusicData.Playing = true;
  Timeline->BeatMarkerPlaced = false;

  for ( TS32 x = 0; x < Scenes.NumItems(); x++ )
  {
    Scenes[ x ]->SwapParticleBuffers();
    Scenes[ x ]->ResetParticles();
    Scenes[ x ]->SwapParticleBuffers();
    Scenes[ x ]->ResetParticles();
  }
}

void CapexProject::UpdateCurrentFrameFromPlayback()
{
  if ( !MusicData.Playing ) return;

  if ( Song )
    MusicData.CurrentFrame = max( 0, (TS32)( Song->GetPlayPosition() / 1000.0f*Timeline->Timeline->FrameRate ) );
  else
    MusicData.CurrentFrame = max( 0, (TS32)( ( globalTimer.GetTime() - MusicData.PlaybackStart ) / 1000.0f*Timeline->Timeline->FrameRate ) );

}

void CapexProject::StopPlayback()
{
  if ( !MusicData.Playing ) return;
  TogglePlaying();
}

void CapexProject::SeekToTime( TS32 timepos )
{
  SeekToFrame( max( 0, (TS32)( timepos / 1000.0f*Timeline->Timeline->FrameRate ) ) );
}

void CapexProject::SeekToFrame( TS32 timepos )
{
  if ( !MusicData.Playing )
  {
    MusicData.CurrentFrame = max( 0, timepos );
    return;
  }

  TogglePlaying();
  MusicData.CurrentFrame = max( 0, timepos );
  TogglePlaying();
}

TS32 CapexProject::GetTimepos()
{
  UpdateCurrentFrameFromPlayback();
  return (TS32)( MusicData.CurrentFrame / (TF32)Timeline->Timeline->FrameRate*1000.0f );
}

void CapexProject::AcquireTickData()
{
  MusicData.TickData.Flush();
  MusicData.OfflineLength = 0;

  if ( !Song ) return;

  MusicData.OfflineLength = Song->GetSongLength();

  Song->GetMarkers( MusicData.TickData );
}

TBOOL CapexProject::CanLoadSong()
{
  if ( !Song ) return true;
  return Song->IsLoaded();
}

CphxEvent_Tool * CapexProject::GetEvent( CphxGUID &ID )
{
  if ( !Timeline ) return NULL;

  for ( TS32 x = 0; x < Timeline->Events.NumItems(); x++ )
    if ( Timeline->Events[ x ]->GetGUID() == ID ) return Timeline->Events[ x ];

  return NULL;
}

CphxModel_Tool * CapexProject::CreateModel()
{
  CphxModel_Tool *m = new CphxModel_Tool();
  Models += m;
  return m;
}

CphxModel_Tool * CapexProject::GetModel( CphxGUID &ID )
{
  for ( TS32 x = 0; x < Models.NumItems(); x++ )
    if ( Models[ x ]->GetGUID() == ID ) return Models[ x ];
  return NULL;
}

CphxMaterial_Tool * CapexProject::CreateMaterial()
{
  CphxMaterial_Tool *m = new CphxMaterial_Tool();
  Materials += m;
  return m;
}

CphxMaterial_Tool * CapexProject::GetMaterial( CphxGUID &ID )
{
  for ( TS32 x = 0; x < Materials.NumItems(); x++ )
    if ( Materials[ x ]->GetGUID() == ID ) return Materials[ x ];

  if ( ID == DefaultMaterial->GetGUID() ) return DefaultMaterial;

  return NULL;
}

CphxRenderLayerDescriptor_Tool * CapexProject::CreateRenderLayer()
{
  CphxRenderLayerDescriptor_Tool *m = new CphxRenderLayerDescriptor_Tool();
  RenderLayers += m;
  return m;
}

CphxRenderLayerDescriptor_Tool * CapexProject::GetRenderLayer( CphxGUID &ID )
{
  for ( TS32 x = 0; x < RenderLayers.NumItems(); x++ )
    if ( RenderLayers[ x ]->GetGUID() == ID ) return RenderLayers[ x ];

  //if (ID == DefaultMaterial->GetGUID()) return DefaultMaterial;

  return NULL;
}

//ID3D11DepthStencilView * CapexProject::GetDepthBuffer()
//{
//	return DepthBuffer->GetDepthView();
//}

CphxModelObject_Tool * CapexProject::GetModelObject( CphxGUID &guid )
{
  for ( TS32 x = 0; x < Models.NumItems(); x++ )
    for ( TS32 y = 0; y < Models[ x ]->GetObjectCount(); y++ )
      if ( Models[ x ]->GetObjectByIndex( y )->GetGUID() == guid ) return Models[ x ]->GetObjectByIndex( y );
  return NULL;
}

CapexRenderTargetCollection * CapexProject::SpawnRenderTargetCollection( CCoreDevice *Device, TS32 XRes, TS32 YRes, TBOOL AspectCorrect )
{
  CapexRenderTargetCollection *rtc = new CapexRenderTargetCollection( Device );
  SpawnedCollections += rtc;
  rtc->UpdateCollection( XRes, YRes, AspectCorrect );
  return rtc;
}

void CapexProject::DeleteRenderTargetCollection( CapexRenderTargetCollection *rtc )
{
  SpawnedCollections.Delete( rtc );
}

void CapexProject::UpdateRTCollections()
{
  for ( TS32 x = 0; x < SpawnedCollections.NumItems(); x++ )
    SpawnedCollections[ x ]->ReallocateCollection();
}

ID3D11ShaderResourceView * CapexProject::GetTextureView( CphxGUID &Guid )
{
  CphxTextureOperator_Tool *t = GetTexgenOp( Guid );
  if ( t && t->GetContentOp() && t->GetContentOp()->Result ) return t->GetContentOp()->Result->View;
  CphxRenderTarget_Tool *r = GetRenderTarget( Guid );
  if ( !r ) return NULL;
  return r->rt.View;
}

CphxResource * CapexProject::GetResource( CphxGUID &g )
{
  CphxResource *s = NULL;
  s = GetTextureFilter( g ); if ( s ) return s;
  s = GetRenderTarget( g ); if ( s ) return s;
  s = GetTech( g ); if ( s ) return s;
  s = GetMaterial( g ); if ( s ) return s;
  s = GetRenderLayer( g ); if ( s ) return s;
  s = GetTexgenOp( g ); if ( s ) return s;
  s = GetModel( g ); if ( s ) return s;
  s = GetModelObject( g ); if ( s ) return s;

  return s;
}

CphxScene_Tool * CapexProject::CreateScene()
{
  CphxScene_Tool *m = new CphxScene_Tool();
  Scenes += m;
  return m;
}

CphxScene_Tool * CapexProject::GetScene( CphxGUID &ID )
{
  for ( TS32 x = 0; x < Scenes.NumItems(); x++ )
    if ( Scenes[ x ]->GetGUID() == ID ) return Scenes[ x ];
  return NULL;
}

CphxModel_Tool * CapexProject::GetModelByName( CString Name )
{
  Name.ToLower();
  for ( TS32 x = 0; x < Models.NumItems(); x++ )
  {
    CString s = Models[ x ]->GetName();
    s.ToLower();
    if ( s == Name ) return Models[ x ];
  }
  return NULL;
}

CphxScene_Tool * CapexProject::GetSceneByName( CString Name )
{
  Name.ToLower();
  for ( TS32 x = 0; x < Scenes.NumItems(); x++ )
  {
    CString s = Scenes[ x ]->GetName();
    s.ToLower();
    if ( s == Name ) return Scenes[ x ];
  }
  return NULL;
}

TS32 CapexProject::GetSceneIndex( CphxGUID &ID )
{
  for ( TS32 x = 0; x < Scenes.NumItems(); x++ )
    if ( Scenes[ x ]->GetGUID() == ID ) return x;
  return -1;
}

float CapexProject::GetAspect()
{
  return Timeline->Timeline->AspectX / (float)Timeline->Timeline->AspectY;
}

TBOOL CapexProject::DeleteModel( CphxGUID &ID )
{
  for ( int x = 0; x < Models.NumItems(); x++ )
    if ( Models[ x ]->GetGUID() == ID )
    {
      Models.FreeByIndex( x );
      return true;
    }

  return false;
}

TBOOL CapexProject::DeleteScene( CphxGUID &ID )
{
  for ( int x = 0; x < Scenes.NumItems(); x++ )
    if ( Scenes[ x ]->GetGUID() == ID )
    {
      Scenes.FreeByIndex( x );
      return true;
    }

  return false;
}

TS32 TreeSpeciesNameSorter( CphxTreeSpecies **a, CphxTreeSpecies **b )
{
  return CString::CompareNoCase( ( *a )->Name, ( *b )->Name );
}

void CapexProject::ExportTreeSpecies( CXMLNode *RootNode )
{
  if ( !RootNode ) return;
  for ( TS32 x = 0; x < TreeSpecies.NumItems(); x++ )
  {
    CXMLNode n = RootNode->AddChild( _T( "treespecies" ) );
    TreeSpecies[ x ]->Export( &n );
  }
}

void CapexProject::ImportTreeSpecies( CXMLNode *RootNode, TBOOL External )
{
  if ( !RootNode ) return;

  for ( TS32 x = 0; x < RootNode->GetChildCount( _T( "treespecies" ) ); x++ )
  {
    CphxTreeSpecies *f = new CphxTreeSpecies();
    f->Import( &RootNode->GetChild( _T( "treespecies" ), x ) );
    f->External = External;

    if ( GetTreeSpecies( f->GetGUID() ) )
      delete f;
    else
      TreeSpecies += f;
  }

  TreeSpecies.Sort( TreeSpeciesNameSorter );
}

CphxTreeSpecies * CapexProject::CreateTreeSpecies()
{
  CphxTreeSpecies *m = new CphxTreeSpecies();
  TreeSpecies += m;
  return m;
}

CphxTreeSpecies * CapexProject::GetTreeSpecies( CphxGUID &ID )
{
  for ( TS32 x = 0; x < TreeSpecies.NumItems(); x++ )
    if ( TreeSpecies[ x ]->GetGUID() == ID ) return TreeSpecies[ x ];
  return NULL;
}

CphxTreeSpecies * CapexProject::GetTreeSpeciesByName( CString Name )
{
  Name.ToLower();
  for ( TS32 x = 0; x < TreeSpecies.NumItems(); x++ )
  {
    CString s = TreeSpecies[ x ]->GetName();
    s.ToLower();
    if ( s == Name ) return TreeSpecies[ x ];
  }
  return NULL;
}

TS32 CapexProject::GetTreeSpeciesIndex( CphxGUID &ID )
{
  for ( TS32 x = 0; x < TreeSpecies.NumItems(); x++ )
    if ( TreeSpecies[ x ]->GetGUID() == ID ) return x;
  return -1;
}

TBOOL CapexProject::DeleteTreeSpecies( CphxGUID &ID )
{
  for ( int x = 0; x < TreeSpecies.NumItems(); x++ )
    if ( TreeSpecies[ x ]->GetGUID() == ID )
    {
      TreeSpecies.FreeByIndex( x );
      return true;
    }

  return false;
}

void CapexProject::ResetParticles()
{
  for ( TS32 x = 0; x < Scenes.NumItems(); x++ )
    Scenes[ x ]->ResetParticles();
  for ( TS32 x = 0; x < Timeline->Events.NumItems(); x++ )
    Timeline->Events[ x ]->Event->OnScreenLastFrame = false;
}

CapexRenderTargetCollection::CapexRenderTargetCollection( CCoreDevice *Device )
{
  Dev = Device;
  DepthBuffer = NULL;
  Internal = NULL;
  XRes = YRes = 1;
}

CapexRenderTargetCollection::~CapexRenderTargetCollection()
{
  Project.DeleteRenderTargetCollection( this );
  SAFEDELETE( DepthBuffer );
  RenderTargets.FreeArray();
  SAFEDELETE( Internal );
}

void CapexRenderTargetCollection::UpdateCollection( TS32 xres, TS32 yres, TBOOL AspectCorrect )
{
  LastAspectCorrect = AspectCorrect;
  TBOOL First = !DepthBuffer;

  TS32 odx = XRes;
  TS32 ody = YRes;

  TS32 tax = Project.Timeline->Timeline->AspectX;
  TS32 tay = Project.Timeline->Timeline->AspectY;

  TS32 max = Config::MonitorAspectX;
  TS32 may = Config::MonitorAspectY;

  TS32 sax = GetSystemMetrics( SM_CXSCREEN );
  TS32 say = GetSystemMetrics( SM_CYSCREEN );

  TF32 aspect = ( tax*max*say ) / (TF32)( tay*may*sax );

  XRes = xres;
  YRes = yres;

  if ( AspectCorrect )
  {
    if ( aspect > xres / (TF32)yres )	YRes = (TS32)( XRes / aspect );
    else XRes = (TS32)( YRes*aspect );
  }

  if ( !First && ( XRes == odx && YRes == ody ) ) return; //no change

  SAFEDELETE( Internal );
  SAFEDELETE( DepthBuffer );
  RenderTargets.FreeArray();

  for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
  {
    CphxRenderTarget_Tool *rt = Project.GetRenderTargetByIndex( x );
    TS32 rtx = 1, rty = 1;
    GetRenderTargetResolution( rt->ResolutionDescriptor, rtx, rty, XRes, YRes );
    CCoreDX11Texture2D *t = new CCoreDX11Texture2D( (CCoreDX11Device*)Dev );
    t->Create( rtx, rty, NULL, 8, pixelFormats[ rt->pixelFormat ], true );
    RenderTargets += t;
  }

  DepthBuffer = new CCoreDX11Texture2D( (CCoreDX11Device*)Dev );
  DepthBuffer->CreateDepthBuffer( XRes, YRes );

  Internal = new CCoreDX11Texture2D( (CCoreDX11Device*)Dev );
  Internal->Create( XRes, YRes, NULL, 8, COREFMT_R16G16B16A16_FLOAT, true );
}

void CapexRenderTargetCollection::ReallocateCollection()
{
  SAFEDELETE( Internal );
  SAFEDELETE( DepthBuffer );
  RenderTargets.FreeArray();
  UpdateCollection( XRes, YRes, LastAspectCorrect );
}

CCoreDX11Texture2D * CapexRenderTargetCollection::GetRenderTarget( CphxRenderTarget *rt )
{
  for ( TS32 x = 0; x < RenderTargets.NumItems(); x++ )
    if ( RenderTargets[ x ]->GetRenderTargetView() == rt->RTView ) return RenderTargets[ x ];
  return NULL;
}
