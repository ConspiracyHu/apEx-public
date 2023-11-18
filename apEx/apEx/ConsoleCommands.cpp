#include "BasePCH.h"
#include "apExRoot.h"

extern CapexRoot *Root;
bool pickingEnabled = true;

void Console_List( CStringArray &Commandline )
{
  Root->ListCommands();
}

void Console_DumpAtlas( CStringArray &Commandline )
{
  CString FileName = _T( "atlas.png" );
  if ( Commandline.NumItems() > 1 ) FileName = Commandline[ 1 ];

  CWBApplication *App = Root->GetApplication();
  CAtlas *Atlas = App->GetAtlas();
  CCoreTexture2D *Texture = Atlas->GetTexture();

  Texture->ExportToImage( FileName, false, CORE_PNG, false );

  LOG_NFO( "Atlas has been dumped to %s", FileName.GetPointer() );
}

void Console_OptimizeAtlas( CStringArray &Commandline )
{
  CWBApplication *App = Root->GetApplication();
  CAtlas *Atlas = App->GetAtlas();
  Atlas->Optimize( Commandline.NumItems() == 1 );

  LOG_NFO( "Atlas has been optimized (Debugmode: %d)", Commandline.NumItems() == 1 );
}

void Console_ResizeAtlas( CStringArray &Commandline )
{
  if ( Commandline.NumItems() < 3 )
  {
    LOG_NFO( "Usage: AtlasResize <width> <height>" );
    return;
  }

  TS32 xr, yr;
  if ( Commandline[ 1 ].Scan( _T( "%d" ), &xr ) != 1 || Commandline[ 2 ].Scan( _T( "%d" ), &yr ) != 1 || xr <= 0 || yr <= 0 )
  {
    LOG_NFO( "Invalid numeric input for atlas resolution" );
    return;
  }

  CWBApplication *App = Root->GetApplication();
  CAtlas *Atlas = App->GetAtlas();

  Atlas->Resize( App->GetDevice(), xr, yr );

  LOG_NFO( "Atlas has been resized to %dx%d", xr, yr );
}

void Console_TexgenMemPool( CStringArray &Commandline )
{
  extern TS32 TexgenMemPoolSize;
  if ( Commandline.NumItems() < 2 )
  {
    LOG_NFO( "Texgen Memory Pool size is %d MB", TexgenMemPoolSize );
    LOG_NFO( "Use TexgenMemPool <size in MB> to set" );
  }

  TS32 size;
  if ( Commandline[ 1 ].Scan( _T( "%d" ), &size ) != 1 || size <= 0 )
  {
    LOG_NFO( "Invalid numeric input for texture memory pool size" );
    return;
  }

  TexgenMemPoolSize = size;

  LOG_NFO( "Texgen Memory Pool has been resized to %d MB", TexgenMemPoolSize );
}

#include "ExternalTools.h"
#include "HLSLParser.h"

TBOOL MinifyShader_CNS( char *Data, CString &Result, TBOOL Verbose, TBOOL ResetDictionary, TBOOL RebuildDictionary );

TS32 ShaderID = 0;

#include <d3dcompiler.h>

typedef HRESULT( __stdcall d3dstripdhaderfunc )( _In_reads_bytes_( BytecodeLength ) LPCVOID pShaderBytecode,
                                                 _In_ SIZE_T BytecodeLength,
                                                 _In_ UINT uStripFlags,
                                                 _Out_ ID3DBlob** ppStrippedBlob );
d3dstripdhaderfunc *D3DStripCall = NULL;

void *GetFunctionFromD3DCompileDLL( TCHAR *FunctName );

CStreamWriterMemory *BlobOutput = NULL;
CStreamWriterMemory *BlobOutputText = NULL;

#pragma comment(lib,"d3dcompiler.lib")

void DumpShader( CCoreShader *Shader )
{
  if ( !Shader ) return;

  if ( !D3DStripCall )
    D3DStripCall = (d3dstripdhaderfunc *)GetFunctionFromD3DCompileDLL( "D3DStripShader" );

  if ( !D3DStripCall )
  {
    LOG_ERR( "[apex] Failed to load D3DStripShader()" );
    return;
  }

  ID3DBlob *Blob = NULL;

  D3DStripCall( Shader->GetBinary(), Shader->GetBinaryLength(), D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS | D3DCOMPILER_STRIP_PRIVATE_DATA, &Blob );

  if ( !Blob )
  {
    LOG_ERR( "[apex] Shader strip failed" );
    return;
  }

  CreateDirectory( "Data\\ShaderDump", NULL );

  ID3DBlob *disasmBlob = NULL;
  D3DDisassemble( Blob->GetBufferPointer(), Blob->GetBufferSize(), 0, NULL, &disasmBlob );

  CString f = CString::Format( "Data\\ShaderDump\\%.3d.dis", ShaderID );
  CStreamWriterFile o;
  if ( o.Open( f.GetPointer() ) )
    o.Write( disasmBlob->GetBufferPointer(), disasmBlob->GetBufferSize() );

  BlobOutput->Write( Blob->GetBufferPointer(), Blob->GetBufferSize() );

  Blob->Release();

  ShaderID++;
}

