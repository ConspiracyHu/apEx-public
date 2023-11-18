#include "BasePCH.h"
#include "HLSLLexer.h"

//////////////////////////////////////////////////////////////////////////
// HLSL Processing lifted from from Unreal Engine 4 v4.7 (c) Epic Games

namespace CrossCompiler
{
  template <typename T, size_t N>
  char( &ArraySizeHelper( T( &array )[ N ] ) )[ N ];
#define ArraySize(array) (sizeof(ArraySizeHelper(array)))
#define MATCH_TARGET(S) S, (TS32)ArraySize(S) - 1

  static FORCEINLINE bool IsSpaceOrTab( TCHAR Char )
  {
    return Char == ' ' || Char == '\t';
  }

  static FORCEINLINE bool IsEOL( TCHAR Char )
  {
    return Char == '\r' || Char == '\n';
  }

  static FORCEINLINE bool IsSpaceOrTabOrEOL( TCHAR Char )
  {
    return IsEOL( Char ) || IsSpaceOrTab( Char );
  }

  static FORCEINLINE bool IsAlpha( TCHAR Char )
  {
    return ( Char >= 'a' && Char <= 'z' ) || ( Char >= 'A' && Char <= 'Z' );
  }

  static FORCEINLINE bool IsDigit( TCHAR Char )
  {
    return Char >= '0' && Char <= '9';
  }

  static FORCEINLINE bool IsHexDigit( TCHAR Char )
  {
    return IsDigit( Char ) || ( Char >= 'a' && Char <= 'f' ) || ( Char >= 'A' && Char <= 'F' );
  }

  static FORCEINLINE bool IsAlphaOrDigit( TCHAR Char )
  {
    return IsAlpha( Char ) || IsDigit( Char );
  }

  struct FKeywordToken;
  typedef CDictionary<TCHAR, FKeywordToken> TCharKeywordTokenMap;
  typedef CDictionary<int, CString> TokenNameMap;

  struct FKeywordToken
  {
    EHlslToken Current;
    TCharKeywordTokenMap* Map = NULL;

    FKeywordToken() : Current( EHlslToken::Invalid ), Map( nullptr ) {}
    virtual ~FKeywordToken() { SAFEDELETE( Map ); }
  };

  TCharKeywordTokenMap Keywords;

  CString tokenNames[] = 
  {
    _T("Invalid"),
    _T("Plus"),
    _T("PlusEqual"),
    _T("Minus"),
    _T("MinusEqual"),
    _T("Times"),
    _T("TimesEqual"),
    _T("Div"),
    _T("DivEqual"),
    _T("Mod"),
    _T("ModEqual"),
    _T("LeftParenthesis"),
    _T("RightParenthesis"),
    _T("EqualEqual"),
    _T("NotEqual"),
    _T("Lower"),
    _T("LowerEqual"),
    _T("Greater"),
    _T("GreaterEqual"),
    _T("AndAnd"),
    _T("OrOr"),
    _T("LowerLower"),
    _T("LowerLowerEqual"),
    _T("GreaterGreater"),
    _T("GreaterGreaterEqual"),
    _T("And"),
    _T("AndEqual"),
    _T("Or"),
    _T("OrEqual"),
    _T("Xor"),
    _T("XorEqual"),
    _T("Not"),
    _T("Neg"),
    _T("Equal"),
    _T("LeftBrace"),
    _T("RightBrace"),
    _T("Semicolon"),
    _T("If"),
    _T("Else"),
    _T("For"),
    _T("While"),
    _T("Do"),
    _T("Return"),
    _T("Switch"),
    _T("Case"),
    _T("Break"),
    _T("Default"),
    _T("Continue"),
    _T("Goto"),
    _T("PlusPlus"),
    _T("MinusMinus"),
    _T("Void"),
    _T("Const"),
    _T("Bool"),
    _T("Bool1"),
    _T("Bool2"),
    _T("Bool3"),
    _T("Bool4"),
    _T("Bool1x1"),
    _T("Bool2x1"),
    _T("Bool3x1"),
    _T("Bool4x1"),
    _T("Bool1x2"),
    _T("Bool2x2"),
    _T("Bool3x2"),
    _T("Bool4x2"),
    _T("Bool1x3"),
    _T("Bool2x3"),
    _T("Bool3x3"),
    _T("Bool4x3"),
    _T("Bool1x4"),
    _T("Bool2x4"),
    _T("Bool3x4"),
    _T("Bool4x4"),
    _T("Int"),
    _T("Int1"),
    _T("Int2"),
    _T("Int3"),
    _T("Int4"),
    _T("Int1x1"),
    _T("Int2x1"),
    _T("Int3x1"),
    _T("Int4x1"),
    _T("Int1x2"),
    _T("Int2x2"),
    _T("Int3x2"),
    _T("Int4x2"),
    _T("Int1x3"),
    _T("Int2x3"),
    _T("Int3x3"),
    _T("Int4x3"),
    _T("Int1x4"),
    _T("Int2x4"),
    _T("Int3x4"),
    _T("Int4x4"),
    _T("Uint"),
    _T("Uint1"),
    _T("Uint2"),
    _T("Uint3"),
    _T("Uint4"),
    _T("Uint1x1"),
    _T("Uint2x1"),
    _T("Uint3x1"),
    _T("Uint4x1"),
    _T("Uint1x2"),
    _T("Uint2x2"),
    _T("Uint3x2"),
    _T("Uint4x2"),
    _T("Uint1x3"),
    _T("Uint2x3"),
    _T("Uint3x3"),
    _T("Uint4x3"),
    _T("Uint1x4"),
    _T("Uint2x4"),
    _T("Uint3x4"),
    _T("Uint4x4"),
    _T("Half"),
    _T("Half1"),
    _T("Half2"),
    _T("Half3"),
    _T("Half4"),
    _T("Half1x1"),
    _T("Half2x1"),
    _T("Half3x1"),
    _T("Half4x1"),
    _T("Half1x2"),
    _T("Half2x2"),
    _T("Half3x2"),
    _T("Half4x2"),
    _T("Half1x3"),
    _T("Half2x3"),
    _T("Half3x3"),
    _T("Half4x3"),
    _T("Half1x4"),
    _T("Half2x4"),
    _T("Half3x4"),
    _T("Half4x4"),
    _T("Float"),
    _T("Float1"),
    _T("Float2"),
    _T("Float3"),
    _T("Float4"),
    _T("Float1x1"),
    _T("Float2x1"),
    _T("Float3x1"),
    _T("Float4x1"),
    _T("Float1x2"),
    _T("Float2x2"),
    _T("Float3x2"),
    _T("Float4x2"),
    _T("Float1x3"),
    _T("Float2x3"),
    _T("Float3x3"),
    _T("Float4x3"),
    _T("Float1x4"),
    _T("Float2x4"),
    _T("Float3x4"),
    _T("Float4x4"),
    _T("Texture"),
    _T("Texture1D"),
    _T("Texture1DArray"),
    _T("Texture2D"),
    _T("Texture2DArray"),
    _T("Texture2DMS"),
    _T("Texture2DMSArray"),
    _T("Texture3D"),
    _T("TextureCube"),
    _T("TextureCubeArray"),
    _T("Sampler"),
    _T("Sampler1D"),
    _T("Sampler2D"),
    _T("Sampler3D"),
    _T("SamplerCube"),
    _T("SamplerState"),
    _T("SamplerComparisonState"),
    _T("Buffer"),
    _T("AppendStructuredBuffer"),
    _T("ByteAddressBuffer"),
    _T("ConsumeStructuredBuffer"),
    _T("RWBuffer"),
    _T("RWByteAddressBuffer"),
    _T("RWStructuredBuffer"),
    _T("RWTexture1D"),
    _T("RWTexture1DArray"),
    _T("RWTexture2D"),
    _T("RWTexture2DArray"),
    _T("RWTexture3D"),
    _T("StructuredBuffer"),
    _T("InputPatch"),
    _T("OutputPatch"),
    _T("In"),
    _T("Out"),
    _T("InOut"),
    _T("Static"),
    _T("LeftSquareBracket"),
    _T("RightSquareBracket"),
    _T("Question"),
    _T("Colon"),
    _T("Comma"),
    _T("Dot"),
    _T("Struct"),
    _T("CBuffer"),
    _T("GroupShared"),
    _T("NoInterpolation"),
    _T("RowMajor"),
    _T("Identifier"),
    _T("UnsignedIntegerConstant"),
    _T("FloatConstant"),
    _T("BoolConstant"),
    _T("StringConstant"),
    _T("Register"),
    _T("DefineMacro"),
  };

