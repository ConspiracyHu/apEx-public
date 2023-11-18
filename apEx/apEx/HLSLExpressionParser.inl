#pragma once
//#include "ShaderCompilerCommon.h"
#include "HlslParser.h"
#include "HlslAST.h"

namespace CrossCompiler
{

  namespace AST
  {
    TBOOL GetOperatorAssociativity( EOperators Op );
  }

  CString GetTokenName( EHlslToken Token );


  struct FSymbolScope;
  //struct FInfo;

  EParseResult ComputeExpr( FHlslScanner& Scanner, TS32 MinPrec, /*FInfo& Info,*/ FSymbolScope* SymbolScope, bool bAllowAssignment, AST::FExpression** OutExpression, AST::FExpression** OutTernaryExpression );
  EParseResult ParseExpressionList( EHlslToken EndListToken, FHlslScanner& Scanner, FSymbolScope* SymbolScope, EHlslToken NewStartListToken, AST::FExpression* OutExpression );

  struct FSymbolScope
  {
    FSymbolScope* Parent;

    CArray/*TLinearSet*/<CString> Symbols;
    CArray<FSymbolScope> Children;

    FSymbolScope( FSymbolScope* InParent ) : Parent( InParent ), Children()/*, Symbols(InAllocator)*/ {}
    ~FSymbolScope() {}

    void Add( const CString& Type )
    {
      Symbols.AddUnique( Type );
    }

    static bool FindType( const FSymbolScope* Scope, const CString& Type )
    {
      while ( Scope )
      {
        if ( Scope->Symbols.Find( Type ) >= 0 )
        {
          return true;
        }

        Scope = Scope->Parent;
      }

      return false;
    }
  };

  struct FCreateSymbolScope
  {
    FSymbolScope *Created = NULL;

    FSymbolScope* Original;
    FSymbolScope** Current;

    FCreateSymbolScope( FSymbolScope** InCurrent ) :
      Current( InCurrent )
    {
      Original = *InCurrent;
      auto* NewScope = new FSymbolScope( Original );
      *Current = NewScope;
      Created = NewScope;
    }

    ~FCreateSymbolScope()
    {
      *Current = Original;
      delete Created;
    }
  };

  /*
    struct FInfo
    {
    TS32 Indent;
    bool bEnabled;

    FInfo(bool bInEnabled = false) : Indent(0), bEnabled(bInEnabled) {}

    void Print(const CString& Message)
    {
    if (bEnabled)
    {
    FPlatformMisc::LocalPrint(*Message);
    }
    }

    void PrintTabs()
    {
    if (bEnabled)
    {
    for (TS32 Index = 0; Index < Indent; ++Index)
    {
    FPlatformMisc::LocalPrint(_T("\t"));
    }
    }
    }

    void PrintWithTabs(const CString& Message)
    {
    PrintTabs();
    Print(Message);
    }
    };

    struct FInfoIndentScope
    {
    FInfoIndentScope(FInfo& InInfo) : Info(InInfo)
    {
    ++Info.Indent;
    }

    ~FInfoIndentScope()
    {
    --Info.Indent;
    }

    FInfo& Info;
    };*/

  enum ETypeFlags
  {
    ETF_VOID = 0x0001,
    ETF_BUILTIN_NUMERIC = 0x0002,
    ETF_SAMPLER_TEXTURE_BUFFER = 0x0004,
    ETF_USER_TYPES = 0x0008,
  };

