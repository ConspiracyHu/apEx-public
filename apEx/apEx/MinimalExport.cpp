#include "BasePCH.h"
#include "MinimalExport.h"
#include "../Phoenix_Tool/apxProject.h"
#include "../Phoenix/Project.h"
#include "../Phoenix/Timeline.h"
#include "ExternalTools.h"
#define MINIZ_HEADER_FILE_ONLY
#include "../../Bedrock/UtilLib/miniz.c"

CArray<CString> symbolNameStack;
CArray<CString> symbolList;

void ResetSymbolBuilder()
{
  symbolNameStack.FlushFast();
  symbolList.Flush();
}

void PushSymbolName( const CString& str )
{
  symbolNameStack += str;
}

void PopSymbolName()
{
  symbolNameStack.DeleteByIndex( symbolNameStack.NumItems() - 1 );
}

class SymbolNameScoper
{
public:
  SymbolNameScoper( const CString& string )
  {
    PushSymbolName( string );
  }

  ~SymbolNameScoper()
  {
    PopSymbolName();
  }
};

CString BuildSymbolName()
{
  CString separator = "::";
  //separator[ 0 ] = (char)0xfe;    
  CString s = symbolNameStack[ 0 ];
  for ( int x = 1; x < symbolNameStack.NumItems(); x++ )
    s += separator + symbolNameStack[ x ];
  return s;
}

unsigned short GetSymbolID()
{
  CString symbolName = BuildSymbolName();
  int id = symbolList.Find( symbolName );
  if ( id >= 0 )
    return id;
  symbolList += symbolName;
  return symbolList.NumItems() - 1;
}

struct MiniMesh
{
  TS32 vxDataSize;
  TS32 polyDataSize;
  unsigned char* vxData = nullptr;
  unsigned char* polyData = nullptr;
};

CArray<MiniMesh> miniMeshes;

bool hasLTC1 = false;
bool hasLTC2 = false;

namespace CrossCompiler
{
namespace Parser
{
bool MinifyMultipleShaders( CArray<CString>& input, CString& include, CArray<CString>& output );
}
}

TS32 SplineCount = 0;
TS32 SplineDataSize = 0;
TS32 SplineDataSizeScenes = 0;
TS32 TexgenOpCount = 0;
TS32 TexgenOpDataSize = 0;
TS32 MaterialParamCount = 0;
TS32 MaterialParamSize = 0;
TS32 MaterialParameterDataCount = 0;
TS32 MaterialParameterDataSize = 0;
TS32 ModelFilterCount = 0;
TS32 ModelFilterSize = 0;
TS32 MaterialDataPackCount = 0;
TS32 MaterialParamDefaultCount = 0;
TS32 ModelObjectCount = 0;
TS32 ModelObjectDataSize = 0;
TS32 ClipSplineCount = 0;
TS32 SceneObjectCount = 0;
TS32 SceneObjectDataSize = 0;
TS32 EventCount = 0;
TS32 EventDataSize = 0;
TS32 MiscDataSize = 0;
TS32 SetupDataSize = 0;
TS32 RenderTargetDataSize = 0;
TS32 RenderLayerDataSize = 0;
TS32 TextureFilterDataSize = 0;
TS32 ShaderSize = 0;
TS32 TexgenPageDataSize = 0;
TS32 MaterialTechCount = 0;
TS32 MaterialTechDataSize = 0;
TS32 MaterialDescriptionSize = 0;
TS32 TreeDataSize = 0;
TS32 ModelSetupSize = 0;
TS32 SceneSetupSize = 0;
TS32 TimelineSetupSize = 0;
TS32 MinimalDataSize = 0;
TS32 defaultSplineCount = 0;
TS32 constantSplineCount = 0;
TS32 complexConstantSplineCount = 0;
TS32 fullyConstantSplines = 0;

CphxMaterial_Tool* NoMaterial = NULL;

int UsedModelPrimitives[ 256 ];
int UsedModelFilters[ 256 ];
int UsedTexgenInputs[ 256 ];
int UsedScenePrimitives[ 256 ];
int UsedEventTypes[ 256 ];
TBOOL hasUVClip = false;
TBOOL hasMeshParticle = false;
TBOOL hasNormalParticle = false;
TBOOL hasSubSceneParticle = false;
TBOOL TexgenImageLoad = false;
TBOOL MinimalExportSuccessful = true;
TBOOL scatterVertexSupport = false;
TBOOL scatterEdgeSupport = false;
TBOOL scatterPolySupport = false;
TBOOL scatterPolyMultiSupport = false;
TBOOL splineHasZeroKeyExport = false;
TBOOL hasTechParams = false;
TBOOL hasParticleSort = false;
TBOOL hasScatterOrientationOriginal = false;
TBOOL hasScatterOrientationNormal = false;
TBOOL hasScatterOrientationNormalRotate = false;
TBOOL hasScatterOrientationFullRotate = false;

CStreamWriterMemory MinimalStreams[ StreamCount ];
CStreamWriterMemory SymbolStreams[ StreamCount ];
int MinimalStreamPositions[ StreamCount ]{};
void StoreStreamPositions()
{
  for ( int x = 0; x < StreamCount; x++ )
    MinimalStreamPositions[ x ] = MinimalStreams[ x ].GetLength();
}

CStringArray fontList;
extern char* EngineFontList[];

int GetStreamSizeDiff()
{
  int count = 0;
  for ( int x = 0; x < StreamCount; x++ )
    count += MinimalStreams[ x ].GetLength() - MinimalStreamPositions[ x ];
  return count;
}

static unsigned char MeshPrimitiveParameterCounts[] = //IF THESE CHANGE YOU NEED TO CHANGE THEM IN THE PROJECT.CPP AS WELL - it's a size optimization thing
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

static unsigned char MeshFilterParameterCounts[] = //IF THESE CHANGE YOU NEED TO CHANGE THEM IN THE PROJECT.CPP AS WELL - it's a size optimization thing
{
  2,//ModelFilter_UVMap = 0,
  1,//ModelFilter_Bevel,
  0,//ModelFilter_MapXForm,
  2,//ModelFilter_MeshSmooth,
  1,//ModelFilter_SmoothGroup,
  2,//ModelFilter_TintMesh,
  3,//ModelFilter_TintMeshShape,
  1,//ModelFilter_Replicate,
  2,//ModelFilter_NormalDeform
  2,//ModelFilter_CSG
  3,//ModelFilter_Greeble
};

#include "HLSLParser.h"

TBOOL MinifyShader_CNS( char* Data, CString& Result, TBOOL Verbose, TBOOL ResetDictionary, TBOOL RebuildDictionary )
{
  CString Output;
  CString Shader = Data;
  if ( CrossCompiler::Parser::Parse( Shader, Output, Verbose, ResetDictionary, RebuildDictionary ) )
  {
    Result = Output;
    return true;
  }
  return false;
}

CString ToMacroCompatible( CString& _name )
{
  CString s = _name;
  for ( int i = 0; i < (int)s.Length(); i++ ) if ( !isalnum( s[ i ] ) ) s[ i ] = '_'; else s[ i ] = toupper( s[ i ] );
  s += ""; // reset hash!
  return s;
}

void WriteSymbolStream( int stream, int bytes )
{
  for ( int x = 0; x < bytes; x++ )
    SymbolStreams[ stream ].WriteWord( GetSymbolID() );
}

#define WriteByte(a,x,y) do { MinimalStreams[a].WriteByte(x); y++; WriteSymbolStream(a,1); } while(0)
#define WriteWord(a,x,y) do { MinimalStreams[a].WriteWord(x); y+=2; WriteSymbolStream(a,2); } while(0)
#define WriteFloat16s(a,x,n,y) do { MinimalStreams[a].Write(x,n*2); y+=2*n; WriteSymbolStream(a,2*n); } while(0)
#define Write(a,x,y,z) do { MinimalStreams[a].Write(x,y); z+=y; WriteSymbolStream(a,y); } while(0)
#define WriteASCIIZ(a,x,y) do { MinimalStreams[a].WriteASCIIZ(x); y+=x.Length()+1; WriteSymbolStream(a,x.Length()+1); } while(0)

CArray<CphxSpline_Tool_float16*> exportedSplines;

int duplicateSplines = 0;
int duplicateSplineSize = 0;

bool IsSplineConstant( CphxSpline_Tool_float16* Spline, D3DXFLOAT16& constValue )
{
  CphxSpline_float16* sp = (CphxSpline_float16*)Spline->Spline;

  D3DXFLOAT16 value = 0;
  bool isConstant = true;

  /*
    if ( !Spline->Keys.NumItems() )
      value = sp->Value[ 0 ];
    if ( Spline->Keys.NumItems() == 1 )
      value = Spline->Keys[ 0 ]->Key.Value[ 0 ];

    if ( Spline->Keys.NumItems() > 1 ) // go through every time position on the spline and check if the value is the same
  */
  {
    sp->CalculateValue( 0 );
    value = sp->Value[ 0 ];
    for ( int x = 0; x < 512; x++ )
    {
      sp->CalculateValue( x / 512.0f );
      D3DXFLOAT16 v2 = sp->Value[ 0 ];
      if ( value != v2 )
      {
        isConstant = false;
        break;
      }
    }
  }

  constValue = value;
  return isConstant;
}

int uniqueNonSpecialSplines = 0;
int splineTypeCounts[ 16 ];

int floatSplineWaveforms[ 16 ];
int floatSplineInterpolations[ 16 ];
int quatSplineInterpolations[ 16 ];

void ExportSpline( CphxSpline_Tool_float16* Spline )
{
  SymbolNameScoper sym( "Splines" );

  SplineCount++;

  CphxSpline_float16* sp = (CphxSpline_float16*)Spline->Spline;

  // determine if the spline is a constant spline first

  if ( sp->Waveform == WAVEFORM_NONE )
  {
    DEFAULTSPLINETYPE defType = DEFAULTSPLINE_NOT_DEFAULT;

    D3DXFLOAT16 value = 0;
    bool isConstant = true;

    /*
        if ( !Spline->Keys.NumItems() )
          value = sp->Value[ 0 ];
        if ( Spline->Keys.NumItems() == 1 )
          value = Spline->Keys[ 0 ]->Key.Value[ 0 ];
    */

    //if ( Spline->Keys.NumItems() > 1 ) // go through every time position on the spline and check if the value is the same
    {
      sp->CalculateValue( 0 );
      value = sp->Value[ 0 ];
      for ( int x = 0; x < 512; x++ )
      {
        sp->CalculateValue( x / 512.0f );
        D3DXFLOAT16 v2 = sp->Value[ 0 ];
        if ( value != v2 )
        {
          isConstant = false;
          break;
        }
      }
    }

    if ( isConstant )
    {
      defType = DEFAULTSPLINE_CONSTANTVALUE;

      constantSplineCount++;

      if ( value == D3DXFLOAT16( 0.0f ) )
      {
        defType = DEFAULTSPLINE_ZERO;
        defaultSplineCount++;
      }
      if ( value == D3DXFLOAT16( 1.0f ) )
      {
        defType = DEFAULTSPLINE_ONE;
        defaultSplineCount++;
      }

      if ( Spline->Keys.NumItems() > 1 )
        complexConstantSplineCount++;

      unsigned char c = defType;
      WriteByte( Stream_SplineDescriptors, c, SplineDataSize );

      if ( defType == DEFAULTSPLINE_CONSTANTVALUE )
        Write( Stream_SplineKeyValues, &value, 2, SplineDataSize );

      return;
    }
  }

  // do this here so constant splines are ignored
  floatSplineInterpolations[ sp->Interpolation ]++;
  floatSplineWaveforms[ sp->Waveform ]++;

  unsigned char c = sp->Interpolation << 2;
  c |= sp->Loop << 4;
  c |= sp->Waveform << 5;
  WriteByte( Stream_SplineDescriptors, c, SplineDataSize );

  //MINIMALSPLINESETUPDATA m;
  //m.Loop = sp->Loop;
  //m.Interpolation = sp->Interpolation;
  //m.Waveform = sp->Waveform;
  //
  //WriteByte(*(unsigned char*)&m, SplineDataSize);

  //WriteByte(Stream_SplineDescriptors, sp->Loop, SplineDataSize); //loop info
  //WriteByte(Stream_SplineDescriptors, sp->Interpolation, SplineDataSize); //interpolation type - todo: merge interpolation with waveform in export
  //WriteByte(Stream_SplineDescriptors, sp->Waveform, SplineDataSize); //waveform type
  if ( sp->Waveform != WAVEFORM_NONE )
  {
    WriteByte( Stream_SplineDescriptors, sp->MultiplicativeWaveform, SplineDataSize );
    Write( Stream_SplineDescriptors, &sp->WaveformAmplitude, 2, SplineDataSize );
    Write( Stream_SplineDescriptors, &sp->WaveformFrequency, 2, SplineDataSize );
    if ( sp->Waveform == WAVEFORM_NOISE )
      WriteByte( Stream_SplineDescriptors, sp->RandSeed, SplineDataSize );
  }

  //special case of 1 key but linear data and no waveform
  if ( Spline->Keys.NumItems() == 1 && sp->Interpolation != INTERPOLATION_BEZIER && sp->Waveform == WAVEFORM_NONE )
  {
    splineHasZeroKeyExport = true;
    WriteByte( Stream_SplineKeyCounts, 0, SplineDataSize );
    D3DXFLOAT16 v = Spline->Keys[ 0 ]->Key.Value[ 0 ];
    Write( Stream_SplineKeyValues, &v, 2, SplineDataSize );
    return;
  }

  WriteByte( Stream_SplineKeyCounts, Spline->Keys.NumItems(), SplineDataSize ); //key count
  if ( !Spline->Keys.NumItems() )
  {
    splineHasZeroKeyExport = true;
    D3DXFLOAT16 v = Spline->Spline->Value[ 0 ];
    Write( Stream_SplineKeyValues, &v, 2, SplineDataSize );
  }

  for ( TS32 x = 0; x < Spline->Keys.NumItems(); x++ )
  {
    WriteByte( Stream_SplineKeyTimes, Spline->Keys[ x ]->Key.t, SplineDataSize ); //key position
    Write( Stream_SplineKeyValues, &Spline->Keys[ x ]->Key.Value, 2, SplineDataSize );

    if ( sp->Interpolation == INTERPOLATION_BEZIER )
    {
      WriteByte( Stream_SplineKeyTimes, Spline->Keys[ x ]->Key.controlpositions[ 0 ], SplineDataSize );
      WriteByte( Stream_SplineKeyTimes, Spline->Keys[ x ]->Key.controlpositions[ 1 ], SplineDataSize );
      Write( Stream_SplineKeyValues, &Spline->Keys[ x ]->Key.controlvalues[ 0 ], 2, SplineDataSize );
      Write( Stream_SplineKeyValues, &Spline->Keys[ x ]->Key.controlvalues[ 1 ], 2, SplineDataSize );
    }
  }
}

void ExportSpline( CphxSpline_Tool_Quaternion16* Spline )
{
  SplineCount++;

  CphxSpline_Quaternion16* sp = (CphxSpline_Quaternion16*)Spline->Spline;

  // determine if the spline is a constant spline first

  if ( sp->Waveform == WAVEFORM_NONE )
  {
    DEFAULTSPLINETYPE defType = DEFAULTSPLINE_NOT_DEFAULT;

    D3DXFLOAT16 value[ 4 ];
    value[ 0 ] = 0;
    value[ 1 ] = 0;
    value[ 2 ] = 0;
    value[ 3 ] = 1;
    bool isConstant = true;

    if ( !Spline->Keys.NumItems() )
      for ( int x = 0; x < 4; x++ )
        value[ x ] = sp->Value[ x ];

    if ( Spline->Keys.NumItems() == 1 )
      for ( int x = 0; x < 4; x++ )
        value[ x ] = Spline->Keys[ 0 ]->Key.Value[ x ];

    if ( Spline->Keys.NumItems() > 1 ) // go through every time position on the spline and check if the value is the same
    {
      sp->CalculateValue( 0 );

      for ( int x = 0; x < 4; x++ )
        value[ x ] = sp->Value[ x ];

      for ( int x = 0; x < 512; x++ )
      {
        sp->CalculateValue( x / 512.0f );
        for ( int y = 0; y < 4; y++ )
        {
          D3DXFLOAT16 v2 = sp->Value[ y ];
          if ( value[ y ] != v2 )
          {
            isConstant = false;
            break;
          }
        }
        if ( !isConstant )
          break;
      }
    }

    if ( isConstant )
    {
      defType = DEFAULTSPLINE_CONSTANTVALUE;
      constantSplineCount++;

      if ( value[ 0 ] == D3DXFLOAT16( 0.0f ) &&
           value[ 1 ] == D3DXFLOAT16( 0.0f ) &&
           value[ 2 ] == D3DXFLOAT16( 0.0f ) &&
           value[ 3 ] == D3DXFLOAT16( 1.0f ) )
      {
        defaultSplineCount++;
        defType = DEFAULTSPLINE_ZERO;
      }

      if ( Spline->Keys.NumItems() > 1 )
        complexConstantSplineCount++;

      unsigned char c = defType;
      WriteByte( Stream_SplineDescriptors, c, SplineDataSize );

      if ( defType == DEFAULTSPLINE_CONSTANTVALUE )
        for ( int x = 0; x < 4; x++ )
          Write( Stream_SplineKeyValues, &value[ x ], 2, SplineDataSize );

      return;
    }
  }

  quatSplineInterpolations[ sp->Interpolation ]++;

  unsigned char c = sp->Interpolation << 2;
  c |= sp->Loop << 4;
  //c |= sp->Waveform << 3; // quaternion splines don't have waveforms
  WriteByte( Stream_SplineDescriptors, c, SplineDataSize );

  //MINIMALSPLINESETUPDATA m;
  //m.Loop = sp->Loop;
  //m.Interpolation = sp->Interpolation;
  //m.Waveform = sp->Waveform;

  //WriteByte(*(unsigned char*)&m, SplineDataSize);

  //WriteByte(Stream_SplineDescriptors, sp->Loop, SplineDataSize); //loop info
  //WriteByte(Stream_SplineDescriptors, sp->Interpolation, SplineDataSize); //interpolation type - todo: merge interpolation with waveform in export

  if ( Spline->Keys.NumItems() == 1 ) //special case for one key
  {
    WriteByte( Stream_SplineKeyCounts, 0, SplineDataSize ); //key count
    for ( TS32 x = 0; x < 4; x++ ) //quaternion spline export
    {
      D3DXFLOAT16 v = Spline->Keys[ 0 ]->Key.Value[ x ];
      Write( Stream_SplineKeyValues, &v, 2, SplineDataSize );
      //WriteByte( Stream_SplineKeyValues, (char)( v * 127 ), SplineDataSize );
    }
    return;
  }

  WriteByte( Stream_SplineKeyCounts, Spline->Keys.NumItems(), SplineDataSize ); //key count
  if ( !Spline->Keys.NumItems() )
  {
    for ( TS32 x = 0; x < 4; x++ ) //quaternion spline export
    {
      D3DXFLOAT16 v = Spline->Spline->Value[ x ];
      Write( Stream_SplineKeyValues, &v, 2, SplineDataSize );
      //WriteByte( Stream_SplineKeyValues, (char)( v * 127 ), SplineDataSize );
    }
  }

  for ( TS32 x = 0; x < Spline->Keys.NumItems(); x++ )
  {
    WriteByte( Stream_SplineKeyTimes, Spline->Keys[ x ]->Key.t, SplineDataSize ); //key position
    for ( TS32 y = 0; y < 4; y++ ) //quaternion spline export
    {
      D3DXFLOAT16 v = Spline->Keys[ x ]->Key.Value[ y ];
      Write( Stream_SplineKeyValues, &v, 2, SplineDataSize );
      //WriteByte( Stream_SplineKeyValues, (char)( v * 127 ), SplineDataSize );
    }
  }
}