  CString GetTokenName( EHlslToken Token )
  {
    return tokenNames[ (int)Token ];
  }

  static void InsertToken( const TCHAR* String, EHlslToken Token )
  {
    tokenNames[ (int)Token ] = CString( String );

    TCharKeywordTokenMap* Map = &Keywords;
    while ( *String )
    {
      FKeywordToken& KT = Map->GetByKey( *String );
      ++String;
      if ( !*String )
      {
        KT.Current = Token;
        return;
      }

      if ( !KT.Map )
      {
        KT.Map = new TCharKeywordTokenMap();
      }

      Map = (TCharKeywordTokenMap*)KT.Map;
    }
  }

  static bool MatchSymbolToken( const TCHAR* InString, const TCHAR** OutString, EHlslToken& OutToken, CString* OutTokenString, bool bGreedy )
  {
    const TCHAR* OriginalString = InString;
    FKeywordToken* Found = NULL;
    if ( Keywords.HasKey( ( *InString ) ) )
      Found = &Keywords[ *InString ];

    if ( OutString )
    {
      *OutString = OriginalString;
    }

    if ( !Found )
    {
      return false;
    }

    do
    {
      ++InString;
      if ( Found->Map )
      {
        auto* Map = (TCharKeywordTokenMap*)Found->Map;
        FKeywordToken* NewFound = NULL;
        if ( Map->HasKey( *InString ) )
          NewFound = &Map->GetByKey( *InString );

        if ( !NewFound )
        {
          if ( Found->Current != EHlslToken::Invalid )
          {
            // Don't early out on a partial match (e.g., Texture1DSample should not be 2 tokens)
            if ( !bGreedy || !*InString )
            {
              OutToken = Found->Current;
              if ( OutTokenString )
              {
                *OutTokenString = _T( "" );
                OutTokenString->Append( OriginalString, (TS32)( InString - OriginalString ) );
              }

              if ( OutString )
              {
                *OutString = InString;
              }
              return true;
            }
          }

          return false;
        }
        Found = NewFound;
      }
      else if ( bGreedy && *InString )
      {
        break;
      }
      else
      {
        OutToken = Found->Current;
        if ( OutTokenString )
        {
          *OutTokenString = _T( "" );
          OutTokenString->Append( OriginalString, (TS32)( InString - OriginalString ) );
        }

        if ( OutString )
        {
          *OutString = InString;
        }
        return true;
      }
    } while ( *InString );

    return false;
  }