void DoMinifyPass( TBOOL DictionaryBuildPass, TBOOL Apply, TBOOL OnlyApply, TS32 &AllCodeSize, TS32 &ResultCodeSize, TBOOL DumpFile )
{
  if ( DumpFile )
  {
    BlobOutput = new CStreamWriterMemory();
    //BlobOutput->Open("Data\\ShaderDump\\master.bin");
    BlobOutputText = new CStreamWriterMemory();
    //BlobOutputText->Open("Data\\ShaderDump\\master.txt");
  }

  ShaderID = 0;
  CWBApplication *App = Root->GetApplication();

  TBOOL ResetDic = OnlyApply;
  TBOOL BuildDic = true;
  if ( !OnlyApply ) BuildDic = DictionaryBuildPass;

  if ( DictionaryBuildPass )
    CrossCompiler::Parser::NumStats.Flush();

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    for ( TS32 y = 0; y < Project.GetTechByIndex( x )->Passes.NumItems(); y++ )
    {
      if ( !Project.GetTechByIndex( x )->Passes[ y ]->Minifiable )
      {
        LOG_WARN( "Material Tech %d/%d '%s' Pass %d/%d marked as not minifiable - skipping.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
        continue;
      }

      CString Code = Project.GetTechByIndex( x )->Passes[ y ]->ShaderCode;

      CString Result;
      if ( !MinifyShader_CNS( Code.GetPointer(), Result, false, ResetDic, BuildDic ) )
      {
        LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
        continue;
      }

      CString ps, vs, gs, hs, ds;
      CCorePixelShader *PS = App->GetDevice()->CreatePixelShader( Result.GetPointer(), Result.Length(), "p", "ps_5_0", &ps );
      CCoreVertexShader *VS = App->GetDevice()->CreateVertexShader( Result.GetPointer(), Result.Length(), "v", "vs_5_0", &vs );
      CCoreGeometryShader *GS = App->GetDevice()->CreateGeometryShader( Result.GetPointer(), Result.Length(), "g", "gs_5_0", &gs );
      CCoreHullShader *HS = App->GetDevice()->CreateHullShader( Result.GetPointer(), Result.Length(), "h", "hs_5_0", &hs );
      CCoreDomainShader *DS = App->GetDevice()->CreateDomainShader( Result.GetPointer(), Result.Length(), "d", "ds_5_0", &ds );

      if ( DumpFile )
      {
        DumpShader( PS );
        DumpShader( VS );
        DumpShader( GS );
        DumpShader( HS );
        DumpShader( DS );
        BlobOutputText->WriteASCIIZ( Result );
      }

      TBOOL ep, ev, eg, eh, ed;
      ep = ps.Find( _T( "error X3501" ) ) < 0;
      ev = vs.Find( _T( "error X3501" ) ) < 0;
      eg = gs.Find( _T( "error X3501" ) ) < 0;
      eh = hs.Find( _T( "error X3501" ) ) < 0;
      ed = ds.Find( _T( "error X3501" ) ) < 0;

      switch ( Project.GetTechByIndex( x )->Tech->Type )
      {
      case TECH_MATERIAL:
        if ( PS && VS && ( GS || !eg ) && ( HS || !eh ) && ( DS || !ed ) )
        {
          LOG_DBG( "Material Tech %d/%d '%s' Pass %d/%d: OK. - %d%%", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems(), (TS32)( Result.Length() / (TF32)Code.Length() * 100 ) );
          AllCodeSize += Code.Length();
          ResultCodeSize += Result.Length();
          if ( Apply )
          {
            Project.GetTechByIndex( x )->Passes[ y ]->ShaderCode = Result;
            Project.GetTechByIndex( x )->InvalidateUptoDateFlag();
          }
        }
        else
        {
          if ( !PS ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Pixel Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !VS ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Vertex Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !GS && eg ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Geometry Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !HS && eh ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Hull Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !DS && ed ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Domain Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
        }
        break;
      case TECH_PARTICLE:
        if ( PS && VS && GS && ( HS || !eh ) && ( DS || !ed ) )
        {
          LOG_DBG( "Material Tech %d/%d '%s' Pass %d/%d: OK. - %d%%", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems(), (TS32)( Result.Length() / (TF32)Code.Length() * 100 ) );
          AllCodeSize += Code.Length();
          ResultCodeSize += Result.Length();
          if ( Apply )
          {
            Project.GetTechByIndex( x )->Passes[ y ]->ShaderCode = Result;
            Project.GetTechByIndex( x )->InvalidateUptoDateFlag();
          }
        }
        else
        {
          if ( !PS ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Pixel Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !VS ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Vertex Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !GS ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Geometry Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !HS && eh ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Hull Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !DS && ed ) LOG_ERR( "Material Tech %d/%d '%s' Pass %d/%d: Domain Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
        }
        break;
      default:
        if ( PS && ( VS || !ev ) && ( GS || !eg ) && ( HS || !eh ) && ( DS || !ed ) )
        {
          LOG_DBG( "PostProc Tech %d/%d '%s' Pass %d/%d: OK. - %d%%", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems(), (TS32)( Result.Length() / (TF32)Code.Length() * 100 ) );
          AllCodeSize += Code.Length();
          ResultCodeSize += Result.Length();
          if ( Apply )
          {
            Project.GetTechByIndex( x )->Passes[ y ]->ShaderCode = Result;
            Project.GetTechByIndex( x )->InvalidateUptoDateFlag();
          }
        }
        else
        {
          if ( !PS ) LOG_ERR( "PostProc Tech %d/%d '%s' Pass %d/%d: Pixel Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !VS && ev ) LOG_ERR( "PostProc Tech %d/%d '%s' Pass %d/%d: Vertex Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !GS && eg ) LOG_ERR( "PostProc Tech %d/%d '%s' Pass %d/%d: Geometry Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !HS && eh ) LOG_ERR( "PostProc Tech %d/%d '%s' Pass %d/%d: Hull Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
          if ( !DS && ed ) LOG_ERR( "PostProc Tech %d/%d '%s' Pass %d/%d: Domain Shader FAILED.", x + 1, Project.GetTechCount(), Project.GetTechByIndex( x )->Name.GetPointer(), y + 1, Project.GetTechByIndex( x )->Passes.NumItems() );
        }
        break;
      }

      SAFEDELETE( PS );
      SAFEDELETE( VS );
      SAFEDELETE( GS );
      SAFEDELETE( HS );
      SAFEDELETE( DS );

      App->HandleOSMessages();
      App->Display();
    }
  }

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    if ( !Project.GetTextureFilterByIndex( x )->Minifiable )
    {
      LOG_WARN( "Texture Filter %d/%d '%s' marked as not minifiable - skipping.", x + 1, Project.GetTextureFilterCount(), Project.GetTextureFilterByIndex( x )->Name.GetPointer() );
      continue;
    }

    CString Code = Project.GetTextureFilterByIndex( x )->Code;
    CString Result;
    if ( !MinifyShader_CNS( Code.GetPointer(), Result, false, ResetDic, BuildDic ) )
    {
      LOG_ERR( "Texture Filter %d/%d '%s': FAILED.", x + 1, Project.GetTextureFilterCount(), Project.GetTextureFilterByIndex( x )->Name.GetPointer() );
      continue;
    }

    CCorePixelShader *Shader = App->GetDevice()->CreatePixelShader( Result.GetPointer(), Result.Length(), "p", "ps_5_0" );

    if ( DumpFile )
    {
      DumpShader( Shader );
      BlobOutputText->WriteASCIIZ( Result );
    }

    if ( Shader )
    {
      LOG_DBG( "Texture Filter %d/%d '%s': OK. - %d%%", x + 1, Project.GetTextureFilterCount(), Project.GetTextureFilterByIndex( x )->Name.GetPointer(), (TS32)( Result.Length() / (TF32)Code.Length() * 100 ) );
      AllCodeSize += Code.Length();
      ResultCodeSize += Result.Length();
      if ( Apply )
      {
        Project.GetTextureFilterByIndex( x )->Code = Result;
        Project.GetTextureFilterByIndex( x )->InvalidateUptoDateFlag();
      }
    }
    else
      LOG_ERR( "Texture Filter %d/%d '%s': FAILED.", x + 1, Project.GetTextureFilterCount(), Project.GetTextureFilterByIndex( x )->Name.GetPointer() );

    SAFEDELETE( Shader );

    App->HandleOSMessages();
    App->Display();
  }

  if ( DumpFile )
  {
    CStreamWriterFile o1;
    o1.Open( "Data\\ShaderDump\\master.bin" );

    if ( BlobOutput->GetLength() % 4 == 0 )
    {
      TU8 *deinterleaved = new TU8[ BlobOutput->GetLength() ];
      for ( TU32 x = 0; x < BlobOutput->GetLength(); x++ )
        deinterleaved[ ( x / 4 )*( ( x % 4 ) + 1 ) ] = BlobOutput->GetData()[ x ];

      o1.Write( deinterleaved, BlobOutput->GetLength() );
    }

    CStreamWriterFile o2;
    o2.Open( "Data\\ShaderDump\\master.txt" );
    o2.Write( BlobOutput->GetData(), BlobOutput->GetLength() );

    SAFEDELETE( BlobOutput );
    SAFEDELETE( BlobOutputText );
  }
}

void Console_MinifyShaderTest( CStringArray &CommandLine )
{
  TBOOL Apply = CommandLine.NumItems() > 1;
  LOG_NFO( "Testing shaders. This is gonna take a while, sit back..." );

  TS32 AllCodeSize = 0;
  TS32 ResultCodeSize = 0;

  if ( Apply )
  {
    DoMinifyPass( true, false, false, AllCodeSize, ResultCodeSize, false );
    AllCodeSize = 0;
    ResultCodeSize = 0;
    DoMinifyPass( false, true, false, AllCodeSize, ResultCodeSize, false );
  }
  else
  {
    DoMinifyPass( false, false, true, AllCodeSize, ResultCodeSize, false );
  }

  if ( AllCodeSize )
    LOG_DBG( "Global minification ratio: %d%%", (TS32)( ResultCodeSize / (TF32)AllCodeSize * 100 ) );

  if ( !Apply )
    LOG_DBG( "Use with 'ShaderTest <any parameter here>' to apply results to shaders." );
}

void Console_DumpShaderBinaries( CStringArray &CommandLine )
{
  TBOOL Apply = CommandLine.NumItems() > 1;

  TS32 AllCodeSize = 0;
  TS32 ResultCodeSize = 0;

  DoMinifyPass( false, false, true, AllCodeSize, ResultCodeSize, true );
}

void Console_TestShaderBlob( CStringArray &CommandLine )
{
  TBOOL Apply = CommandLine.NumItems() > 1;
  LOG_NFO( "Testing shader blob. This is gonna take a while, sit back..." );

  CArray<CString> shaders;

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
    for ( TS32 y = 0; y < Project.GetTechByIndex( x )->Passes.NumItems(); y++ )
      shaders += Project.GetTechByIndex( x )->Passes[ y ]->ShaderCode;

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
    shaders += Project.GetTextureFilterByIndex( x )->Code;

  CrossCompiler::Parser::BuildShaderBlob( shaders );
}


#ifdef MEMORY_TRACKING
void Console_MemoryDiagnostics( CStringArray &CommandLine )
{
  LOG_NFO( _T( "Allocated memory: %d bytes" ), memTracker.GetAllocatedMemorySize() );
}
#endif

void Console_ToggleShaderMinifier( CStringArray &CommandLine )
{
  Project.EnableShaderMinifier = !Project.EnableShaderMinifier;
  LOG_NFO( "Shader minifier is now %s", Project.EnableShaderMinifier ? "Enabled" : "Disabled" );
}

void Console_ToggleGlobalShaderMinifier( CStringArray &CommandLine )
{
  Project.EnableGlobalShaderMinifier = !Project.EnableGlobalShaderMinifier;
  LOG_NFO( "Global shader minifier is now %s", Project.EnableGlobalShaderMinifier ? "Enabled" : "Disabled" );
}

void Console_ToggleFarbrauschPrecalc( CStringArray &CommandLine )
{
  Project.EnableFarbrauschPrecalc = !Project.EnableFarbrauschPrecalc;
  LOG_NFO( "Farbrausch precalc export is now %s", Project.EnableFarbrauschPrecalc ? "Enabled" : "Disabled" );
}

void Console_ToggleMinimalPrecalc( CStringArray& CommandLine )
{
  Project.EnableMinimalPrecalc = !Project.EnableMinimalPrecalc;
  LOG_NFO( "Minimal precalc export is now %s", Project.EnableMinimalPrecalc ? "Enabled" : "Disabled" );
}

void Console_ToggleSetupHasSocial( CStringArray& CommandLine )
{
  Project.EnableSetupHasSocial = !Project.EnableSetupHasSocial;
  LOG_NFO( "Setup dialog with social network links is now %s", Project.EnableSetupHasSocial ? "Enabled" : "Disabled" );
}

void Console_DumpSubSceneReferencesForEditedClip( CStringArray& CommandLine )
{
  auto* s = GetActiveWorkBench()->GetEditedScene();
  if ( !s )
    return;
  int clipIdx = s->GetActiveClip();

  ClearRequiredFlagForAllResources();
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  int subSceneCount = s->GetChildCount( PHX_OBJECT );
  if ( subSceneCount )
  {
    for ( int y = 0; y < subSceneCount; y++ )
    {
      CphxObject_Tool* obj = (CphxObject_Tool*)s->GetChild( PHX_OBJECT, y );
      if ( !obj->IsRequired() )
        continue;
      if ( obj->GetObjectType() == Object_ParticleEmitterCPU && clipIdx == 0 )
      {
        LOG_NFO( "Scene: %s Obj: %s (ALL CLIPS)", obj->GetParentScene()->GetName().GetPointer(), obj->GetName().GetPointer() );
        continue;
      }

      if ( obj->GetObjectType() == Object_SubScene )
      {
        CphxObject_SubScene_Tool* sub = (CphxObject_SubScene_Tool*)obj;
        for ( int z = 0; z < sub->GetClipArray().GetClipCount(); z++ )
        {
          if ( sub->GetSubScene( z ) != s )
            continue;
          if ( !obj->GetParentScene()->GetClipByIndex( z )->IsRequired() && !sub->GetParentScene()->IsClipUsedInSubScene( z ) )
            continue;
          auto* spline = sub->GetClipArray().GetClip( z )->GetSpline( Spline_SubScene_Clip );
          if ( spline && spline->HasIntValue( clipIdx ) )
          {
            LOG_NFO( "Scene: %s Obj: %s Clip: %s", obj->GetParentScene()->GetName().GetPointer(), obj->GetName().GetPointer(), obj->GetParentScene()->GetClipByIndex( z )->GetName().GetPointer() );
            continue;
          }
        }
      }
    }
  }

  subSceneCount = s->GetWeakChildCount( PHX_OBJECT );
  if ( subSceneCount )
  {
    for ( int y = 0; y < subSceneCount; y++ )
    {
      CphxObject_Tool* obj = (CphxObject_Tool*)s->GetWeakChild( PHX_OBJECT, y );
      if ( !obj->IsRequired() )
        continue;

      if ( obj->GetObjectType() == Object_ParticleEmitterCPU && clipIdx == 0 )
      {
        LOG_NFO( "Scene: %s Obj: %s (ALL CLIPS)", obj->GetParentScene()->GetName().GetPointer(), obj->GetName().GetPointer() );
        continue;
      }

      if ( obj->GetObjectType() == Object_SubScene )
      {
        CphxObject_SubScene_Tool* sub = (CphxObject_SubScene_Tool*)obj;
        for ( int z = 0; z < sub->GetClipArray().GetClipCount(); z++ )
        {
          if ( sub->GetSubScene( z ) != s )
            continue;
          if ( !obj->GetParentScene()->GetClipByIndex( z )->IsRequired() && !sub->GetParentScene()->IsClipUsedInSubScene( z ) )
            continue;
          auto* spline = sub->GetClipArray().GetClip( z )->GetSpline( Spline_SubScene_Clip );
          if ( spline && spline->HasIntValue( clipIdx ) )
          {
            LOG_NFO( "Scene: %s Obj: %s Clip: %s", obj->GetParentScene()->GetName().GetPointer(), obj->GetName().GetPointer(), obj->GetParentScene()->GetClipByIndex( z )->GetName().GetPointer() );
            continue;
          }
        }
      }
    }
  }
}

void Console_DumpUnUsedClips( CStringArray& CommandLine )
{
  ClearRequiredFlagForAllResources();
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  for ( int x = 0; x < Project.GetSceneCount(); x++ )
  {
    if ( !Project.GetSceneByIndex( x )->IsRequired() )
      continue;

    for ( int y = 0; y < Project.GetSceneByIndex( x )->GetClipCount(); y++ )
    {
      if ( Project.GetSceneByIndex( x )->GetClipByIndex( y )->IsRequired() )
        continue;

      LOG_NFO( "Scene: %s Clip: %s", Project.GetSceneByIndex( x )->GetName().GetPointer(), Project.GetSceneByIndex( x )->GetClipByIndex( y )->GetName().GetPointer() );
    }
  }
}


#include "HLSLAST.h"
void Console_ToggleShaderMinifierPrettyPrint( CStringArray &CommandLine )
{
  extern TBOOL CrossCompiler::PrettyPrint;
  CrossCompiler::PrettyPrint = !CrossCompiler::PrettyPrint;
  LOG_NFO( "Shader minifier readable printing is now %s", CrossCompiler::PrettyPrint ? "Enabled" : "Disabled" );
}

void Console_ToggleShaderMinifierRenaming( CStringArray &CommandLine )
{
  extern TBOOL CrossCompiler::RenameIdentifiers;
  CrossCompiler::RenameIdentifiers = !CrossCompiler::RenameIdentifiers;
  LOG_NFO( "Shader minifier renaming is now %s", CrossCompiler::RenameIdentifiers ? "Enabled" : "Disabled" );
}

void CreateMinimalConfig( TCHAR *TargetFile );
void ExportMinimal( CString TargetFile, bool zip, bool exportHeaders );

void Console_CreateEngineConfigHeader( CStringArray &CommandLine )
{
  ExportMinimal( _T( "Data\\Minimals\\testoutput.64k" ), false, false );
  CreateMinimalConfig( _T( "Data\\Minimals\\PhoenixConfig.h" ) );
  LOG_NFO( "Engine config dumped to Data\\Minimals\\PhoenixConfig.h" );
}

TBOOL UseNewkkrunchy = true;

void Console_ToggleNewKkruncy( CStringArray &CommandLine )
{
  UseNewkkrunchy = !UseNewkkrunchy;
  LOG_NFO( "New kkrunchy is now %s", UseNewkkrunchy ? "Enabled" : "Disabled" );
}

void Console_Resync( CStringArray &Commandline )
{
  if ( Commandline.NumItems() <= 1 )
  {
    LOG_NFO( "Usage: Resync <framerate>" );
    return;
  }

  TS32 newfps;
  if ( Commandline[ 1 ].Scan( _T( "%d" ), &newfps ) != 1 )
  {
    LOG_NFO( "Invalid numeric input for new framerate" );
    return;
  }

  TS32 oldfps = Project.Timeline->Timeline->FrameRate;
  TF32 change = oldfps / (TF32)newfps;

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    CphxEvent *e = Project.Timeline->Events[ x ]->Event;
    e->StartFrame = (TS32)( ( e->StartFrame / (TF32)oldfps )*newfps );
    e->EndFrame = (TS32)( ( e->EndFrame / (TF32)oldfps )*newfps );
  }

  Project.Timeline->Timeline->FrameRate = newfps;
  LOG_NFO( "Project framerate changed to %d", newfps );
}

#include "ModelParameters.h"

void Console_AddVertex( CStringArray &CommandLine )
{
  if ( CommandLine.NumItems() <= 3 )
  {
    LOG_NFO( "Usage: AddVertex x y z" );
    return;
  }

  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();

  CapexWorkBench *wb = GetActiveWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();
  if ( !model )
  {
    LOG_NFO( "No edited model." );
    return;
  }

  CphxModelObject_Tool *edobj = GetActiveWorkBench()->GetEditedModelObject();
  if ( edobj->GetPrimitive() != Mesh_Stored )
  {
    LOG_NFO( "Edited obj is not a stored mesh" );
    return;
  }

  TF32 xp, yp, zp;
  if ( CommandLine[ 1 ].Scan( _T( "%f" ), &xp ) != 1 || CommandLine[ 2 ].Scan( _T( "%f" ), &yp ) != 1 || CommandLine[ 3 ].Scan( _T( "%f" ), &zp ) != 1 )
  {
    LOG_NFO( "One of the parameters is not a float" );
    return;
  }

  CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)edobj;
  CphxVertex *v = new CphxVertex[ mesh->GetModelObject()->StoredVertexCount + 1 ];
  for ( TS32 x = 0; x < mesh->GetModelObject()->StoredVertexCount; x++ )
    v[ x ] = mesh->GetModelObject()->StoredVertices[ x ];

  v[ mesh->GetModelObject()->StoredVertexCount ].Position = D3DXVECTOR3( xp, yp, zp );
  SAFEDELETEA( mesh->GetModelObject()->StoredVertices );
  mesh->GetModelObject()->StoredVertices = v;
  mesh->GetModelObject()->StoredVertexCount++;
  edobj->InvalidateUptoDateFlag();
}