  EParseResult ParseGeneralType( const FHlslToken* Token, TS32 TypeFlags, AST::FTypeSpecifier** OutSpecifier )
  {
    if ( !Token )
    {
      return EParseResult::Error;
    }

    bool bMatched = false;
    const TCHAR* InnerType = nullptr;
    switch ( Token->Token )
    {
    case EHlslToken::Void:
      if ( TypeFlags & ETF_VOID )
      {
        bMatched = true;
      }
      break;

    case EHlslToken::Bool:
    case EHlslToken::Bool1:
    case EHlslToken::Bool2:
    case EHlslToken::Bool3:
    case EHlslToken::Bool4:
    case EHlslToken::Bool1x1:
    case EHlslToken::Bool1x2:
    case EHlslToken::Bool1x3:
    case EHlslToken::Bool1x4:
    case EHlslToken::Bool2x1:
    case EHlslToken::Bool2x2:
    case EHlslToken::Bool2x3:
    case EHlslToken::Bool2x4:
    case EHlslToken::Bool3x1:
    case EHlslToken::Bool3x2:
    case EHlslToken::Bool3x3:
    case EHlslToken::Bool3x4:
    case EHlslToken::Bool4x1:
    case EHlslToken::Bool4x2:
    case EHlslToken::Bool4x3:
    case EHlslToken::Bool4x4:

    case EHlslToken::Int:
    case EHlslToken::Int1:
    case EHlslToken::Int2:
    case EHlslToken::Int3:
    case EHlslToken::Int4:
    case EHlslToken::Int1x1:
    case EHlslToken::Int1x2:
    case EHlslToken::Int1x3:
    case EHlslToken::Int1x4:
    case EHlslToken::Int2x1:
    case EHlslToken::Int2x2:
    case EHlslToken::Int2x3:
    case EHlslToken::Int2x4:
    case EHlslToken::Int3x1:
    case EHlslToken::Int3x2:
    case EHlslToken::Int3x3:
    case EHlslToken::Int3x4:
    case EHlslToken::Int4x1:
    case EHlslToken::Int4x2:
    case EHlslToken::Int4x3:
    case EHlslToken::Int4x4:

    case EHlslToken::Uint:
    case EHlslToken::Uint1:
    case EHlslToken::Uint2:
    case EHlslToken::Uint3:
    case EHlslToken::Uint4:
    case EHlslToken::Uint1x1:
    case EHlslToken::Uint1x2:
    case EHlslToken::Uint1x3:
    case EHlslToken::Uint1x4:
    case EHlslToken::Uint2x1:
    case EHlslToken::Uint2x2:
    case EHlslToken::Uint2x3:
    case EHlslToken::Uint2x4:
    case EHlslToken::Uint3x1:
    case EHlslToken::Uint3x2:
    case EHlslToken::Uint3x3:
    case EHlslToken::Uint3x4:
    case EHlslToken::Uint4x1:
    case EHlslToken::Uint4x2:
    case EHlslToken::Uint4x3:
    case EHlslToken::Uint4x4:

    case EHlslToken::Half:
    case EHlslToken::Half1:
    case EHlslToken::Half2:
    case EHlslToken::Half3:
    case EHlslToken::Half4:
    case EHlslToken::Half1x1:
    case EHlslToken::Half1x2:
    case EHlslToken::Half1x3:
    case EHlslToken::Half1x4:
    case EHlslToken::Half2x1:
    case EHlslToken::Half2x2:
    case EHlslToken::Half2x3:
    case EHlslToken::Half2x4:
    case EHlslToken::Half3x1:
    case EHlslToken::Half3x2:
    case EHlslToken::Half3x3:
    case EHlslToken::Half3x4:
    case EHlslToken::Half4x1:
    case EHlslToken::Half4x2:
    case EHlslToken::Half4x3:
    case EHlslToken::Half4x4:

    case EHlslToken::Float:
    case EHlslToken::Float1:
    case EHlslToken::Float2:
    case EHlslToken::Float3:
    case EHlslToken::Float4:
    case EHlslToken::Float1x1:
    case EHlslToken::Float1x2:
    case EHlslToken::Float1x3:
    case EHlslToken::Float1x4:
    case EHlslToken::Float2x1:
    case EHlslToken::Float2x2:
    case EHlslToken::Float2x3:
    case EHlslToken::Float2x4:
    case EHlslToken::Float3x1:
    case EHlslToken::Float3x2:
    case EHlslToken::Float3x3:
    case EHlslToken::Float3x4:
    case EHlslToken::Float4x1:
    case EHlslToken::Float4x2:
    case EHlslToken::Float4x3:
    case EHlslToken::Float4x4:
      if ( TypeFlags & ETF_BUILTIN_NUMERIC )
      {
        bMatched = true;
      }
      break;

    case EHlslToken::Texture:
    case EHlslToken::Texture1D:
    case EHlslToken::Texture1DArray:
    case EHlslToken::Texture2D:
    case EHlslToken::Texture2DArray:
    case EHlslToken::Texture2DMS:
    case EHlslToken::Texture2DMSArray:
    case EHlslToken::Texture3D:
    case EHlslToken::TextureCube:
    case EHlslToken::TextureCubeArray:

    case EHlslToken::Buffer:
    case EHlslToken::AppendStructuredBuffer:
    case EHlslToken::ByteAddressBuffer:
    case EHlslToken::ConsumeStructuredBuffer:
    case EHlslToken::RWBuffer:
    case EHlslToken::RWByteAddressBuffer:
    case EHlslToken::RWStructuredBuffer:
    case EHlslToken::RWTexture1D:
    case EHlslToken::RWTexture1DArray:
    case EHlslToken::RWTexture2D:
    case EHlslToken::RWTexture2DArray:
    case EHlslToken::RWTexture3D:
    case EHlslToken::StructuredBuffer:

    case EHlslToken::Sampler:
    case EHlslToken::Sampler1D:
    case EHlslToken::Sampler2D:
    case EHlslToken::Sampler3D:
    case EHlslToken::SamplerCube:
    case EHlslToken::SamplerState:
    case EHlslToken::SamplerComparisonState:
      if ( TypeFlags & ETF_SAMPLER_TEXTURE_BUFFER )
      {
        bMatched = true;
        //InnerType = _T("float4");
        InnerType = _T( "" );
      }
      break;
    }

    if ( bMatched )
    {
      auto* Type = new AST::FTypeSpecifier( Token->SourceInfo );
      Type->TypeName = Token->String;
      Type->InnerType = InnerType;
      *OutSpecifier = Type;
      return EParseResult::Matched;
    }

    return EParseResult::NotMatched;
  }