  namespace Tokens
  {
    static struct FStaticInitializer
    {
      FStaticInitializer()
      {
        // Math
        InsertToken( _T( "+" ), EHlslToken::Plus );
        InsertToken( _T( "+=" ), EHlslToken::PlusEqual );
        InsertToken( _T( "-" ), EHlslToken::Minus );
        InsertToken( _T( "-=" ), EHlslToken::MinusEqual );
        InsertToken( _T( "*" ), EHlslToken::Times );
        InsertToken( _T( "*=" ), EHlslToken::TimesEqual );
        InsertToken( _T( "/" ), EHlslToken::Div );
        InsertToken( _T( "/=" ), EHlslToken::DivEqual );
        InsertToken( _T( "%" ), EHlslToken::Mod );
        InsertToken( _T( "%=" ), EHlslToken::ModEqual );
        InsertToken( _T( "(" ), EHlslToken::LeftParenthesis );
        InsertToken( _T( ")" ), EHlslToken::RightParenthesis );

        // Logical
        InsertToken( _T( "==" ), EHlslToken::EqualEqual );
        InsertToken( _T( "!=" ), EHlslToken::NotEqual );
        InsertToken( _T( "<" ), EHlslToken::Lower );
        InsertToken( _T( "<=" ), EHlslToken::LowerEqual );
        InsertToken( _T( ">" ), EHlslToken::Greater );
        InsertToken( _T( ">=" ), EHlslToken::GreaterEqual );
        InsertToken( _T( "&&" ), EHlslToken::AndAnd );
        InsertToken( _T( "||" ), EHlslToken::OrOr );

        // Bit
        InsertToken( _T( "<<" ), EHlslToken::LowerLower );
        InsertToken( _T( "<<=" ), EHlslToken::LowerLowerEqual );
        InsertToken( _T( ">>" ), EHlslToken::GreaterGreater );
        InsertToken( _T( ">>=" ), EHlslToken::GreaterGreaterEqual );
        InsertToken( _T( "&" ), EHlslToken::And );
        InsertToken( _T( "&=" ), EHlslToken::And );
        InsertToken( _T( "|" ), EHlslToken::Or );
        InsertToken( _T( "|=" ), EHlslToken::OrEqual );
        InsertToken( _T( "^" ), EHlslToken::Xor );
        InsertToken( _T( "^=" ), EHlslToken::XorEqual );
        InsertToken( _T( "!" ), EHlslToken::Not );
        InsertToken( _T( "~" ), EHlslToken::Neg );

        // Statements/Keywords
        InsertToken( _T( "=" ), EHlslToken::Equal );
        InsertToken( _T( "{" ), EHlslToken::LeftBrace );
        InsertToken( _T( "}" ), EHlslToken::RightBrace );
        InsertToken( _T( ";" ), EHlslToken::Semicolon );
        InsertToken( _T( "if" ), EHlslToken::If );
        InsertToken( _T( "else" ), EHlslToken::Else );
        InsertToken( _T( "for" ), EHlslToken::For );
        InsertToken( _T( "while" ), EHlslToken::While );
        InsertToken( _T( "do" ), EHlslToken::Do );
        InsertToken( _T( "return" ), EHlslToken::Return );
        InsertToken( _T( "switch" ), EHlslToken::Switch );
        InsertToken( _T( "case" ), EHlslToken::Case );
        InsertToken( _T( "break" ), EHlslToken::Break );
        InsertToken( _T( "default" ), EHlslToken::Default );
        InsertToken( _T( "continue" ), EHlslToken::Continue );
        InsertToken( _T( "goto" ), EHlslToken::Goto );

        // Unary
        InsertToken( _T( "++" ), EHlslToken::PlusPlus );
        InsertToken( _T( "--" ), EHlslToken::MinusMinus );

        // Types
        InsertToken( _T( "void" ), EHlslToken::Void );
        InsertToken( _T( "const" ), EHlslToken::Const );

        InsertToken( _T( "bool" ), EHlslToken::Bool );
        InsertToken( _T( "bool1" ), EHlslToken::Bool1 );
        InsertToken( _T( "bool2" ), EHlslToken::Bool2 );
        InsertToken( _T( "bool3" ), EHlslToken::Bool3 );
        InsertToken( _T( "bool4" ), EHlslToken::Bool4 );
        InsertToken( _T( "bool1x1" ), EHlslToken::Bool1x1 );
        InsertToken( _T( "bool2x1" ), EHlslToken::Bool2x1 );
        InsertToken( _T( "bool3x1" ), EHlslToken::Bool3x1 );
        InsertToken( _T( "bool4x1" ), EHlslToken::Bool4x1 );
        InsertToken( _T( "bool1x2" ), EHlslToken::Bool1x2 );
        InsertToken( _T( "bool2x2" ), EHlslToken::Bool2x2 );
        InsertToken( _T( "bool3x2" ), EHlslToken::Bool3x2 );
        InsertToken( _T( "bool4x2" ), EHlslToken::Bool4x2 );
        InsertToken( _T( "bool1x3" ), EHlslToken::Bool1x3 );
        InsertToken( _T( "bool2x3" ), EHlslToken::Bool2x3 );
        InsertToken( _T( "bool3x3" ), EHlslToken::Bool3x3 );
        InsertToken( _T( "bool4x3" ), EHlslToken::Bool4x3 );
        InsertToken( _T( "bool1x4" ), EHlslToken::Bool1x4 );
        InsertToken( _T( "bool2x4" ), EHlslToken::Bool2x4 );
        InsertToken( _T( "bool3x4" ), EHlslToken::Bool3x4 );
        InsertToken( _T( "bool4x4" ), EHlslToken::Bool4x4 );

        InsertToken( _T( "int" ), EHlslToken::Int );
        InsertToken( _T( "int1" ), EHlslToken::Int1 );
        InsertToken( _T( "int2" ), EHlslToken::Int2 );
        InsertToken( _T( "int3" ), EHlslToken::Int3 );
        InsertToken( _T( "int4" ), EHlslToken::Int4 );
        InsertToken( _T( "int1x1" ), EHlslToken::Int1x1 );
        InsertToken( _T( "int2x1" ), EHlslToken::Int2x1 );
        InsertToken( _T( "int3x1" ), EHlslToken::Int3x1 );
        InsertToken( _T( "int4x1" ), EHlslToken::Int4x1 );
        InsertToken( _T( "int1x2" ), EHlslToken::Int1x2 );
        InsertToken( _T( "int2x2" ), EHlslToken::Int2x2 );
        InsertToken( _T( "int3x2" ), EHlslToken::Int3x2 );
        InsertToken( _T( "int4x2" ), EHlslToken::Int4x2 );
        InsertToken( _T( "int1x3" ), EHlslToken::Int1x3 );
        InsertToken( _T( "int2x3" ), EHlslToken::Int2x3 );
        InsertToken( _T( "int3x3" ), EHlslToken::Int3x3 );
        InsertToken( _T( "int4x3" ), EHlslToken::Int4x3 );
        InsertToken( _T( "int1x4" ), EHlslToken::Int1x4 );
        InsertToken( _T( "int2x4" ), EHlslToken::Int2x4 );
        InsertToken( _T( "int3x4" ), EHlslToken::Int3x4 );
        InsertToken( _T( "int4x4" ), EHlslToken::Int4x4 );

        InsertToken( _T( "uint" ), EHlslToken::Uint );
        InsertToken( _T( "uint1" ), EHlslToken::Uint1 );
        InsertToken( _T( "uint2" ), EHlslToken::Uint2 );
        InsertToken( _T( "uint3" ), EHlslToken::Uint3 );
        InsertToken( _T( "uint4" ), EHlslToken::Uint4 );
        InsertToken( _T( "uint1x1" ), EHlslToken::Uint1x1 );
        InsertToken( _T( "uint2x1" ), EHlslToken::Uint2x1 );
        InsertToken( _T( "uint3x1" ), EHlslToken::Uint3x1 );
        InsertToken( _T( "uint4x1" ), EHlslToken::Uint4x1 );
        InsertToken( _T( "uint1x2" ), EHlslToken::Uint1x2 );
        InsertToken( _T( "uint2x2" ), EHlslToken::Uint2x2 );
        InsertToken( _T( "uint3x2" ), EHlslToken::Uint3x2 );
        InsertToken( _T( "uint4x2" ), EHlslToken::Uint4x2 );
        InsertToken( _T( "uint1x3" ), EHlslToken::Uint1x3 );
        InsertToken( _T( "uint2x3" ), EHlslToken::Uint2x3 );
        InsertToken( _T( "uint3x3" ), EHlslToken::Uint3x3 );
        InsertToken( _T( "uint4x3" ), EHlslToken::Uint4x3 );
        InsertToken( _T( "uint1x4" ), EHlslToken::Uint1x4 );
        InsertToken( _T( "uint2x4" ), EHlslToken::Uint2x4 );
        InsertToken( _T( "uint3x4" ), EHlslToken::Uint3x4 );
        InsertToken( _T( "uint4x4" ), EHlslToken::Uint4x4 );

        InsertToken( _T( "half" ), EHlslToken::Half );
        InsertToken( _T( "half1" ), EHlslToken::Half1 );
        InsertToken( _T( "half2" ), EHlslToken::Half2 );
        InsertToken( _T( "half3" ), EHlslToken::Half3 );
        InsertToken( _T( "half4" ), EHlslToken::Half4 );
        InsertToken( _T( "half1x1" ), EHlslToken::Half1x1 );
        InsertToken( _T( "half2x1" ), EHlslToken::Half2x1 );
        InsertToken( _T( "half3x1" ), EHlslToken::Half3x1 );
        InsertToken( _T( "half4x1" ), EHlslToken::Half4x1 );
        InsertToken( _T( "half1x2" ), EHlslToken::Half1x2 );
        InsertToken( _T( "half2x2" ), EHlslToken::Half2x2 );
        InsertToken( _T( "half3x2" ), EHlslToken::Half3x2 );
        InsertToken( _T( "half4x2" ), EHlslToken::Half4x2 );
        InsertToken( _T( "half1x3" ), EHlslToken::Half1x3 );
        InsertToken( _T( "half2x3" ), EHlslToken::Half2x3 );
        InsertToken( _T( "half3x3" ), EHlslToken::Half3x3 );
        InsertToken( _T( "half4x3" ), EHlslToken::Half4x3 );
        InsertToken( _T( "half1x4" ), EHlslToken::Half1x4 );
        InsertToken( _T( "half2x4" ), EHlslToken::Half2x4 );
        InsertToken( _T( "half3x4" ), EHlslToken::Half3x4 );
        InsertToken( _T( "half4x4" ), EHlslToken::Half4x4 );

        InsertToken( _T( "float" ), EHlslToken::Float );
        InsertToken( _T( "float1" ), EHlslToken::Float1 );
        InsertToken( _T( "float2" ), EHlslToken::Float2 );
        InsertToken( _T( "float3" ), EHlslToken::Float3 );
        InsertToken( _T( "float4" ), EHlslToken::Float4 );
        InsertToken( _T( "float1x1" ), EHlslToken::Float1x1 );
        InsertToken( _T( "float2x1" ), EHlslToken::Float2x1 );
        InsertToken( _T( "float3x1" ), EHlslToken::Float3x1 );
        InsertToken( _T( "float4x1" ), EHlslToken::Float4x1 );
        InsertToken( _T( "float1x2" ), EHlslToken::Float1x2 );
        InsertToken( _T( "float2x2" ), EHlslToken::Float2x2 );
        InsertToken( _T( "float3x2" ), EHlslToken::Float3x2 );
        InsertToken( _T( "float4x2" ), EHlslToken::Float4x2 );
        InsertToken( _T( "float1x3" ), EHlslToken::Float1x3 );
        InsertToken( _T( "float2x3" ), EHlslToken::Float2x3 );
        InsertToken( _T( "float3x3" ), EHlslToken::Float3x3 );
        InsertToken( _T( "float4x3" ), EHlslToken::Float4x3 );
        InsertToken( _T( "float1x4" ), EHlslToken::Float1x4 );
        InsertToken( _T( "float2x4" ), EHlslToken::Float2x4 );
        InsertToken( _T( "float3x4" ), EHlslToken::Float3x4 );
        InsertToken( _T( "float4x4" ), EHlslToken::Float4x4 );

        InsertToken( _T( "Texture" ), EHlslToken::Texture );
        InsertToken( _T( "Texture1D" ), EHlslToken::Texture1D );
        InsertToken( _T( "Texture1DArray" ), EHlslToken::Texture1DArray );
        InsertToken( _T( "Texture2D" ), EHlslToken::Texture2D );
        InsertToken( _T( "Texture2DArray" ), EHlslToken::Texture2DArray );
        InsertToken( _T( "Texture2DMS" ), EHlslToken::Texture2DMS );
        InsertToken( _T( "Texture2DMSArray" ), EHlslToken::Texture2DMSArray );
        InsertToken( _T( "Texture3D" ), EHlslToken::Texture3D );
        InsertToken( _T( "TextureCube" ), EHlslToken::TextureCube );
        InsertToken( _T( "TextureCubeArray" ), EHlslToken::TextureCubeArray );

        InsertToken( _T( "sampler" ), EHlslToken::Sampler );
        InsertToken( _T( "sampler1D" ), EHlslToken::Sampler1D );
        InsertToken( _T( "sampler2D" ), EHlslToken::Sampler2D );
        InsertToken( _T( "sampler3D" ), EHlslToken::Sampler3D );
        InsertToken( _T( "samplerCUBE" ), EHlslToken::SamplerCube );
        InsertToken( _T( "SamplerState" ), EHlslToken::SamplerState );
        InsertToken( _T( "SamplerComparisonState" ), EHlslToken::SamplerComparisonState );

        InsertToken( _T( "Buffer" ), EHlslToken::Buffer );
        InsertToken( _T( "AppendStructuredBuffer" ), EHlslToken::AppendStructuredBuffer );
        InsertToken( _T( "ByteAddressBuffer" ), EHlslToken::ByteAddressBuffer );
        InsertToken( _T( "ConsumeStructuredBuffer" ), EHlslToken::ConsumeStructuredBuffer );
        InsertToken( _T( "RWBuffer" ), EHlslToken::RWBuffer );
        InsertToken( _T( "RWByteAddressBuffer" ), EHlslToken::RWByteAddressBuffer );
        InsertToken( _T( "RWStructuredBuffer" ), EHlslToken::RWStructuredBuffer );
        InsertToken( _T( "RWTexture1D" ), EHlslToken::RWTexture1D );
        InsertToken( _T( "RWTexture1DArray" ), EHlslToken::RWTexture1DArray );
        InsertToken( _T( "RWTexture2D" ), EHlslToken::RWTexture2D );
        InsertToken( _T( "RWTexture2DArray" ), EHlslToken::RWTexture2DArray );
        InsertToken( _T( "RWTexture3D" ), EHlslToken::RWTexture3D );
        InsertToken( _T( "StructuredBuffer" ), EHlslToken::StructuredBuffer );
        InsertToken( _T( "InputPatch" ), EHlslToken::InputPatch );
        InsertToken( _T( "OutputPatch" ), EHlslToken::OutputPatch );

        // Modifiers
        InsertToken( _T( "in" ), EHlslToken::In );
        InsertToken( _T( "out" ), EHlslToken::Out );
        InsertToken( _T( "inout" ), EHlslToken::InOut );
        InsertToken( _T( "static" ), EHlslToken::Static );

        // Misc
        InsertToken( _T( "[" ), EHlslToken::LeftSquareBracket );
        InsertToken( _T( "]" ), EHlslToken::RightSquareBracket );
        InsertToken( _T( "?" ), EHlslToken::Question );
        InsertToken( _T( ":" ), EHlslToken::Colon );
        InsertToken( _T( "," ), EHlslToken::Comma );
        InsertToken( _T( "." ), EHlslToken::Dot );
        InsertToken( _T( "struct" ), EHlslToken::Struct );
        InsertToken( _T( "cbuffer" ), EHlslToken::CBuffer );
        InsertToken( _T( "groupshared" ), EHlslToken::GroupShared );
        InsertToken( _T( "nointerpolation" ), EHlslToken::NoInterpolation );
        InsertToken( _T( "row_major" ), EHlslToken::RowMajor );
        InsertToken( _T( "register" ), EHlslToken::Register );
      }
    } GStaticInitializer;
  }