void Console_AddPoly( CStringArray &CommandLine )
{
  if ( CommandLine.NumItems() <= 3 )
  {
    LOG_NFO( "Usage: AddPoly v1 v2 v3 (v4)" );
    return;
  }

  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();

  CapexWorkBench *wb = GetActiveWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();
  if ( !model )
  {
    LOG_NFO( "No edited model." );
    return;
  }

  CphxModelObject_Tool *edobj = GetActiveWorkBench()->GetEditedModelObject();
  if ( edobj->GetPrimitive() != Mesh_Stored )
  {
    LOG_NFO( "Edited obj is not a stored mesh" );
    return;
  }

  TS32 vi[ 4 ];
  TS32 vc = CommandLine.NumItems() - 1;

  for ( TS32 x = 1; x < CommandLine.NumItems(); x++ )
  {
    if ( CommandLine[ x ].Scan( _T( "%d" ), &vi[ x - 1 ] ) != 1 )
    {
      LOG_NFO( "One of the parameters is not an int" );
      return;
    }
  }

  CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)edobj;

  CphxPolygon *v = new CphxPolygon[ mesh->GetModelObject()->StoredPolyCount + 1 ];
  for ( TS32 x = 0; x < mesh->GetModelObject()->StoredPolyCount; x++ )
    v[ x ] = mesh->GetModelObject()->StoredPolygons[ x ];

  v[ mesh->GetModelObject()->StoredPolyCount ].VertexCount = vc;

  for ( TS32 x = 0; x < vc; x++ )
    v[ mesh->GetModelObject()->StoredPolyCount ].VertexIDs[ x ] = vi[ x ];

  SAFEDELETEA( mesh->GetModelObject()->StoredPolygons );
  mesh->GetModelObject()->StoredPolygons = v;
  mesh->GetModelObject()->StoredPolyCount++;
  mesh->GetModelObject()->Mesh.SkipNormalCalculation = false;
  edobj->InvalidateUptoDateFlag();
}