void ExportTexgenOp( CphxTextureOperator_Tool* op )
{
  SymbolNameScoper sym( op->BuildsTexture );

  if ( !op->Filter )
  {
    LOG_ERR( "[minimalexport] Serious export issue, operator filter missing!" );
    MinimalExportSuccessful = false;
    return;
  }

  TexgenOpCount++;

  unsigned char filter = op->Filter->GetResourceIndex() | ( op->ParentPage->IsHDR() << 7 );
  //if (op->IsRendered()) //no need for this anymore since the first x operators are generated
  //	filter = filter | 0x80;

  WriteByte( Stream_MaterialParam, filter, TexgenOpDataSize ); //filter ID

  if ( op->Filter->Filter.DataDescriptor.ParameterCount ) //parameters
    Write( Stream_Main, op->OpData.Parameters, op->Filter->Filter.DataDescriptor.ParameterCount, TexgenOpDataSize );

  if ( op->Filter->Filter.DataDescriptor.NeedsRandSeed ) //randseed
    WriteByte( Stream_Main, op->OpData.RandSeed, TexgenOpDataSize );

  for ( TU32 x = 0; x < op->Filter->Filter.DataDescriptor.InputCount; x++ ) //inputs
  {
    CphxTextureOperator_Tool* p = ( (CphxTextureOperator_Tool*)op->GetParent( PHX_TEXTUREOPERATOR, x ) );
    if ( p ) p = p->GetContentOp();
    WriteWord( Stream_TextureOperatorReference, p->GetResourceIndex(), TexgenOpDataSize );
  }

  switch ( op->Filter->Filter.DataDescriptor.LookupType )
  {
  case 0: //no extra data needed
    break;
  case 1: //image load
    TexgenImageLoad = true;
    Write( Stream_Main, &op->ImageDataSize, 4, TexgenOpDataSize );
    Write( Stream_Main, op->ImageData, op->ImageDataSize, TexgenOpDataSize );
    break;
  case 2: //text data

  {
    int oldFont = op->TextData.Font;
    op->TextData.Font = fontList.Find( EngineFontList[ oldFont ] );

    Write( Stream_Main, &op->TextData, 5, TexgenOpDataSize );

    op->TextData.Font = oldFont;
  }

    //mem.WriteByte(op->TextData.XPos);
    //mem.WriteByte(op->TextData.YPos);
    //mem.WriteByte(op->TextData.Bold);
    //mem.WriteByte(op->TextData.Italic);
    //mem.WriteByte(op->TextData.CharSpace);
    //mem.WriteByte(op->TextData.Font);
    //mem.WriteByte(op->TextData.Size);
    //mem.WriteASCIIZ(CString(op->TextData.Text));
    WriteASCIIZ( Stream_ASCIIZ, CString( op->Text ), TexgenOpDataSize );
    break;
  case 3: //spline data
    for ( TS32 x = 0; x < 4; x++ )
      ExportSpline( op->Curves[ x ] );
    break;
  }
}

void ExportMaterialParameterValue( MATERIALPARAMTYPE& type, MATERIALVALUE& value, CphxGUID& textureGUID )
{
  switch ( type )
  {
  case PARAM_FLOAT: //stored on one byte
    WriteByte( Stream_MaterialParam, (TS32)( max( 0, min( 1, value.Float ) ) * 255 ), MaterialParameterDataSize );
    break;
  case PARAM_COLOR:
    for ( TS32 x = 0; x < 4; x++ )
      WriteByte( Stream_MaterialParam, (TS32)( max( 0, min( 1, value.Color[ x ] ) ) * 255 ), MaterialParameterDataSize );
    break;
  case PARAM_ZMODE:
    WriteByte( Stream_MaterialParam, value.ZMode, MaterialParameterDataSize );
    break;
  case PARAM_ZFUNCTION:
    WriteByte( Stream_MaterialParam, value.ZFunction, MaterialParameterDataSize );
    break;
  case PARAM_FILLMODE:
    WriteByte( Stream_MaterialParam, value.Wireframe, MaterialParameterDataSize );
    break;
  case PARAM_CULLMODE:
    WriteByte( Stream_MaterialParam, value.CullMode, MaterialParameterDataSize );
    break;
  case PARAM_RENDERPRIORITY:
    WriteByte( Stream_MaterialParam, value.RenderPriority, MaterialParameterDataSize );
    break;
  case PARAM_BLENDMODE0:
  case PARAM_BLENDMODE1:
  case PARAM_BLENDMODE2:
  case PARAM_BLENDMODE3:
  case PARAM_BLENDMODE4:
  case PARAM_BLENDMODE5:
  case PARAM_BLENDMODE6:
  case PARAM_BLENDMODE7:
    WriteByte( Stream_MaterialParam, value.BlendMode + 0x11, MaterialParameterDataSize );
    break;
  case PARAM_TEXTURE0:
  case PARAM_TEXTURE1:
  case PARAM_TEXTURE2:
  case PARAM_TEXTURE3:
  case PARAM_TEXTURE4:
  case PARAM_TEXTURE5:
  case PARAM_TEXTURE6:
  case PARAM_TEXTURE7:
  case PARAM_RENDERTARGET:
  {
    CphxTextureOperator_Tool* op = Project.GetTexgenOp( textureGUID );
    CphxRenderTarget_Tool* rt = Project.GetRenderTarget( textureGUID );
    if ( !op && !rt )
    {
      //LOG_ERR("[minimalexport] Fatal error: texture/rt not found while exporting material parameter (%s)",Param->TextureGUID.GetString());

      //export no texture set
      WriteWord( Stream_TextureOperatorReference, 0xffff, MaterialParameterDataSize );
      return;
    }

    if ( op )
    {
      op = op->GetContentOp();
      op->SetRenderRequirement();
      if ( !op )
      {
        LOG_ERR( "[minimalexport] Fatal error: texture content operator empty while exporting material parameter" );
        MinimalExportSuccessful = false;
        return;
      }
      WriteWord( Stream_TextureOperatorReference, op->GetResourceIndex(), MaterialParameterDataSize );
    }
    else
      if ( rt )
        WriteWord( Stream_TextureOperatorReference, rt->GetResourceIndex() | 0x8000, MaterialParameterDataSize );
  }
  break;
  case PARAM_LTC1:
    hasLTC1 = true;
    break;
  case PARAM_LTC2:
    hasLTC2 = true;
    break;
  }
}

void ExportMaterialParameterValue( CphxMaterialParameter_Tool* Param )
{
  MaterialParameterDataCount++;
  ExportMaterialParameterValue( Param->Parameter.Type, Param->Parameter.Value, Param->TextureGUID );
}

void ExportMaterialParamDefaultValue( CphxMaterialParameter_Tool* Param )
{
  MaterialParameterDataCount++;
  ExportMaterialParameterValue( Param->Parameter.Type, Param->DefaultValue, Param->TextureGUID );
}

void ExportMaterialParams( CphxMaterialParameterBatch_Tool& Params, CString& headerPrefix, CStreamWriter& headerFile )
{
  SymbolNameScoper sym( "MaterialParameters" );

  MaterialParamCount += Params.Parameters.NumItems();
  WriteByte( Stream_Main, Params.Parameters.NumItems(), MaterialParamSize ); //param count
  for ( TS32 x = 0; x < Params.Parameters.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool* p = Params.Parameters[ x ];

    CString paramName = ToMacroCompatible( p->Name );
    headerFile.WriteFormat( "#define PARAM_%s_%-30s %d\n", headerPrefix.GetPointer(), paramName.GetPointer(), x );

    WriteByte( Stream_Main, p->Parameter.Scope, MaterialParamSize ); //parameter scope
    WriteByte( Stream_Main, p->Parameter.Type, MaterialParamSize ); //parameter type
    if ( p->Parameter.Scope == PARAM_CONSTANT )
      ExportMaterialParameterValue( p );
    /*
        else
          ExportMaterialParamDefaultValue( p );
    */
  };
}

void ExportModelFilters( CphxModelObject_Tool* o, CphxModel_Tool* m )
{
  SymbolNameScoper sym( "Filters" );
  //filter export
  TS32 filtercnt = 0;
  for ( TS32 x = 0; x < o->GetFilterCount(); x++ )
    if ( o->GetFilter( x )->Enabled )
    {
      filtercnt++;
      ModelFilterCount++;
    }

  WriteByte( Stream_ModelFilterCount, filtercnt, ModelFilterSize ); //filter count
  for ( TS32 x = 0; x < o->GetFilterCount(); x++ )
  {
    CphxMeshFilter_Tool* f = o->GetFilter( x );
    if ( !f->Enabled ) continue;

    UsedModelFilters[ f->Filter ]++;
    WriteByte( Stream_ModelFilterType, f->Filter, ModelFilterSize ); //filter type

    if ( MeshFilterParameterCounts[ f->Filter ] ) //filter parameters
      Write( Stream_ModelFilterData, f->Parameters, MeshFilterParameterCounts[ f->Filter ], ModelFilterSize );

    switch ( f->Filter )
    {
    case ModelFilter_UVMap:
    {
      if ( f->Parameters[ 1 ] & 0xf0 )
        hasUVClip = true;
    }
    case ModelFilter_MapXForm:
      Write( Stream_ModelTransformation, f->srt, 20, ModelFilterSize );
      break;
    case ModelFilter_TintMeshShape:
      Write( Stream_ModelTransformation, f->srt, 20, ModelFilterSize );
      break;
    case ModelFilter_Replicate:
      Write( Stream_ModelTransformation, f->srt, 24, ModelFilterSize );
      break;
    case ModelFilter_TintMesh:
      if ( !f->Texture )
      {
        LOG_ERR( "[minimalexport] Fatal error: mesh tint missing texture on object %s in model %s!", o->GetName().GetPointer(), m->GetName().GetPointer() );
        MinimalExportSuccessful = false;
      }
      else
      {
        CphxTextureOperator_Tool* t = f->Texture;
        t = t->GetContentOp();
        if ( !t )
        {
          LOG_ERR( "[minimalexport] Fatal error: mesh tint load op has no parent on object %s in model %s!", o->GetName().GetPointer(), m->GetName().GetPointer() );
          MinimalExportSuccessful = false;
        }
        else
        {
          t->SetRenderRequirement();
          WriteWord( Stream_TextureOperatorReference, t->GetResourceIndex(), ModelFilterSize );
        }
      }
      break;
    default:
      break;
    }
  }
}

void ExportMaterialAndVariables( CphxMaterial_Tool* mat, CString Name )
{
  SymbolNameScoper sym( "MaterialParameters" );
  if ( !mat || mat == DefaultMaterial )
  {
    if ( NoMaterial )
      mat = NoMaterial;
    else
    {
      LOG_ERR( "[minimalexport] Fatal error: Material not set for '%s'!", Name.GetPointer() );
      MinimalExportSuccessful = false;
      return;
    }
  }

  WriteByte( Stream_MaterialID, mat->GetResourceIndex(), MaterialDataPackCount ); //material index

  for ( TS32 x = 0; x < mat->Techniques.NumItems(); x++ ) //for each tech
  {
    CphxMaterialTechnique_Tool* t = mat->Techniques[ x ];
    for ( TS32 y = 0; y < t->TechParameters.Parameters.NumItems(); y++ ) //tech level params
    {
      if ( t->TechParameters.Parameters[ y ]->Parameter.Scope == PARAM_VARIABLE )
      {
        hasTechParams = true;
        /*
                bool isDefault = t->TechParameters.Parameters[ y ]->IsDefault();
                WriteByte( Stream_MaterialParamDefault, isDefault, MaterialParamDefaultCount);
                if ( !isDefault )
        */
        ExportMaterialParameterValue( t->TechParameters.Parameters[ y ] );
      }
    }
    for ( TS32 y = 0; y < t->Passes.NumItems(); y++ ) //for each pass
    {
      for ( TS32 z = 0; z < t->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ ) //pass level params
      {
        if ( t->Passes[ y ]->PassParameters.Parameters[ z ]->Parameter.Scope == PARAM_VARIABLE )
        {
          /*
                    bool isDefault = t->Passes[ y ]->PassParameters.Parameters[ z ]->IsDefault();
                    WriteByte( Stream_MaterialParamDefault, isDefault, MaterialParamDefaultCount );
                    if ( !isDefault )
          */
          ExportMaterialParameterValue( t->Passes[ y ]->PassParameters.Parameters[ z ] );
        }
      }
    }
  }
}