  struct FTokenizer
  {
    CString Filename;
    const TCHAR* Current;
    const TCHAR* End;
    const TCHAR* CurrentLineStart;
    TS32 Line;

    FTokenizer( const CString& InString, const CString& InFilename = _T( "" ) ) :
      Filename( InFilename ),
      Current( nullptr ),
      End( nullptr ),
      CurrentLineStart( nullptr ),
      Line( 0 )
    {
      if ( InString.Length() > 0 )
      {
        Current = InString.GetPointer();
        End = InString.GetPointer() + InString.Length();
        Line = 1;
        CurrentLineStart = Current;
      }
    }

    bool HasCharsAvailable() const
    {
      return Current < End;
    }

    void SkipWhitespaceInLine()
    {
      while ( HasCharsAvailable() )
      {
        auto Char = Peek();
        if ( !IsSpaceOrTab( Char ) )
        {
          break;
        }

        ++Current;
      }
    }

    void SkipWhitespaceAndEmptyLines()
    {
      while ( HasCharsAvailable() )
      {
        SkipWhitespaceInLine();
        auto Char = Peek();
        if ( IsEOL( Char ) )
        {
          SkipToNextLine();
        }
        else
        {
          auto NextChar = Peek( 1 );
          if ( Char == '/' && NextChar == '/' )
          {
            // C++ comment
            Current += 2;
            this->SkipToNextLine();
            continue;
          }
          else if ( Char == '/' && NextChar == '*' )
          {
            // C Style comment, eat everything up to * /
            Current += 2;
            bool bClosedComment = false;
            while ( HasCharsAvailable() )
            {
              if ( Peek() == '*' )
              {
                if ( Peek( 1 ) == '/' )
                {
                  bClosedComment = true;
                  Current += 2;
                  break;
                }

              }
              else if ( Peek() == '\n' )
              {
                SkipToNextLine();

                // Don't increment current!
                continue;
              }

              ++Current;
            }
            //@todo-rco: Error if no closing * / found and we got to EOL
            //BASEASSERT(bClosedComment);
          }
          else
          {
            break;
          }
        }
      }
    }