void Console_SetSceneImportScale( CStringArray &CommandLine )
{
  extern float sceneImportScale;

  if ( CommandLine.NumItems() <= 1 )
  {
    LOG_NFO( "Usage: SetSceneImportScale value" );
    return;
  }

  TF32 s;
  if ( CommandLine[ 1 ].Scan( _T( "%f" ), &s ) != 1 )
  {
    LOG_NFO( "Parameter is not a float" );
    return;
  }

  sceneImportScale = s;
  LOG_NFO( "Scene Import Scale set to %f", s );
}

void Console_MergeSelectedModels( CStringArray &CommandLine )
{
  CapexWorkBench *wb = GetActiveWorkBench();
  CphxScene_Tool *scene = NULL;
  if ( wb ) scene = wb->GetEditedScene();
  if ( !scene )
  {
    LOG_NFO( "No edited scene." );
    return;
  }

  int selectionCount = 0;
  for ( TS32 x = 0; x < scene->GetObjectCount(); x++ )
  {
    if ( !scene->GetObjectByIndex( x )->Selected )
      continue;

    selectionCount++;

    if ( scene->GetObjectByIndex( x )->GetObjectType() != Object_Model )
    {
      LOG_NFO( "Not all items selected are models." );
      return;
    }
  }

  if ( selectionCount < 2 )
  {
    LOG_NFO( "Need to have at least 2 objects selected to be merged" );
    return;
  }

  CphxModel_Tool *targetModel = nullptr;

  D3DXMATRIX targetMatrixInv;

  for ( TS32 x = 0; x < scene->GetObjectCount(); x++ )
  {
    if ( !scene->GetObjectByIndex( x )->Selected )
      continue;

    if ( !targetModel )
    {
      CphxObject_Model_Tool *m = (CphxObject_Model_Tool*)scene->GetObjectByIndex( x );
      targetModel = m->Model;
      targetMatrixInv = scene->GetObjectByIndex( x )->GetMatrix();
      D3DXMatrixInverse( &targetMatrixInv, nullptr, &targetMatrixInv );
      targetModel->InvalidateUptoDateFlag();
      continue;
    }

    CphxObject_Model_Tool *m = (CphxObject_Model_Tool*)scene->GetObjectByIndex( x );
    CphxModel_Tool *srcModel = m->Model;
    D3DXMATRIX diffMatrix = m->GetMatrix();
    D3DXMatrixMultiply( &diffMatrix, &diffMatrix, &targetMatrixInv );

    while ( srcModel->Objects.NumItems() )
    {
      D3DXMATRIX mat = srcModel->Objects[ 0 ]->GetMatrix();
      D3DXMatrixMultiply( &mat, &diffMatrix, &mat );
      srcModel->Objects[ 0 ]->SetMatrix( mat );
      targetModel->Objects.Add( srcModel->Objects[ 0 ] );
      srcModel->Objects[ 0 ]->InvalidateUptoDateFlag();
      srcModel->Objects.DeleteByIndex( 0 );
    }

    scene->DeleteByIndex( x );
    x--;
    Project.DeleteModel( srcModel->GetGUID() );
  }

  Root->UpdateWindowData();

  LOG_NFO( "Merge done" );
}