void ExportModelObject( CphxModelObject_Tool* o, CphxModel_Tool* m )
{
  ModelObjectCount++;

  TU8 modelType = o->GetPrimitive();

  WriteByte( Stream_ModelPrimitives, modelType, ModelObjectDataSize ); //mesh primitive
  UsedModelPrimitives[ o->GetPrimitive() ]++;

  D3DXMATRIX mat = o->GetMatrix();

  for ( int x = 0; x < 3; x++ )
  {
    mat.m[ 3 ][ x ] = 0;
    mat.m[ x ][ 3 ] = 0;
  }

  mat.m[ 3 ][ 3 ] = 1;

  D3DXMATRIX matinv;
  D3DXMatrixTranspose( &matinv, &mat );
  D3DXMatrixMultiply( &matinv, &mat, &matinv );

  TBOOL ortho = true;

  D3DXVECTOR3 v1( matinv.m[ 0 ] );
  D3DXVECTOR3 v2( matinv.m[ 1 ] );
  D3DXVECTOR3 v3( matinv.m[ 2 ] );

  D3DXVec3Normalize( &v1, &v1 );
  D3DXVec3Normalize( &v2, &v2 );
  D3DXVec3Normalize( &v3, &v3 );

  if ( abs( v1.y ) > 0.005 ) ortho = false;
  if ( abs( v1.z ) > 0.005 ) ortho = false;

  if ( abs( v2.x ) > 0.005 ) ortho = false;
  if ( abs( v2.z ) > 0.005 ) ortho = false;

  if ( abs( v3.x ) > 0.005 ) ortho = false;
  if ( abs( v3.y ) > 0.005 ) ortho = false;

  //D3DXMATRIX i;
  //D3DXMatrixIdentity( &i );

  //TBOOL same = true;
  //for ( TS32 x = 0; x < 3; x++ )
  //  for ( TS32 y = 0; y < 3; y++ )
  //    if ( abs( matinv.m[ x ][ y ] - i.m[ x ][ y ] ) > 0.001 )
  //      same = false;

  D3DXVECTOR3 scale;
  D3DXQUATERNION rotation;
  D3DXVECTOR3 translation;
  mat = o->GetMatrix();
  D3DXMatrixDecompose( &scale, &rotation, &translation, &mat );

  TBOOL SaveTransform = true;
  TBOOL SaveFullMatrix = true;
  TBOOL SaveScale = false;
  TBOOL SaveRotation = false;

  for ( TS32 x = 9; x < 12; x++ )
    if ( o->GetObject()->TransformationF16[ x ] != D3DXFLOAT16( 0 ) )
      SaveTransform = true;

  if ( ortho )
  {
    SaveFullMatrix = false;
    //LOG_DBG( "MATRIX IS ORTHOGONAL: %f %f %f | %f %f %f | %f %f %f %f", translation.x, translation.y, translation.z, scale.x, scale.y, scale.z, rotation.x, rotation.y, rotation.z, rotation.w );

    if ( abs( scale.x - 1 ) > 0.001 || abs( scale.y - 1 ) > 0.001 || abs( scale.z - 1 ) > 0.001 )
      SaveScale = true;

    if ( abs( rotation.x ) > 0.001 || abs( rotation.y ) > 0.001 || abs( rotation.z ) > 0.001 || abs( rotation.w - 1 ) > 0.001 )
      SaveRotation = true;
  }
  else
  {
    //LOG_DBG( "-- MATRIX IS NOT ORTHOGONAL: %f %f %f | %f %f %f | %f %f %f %f", translation.x, translation.y, translation.z, scale.x, scale.y, scale.z, rotation.x, rotation.y, rotation.z, rotation.w );
  }

  //TU8 MatrixData = SaveTransform | ( SaveRotation << 1 ) | ( SaveScale << 2 ) | ( SaveFullMatrix << 3 );
  //WriteByte( Stream_ModelParameters, MatrixData, ModelObjectDataSize );

  //if ( SaveFullMatrix )
  //  Write( Stream_ModelTransformation, o->GetObject()->TransformationF16, 18, ModelObjectDataSize ); //orientation

  //if ( SaveTransform )
  //  Write( Stream_ModelTransformation, o->GetObject()->TransformationF16 + 9, 6, ModelObjectDataSize ); //position

  //if ( SaveScale )
  //{
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)scale.x, 2, ModelObjectDataSize ); //orientation
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)scale.y, 2, ModelObjectDataSize ); //orientation
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)scale.z, 2, ModelObjectDataSize ); //orientation
  //}

  //if ( SaveRotation )
  //{
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)rotation.x, 2, ModelObjectDataSize ); //orientation
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)rotation.y, 2, ModelObjectDataSize ); //orientation
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)rotation.z, 2, ModelObjectDataSize ); //orientation
  //  Write( Stream_ModelTransformation, &(D3DXFLOAT16)rotation.w, 2, ModelObjectDataSize ); //orientation
  //}

  Write( Stream_ModelTransformation, o->GetObject()->TransformationF16, 24, ModelObjectDataSize ); //transformation

  unsigned char odata = o->GetParameters()[ 0 ];

  // Update tree species index parameter
  if ( o->GetPrimitive() == Mesh_Tree || o->GetPrimitive() == Mesh_TreeLeaves )
  {
    CphxTreeSpecies* spec = Project.GetTreeSpecies( o->ParentGUIDS[ 0 ] );
    if ( !spec )
    {
      LOG_ERR( "Minimal Export error: TREE SPECIES NOT SPECIFIED." );
      return;
    }
    o->GetParameters()[ 0 ] = spec->GetResourceIndex();
  }

  if ( o->GetPrimitive() == Mesh_Text )
    o->GetParameters()[ 0 ] = fontList.Find( EngineFontList[ o->GetParameters()[ 0 ] ] );

  if ( MeshPrimitiveParameterCounts[ o->GetPrimitive() ] ) //mesh primitive parameters
  {
    if ( o->GetPrimitive() != Mesh_Scatter )
      Write( Stream_ModelParameters, o->GetParameters(), MeshPrimitiveParameterCounts[ o->GetPrimitive() ], ModelObjectDataSize );
    else
    {
      if ( o->GetParameters()[ 3 ] > 0 )
        scatterVertexSupport = true;
      if ( o->GetParameters()[ 4 ] > 0 )
        scatterEdgeSupport = true;
      if ( o->GetParameters()[ 5 ] > 0 )
        scatterPolySupport = true;
      if ( o->GetParameters()[ 5 ] > 0 && o->GetParameters()[ 6 ] > 1 )
        scatterPolyMultiSupport = true;

      if ( ( (PHXSCATTERORIENTATIONTYPE)o->GetParameters()[ 8 ] ) == flareScatterOrientation_Original )
        hasScatterOrientationOriginal = true;
      if ( ( (PHXSCATTERORIENTATIONTYPE)o->GetParameters()[ 8 ] ) == flareScatterOrientation_Normal )
        hasScatterOrientationNormal = true;
      if ( ( (PHXSCATTERORIENTATIONTYPE)o->GetParameters()[ 8 ] ) == flareScatterOrientation_NormalRotate )
        hasScatterOrientationNormalRotate = true;
      if ( ( (PHXSCATTERORIENTATIONTYPE)o->GetParameters()[ 8 ] ) == flareScatterOrientation_FullRotate )
        hasScatterOrientationFullRotate = true;

      Write( Stream_ModelParameters, o->GetParameters(), MeshPrimitiveParameterCounts[ o->GetPrimitive() ] - 2, ModelObjectDataSize );
      Write( Stream_ModelParameters, &o->FloatParameter, 2, ModelObjectDataSize );
    }
  }

  o->GetParameters()[ 0 ] = odata;

  //special params:
  TS32 idx = 0;
  TS32 pcount = 0;
  switch ( o->GetPrimitive() )
  {
  case Mesh_Text:
    WriteASCIIZ( Stream_ASCIIZ, o->GetText(), ModelObjectDataSize );
    break;
  case Mesh_Spline:
    //todo: spline export
    break;
  case Mesh_Stored:
  {
    CphxModelObject_Mesh* mesh = (CphxModelObject_Mesh*)o->GetObject();

    Write( Stream_ModelParameters, &mesh->StoredVertexCount, 4, ModelObjectDataSize );

    for ( int x = 0; x < mesh->StoredVertexCount; x++ )
    {
      Write( Stream_ModelParameters, &mesh->StoredVertices[ x ].Position.x, sizeof( float ), ModelObjectDataSize );
      Write( Stream_ModelParameters, &mesh->StoredVertices[ x ].Position.y, sizeof( float ), ModelObjectDataSize );
      Write( Stream_ModelParameters, &mesh->StoredVertices[ x ].Position.z, sizeof( float ), ModelObjectDataSize );
      Write( Stream_ModelParameters, &mesh->StoredVertices[ x ].Normal.x, sizeof( float ), ModelObjectDataSize );
      Write( Stream_ModelParameters, &mesh->StoredVertices[ x ].Normal.y, sizeof( float ), ModelObjectDataSize );
      Write( Stream_ModelParameters, &mesh->StoredVertices[ x ].Normal.z, sizeof( float ), ModelObjectDataSize );
    }

    Write( Stream_ModelParameters, &mesh->StoredPolyCount, 4, ModelObjectDataSize );

    for ( int x = 0; x < mesh->StoredPolyCount; x++ )
    {
      TU8 vc = mesh->StoredPolygons[ x ].VertexCount;
      WriteByte( Stream_ModelParameters, vc, ModelObjectDataSize );
      for ( int y = 0; y < vc; y++ )
      {
        Write( Stream_ModelParameters, &mesh->StoredPolygons[ x ].VertexIDs[ y ], 4, ModelObjectDataSize ); //index
        Write( Stream_ModelParameters, &mesh->StoredPolygons[ x ].Normals[ y ].x, sizeof( float ), ModelObjectDataSize ); //normal
        Write( Stream_ModelParameters, &mesh->StoredPolygons[ x ].Normals[ y ].y, sizeof( float ), ModelObjectDataSize ); //normal
        Write( Stream_ModelParameters, &mesh->StoredPolygons[ x ].Normals[ y ].z, sizeof( float ), ModelObjectDataSize ); //normal
        Write( Stream_ModelParameters, &mesh->StoredPolygons[ x ].Texcoords[ y ][ 0 ].x, sizeof( float ), ModelObjectDataSize ); //uv x
        Write( Stream_ModelParameters, &mesh->StoredPolygons[ x ].Texcoords[ y ][ 0 ].y, sizeof( float ), ModelObjectDataSize ); //uv y
      }
    }
  }
  break;
  case Mesh_StoredMini:
  {
    CphxModelObject_Mesh* mesh = (CphxModelObject_Mesh*)o->GetObject();
    WriteByte( Stream_ModelParameters, o->miniModelExportIndex, ModelObjectDataSize );
  }
  break;
  case Mesh_Copy:
    pcount = 1;
    break;
  case Mesh_Clone:
  case Mesh_Merge:
  {
    CphxModelObject_Tool* c = (CphxModelObject_Tool*)o;
    WriteByte( Stream_ModelReferences, c->ClonedObjects.NumItems(), ModelObjectDataSize ); //clone count
    for ( TS32 x = 0; x < c->ClonedObjects.NumItems(); x++ )
    {
      idx = m->GetObjectIndex( c->ClonedObjects[ x ] );
      if ( idx < 0 )
      {
        LOG_ERR( "[minimalexport] Fatal error: object parent not found in scene for object %s in model %s!", o->GetName().GetPointer(), m->GetName().GetPointer() );
        MinimalExportSuccessful = false;
      }
      WriteByte( Stream_ModelReferences, idx, ModelObjectDataSize );
    }
  }
  break;
  case Mesh_Scatter:
  case Mesh_Loft:
    pcount = 2;
    break;
  }

  for ( TS32 x = 0; x < pcount; x++ ) //write parent object indices
  {
    idx = m->GetObjectIndex( o->ParentGUIDS[ x ] );
    if ( idx < 0 )
    {
      LOG_ERR( "[minimalexport] Fatal error: object parent not found in scene for object %s in model %s!", o->GetName().GetPointer(), m->GetName().GetPointer() );
      MinimalExportSuccessful = false;
    }
    WriteByte( Stream_ModelReferences, idx, ModelObjectDataSize );
  }

  //material param export

  if ( o->GetPrimitive() == Mesh_Line ||
       o->GetPrimitive() == Mesh_Arc ||
       o->GetPrimitive() == Mesh_Spline ||
       o->GetPrimitive() == Mesh_Clone )
    return; //these primitives don't need materials

  o->UpdateMaterialState();
  ExportMaterialAndVariables( o->GetMaterial(), "Model Object " + o->GetName() );

  //CphxMaterial_Tool *mat = o->GetMaterial();

  //if (!mat || mat == DefaultMaterial)
  //{
  //	if (NoMaterial)
  //		mat = NoMaterial;
  //	else
  //	{
  //		LOG_ERR("[minimalexport] Fatal error: Material not set for model object %s!", o->GetName().GetPointer());
  //		MinimalExportSuccessful = false;
  //		return;
  //	}
  //}
  //mem.WriteByte(o->GetMaterial()->GetResourceIndex()); //material index

  //o->UpdateMaterialState();

  //for (TS32 x = 0; x < o->GetMaterial()->Techniques.NumItems(); x++) //for each tech
  //{
  //	CphxMaterialTechnique_Tool *t = o->GetMaterial()->Techniques[x];
  //	for (TS32 y = 0; y < t->TechParameters.Parameters.NumItems(); y++) //tech level params
  //	{
  //		if (t->TechParameters.Parameters[y]->Parameter.Scope == PARAM_VARIABLE)
  //			ExportMaterialParameterValue(t->TechParameters.Parameters[y]);
  //	}
  //	for (TS32 y = 0; y < t->Passes.NumItems(); y++) //for each pass
  //	{
  //		for (TS32 z = 0; z < t->Passes[y]->PassParameters.Parameters.NumItems(); z++) //pass level params
  //		{
  //			if (t->Passes[y]->PassParameters.Parameters[z]->Parameter.Scope == PARAM_VARIABLE)
  //				ExportMaterialParameterValue(t->Passes[y]->PassParameters.Parameters[z]);
  //		}
  //	}
  //}
}

int clipSplineTypes[ Spline_Count ];

void ExportClipSpline( CphxObject_Tool* o, int clipId, CphxClipSpline_Tool* s )
{
  WriteByte( Stream_Main, s->MinimalSpline.Type, ClipSplineCount ); //spline type

  //if (s->MinimalSpline.Type == Spline_MaterialParam) //export material parameter link data for spline
  //{

  //}

  CphxSpline_Tool* spline = s->Spline;

  if ( s->MinimalSpline.Type == Spline_SubScene_Clip && o->GetObjectType()==Object_SubScene )
  {
    CphxObject_SubScene_Tool* sub = (CphxObject_SubScene_Tool*)o;
    CphxScene_Tool* target = sub->GetSubScene( clipId );
    spline = new CphxSpline_Tool_float16();
    s->Spline->CopyTo( spline );
    
    for ( int x = 0; x < spline->Keys.NumItems(); x++ )
    {
      D3DXFLOAT16 old = spline->Keys[ x ]->Key.Value[ 0 ];
      spline->Keys[ x ]->Key.Value[ 0 ] = target->GetClipByIndex( (int)spline->Keys[ x ]->Key.Value[ 0 ] )->GetResourceIndex();
      D3DXFLOAT16 newVal = spline->Keys[ x ]->Key.Value[ 0 ];      
      //LOG_NFO( "Changed subscene spline key value from %f to %f", (float)old, float( newVal ) );
    }

    spline->UpdateSplineKeys();
  }

  clipSplineTypes[ s->MinimalSpline.Type ]++;

  if ( s->MinimalSpline.Type == Spline_Rotation )
    ExportSpline( (CphxSpline_Tool_Quaternion16*)spline );
  else
    ExportSpline( (CphxSpline_Tool_float16*)spline );

  if ( s->MinimalSpline.Type == Spline_SubScene_Clip && o->GetObjectType() == Object_SubScene )
    delete spline;

}

int materialSplineCount = 0;
int defaultMaterialSplineCount = 0;

void ExportMaterialParamSplineBatch( CphxMaterialParameterBatch_Tool* t, CphxMaterialSplineBatch_Tool* batch, void* GroupingData, TS32 ClipID, CString& prefix, CStreamWriter& headerFile )
{
  unsigned int splineOrder = 0;
  for ( TS32 x = 0; x < t->Parameters.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool* p = t->Parameters[ x ];
    if ( p->Parameter.Scope == PARAM_ANIMATED )
    {
      if ( p->Parameter.Type == PARAM_FLOAT || p->Parameter.Type == PARAM_COLOR || ( p->Parameter.Type == PARAM_PARTICLELIFEFLOAT && !ClipID ) )
      {
        headerFile.WriteFormat( "#define SPLINE_%s_%-30s %d\n", prefix.GetPointer(), ToMacroCompatible( p->Name ).GetPointer(), splineOrder++ );

        CphxMaterialSpline_Tool* s = batch->FindSpline( p, GroupingData );
        if ( !s )
        {
          LOG_WARN( "[minimalexport] Warning: didn't find spline for %s. Dumping constant spline with default value instead.", p->Name.GetPointer() );

          CphxSpline_Tool_float16 splines[ 4 ];
          for ( TS32 z = 0; z < 4; z++ )
            for ( TS32 i = 0; i < 4; i++ )
              splines[ z ].Spline->Value[ i ] = p->DefaultValue.Color[ i ];

          materialSplineCount++;
          clipSplineTypes[ Spline_MaterialParam ]++;
          ExportSpline( &splines[ 0 ] );
          if ( p->Parameter.Type == PARAM_COLOR )
          {
            for ( TS32 z = 1; z < 4; z++ )
            {
              clipSplineTypes[ Spline_MaterialParam ]++;
              materialSplineCount++;
              ExportSpline( &splines[ z ] );
            }
          }

          continue;
        }

        //LOG_NFO("[minimalexport] Exporting material param spline for %s", p->Name.GetPointer());

        //check if the splines are the default value here!!!!!!!!!!

        materialSplineCount++;

        D3DXFLOAT16 constVal = 0;
        bool isConst = IsSplineConstant( &s->Splines[ 0 ], constVal );
        if ( isConst && constVal == (D3DXFLOAT16)p->DefaultValue.Color[ 0 ] )
          defaultMaterialSplineCount++;

        clipSplineTypes[ Spline_MaterialParam ]++;
        ExportSpline( &s->Splines[ 0 ] );

        if ( p->Parameter.Type == PARAM_COLOR )
        {
          for ( TS32 z = 1; z < 4; z++ )
          {
            bool isConst = IsSplineConstant( &s->Splines[ z ], constVal );
            if ( isConst && constVal == (D3DXFLOAT16)p->DefaultValue.Color[ z ] )
              defaultMaterialSplineCount++;

            materialSplineCount++;
            clipSplineTypes[ Spline_MaterialParam ]++;
            ExportSpline( &s->Splines[ z ] );
          }
        }

      }
      else
      {
        if ( p->Parameter.Type == PARAM_PARTICLELIFEFLOAT && ClipID )
        {
          LOG_WARN( "[minimalexport] Warning: Particle life spline in multiple clips" );
        }
        else
        {
          LOG_ERR( "[minimalexport] Error: Trying to animate non animatable parameter type - (%s)", p->Name.GetPointer() );
          MinimalExportSuccessful = false;
        }
      }
    }
  }
}

void ExportTechSplineBatch( CphxMaterialTechnique_Tool* t, CphxMaterialSplineBatch_Tool* batch, void* GroupingData, TS32 ClipID, CString& prefix, CStreamWriter& headerFile )
{
  ExportMaterialParamSplineBatch( &t->TechParameters, batch, GroupingData, ClipID, prefix, headerFile );
  for ( TS32 x = 0; x < t->Passes.NumItems(); x++ )
    ExportMaterialParamSplineBatch( &t->Passes[ x ]->PassParameters, batch, GroupingData, ClipID, prefix, headerFile );
}

void ExportMaterialSplineBatch( CphxObject_Model_Tool* b, CphxMaterialSplineBatch_Tool* batch, TS32 ClipID, CStreamWriter& headerFile )
{
  if ( !b->Model )
  {
    LOG_ERR( "[minimalexport] Fatal error: %s has no model.", b->GetName().GetPointer() );
    MinimalExportSuccessful = false;
    return;
  }

  //for each object in the model
  for ( TS32 x = 0; x < b->Model->GetObjectCount(); x++ )
  {
    CphxModelObject_Tool* o = b->Model->GetObjectByIndex( x );

    //see if it's a clone
    if ( o->GetPrimitive() == Mesh_Clone ) continue;

    CphxMaterial_Tool* m = o->GetMaterial();

    //see if it has a material
    if ( !m || m == DefaultMaterial )
    {
      if ( NoMaterial ) m = NoMaterial;
      else
      {
        LOG_ERR( "[minimalexport] Fatal error: %s in %s has no material set.", o->GetName().GetPointer(), b->GetName().GetPointer() );
        MinimalExportSuccessful = false;
        return;
      }
    }

    //export splines for each tech in the material
    for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
      ExportTechSplineBatch( o->GetMaterial()->Techniques[ y ], batch, o, ClipID, ToMacroCompatible( b->GetName() ), headerFile );
  }
}