    TCHAR Peek() const
    {
      if ( HasCharsAvailable() )
      {
        return *Current;
      }

      return 0;
    }

    TCHAR Peek( TS32 Delta ) const
    {
      BASEASSERT( Delta > 0 );
      if ( Current + Delta < End )
      {
        return Current[ Delta ];
      }

      return 0;
    }

    void SkipToNextLine()
    {
      while ( HasCharsAvailable() )
      {
        auto Char = Peek();
        ++Current;
        if ( Char == '\r' && Peek() == '\n' )
        {
          ++Current;
          break;
        }
        else if ( Char == '\n' )
        {

          break;
        }
      }

      ++Line;
      CurrentLineStart = Current;
    }

    bool MatchString( const TCHAR* Target, TS32 TargetLen )
    {
      if ( Current + TargetLen <= End )
      {
        if ( CString::Strncmp( Current, Target, TargetLen ) == 0 )
        {
          Current += TargetLen;
          return true;
        }
      }
      return false;
    }

    bool PeekDigit() const
    {
      return IsDigit( Peek() );
    }

    bool MatchAndSkipDigits()
    {
      auto* Original = Current;
      while ( PeekDigit() )
      {
        ++Current;
      }

      return Original != Current;
    }

    bool Match( TCHAR Char )
    {
      if ( Char == Peek() )
      {
        ++Current;
        return true;
      }

      return false;
    }