TBOOL MarkTextureAsRequired( CphxGUID &GUID );
namespace CrossCompiler
{
  namespace Parser
  {
    bool MinifyMultipleShaders( CArray<CString>& input, CString& include, CArray<CString>& output );
  }
}

void Console_MinifyMaterialShaders( CStringArray &CommandLine )
{
  ClearRequiredFlagForAllResources();
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    if ( Project.Timeline->Events[ x ]->Type == EVENT_SHADERTOY )
    {
      CphxEvent_Shadertoy_Tool *e = (CphxEvent_Shadertoy_Tool*)Project.Timeline->Events[ x ];
      e->GetTech()->MarkAsRequired();

      for ( TS32 z = 0; z < e->MaterialData.MaterialTextures.NumItems(); z++ )
        if ( !MarkTextureAsRequired( e->MaterialData.MaterialTextures.GetByIndex( z ) ) )
          LOG_ERR( "[minimalexport] Error occured in event %s (%d) texture %d", e->GetName().GetPointer(), x, z );
    }
  }

  CArray< CString > shaderArray;
  CArray< CString > outputArray;
  CString include;

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( x );
    if ( t->IsRequired() )
    {
      for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      {
        CphxMaterialRenderPass_Tool *p = t->Passes[ y ];
        shaderArray += p->ShaderCode;
      }
    }
  }

  LOG_NFO( "Minifying %d shaders at once", shaderArray.NumItems() );

  CrossCompiler::Parser::MinifyMultipleShaders( shaderArray, include, outputArray );

  int counter = 0;

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( x );
    if ( t->IsRequired() )
    {
      for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      {
        CphxMaterialRenderPass_Tool *p = t->Passes[ y ];
        p->ShaderCode = include + outputArray[ counter ];
        t->InvalidateUptoDateFlag();
        counter++;
      }
    }
  }
}

void Console_MinifyTextureShaders( CStringArray &CommandLine )
{
  ClearRequiredFlagForAllResources();
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    if ( Project.Timeline->Events[ x ]->Type == EVENT_SHADERTOY )
    {
      CphxEvent_Shadertoy_Tool *e = (CphxEvent_Shadertoy_Tool*)Project.Timeline->Events[ x ];
      e->GetTech()->MarkAsRequired();

      for ( TS32 z = 0; z < e->MaterialData.MaterialTextures.NumItems(); z++ )
        if ( !MarkTextureAsRequired( e->MaterialData.MaterialTextures.GetByIndex( z ) ) )
          LOG_ERR( "[minimalexport] Error occured in event %s (%d) texture %d", e->GetName().GetPointer(), x, z );
    }
  }

  CArray< CString > shaderArray;
  CArray< CString > outputArray;
  CString include;

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *r = Project.GetTextureFilterByIndex( x );
    if ( r->IsRequired() )
      shaderArray += r->Code;
  }

  LOG_NFO( "Minifying %d shaders at once", shaderArray.NumItems() );

  CrossCompiler::Parser::MinifyMultipleShaders( shaderArray, include, outputArray );

  int counter = 0;

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *r = Project.GetTextureFilterByIndex( x );
    if ( r->IsRequired() )
    {
      r->Code = include + outputArray[ counter ];
      r->InvalidateUptoDateFlag();
      counter++;
    }
  }
}

void Console_BackupShaders( CStringArray &CommandLine )
{
  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *r = Project.GetTextureFilterByIndex( x );
    r->CodeBackup = r->Code;
  }

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( x );
    for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
    {
      CphxMaterialRenderPass_Tool *p = t->Passes[ y ];
      p->CodeBackup = p->ShaderCode;
    }
  }
}

void Console_RestoreShaders( CStringArray &CommandLine )
{
  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *r = Project.GetTextureFilterByIndex( x );
    r->Code = r->CodeBackup;
    r->InvalidateUptoDateFlag();
  }

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( x );
    for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
    {
      CphxMaterialRenderPass_Tool *p = t->Passes[ y ];
      p->ShaderCode = p->CodeBackup;
      t->InvalidateUptoDateFlag();
    }
  }
}