void ExportMaterialSplineBatch( CphxObject_ParticleEmitter_CPU_Tool* o, CphxMaterialSplineBatch_Tool* batch, TS32 ClipID, CStreamWriter& headerFile )
{
  CphxMaterial_Tool* m = o->GetMaterial();

  //see if it has a material
  if ( ( !m || m == DefaultMaterial ) && !o->EmitedObject && !o->EmitedScene )
  {
    if ( NoMaterial ) m = NoMaterial;
    else
    {
      LOG_ERR( "[minimalexport] Fatal error: %s has no material set.", o->GetName().GetPointer() );
      MinimalExportSuccessful = false;
      return;
    }
  }

  //export splines for each tech in the material
  if ( !o->EmitedObject && !o->EmitedScene )
    for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
      ExportTechSplineBatch( o->GetMaterial()->Techniques[ y ], batch, NULL, ClipID, ToMacroCompatible( o->GetName() ), headerFile );

  // export material splines for emitted mesh object
  if ( o->EmitedObject )
  {
    //for each object in the model
    for ( TS32 x = 0; x < o->EmitedObject->GetObjectCount(); x++ )
    {
      CphxModelObject_Tool* b = o->EmitedObject->GetObjectByIndex( x );

      //see if it's a clone
      if ( b->GetPrimitive() == Mesh_Clone ) continue;

      CphxMaterial_Tool* m = b->GetMaterial();

      //see if it has a material
      if ( !m || m == DefaultMaterial )
      {
        if ( NoMaterial ) m = NoMaterial;
        else
        {
          LOG_ERR( "[minimalexport] Fatal error: %s in %s has no material set.", b->GetName().GetPointer(), b->GetName().GetPointer() );
          MinimalExportSuccessful = false;
          return;
        }
      }

      //export splines for each tech in the material
      for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
        ExportTechSplineBatch( b->GetMaterial()->Techniques[ y ], batch, b, ClipID, ToMacroCompatible( b->GetName() ), headerFile );
    }
  }
}


void ExportSceneObject( CphxObject_Tool* o, CphxScene_Tool* s, CString& sceneName, CString& objName, CStreamWriter& headerFile )
{
  UsedScenePrimitives[ o->GetObjectType() ]++;

  SceneObjectCount++;
  WriteByte( Stream_Main, o->GetObjectType(), SceneObjectDataSize ); //object type
  if ( o->GetParentObject() )
  {
    int x = s->GetObjectIndex( o->GetParentObject() );
    WriteByte( Stream_Main, x, SceneObjectDataSize );
  }
  else WriteByte( Stream_Main, 0xff, SceneObjectDataSize ); //no parent

  switch ( o->GetObjectType() )
  {
  case Object_Model:
  {
    CphxObject_Model_Tool* t = (CphxObject_Model_Tool*)o;
    if ( !t->Model )
    {
      LOG_ERR( "[minimalexport] Fatal error: %s has no model.", o->GetName().GetPointer() );
      MinimalExportSuccessful = false;
    }
    WriteByte( Stream_Main, t->Model->GetResourceIndex(), SceneObjectDataSize );
  }
  break;
  case Object_Light:
  {
    CphxObject_Light_Tool* t = (CphxObject_Light_Tool*)o;
    WriteByte( Stream_Main, t->IsPointLight(), SceneObjectDataSize ); //is point light?
    if ( t->IsPointLight() )
      WriteByte( Stream_Main, s->GetObjectIndex( t->TargetObject ), SceneObjectDataSize ); //target index. -1 is allowed for pure point lights!!
  }
  break;
  case Object_CamEye:
  {
    TS32 idx = s->GetObjectIndex( o->TargetObject );
    if ( idx < 0 )
    {
      LOG_ERR( "[minimalexport] Fatal error: camera %s has no target in scene %s!", o->GetName().GetPointer(), s->GetName().GetPointer() );
      MinimalExportSuccessful = false;
    }
    WriteByte( Stream_Main, idx, SceneObjectDataSize );
  }
  break;
  case Object_Dummy:
    break;
  case Object_SubScene:
  {
    CphxObject_SubScene_Tool* t = (CphxObject_SubScene_Tool*)o;

    for ( int x = 0; x < s->GetClipCount(); x++ )
    {
      if ( !s->GetClipByIndex( x )->IsRequired() )
        continue;
      if ( !t->GetClip( x )->GetSubSceneTarget() )
      {
        LOG_ERR( "[minimalexport] Fatal error: subscene %s has no Scene in scene %s", o->GetName().GetPointer(), s->GetName().GetPointer() );
        MinimalExportSuccessful = false;
      }
      else
        WriteByte( Stream_Main, t->GetClip( x )->GetSubSceneTarget()->GetResourceIndex(), SceneObjectDataSize );
    }
  }
  break;
  case Object_ParticleEmitterCPU:
  {
    auto* p = (CphxObject_ParticleEmitter_CPU_Tool*)o;
    auto* e = (CphxObject_ParticleEmitter_CPU*)p->GetObject();

    unsigned char particleFlags = ( e->Aging ) | ( e->RandRotate << 1 ) | ( e->TwoDirRotate << 2 ) | ( e->RotateToDirection << 3 ) | ( e->Sort << 4 ) | ( ( p->EmitedScene != 0 ) << 5 );
    WriteByte( Stream_Main, particleFlags, SceneObjectDataSize );

/*
    WriteByte( Stream_Main, e->Aging, SceneObjectDataSize );
    WriteByte( Stream_Main, e->RandRotate, SceneObjectDataSize );
    WriteByte( Stream_Main, e->TwoDirRotate, SceneObjectDataSize );
    WriteByte( Stream_Main, e->RotateToDirection, SceneObjectDataSize );
    WriteByte( Stream_Main, ( e->Sort & 0x01 ) | ( ( p->EmitedScene != 0 ) << 1 ), SceneObjectDataSize );
*/
    WriteByte( Stream_Main, e->BufferSize, SceneObjectDataSize );
    WriteByte( Stream_Main, e->EmitterType, SceneObjectDataSize );
    WriteByte( Stream_Main, e->InnerRadius, SceneObjectDataSize );
    WriteByte( Stream_Main, e->StartCount, SceneObjectDataSize );
    WriteByte( Stream_Main, e->RandSeed, SceneObjectDataSize );
    WriteByte( Stream_Main, s->GetObjectIndex( p->TargetObject ), SceneObjectDataSize ); //target index. -1 is allowed!

    if ( e->Sort )
      hasParticleSort = true;
    if ( !p->EmitedObject && !p->EmitedScene )
      hasNormalParticle = true;
    if ( p->EmitedObject )
      hasMeshParticle = true;
    if ( p->EmitedScene )
      hasSubSceneParticle = true;

    if ( p->EmitedObject )
      WriteByte( Stream_Main, p->EmitedObject->GetResourceIndex(), SceneObjectDataSize );
    else
      if (p->EmitedScene )
        WriteByte( Stream_Main, p->EmitedScene->GetResourceIndex(), SceneObjectDataSize );
      else
        WriteByte( Stream_Main, -1, SceneObjectDataSize );

    if ( !p->EmitedObject && !p->EmitedScene )
    {
      p->UpdateMaterialState();
      ExportMaterialAndVariables( p->GetMaterial(), "Particle Emitter " + o->GetName() );
    }
  }
  break;
  case Object_ParticleGravity:
  {
    auto* p = (CphxObject_ParticleGravity_Tool*)o;
    auto* e = (CphxObject_ParticleGravity*)p->GetObject();
    WriteByte( Stream_Main, e->AreaType, SceneObjectDataSize );
    WriteByte( Stream_Main, e->Directional, SceneObjectDataSize );
  }
  break;
  case Object_ParticleDrag:
  {
    auto* p = (CphxObject_ParticleDrag_Tool*)o;
    auto* e = (CphxObject_ParticleDrag*)p->GetObject();
    WriteByte( Stream_Main, e->AreaType, SceneObjectDataSize );
  }
  break;
  case Object_ParticleTurbulence:
  {
    auto* p = (CphxObject_ParticleTurbulence_Tool*)o;
    auto* e = (CphxObject_ParticleTurbulence*)p->GetObject();
    WriteByte( Stream_Main, e->AreaType, SceneObjectDataSize );
    //WriteByte( Stream_Main, e->RandSeed, SceneObjectDataSize );
    //Write( Stream_Main, &e->Frequency, 2, SceneObjectDataSize );
    for ( int x = 0; x < s->GetClipCount(); x++ )
    {
      if ( !s->GetClipByIndex( x )->IsRequired() )
        continue;
      WriteByte( Stream_Main, p->GetClip( x )->GetRandSeed(), SceneObjectDataSize );
      WriteByte( Stream_Main, p->GetClip( x )->GetTurbulenceFreq(), SceneObjectDataSize );
    }
  }
  break;
  case Object_ParticleVortex:
  {
    auto* p = (CphxObject_ParticleVortex_Tool*)o;
    auto* e = (CphxObject_ParticleVortex*)p->GetObject();
    WriteByte( Stream_Main, e->AreaType, SceneObjectDataSize );
  }
  break;
  case Object_LogicObject:
  {
    WriteByte( Stream_Main, o->GetObject()->camCenterX, SceneObjectDataSize );
    WriteByte( Stream_Main, o->GetObject()->camCenterY, SceneObjectDataSize );
  }
  break;
  default:
    break;
  }

  // sanity check
  for ( TS32 x = 1; x < s->GetClipCount(); x++ )
  {
    CphxObjectClip_Tool* c = o->GetClip( x - 1 );
    CphxObjectClip_Tool* c2 = o->GetClip( x );
    if ( c->GetSplineCount() != c2->GetSplineCount() )
    {
      LOG_ERR( "[minimalexport] Fatal error: object %s clip spline count discrepancy!", o->GetName().GetPointer() );
      MinimalExportSuccessful = false;
      return;
    }
  }

  bool isConstant = true;

  for ( TS32 x = 0; x < s->GetClipCount(); x++ )
  {
    if ( !s->GetClipByIndex( x )->IsRequired() )
      continue;
    CphxObjectClip_Tool* c = o->GetClip( x );

    for ( TS32 y = 0; y < c->GetSplineCount(); y++ )
    {
      D3DXFLOAT16 constVal[ 4 ];
      isConstant = c->GetSplineByIndex( y )->Spline->IsConstant( constVal );

      TBOOL alwaysDefaultSpline = c->GetSplineByIndex( y )->IsDefaultSpline();

      if ( isConstant )
      {
        for ( TS32 z = 0; z < s->GetClipCount(); z++ )
        {
          if ( !s->GetClipByIndex( z )->IsRequired() )
            continue;
          D3DXFLOAT16 constVal2[ 4 ];
          CphxObjectClip_Tool* c2 = o->GetClip( z );
          CphxClipSpline_Tool* sp = c2->GetClipSpline( c->GetSplineByIndex( y )->MinimalSpline.Type );
          isConstant &= sp->Spline->IsConstant( constVal2 );
          if ( c->GetSplineByIndex( y )->MinimalSpline.Type != Spline_Rotation )
            isConstant &= constVal[ 0 ] == constVal2[ 0 ];
          else
            isConstant &= constVal[ 0 ] == constVal2[ 0 ] && constVal[ 1 ] == constVal2[ 1 ] && constVal[ 2 ] == constVal2[ 2 ] && constVal[ 3 ] == constVal2[ 3 ];
        }
      }
      if ( isConstant )
        fullyConstantSplines++;
    }
    break;
  }

  for ( TS32 x = 0; x < s->GetClipCount(); x++ )
  {
    SymbolNameScoper sym( s->GetClipByIndex( x )->GetName() );

    if ( !s->GetClipByIndex( x )->IsRequired() )
      continue;

    CphxObjectClip_Tool* c = o->GetClip( x );
    if ( !c )
    {
      LOG_ERR( "[minimalexport] Fatal error: object %s is missing clips!", o->GetName().GetPointer() );
      MinimalExportSuccessful = false;
      continue;
    }

    TS32 SplineCount = 0;
    bool isConstant = true;

    for ( TS32 y = 0; y < c->GetSplineCount(); y++ )
    {
      TBOOL alwaysDefaultSpline = c->GetSplineByIndex( y )->IsDefaultSpline();

      if ( alwaysDefaultSpline )
        for ( TS32 z = 0; z < s->GetClipCount(); z++ )
        {
          if ( !s->GetClipByIndex( z )->IsRequired() )
            continue;
          CphxObjectClip_Tool* c2 = o->GetClip( z );
          CphxClipSpline_Tool* sp = c2->GetClipSpline( c->GetSplineByIndex( y )->MinimalSpline.Type );
          if ( sp )
          {
            alwaysDefaultSpline = alwaysDefaultSpline && sp->IsDefaultSpline();
          }
          else
            LOG_ERR( "[minimalexport] Fatal error: object %s clip spline discrepancy!", o->GetName().GetPointer() );
        }

      if ( !alwaysDefaultSpline ) SplineCount++;
    }

    WriteByte( Stream_Main, SplineCount, SceneObjectDataSize ); //clip spline count
    for ( TS32 y = 0; y < c->GetSplineCount(); y++ )
    {
      TBOOL alwaysDefaultSpline = c->GetSplineByIndex( y )->IsDefaultSpline();

      if ( alwaysDefaultSpline )
        for ( TS32 z = 0; z < s->GetClipCount(); z++ )
        {
          if ( !s->GetClipByIndex( z )->IsRequired() )
            continue;

          CphxObjectClip_Tool* c2 = o->GetClip( z );
          CphxClipSpline_Tool* sp = c2->GetClipSpline( c->GetSplineByIndex( y )->MinimalSpline.Type );
          if ( sp )
          {
            alwaysDefaultSpline = alwaysDefaultSpline && sp->IsDefaultSpline();
          }
          else
            LOG_ERR( "[minimalexport] Fatal error: object %s clip spline discrepancy!", o->GetName().GetPointer() );
        }

      if ( !alwaysDefaultSpline )
        ExportClipSpline( o, x, c->GetSplineByIndex( y ) ); //export splines
    }

    if ( o->GetObjectType() == Object_Model )
      ExportMaterialSplineBatch( (CphxObject_Model_Tool*)o, c->GetMaterialSplineBatch(), x, headerFile ); //export material splines

    if ( o->GetObjectType() == Object_ParticleEmitterCPU )
    {
      CphxObject_ParticleEmitter_CPU_Tool* e = (CphxObject_ParticleEmitter_CPU_Tool*)o;
      ExportMaterialSplineBatch( e, c->GetMaterialSplineBatch(), x, headerFile ); //export material splines
    }
  }
}