  EParseResult ParseGeneralType( const FHlslToken* Token, TS32 TypeFlags, FSymbolScope* SymbolScope, AST::FTypeSpecifier** OutSpecifier )
  {
    if ( Token )
    {
      if ( ParseGeneralType( Token, TypeFlags, OutSpecifier ) == EParseResult::Matched )
      {
        return EParseResult::Matched;
      }

      if ( TypeFlags & ETF_USER_TYPES )
      {
        BASEASSERT( SymbolScope );
        if ( Token->Token == EHlslToken::Identifier && FSymbolScope::FindType( SymbolScope, Token->String ) )
        {
          auto* Type = new AST::FTypeSpecifier( Token->SourceInfo );
          Type->TypeName = Token->String;
          *OutSpecifier = Type;
          return EParseResult::Matched;
        }
      }

      return EParseResult::NotMatched;
    }

    return EParseResult::Error;
  }

  EParseResult ParseGeneralType( FHlslScanner& Scanner, TS32 TypeFlags, FSymbolScope* SymbolScope, AST::FTypeSpecifier** OutSpecifier )
  {
    const void* Token = Scanner.PeekToken();
    if ( ParseGeneralType( (const FHlslToken*)Token, TypeFlags, SymbolScope, OutSpecifier ) == EParseResult::Matched )
    {
      Scanner.Advance();
      return EParseResult::Matched;
    }

    return EParseResult::NotMatched;
  }

  // Unary!(Unary-(Unary+())) would have ! as Top, and + as Inner
  EParseResult MatchUnaryOperator( FHlslScanner& Scanner, /*FInfo& Info,*/ FSymbolScope* SymbolScope, AST::FExpression** OuterExpression, AST::FExpression** InnerExpression )
  {
    bool bFoundAny = false;
    bool bTryAgain = true;
    AST::FExpression*& PrevExpression = *InnerExpression;
    while ( Scanner.HasMoreTokens() && bTryAgain )
    {
      auto* Token = Scanner.GetCurrentToken();
      //LOG_NFO( "MatchUnaryOperator %s: %s - %d - %f", GetTokenName( Token->Token ).GetPointer(), Token->String.GetPointer(), Token->UnsignedInteger, Token->Float );
      AST::EOperators Operator = AST::EOperators::Plus;

      switch ( Token->Token )
      {
      case EHlslToken::PlusPlus:
        bFoundAny = true;
        Scanner.Advance();
        Operator = AST::EOperators::PreInc;
        break;

      case EHlslToken::MinusMinus:
        bFoundAny = true;
        Scanner.Advance();
        Operator = AST::EOperators::PreDec;
        break;

      case EHlslToken::Plus:
        Scanner.Advance();
        bFoundAny = true;
        Operator = AST::EOperators::Plus;
        break;

      case EHlslToken::Minus:
        Scanner.Advance();
        bFoundAny = true;
        Operator = AST::EOperators::Neg;
        break;

      case EHlslToken::Not:
        Scanner.Advance();
        bFoundAny = true;
        Operator = AST::EOperators::LogicNot;
        break;

      case EHlslToken::Neg:
        Scanner.Advance();
        bFoundAny = true;
        Operator = AST::EOperators::BitNot;
        break;

      case EHlslToken::LeftParenthesis:
        // Only cast expressions are Unary
      {
        const auto* Peek1 = Scanner.PeekToken( 1 );
        const auto* Peek2 = Scanner.PeekToken( 2 );
        AST::FTypeSpecifier* TypeSpecifier = nullptr;
        if ( Peek1 && ParseGeneralType( Peek1, ETF_BUILTIN_NUMERIC | ETF_USER_TYPES, SymbolScope, &TypeSpecifier ) == EParseResult::Matched && Peek2 && Peek2->Token == EHlslToken::RightParenthesis )
        {
          // Cast
          Scanner.Advance();
          Scanner.Advance();
          Scanner.Advance();
          bFoundAny = true;

          auto* Expression = new AST::FUnaryExpression( AST::EOperators::TypeCast, nullptr, Token->SourceInfo );
          if ( PrevExpression )
          {
            PrevExpression->SubExpressions[ 0 ] = Expression;
          }

          Expression->TypeSpecifier = TypeSpecifier;

          if ( !*OuterExpression )
          {
            *OuterExpression = Expression;
          }

          PrevExpression = Expression;
          continue;
        }
        else
        {
          // Non-unary
          return bFoundAny ? EParseResult::Matched : EParseResult::NotMatched;
        }
      }
      break;

      default:
        return bFoundAny ? EParseResult::Matched : EParseResult::NotMatched;
      }

      auto* Expression = new AST::FUnaryExpression( Operator, nullptr, Token->SourceInfo );
      if ( PrevExpression )
      {
        PrevExpression->SubExpressions[ 0 ] = Expression;
      }

      if ( !*OuterExpression )
      {
        *OuterExpression = Expression;
      }

      PrevExpression = Expression;
    }

    // Ran out of tokens!
    return EParseResult::Error;
  }