void Console_MinifyAllShaders( CStringArray &CommandLine )
{
  Console_BackupShaders( CommandLine );

  ClearRequiredFlagForAllResources();
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    if ( Project.Timeline->Events[ x ]->Type == EVENT_SHADERTOY )
    {
      CphxEvent_Shadertoy_Tool *e = (CphxEvent_Shadertoy_Tool*)Project.Timeline->Events[ x ];
      e->GetTech()->MarkAsRequired();

      for ( TS32 z = 0; z < e->MaterialData.MaterialTextures.NumItems(); z++ )
        if ( !MarkTextureAsRequired( e->MaterialData.MaterialTextures.GetByIndex( z ) ) )
          LOG_ERR( "[minimalexport] Error occured in event %s (%d) texture %d", e->GetName().GetPointer(), x, z );
    }
  }

  CArray< CString > shaderArray;
  CArray< CString > outputArray;
  CString include;

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *r = Project.GetTextureFilterByIndex( x );
    if ( r->IsRequired() )
      shaderArray += r->Code;
  }

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( x );
    if ( t->IsRequired() )
    {
      for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      {
        CphxMaterialRenderPass_Tool *p = t->Passes[ y ];
        shaderArray += p->ShaderCode;
      }
    }
  }

  LOG_NFO( "Minifying %d shaders at once", shaderArray.NumItems() );

  CrossCompiler::Parser::MinifyMultipleShaders( shaderArray, include, outputArray );

  int counter = 0;

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *r = Project.GetTextureFilterByIndex( x );
    if ( r->IsRequired() )
    {
      r->Code = include + "\n" + outputArray[ counter ];
      r->InvalidateUptoDateFlag();
      counter++;
    }
  }

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( x );
    if ( t->IsRequired() )
    {
      for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      {
        CphxMaterialRenderPass_Tool *p = t->Passes[ y ];
        p->ShaderCode = include + "\n" + outputArray[ counter ];
        t->InvalidateUptoDateFlag();
        counter++;
      }
    }
  }
}

void Console_GetMaterialStats( CStringArray &CommandLine )
{

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    if ( Project.Timeline->Events[ x ]->Type == EVENT_SHADERTOY )
    {
      CphxEvent_Shadertoy_Tool *e = (CphxEvent_Shadertoy_Tool*)Project.Timeline->Events[ x ];
      e->GetTech()->MarkAsRequired();
    }
  }

  CphxArray<CphxMaterial_Tool*> mats;

  for ( int x = 0; x < Project.GetMaterialCount(); x++ )
    mats += Project.GetMaterialByIndex( x );

  mats.Sort( []
              ( CphxMaterial_Tool** a, CphxMaterial_Tool** b ) -> int 
              { 
                return (*a)->GetChildCount() - (*b)->GetChildCount(); 
              } );

  for ( int x = 0; x < mats.NumItems(); x++ )
  {
    if ( !mats[ x ]->IsRequired() )
      continue;

    int cnt = 0;
    for ( int y = 0; y < mats[ x ]->GetChildCount(); y++ )
    {
      if ( mats[ x ]->GetChild( y )->IsRequired() )
        cnt++;
    }

    if ( !cnt )
      continue;

    LOG_NFO( "Material %s is referenced %d times:", mats[ x ]->Name.GetPointer(), cnt );
    for ( int y = 0; y < mats[ x ]->GetChildCount(); y++ )
    {
      if ( !mats[ x ]->GetChild( y )->IsRequired() )
        continue;
        
      CString resourceName = "";

      switch ( mats[ x ]->GetChild( y )->GetType() )
      {
      case PHX_MODELOBJECT:
      {
        CphxModelObject_Tool* obj = (CphxModelObject_Tool*)mats[ x ]->GetChild( y );
        resourceName = obj->GetParentModel()->GetName() + " / " + obj->GetName();
      }
        break;

        default:
          resourceName = "not implemented for display";
      }

      LOG_NFO( "    %d: %s", mats[ x ]->GetChild( y )->GetType(), resourceName.GetPointer() );
    }
  }

}

void Console_SetAllMeshMaterials( CStringArray &CommandLine )
{
  auto *mat = Project.GetMaterialByIndex( 0 );

  if ( CommandLine.NumItems() >= 2 )
    for ( int x = 0; x < Project.GetMaterialCount(); x++ )
      if ( Project.GetMaterialByIndex( x )->Name == CommandLine[ 1 ] )
        mat = Project.GetMaterialByIndex( x );


  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();

  CapexWorkBench *wb = GetActiveWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();
  if ( !model )
  {
    LOG_NFO( "No edited model." );
    return;
  }

  for ( TS32 x = 0; x < model->GetObjectCount(); x++ )
  {
    model->GetObjectByIndex( x )->SetMaterial( mat );
  }
}


void Console_MergeStoredMeshesInModel( CStringArray &CommandLine )
{

  if ( CommandLine.NumItems() <= 1 )
  {
    LOG_NFO( "Usage: MergeStoredMeshesInModel value" );
    return;
  }

  TF32 s;
  if ( CommandLine[ 1 ].Scan( _T( "%f" ), &s ) != 1 )
  {
    LOG_NFO( "Parameter is not a float" );
    return;
  }

  CphxModel_Tool *m = GetActiveWorkBench()->GetEditedModel();

  CapexWorkBench *wb = GetActiveWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();
  if ( !model )
  {
    LOG_NFO( "No edited model." );
    return;
  }

  for ( TS32 x = 0; x < model->GetObjectCount(); x++ )
    model->GetObjectByIndex( x )->Selected = false;

  for ( TS32 x = 0; x < model->GetObjectCount(); x++ )
  {
    if ( model->GetObjectByIndex( x )->GetPrimitive() == Mesh_Stored )
    {
      CphxModelObject_Tool_Mesh* n = (CphxModelObject_Tool_Mesh*)model->GetObjectByIndex( x );
      int vxCount = n->GetModelObject()->StoredVertexCount;
      int polyCount = n->GetModelObject()->StoredPolyCount;


      if ( vxCount < 20 )
        n->Selected = true;

      if ( polyCount < 12 )
        n->Selected = true;

      bool allBelow = true;

      for ( int y = 0; y < vxCount; y++ )
        if ( n->GetModelObject()->StoredVertices[ y ].Position.z > s )
        {
          allBelow = false;
          break;
        }

      if ( allBelow )
        n->Selected = true;
    }
  }

  return;

  CphxModelObject_Tool_Mesh* o = nullptr;

  int vxCount = 0;
  int polyCount = 0;

  for ( TS32 x = 0; x < model->GetObjectCount(); x++ )
  {
    if ( model->GetObjectByIndex( x )->GetPrimitive() == Mesh_Stored )
    {
      if ( !o )
        o = (CphxModelObject_Tool_Mesh*)model->GetObjectByIndex( x );

      CphxModelObject_Tool_Mesh* n = (CphxModelObject_Tool_Mesh*)model->GetObjectByIndex( x );
      vxCount += n->GetModelObject()->StoredVertexCount;
      polyCount += n->GetModelObject()->StoredPolyCount;

    }
  }

  if ( !o )
    return;

  int currVxId = 0;
  int currPolyId = 0;

  auto *oldVertices = o->GetModelObject()->StoredVertices;
  auto *oldPolygons = o->GetModelObject()->StoredPolygons;
  int oldVxCount = o->GetModelObject()->StoredVertexCount;
  int oldPolycount = o->GetModelObject()->StoredPolyCount;

  o->GetModelObject()->StoredVertices = new CphxVertex[ vxCount ];
  o->GetModelObject()->StoredPolygons = new CphxPolygon[ polyCount ];

  for ( TS32 x = 0; x < model->GetObjectCount(); x++ )
  {
    if ( model->GetObjectByIndex( x )->GetPrimitive() == Mesh_Stored )
    {
      CphxModelObject_Tool_Mesh* n = (CphxModelObject_Tool_Mesh*)model->GetObjectByIndex( x );

      auto *srcVertices = n->GetModelObject()->StoredVertices;
      auto *srcPolygons = n->GetModelObject()->StoredPolygons;
      int srcVxCount = n->GetModelObject()->StoredVertexCount;
      int srcPolycount = n->GetModelObject()->StoredPolyCount;

      if ( model->GetObjectByIndex( x ) == o )
      {
        srcVertices = oldVertices;
        srcPolygons = oldPolygons;
        srcVxCount = oldVxCount;
        srcPolycount = oldPolycount;
      }

      int vxId = currVxId;

      for ( int y = 0; y < srcVxCount; y++ )
        o->GetModelObject()->StoredVertices[ currVxId++ ] = srcVertices[ y ];

      for ( int y = 0; y < srcPolycount; y++ )
      {
        o->GetModelObject()->StoredPolygons[ currPolyId ] = srcPolygons[ y ];
        for ( int z = 0; z < 4; z++ )
          o->GetModelObject()->StoredPolygons[ currPolyId ].VertexIDs[ z ] += vxId;

        currPolyId++;
      }

      n->GetModelObject()->StoredPolyCount = 0;
      n->GetModelObject()->StoredVertexCount = 0;
      if ( n != o )
        n->InvalidateUptoDateFlag();
    }

    LOG_ERR( "COUNTS (%d): %d/%d %d/%d", x, vxCount, currVxId, polyCount, currPolyId );
  }

  if ( vxCount != currVxId || polyCount != currPolyId )
  {
    LOG_ERR( "COUNT MISMATCH! %d/%d %d/%d", vxCount, currVxId, polyCount, currPolyId );
  }

  if ( o )
  {
    o->GetModelObject()->StoredPolyCount = vxCount;
    o->GetModelObject()->StoredVertexCount = polyCount;
    o->InvalidateUptoDateFlag();
  }

}