void ExportEvent( CphxEvent_Tool* e, CStreamWriter& headerFile )
{
  UsedEventTypes[ e->GetEventType() ]++;

  TBOOL SplineStored = true;
  bool meSucc = MinimalExportSuccessful != 0;
  e->exportProblem = false;

  if ( e->Time->Keys.NumItems() == 2 &&
       e->Time->Keys[ 0 ]->Key.t == 0 && e->Time->Keys[ 0 ]->Key.Value[ 0 ] == D3DXFLOAT16( 0 ) &&
       e->Time->Keys[ 1 ]->Key.t == 255 && e->Time->Keys[ 1 ]->Key.Value[ 0 ] == D3DXFLOAT16( 1 ) &&
       e->Time->Spline->Interpolation == INTERPOLATION_LINEAR && e->Time->Spline->Loop == false )
    SplineStored = false;

  EventCount++;
  WriteByte( Stream_EventType, e->Type | ( SplineStored << 7 ), EventDataSize ); //event type
  WriteWord( Stream_EventPosition, e->Event->StartFrame, EventDataSize ); //start frame
  WriteWord( Stream_EventPosition, e->Event->EndFrame, EventDataSize ); //end frame

  //CphxRenderTarget_Tool *t = Project.GetRenderTarget(e->TargetID);
  //int id = -1;
  //if (t) id = t->GetResourceIndex();
  //WriteByte(Stream_EventTargets, id, EventDataSize);

  //WriteByte(Stream_SplineStorageData, SplineStored, EventDataSize);

  if ( SplineStored )
    ExportSpline( e->Time ); //export time envelope

  switch ( e->Type )
  {
  case EVENT_ENDDEMO:
    break;
  case EVENT_RENDERDEMO:
    break;
  case EVENT_SHADERTOY:
  {
    CphxEvent_Shadertoy_Tool* d = (CphxEvent_Shadertoy_Tool*)e;
    WriteByte( Stream_Main, d->GetTech()->GetResourceIndex(), EventDataSize );

    d->UpdateMaterialState();

    CphxMaterialTechnique_Tool* tech = d->GetTech();
    for ( TS32 y = 0; y < tech->TechParameters.Parameters.NumItems(); y++ ) //tech level params
    {
      if ( tech->TechParameters.Parameters[ y ]->Parameter.Scope == PARAM_VARIABLE )
      {
        //hasTechParams = true;
        ExportMaterialParameterValue( tech->TechParameters.Parameters[ y ] );
      }
    }
    for ( TS32 y = 0; y < tech->Passes.NumItems(); y++ ) //for each pass
    {
      for ( TS32 z = 0; z < tech->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ ) //pass level params
      {
        if ( tech->Passes[ y ]->PassParameters.Parameters[ z ]->Parameter.Scope == PARAM_VARIABLE )
          ExportMaterialParameterValue( tech->Passes[ y ]->PassParameters.Parameters[ z ] );
      }
    }

    ExportTechSplineBatch( tech, &d->ShaderSplines, NULL, 0, ToMacroCompatible( d->GetName() ), headerFile );

  }
  break;
  case EVENT_RENDERSCENE:
  {
    CphxEvent_RenderScene_Tool* d = (CphxEvent_RenderScene_Tool*)e;
    if ( !d->Scene )
    {
      LOG_ERR( "[minimalexport] Fatal error: 3d event missing scene!" );
      MinimalExportSuccessful = false;
      return;
    }
    if ( !d->Camera )
    {
      LOG_NFO( "[minimalexport] 3d event missing camera - assuming it's been set by a previous event" );
      //MinimalExportSuccessful = false;
      //return;
    }
    if ( !d->Clip )
    {
      LOG_ERR( "[minimalexport] Fatal error: 3d event missing clip!" );
      MinimalExportSuccessful = false;
      return;
    }

    WriteByte( Stream_Main, d->Scene->GetResourceIndex(), EventDataSize );
    if ( d->Camera )
      WriteByte( Stream_Main, d->Camera->GetResourceIndex(), EventDataSize );
    else
      WriteByte( Stream_Main, -1, EventDataSize );
    WriteByte( Stream_Main, d->Clip->GetResourceIndex(), EventDataSize );
    CphxEvent_RenderScene* s = (CphxEvent_RenderScene*)d->Event;
    WriteByte( Stream_Main, ( s->ClearColor << 1 ) | ( (int)s->ClearZ ), EventDataSize );
  }
  break;
  case EVENT_PARTICLECALC:
  {
    CphxEvent_ParticleCalc_Tool* d = (CphxEvent_ParticleCalc_Tool*)e;
    if ( !d->Scene )
    {
      LOG_ERR( "[minimalexport] Fatal error: Particle calc missing scene!" );
      MinimalExportSuccessful = false;
      return;
    }
    if ( !d->Camera )
    {
      LOG_NFO( "[minimalexport] Particle calc missing camera - assuming it's been set by a previous event" );
      //MinimalExportSuccessful = false;
      //return;
    }
    if ( !d->Clip )
    {
      LOG_ERR( "[minimalexport] Fatal error: Particle calc missing clip!" );
      MinimalExportSuccessful = false;
      return;
    }

    WriteByte( Stream_Main, d->Scene->GetResourceIndex(), EventDataSize );
    //WriteByte( Stream_Main, d->Camera->GetResourceIndex(), EventDataSize );
    WriteByte( Stream_Main, d->Clip->GetResourceIndex(), EventDataSize );
  }
  break;
  case EVENT_CAMERAOVERRIDE:
  {
    CphxEvent_CameraOverride_Tool* d = (CphxEvent_CameraOverride_Tool*)e;
    if ( !d->Scene )
    {
      LOG_ERR( "[minimalexport] Fatal error: Particle calc missing scene!" );
      MinimalExportSuccessful = false;
      return;
    }
    if ( !d->Camera )
    {
      LOG_ERR( "[minimalexport] Fatal error: Particle calc missing camera!" );
      MinimalExportSuccessful = false;
      return;
    }
    if ( !d->Clip )
    {
      LOG_ERR( "[minimalexport] Fatal error: Particle calc missing clip!" );
      MinimalExportSuccessful = false;
      return;
    }

    WriteByte( Stream_Main, d->Scene->GetResourceIndex(), EventDataSize );
    WriteByte( Stream_Main, d->Camera->GetResourceIndex(), EventDataSize );
    WriteByte( Stream_Main, d->Clip->GetResourceIndex(), EventDataSize );
  }
  break;
  case EVENT_CAMERASHAKE:
  {
    CphxEvent_CameraShake_Tool* d = (CphxEvent_CameraShake_Tool*)e;
    CphxEvent_CameraShake* o = (CphxEvent_CameraShake*)e->Event;
    WriteByte( Stream_Main, o->ShakesPerSec, EventDataSize );
    ExportSpline( d->EyeIntensity );
    ExportSpline( d->TargetIntensity );
  }
  break;

  default:
    break;
  }

  e->exportProblem = meSucc != ( MinimalExportSuccessful != 0 );
}

TBOOL MarkTextureAsRequired( CphxGUID& GUID )
{
  CphxTextureOperator_Tool* r = Project.GetTexgenOp( GUID );
  CphxRenderTarget_Tool* rt = Project.GetRenderTarget( GUID );
  if ( !r && !rt )
  {
    CphxGUID g;
    g.SetString( "NONENONENONENONENONENONENONENONE" );
    if ( GUID != g )
    {
      LOG_ERR( "[minimalexport] Fatal error: Unknown texture operator/rendertarget referenced! (%s)", GUID.GetString() );
      MinimalExportSuccessful = false;
      return false;
    }
  }

  if ( r )
  {
    r = r->GetContentOp();
    if ( !r )
    {
      LOG_ERR( "[minimalexport] Fatal error: Texture load has no input set!" );
      MinimalExportSuccessful = false;
      return false;
    }
    else
    {
      //texture render requirement
      r->SetRenderRequirement();
      r->MarkAsRequired();
    }
  }

  if ( rt )
    rt->MarkAsRequired();

  return true;
}

void MarkPassParamRendertargetsAsRequired( CphxMaterialParameterBatch_Tool* t )
{
  for ( TS32 x = 0; x < t->Parameters.NumItems(); x++ )
    if ( t->Parameters[ x ]->Parameter.Scope == PARAM_CONSTANT )
    {
      switch ( t->Parameters[ x ]->Parameter.Type )
      {
      case PARAM_TEXTURE0:
      case PARAM_TEXTURE1:
      case PARAM_TEXTURE2:
      case PARAM_TEXTURE3:
      case PARAM_TEXTURE4:
      case PARAM_TEXTURE5:
      case PARAM_TEXTURE6:
      case PARAM_TEXTURE7:
      case PARAM_RENDERTARGET:
      {
        CphxResource* r = Project.GetResource( t->Parameters[ x ]->TextureGUID );
        if ( !r )
        {
          LOG_ERR( "[minimalexport] Material Parameter %s referenced an unknown texture", t->Parameters[ x ]->Name.GetPointer() );
          MinimalExportSuccessful = false;
          continue;
        }
        r = r->GetContentResource();
        r->SetRenderRequirement();
        r->MarkAsRequired();
      }
      }
    }
}

void MarkTechRendertargetsAsRequired( CphxMaterialTechnique_Tool* t )
{
  MarkPassParamRendertargetsAsRequired( &t->TechParameters );
  for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
    MarkPassParamRendertargetsAsRequired( &t->Passes[ y ]->PassParameters );
}

void ResetMinimalCounters()
{
  SplineCount = 0;
  SplineDataSize = 0;
  SplineDataSizeScenes = 0;
  TexgenOpCount = 0;
  TexgenOpDataSize = 0;
  MaterialParamCount = 0;
  MaterialParamSize = 0;
  MaterialParameterDataCount = 0;
  MaterialParameterDataSize = 0;
  ModelFilterCount = 0;
  ModelFilterSize = 0;
  MaterialDataPackCount = 0;
  MaterialParamDefaultCount = 0;
  ModelObjectCount = 0;
  ModelObjectDataSize = 0;
  ClipSplineCount = 0;
  SceneObjectCount = 0;
  SceneObjectDataSize = 0;
  EventCount = 0;
  EventDataSize = 0;
  MiscDataSize = 0;
  SetupDataSize = 0;
  RenderTargetDataSize = 0;
  RenderLayerDataSize = 0;
  TextureFilterDataSize = 0;
  ShaderSize = 0;
  TexgenPageDataSize = 0;
  MaterialTechCount = 0;
  MaterialTechDataSize = 0;
  MaterialDescriptionSize = 0;
  ModelSetupSize = 0;
  SceneSetupSize = 0;
  TimelineSetupSize = 0;
  MinimalDataSize = 0;
  TreeDataSize = 0;
  memset( splineTypeCounts, 0, sizeof( splineTypeCounts ) );
}

TS32 OperatorArraySorter( CphxTextureOperator_Tool** a, CphxTextureOperator_Tool** b )
{
  if ( ( *a )->IsRendered() != ( *b )->IsRendered() ) return ( (TS32)( *b )->IsRendered() ) - (TS32)( ( *a )->IsRendered() );

  TS32 xresdif = ( *a )->ParentPage->GetBaseXRes() - ( *b )->ParentPage->GetBaseXRes();
  TS32 yresdif = ( *a )->ParentPage->GetBaseYRes() - ( *b )->ParentPage->GetBaseYRes();

  if ( xresdif != 0 ) return xresdif;
  if ( yresdif != 0 ) return yresdif;

  return Project.GetTextureFilterIndex( ( *a )->FilterGuid ) - Project.GetTextureFilterIndex( ( *b )->FilterGuid );
}

bool hasNonRelativeRts = false;

void MarkRequiredTextures()
{
  //material textures - rendertargets
  for ( TS32 x = 0; x < Project.GetModelCount(); x++ )
  {
    if ( Project.GetModelByIndex( x )->IsRequired() )
    {
      CphxModel_Tool* m = Project.GetModelByIndex( x );
      for ( TS32 y = 0; y < m->GetObjectCount(); y++ )
      {
        CphxModelObject_Tool* o = m->GetObjectByIndex( y );
        if ( o->GetPrimitive() != Mesh_Clone && o->GetPrimitive() != Mesh_Arc &&
             o->GetPrimitive() != Mesh_Spline && o->GetPrimitive() != Mesh_Line )
        {
          CphxModelObject_Tool_Mesh* i = (CphxModelObject_Tool_Mesh*)o;
          for ( TS32 z = 0; z < i->MaterialData.MaterialTextures.NumItems(); z++ )
            if ( !MarkTextureAsRequired( i->MaterialData.MaterialTextures.GetByIndex( z ) ) )
              LOG_ERR( "[minimalexport] Error occured in model %s (%d) object %s (%d) texture %d", m->GetName().GetPointer(), x, o->GetName().GetPointer(), y, z );
        }

        if ( o->GetPrimitive() == Mesh_Tree || o->GetPrimitive() == Mesh_TreeLeaves )
        {
          CphxTreeSpecies* sp = Project.GetTreeSpecies( o->ParentGUIDS[ 0 ] );
          if ( sp )
            sp->MarkAsRequired();
          else
            LOG_ERR( "[minimalexport] Error occured in model %s (%d) object %s (%d): no tree species selected", m->GetName().GetPointer(), x, o->GetName().GetPointer(), y );
        }

        //tint texture render requirement
        for ( TS32 z = 0; z < o->GetFilterCount(); z++ )
          if ( o->GetFilter( z )->Filter == ModelFilter_TintMesh && o->GetFilter( z )->Enabled )
          {
            if ( o->GetFilter( z )->Texture && o->GetFilter( z )->Texture->GetContentOp() )
              o->GetFilter( z )->Texture->GetContentOp()->SetRenderRequirement();
          }
      }
    }
  }

  //material textures - particle emitters
  for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
  {
    if ( Project.GetSceneByIndex( x )->IsRequired() )
    {
      CphxScene_Tool* s = Project.GetSceneByIndex( x );
      for ( TS32 y = 0; y < s->GetObjectCount(); y++ )
      {
        CphxObject_Tool* o = s->GetObjectByIndex( y );
        if ( o->GetObjectType() == Object_ParticleEmitterCPU )
        {
          auto* i = (CphxObject_ParticleEmitter_CPU_Tool*)o;
          for ( TS32 z = 0; z < i->MaterialData.MaterialTextures.NumItems(); z++ )
            if ( !MarkTextureAsRequired( i->MaterialData.MaterialTextures.GetByIndex( z ) ) )
              LOG_ERR( "[minimalexport] Error occured in scene %s (%d) object %s (%d) texture %d", s->GetName().GetPointer(), x, o->GetName().GetPointer(), y, z );
        }
      }
    }
  }

  //event textures - rendertargets
  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
  {
    if ( Project.Timeline->Events[ x ]->Type == EVENT_SHADERTOY )
    {
      CphxEvent_Shadertoy_Tool* e = (CphxEvent_Shadertoy_Tool*)Project.Timeline->Events[ x ];
      e->GetTech()->MarkAsRequired();
      MarkTechRendertargetsAsRequired( e->GetTech() );

      for ( TS32 z = 0; z < e->MaterialData.MaterialTextures.NumItems(); z++ )
        if ( !MarkTextureAsRequired( e->MaterialData.MaterialTextures.GetByIndex( z ) ) )
          LOG_ERR( "[minimalexport] Error occured in event %s (%d) texture %d", e->GetName().GetPointer(), x, z );
    }
  }
}

void MarkRequiredClips()
{
/*
  for ( int x = 0; x < Project.GetSceneCount(); x++ )
  {
    auto* scene = Project.GetSceneByIndex( x );
    for ( int y = 0; y < scene->GetClipCount(); y++ )
    {
      if ( scene->IsClipUsedInSubScene( y ) )
        scene->GetClipByIndex( y )->MarkAsRequired();
    }
  }
*/

  for ( int x = 0; x < Project.GetSceneCount(); x++ )
  {
    int resourceIdx = 0;
    int delta = 0;
    auto* scene = Project.GetSceneByIndex( x );
    for ( int y = 0; y < scene->GetClipCount(); y++ )
    {
      if ( scene->GetClipByIndex( y )->IsRequired() )
        scene->GetClipByIndex( y )->SetResourceIndex( resourceIdx++ );
    }
  }
}