    bool MatchFloatNumber( float& OutNum, CString &OutString )
    {
      auto* Original = Current;
      TCHAR Char = Peek();

      // \.[0-9]+([eE][+-]?[0-9]+)?[fF]?			-> Dot Digits+ Exp? F?
      // [0-9]+\.([eE][+-]?[0-9]+)?[fF]?			-> Digits+ Dot Exp? F?
      // [0-9]+\.[0-9]+([eE][+-]?[0-9]+)?[fF]?	-> Digits+ Dot Digits+ Exp? F?
      // [0-9]+[eE][+-]?[0-9]+[fF]?				-> Digits+ Exp F?
      // [0-9]+[fF]								-> Digits+ F
      if ( !IsDigit( Char ) && Char != '.' )
      {
        return false;
      }

      bool bExpOptional = false;
      if ( Match( '.' ) && MatchAndSkipDigits() )
      {
        bExpOptional = true;
      }
      else if ( MatchAndSkipDigits() )
      {
        if ( Match( '.' ) )
        {
          bExpOptional = true;
          MatchAndSkipDigits();
        }
        else
        {
          if ( Match( 'f' ) || Match( 'F' ) )
          {
            goto Done;
          }

          bExpOptional = false;
        }
      }
      else
      {
        goto NotFloat;
      }

      {
        // Exponent [eE][+-]?[0-9]+
        bool bExponentFound = false;
        if ( Match( 'e' ) || Match( 'E' ) )
        {
          Char = Peek();
          if ( Char == '+' || Char == '-' )
          {
            ++Current;
          }

          if ( MatchAndSkipDigits() )
          {
            bExponentFound = true;
          }
        }

        if ( !bExponentFound && !bExpOptional )
        {
          goto NotFloat;
        }
      }

      // [fF]
      Char = Peek();
      if ( Char == 'F' || Char == 'f' )
      {
        ++Current;
      }

    Done:
      OutNum = CString::Atof( Original );
      OutString = CString( Original, Current - Original );
      return true;

    NotFloat:
      Current = Original;
      return false;
    }

    bool MatchQuotedString( CString& OutString )
    {
      if ( !Match( '"' ) )
      {
        return false;
      }

      OutString = _T( "" );
      while ( Peek() != '"' )
      {
        OutString += Peek();
        //@todo-rco: Check for \"
        //@todo-rco: Check for EOL
        ++Current;
      }

      if ( Match( '"' ) )
      {
        return true;
      }

      //@todo-rco: Error!
      BASEASSERT( 0 );
      return false;
    }