void Console_TogglePicking( CStringArray &CommandLine )
{
  pickingEnabled = !pickingEnabled;
  LOG_NFO( "Picking is now %s", pickingEnabled ? "Enabled" : "Disabled" );
}

void Console_ImportKasparov( CStringArray &CommandLine )
{
  Root->ImportKasparov();
}

#include "TexGenPreview.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

void Console_ExportLTCDownSample( CStringArray &CommandLine )
{
  //CStreamWriterMemory Writer;
  //HRESULT res = SaveDDSTexture( DeviceContext, TextureHandle, Writer );
  auto *wnd = (CapexTexGenPreview*)GetActiveWorkBench()->GetWindow( apEx_TexGenPreview );
  auto *op = Project.GetTexgenOp( wnd->GetEditedOperator() );
  if ( op )
    op = op->GetContentOp();
  if ( op && op->ContentReady() )
  {   

    CCoreDX11Texture2D *dummy = new CCoreDX11Texture2D( (CCoreDX11Device*)GetActiveWorkBench()->GetApplication()->GetDevice() );
    dummy->SetTextureHandle( op->Result->Texture );
    dummy->SetView( op->Result->View );

    auto* Texture = (CCoreDX11Texture2D*)( GetActiveWorkBench()->GetApplication()->GetDevice()->CopyTexture( dummy ) );

    CString Filename( "ltcexport.raw" );

    CStreamWriterMemory Writer;
    HRESULT res = SaveDDSTexture( Texture->GetDeviceContext(), Texture->GetTextureHandle(), Writer );
    if ( res != S_OK )
    {
      _com_error err( res );
      LOG( LOG_ERROR, _T( "Failed to export LTC texture to '%s' (%x: %s)" ), Filename.GetPointer(), res, err.ErrorMessage() );
      return;
    }

    struct DDSHEAD
    {
      TU32 DDS;
      TS32 dwSize;
      TS32 dwFlags;
      TS32 dwHeight;
      TS32 dwWidth;
      TS32 dwPitchOrLinearSize;
      TS32 dwDepth;
      TS32 dwMipMapCount;
      TS32 dwReserved1[ 11 ];

      TS32 _dwSize;
      TS32 _dwFlags;
      TS32 dwFourCC;
      TS32 dwRGBBitCount;
      TS32 dwRBitMask;
      TS32 dwGBitMask;
      TS32 dwBBitMask;
      TS32 dwABitMask;

      TS32 dwCaps;
      TS32 dwCaps2;
      TS32 dwCaps3;
      TS32 dwCaps4;
      TS32 dwReserved2;
    };

    struct DDS_HEADER_DXT10
    {
      DXGI_FORMAT              dxgiFormat;
      D3D10_RESOURCE_DIMENSION resourceDimension;
      UINT                     miscFlag;
      UINT                     arraySize;
      UINT                     miscFlags2;
    };

    TU8 *Data = (TU8*)Writer.GetData();
    DDSHEAD head;
    memcpy( &head, Data, sizeof( DDSHEAD ) );
    Data += head.dwSize + 4;

    if ( head.dwFourCC == 113 )
    {
      float *img2 = new float[ head.dwWidth*head.dwHeight * 4 ];
      DirectX::PackedVector::XMConvertHalfToFloatStream( img2, 4, ( const DirectX::PackedVector::HALF* )Data, 2, head.dwWidth*head.dwHeight * 4 );

      float out[ 16 * 16 * 4 ];
      for ( int x = 0; x < 16; x++ )
        for ( int y = 0; y < 16; y++ )
          for ( int z = 0; z < 4; z++ )
          {
            float v = 0;
            int c = 0;
            int xc = head.dwWidth / 16;
            int yc = head.dwHeight / 16;

            for ( int a = 0; a < xc; a++ )
              for ( int b = 0; b < yc; b++ )
              {
                int xp = x*xc + a;
                int yp = y*yc + b;
                v += img2[ ( xp + yp*head.dwHeight ) * 4 + z ];
                c++;
              }

            out[ ( x + y * 16 ) * 4 + z ] = v / c;
          }

      delete[] img2;

      FILE *f16 = NULL;
      fopen_s( &f16, "ltcData.raw16", "w+b" );
      FILE *f8 = NULL;
      fopen_s( &f8, "ltcData.raw8", "w+b" );

      for ( int y = 0; y < 4; y++ )
      {
        float minv = out[ y ];
        float maxv = out[ y ];

        for ( int x = 0; x < 16 * 16; x++ )
        {
          minv = min( out[ x * 4 + y ], minv );
          maxv = max( out[ x * 4 + y ], maxv );
        }

        float dif = maxv - minv;

        for ( int x = 0; x < 16 * 16; x++ )
        {
          unsigned short quanted = 0;
          if ( dif != 0 )
            quanted = (unsigned short)( ( out[ x * 4 + y ] - minv ) / dif * 65535 );

          D3DXFLOAT16 v( out[ x * 4 + y ] );
          fwrite( &v, 2, 1, f16 );
          unsigned char vc = (unsigned char)( max( 0, min( 1, out[ x * 4 + y ] ) ) * 255 );
          fwrite( &vc, 1, 1, f8 );
        }
      }

      fclose( f8 );
      fclose( f16 );
    }
    else
    {
      LOG( LOG_ERROR, _T( "Previewed Texture is not HDR!" ) );
    }


    delete Texture;
  }

}