void ExportMinimal( CString TargetFile, bool zip, bool exportHeader )
{
  ResetSymbolBuilder();
  PushSymbolName( "64k" );

  hasLTC1 = false;
  hasLTC2 = false;
  hasUVClip = false;
  hasMeshParticle = false;
  hasNormalParticle = false;
  hasSubSceneParticle = false;
  scatterVertexSupport = false;
  scatterEdgeSupport = false;
  scatterPolySupport = false;
  scatterPolyMultiSupport = false;
  splineHasZeroKeyExport = false;
  hasTechParams = false;
  hasParticleSort = false;
  hasScatterOrientationOriginal = false;
  hasScatterOrientationNormal = false;
  hasScatterOrientationNormalRotate = false;
  hasScatterOrientationFullRotate = false;
  fullyConstantSplines = 0;
  memset( clipSplineTypes, 0, sizeof( clipSplineTypes ) );

  miniMeshes.FlushFast();
  ResetMinimalCounters();
  MinimalExportSuccessful = true;
  memset( UsedModelFilters, 0, sizeof( UsedModelFilters ) );
  memset( UsedModelPrimitives, 0, sizeof( UsedModelPrimitives ) );
  memset( UsedTexgenInputs, 0, sizeof( UsedTexgenInputs ) );
  memset( UsedScenePrimitives, 0, sizeof( UsedScenePrimitives ) );
  memset( UsedEventTypes, 0, sizeof( UsedEventTypes ) );
  memset( floatSplineWaveforms, 0, sizeof( floatSplineWaveforms ) );
  memset( floatSplineInterpolations, 0, sizeof( floatSplineInterpolations ) );
  memset( quatSplineInterpolations, 0, sizeof( quatSplineInterpolations ) );

  hasNonRelativeRts = false;
  for ( int x = 0; x < Project.GetRenderTargetCount(); x++ )
    if ( ( Project.GetRenderTargetByIndex( x )->ResolutionDescriptor & 0x80 ) == 0 )
      hasNonRelativeRts = true;

  for ( int x = 0; x < Project.GetTextureFilterCount(); x++ )
    Project.GetTextureFilterByIndex( x )->useCount = 0;

  TexgenImageLoad = false;
  defaultSplineCount = 0;
  constantSplineCount = 0;
  complexConstantSplineCount = 0;
  exportedSplines.FlushFast();
  duplicateSplines = 0;
  duplicateSplineSize = 0;
  defaultMaterialSplineCount = 0;
  uniqueNonSpecialSplines = 0;

  ClearRequiredFlagForAllResources();
  if ( !Project.Timeline )
  {
    MinimalExportSuccessful = false;
    return;
  }
  Project.Timeline->Sort();

  //find hidden material

  NoMaterial = NULL;

  for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
  {
    CphxMaterial_Tool* m = Project.GetMaterialByIndex( x );
    if ( !m->Techniques.NumItems() )
    {
      NoMaterial = m;
      break;
    }
  }

  if ( !NoMaterial )
  {
    LOG_ERR( "[minimalexport] Warning, no HIDE MATERIAL found in project, this may cause some issues if an object has no material set." );
    MinimalExportSuccessful = false;
  }
  else
    NoMaterial->MarkAsRequired();

  // fix particle material not always required but may be set
  for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
    for ( TS32 y = 0; y < Project.GetSceneByIndex( x )->GetObjectCount(); y++ )
    {
      auto* obj = Project.GetSceneByIndex( x )->GetObjectByIndex( y );
      if ( obj->GetObjectType() != Object_ParticleEmitterCPU )
        continue;
      CphxObject_ParticleEmitter_CPU_Tool* e = (CphxObject_ParticleEmitter_CPU_Tool*)obj;
      if ( e->EmitedObject || e->EmitedScene )
        e->RemoveParent( e->Material );
    }

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    Project.Timeline->Events[ x ]->MarkAsRequired();

  MarkRequiredTextures();
  MarkRequiredClips();

  for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
  {
    CphxMaterial_Tool* m = Project.GetMaterialByIndex( x );
    if ( m->IsRequired() )
      for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
        MarkTechRendertargetsAsRequired( m->Techniques[ y ] );
  }


  //CStreamWriterMemory mem;
  for ( TS32 x = 0; x < StreamCount; x++ )
  {
    MinimalStreams[ x ].Flush();
    SymbolStreams[ x ].Flush();
  }

  CStreamWriterFile headerFile;

  if ( exportHeader )
  {
    CString headerFileName = TargetFile + ".h";
    headerFile.Open( headerFileName.GetPointer() );
  }

  PushSymbolName( "ProdInfo" );

  //setup screen data
  WriteASCIIZ( Stream_Main, Project.Group, MiscDataSize );
  WriteASCIIZ( Stream_Main, Project.Title, MiscDataSize );
  for ( TS32 x = 0; x < 5; x++ )
  {
    if ( Project.EnableSetupHasSocial )
      WriteASCIIZ( Stream_Main, Project.Urls[ x ], MiscDataSize );
    else
      WriteByte( Stream_Main, 0, MiscDataSize );
  }

  PopSymbolName();

  //stream start data
  //////////////////////////////////////////////////////////////////////////

  PushSymbolName( "StreamOffsets" );

  TS32 StreamStartArrayPosition = MinimalStreams[ Stream_Main ].GetLength();
  TS32 dta = 0;
  for ( TS32 x = 0; x < StreamCount; x++ )
    Write( Stream_Main, &dta, 4, MiscDataSize );

  PopSymbolName();

  //project data
  //////////////////////////////////////////////////////////////////////////
  // aspect ratio and framerate info

  PushSymbolName( "ScreenInfo" );

  WriteByte( Stream_Main, Project.Timeline->Timeline->AspectX, SetupDataSize ); //aspect x
  WriteByte( Stream_Main, Project.Timeline->Timeline->AspectY, SetupDataSize ); //aspect y
  WriteByte( Stream_Main, Project.Timeline->Timeline->FrameRate, SetupDataSize ); //frame rate

  PopSymbolName();

  TS32 Counter = 0;

  PushSymbolName( "Shaders" );

  CArray< CString > globalShaders;
  CArray<CArray<CString>> globalShaderSymbolNames;

  if ( !Project.EnableGlobalShaderMinifier )
    WriteByte( Stream_Shaders, 0, ShaderSize );

  PopSymbolName();

  //////////////////////////////////////////////////////////////////////////
  // rendertargets

  PushSymbolName( "RenderTargets" );

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
  {
    CphxResource* r = Project.GetRenderTargetByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  WriteByte( Stream_Main, Counter, RenderTargetDataSize ); //rendertarget count

  for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
  {
    CphxRenderTarget_Tool* r = Project.GetRenderTargetByIndex( x );
    if ( r->IsRequired() )
    {
      WriteByte( Stream_Main, r->ResolutionDescriptor, RenderTargetDataSize );
      WriteByte( Stream_Main, r->pixelFormat, RenderTargetDataSize );
    }
  }

  PopSymbolName();

  //////////////////////////////////////////////////////////////////////////
  // renderlayers

  PushSymbolName( "RenderLayers" );

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
  {
    CphxResource* r = Project.GetRenderLayerByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  WriteByte( Stream_Main, Counter, RenderLayerDataSize ); //renderlayer count

  for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
  {
    CphxRenderLayerDescriptor_Tool* r = Project.GetRenderLayerByIndex( x );
    if ( r->IsRequired() )
    {
      //WriteByte( Stream_Main, r->RenderLayer.clearRenderTargets, RenderLayerDataSize );

      unsigned char nfo = r->RenderTargets.NumItems();
      nfo = nfo | ( r->RenderLayer.OmitDepthBuffer << 6 );
      nfo = nfo | ( r->RenderLayer.clearRenderTargets << 7 );

      WriteByte( Stream_Main, nfo, RenderLayerDataSize );
      for ( TS32 y = 0; y < r->RenderTargets.NumItems(); y++ )
        WriteByte( Stream_Main, r->RenderTargets[ y ]->GetResourceIndex(), RenderLayerDataSize );
    }
  }

  PopSymbolName();


  //////////////////////////////////////////////////////////////////////////
  // texturefilters

  PushSymbolName( "TextureFilters" );

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxResource* r = Project.GetTextureFilterByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  WriteByte( Stream_Main, Counter, TextureFilterDataSize ); //texturefilter count
  LOG_NFO( "[minimalexport] Texture filter count: %d", Counter );

  TS32 textureFilterShaderSize = 0;

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool* r = Project.GetTextureFilterByIndex( x );
    if ( r->IsRequired() )
    {
      PushSymbolName( r->Name );

      r->Filter.DataDescriptor.ParameterCount = 0;
      for ( TS32 y = 0; y < r->Parameters.NumItems(); y++ )
      {
        r->Filter.DataDescriptor.ParameterCount += 1;// r->Parameters[y]->Type
        if ( r->Parameters[ y ]->Type == 2 ) r->Filter.DataDescriptor.ParameterCount += 3;
      }

      //r->Filter.DataDescriptor.ParameterCount = r->Parameters.NumItems();
      Write( Stream_Main, &r->Filter.DataDescriptor, 2, TextureFilterDataSize );
      CString Mini = r->Code;
      globalShaders += r->Code;
      globalShaderSymbolNames += symbolNameStack;

      if ( !Project.EnableGlobalShaderMinifier )
      {
        if ( Project.EnableShaderMinifier && r->Minifiable && !MinifyShader_CNS( r->Code.GetPointer(), Mini, false, true, true ) )
        {
          LOG_ERR( "[minimalexport] Filter shader minification failed! (%s)", r->Name.GetPointer() );
          Mini = r->Code;
        }
        WriteASCIIZ( Stream_Shaders, Mini, ShaderSize );
        if ( r->Minifiable )
          LOG_NFO( "[minimalexport] Texture Shader Size: %d (%s)", Mini.Length(), r->Name.GetPointer() );
        else
          LOG_WARN( "[minimalexport] Texture Shader Size: %d (%s)", Mini.Length(), r->Name.GetPointer() );
        textureFilterShaderSize += Mini.Length();
      }

      PopSymbolName();
    }
  }

  PopSymbolName();

  LOG_NFO( "[minimalexport] Texture Filter Combined Shader Size: %d", textureFilterShaderSize );

  //////////////////////////////////////////////////////////////////////////
  // texture subroutines

  // ...

  //////////////////////////////////////////////////////////////////////////
  // texture operators

  //sort texture operators by filter
  //for (TS32 x = 0; x < Project.GetTexgenPageCount(); x++)
  //	Project.GetTexgenPageByIndex(x)->SortOpsByFilter();

  //clear required flags for load/store/nop operators as they are skipped

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    for ( TS32 y = 0; y < Project.GetTexgenPageByIndex( x )->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool* o = Project.GetTexgenPageByIndex( x )->GetOp( y );
      o->BuildsTexture = "";
    }
  }

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    for ( TS32 y = 0; y < Project.GetTexgenPageByIndex( x )->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool* o = Project.GetTexgenPageByIndex( x )->GetOp( y );
      if ( o->GetOpType() != TEXGEN_OP_NORMAL && o->GetOpType() != TEXGEN_OP_SUBROUTINECALL )
      {
        o->SetRequired( false );
      }
    }
  }


  CArray<CphxTextureOperator_Tool*> OperatorArray;

  TS32 PageCount = 0;
  Counter = 0;
  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    Project.GetTexgenPageByIndex( x )->NeedsExport = false;

    //TS32 c = Counter;
    for ( TS32 y = 0; y < Project.GetTexgenPageByIndex( x )->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool* r = Project.GetTexgenPage( x )->GetOp( y );
      if ( r->IsRequired() )
      {
        //NEED TO ADD EXCEPTION TO SUBROUTINES HERE!
        OperatorArray += r;
        r->ParentPage = Project.GetTexgenPageByIndex( x );
        //r->SetResourceIndex(Counter++);
      }
    }
    //if (Counter > c)
    //{
    //	Project.GetTexgenPageByIndex(x)->NeedsExport = true;
    //	PageCount++;
    //}
  }

  TS32 RenderedCount = 0;

  OperatorArray.Sort( OperatorArraySorter );

  for ( TS32 x = 0; x < OperatorArray.NumItems(); x++ )
    OperatorArray[ x ]->SetResourceIndex( x );

  //eliminate matching subtrees
  TS32 EliminatedCount = 0;
  for ( TS32 x = 0; x < OperatorArray.NumItems(); x++ )
  {
    TS32 TargetIndex = -1;

    for ( TS32 y = 0; y < x; y++ )
    {
      if ( OperatorArray[ x ]->SubTreeMatch( OperatorArray[ y ] ) )
      {
        TargetIndex = OperatorArray[ y ]->GetResourceIndex();
        break;
      }
    }

    if ( TargetIndex != -1 )
    {
      LOG_DBG( "[minimalexport] Eliminating Operator %s from page %s", OperatorArray[ x ]->GetName().GetPointer(), OperatorArray[ x ]->ParentPage->GetName().GetPointer() );
      OperatorArray[ x ]->SetResourceIndex( TargetIndex );
      OperatorArray.DeleteByIndex( x );
      for ( TS32 y = x; y < OperatorArray.NumItems(); y++ )
        OperatorArray[ y ]->SetResourceIndex( y );
      x--;
      EliminatedCount++;
    }
  }

  LOG_DBG( "[minimalexport] %d texture operators have been eliminated", EliminatedCount );

  TS32 xr = -9999999;
  TS32 yr = -9999999;
  for ( TS32 x = 0; x < OperatorArray.NumItems(); x++ )
  {
    RenderedCount += OperatorArray[ x ]->IsRendered();
    if ( OperatorArray[ x ]->ParentPage->GetBaseXRes() != xr || OperatorArray[ x ]->ParentPage->GetBaseYRes() != yr )
    {
      PageCount++;
      xr = OperatorArray[ x ]->ParentPage->GetBaseXRes();
      yr = OperatorArray[ x ]->ParentPage->GetBaseYRes();
    }
  }

  // build font list
  fontList.Flush();

  for ( TS32 x = 0; x < OperatorArray.NumItems(); x++ )
  {
    if ( !OperatorArray[ x ]->Filter )
      continue;

    if ( OperatorArray[ x ]->Filter->Filter.DataDescriptor.LookupType != 2 ) // font lookup
      continue;

    if ( fontList.Find( EngineFontList[ OperatorArray[ x ]->TextData.Font ] ) >= 0 )
      continue;
    fontList += CString( EngineFontList[ OperatorArray[ x ]->TextData.Font ] );
  }

  for ( TS32 x = 0; x < Project.GetModelCount(); x++ )
  {
    for ( TS32 y = 0; y < Project.GetModelByIndex( x )->GetObjectCount(); y++ )
    {
      auto* model = Project.GetModelByIndex( x )->GetObjectByIndex( y );
      if ( model->GetPrimitive() != Mesh_Text )
        continue;
      CphxModelObject_Tool_Mesh* mesh = (CphxModelObject_Tool_Mesh*)model;      
      if ( fontList.Find( EngineFontList[ mesh->GetParameters()[ 0 ] ] ) >= 0 )
        continue;
      fontList += CString( EngineFontList[ mesh->GetParameters()[ 0 ] ] );
    }
  }

  // write font list
  PushSymbolName( "FontNames" );
  WriteByte( Stream_Main, fontList.NumItems(), TexgenPageDataSize );
  for ( int x = 0; x < fontList.NumItems(); x++ )
    WriteASCIIZ( Stream_ASCIIZ, fontList[ x ], TexgenPageDataSize );
  PopSymbolName();

  Counter = OperatorArray.NumItems();
  LOG_NFO( "[minimalexport] Texture operator count: %d", Counter );

  PushSymbolName( "Textures" );

  WriteWord( Stream_Main, Counter, TexgenPageDataSize ); //16 bits! texture operator count
  WriteWord( Stream_Main, RenderedCount, TexgenPageDataSize );
  WriteByte( Stream_Main, PageCount, TexgenPageDataSize );

  LOG_NFO( "[minimalexport] Texture Page count: %d", PageCount );

  for ( TS32 x = 0; x < RenderedCount; x++ )
  {
    bool found = false;
    for ( int y = 0; y < OperatorArray[ x ]->GetChildCount( PHX_TEXTUREOPERATOR ); y++ )
    {
      CphxTextureOperator_Tool* op = (CphxTextureOperator_Tool*)OperatorArray[ x ]->GetChild( PHX_TEXTUREOPERATOR, y );
      if ( op->GetOpType() == TEXGEN_OP_SAVE )
      {
        if ( !op->GetName().Length() )
          continue;

        found = true;
        OperatorArray[ x ]->SetBuildsTextureSymbolRecursive( op->GetName() );
        break;
      }
    }

    if ( !found )
      OperatorArray[ x ]->SetBuildsTextureSymbolRecursive( OperatorArray[ x ]->GetName() );
  }

  for ( TS32 x = 0; x < OperatorArray.NumItems();)
  {
    TS32 cnt = 0; //number of texture operators on this page

    CapexTexGenPage* p = OperatorArray[ x ]->ParentPage;

    for ( TS32 y = x; y < OperatorArray.NumItems(); y++ )
    {
      if ( OperatorArray[ y ]->ParentPage->GetBaseXRes() != p->GetBaseXRes() ||
           OperatorArray[ y ]->ParentPage->GetBaseYRes() != p->GetBaseYRes() ) break;
      cnt++; //NEED TO ADD EXCEPTION TO SUBROUTINES HERE! - ???? this comment is from the old export code
    }

    TS32 res = ( p->GetBaseXRes() << 4 ) + p->GetBaseYRes();

    //PushSymbolName( CString::Format( "%dx%d", 1 << p->GetBaseXRes(), 1 << p->GetBaseYRes() ) );
    PushSymbolName( "TexturePageData" );
    WriteByte( Stream_Main, res, TexgenPageDataSize ); //resolution data
    WriteWord( Stream_Main, cnt, TexgenPageDataSize ); //16 bits!
    PopSymbolName();

    LOG_NFO( "[minimalexport] Texture Page %d Op count: %d", x, cnt );

    for ( TS32 y = 0; y < cnt; y++ )
    {
      //NEED TO ADD EXCEPTION TO SUBROUTINES HERE! - ???? this comment is from the old export code

      if ( OperatorArray[ x ]->Filter )
        OperatorArray[ x ]->Filter->useCount++;

      ExportTexgenOp( OperatorArray[ x++ ] );
    }
  }

  PopSymbolName();

  //for (TS32 x = 0; x < Project.GetTexgenPageCount(); x++)
  //	if (Project.GetTexgenPageByIndex(x)->NeedsExport)
  //	{
  //		CapexTexGenPage* p = Project.GetTexgenPageByIndex(x);
  //		TS32 cnt = 0; //number of texture operators on this page
  //		for (TS32 y = 0; y < p->GetOpCount(); y++)
  //			if (p->GetOp(y)->IsRequired())
  //			{
  //				//NEED TO ADD EXCEPTION TO SUBROUTINES HERE!
  //				cnt++;
  //			}

  //		TS32 res = (p->GetBaseXRes() << 4) + p->GetBaseYRes();

  //		WriteByte(Stream_Main, res, TexgenPageDataSize); //resolution data
  //		WriteWord(Stream_Main, cnt, TexgenPageDataSize); //16 bits!

  //		LOG_NFO("[minimalexport] Texture Page %d Op count: %d", x, cnt);

  //		for (TS32 y = 0; y < p->GetOpCount(); y++)
  //			if (p->GetOp(y)->IsRequired())
  //			{
  //				//NEED TO ADD EXCEPTION TO SUBROUTINES HERE!

  //				ExportTexgenOp(p->GetOp(y));
  //			}
  //	}

  //undo texture filter sort
  //for (TS32 x = 0; x < Project.GetTexgenPageCount(); x++)
  //	Project.GetTexgenPageByIndex(x)->BuildOperatorConnections();

  //////////////////////////////////////////////////////////////////////////
  // tech export

  PushSymbolName( "MaterialTechniques" );

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxResource* r = Project.GetTechByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  WriteByte( Stream_Main, Counter, MaterialTechDataSize ); //tech count

  LOG_NFO( "[minimalexport] Material Tech Count: %d", Counter );

  int materialShaderSize = 0;

  for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
  {
    CphxMaterialTechnique_Tool* t = Project.GetTechByIndex( x );
    if ( t->IsRequired() )
    {
      PushSymbolName( t->Name );
      MaterialTechCount++;
      CphxRenderLayerDescriptor_Tool* l = Project.GetRenderLayer( t->TargetLayer );
      TS32 targetlayer = 0;
      if ( l ) targetlayer = l->GetResourceIndex();

      if ( !l && t->Tech->Type == TECH_MATERIAL )
      {
        LOG_ERR( "[minimalexport] Fatal error: Missing target renderlayer in project! (Tech: %s)", t->Name.GetPointer() );
        MinimalExportSuccessful = false;
        return;
      }

      CString techName = ToMacroCompatible( t->Name );
      headerFile.WriteFormat( "\n\n#define TECHNIQUE_%-30s %d\n\n", techName.GetPointer(), MaterialTechCount - 1 );

      WriteByte( Stream_Main, targetlayer, MaterialTechDataSize ); //target layer
      ExportMaterialParams( t->TechParameters, techName, headerFile );
      WriteByte( Stream_Main, t->Passes.NumItems(), MaterialTechDataSize ); //pass count
      for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      {
        CphxMaterialRenderPass_Tool* p = t->Passes[ y ];
        ExportMaterialParams( p->PassParameters, techName, headerFile ); //pass parameters

        CString Minified = p->ShaderCode;
        globalShaders += p->ShaderCode;
        globalShaderSymbolNames += symbolNameStack;

        if ( !Project.EnableGlobalShaderMinifier )
        {
          if ( Project.EnableShaderMinifier && p->Minifiable && !MinifyShader_CNS( p->ShaderCode.GetPointer(), Minified, false, true, true ) )
          {
            LOG_ERR( "[minimalexport] Shader minification failed! Tech: %s Pass: %d", t->Name.GetPointer(), y );
            Minified = p->ShaderCode;
          }

          WriteASCIIZ( Stream_Shaders, Minified, ShaderSize ); //shader code
          if ( p->Minifiable )
            LOG_NFO( "[minimalexport] Material Technique Pass %d Shader Size: %d (%s - %s)", y, Minified.Length(), t->Name.GetPointer(), p->Name.GetPointer() );
          else
            LOG_WARN( "[minimalexport] Material Technique Pass %d Shader Size: %d (%s - %s)", y, Minified.Length(), t->Name.GetPointer(), p->Name.GetPointer() );
          materialShaderSize += Minified.Length();
        }
      }
      PopSymbolName();
    }
  }

  PopSymbolName();

  LOG_NFO( "[minimalexport] Material Combined Shader Size: %d", materialShaderSize );

  //////////////////////////////////////////////////////////////////////////
  // material export

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
  {
    CphxResource* r = Project.GetMaterialByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  PushSymbolName( "Materials" );

  WriteByte( Stream_Main, Counter, MaterialDescriptionSize ); //material count
  LOG_NFO( "[minimalexport] Material count: %d", Counter );

  for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
  {
    CphxMaterial_Tool* m = Project.GetMaterialByIndex( x );
    if ( m->IsRequired() )
    {
      WriteByte( Stream_Main, m->Techniques.NumItems(), MaterialDescriptionSize ); //tech count
      for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
        WriteByte( Stream_Main, m->Techniques[ y ]->GetResourceIndex(), MaterialDescriptionSize );
    }
  }

  PopSymbolName();

  //////////////////////////////////////////////////////////////////////////
  // tree species export

  PushSymbolName( "TreeSpecies" );

  Counter = 0;
  for ( int x = 0; x < Project.GetTreeSpeciesCount(); x++ )
  {
    CphxResource* r = Project.GetTreeSpeciesByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  WriteByte( Stream_Main, Counter, TreeDataSize ); //treespecies count
  LOG_NFO( "[minimalexport] Tree species count: %d", Counter );

  for ( int x = 0; x < Project.GetTreeSpeciesCount(); x++ )
  {
    CphxTreeSpecies* t = Project.GetTreeSpeciesByIndex( x );
    if ( t->IsRequired() )
    {
      TREESPECIESDESCRIPTOR& d = t->Tree;
      Write( Stream_Main, &d, sizeof( d ), TreeDataSize );
    }
  }

  PopSymbolName();


  //////////////////////////////////////////////////////////////////////////
  // model export

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetModelCount(); x++ )
  {
    CphxModel_Tool* r = Project.GetModelByIndex( x );
    if ( r->IsRequired() )
    {
      r->SetResourceIndex( Counter++ );
      for ( TS32 y = 0; y < r->GetObjectCount(); y++ ) //check minimodels
      {
        auto* obj = r->GetObjectByIndex( y );
        if ( obj->GetPrimitive() == Mesh_StoredMini )
        {
          bool found = false;
          for ( int z = 0; z < miniMeshes.NumItems() && !found; z++ )
          {
            if ( miniMeshes[ z ].vxDataSize == obj->miniModelVertices.NumItems() &&
                 miniMeshes[ z ].polyDataSize == obj->miniModelTriangles.NumItems() )
            {
              if ( memcmp( miniMeshes[ z ].vxData, obj->miniModelVertices.GetPointer( 0 ), miniMeshes[ z ].vxDataSize ) )
                continue;
              if ( memcmp( miniMeshes[ z ].polyData, obj->miniModelTriangles.GetPointer( 0 ), miniMeshes[ z ].polyDataSize ) )
                continue;

              found = true;
              obj->miniModelExportIndex = z;

              LOG_NFO( "Duplicate minimesh located." );
            }
          }

          if ( !found )
          {
            obj->miniModelExportIndex = miniMeshes.NumItems();
            MiniMesh m;
            m.vxDataSize = obj->miniModelVertices.NumItems();
            m.vxData = obj->miniModelVertices.GetPointer( 0 );
            m.polyDataSize = obj->miniModelTriangles.NumItems();
            m.polyData = obj->miniModelTriangles.GetPointer( 0 );
            miniMeshes += m;

          }
        }
      }
    }
  }

  PushSymbolName( "Models" );

  // export minimodels
  WriteByte( Stream_ModelParameters, miniMeshes.NumItems(), ModelObjectDataSize );
  for ( TS32 x = 0; x < miniMeshes.NumItems(); x++ )
  {
    auto& m = miniMeshes[ x ];
    WriteWord( Stream_ModelParameters, m.vxDataSize, ModelObjectDataSize );
    WriteWord( Stream_ModelParameters, m.polyDataSize, ModelObjectDataSize );
    Write( Stream_ModelParameters, m.vxData, m.vxDataSize, ModelObjectDataSize );
    Write( Stream_ModelParameters, m.polyData, m.polyDataSize, ModelObjectDataSize );
  }

  WriteByte( Stream_Main, Counter, ModelSetupSize ); //model count
  LOG_NFO( "[minimalexport] Model count: %d", Counter );

  for ( TS32 x = 0; x < Project.GetModelCount(); x++ )
  {
    CphxModel_Tool* m = Project.GetModelByIndex( x );
    if ( m->IsRequired() )
    {
      PushSymbolName( m->GetName() );
      StoreStreamPositions();
      WriteByte( Stream_Main, m->GetObjectCount(), ModelSetupSize ); //object count
      if ( m->GetObjectCount() > 255 )
      {
        LOG_ERR( "[minimalexport] Fatal error: model %s contains %d objects (higher than 255)", m->GetName().GetPointer(), m->GetObjectCount() );
        MinimalExportSuccessful = false;
      }
      for ( TS32 y = 0; y < m->GetObjectCount(); y++ ) //base model export
      {
        PushSymbolName( CString::Format( "(%03d)", y ) + m->GetObjectByIndex( y )->GetName() );
        ExportModelObject( m->GetObjectByIndex( y ), m );
        ExportModelFilters( m->GetObjectByIndex( y ), m );
        PopSymbolName();
      }
      int modelSize = GetStreamSizeDiff();
      LOG_NFO( "Model %s size: %d", m->GetName().GetPointer(), modelSize );
      PopSymbolName();
    }
  }

  PopSymbolName();

  //////////////////////////////////////////////////////////////////////////
  // scene export

  // todo: DON'T EXPORT UNUSED CLIPS!

  Counter = 0;
  for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
  {
    CphxResource* r = Project.GetSceneByIndex( x );
    if ( r->IsRequired() ) r->SetResourceIndex( Counter++ );
  }

  PushSymbolName( "Scenes" );

  WriteByte( Stream_Main, Counter, SceneSetupSize ); //scene count
  LOG_NFO( "[minimalexport] Scene count: %d", Counter );

  int sceneCount = 0;
  for ( TS32 x = 0; x < Project.GetSceneCount(); x++ )
  {
    CphxScene_Tool* m = Project.GetSceneByIndex( x );
    if ( m->IsRequired() )
    {
      PushSymbolName( m->GetName() );
      StoreStreamPositions();
      TS32 reqCnt = 0;

      for ( TS32 y = 0; y < m->GetClipCount(); y++ )
      {
        if ( m->GetClipByIndex( y )->IsRequired() )
          reqCnt++;
        //m->GetClipByIndex( y )->SetResourceIndex( y ); // already set along with subscene references
      }

      LOG_NFO( "[minimalexport] Scene %s: %d of %d clips required", m->GetName().GetPointer(), reqCnt, m->GetClipCount() );

      WriteByte( Stream_Main, m->GetObjectCount(), SceneSetupSize ); //object count
      WriteByte( Stream_Main, reqCnt, SceneSetupSize ); //clip count

      int oldSplineSize = SplineDataSize;

      CString sceneName = ToMacroCompatible( m->GetName() );
      headerFile.WriteFormat( "\n\n#define SCENE_%-30s %d\n\n", sceneName.GetPointer(), sceneCount++ );

      CDictionary<CString, int> collisions;
      for ( TS32 y = 0; y < m->GetObjectCount(); y++ )
      {
        PushSymbolName( CString::Format( "(%03d)", y ) + m->GetObjectByIndex( y )->GetName() );
        m->GetObjectByIndex( y )->SetResourceIndex( y );

        CString objName = ToMacroCompatible( m->GetObjectByIndex( y )->GetName() );
        if ( collisions.HasKey( objName ) )
        {
          collisions[ objName ]++;
          objName += CString::Format( "_%03d", collisions[ objName ] );
        }
        else
        {
          collisions.Add( objName, 1 );
        }

        headerFile.WriteFormat( "#define OBJECT_%s_%-30s %d\n", sceneName.GetPointer(), objName.GetPointer(), y );

        ExportSceneObject( m->GetObjectByIndex( y ), m, sceneName, objName, headerFile );
        PopSymbolName();
      }

      LOG_NFO( "[minimalexport] Scene %s spline data size: %d", m->GetName().GetPointer(), SplineDataSize - oldSplineSize );
      SplineDataSizeScenes += SplineDataSize - oldSplineSize;

      int sceneSize = GetStreamSizeDiff();
      LOG_NFO( "Scene %s size: %d", m->GetName().GetPointer(), sceneSize );
      PopSymbolName();
    }
  }

  PopSymbolName();

  //////////////////////////////////////////////////////////////////////////
  // global shader minimizing

  PushSymbolName( "GlobalShaderMinifier" );

  if ( Project.EnableGlobalShaderMinifier )
  {
    CString commonHeader;
    CArray<CString> minifiedShaders;

    CString globalShaderBlob;
    for ( int x = 0; x < globalShaders.NumItems(); x++ )
      globalShaderBlob += globalShaders[ x ];

    LOG_WARN( "Global shader blob size: %d hash: %x", globalShaderBlob.Length(), globalShaderBlob.GetHash() );


    if ( !CrossCompiler::Parser::MinifyMultipleShaders( globalShaders, commonHeader, minifiedShaders ) )
      LOG_ERR( "CATASTROPHIC FAILURE DURING GLOBAL SHADER MINIFICATION. ABORT." );

    CString globalMinifiedShaderBlob;
    for ( int x = 0; x < minifiedShaders.NumItems(); x++ )
      globalMinifiedShaderBlob += minifiedShaders[ x ];

    LOG_WARN( "Global common header size: %d hash: %x", commonHeader.Length(), commonHeader.GetHash() );
    LOG_WARN( "Global minified shader blob size: %d hash: %x", globalMinifiedShaderBlob.Length(), globalMinifiedShaderBlob.GetHash() );

    PushSymbolName( "CommonHeader" );
    WriteASCIIZ( Stream_Shaders, commonHeader, ShaderSize );
    PopSymbolName();

    auto symbolNameStackBackup = symbolNameStack;

    for ( int x = 0; x < minifiedShaders.NumItems(); x++ )
    {
      symbolNameStack = globalShaderSymbolNames[ x ];
      WriteASCIIZ( Stream_Shaders, minifiedShaders[ x ], ShaderSize );
    }

    symbolNameStack = symbolNameStackBackup;

  }

  PopSymbolName();

  //////////////////////////////////////////////////////////////////////////
  // timeline export

  PushSymbolName( "Timeline" );
  WriteWord( Stream_Main, Project.Timeline->Events.NumItems(), TimelineSetupSize ); //event count
  LOG_NFO( "[minimalexport] Event count: %d", Project.Timeline->Events.NumItems() );

  for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
    ExportEvent( Project.Timeline->Events[ x ], headerFile );

  PopSymbolName();

  LOG_NFO( "[minimalexport] Minimalexport didn't exit early." );
  LOG_NFO( "[minimalexport] Combining %d streams...", StreamCount );

  TS32 Position = 0;

#undef Write
#undef WriteASCIIZ

  for ( TS32 x = 0; x < StreamCount; x++ )
  {
    CStreamWriterFile f;
    f.Open( CString::Format( _T( "Data\\Streams\\%.2d.bin" ), x ).GetPointer() );
    f.Write( MinimalStreams[ x ].GetData(), MinimalStreams[ x ].GetLength() );
  }

  for ( TS32 x = 0; x < StreamCount; x++ )
  {
    ( (TS32*)( MinimalStreams[ Stream_Main ].GetData() + StreamStartArrayPosition ) )[ x ] = Position;
    if ( x != Stream_Main )
    {
      Position += MinimalStreams[ x ].GetLength();
    }
    else
    {
      ( (TS32*)( MinimalStreams[ Stream_Main ].GetData() + StreamStartArrayPosition ) )[ x ] = StreamCount * 4; // add the size of the stream position array so it's not needed in the minimalplayer
      Position += MinimalStreams[ x ].GetLength() - StreamStartArrayPosition;
    }
  }

  for ( TS32 x = 0; x < StreamCount; x++ )
    if ( x != Stream_Main )
    {
      TS32 cnt = 0;
      MinimalStreams[ Stream_Main ].Write( MinimalStreams[ x ].GetData(), MinimalStreams[ x ].GetLength() );
      SymbolStreams[ Stream_Main ].Write( SymbolStreams[ x ].GetData(), SymbolStreams[ x ].GetLength() );
    }

  if ( zip )
  {
    TU8* buf = new TU8[ MinimalStreams[ Stream_Main ].GetLength() ];

    mz_ulong len = MinimalStreams[ Stream_Main ].GetLength();
    int uncmplen = MinimalStreams[ Stream_Main ].GetLength();

    int cmplen = compress( buf, &len, MinimalStreams[ Stream_Main ].GetData(), MinimalStreams[ Stream_Main ].GetLength() );

    cmplen = len;

    CStreamWriterFile f;
    f.Open( TargetFile.GetPointer() );
    f.Write( &cmplen, 4 );
    f.Write( &uncmplen, 4 );
    f.Write( buf, cmplen );

    delete[] buf;
  }
  else
  {
    CStreamWriterFile f;
    f.Open( TargetFile.GetPointer() );
    MinimalDataSize = MinimalStreams[ Stream_Main ].GetLength();
    f.Write( MinimalStreams[ Stream_Main ].GetData(), MinimalStreams[ Stream_Main ].GetLength() );

    CStreamWriterFile s;
    s.Open( ( TargetFile + ".sym" ).GetPointer() );
    s.Write( "PHXP", 4 );
    s.WriteDWord( MinimalDataSize );
    s.WriteDWord( symbolList.NumItems() );
    for ( int x = 0; x < symbolList.NumItems(); x++ )
      s.WriteASCIIZ( symbolList[ x ] );
    s.Write( SymbolStreams[ Stream_Main ].GetData(), SymbolStreams[ Stream_Main ].GetLength() );
  }

  LOG_NFO( "[minimalexport] Constant Spline Count: %d Default Spline Count: %d Complex Constant Splines: %d", constantSplineCount, defaultSplineCount, complexConstantSplineCount );

  LOG_NFO( "[minimalexport] Estimated packed size of data: %d", GetKKrunchedSize( TargetFile ) );

  LOG_NFO( "[minimalexport] Datafile statistics follow" );
  LOG_NFO( "[minimalexport] --------------------------" );

  LOG_NFO( "[minimalexport] SplineCount: %d", SplineCount, SplineCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] SplineDataSize: %d (%.2f%%), %d in scenes (%.2f%%)", SplineDataSize, SplineDataSize / (TF32)MinimalDataSize * 100, SplineDataSizeScenes, SplineDataSizeScenes / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] Duplicate spline count: %d", duplicateSplines );
  LOG_NFO( "[minimalexport] Duplicate spline size:  %d (%.2f%%)", duplicateSplineSize, duplicateSplineSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] Possible Duplicate spline size save:  %d (%.2f%%)", duplicateSplineSize - duplicateSplines * 2, ( duplicateSplineSize - duplicateSplines * 2 ) / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] Unique spline count:  %d", exportedSplines.NumItems() );
  LOG_NFO( "[minimalexport] Unique nonconstant spline count:  %d", uniqueNonSpecialSplines );
  LOG_NFO( "[minimalexport] Splines that are the same constant value across all clips in an object:  %d", fullyConstantSplines );
  
  LOG_NFO( "[minimalexport] DEFAULTSPLINE_ZERO: %d", splineTypeCounts[ DEFAULTSPLINE_ZERO ] );
  LOG_NFO( "[minimalexport] DEFAULTSPLINE_ONE: %d", splineTypeCounts[ DEFAULTSPLINE_ONE ] );
  /*
    LOG_NFO( "[minimalexport] DEFAULTSPLINE_ZEROTOONE: %d", splineTypeCounts[ DEFAULTSPLINE_ZEROTOONE ] );
    LOG_NFO( "[minimalexport] DEFAULTSPLINE_CONSTANT_ONEBYTE: %d", splineTypeCounts[ DEFAULTSPLINE_CONSTANT_ONEBYTE ] );
    LOG_NFO( "[minimalexport] DEFAULTSPLINE_CONSTANT_TWOBYTE: %d", splineTypeCounts[ DEFAULTSPLINE_CONSTANT_TWOBYTE ] );
    LOG_NFO( "[minimalexport] DEFAULTSPLINE_POOL_ONEBYTEINDEX: %d", splineTypeCounts[ DEFAULTSPLINE_POOL_ONEBYTEINDEX ] );
    LOG_NFO( "[minimalexport] DEFAULTSPLINE_POOL_STORE: %d", splineTypeCounts[ DEFAULTSPLINE_POOL_STORE ] );
    LOG_NFO( "[minimalexport] DEFAULTSPLINE_DEFAULTMATERIALPARAM: %d", splineTypeCounts[ DEFAULTSPLINE_DEFAULTMATERIALPARAM ] );
  */

  LOG_NFO( "[minimalexport] Default material spline count:  %d", defaultMaterialSplineCount );
  LOG_NFO( "[minimalexport] TexgenOpCount: %d", TexgenOpCount, TexgenOpCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] TexgenOpDataSize: %d (%.2f%%)", TexgenOpDataSize, TexgenOpDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialParamCount: %d", MaterialParamCount, MaterialParamCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialParamSize: %d (%.2f%%)", MaterialParamSize, MaterialParamSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialParameterDataCount: %d", MaterialParameterDataCount, MaterialParameterDataCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialParameterDataSize: %d (%.2f%%)", MaterialParameterDataSize, MaterialParameterDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ModelFilterCount: %d", ModelFilterCount, ModelFilterCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ModelFilterSize: %d (%.2f%%)", ModelFilterSize, ModelFilterSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialDataPackCount: %d", MaterialDataPackCount, MaterialDataPackCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialParamDefaultCount: %d", MaterialParamDefaultCount, MaterialParamDefaultCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ModelObjectCount: %d", ModelObjectCount, ModelObjectCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ModelObjectDataSize: %d (%.2f%%)", ModelObjectDataSize, ModelObjectDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ClipSplineCount: %d", ClipSplineCount, ClipSplineCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] SceneObjectCount: %d", SceneObjectCount, SceneObjectCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] SceneObjectDataSize: %d (%.2f%%)", SceneObjectDataSize, SceneObjectDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] EventCount: %d", EventCount, EventCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] EventDataSize: %d (%.2f%%)", EventDataSize, EventDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MiscDataSize: %d (%.2f%%)", MiscDataSize, MiscDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] SetupDataSize: %d (%.2f%%)", SetupDataSize, SetupDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] RenderTargetDataSize: %d (%.2f%%)", RenderTargetDataSize, RenderTargetDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] RenderLayerDataSize: %d (%.2f%%)", RenderLayerDataSize, RenderLayerDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] TextureFilterDataSize: %d (%.2f%%)", TextureFilterDataSize, TextureFilterDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ShaderSize: %d (%.2f%%)", ShaderSize, ShaderSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] TexgenPageDataSize: %d (%.2f%%)", TexgenPageDataSize, TexgenPageDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialTechCount: %d", MaterialTechCount, MaterialTechCount / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialTechDataSize: %d (%.2f%%)", MaterialTechDataSize, MaterialTechDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MaterialDescriptionSize: %d (%.2f%%)", MaterialDescriptionSize, MaterialDescriptionSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] TreeDataSize: %d (%.2f%%)", TreeDataSize, TreeDataSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] ModelSetupSize: %d (%.2f%%)", ModelSetupSize, ModelSetupSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] SceneSetupSize: %d (%.2f%%)", SceneSetupSize, SceneSetupSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] TimelineSetupSize: %d (%.2f%%)", TimelineSetupSize, TimelineSetupSize / (TF32)MinimalDataSize * 100 );
  LOG_NFO( "[minimalexport] MinimalDataSize: %d (%.2f%%)", MinimalDataSize, MinimalDataSize / (TF32)MinimalDataSize * 100 );

  LOG_NFO( "[minimalexport] --------------------------" );

  char* meshNames[] =
  {
  "Mesh_Cube"          ,
  "Mesh_Plane"         ,
  "Mesh_Sphere"        ,
  "Mesh_Cylinder"      ,
  "Mesh_Cone"          ,
  "Mesh_Arc"           ,
  "Mesh_Line"          ,
  "Mesh_Spline"        ,
  "Mesh_Loft"          ,
  "Mesh_Clone"         ,
  "Mesh_Copy"          ,
  "Mesh_GeoSphere"     ,
  "Mesh_Scatter"       ,
  "Mesh_Stored"        ,
  "Mesh_Tree"          ,
  "Mesh_TreeLeaves"    ,
  "Mesh_Text"          ,
  "Mesh_Marched"       ,
  "Mesh_StoredMini"    ,
  "Mesh_Merge" };

  char* meshFilterNames[] =
  {
    "ModelFilter_UVMap"          ,
    "ModelFilter_Bevel"          ,
    "ModelFilter_MapXForm"       ,
    "ModelFilter_MeshSmooth"     ,
    "ModelFilter_SmoothGroup"    ,
    "ModelFilter_TintMesh"       ,
    "ModelFilter_TintMeshShape"  ,
    "ModelFilter_Replicate"      ,
    "ModelFilter_NormalDeform"   ,
    "ModelFilter_CSG"            ,
    "ModelFilter_Greeble"        ,
    "ModelFilter_Invert"         ,
    "ModelFilter_SavePos2" };

  char* scenePrimitiveNames[] =
  {
  "ScenePrimitive_Model",
  "ScenePrimitive_Light",
  "ScenePrimitive_CamEye",
  "ScenePrimitive_Dummy",
  "ScenePrimitive_SubScene",
  "ScenePrimitive_ParticleEmitter",
  "ScenePrimitive_ParticleEmitterCPU",
  "ScenePrimitive_ParticleDrag",
  "ScenePrimitive_ParticleGravity",
  "ScenePrimitive_ParticleTurbulence",
  "ScenePrimitive_ParticleVortex",
  "ScenePrimitive_LogicObject",
  };

  char* eventTypeNames[] =
  {
  "Event_Enddemo",
  "Event_Renderdemo",
  "Event_Shadertoy",
  "Event_Renderscene",
  "Event_Particlecalc",
  "Event_Camerashake",
  "Event_Cameraoverride",
  };

  char* clipSplineNames[] =
  {
    "Spline_MaterialParam"                     ,
    "Spline_Scale_x"                           ,
    "Spline_Scale_y"                           ,
    "Spline_Scale_z"                           ,
    "Spline_Rotation"                          ,
    "Spline_SubScene_Clip"                     ,
    "Spline_SubScene_Time"                     ,
    "Padding"                                  ,
    "Spline_Position_x"                        ,
    "Spline_Position_y"                        ,
    "Spline_Position_z"                        ,
    "Spline_Position_w"                        ,
    "Spline_Light_AmbientR"                    ,
    "Spline_Light_AmbientG"                    ,
    "Spline_Light_AmbientB"                    ,
    "Padding"                                  ,
    "Spline_Light_DiffuseR"                    ,
    "Spline_Light_DiffuseG"                    ,
    "Spline_Light_DiffuseB"                    ,
    "Padding"                                  ,
    "Spline_Light_SpecularR"                   ,
    "Spline_Light_SpecularG"                   ,
    "Spline_Light_SpecularB"                   ,
    "Padding"                                  ,
    "Spot_Direction_X"                         ,
    "Spot_Direction_Y"                         ,
    "Spot_Direction_Z"                         ,
    "Padding"                                  ,
    "Spline_Light_Exponent"                    ,
    "Spline_Light_Cutoff"                      ,
    "Spline_Light_Attenuation_Linear"          ,
    "Spline_Light_Attenuation_Quadratic"       ,
    "Padding"                                  ,
    "Spline_Camera_FOV"                        ,
    "Spline_Camera_Roll"                       ,
    "Padding"                                  ,
    "Padding"                                  ,
    "Spline_Particle_Offset_x"                 ,
    "Spline_Particle_Offset_y"                 ,
    "Spline_Particle_Offset_z"                 ,
    "Spline_Particle_EmissionPerSecond"        ,
    "Spline_Particle_EmissionTrigger"          ,
    "Spline_Particle_EmissionVelocity"         ,
    "Spline_Particle_Life"                     ,
    "Spline_Particle_EmissionRotation"         ,
    "Spline_Particle_EmissionVelocityChaos"    ,
    "Spline_Particle_EmissionRotationChaos"    ,
    "Spline_Particle_LifeChaos"                ,
    "Spline_Light_OrthoX"                      ,
    "Spline_Light_OrthoY"                      ,
    "Spline_AffectorPower"                     ,
    "Spline_Particle_Scale"                    ,
    "Spline_Particle_ScaleChaos"               ,
    "Spline_Particle_Stretch_X"                ,
    "Spline_Particle_Stretch_Y"                ,
    "Spline_SubScene_RepeatCount"              ,
    "Spline_SubScene_RepeatTimeOffset"         ,
  };

  for ( int x = 0; x < Mesh_Merge; x++ )
    if ( UsedModelPrimitives[ x ] )
      LOG_NFO( "Mesh Primitive #%s: %d", meshNames[ x ], UsedModelPrimitives[ x ] );
  for ( int x = 0; x < ModelFilter_NONE; x++ )
    if ( UsedModelFilters[ x ] )
      LOG_NFO( "Mesh Filter #%s: %d", meshFilterNames[ x ], UsedModelFilters[ x ] );
  for ( int x = 0; x < Object_Count; x++ )
    if ( UsedScenePrimitives[ x ] )
      LOG_NFO( "Scene Primitive #%s: %d", scenePrimitiveNames[ x ], UsedScenePrimitives[ x ] );
  for ( int x = 0; x < Event_Count; x++ )
    if ( UsedEventTypes[ x ] )
      LOG_NFO( "Event #%s: %d", eventTypeNames[ x ], UsedEventTypes[ x ] );

  for ( int x = 0; x < Spline_Count; x++ )
    if ( clipSplineTypes[ x ] )
      LOG_NFO( "Spline #%s: %d", clipSplineNames[ x ], clipSplineTypes[ x ] );

  for ( int x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    if ( Project.GetTextureFilterByIndex( x )->IsRequired() && Project.GetTextureFilterByIndex( x )->useCount )
      LOG_NFO( "Texture Filter %s : %d", Project.GetTextureFilterByIndex( x )->Name.GetPointer(), Project.GetTextureFilterByIndex( x )->useCount );
  }

  LOG_NFO( "[minimalexport] --------------------------" );
}