    bool MatchIdentifier( CString& OutIdentifier )
    {
      if ( HasCharsAvailable() )
      {
        auto Char = Peek();
        if ( !IsAlpha( Char ) && Char != '_' )
        {
          return false;
        }

        ++Current;
        OutIdentifier = _T( "" );
        OutIdentifier.Append( &Char, 1 );
        do
        {
          Char = Peek();
          if ( !IsAlphaOrDigit( Char ) && Char != '_' )
          {
            break;
          }
          OutIdentifier.Append( &Char, 1 );
          ++Current;
        } while ( HasCharsAvailable() );
        return true;
      }

      return false;
    }

    bool MatchSymbol( EHlslToken& OutToken, CString& OutTokenString )
    {
      if ( HasCharsAvailable() )
      {
        if ( MatchSymbolToken( Current, &Current, OutToken, &OutTokenString, false ) )
        {
          return true;
        }
      }

      return false;
    }

    static void ProcessDirective( FHlslScanner *scanner, FTokenizer& Tokenizer )
    {
      BASEASSERT( Tokenizer.Peek() == '#' );
      if ( Tokenizer.MatchString( MATCH_TARGET( _T( "#line" ) ) ) )
      {
        Tokenizer.SkipWhitespaceInLine();
        TU32 Line = 0;
        if ( Tokenizer.RuleInteger( Line ) )
        {
          Tokenizer.Line = Line - 1;
          Tokenizer.SkipWhitespaceInLine();
          CString Filename;
          if ( Tokenizer.MatchQuotedString( Filename ) )
          {
            Tokenizer.Filename = Filename;
          }
        }
        else
        {
          //@todo-rco: Warn malformed #line directive
          BASEASSERT( 0 );
        }
      }
      else
      {
        const TCHAR *Curr = Tokenizer.Current;

        if ( Tokenizer.MatchString( MATCH_TARGET( _T( "#define" ) ) ) )
        {
          Tokenizer.SkipToNextLine();
          scanner->AddToken( FHlslToken( EHlslToken::DefineMacro, CString( Curr, Tokenizer.Current - Curr ) ), Tokenizer );
          Tokenizer.Current = Curr;
        }
        else
        {
          //@todo-rco: Warn about unknown pragma
          LOG_ERR( "[hlsl] Unknown preprocessor directive encountered." );
          BASEASSERT( 0 );
        }
      }

      Tokenizer.SkipToNextLine();
    }

    bool RuleDecimalInteger( TU32& OutValue )
    {
      // [1-9][0-9]*
      auto Char = Peek();
      {
        if ( Char < '1' || Char > '9' )
        {
          return false;
        }

        ++Current;
        OutValue = Char - '0';
      }

      while ( HasCharsAvailable() )
      {
        Char = Peek();
        if ( !IsDigit( Char ) )
        {
          break;
        }
        OutValue = OutValue * 10 + Char - '0';
        ++Current;
      }

      return true;
    }

    bool RuleOctalInteger( TU32& OutValue )
    {
      // 0[0-7]*
      auto Char = Peek();
      if ( Char != '0' )
      {
        return false;
      }

      OutValue = 0;
      ++Current;
      while ( HasCharsAvailable() )
      {
        Char = Peek();
        if ( Char >= '0' && Char <= '7' )
        {
          OutValue = OutValue * 8 + Char - '0';
        }
        else
        {
          break;
        }
        ++Current;
      }

      return true;
    }

    bool RuleHexadecimalInteger( TU32& OutValue )
    {
      // 0[xX][0-9a-zA-Z]+
      auto Char = Peek();
      auto Char1 = Peek( 1 );
      auto Char2 = Peek( 2 );
      if ( Char == '0' && ( Char1 == 'x' || Char1 == 'X' ) && IsHexDigit( Char2 ) )
      {
        Current += 2;
        OutValue = 0;
        do
        {
          Char = Peek();
          if ( IsDigit( Char ) )
          {
            OutValue = OutValue * 16 + Char - '0';
          }
          else if ( Char >= 'a' && Char <= 'f' )
          {
            OutValue = OutValue * 16 + Char - 'a' + 10;
          }
          else if ( Char >= 'A' && Char <= 'F' )
          {
            OutValue = OutValue * 16 + Char - 'A' + 10;
          }
          else
          {
            break;
          }
          ++Current;
        } while ( HasCharsAvailable() );

        return true;
      }

      return false;
    }

    bool RuleInteger( TU32& OutValue )
    {
      return RuleDecimalInteger( OutValue ) || RuleHexadecimalInteger( OutValue ) || RuleOctalInteger( OutValue );
    }

    bool MatchLiteralInteger( TU32& OutValue )
    {
      if ( RuleInteger( OutValue ) )
      {
        auto Char = Peek();
        if ( Char == 'u' || Char == 'U' )
        {
          ++Current;
        }

        return true;
      }

      return false;
    }
  };

  FHlslScanner::FHlslScanner() :
    CurrentToken( 0 )
  {
  }

  FHlslScanner::~FHlslScanner()
  {
  }

  inline void FHlslScanner::AddToken( const FHlslToken& Token, const FTokenizer& Tokenizer )
  {
    Tokens.Add( Token );
    //Tokens.Last().SourceInfo.Filename = SourceFilenames.Last();
    Tokens.Last().SourceInfo.Line = Tokenizer.Line;
    Tokens.Last().SourceInfo.Column = (TS32)( Tokenizer.Current - Tokenizer.CurrentLineStart ) + 1;
  }