void Console_SetSolutionRoot( CStringArray & Commandline )
{
  if ( Root && Commandline.NumItems() > 1 )
  {
    Root->TrySetSolutionRoot( Commandline[ 1 ] );
  }
}

void InitializeConsoleCommands()
{
  if ( !Root ) return;
  Root->RegisterConsoleCommand( _T( "List" ), _T( "List all available console commands" ), Console_List );
  Root->RegisterConsoleCommand( _T( "Help" ), _T( "List all available console commands" ), Console_List );
  Root->RegisterConsoleCommand( _T( "AtlasDump" ), _T( "Dump the GUI atlas texture into a file (atlas.png by default)" ), Console_DumpAtlas );
  Root->RegisterConsoleCommand( _T( "AtlasOptimize" ), _T( "Optimize the GUI atlas" ), Console_OptimizeAtlas );
  Root->RegisterConsoleCommand( _T( "AtlasResize" ), _T( "Resize the GUI atlas" ), Console_ResizeAtlas );
  Root->RegisterConsoleCommand( _T( "ShaderTest" ), _T( "Try to minify all shaders and recompile them" ), Console_MinifyShaderTest );
  Root->RegisterConsoleCommand( _T( "DumpShaders" ), _T( "Dump all shader binaries" ), Console_DumpShaderBinaries );
  Root->RegisterConsoleCommand( _T( "ToggleShaderMinifier" ), _T( "Enable/Disable Shader Minifier during minimal export" ), Console_ToggleShaderMinifier );
  Root->RegisterConsoleCommand( _T( "ToggleGlobalShaderMinifier" ), _T( "Enable/Disable Global shader Minifier during minimal export" ), Console_ToggleGlobalShaderMinifier );
  Root->RegisterConsoleCommand( _T( "ToggleSetupSocial" ), _T( "Enable/Disable social network links on setup dialog" ), Console_ToggleSetupHasSocial );
  Root->RegisterConsoleCommand( _T( "TogglePrettyPrint" ), _T( "Enable/Disable Shader Minifier readable output" ), Console_ToggleShaderMinifierPrettyPrint );
  Root->RegisterConsoleCommand( _T( "ToggleRenaming" ), _T( "Enable/Disable Shader Minifier identifier renaming" ), Console_ToggleShaderMinifierRenaming );
  Root->RegisterConsoleCommand( _T( "CreateEngineConfigHeader" ), _T( "Create Config Header of Used Engine Parts" ), Console_CreateEngineConfigHeader );
  Root->RegisterConsoleCommand( _T( "ToggleNewKKrunchy" ), _T( "Enable/Disable kkrunchy v7" ), Console_ToggleNewKkruncy );
  Root->RegisterConsoleCommand( _T( "ToggleFarbrauschPrecalc" ), _T( "Enable/Disable Farbrausch precalc" ), Console_ToggleFarbrauschPrecalc );
  Root->RegisterConsoleCommand( _T( "ToggleMinimalPrecalc" ), _T( "Enable/Disable Minimal precalc" ), Console_ToggleMinimalPrecalc );
  Root->RegisterConsoleCommand( _T( "ReSync" ), _T( "Change framerate of timeline and attempt to sync events to the new framerate" ), Console_Resync );
  Root->RegisterConsoleCommand( _T( "AddVertex" ), _T( "Add a vertex to a stored mesh" ), Console_AddVertex );
  Root->RegisterConsoleCommand( _T( "AddPoly" ), _T( "Add a poly to a stored mesh" ), Console_AddPoly );
  Root->RegisterConsoleCommand( _T( "SetSceneImportScale" ), _T( "Set Default Scaling For Imported Scenes" ), Console_SetSceneImportScale );
  Root->RegisterConsoleCommand( _T( "MergeSelectedModels" ), _T( "Merges the selected objects in the scene into a single model" ), Console_MergeSelectedModels );
  Root->RegisterConsoleCommand( _T( "TestShaderBlob" ), _T( "Tries to build a single binary blob of all the shaders" ), Console_TestShaderBlob );
  Root->RegisterConsoleCommand( _T( "MinifyAllShaders" ), _T( "Minifies all shaders together, deduplicating the code where possible" ), Console_MinifyAllShaders );
  Root->RegisterConsoleCommand( _T( "MinifyMaterialShaders" ), _T( "Minifies all material shaders together, deduplicating the code where possible" ), Console_MinifyMaterialShaders );
  Root->RegisterConsoleCommand( _T( "MinifyTextureShaders" ), _T( "Minifies all texture shaders together, deduplicating the code where possible" ), Console_MinifyTextureShaders );
  Root->RegisterConsoleCommand( _T( "GetMaterialStats" ), _T( "Display material usage statistics" ), Console_GetMaterialStats );
  Root->RegisterConsoleCommand( _T( "TogglePicking" ), _T( "Enable/Disable picking in scene previews" ), Console_TogglePicking );
  Root->RegisterConsoleCommand( _T( "MergeMeshes" ), _T( "Merge all stored meshes into one" ), Console_MergeStoredMeshesInModel );
  Root->RegisterConsoleCommand( _T( "SetAllMaterials" ), _T( "Set all materials of the scene to parameter" ), Console_SetAllMeshMaterials );
  Root->RegisterConsoleCommand( _T( "ImportKasparov" ), _T( "Import Kasparov" ), Console_ImportKasparov );
  Root->RegisterConsoleCommand( _T( "TexgenMemPool" ), _T( "Report or change the size of the texgen memory pool" ), Console_TexgenMemPool );
  Root->RegisterConsoleCommand( _T( "ExportLTCDownSample" ), _T( "Export downsampled texture for LTC data" ), Console_ExportLTCDownSample );
  Root->RegisterConsoleCommand( _T( "BackupShaders" ), _T( "Back up shaders (for shader minification testing)" ), Console_BackupShaders );
  Root->RegisterConsoleCommand( _T( "RestoreShaders" ), _T( "Restore shaders (for shader minification testing)" ), Console_RestoreShaders );
  Root->RegisterConsoleCommand( _T( "SetSolutionRoot" ), _T( "Set root path for apEx VS solution" ), Console_SetSolutionRoot );
  Root->RegisterConsoleCommand( _T( "DumpSubSceneReferencesForEditedClip" ), _T( "Write out which objects reference this clip through a subscene clip spline" ), Console_DumpSubSceneReferencesForEditedClip );
  Root->RegisterConsoleCommand( _T( "DumpUnUsedClips" ), _T( "Write out unused clips in scenes that are used" ), Console_DumpUnUsedClips );


#ifdef MEMORY_TRACKING
  Root->RegisterConsoleCommand( _T( "MemoryDiagnostics" ), _T( "Memtracker diagnostic info" ), Console_MemoryDiagnostics );
#endif
}