  EParseResult MatchSuffixOperator( FHlslScanner& Scanner, /*FInfo& Info,*/ FSymbolScope* SymbolScope, bool bAllowAssignment, AST::FExpression** InOutExpression, AST::FExpression** OutTernaryExpression )
  {
    bool bFoundAny = false;
    bool bTryAgain = true;
    AST::FExpression*& PrevExpression = *InOutExpression;
    while ( Scanner.HasMoreTokens() && bTryAgain )
    {
      auto* Token = Scanner.GetCurrentToken();

      //LOG_NFO( "MatchSuffixOperator %s: %s - %d - %f", GetTokenName( Token->Token ).GetPointer(), Token->String.GetPointer(), Token->UnsignedInteger, Token->Float );

      AST::EOperators Operator = AST::EOperators::Plus;

      switch ( Token->Token )
      {
      case EHlslToken::LeftSquareBracket:
      {
        Scanner.Advance();
        AST::FExpression* ArrayIndex = nullptr;
        auto Result = ComputeExpr( Scanner, 1, /*Info,*/ SymbolScope, bAllowAssignment, &ArrayIndex, nullptr );
        if ( Result != EParseResult::Matched )
        {
          Scanner.SourceError( _T( "Expected expression!" ) );
          return EParseResult::Error;
        }

        if ( !Scanner.MatchToken( EHlslToken::RightSquareBracket ) )
        {
          Scanner.SourceError( _T( "Expected ']'!" ) );
          return EParseResult::Error;
        }

        auto* ArrayIndexExpression = new AST::FBinaryExpression( AST::EOperators::ArrayIndex, PrevExpression, ArrayIndex, Token->SourceInfo );
        PrevExpression = ArrayIndexExpression;
        bFoundAny = true;
      }
      break;
      case EHlslToken::Dot:
      {
        Scanner.Advance();
        const auto* Identifier = Scanner.GetCurrentToken();
        if ( !Scanner.MatchToken( EHlslToken::Identifier ) )
        {
          Scanner.SourceError( _T( "Expected identifier for member or swizzle!" ) );
          return EParseResult::Error;
        }
        auto* FieldExpression = new AST::FUnaryExpression( AST::EOperators::FieldSelection, PrevExpression, Token->SourceInfo );
        FieldExpression->Identifier = Identifier->String;
        PrevExpression = FieldExpression;
        bFoundAny = true;
      }
      break;
      case EHlslToken::LeftParenthesis:
      {
        Scanner.Advance();

        // Function Call
        auto* FunctionCall = new AST::FFunctionExpression( Token->SourceInfo, PrevExpression );
        auto Result = ParseExpressionList( EHlslToken::RightParenthesis, Scanner, SymbolScope, EHlslToken::Invalid, FunctionCall );
        if ( Result != EParseResult::Matched )
        {
          Scanner.SourceError( _T( "Expected ')'!" ) );
          return EParseResult::Error;
        }

        PrevExpression = FunctionCall;

        bFoundAny = true;
      }
      break;
      case EHlslToken::PlusPlus:
      {
        Scanner.Advance();
        auto* IncExpression = new AST::FUnaryExpression( AST::EOperators::PostInc, PrevExpression, Token->SourceInfo );
        PrevExpression = IncExpression;
        bFoundAny = true;
      }
      break;
      case EHlslToken::MinusMinus:
      {
        Scanner.Advance();
        auto* DecExpression = new AST::FUnaryExpression( AST::EOperators::PostDec, PrevExpression, Token->SourceInfo );
        PrevExpression = DecExpression;
        bFoundAny = true;
      }
      break;
      case EHlslToken::Question:
      {
        Scanner.Advance();
        AST::FExpression* Left = nullptr;
        if ( ComputeExpr( Scanner, 0, /*Info,*/ SymbolScope, true, &Left, nullptr ) != EParseResult::Matched )
        {
          Scanner.SourceError( _T( "Expected expression!" ) );
          return EParseResult::Error;
        }
        if ( !Scanner.MatchToken( EHlslToken::Colon ) )
        {
          Scanner.SourceError( _T( "Expected ':'!" ) );
          return EParseResult::Error;
        }
        AST::FExpression* Right = nullptr;
        if ( ComputeExpr( Scanner, 0, /*Info,*/ SymbolScope, true, &Right, nullptr ) != EParseResult::Matched )
        {
          Scanner.SourceError( _T( "Expected expression!" ) );
          return EParseResult::Error;
        }

        auto* Ternary = new AST::FExpression( AST::EOperators::Conditional, nullptr, Left, Right, Token->SourceInfo );
        *OutTernaryExpression = Ternary;
        //PrevExpression = Ternary;
        bFoundAny = true;
        bTryAgain = false;
      }
      break;

      default:
        bTryAgain = false;
        break;
      }
    }

    *InOutExpression = PrevExpression;
    return bFoundAny ? EParseResult::Matched : EParseResult::NotMatched;
  }