  void FHlslScanner::Clear(/*const CString& Filename*/ )
  {
    Tokens.Flush();
    //new (SourceFilenames)CString(Filename);
  }

  bool FHlslScanner::Lex( const CString& String/*, const CString& Filename*/ )
  {
    Clear(/*Filename*/ );

    // Simple heuristic to avoid reallocating
    //Tokens.Reserve(String.Length() / 4);

    FTokenizer Tokenizer( String );
    while ( Tokenizer.HasCharsAvailable() )
    {
      auto* Sanity = Tokenizer.Current;
      Tokenizer.SkipWhitespaceAndEmptyLines();
      if ( Tokenizer.Peek() == '#' )
      {
        FTokenizer::ProcessDirective( this, Tokenizer );
        //if (Tokenizer.Filename != *SourceFilenames.Last())
        //{
        //	new(SourceFilenames)CString(Tokenizer.Filename);
        //}
      }
      else
      {
        CString Identifier;
        EHlslToken SymbolToken;
        TU32 UnsignedInteger;
        float FloatNumber;
        CString OriginalFloatString;

        if ( Tokenizer.MatchFloatNumber( FloatNumber, OriginalFloatString ) )
        {
          AddToken( FHlslToken( FloatNumber, OriginalFloatString ), Tokenizer );
        }
        else if ( Tokenizer.MatchLiteralInteger( UnsignedInteger ) )
        {
          AddToken( FHlslToken( UnsignedInteger ), Tokenizer );
        }
        else if ( Tokenizer.MatchIdentifier( Identifier ) )
        {
          if ( !CString::Strcmp( Identifier.GetPointer(), _T( "true" ) ) )
          {
            AddToken( FHlslToken( true ), Tokenizer );
          }
          else if ( !CString::Strcmp( Identifier.GetPointer(), _T( "false" ) ) )
          {
            AddToken( FHlslToken( false ), Tokenizer );
          }
          else if ( MatchSymbolToken( Identifier.GetPointer(), nullptr, SymbolToken, nullptr, true ) )
          {
            AddToken( FHlslToken( SymbolToken, Identifier ), Tokenizer );
          }
          else
          {
            AddToken( FHlslToken( Identifier ), Tokenizer );
          }
        }
        else if ( Tokenizer.MatchSymbol( SymbolToken, Identifier ) )
        {
          AddToken( FHlslToken( SymbolToken, Identifier ), Tokenizer );
        }
        else if ( Tokenizer.HasCharsAvailable() )
        {
          //@todo-rco: Unknown token!
          if ( Tokenizer.Filename.Length() > 0 )
          {
            LOG_ERR( "Unknown token at line %d, file '%s'!", Tokenizer.Line, Tokenizer.Filename.GetPointer() );
          }
          else
          {
            LOG_ERR( "Unknown token at line %d!", Tokenizer.Line );
          }
          return false;
        }
      }

      BASEASSERT( Sanity != Tokenizer.Current );
    }

    return true;
  }

  void FHlslScanner::Dump()
  {
    for ( TS32 Index = 0; Index < Tokens.NumItems(); ++Index )
    {
      auto& Token = Tokens[ Index ];
      switch ( Token.Token )
      {
      case EHlslToken::UnsignedIntegerConstant:
        LOG_DBG( "** %d: UnsignedIntegerConstant '%d'", Index, Token.UnsignedInteger );
        break;

      case EHlslToken::FloatConstant:
        LOG_DBG( "** %d: FloatConstant '%f'", Index, Token.Float );
        break;

      default:
        LOG_DBG( "** %d: %d '%s'", Index, Token.Token, Token.String.GetPointer() );
        break;
      }
    }
  }

  bool FHlslScanner::MatchToken( EHlslToken InToken )
  {
    const auto* Token = GetCurrentToken();
    if ( Token )
    {
      if ( Token->Token == InToken )
      {
        ++CurrentToken;
        return true;
      }
    }

    return false;
  }

  const FHlslToken* FHlslScanner::PeekToken( TU32 LookAhead /*= 0*/ ) const
  {
    if ( CurrentToken + LookAhead < (TU32)Tokens.NumItems() )
    {
      return Tokens.GetPointer( (TS32)( CurrentToken + LookAhead ) );
    }

    return nullptr;
  }

  bool FHlslScanner::HasMoreTokens() const
  {
    return CurrentToken < (TU32)Tokens.NumItems();
  }

  const FHlslToken* FHlslScanner::GetCurrentToken() const
  {
    if ( CurrentToken < (TU32)Tokens.NumItems() )
    {
      return Tokens.GetPointer( CurrentToken );
    }

    return nullptr;
  }

  const FHlslToken* FHlslScanner::GetCurrentTokenAndAdvance()
  {
    if ( CurrentToken < (TU32)Tokens.NumItems() )
    {
      auto* Return = Tokens.GetPointer( CurrentToken );
      Advance();
    }

    return nullptr;
  }

  void FHlslScanner::SetCurrentTokenIndex( TU32 NewToken )
  {
    BASEASSERT( NewToken <= (TU32)Tokens.NumItems() );
    CurrentToken = NewToken;
  }

  void FHlslScanner::SourceError( const CString& Error )
  {
    if ( CurrentToken < (TU32)Tokens.NumItems() )
    {
      const auto& Token = Tokens[ CurrentToken ];
      //BASEASSERT(Token.SourceInfo.Filename);
      LOG_ERR( "%d: (%d) %s", Token.SourceInfo.Line, Token.SourceInfo.Column, Error.GetPointer() );
    }
    else
    {
      LOG_ERR( "%s", Error.GetPointer() );
    }
  }
}