void CreateMinimalConfig( TCHAR* TargetFile )
{
  FILE* f = NULL;
  fopen_s( &f, TargetFile, "w+t" );
  if ( !f ) return;

  fprintf_s( f, "#pragma once\n\n" );
  fprintf_s( f, "#include \"PhxManualConfig.h\"\n" );

  //fprintf_s( f, "#define PHX_MINIMAL_BUILD\n" );

  if ( Project.EnableFarbrauschPrecalc )
    fprintf_s( f, "#define PHX_FAKE_FARBRAUSCH_INTRO_BUILD\n" );

  if ( Project.EnableSetupHasSocial )
    fprintf_s( f, "#define SETUPBOX_HAS_SOCIAL\n" );

  if ( !TexgenImageLoad ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_TEXGEN_IMAGE\n" );

  if ( !UsedModelPrimitives[ Mesh_Cube ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CUBE\n" );
  if ( !UsedModelPrimitives[ Mesh_Sphere ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_SPHERE\n" );
  if ( !UsedModelPrimitives[ Mesh_Plane ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_PLANE\n" );
  if ( !UsedModelPrimitives[ Mesh_Cone ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CONE\n" );
  if ( !UsedModelPrimitives[ Mesh_Cylinder ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CYLINDER\n" );
  if ( !UsedModelPrimitives[ Mesh_Arc ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_ARC\n" );
  if ( !UsedModelPrimitives[ Mesh_Line ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_LINE\n" );
  if ( !UsedModelPrimitives[ Mesh_Spline ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_SPLINE\n" );
  if ( !UsedModelPrimitives[ Mesh_Loft ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_LOFT\n" );
  if ( !UsedModelPrimitives[ Mesh_Copy ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_COPY\n" );
  if ( !UsedModelPrimitives[ Mesh_GeoSphere ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CREATEGEOSPHERE\n" );
  if ( !UsedModelPrimitives[ Mesh_Tree ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CREATETREE\n" );
  if ( !UsedModelPrimitives[ Mesh_TreeLeaves ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CREATETREELEAVES\n" );
  if ( !UsedModelPrimitives[ Mesh_Text ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CREATETEXT\n" );
  if ( !UsedModelPrimitives[ Mesh_Scatter ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_SCATTER\n" );
  if ( !UsedModelPrimitives[ Mesh_Stored ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_LOADSTOREDMESH\n" );
  if ( !UsedModelPrimitives[ Mesh_StoredMini ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_LOADSTOREDMINIMESH\n" );
  if ( !UsedModelPrimitives[ Mesh_Merge ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_MERGE\n" );

  if ( !UsedModelFilters[ ModelFilter_Bevel ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_BEVEL\n" );
  if ( !UsedModelFilters[ ModelFilter_MapXForm ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_MAPXFORM\n" );
  if ( !UsedModelFilters[ ModelFilter_MeshSmooth ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_SMOOTH\n" );
  if ( !UsedModelFilters[ ModelFilter_TintMesh ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CALCULATETINT\n" );
  if ( !UsedModelFilters[ ModelFilter_TintMeshShape ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CALCULATETINTSHAPE\n" );
  if ( !UsedModelFilters[ ModelFilter_Replicate ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_REPLICATE\n" );
  if ( !UsedModelFilters[ ModelFilter_NormalDeform ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_NORMALDEFORM\n" );
  if ( !UsedModelFilters[ ModelFilter_CSG ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_CSG\n" );
  if ( !UsedModelFilters[ ModelFilter_Greeble ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_GREEBLE\n" );
  if ( !hasUVClip ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_MESH_UV_HAS_CLIPPING\n" );
  if ( !scatterVertexSupport ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_VERTEX_SUPPORT\n" );
  if ( !scatterEdgeSupport ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_EDGE_SUPPORT\n" );
  if ( !scatterPolySupport ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_POLY_SUPPORT\n" );
  if ( !scatterPolyMultiSupport ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_POLY_MORE_THAN_1_INSTANCE_SUPPORT\n" );
  if ( !hasScatterOrientationOriginal ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_ORIENTATION_ORIGINAL\n" );
  if ( !hasScatterOrientationNormal ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_ORIENTATION_NORMAL\n" );
  if ( !hasScatterOrientationNormalRotate ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_ORIENTATION_NORMALROTATE\n" );
  if ( !hasScatterOrientationFullRotate ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_SCATTER_HAS_ORIENTATION_FULLROTATE\n" );


  if ( !UsedScenePrimitives[ Object_Model ] )               fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_MODEL\n" );
  if ( !UsedScenePrimitives[ Object_Light ] )               fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_LIGHT\n" );
  if ( !UsedScenePrimitives[ Object_CamEye ] )              fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_CAMEYE\n" );
  if ( !UsedScenePrimitives[ Object_Dummy ] )               fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_DUMMY\n" );
  if ( !UsedScenePrimitives[ Object_SubScene ] )            fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_SUBSCENE\n" );
  if ( !UsedScenePrimitives[ Object_ParticleEmitter ] )     fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_EMITTER\n" );
  if ( !UsedScenePrimitives[ Object_ParticleEmitterCPU ] )  fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_EMITTERCPU\n" );
  if ( !UsedScenePrimitives[ Object_ParticleDrag ] )        fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_PARTICLEDRAG\n" );
  if ( !UsedScenePrimitives[ Object_ParticleGravity ] )     fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_PARTICLEGRAVITY\n" );
  if ( !UsedScenePrimitives[ Object_ParticleTurbulence ] )  fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_PARTICLETURBULENCE\n" );
  if ( !UsedScenePrimitives[ Object_ParticleVortex ] )      fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_PARTICLEVORTEX\n" );
  if ( !UsedScenePrimitives[ Object_LogicObject ] )         fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_OBJ_LOGICOBJECT\n" );

  if ( !hasNormalParticle ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_HAS_STANDARD_PARTICLES\n" );
  if ( !hasMeshParticle ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_HAS_MESH_PARTICLES\n" );
  if ( !hasSubSceneParticle ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_HAS_SUBSCENE_PARTICLES\n" );
  if ( !hasParticleSort ) fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_HAS_PARTICLE_SORTING\n" );

  if ( !UsedEventTypes[ EVENT_ENDDEMO ] )         fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_ENDDEMO\n" );
  if ( !UsedEventTypes[ EVENT_RENDERDEMO ] )      fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_RENDERDEMO\n" );
  if ( !UsedEventTypes[ EVENT_SHADERTOY ] )       fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_SHADERTOY\n" );
  if ( !UsedEventTypes[ EVENT_RENDERSCENE ] )     fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_RENDERSCENE\n" );
  if ( !UsedEventTypes[ EVENT_PARTICLECALC ] )    fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_PARTICLECALC\n" );
  if ( !UsedEventTypes[ EVENT_CAMERASHAKE ] )     fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_CAMSHAKE\n" );
  if ( !UsedEventTypes[ EVENT_CAMERAOVERRIDE ] )  fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_EVENT_CAMOVERRIDE\n" );

  if ( !floatSplineInterpolations[ INTERPOLATION_CONSTANT ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_INTERPOLATION_CONSTANT\n" );
  if ( !floatSplineInterpolations[ INTERPOLATION_LINEAR ] )   fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_INTERPOLATION_LINEAR\n" );
  if ( !floatSplineInterpolations[ INTERPOLATION_CUBIC ] )    fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_INTERPOLATION_CUBIC\n" );
  if ( !floatSplineInterpolations[ INTERPOLATION_BEZIER ] )   fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_INTERPOLATION_BEZIER\n" );
  if ( !splineHasZeroKeyExport ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_HASZEROKEYEXPORT\n" );

  if ( !hasTechParams ) fprintf_s( f, "//" ); fprintf_s( f, "#define HAS_TECH_PARAMS\n" );

  if ( !quatSplineInterpolations[ INTERPOLATION_LINEAR ] )   fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_INTERPOLATION_SLERP\n" );
  if ( !quatSplineInterpolations[ INTERPOLATION_CUBIC ] )    fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_INTERPOLATION_SQUAD\n" );

  if ( !floatSplineWaveforms[ WAVEFORM_SIN ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_WAVEFORM_SIN\n" );
  if ( !floatSplineWaveforms[ WAVEFORM_SQUARE ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_WAVEFORM_SQUARE\n" );
  if ( !floatSplineWaveforms[ WAVEFORM_TRIANGLE ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_WAVEFORM_TRIANGLE\n" );
  if ( !floatSplineWaveforms[ WAVEFORM_SAWTOOTH ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_WAVEFORM_SAWTOOTH\n" );
  if ( !floatSplineWaveforms[ WAVEFORM_NOISE ] ) fprintf_s( f, "//" ); fprintf_s( f, "#define SPLINE_WAVEFORM_NOISE\n" );

  if ( !hasNonRelativeRts )fprintf_s( f, "//" ); fprintf_s( f, "#define PHX_HAS_NON_DEMO_RELATIVE_RENDERTARGETS\n" );


  if ( !hasLTC1 ) fprintf_s( f, "//" ); fprintf_s( f, "#define LTC1\n" );
  if ( !hasLTC2 ) fprintf_s( f, "//" ); fprintf_s( f, "#define LTC2\n" );

  fprintf_s( f, "\n" );

  if ( Project.IsSongLoaded() && Project.GetSongType().Length() )
    fprintf_s( f, "#define %s\n", Project.GetSongType().GetPointer() );

  fclose( f );
}