  EParseResult ComputeAtom( FHlslScanner& Scanner, /*FInfo& Info,*/ FSymbolScope* SymbolScope, bool bAllowAssignment, AST::FExpression** OutExpression, AST::FExpression** OutTernaryExpression )
  {
    AST::FExpression* InnerUnaryExpression = nullptr;
    auto UnaryResult = MatchUnaryOperator( Scanner, /*Info,*/ SymbolScope, OutExpression, &InnerUnaryExpression );
    auto* Token = Scanner.GetCurrentToken();
    if ( !Token || UnaryResult == EParseResult::Error )
    {
      return EParseResult::Error;
    }

    //LOG_NFO( "ComputeAtom %s: %s - %d - %f", GetTokenName( Token->Token ).GetPointer(), Token->String.GetPointer(), Token->UnsignedInteger, Token->Float );

    AST::FExpression* AtomExpression = nullptr;
    switch ( Token->Token )
    {
    case EHlslToken::BoolConstant:
      Scanner.Advance();
      AtomExpression = new AST::FUnaryExpression( AST::EOperators::BoolConstant, nullptr, Token->SourceInfo );
      AtomExpression->BoolConstant = Token->UnsignedInteger != 0;
      break;

    case EHlslToken::UnsignedIntegerConstant:
      Scanner.Advance();
      AtomExpression = new AST::FUnaryExpression( AST::EOperators::UintConstant, nullptr, Token->SourceInfo );
      AtomExpression->UintConstant = Token->UnsignedInteger;
      break;

    case EHlslToken::FloatConstant:
      Scanner.Advance();
      AtomExpression = new AST::FUnaryExpression( AST::EOperators::FloatConstant, nullptr, Token->SourceInfo );
      AtomExpression->FloatConstant = Token->Float;
      AtomExpression->OriginalFloatString = Token->String;
      break;

    case EHlslToken::Identifier:
      Scanner.Advance();
      AtomExpression = new AST::FUnaryExpression( AST::EOperators::Identifier, nullptr, Token->SourceInfo );
      AtomExpression->Identifier = Token->String;
      break;

    case EHlslToken::LeftParenthesis:
    {
      Scanner.Advance();

      // Check if it's a cast expression first
      const auto* Peek1 = Scanner.PeekToken( 0 );
      const auto* Peek2 = Scanner.PeekToken( 1 );
      // Parenthesis expression
      if ( ComputeExpr( Scanner, 1, /*Info,*/ SymbolScope, bAllowAssignment, &AtomExpression, nullptr ) != EParseResult::Matched )
      {
        Scanner.SourceError( _T( "Expected expression!" ) );
        return EParseResult::Error;
      }

      if ( !Scanner.MatchToken( EHlslToken::RightParenthesis ) )
      {
        Scanner.SourceError( _T( "Expected ')'!" ) );
        return EParseResult::Error;
      }
    }
    break;

    default:
    {
      AST::FTypeSpecifier* TypeSpecifier = nullptr;

      // Grrr handle Sampler as a variable name... This is safe here since Declarations are always handled first
      if ( ParseGeneralType( Scanner, ETF_SAMPLER_TEXTURE_BUFFER, nullptr, &TypeSpecifier ) == EParseResult::Matched )
      {
        //@todo-rco: Check this var exists on the symnbol table
        AtomExpression = new AST::FUnaryExpression( AST::EOperators::Identifier, nullptr, TypeSpecifier->SourceInfo );
        AtomExpression->Identifier = TypeSpecifier->TypeName;
        break;
      }
      // Handle float3(x,y,z)
      else if ( ParseGeneralType( Scanner, ETF_BUILTIN_NUMERIC, nullptr, &TypeSpecifier ) == EParseResult::Matched )
      {
        if ( Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
        {
          auto* TypeExpression = new AST::FUnaryExpression( AST::EOperators::Identifier, nullptr, TypeSpecifier->SourceInfo );
          TypeExpression->Identifier = TypeSpecifier->TypeName;
          SAFEDELETE( TypeSpecifier );
          auto* FunctionCall = new AST::FFunctionExpression( Token->SourceInfo, TypeExpression );
          auto Result = ParseExpressionList( EHlslToken::RightParenthesis, Scanner, SymbolScope, EHlslToken::Invalid, FunctionCall );
          if ( Result != EParseResult::Matched )
          {
            Scanner.SourceError( _T( "Unexpected type in numeric constructor!" ) );
            return EParseResult::Error;
          }

          AtomExpression = FunctionCall;
        }
        else
        {
          Scanner.SourceError( _T( "Unexpected type in declaration!" ) );
          return EParseResult::Error;
        }
        break;
      }
      else
      {
        if ( UnaryResult == EParseResult::Matched )
        {
          Scanner.SourceError( _T( "Expected expression!" ) );
          return EParseResult::Error;
        }

        return EParseResult::NotMatched;
      }
    }
    break;
    }

    BASEASSERT( AtomExpression );

    // Patch unary if necessary
    if ( InnerUnaryExpression )
    {
      BASEASSERT( !InnerUnaryExpression->SubExpressions[ 0 ] );
      InnerUnaryExpression->SubExpressions[ 0 ] = AtomExpression;
      AtomExpression = InnerUnaryExpression;
    }

    auto SuffixResult = MatchSuffixOperator( Scanner, /*Info,*/ SymbolScope, bAllowAssignment, &AtomExpression, OutTernaryExpression );
    //auto* Token = Scanner.GetCurrentToken();
    if (/*!Token || */SuffixResult == EParseResult::Error )
    {
      return EParseResult::Error;
    }

    //if (!*OutExpression)
    {
      *OutExpression = AtomExpression;
    }

    // original:
    //if (InnerUnaryExpression)
    //{
    //	BASEASSERT(!InnerUnaryExpression->SubExpressions[0]);
    //	InnerUnaryExpression->SubExpressions[0] = AtomExpression;
    //}
    //if (!*OutExpression)
    //{
    //	*OutExpression = AtomExpression;
    //}


    return EParseResult::Matched;
  }

  TS32 GetPrecedence( const FHlslToken* Token )
  {
    if ( Token )
    {
      switch ( Token->Token )
      {
      case EHlslToken::Equal:
      case EHlslToken::PlusEqual:
      case EHlslToken::MinusEqual:
      case EHlslToken::TimesEqual:
      case EHlslToken::DivEqual:
      case EHlslToken::ModEqual:
      case EHlslToken::GreaterGreaterEqual:
      case EHlslToken::LowerLowerEqual:
      case EHlslToken::AndEqual:
      case EHlslToken::OrEqual:
      case EHlslToken::XorEqual:
        return 1;

      case EHlslToken::Question:
        return 2;

      case EHlslToken::OrOr:
        return 3;

      case EHlslToken::AndAnd:
        return 4;

      case EHlslToken::Or:
        return 5;

      case EHlslToken::Xor:
        return 6;

      case EHlslToken::And:
        return 7;

      case EHlslToken::EqualEqual:
      case EHlslToken::NotEqual:
        return 8;

      case EHlslToken::Lower:
      case EHlslToken::Greater:
      case EHlslToken::LowerEqual:
      case EHlslToken::GreaterEqual:
        return 9;

      case EHlslToken::LowerLower:
      case EHlslToken::GreaterGreater:
        return 10;

      case EHlslToken::Plus:
      case EHlslToken::Minus:
        return 11;

      case EHlslToken::Times:
      case EHlslToken::Div:
      case EHlslToken::Mod:
        return 12;

      default:
        break;
      }
    }

    return -1;
  }

  bool IsBinaryOperator( const FHlslToken* Token )
  {
    return GetPrecedence( Token ) > 0;
  }

  bool IsAssignmentOperator( const FHlslToken* Token )
  {
    if ( Token )
    {
      switch ( Token->Token )
      {
      case EHlslToken::Equal:
      case EHlslToken::PlusEqual:
      case EHlslToken::MinusEqual:
      case EHlslToken::TimesEqual:
      case EHlslToken::DivEqual:
      case EHlslToken::ModEqual:
      case EHlslToken::GreaterGreaterEqual:
      case EHlslToken::LowerLowerEqual:
      case EHlslToken::AndEqual:
      case EHlslToken::OrEqual:
      case EHlslToken::XorEqual:
        return true;

      default:
        break;
      }
    }

    return false;
  }

  static inline bool IsSequenceOperator( const FHlslToken* Token )
  {
    if ( Token )
    {
      return ( Token->Token == EHlslToken::Comma );
    }
    return false;
  }

  bool IsTernaryOperator( const FHlslToken* Token )
  {
    return ( Token && Token->Token == EHlslToken::Question );
  }

  bool IsRightAssociative( const FHlslToken* Token )
  {
    return IsTernaryOperator( Token );
  }

  EParseResult ComputeExpr( FHlslScanner& Scanner, TS32 MinPrec, /*FInfo& Info,*/ FSymbolScope* SymbolScope, bool bAllowAssignment, AST::FExpression** OutExpression, AST::FExpression** OutTernaryExpression )
  {
    auto OriginalToken = Scanner.GetCurrentTokenIndex();
    //FInfoIndentScope Scope(Info);
    /*
    // Precedence Climbing
    // http://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
    compute_expr(min_prec):
    result = compute_atom()

    while cur token is a binary operator with precedence >= min_prec:
    prec, assoc = precedence and associativity of current token
    if assoc is left:
    next_min_prec = prec + 1
    else:
    next_min_prec = prec
    rhs = compute_expr(next_min_prec)
    result = compute operator(result, rhs)

    return result
    */
    //Info.PrintWithTabs(CString::Printf(_T("Compute Expr %d\n"), MinPrec));
    AST::FExpression* TernaryExpression=nullptr;
    auto Result = ComputeAtom( Scanner, /*Info,*/ SymbolScope, bAllowAssignment, OutExpression, &TernaryExpression );
    if ( Result != EParseResult::Matched )
    {
      return Result;
    }
    BASEASSERT( *OutExpression );
    do
    {
      auto* Token = Scanner.GetCurrentToken();

      //LOG_NFO( "ComputeExpr: *** %s *** - isBinary: %d", GetTokenName( Token->Token ).GetPointer(), IsBinaryOperator( Token ) );

      TS32 Precedence = GetPrecedence( Token );
      if ( !Token || !IsBinaryOperator( Token ) || Precedence < MinPrec || ( !bAllowAssignment && IsAssignmentOperator( Token ) ) || IsSequenceOperator( Token ) || ( OutTernaryExpression && *OutTernaryExpression ) )
      {
        break;
      }

      Scanner.Advance();

      auto NextMinPrec = IsRightAssociative( Token ) ? Precedence : Precedence + 1;
      AST::FExpression* RHSExpression = nullptr;
      AST::FExpression* RHSTernaryExpression = nullptr;
      Result = ComputeExpr( Scanner, NextMinPrec, /*Info,*/ SymbolScope, bAllowAssignment, &RHSExpression, &RHSTernaryExpression );
      if ( Result == EParseResult::Error )
      {
        return EParseResult::Error;
      }
      else if ( Result == EParseResult::NotMatched )
      {
        break;
      }
      BASEASSERT( RHSExpression );
      auto BinaryOperator = AST::TokenToASTOperator( Token->Token );
      *OutExpression = new AST::FBinaryExpression( BinaryOperator, *OutExpression, RHSExpression, Token->SourceInfo );
      if ( RHSTernaryExpression )
      {
        BASEASSERT( !TernaryExpression );
        TernaryExpression = RHSTernaryExpression;
        break;
      }

/*
      {
        TBOOL LeftAssoc = AST::GetOperatorAssociativity( AST::TokenToASTOperator( Token->Token ) );

        auto NextMinPrec = Precedence + 1;
        if ( !LeftAssoc ) NextMinPrec = Precedence;

        AST::FExpression* RHSExpression = nullptr;
        Result = ComputeExpr( Scanner, NextMinPrec, / *Info,* / SymbolScope, bAllowAssignment, &RHSExpression );
        if ( Result == EParseResult::Error )
        {
          return EParseResult::Error;
        }
        else if ( Result == EParseResult::NotMatched )
        {
          break;
        }
        BASEASSERT( RHSExpression );
        auto BinaryOperator = AST::TokenToASTOperator( Token->Token );
        *OutExpression = new AST::FBinaryExpression( BinaryOperator, *OutExpression, RHSExpression, Token->SourceInfo );
      }
*/
    } while ( Scanner.HasMoreTokens() );

    if ( OriginalToken == Scanner.GetCurrentTokenIndex() )
    {
      return EParseResult::NotMatched;
    }

    if ( TernaryExpression )
    {
      if ( !OutTernaryExpression )
      {
        if ( !TernaryExpression->SubExpressions[ 0 ] )
        {
          TernaryExpression->SubExpressions[ 0 ] = *OutExpression;
          *OutExpression = TernaryExpression;
        }
        else
        {
          BASEASSERT( 0 );
        }
      }
      else
      {
        *OutTernaryExpression = TernaryExpression;
      }
    }


    return EParseResult::Matched;
  }

  EParseResult ParseExpression( FHlslScanner& Scanner, FSymbolScope* SymbolScope, bool bAllowAssignment, AST::FExpression** OutExpression )
  {
    /*FInfo Info(!true);*/
    return ComputeExpr( Scanner, 0, /*Info,*/ SymbolScope, bAllowAssignment, OutExpression, nullptr);
  }

  EParseResult ParseExpressionList( EHlslToken EndListToken, FHlslScanner& Scanner, FSymbolScope* SymbolScope, EHlslToken NewStartListToken, AST::FExpression* OutExpression )
  {
    BASEASSERT( OutExpression );
    while ( Scanner.HasMoreTokens() )
    {
      const auto* Token = Scanner.PeekToken();
      if ( Token->Token == EndListToken )
      {
        Scanner.Advance();
        return EParseResult::Matched;
      }
      else if ( NewStartListToken != EHlslToken::Invalid && Token->Token == NewStartListToken )
      {
        Scanner.Advance();
        auto* SubExpression = new AST::FInitializerListExpression( Token->SourceInfo );
        auto Result = ParseExpressionList( EndListToken, Scanner, SymbolScope, NewStartListToken, SubExpression );
        if ( Result != EParseResult::Matched )
        {
          return Result;
        }

        OutExpression->Expressions.Add( SubExpression );
      }
      else
      {
        AST::FExpression* Expression = nullptr;
        auto Result = ParseExpression( Scanner, SymbolScope, true, &Expression );
        if ( Result == EParseResult::Error )
        {
          Scanner.SourceError( _T( "Invalid expression list\n" ) );
          return EParseResult::Error;
        }
        else if ( Result == EParseResult::NotMatched )
        {
          Scanner.SourceError( _T( "Expected expression\n" ) );
          return EParseResult::Error;
        }

        OutExpression->Expressions.Add( Expression );
      }

      if ( Scanner.MatchToken( EHlslToken::Comma ) )
      {
        continue;
      }
      else if ( Scanner.MatchToken( EndListToken ) )
      {
        return EParseResult::Matched;
      }

      Scanner.SourceError( _T( "Expected ','\n" ) );
      break;
    }

    return EParseResult::Error;
  }
}
