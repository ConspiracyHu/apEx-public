#include "BasePCH.h"
#include "HLSLParser.h"
#include "HLSLExpressionParser.inl"

//////////////////////////////////////////////////////////////////////////
// HLSL Processing lifted from from Unreal Engine 4 v4.7 (c) Epic Games

extern CStringArray macroNames;

namespace CrossCompiler
{
  EParseResult ParseExpressionStatement( class FHlslParser& Parser, AST::FNode** OutStatement );
  EParseResult ParseStructBody( FHlslScanner& Scanner, FSymbolScope* SymbolScope, AST::FTypeSpecifier** OutTypeSpecifier );
  EParseResult TryParseAttribute( FHlslParser& Parser, AST::FAttribute** OutAttribute );

  typedef EParseResult( *TTryRule )( class FHlslParser& Scanner, AST::FNode** OutStatement );
  struct FRulePair
  {
    EHlslToken Token;
    TTryRule TryRule;
    bool bSupportsAttributes;

    FRulePair() { Token = EHlslToken::Invalid; }
    FRulePair( EHlslToken InToken, TTryRule InTryRule, bool bInSupportsAttributes = false ) : Token( InToken ), TryRule( InTryRule ), bSupportsAttributes( bInSupportsAttributes ) {}
  };
  typedef CArray<FRulePair> TRulesArray;

  class FHlslParser
  {
  public:
    FHlslParser();
    FHlslScanner Scanner;
    FSymbolScope GlobalScope;
    FSymbolScope* CurrentScope;
  };

  TRulesArray RulesTranslationUnit;
  TRulesArray RulesStatements;

  EParseResult TryRules( const TRulesArray& Rules, FHlslParser& Parser, const CString& RuleNames, bool bErrorIfNoMatch, AST::FNode** OutNode )
  {
    for ( TS32 x = 0; x < Rules.NumItems(); x++ )
    {
      const auto &Rule = Rules[ x ];
      auto CurrentTokenIndex = Parser.Scanner.GetCurrentTokenIndex();
      CArray<AST::FAttribute*> Attributes;
      if ( Rule.bSupportsAttributes )
      {
        while ( Parser.Scanner.HasMoreTokens() )
        {
          const auto* Peek = Parser.Scanner.GetCurrentToken();
          if ( Peek->Token == EHlslToken::LeftSquareBracket )
          {
            AST::FAttribute* Attribute = nullptr;
            auto Result = TryParseAttribute( Parser, &Attribute );
            if ( Result == EParseResult::Matched )
            {
              Attributes.Add( Attribute );
              continue;
            }
            else if ( Result == EParseResult::Error )
            {
              return EParseResult::Error;
            }
          }

          break;
        }
      }

      if ( Parser.Scanner.MatchToken( Rule.Token ) || Rule.Token == EHlslToken::Invalid )
      {
        AST::FNode* Node = nullptr;
        EParseResult Result = ( *Rule.TryRule )( Parser, &Node );
        if ( Result == EParseResult::Error )
        {
          return EParseResult::Error;
        }
        else if ( Result == EParseResult::Matched )
        {
          if ( Attributes.NumItems() > 0 )
          {
            //Swap(Node->Attributes, Attributes);
            //////////////////////////////////////////////////////////////////////////
            // THIS BELOW NEEDS TO BE CHECKED!

            CArray<CrossCompiler::AST::FAttribute*> AttribTemp = Node->Attributes;
            Node->Attributes = Attributes;
            Attributes = AttribTemp;
          }
          *OutNode = Node;
          return EParseResult::Matched;
        }
      }

      Parser.Scanner.SetCurrentTokenIndex( CurrentTokenIndex );
    }

    if ( bErrorIfNoMatch )
    {
      Parser.Scanner.SourceError( CString::Format( _T( "No matching %s rules found!" ), RuleNames.GetPointer() ) );
      return EParseResult::Error;
    }

    return EParseResult::NotMatched;
  }

  EParseResult ParseDeclarationArrayBracketsAndIndex( FHlslScanner& Scanner, FSymbolScope* SymbolScope, bool bNeedsDimension, AST::FExpression** OutExpression )
  {
    if ( Scanner.MatchToken( EHlslToken::LeftSquareBracket ) )
    {
      auto ExpressionResult = ParseExpression( Scanner, SymbolScope, false, OutExpression );
      if ( ExpressionResult == EParseResult::Error )
      {
        Scanner.SourceError( _T( "Expected expression!" ) );
        return EParseResult::Error;
      }

      if ( !Scanner.MatchToken( EHlslToken::RightSquareBracket ) )
      {
        Scanner.SourceError( _T( "Expected ']'!" ) );
        return EParseResult::Error;
      }

      if ( ExpressionResult == EParseResult::NotMatched )
      {
        if ( bNeedsDimension )
        {
          Scanner.SourceError( _T( "Expected array dimension!" ) );
          return EParseResult::Error;
        }
      }

      return EParseResult::Matched;
    }

    return EParseResult::NotMatched;
  }

  EParseResult ParseDeclarationMultiArrayBracketsAndIndex( FHlslScanner& Scanner, FSymbolScope* SymbolScope, bool bNeedsDimension, AST::FDeclaration* Declaration )
  {
    bool bFoundOne = false;
    do
    {
      AST::FExpression* Dimension = nullptr;
      auto Result = ParseDeclarationArrayBracketsAndIndex( Scanner, SymbolScope, bNeedsDimension, &Dimension );
      if ( Result == EParseResult::Error )
      {
        return EParseResult::Error;
      }
      else if ( Result == EParseResult::NotMatched )
      {
        break;
      }

      Declaration->ArraySize.Add( Dimension );

      bFoundOne = true;
    } while ( Scanner.HasMoreTokens() );

    if ( bFoundOne )
    {
      Declaration->bIsArray = true;
      return EParseResult::Matched;
    }

    return EParseResult::NotMatched;
  }

  EParseResult ParseTextureOrBufferSimpleDeclaration( FHlslScanner& Scanner, FSymbolScope* SymbolScope, bool bMultiple, AST::FDeclaratorList** OutDeclaratorList )
  {
    auto OriginalToken = Scanner.GetCurrentTokenIndex();
    const auto* Token = Scanner.GetCurrentToken();
    auto* FullType = ( *OutDeclaratorList )->Type;
    if ( ParseGeneralType( Scanner, ETF_SAMPLER_TEXTURE_BUFFER, nullptr, &FullType->Specifier ) == EParseResult::Matched )
    {
      if ( Scanner.MatchToken( EHlslToken::Lower ) )
      {
        AST::FTypeSpecifier* ElementTypeSpecifier = nullptr;
        auto Result = ParseGeneralType( Scanner, ETF_BUILTIN_NUMERIC | ETF_USER_TYPES, SymbolScope, &ElementTypeSpecifier );
        if ( Result != EParseResult::Matched )
        {
          Scanner.SourceError( _T( "Expected type!" ) );
          return EParseResult::Error;
        }

        FullType->Specifier->InnerType = ElementTypeSpecifier->TypeName;

        if ( Scanner.MatchToken( EHlslToken::Comma ) )
        {
          auto* Integer = Scanner.GetCurrentToken();
          if ( !Scanner.MatchToken( EHlslToken::UnsignedIntegerConstant ) )
          {
            Scanner.SourceError( _T( "Expected constant!" ) );
            return EParseResult::Error;
          }
          FullType->Specifier->TextureMSNumSamples = Integer->UnsignedInteger;
        }

        if ( !Scanner.MatchToken( EHlslToken::Greater ) )
        {
          Scanner.SourceError( _T( "Expected '>'!" ) );
          return EParseResult::Error;
        }
      }
      else
      {
        //TypeSpecifier->InnerName = "float4";
      }

      do
      {
        // Handle 'Sampler2D Sampler'
        AST::FTypeSpecifier* DummyTypeSpecifier = nullptr;
        const auto* IdentifierToken = Scanner.GetCurrentToken();
        AST::FDeclaration* Declaration = nullptr;
        if ( ParseGeneralType( Scanner, ETF_SAMPLER_TEXTURE_BUFFER, nullptr, &DummyTypeSpecifier ) == EParseResult::Matched )
        {
          Declaration = new AST::FDeclaration( DummyTypeSpecifier->SourceInfo );
          Declaration->Identifier = DummyTypeSpecifier->TypeName;
        }
        else if ( Scanner.MatchToken( EHlslToken::Identifier ) )
        {
          Declaration = new AST::FDeclaration( IdentifierToken->SourceInfo );
          Declaration->Identifier = IdentifierToken->String;
        }
        else
        {
          Scanner.SourceError( _T( "Expected Identifier!" ) );
          return EParseResult::Error;
        }

        if ( ParseDeclarationMultiArrayBracketsAndIndex( Scanner, SymbolScope, true, Declaration ) == EParseResult::Error )
        {
          return EParseResult::Error;
        }

        //////////////////////////////////////////////////////////////////////////
        // parse register semantics - extended by BoyC

        if ( Scanner.MatchToken( EHlslToken::Colon ) )
        {
          if ( !Scanner.MatchToken( EHlslToken::Register ) )
          {
            Scanner.SourceError( _T( "Expected 'register'!" ) );
            return EParseResult::Error;
          }

          if ( !Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
          {
            Scanner.SourceError( _T( "Expected '('!" ) );
            return EParseResult::Error;
          }

          const auto* Register = Scanner.GetCurrentToken();
          if ( !Scanner.MatchToken( EHlslToken::Identifier ) )
          {
            Scanner.SourceError( _T( "Expected identifier for register()!\n" ) );
            return EParseResult::Error;
          }
          else Declaration->Register = Register->String;

          if ( !Scanner.MatchToken( EHlslToken::RightParenthesis ) )
          {
            Scanner.SourceError( _T( "Expected ')'!" ) );
            return EParseResult::Error;
          }
        }

        //////////////////////////////////////////////////////////////////////////

        ( *OutDeclaratorList )->Declarations.Add( Declaration );
      } while ( bMultiple && Scanner.MatchToken( EHlslToken::Comma ) );

      return EParseResult::Matched;
    }

    // Unmatched
    Scanner.SetCurrentTokenIndex( OriginalToken );
    return EParseResult::NotMatched;
  }

  // Multi declaration parser flags
  enum EDeclarationFlags
  {
    //EDF_ROLLBACK_IF_NO_MATCH		= 0x0001,
    EDF_CONST_ROW_MAJOR = 0x0002,
    EDF_STATIC = 0x0004,
    EDF_TEXTURE_SAMPLER_OR_BUFFER = 0x0008,
    EDF_INITIALIZER = 0x0010,
    EDF_INITIALIZER_LIST = 0x0020 | EDF_INITIALIZER,
    EDF_SEMANTIC = 0x0040,
    EDF_SEMICOLON = 0x0080,
    EDF_IN_OUT = 0x0100,
    EDF_MULTIPLE = 0x0200,
    EDF_PRIMITIVE_DATA_TYPE = 0x0400,
    EDF_SHARED = 0x0800,
    EDF_NOINTERPOLATION = 0x1000,
    EDF_REGISTERSEMANTIC = 0x2000,
  };

  EParseResult ParseInitializer( FHlslScanner& Scanner, FSymbolScope* SymbolScope, bool bAllowLists, AST::FExpression** OutList )
  {
    if ( bAllowLists && Scanner.MatchToken( EHlslToken::LeftBrace ) )
    {
      *OutList = new AST::FInitializerListExpression( Scanner.GetCurrentToken()->SourceInfo );
      auto Result = ParseExpressionList( EHlslToken::RightBrace, Scanner, SymbolScope, EHlslToken::LeftBrace, *OutList );
      if ( Result != EParseResult::Matched )
      {
        Scanner.SourceError( _T( "Invalid initializer list\n" ) );
      }

      return EParseResult::Matched;
    }
    else
    {
      //@todo-rco?
      auto Result = ParseExpression( Scanner, SymbolScope, true, OutList );
      if ( Result == EParseResult::Error )
      {
        Scanner.SourceError( _T( "Invalid initializer expression\n" ) );
      }

      return Result;
    }

    return EParseResult::NotMatched;
  }

  EParseResult ParseDeclarationStorageQualifiers( FHlslScanner& Scanner, TS32 Flags, bool& bOutPrimitiveFound, AST::FTypeQualifier* Qualifier, CString &Primitive )
  {
    bOutPrimitiveFound = false;
    TS32 StaticFound = 0;
    TS32 NoInterpolationFound = 0;
    TS32 SharedFound = 0;
    TS32 ConstFound = 0;
    TS32 RowMajorFound = 0;
    TS32 InFound = 0;
    TS32 OutFound = 0;
    TS32 InOutFound = 0;
    TS32 PrimitiveFound = 0;

    if ( Flags & EDF_PRIMITIVE_DATA_TYPE )
    {
      const auto* Token = Scanner.GetCurrentToken();
      if ( Token && Token->Token == EHlslToken::Identifier )
      {
        if ( Token->String == _T( "point" ) ||
             Token->String == _T( "line" ) ||
             Token->String == _T( "triangle" ) ||
             Token->String == _T( "lineadj" ) ||
             Token->String == _T( "triangleadj" ) )
        {
          Primitive = Token->String;
          Scanner.Advance();
          ++PrimitiveFound;
        }
      }
    }

    while ( Scanner.HasMoreTokens() )
    {
      bool bFound = false;
      if ( ( Flags & EDF_STATIC ) && Scanner.MatchToken( EHlslToken::Static ) )
      {
        ++StaticFound;
        Qualifier->bIsStatic = true;
        if ( StaticFound > 1 )
        {
          Scanner.SourceError( _T( "'static' found more than once!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_NOINTERPOLATION ) && Scanner.MatchToken( EHlslToken::NoInterpolation ) )
      {
        ++NoInterpolationFound;
        if ( NoInterpolationFound > 1 )
        {
          Scanner.SourceError( _T( "'nointerpolation' found more than once!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_SHARED ) && Scanner.MatchToken( EHlslToken::GroupShared ) )
      {
        ++SharedFound;
        Qualifier->bShared = true;
        if ( SharedFound > 1 )
        {
          Scanner.SourceError( _T( "'groupshared' found more than once!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_CONST_ROW_MAJOR ) && Scanner.MatchToken( EHlslToken::Const ) )
      {
        ++ConstFound;
        Qualifier->bConstant = true;
        if ( ConstFound > 1 )
        {
          Scanner.SourceError( _T( "'const' found more than once!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_CONST_ROW_MAJOR ) && Scanner.MatchToken( EHlslToken::RowMajor ) )
      {
        ++RowMajorFound;
        Qualifier->bRowMajor = true;
        if ( RowMajorFound > 1 )
        {
          Scanner.SourceError( _T( "'row_major' found more than once!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_IN_OUT ) && Scanner.MatchToken( EHlslToken::In ) )
      {
        ++InFound;
        Qualifier->bIn = true;
        if ( InFound > 1 )
        {
          Scanner.SourceError( _T( "'in' found more than once!\n" ) );
          return EParseResult::Error;
        }
        else if ( InOutFound > 0 )
        {
          Scanner.SourceError( _T( "'in' can't be used with 'inout'!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_IN_OUT ) && Scanner.MatchToken( EHlslToken::Out ) )
      {
        ++OutFound;
        Qualifier->bOut = true;
        if ( OutFound > 1 )
        {
          Scanner.SourceError( _T( "'out' found more than once!\n" ) );
          return EParseResult::Error;
        }
        else if ( InOutFound > 0 )
        {
          Scanner.SourceError( _T( "'out' can't be used with 'inout'!\n" ) );
          return EParseResult::Error;
        }
      }
      else if ( ( Flags & EDF_IN_OUT ) && Scanner.MatchToken( EHlslToken::InOut ) )
      {
        ++InOutFound;
        Qualifier->bIn = true;
        Qualifier->bOut = true;
        if ( InOutFound > 1 )
        {
          Scanner.SourceError( _T( "'inout' found more than once!\n" ) );
          return EParseResult::Error;
        }
        else if ( InFound > 0 || OutFound > 0 )
        {
          Scanner.SourceError( _T( "'inout' can't be used with 'in' or 'out'!\n" ) );
          return EParseResult::Error;
        }
      }
      else
      {
        break;
      }
    }

    bOutPrimitiveFound = ( PrimitiveFound > 0 );

    return ( ConstFound + RowMajorFound + InFound + OutFound + InOutFound + StaticFound + SharedFound + PrimitiveFound + NoInterpolationFound )
      ? EParseResult::Matched
      : EParseResult::NotMatched;
  }

  EParseResult ParseGeneralDeclarationNoSemicolon( FHlslScanner& Scanner, FSymbolScope* SymbolScope, TS32 Flags, AST::FDeclaratorList** OutDeclaratorList )
  {
    auto OriginalToken = Scanner.GetCurrentTokenIndex();
    bool bPrimitiveFound = false;
    auto* FullType = new AST::FFullySpecifiedType( Scanner.GetCurrentToken()->SourceInfo );

    CString Primitive;
    auto Result = ParseDeclarationStorageQualifiers( Scanner, Flags, bPrimitiveFound, &FullType->Qualifier, Primitive );
    if ( Result == EParseResult::Error )
    {
      return EParseResult::Error;
    }
    bool bCanBeUnmatched = ( Result == EParseResult::NotMatched );

    auto* DeclaratorList = new AST::FDeclaratorList( FullType->SourceInfo );
    DeclaratorList->Type = FullType;

    if ( bPrimitiveFound && ( Flags & EDF_PRIMITIVE_DATA_TYPE ) )
    {
      FullType->Primitive = Primitive;
    }

    if ( !bPrimitiveFound && ( Flags & EDF_PRIMITIVE_DATA_TYPE ) )
    {
      const auto* StreamToken = Scanner.GetCurrentToken();
      if ( StreamToken && StreamToken->Token == EHlslToken::Identifier )
      {
        if ( StreamToken->String == _T( "PointStream" ) ||
             StreamToken->String == _T( "LineStream" ) ||
             StreamToken->String == _T( "TriangleStream" ) )
        {
          Scanner.Advance();
          bCanBeUnmatched = false;

          if ( !Scanner.MatchToken( EHlslToken::Lower ) )
          {
            Scanner.SourceError( _T( "Expected '<'!" ) );
            return EParseResult::Error;
          }

          AST::FTypeSpecifier* TypeSpecifier = nullptr;
          if ( ParseGeneralType( Scanner, ETF_BUILTIN_NUMERIC | ETF_USER_TYPES, SymbolScope, &TypeSpecifier ) != EParseResult::Matched )
          {
            Scanner.SourceError( _T( "Expected type!" ) );
            return EParseResult::Error;
          }

          if ( !Scanner.MatchToken( EHlslToken::Greater ) )
          {
            Scanner.SourceError( _T( "Expected '>'!" ) );
            return EParseResult::Error;
          }

          auto* IdentifierToken = Scanner.GetCurrentToken();
          if ( !Scanner.MatchToken( EHlslToken::Identifier ) )
          {
            Scanner.SourceError( _T( "Expected identifier!" ) );
            return EParseResult::Error;
          }

          TypeSpecifier->InnerType = TypeSpecifier->TypeName;
          TypeSpecifier->TypeName = StreamToken->String;
          FullType->Specifier = TypeSpecifier;

          auto* Declaration = new AST::FDeclaration( IdentifierToken->SourceInfo );
          Declaration->Identifier = IdentifierToken->String;

          DeclaratorList->Declarations.Add( Declaration );
          *OutDeclaratorList = DeclaratorList;
          return EParseResult::Matched;
        }
      }
    }

    if ( Flags & EDF_TEXTURE_SAMPLER_OR_BUFFER )
    {
      auto Result = ParseTextureOrBufferSimpleDeclaration( Scanner, SymbolScope, ( Flags & EDF_MULTIPLE ) == EDF_MULTIPLE, &DeclaratorList );
      if ( Result == EParseResult::Matched )
      {
        *OutDeclaratorList = DeclaratorList;
        return EParseResult::Matched;
      }
      else if ( Result == EParseResult::Error )
      {
        return EParseResult::Error;
      }
    }

    const bool bAllowInitializerList = ( Flags & EDF_INITIALIZER_LIST ) == EDF_INITIALIZER_LIST;

    if ( Scanner.MatchToken( EHlslToken::Struct ) )
    {
      auto Result = ParseStructBody( Scanner, SymbolScope, &FullType->Specifier );
      if ( Result != EParseResult::Matched )
      {
        return EParseResult::Error;
      }

      do
      {
        auto* IdentifierToken = Scanner.GetCurrentToken();
        if ( Scanner.MatchToken( EHlslToken::Identifier ) )
        {
          //... Instance

          auto* Declaration = new AST::FDeclaration( IdentifierToken->SourceInfo );
          Declaration->Identifier = IdentifierToken->String;

          if ( ParseDeclarationMultiArrayBracketsAndIndex( Scanner, SymbolScope, false, Declaration ) == EParseResult::Error )
          {
            return EParseResult::Error;
          }

          if ( Flags & EDF_INITIALIZER )
          {
            if ( Scanner.MatchToken( EHlslToken::Equal ) )
            {
              if ( ParseInitializer( Scanner, SymbolScope, bAllowInitializerList, &Declaration->Initializer ) != EParseResult::Matched )
              {
                Scanner.SourceError( _T( "Invalid initializer\n" ) );
                return EParseResult::Error;
              }
            }
          }

          DeclaratorList->Declarations.Add( Declaration );
        }
      } while ( ( Flags & EDF_MULTIPLE ) == EDF_MULTIPLE && Scanner.MatchToken( EHlslToken::Comma ) );
      *OutDeclaratorList = DeclaratorList;
    }
    else
    {
      auto Result = ParseGeneralType( Scanner, ETF_BUILTIN_NUMERIC | ETF_USER_TYPES, SymbolScope, &FullType->Specifier );
      if ( Result == EParseResult::Matched )
      {
        bool bMatched = false;
        do
        {
          auto* IdentifierToken = Scanner.GetCurrentToken();
          if ( !Scanner.MatchToken( EHlslToken::Identifier ) )
          {
            Scanner.SetCurrentTokenIndex( OriginalToken );
            SAFEDELETE( DeclaratorList );
            return EParseResult::NotMatched;
          }

          auto* Declaration = new AST::FDeclaration( IdentifierToken->SourceInfo );
          Declaration->Identifier = IdentifierToken->String;
          //AddVar

          if ( ParseDeclarationMultiArrayBracketsAndIndex( Scanner, SymbolScope, false, Declaration ) == EParseResult::Error )
          {
            return EParseResult::Error;
          }

          bool bSemanticFound = false;

          //////////////////////////////////////////////////////////////////////////
          // expanded to handle register() semantics by BoyC

          if ( ( ( Flags & EDF_SEMANTIC ) || ( Flags & EDF_REGISTERSEMANTIC ) ) && Scanner.MatchToken( EHlslToken::Colon ) )
          {
            auto* Semantic = Scanner.GetCurrentToken();
            if ( ( Flags & EDF_SEMANTIC ) && Scanner.MatchToken( EHlslToken::Identifier ) )
            {
              Declaration->Semantic = Semantic->String;
            }
            else
              if ( ( Flags & EDF_REGISTERSEMANTIC ) && Scanner.MatchToken( EHlslToken::Register ) )
              {
                if ( !Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
                {
                  Scanner.SourceError( _T( "Expected '('!" ) );
                  return EParseResult::Error;
                }
                auto* Register = Scanner.GetCurrentToken();
                if ( !Scanner.MatchToken( EHlslToken::Identifier ) )
                {
                  Scanner.SourceError( _T( "Expected identifier for register!" ) );
                  return EParseResult::Error;
                }

                Declaration->Register = Register->String;

                if ( !Scanner.MatchToken( EHlslToken::RightParenthesis ) )
                {
                  Scanner.SourceError( _T( "Expected ')'!" ) );
                  return EParseResult::Error;
                }
              }
              else
              {
                Scanner.SourceError( _T( "Expected identifier or register for semantic!" ) );
                return EParseResult::Error;
              }

            bSemanticFound = true;
          }

          //////////////////////////////////////////////////////////////////////////

          if ( ( Flags & EDF_INITIALIZER ) && !bSemanticFound )
          {
            if ( Scanner.MatchToken( EHlslToken::Equal ) )
            {
              if ( ParseInitializer( Scanner, SymbolScope, bAllowInitializerList, &Declaration->Initializer ) != EParseResult::Matched )
              {
                Scanner.SourceError( _T( "Invalid initializer\n" ) );
                return EParseResult::Error;
              }
            }
          }

          DeclaratorList->Declarations.Add( Declaration );
        } while ( ( Flags & EDF_MULTIPLE ) == EDF_MULTIPLE && Scanner.MatchToken( EHlslToken::Comma ) );

        *OutDeclaratorList = DeclaratorList;
      }
      else if ( bCanBeUnmatched && Result == EParseResult::NotMatched )
      {
        Scanner.SetCurrentTokenIndex( OriginalToken );
        SAFEDELETE( DeclaratorList );
        return EParseResult::NotMatched;
      }
    }

    return EParseResult::Matched;
  }

  EParseResult ParseGeneralDeclaration( FHlslScanner& Scanner, FSymbolScope* SymbolScope, AST::FDeclaratorList** OutDeclaration, TS32 Flags )
  {
    auto OriginalToken = Scanner.GetCurrentTokenIndex();

    auto Result = ParseGeneralDeclarationNoSemicolon( Scanner, SymbolScope, Flags, OutDeclaration );
    if ( Result == EParseResult::NotMatched || Result == EParseResult::Error )
    {
      return Result;
    }

    if ( Flags & EDF_SEMICOLON )
    {
      if ( !Scanner.MatchToken( EHlslToken::Semicolon ) )
      {
        Scanner.SourceError( _T( "';' expected!\n" ) );
        return EParseResult::Error;
      }
    }

    return EParseResult::Matched;
  }

  EParseResult ParseCBuffer( FHlslParser& Parser, AST::FNode** OutDeclaration )
  {
    const auto* Token = Parser.Scanner.GetCurrentToken();
    if ( !Token )
    {
      Parser.Scanner.SourceError( _T( "Expected '{'!" ) );
      return EParseResult::Error;
    }

    auto* CBuffer = new AST::FCBufferDeclaration( Token->SourceInfo );
    if ( Parser.Scanner.MatchToken( EHlslToken::Identifier ) )
    {
      CBuffer->Name = Token->String;
    }

    //////////////////////////////////////////////////////////////////////////
    // read register semantic - addition by BoyC

    if ( Parser.Scanner.MatchToken( EHlslToken::Colon ) )
    {
      if ( !Parser.Scanner.MatchToken( EHlslToken::Register ) )
      {
        Parser.Scanner.SourceError( _T( "Expected 'register'!" ) );
        return EParseResult::Error;
      }

      if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
      {
        Parser.Scanner.SourceError( _T( "Expected '('!" ) );
        return EParseResult::Error;
      }

      const auto* Register = Parser.Scanner.GetCurrentToken();
      if ( !Parser.Scanner.MatchToken( EHlslToken::Identifier ) )
      {
        Parser.Scanner.SourceError( _T( "Expected identifier for register()!\n" ) );
        return EParseResult::Error;
      }
      else CBuffer->Register = Register->String;

      if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
      {
        Parser.Scanner.SourceError( _T( "Expected ')'!" ) );
        return EParseResult::Error;
      }
    }

    //////////////////////////////////////////////////////////////////////////

    bool bFoundRightBrace = false;
    if ( Parser.Scanner.MatchToken( EHlslToken::LeftBrace ) )
    {
      while ( Parser.Scanner.HasMoreTokens() )
      {
        if ( Parser.Scanner.MatchToken( EHlslToken::RightBrace ) )
        {
          if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
          {
            // Optional???
          }

          *OutDeclaration = CBuffer;
          return EParseResult::Matched;
        }

        AST::FDeclaratorList* Declaration = nullptr;

        //////////////////////////////////////////////////////////////////////////
        // EDF_MULTIPLE added below by BoyC

        auto Result = ParseGeneralDeclaration( Parser.Scanner, Parser.CurrentScope, &Declaration, EDF_CONST_ROW_MAJOR | EDF_SEMICOLON | EDF_TEXTURE_SAMPLER_OR_BUFFER | EDF_MULTIPLE );
        if ( Result == EParseResult::Error )
        {
          return EParseResult::Error;
        }
        else if ( Result == EParseResult::NotMatched )
        {
          break;
        }
        CBuffer->Declarations.Add( Declaration );
      }
    }

    Parser.Scanner.SourceError( _T( "Expected '}'!" ) );
    return EParseResult::Error;
  }

  EParseResult ParseMacro( FHlslParser& Parser, AST::FNode** OutDeclaration )
  {
    Parser.Scanner.SetCurrentTokenIndex( Parser.Scanner.GetCurrentTokenIndex() - 1 );
    auto *Token = Parser.Scanner.PeekToken( 0 );
    Parser.Scanner.Advance();
    *OutDeclaration = new AST::FMacro( Token->SourceInfo, Token->String );
    return EParseResult::Matched;
  }

  EParseResult ParseStructBody( FHlslScanner& Scanner, FSymbolScope* SymbolScope, AST::FTypeSpecifier** OutTypeSpecifier )
  {
    const auto* Name = Scanner.GetCurrentToken();
    if ( Name && Scanner.MatchToken( EHlslToken::Identifier ) )
    {
      SymbolScope->Add( Name->String );
    }

    CString Parent = "";
    if ( Scanner.MatchToken( EHlslToken::Colon ) )
    {
      const auto* ParentToken = Scanner.GetCurrentToken();
      if ( !Scanner.MatchToken( EHlslToken::Identifier ) )
      {
        Scanner.SourceError( _T( "Identifier expected!\n" ) );
        return EParseResult::Error;
      }

      Parent = ParentToken->String;
    }

    if ( !Scanner.MatchToken( EHlslToken::LeftBrace ) )
    {
      Scanner.SourceError( _T( "Expected '{'!" ) );
      return EParseResult::Error;
    }

    auto* Struct = new AST::FStructSpecifier( Name->SourceInfo );
    Struct->ParentName = Parent;
    Struct->Name = Name->String;

    bool bFoundRightBrace = false;
    while ( Scanner.HasMoreTokens() )
    {
      if ( Scanner.MatchToken( EHlslToken::RightBrace ) )
      {
        bFoundRightBrace = true;
        break;
      }

      AST::FDeclaratorList* Declaration = nullptr;
      auto Result = ParseGeneralDeclaration( Scanner, SymbolScope, &Declaration, EDF_CONST_ROW_MAJOR | EDF_SEMICOLON | EDF_SEMANTIC | EDF_TEXTURE_SAMPLER_OR_BUFFER | EDF_NOINTERPOLATION | EDF_MULTIPLE );
      if ( Result == EParseResult::Error )
      {
        return EParseResult::Error;
      }
      else if ( Result == EParseResult::NotMatched )
      {
        break;
      }
      Struct->Declarations.Add( Declaration );
    }

    if ( !bFoundRightBrace )
    {
      Scanner.SourceError( _T( "Expected '}'!" ) );
      return EParseResult::Error;
    }

    auto* TypeSpecifier = new AST::FTypeSpecifier( Struct->SourceInfo );
    TypeSpecifier->Structure = Struct;
    *OutTypeSpecifier = TypeSpecifier;
    return EParseResult::Matched;
  }

  EParseResult ParseFunctionParameterDeclaration( FHlslParser& Parser, AST::FFunction* Function )
  {
    bool bStrictCheck = false;

    while ( Parser.Scanner.HasMoreTokens() )
    {
      AST::FDeclaratorList* Declaration = nullptr;
      auto Result = ParseGeneralDeclaration( Parser.Scanner, Parser.CurrentScope, &Declaration, EDF_CONST_ROW_MAJOR | EDF_IN_OUT | EDF_TEXTURE_SAMPLER_OR_BUFFER | EDF_INITIALIZER | EDF_SEMANTIC | EDF_PRIMITIVE_DATA_TYPE | EDF_NOINTERPOLATION );
      if ( Result == EParseResult::Error )
      {
        return EParseResult::Error;
      }

      auto* Parameter = AST::FParameterDeclarator::CreateFromDeclaratorList( Declaration );
      Function->Parameters.Add( Parameter );
      if ( !Parser.Scanner.MatchToken( EHlslToken::Comma ) )
      {
        break;
      }
      else if ( Result == EParseResult::NotMatched )
      {
        Parser.Scanner.SourceError( _T( "Internal error on function parameter!\n" ) );
        return EParseResult::Error;
      }
    }

    return EParseResult::Matched;
  }

  EParseResult ParseFunctionDeclarator( FHlslParser& Parser, AST::FFunction** OutFunction )
  {
    auto OriginalToken = Parser.Scanner.GetCurrentTokenIndex();
    AST::FTypeSpecifier* TypeSpecifier = nullptr;
    auto Result = ParseGeneralType( Parser.Scanner, ETF_BUILTIN_NUMERIC | ETF_SAMPLER_TEXTURE_BUFFER | ETF_USER_TYPES | ETF_VOID, Parser.CurrentScope, &TypeSpecifier );
    if ( Result == EParseResult::NotMatched )
    {
      SAFEDELETE( TypeSpecifier );
      Parser.Scanner.SetCurrentTokenIndex( OriginalToken );
      return EParseResult::NotMatched;
    }

    BASEASSERT( Result == EParseResult::Matched );

    auto* Identifier = Parser.Scanner.GetCurrentToken();
    if ( !Parser.Scanner.MatchToken( EHlslToken::Identifier ) )
    {
      // This could be an error... But we should allow testing for a global variable before any rash decisions
      Parser.Scanner.SetCurrentTokenIndex( OriginalToken );
      SAFEDELETE( TypeSpecifier );
      return EParseResult::NotMatched;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
    {
      // This could be an error... But we should allow testing for a global variable before any rash decisions
      Parser.Scanner.SetCurrentTokenIndex( OriginalToken );
      SAFEDELETE( TypeSpecifier );
      return EParseResult::NotMatched;
    }

    auto* Function = new AST::FFunction( Identifier->SourceInfo );
    Function->Identifier = Identifier->String;
    Function->ReturnType = new AST::FFullySpecifiedType( TypeSpecifier->SourceInfo );
    Function->ReturnType->Specifier = TypeSpecifier;

    if ( Parser.Scanner.MatchToken( EHlslToken::Void ) )
    {
      // Nothing to do here...
    }
    else if ( Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      goto Done;
    }
    else
    {
      Result = ParseFunctionParameterDeclaration( Parser, Function );
      if ( Result == EParseResult::Error )
      {
        return EParseResult::Error;
      }
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "')' expected" ) );
      return EParseResult::Error;
    }

  Done:
    *OutFunction = Function;

    return EParseResult::Matched;
  }

  EParseResult ParseStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    const auto* Token = Parser.Scanner.PeekToken();
    if ( Token && Token->Token == EHlslToken::RightBrace )
    {
      return EParseResult::NotMatched;
    }

    static CString Statement( _T( "Statement" ) );
    return TryRules( RulesStatements, Parser, Statement, false, OutStatement );
  }

  EParseResult ParseStatementBlock( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    FCreateSymbolScope SymbolScope( &Parser.CurrentScope );
    auto* Block = new AST::FCompoundStatement( Parser.Scanner.GetCurrentToken()->SourceInfo );
    while ( Parser.Scanner.HasMoreTokens() )
    {
      AST::FNode* Statement = nullptr;
      auto Result = ParseStatement( Parser, &Statement );
      if ( Result == EParseResult::NotMatched )
      {
        if ( Parser.Scanner.MatchToken( EHlslToken::RightBrace ) )
        {
          *OutStatement = Block;
          return EParseResult::Matched;
        }
        else
        {
          Parser.Scanner.SourceError( _T( "Statement expected!" ) );
          break;
        }
      }
      else if ( Result == EParseResult::Error )
      {
        break;
      }

      if ( Statement )
      {
        Block->Statements.Add( Statement );
      }
    }

    Parser.Scanner.SourceError( _T( "'}' expected!" ) );
    return EParseResult::Error;
  }

  EParseResult ParseFunctionDeclaration( FHlslParser& Parser, AST::FNode** OutFunction )
  {
    const auto* CurrentToken = Parser.Scanner.GetCurrentToken();

    AST::FFunction* Function = nullptr;
    EParseResult Result = ParseFunctionDeclarator( Parser, &Function );
    if ( Result == EParseResult::NotMatched || Result == EParseResult::Error )
    {
      return Result;
    }

    if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      BASEASSERT( 0 );
      // Forward declare
      return EParseResult::Matched;
    }
    else
    {
      // Optional semantic
      if ( Parser.Scanner.MatchToken( EHlslToken::Colon ) )
      {
        const auto* ReturnSemantic = Parser.Scanner.GetCurrentToken();
        if ( !Parser.Scanner.MatchToken( EHlslToken::Identifier ) )
        {
          Parser.Scanner.SourceError( _T( "Identifier for semantic expected" ) );
          return EParseResult::Error;
        }
        else
        {
          Function->ReturnSemantic = ReturnSemantic->String;
        }
      }

      if ( !Parser.Scanner.MatchToken( EHlslToken::LeftBrace ) )
      {
        Parser.Scanner.SourceError( _T( "'{' expected" ) );
        return EParseResult::Error;
      }

      auto* FunctionDefinition = new AST::FFunctionDefinition( CurrentToken->SourceInfo );
      AST::FNode* Body = nullptr;
      Result = ParseStatementBlock( Parser, &Body );
      if ( Result == EParseResult::Matched )
      {
        FunctionDefinition->Body = ( AST::FCompoundStatement* )Body;
        FunctionDefinition->Prototype = Function;
        *OutFunction = FunctionDefinition;
      }
    }

    return Result;
  }

  EParseResult ParseLocalDeclaration( FHlslParser& Parser, AST::FNode** OutDeclaration )
  {
    AST::FDeclaratorList* List = nullptr;
    auto Result = ParseGeneralDeclaration( Parser.Scanner, Parser.CurrentScope, &List, EDF_CONST_ROW_MAJOR | EDF_INITIALIZER | EDF_INITIALIZER_LIST | EDF_SEMICOLON | EDF_MULTIPLE );
    *OutDeclaration = List;
    return Result;
  }

  EParseResult ParseGlobalVariableDeclaration( FHlslParser& Parser, AST::FNode** OutDeclaration )
  {
    AST::FDeclaratorList* List = nullptr;
    auto Result = ParseGeneralDeclaration( Parser.Scanner, Parser.CurrentScope, &List, EDF_CONST_ROW_MAJOR | EDF_STATIC | EDF_SHARED | EDF_TEXTURE_SAMPLER_OR_BUFFER | EDF_INITIALIZER | EDF_INITIALIZER_LIST | EDF_SEMICOLON | EDF_MULTIPLE | EDF_REGISTERSEMANTIC );
    *OutDeclaration = List;
    return Result;
  }

  EParseResult ParseReturnStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    auto* Statement = new AST::FJumpStatement( AST::EJumpType::Return, Parser.Scanner.GetCurrentToken()->SourceInfo );

    if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      *OutStatement = Statement;
      return EParseResult::Matched;
    }

    if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &Statement->OptionalExpression ) != EParseResult::Matched )
    {
      Parser.Scanner.SourceError( _T( "Expression expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      Parser.Scanner.SourceError( _T( "';' expected" ) );
      return EParseResult::Error;
    }

    *OutStatement = Statement;
    return EParseResult::Matched;
  }

  EParseResult ParseDoStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    FCreateSymbolScope SymbolScope( &Parser.CurrentScope );
    const auto* Token = Parser.Scanner.GetCurrentToken();
    AST::FNode* Body = nullptr;
    auto Result = ParseStatement( Parser, &Body );
    if ( Result != EParseResult::Matched )
    {
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::While ) )
    {
      Parser.Scanner.SourceError( _T( "'while' expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "'(' expected" ) );
      return EParseResult::Error;
    }

    AST::FExpression* ConditionExpression = nullptr;
    if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &ConditionExpression ) != EParseResult::Matched )
    {
      Parser.Scanner.SourceError( _T( "Expression expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "')' expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      Parser.Scanner.SourceError( _T( "';' expected" ) );
      return EParseResult::Error;
    }

    auto* DoWhile = new AST::FIterationStatement( Token->SourceInfo, AST::EIterationType::DoWhile );
    DoWhile->Condition = ConditionExpression;
    DoWhile->Body = Body;
    *OutStatement = DoWhile;
    return EParseResult::Matched;
  }

  EParseResult ParseWhileStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    FCreateSymbolScope SymbolScope( &Parser.CurrentScope );
    const auto* Token = Parser.Scanner.GetCurrentToken();
    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "'(' expected" ) );
      return EParseResult::Error;
    }

    AST::FExpression* ConditionExpression = nullptr;
    if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &ConditionExpression ) != EParseResult::Matched )
    {
      Parser.Scanner.SourceError( _T( "Expression expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "')' expected" ) );
      return EParseResult::Error;
    }

    AST::FNode* Body = nullptr;
    auto Result = ParseStatement( Parser, &Body );
    if ( Result != EParseResult::Matched )
    {
      return EParseResult::Error;
    }

    auto* While = new AST::FIterationStatement( Token->SourceInfo, AST::EIterationType::While );
    While->Condition = ConditionExpression;
    While->Body = Body;
    *OutStatement = While;
    return EParseResult::Matched;
  }

  EParseResult ParseForStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    FCreateSymbolScope SymbolScope( &Parser.CurrentScope );
    const auto* Token = Parser.Scanner.GetCurrentToken();
    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "Expected '('!\n" ) );
      return EParseResult::Error;
    }

    AST::FNode* InitExpression = nullptr;
    if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      // Do nothing...
    }
    else
    {
      auto Result = ParseLocalDeclaration( Parser, &InitExpression );
      if ( Result == EParseResult::Error )
      {
        Parser.Scanner.SourceError( _T( "Expected expression or declaration!\n" ) );
        return EParseResult::Error;
      }
      else if ( Result == EParseResult::NotMatched )
      {
        Result = ParseExpressionStatement( Parser, &InitExpression );
        if ( Result == EParseResult::Error )
        {
          Parser.Scanner.SourceError( _T( "Expected expression or declaration!\n" ) );
          return EParseResult::Error;
        }
      }
    }

    AST::FExpression* ConditionExpression = nullptr;
    auto Result = ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &ConditionExpression );
    if ( Result == EParseResult::Error )
    {
      Parser.Scanner.SourceError( _T( "Expected expression or declaration!\n" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      Parser.Scanner.SourceError( _T( "Expected ';'!\n" ) );
      return EParseResult::Error;
    }

    AST::FExpression* RestExpression = nullptr;
    Result = ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &RestExpression );
    if ( Result == EParseResult::Error )
    {
      Parser.Scanner.SourceError( _T( "Expected expression or declaration!\n" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "Expected ')'!\n" ) );
      return EParseResult::Error;
    }

    AST::FNode* Body = nullptr;
    Result = ParseStatement( Parser, &Body );
    if ( Result != EParseResult::Matched )
    {
      return EParseResult::Error;
    }

    auto* For = new AST::FIterationStatement( Token->SourceInfo, AST::EIterationType::For );
    For->InitStatement = InitExpression;
    For->Condition = ConditionExpression;
    For->RestExpression = RestExpression;
    For->Body = Body;
    *OutStatement = For;
    return EParseResult::Matched;
  }

  EParseResult ParseIfStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    FCreateSymbolScope SymbolScope( &Parser.CurrentScope );

    auto* Statement = new AST::FSelectionStatement( Parser.Scanner.GetCurrentToken()->SourceInfo );

    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "'(' expected" ) );
      return EParseResult::Error;
    }

    if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &Statement->Condition ) != EParseResult::Matched )
    {
      Parser.Scanner.SourceError( _T( "Expression expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "')' expected" ) );
      return EParseResult::Error;
    }

    if ( ParseStatement( Parser, &Statement->ThenStatement ) != EParseResult::Matched )
    {
      Parser.Scanner.SourceError( _T( "Statement expected" ) );
      return EParseResult::Error;
    }

    if ( Parser.Scanner.MatchToken( EHlslToken::Else ) )
    {
      if ( ParseStatement( Parser, &Statement->ElseStatement ) != EParseResult::Matched )
      {
        Parser.Scanner.SourceError( _T( "Statement expected" ) );
        return EParseResult::Error;
      }
    }

    *OutStatement = Statement;
    return EParseResult::Matched;
  }

  EParseResult ParseAttributeArgList( FHlslScanner& Scanner, FSymbolScope* SymbolScope, AST::FAttribute* OutAttribute )
  {
    while ( Scanner.HasMoreTokens() )
    {
      const auto* Token = Scanner.PeekToken();
      if ( Scanner.MatchToken( EHlslToken::RightParenthesis ) )
      {
        return EParseResult::Matched;
      }

      bool bMultiple = false;
      do
      {
        bMultiple = false;
        Token = Scanner.PeekToken();
        if ( Scanner.MatchToken( EHlslToken::StringConstant ) )
        {
          auto* Arg = new AST::FAttributeArgument( Token->SourceInfo );
          Arg->StringArgument = Token->String;
          OutAttribute->Arguments.Add( Arg );
        }
        else
        {
          AST::FExpression* Expression = nullptr;
          EParseResult Result = ParseExpression( Scanner, SymbolScope, false, &Expression );
          if ( Result != EParseResult::Matched )
          {
            Scanner.SourceError( _T( "Incorrect attribute expression!\n" ) );
            return EParseResult::Error;
          }
          auto* Arg = new AST::FAttributeArgument( Token->SourceInfo );
          Arg->ExpressionArgument = Expression;
          OutAttribute->Arguments.Add( Arg );
        }

        if ( Scanner.MatchToken( EHlslToken::Comma ) )
        {
          bMultiple = true;
        }
      } while ( bMultiple );
    }

    return EParseResult::Error;
  }

  EParseResult TryParseAttribute( FHlslParser& Parser, AST::FAttribute** OutAttribute )
  {
    const auto* Token = Parser.Scanner.GetCurrentToken();
    if ( Parser.Scanner.MatchToken( EHlslToken::LeftSquareBracket ) )
    {
      auto* Identifier = Parser.Scanner.GetCurrentToken();
      if ( !Parser.Scanner.MatchToken( EHlslToken::Identifier ) )
      {
        Parser.Scanner.SourceError( _T( "Incorrect attribute\n" ) );
        return EParseResult::Error;
      }

      //////////////////////////////////////////////////////////////////////////
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! THIS BELOW NEEDS TO BE CHECKED

      //auto* Attribute = new AST::FAttribute(Token->SourceInfo->Strdup(Identifier->String));
      auto* Attribute = new AST::FAttribute( Token->SourceInfo, Identifier->String );

      if ( Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
      {
        auto Result = ParseAttributeArgList( Parser.Scanner, Parser.CurrentScope, Attribute );
        if ( Result != EParseResult::Matched )
        {
          Parser.Scanner.SourceError( _T( "Incorrect attribute! Expected ')'.\n" ) );
          return EParseResult::Error;
        }
      }

      if ( !Parser.Scanner.MatchToken( EHlslToken::RightSquareBracket ) )
      {
        Parser.Scanner.SourceError( _T( "Incorrect attribute\n" ) );
        return EParseResult::Error;
      }

      *OutAttribute = Attribute;
      return EParseResult::Matched;
    }

    return EParseResult::NotMatched;
  }

  EParseResult ParseSwitchBody( FHlslParser& Parser, AST::FSwitchBody** OutBody )
  {
    const auto* Token = Parser.Scanner.GetCurrentToken();

    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftBrace ) )
    {
      Parser.Scanner.SourceError( _T( "'{' expected" ) );
      return EParseResult::Error;
    }

    auto* Body = new AST::FSwitchBody( Token->SourceInfo );

    // Empty switch
    if ( Parser.Scanner.MatchToken( EHlslToken::RightBrace ) )
    {
      *OutBody = Body;
      return EParseResult::Matched;
    }

    Body->CaseList = new AST::FCaseStatementList( Token->SourceInfo );

    bool bDefaultFound = false;
    while ( Parser.Scanner.HasMoreTokens() )
    {
      auto* Token = Parser.Scanner.GetCurrentToken();
      if ( Parser.Scanner.MatchToken( EHlslToken::RightBrace ) )
      {
        break;
      }

      auto* Labels = new AST::FCaseLabelList( Token->SourceInfo );
      auto* CaseStatement = new AST::FCaseStatement( Token->SourceInfo, Labels );

      // Case labels
      bool bLabelFound = false;
      do
      {
        bLabelFound = false;
        AST::FCaseLabel* Label = nullptr;
        Token = Parser.Scanner.GetCurrentToken();
        if ( Parser.Scanner.MatchToken( EHlslToken::Default ) )
        {
          if ( bDefaultFound )
          {
            Parser.Scanner.SourceError( _T( "'default' found twice on switch() statement!" ) );
            return EParseResult::Error;
          }

          if ( !Parser.Scanner.MatchToken( EHlslToken::Colon ) )
          {
            Parser.Scanner.SourceError( _T( "':' expected" ) );
            return EParseResult::Error;
          }

          Label = new AST::FCaseLabel( Token->SourceInfo, nullptr );
          bDefaultFound = true;
          bLabelFound = true;
        }
        else if ( Parser.Scanner.MatchToken( EHlslToken::Case ) )
        {
          AST::FExpression* CaseExpression = nullptr;
          if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &CaseExpression ) != EParseResult::Matched )
          {
            Parser.Scanner.SourceError( _T( "Expression expected on case label!" ) );
            return EParseResult::Error;
          }

          if ( !Parser.Scanner.MatchToken( EHlslToken::Colon ) )
          {
            Parser.Scanner.SourceError( _T( "':' expected" ) );
            return EParseResult::Error;
          }

          Label = new AST::FCaseLabel( Token->SourceInfo, CaseExpression );
          bLabelFound = true;
        }

        if ( Label )
        {
          CaseStatement->Labels->Labels.Add( Label );
        }
      } while ( bLabelFound );

      // Statements
      Token = Parser.Scanner.GetCurrentToken();
      bool bMatchedOnce = false;
      while ( Parser.Scanner.HasMoreTokens() )
      {
        auto* Peek = Parser.Scanner.PeekToken();
        if ( !Peek )
        {
          break;
        }
        else if ( Peek->Token == EHlslToken::RightBrace )
        {
          // End of switch
          break;
        }
        else if ( Peek->Token == EHlslToken::Case || Peek->Token == EHlslToken::Default )
        {
          // Next CaseStatement
          break;
        }
        else
        {
          AST::FNode* Statement = nullptr;
          auto Result = ParseStatement( Parser, &Statement );
          if ( Result == EParseResult::Error )
          {
            return EParseResult::Error;
          }
          else if ( Result == EParseResult::NotMatched )
          {
            Parser.Scanner.SourceError( _T( "Internal Error parsing statment inside case list" ) );
            return EParseResult::Error;
          }
          else
          {
            CaseStatement->Statements.Add( Statement );
          }
        }
      }

      Body->CaseList->Cases.Add( CaseStatement );
    }

    *OutBody = Body;
    return EParseResult::Matched;
  }

  EParseResult ParseSwitchStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    const auto* Token = Parser.Scanner.GetCurrentToken();
    if ( !Parser.Scanner.MatchToken( EHlslToken::LeftParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "'(' expected" ) );
      return EParseResult::Error;
    }

    AST::FExpression* Condition = nullptr;
    if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, false, &Condition ) != EParseResult::Matched )
    {
      Parser.Scanner.SourceError( _T( "Expression expected" ) );
      return EParseResult::Error;
    }

    if ( !Parser.Scanner.MatchToken( EHlslToken::RightParenthesis ) )
    {
      Parser.Scanner.SourceError( _T( "')' expected" ) );
      return EParseResult::Error;
    }

    AST::FSwitchBody* Body = nullptr;
    if ( ParseSwitchBody( Parser, &Body ) != EParseResult::Matched )
    {
      return EParseResult::Error;
    }

    auto* Switch = new AST::FSwitchStatement( Token->SourceInfo, Condition, Body );
    *OutStatement = Switch;

    return EParseResult::Matched;
  }

  EParseResult ParseExpressionStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    auto OriginalToken = Parser.Scanner.GetCurrentTokenIndex();
    auto* Statement = new AST::FExpressionStatement( nullptr, Parser.Scanner.GetCurrentToken()->SourceInfo );
    if ( ParseExpression( Parser.Scanner, Parser.CurrentScope, true, &Statement->Expression ) == EParseResult::Matched )
    {
      if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
      {
        *OutStatement = Statement;
        return EParseResult::Matched;
      }
    }

    Parser.Scanner.SetCurrentTokenIndex( OriginalToken );
    return EParseResult::NotMatched;
  }

  EParseResult ParseEmptyStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    BASEASSERT( *OutStatement == nullptr );

    // Nothing to do here...
    return EParseResult::Matched;
  }

  EParseResult ParseBreakStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    BASEASSERT( *OutStatement == nullptr );

    auto* Statement = new AST::FJumpStatement( AST::EJumpType::Break, Parser.Scanner.PeekToken( -1 )->SourceInfo );

    if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      *OutStatement = Statement;
      return EParseResult::Matched;
    }

    return EParseResult::Error;
  }

  EParseResult ParseContinueStatement( FHlslParser& Parser, AST::FNode** OutStatement )
  {
    BASEASSERT( *OutStatement == nullptr );

    auto* Statement = new AST::FJumpStatement( AST::EJumpType::Continue, Parser.Scanner.PeekToken( -1 )->SourceInfo );

    if ( Parser.Scanner.MatchToken( EHlslToken::Semicolon ) )
    {
      *OutStatement = Statement;
      return EParseResult::Matched;
    }

    return EParseResult::Error;
  }

  namespace ParserRules
  {
    static struct FStaticInitializer
    {
      FStaticInitializer()
      {
        // Top Level constructs
        RulesTranslationUnit.Add( FRulePair( EHlslToken::DefineMacro, ParseMacro ) );
        RulesTranslationUnit.Add( FRulePair( EHlslToken::CBuffer, ParseCBuffer ) );
        RulesTranslationUnit.Add( FRulePair( EHlslToken::Invalid, ParseFunctionDeclaration, true ) );
        RulesTranslationUnit.Add( FRulePair( EHlslToken::Invalid, ParseGlobalVariableDeclaration ) );

        RulesStatements.Add( FRulePair( EHlslToken::DefineMacro, ParseMacro ) );
        RulesStatements.Add( FRulePair( EHlslToken::LeftBrace, ParseStatementBlock ) );
        RulesStatements.Add( FRulePair( EHlslToken::Return, ParseReturnStatement ) );
        RulesStatements.Add( FRulePair( EHlslToken::Do, ParseDoStatement ) );
        RulesStatements.Add( FRulePair( EHlslToken::While, ParseWhileStatement, true ) );
        RulesStatements.Add( FRulePair( EHlslToken::For, ParseForStatement, true ) );
        RulesStatements.Add( FRulePair( EHlslToken::If, ParseIfStatement, true ) );
        RulesStatements.Add( FRulePair( EHlslToken::Switch, ParseSwitchStatement, true ) );
        RulesStatements.Add( FRulePair( EHlslToken::Semicolon, ParseEmptyStatement ) );
        RulesStatements.Add( FRulePair( EHlslToken::Break, ParseBreakStatement ) );
        RulesStatements.Add( FRulePair( EHlslToken::Continue, ParseContinueStatement ) );
        RulesStatements.Add( FRulePair( EHlslToken::Invalid, ParseLocalDeclaration ) );
        // Always try expressions last
        RulesStatements.Add( FRulePair( EHlslToken::Invalid, ParseExpressionStatement ) );
      }
    } GStaticInitializer;
  }

  FHlslParser::FHlslParser() :
    GlobalScope( nullptr )
  {
    CurrentScope = &GlobalScope;
  }

  namespace Parser
  {
    void FlushReferences( AST::FNode *Node )
    {
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
      {
        Node->Children[ x ]->References.FlushFast();
        FlushReferences( Node->Children[ x ] );
      }
    }

    void BuildReferences( AST::FNode *Node )
    {
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
      {
        switch ( Node->Children[ x ]->GetType() )
        {
        case AST::ENodeType::FunctionDefinition:
        {
          auto *n = ( AST::FFunctionDefinition* )Node->Children[ x ];
          //first the function call references
          if ( n->Body ) //check for recursion
            n->Body->FindReferences( n->Prototype );
          for ( TS32 y = x + 1; y < Node->Children.NumItems(); y++ )
            Node->Children[ y ]->FindReferences( n->Prototype );

          //now the function parameter references
          if ( !n->Body ) continue;
          for ( TS32 y = 0; y < n->Prototype->Parameters.NumItems(); y++ )
            n->Body->FindReferences( n->Prototype->Parameters[ y ] );

          if ( n->Body )
            BuildReferences( n->Body ); //recurse in function body
        }
        break;
        case AST::ENodeType::DeclaratorList:
        {
          auto *n = ( AST::FDeclaratorList* )Node->Children[ x ];
          //first set up type references
          for ( TS32 y = 0; y < n->Declarations.NumItems(); y++ )
            if ( n->Declarations[ y ]->GetType() == AST::ENodeType::Declaration )
            {
              auto *d = ( AST::FDeclaration* )n->Declarations[ y ];
              d->Type = n->Type;
            }

          if ( n->Type->Specifier->Structure ) //handle structure type definitions here
          {
            for ( int z = x + 1; z < Node->Children.NumItems(); z++ )
              Node->Children[ z ]->FindReferences( n->Type->Specifier->Structure );
            BuildReferences( n->Type->Specifier->Structure );
          }

          for ( TS32 y = 0; y < n->Declarations.NumItems(); y++ )
          {
            if ( n->Declarations[ y ]->GetType() == AST::ENodeType::Declaration )
            {
              auto *d = ( AST::FDeclaration* )n->Declarations[ y ];
              if ( n->Parent && n->Parent->GetType() == AST::ENodeType::StructSpecifier )
              {
                d->StructMember = true;
                d->ParentStruct = ( AST::FStructSpecifier* )n->Parent;
              }

              if ( !d->StructMember )
              {
                for ( int z = y + 1; z < n->Declarations.NumItems(); z++ ) //finish the current node
                  n->Declarations[ z ]->FindReferences( d );
                for ( int z = x + 1; z < Node->Children.NumItems(); z++ )
                  Node->Children[ z ]->FindReferences( d );
              }
              else
              {
                AST::FNode *t = n->Parent; //struct specifier
                if ( !t || t->GetType() != AST::ENodeType::StructSpecifier ) { LOG_ERR( "[AST] Struct climb error" ); continue; }
                t = t->Parent; //type specifier
                if ( !t || t->GetType() != AST::ENodeType::TypeSpecifier ) { LOG_ERR( "[AST] Struct climb error" ); continue; }
                t = t->Parent; //fully specified type
                if ( !t || t->GetType() != AST::ENodeType::FullySpecifiedType ) { LOG_ERR( "[AST] Struct climb error" ); continue; }
                t = t->Parent; //declarator list
                if ( !t || !t->Parent || t->GetType() != AST::ENodeType::DeclaratorList ) { LOG_ERR( "[AST] Struct climb error" ); continue; }

                TS32 idx = t->Parent->Children.Find( t );
                if ( idx < 0 ) { LOG_ERR( "[AST] Tree Error" ); continue; }

                for ( TS32 z = idx + 1; z < t->Parent->Children.NumItems(); z++ )
                  t->Parent->Children[ z ]->FindReferences( d );
              }
            }
          }
        }
        break;
        case AST::ENodeType::CBufferDeclaration:
        {
          auto *k = ( AST::FCBufferDeclaration* )Node->Children[ x ];
          for ( TS32 i = 0; i < k->Declarations.NumItems(); i++ )
          {
            if ( k->Declarations[ i ]->GetType() == AST::ENodeType::DeclaratorList )
            {
              auto *n = ( AST::FDeclaratorList* )k->Declarations[ i ];
              //do the same as with the declaratorlists, but to the current scope								

              //first set up type references
              for ( TS32 y = 0; y < n->Declarations.NumItems(); y++ )
                if ( n->Declarations[ y ]->GetType() == AST::ENodeType::Declaration )
                {
                  auto *d = ( AST::FDeclaration* )n->Declarations[ y ];
                  d->Type = n->Type;
                  d->CBufferMember = true; //mark as cbuffer member
                }

              //simplified due to special case of cbuffer
              for ( TS32 y = 0; y < n->Declarations.NumItems(); y++ )
              {
                if ( n->Declarations[ y ]->GetType() == AST::ENodeType::Declaration )
                {
                  auto *d = ( AST::FDeclaration* )n->Declarations[ y ];
                  for ( int z = x + 1; z < Node->Children.NumItems(); z++ )
                    Node->Children[ z ]->FindReferences( d );
                }
              }
            }
            else
              LOG_ERR( "[AST] Non declaration list in cbuffer!" );
          }
        }
        break;
        //case AST::ENodeType::IterationStatement:
        //{
        //	auto *n = (AST::FIterationStatement*)Nodes[x];
        //}
        //break;
        default:
          BuildReferences( Node->Children[ x ] );
          break;
        }

      }
    }

    //////////////////////////////////////////////////////////////////////////
    // buggy old version
    //////////////////////////////////////////////////////////////////////////

    //void BuildReferences(CArray<AST::FNode*> &Nodes, CArray<AST::FNode*> &ParentNodes)
    //{
    //	for (TS32 x = 0; x < Nodes.NumItems(); x++)
    //	{
    //		//function and function parameter references
    //		if (!Nodes[x]->Parent && Nodes[x]->GetType() == AST::ENodeType::FunctionDefinition)
    //		{
    //			auto *n = (AST::FFunctionDefinition*)Nodes[x];
    //			if (!n->Prototype) continue;

    //			//first the function call references
    //			if (n->Body) //check for recursion
    //				n->Body->FindReferences(n->Prototype);
    //			for (TS32 y = x + 1; y < Nodes.NumItems(); y++)
    //				Nodes[y]->FindReferences(n->Prototype);

    //			//now the function parameter references
    //			if (!n->Body) continue;
    //			for (TS32 y = 0; y < n->Prototype->Parameters.NumItems(); y++)
    //				n->Body->FindReferences(n->Prototype->Parameters[y]);
    //		}

    //		//variable declaration references
    //		if (Nodes[x]->GetType() == AST::ENodeType::DeclaratorList)
    //		{
    //			auto *n = (AST::FDeclaratorList*)Nodes[x];

    //			//first set up type references
    //			for (TS32 y = 0; y < n->Declarations.NumItems(); y++)
    //				if (n->Declarations[y]->GetType() == AST::ENodeType::Declaration)
    //				{
    //					auto *d = (AST::FDeclaration*)n->Declarations[y];
    //					d->Type = n->Type;
    //				}

    //			//then look through the remainder of the scope for variable references
    //			if (!n->Parent || n->Parent->GetType() != AST::ENodeType::CBufferDeclaration)
    //			{
    //				for (TS32 y = 0; y < n->Declarations.NumItems(); y++)
    //				{
    //					if (n->Declarations[y]->GetType() == AST::ENodeType::Declaration)
    //					{
    //						auto *d = (AST::FDeclaration*)n->Declarations[y];

    //						if (n->Parent && n->Parent->GetType() == AST::ENodeType::StructSpecifier)
    //							d->StructMember = true;

    //						for (int z = y + 1; z < n->Declarations.NumItems(); z++) //finish the current node
    //							n->Declarations[z]->FindReferences(d);
    //						for (int z = x + 1; z < Nodes.NumItems(); z++)
    //							Nodes[z]->FindReferences(d);
    //					}
    //				}
    //			}
    //			else
    //			{	//cbuffer, need to look on the cbuffer declaration level
    //				TS32 idx = ParentNodes.Find(n->Parent);
    //				if (idx >= 0)
    //				{
    //					for (TS32 y = 0; y < n->Declarations.NumItems(); y++)
    //					{
    //						if (n->Declarations[y]->GetType() == AST::ENodeType::Declaration)
    //						{
    //							auto *d = (AST::FDeclaration*)n->Declarations[y];
    //							d->CBufferMember = true;
    //							for (TS32 z = idx + 1; z < ParentNodes.NumItems(); z++)
    //								ParentNodes[z]->FindReferences(d);
    //						}
    //					}
    //				}
    //			}
    //		}
    //	}


    //	for (TS32 x = 0; x < Nodes.NumItems(); x++)
    //		BuildReferences(Nodes[x]->Children, Nodes);
    //}

    void CollapseDeclaratorLists( AST::FNode *Node )
    {
      //collapse trivial, this is also usable for data structures
      for ( TS32 x = 1; x < Node->Children.NumItems(); x++ )
      {
        if ( Node->Children[ x - 1 ]->GetType() == AST::DeclaratorList && Node->Children[ x ]->GetType() == AST::DeclaratorList )
        {
          auto *n0 = ( AST::FDeclaratorList* )Node->Children[ x - 1 ];
          auto *n1 = ( AST::FDeclaratorList* )Node->Children[ x ];
          if ( n0->Type && n1->Type && n0->Type->Specifier && n1->Type->Specifier && n0->Type->Specifier->TypeName.Length() && n1->Type->Specifier->TypeName.Length() )
            if ( n0->Type->Specifier->TypeName == n1->Type->Specifier->TypeName && n0->Type->Qualifier.Raw == n1->Type->Qualifier.Raw )
            {
              for ( TS32 y = 0; y < n1->Declarations.NumItems(); y++ )
              {
                auto *dec = n1->Declarations[ y ];
                n0->Declarations += dec;
                dec->Parent = n0; //hack to fix the tree
                n0->Children.AddUnique( dec );
              }
              n1->Declarations.FlushFast();
              n1->DetachFromParent();
              Node->Children.Delete( n1 ); //this is redundant but fixes the no parent case
              x--;
            }
        }
      }

      //collapse other
      TBOOL Struct = Node->Parent && ( Node->Parent->GetType() == AST::StructSpecifier );
      for ( TS32 x = 1; x < Node->Children.NumItems(); x++ )
      {
        if ( Node->Children[ x ]->GetType() == AST::DeclaratorList )
        {
          //find a target declaration list candidate
          TS32 FoundTargetIdx = -1;
          for ( TS32 y = 0; y < x; y++ )
          {
            if ( Node->Children[ y ]->GetType() == AST::DeclaratorList )
            {
              auto *n0 = ( AST::FDeclaratorList* )Node->Children[ x ];
              auto *n1 = ( AST::FDeclaratorList* )Node->Children[ y ];
              if ( n0->Type && n1->Type && n0->Type->Specifier && n1->Type->Specifier && n0->Type->Specifier->TypeName.Length() && n1->Type->Specifier->TypeName.Length() )
                if ( n0->Type->Specifier->TypeName == n1->Type->Specifier->TypeName && n0->Type->Qualifier.Raw == n1->Type->Qualifier.Raw )
                  FoundTargetIdx = y;
            }
          }

          //try to move declarations from here to there
          if ( FoundTargetIdx >= 0 )
          {
            auto *target = ( AST::FDeclaratorList* )Node->Children[ FoundTargetIdx ];
            auto *srcmain = ( AST::FDeclaratorList* )Node->Children[ x ];
            for ( TS32 y = 0; y < srcmain->Declarations.NumItems(); y++ )
            {
              if ( srcmain->Declarations[ y ]->GetType() != AST::Declaration )
              {
                LOG_ERR( "Non declaration found in declaration list, wtf" );
                continue;
              }
              AST::FDeclaration *d = ( AST::FDeclaration* )srcmain->Declarations[ y ];
              if ( Struct && !d->Semantic.Length() ) continue; //if structure member and doesn't have semantics defined the order can't be changed
              if ( d->CBufferMember ) continue; //cbuffer members are not moved

              TBOOL CanBeMoved = false;
              if ( !d->Initializer ) //not initialized, can be moved by default
                CanBeMoved = true;

              if ( !CanBeMoved )
              {
                TBOOL FoundModification = false;

                CArray<CString> Identifiers;
                d->Initializer->GetUniqueIdentifierList( Identifiers );
                for ( TS32 z = 0; z < d->ArraySize.NumItems(); z++ )
                  d->ArraySize[ z ]->GetUniqueIdentifierList( Identifiers );

                for ( TS32 z = 0; z < Identifiers.NumItems(); z++ )
                  if ( !FoundModification )
                  {
                    for ( TS32 i = FoundTargetIdx + 1; i < x; i++ )
                      FoundModification |= Node->Children[ i ]->FindVariableModification( Identifiers[ z ] );
                    for ( TS32 i = 0; i < y; i++ )
                      FoundModification |= srcmain->Declarations[ i ]->FindVariableModification( Identifiers[ z ] );
                  }

                if ( !FoundModification ) CanBeMoved = true;
              }

              if ( CanBeMoved )
              {
                auto *dec = srcmain->Declarations[ y ];
                target->Declarations += dec;
                //srcmain->Declarations.DeleteByIndex(y);
                dec->DetachFromParent();
                dec->Parent = target; //hack to fix the tree
                target->Children.AddUnique( dec );
                y--;
                if ( srcmain->Declarations.NumItems() == 0 )
                {
                  //declaration list empty
                  srcmain->DetachFromParent();
                  Node->Children.Delete( srcmain ); //redundant, but fixes the no parent case
                  x--;
                }
              }
            }
          }
        }
      }

      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
      {
        CollapseDeclaratorLists( Node->Children[ x ] );
      }
    }

    void RemoveNotNeededCBufferMembers( AST::FNode* Node )
    {
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
      {
        if ( Node->Children[ x ]->GetType() == AST::ENodeType::CBufferDeclaration )
        {
          TBOOL Stop = false;
          auto *cb = ( AST::FCBufferDeclaration* )Node->Children[ x ];
          for ( TS32 y = cb->Declarations.NumItems() - 1; y >= 0; y-- )
          {
            if ( cb->Declarations[ y ]->GetType() != AST::ENodeType::DeclaratorList )
            {
              LOG_ERR( "[AST] Non declaration list in cbuffer!" );
              continue;
            }
            auto *dl = ( AST::FDeclaratorList* )cb->Declarations[ y ];
            for ( TS32 z = dl->Declarations.NumItems() - 1; z >= 0; z-- )
            {
              auto *n = dl->Declarations[ z ];
              if ( !n->References.NumItems() )
                n->DetachFromParent();
              else
              {
                Stop = true;
                break;
              }
            }

            if ( !dl->Declarations.NumItems() )
              dl->DetachFromParent();

            if ( Stop )
              break;
          }
          if ( !cb->Declarations.NumItems() )
          {
            cb->DetachFromParent();
            Node->Children.Delete( cb );
            x--;
          }
        }
      }
    }

    void RemoveNotNeededVariables( AST::FNode *Node )
    {
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
      {
        if ( Node->Children[ x ]->GetType() == AST::ENodeType::DeclaratorList )
        {
          auto *n = ( AST::FDeclaratorList* )Node->Children[ x ];

          for ( TS32 y = 0; y < n->Declarations.NumItems(); y++ )
            if ( n->Declarations[ y ]->GetType() == AST::ENodeType::Declaration )
            {
              auto *d = ( AST::FDeclaration* )n->Declarations[ y ];
              if ( d->CBufferMember || d->StructMember ) continue; //cbuffer and struct members are left alone here

              TBOOL Removeable = !d->References.NumItems();
              if ( d->References.NumItems() == 1 && d->Initializer && !d->bIsArray )
              {
                //only single reference, check if it can be moved

                AST::FNode *Reference = d->References[ 0 ];

                TBOOL FoundModification = false;

                CArray<CString> Identifiers;
                d->Initializer->GetUniqueIdentifierList( Identifiers );
                for ( TS32 z = 0; z < d->ArraySize.NumItems(); z++ )
                  d->ArraySize[ z ]->GetUniqueIdentifierList( Identifiers );

                //first check the rest of the declaration list
                for ( TS32 z = 0; z < Identifiers.NumItems(); z++ )
                  if ( !FoundModification )
                    for ( TS32 i = y + 1; i < n->Declarations.NumItems(); i++ )
                      FoundModification |= Node->Children[ i ]->FindVariableModification( Identifiers[ z ] );

                //find the top node for the reference
                AST::FNode *RefCurrLevel = Reference;
                while ( RefCurrLevel && Node->Children.Find( RefCurrLevel ) == -1 ) RefCurrLevel = RefCurrLevel->Parent;

                TS32 startidx = x + 1;
                TS32 endidx = Node->Children.Find( RefCurrLevel );
                if ( endidx == -1 )
                {
                  LOG_ERR( "[AST] Couldn't find current level reference for variable %s", d->Identifier.GetPointer() );
                  continue;
                }

                CArray<AST::FNode*> CurrentLevel = Node->Children;

                while ( 1 )
                {
                  for ( TS32 i = startidx; i < endidx; i++ )
                    for ( TS32 z = 0; z < Identifiers.NumItems(); z++ )
                      if ( !FoundModification )
                        FoundModification |= CurrentLevel[ i ]->FindVariableModification( Identifiers[ z ] );
                  if ( CurrentLevel[ endidx ] == Reference ) break;
                  CurrentLevel = CurrentLevel[ endidx ]->Children;
                  startidx = 0;
                  AST::FNode *RefCurrLevel = Reference;
                  while ( RefCurrLevel && CurrentLevel.Find( RefCurrLevel ) == -1 ) RefCurrLevel = RefCurrLevel->Parent;
                  endidx = CurrentLevel.Find( RefCurrLevel );
                  if ( endidx == -1 )
                  {
                    LOG_ERR( "[AST] Couldn't find current level reference for variable %s", d->Identifier.GetPointer() );
                    FoundModification = true; //to avoid change
                    break;
                  }
                }

                if ( !FoundModification )
                {
                  Removeable = true;

                  //if ( d->Initializer/* && d->Initializer== AST::ENodeType::InitializerListExpression*/ )
                  //{
                  //  LOG_NFO( "[AST] %s d->Initializer type is %d", d->Identifier.GetPointer(), d->Initializer->GetType() );
                  //}

                  d->Initializer->ForceParenthesis = true;
                  Reference->ReplaceInParentWith( d->Initializer );
                }

              }

              if ( Removeable )
              {
                d->DetachFromParent();
                y--;
              }
            }

          if ( !n->Declarations.NumItems() )
          {
            //check to see if this is actually a struct definition

            AST::FTypeSpecifier *typspec = NULL;
            if ( n->Type && n->Type->Specifier ) typspec = n->Type->Specifier;

            if ( !typspec->Structure )
            {
              n->DetachFromParent();
              Node->Children.Delete( n ); //redundant but fixes the no parent case
              x--;
            }
          }
        }
      }

      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
        RemoveNotNeededVariables( Node->Children[ x ] );
    }

    void GetAlphaNumStatistics( AST::FNode *Node, CDictionary<TCHAR, TS32> &Statistics, TBOOL Verbose )
    {
      CString oldout = Output;
      Output = ""; //reset output string
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ ) //remove identifier bias
        Node->Children[ x ]->RecursiveBackupAndClearIdentifiers();
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ ) //dump data
        Node->Children[ x ]->Dump( 0 );
      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ ) //restore identifiers
        Node->Children[ x ]->RecursiveRestoreIdentifiers();

      if ( Verbose )
      {
        LOG_DBG( "--------------------------------" );
        LOG_DBG( "%s", Output.GetPointer() );
        LOG_DBG( "--------------------------------" );
      }

      for ( TU32 x = 0; x < Output.Length(); x++ )
      {
        if ( isalnum( Output[ x ] ) )
        {
          TCHAR ch = Output[ x ];

          if ( !Statistics.HasKey( ch ) ) Statistics[ ch ] = 0;

          TS32 n = Statistics[ ch ];
          n++;
          Statistics[ ch ] = n;
        }
      }

      //for (TS32 x = 0; x < Statistics.NumItems(); x++)
      //{
      //	auto *kdp = Statistics.GetKDPair(x);
      //	if (kdp)
      //	{
      //		LOG_DBG("[AST] character %c count: %d", kdp->Key, kdp->Data);
      //	}
      //}

      Output = oldout; //restore output

    }

    struct CharStat
    {
      TCHAR Char;
      TS32 Count;
    };

    TS32 AlphabetSorter( CharStat *a, CharStat *b )
    {
      return b->Count - a->Count;
    }

    //CArray<CString> GeneratedVariableNames;
    CArray<CharStat> Alphabet;
    TS32 AlphaCount = 0;
    //TS32 GeneratedNameCount = 0;

    void BuildAlphabet( CDictionary<TCHAR, TS32> &Statistics )
    {
      Alphabet.Flush();
      AlphaCount = 0;

      int maxCount = 0;
      for ( TS32 x = 0; x < Statistics.NumItems(); x++ )
      {
        auto* kdp = Statistics.GetKDPair( x );
        maxCount = max( maxCount, kdp->Data );
      }

      for ( TS32 x = 0; x < Statistics.NumItems(); x++ )
      {
        auto *kdp = Statistics.GetKDPair( x );
        CharStat s;
        s.Char = kdp->Key;

/*
        if ( kdp->Data < maxCount / 10.0f ) // cut bottom 10% of used chars
          continue;
*/

        if ( isalpha( s.Char ) )
          AlphaCount++;
        s.Count = kdp->Data;
        Alphabet += s;
      }

      Alphabet.Sort( AlphabetSorter );

      //for (TS32 x = 0; x < Alphabet.NumItems(); x++)
      //{
      //	auto kdp = Alphabet[x];
      //	LOG_DBG("[AST] character %c count: %d", kdp.Char, kdp.Count);
      //}

    }

    CString GenerateVariableName( TS32 Idx )
    {
      if ( !Alphabet.NumItems() ) return CString( "NULLSTATISTICS" );
      TS32 cnt = Idx;
      TS32 digitcount = 1;
      TS32 digits[ 100 ];
      memset( digits, 0, 100 );

      for ( TS32 x = 0; x < cnt; x++ )
      {
        digits[ 0 ]++;
        for ( TS32 y = 0; y < digitcount; y++ )
        {
          if ( y == digitcount - 1 )
          {
            if ( digits[ y ] >= AlphaCount )
            {
              digitcount++;
              memset( digits, 0, 100 );
              break;
            }
          }
          else
          {
            if ( digits[ y ] < Alphabet.NumItems() ) break;
            digits[ y + 1 ]++;
            digits[ y ] = 0;
          }
        }
      }

      CString s;
      for ( TS32 x = digitcount - 1; x >= 0; x-- )
      {
        if ( x == digitcount - 1 )
        {
          TS32 cnt = 0;
          TCHAR r = 0;
          for ( TS32 y = 0; y < Alphabet.NumItems(); y++ )
          {
            if ( isalpha( Alphabet[ y ].Char ) )
            {
              if ( cnt == digits[ x ] )
              {
                r = Alphabet[ y ].Char;
                break;
              }
              cnt++;
            }
          }
          s.Append( &r, 1 );
        }
        else s.Append( &Alphabet[ digits[ x ] ].Char, 1 );
      }

      return s;
    }

    CString GenerateSafeVariableName( TS32 &ID )
    {
      CString Name;
      do
      {
        Name = GenerateVariableName( ID++ );
      } while ( Name == "p" || Name == "v" || Name == "g" || Name == "d" || Name == "h" || macroNames.Find( Name ) >= 0 /*|| Name=="t0" || Name == "t1" || Name == "t2" || Name == "t3" || Name == "t4" || Name == "t5" || Name == "t6" || Name == "t7"*/ );
      return Name;
    }

    TS32 RefCountSorter( AST::FNode **a, AST::FNode **b )
    {
      return ( *b )->References.NumItems() - ( *a )->References.NumItems();
    }

    void GatherReferencedNodes( CArray<AST::FNode*> &In, CArray<AST::FNode*> &Out )
    {
      Out.FlushFast();
      for ( TS32 x = 0; x < In.NumItems(); x++ )
        In[ x ]->GatherReferencedNodes( Out );
      Out.Sort( RefCountSorter );
    }

    struct namerefcount
    {
      CString Name;
      TS32 RefCount = 0;
      int oldRef = -1;
      CString newName;
    };

    TS32 namerefsorter( namerefcount *a, namerefcount *b )
    {
      return b->RefCount - a->RefCount;
    }

    void GatherReferencedNames( CArray<AST::FNode*> &ReferencedNodes, CArray<namerefcount> &Out )
    {
      CDictionary<CString, TS32> Dic;

      for ( TS32 x = 0; x < ReferencedNodes.NumItems(); x++ )
      {
        AST::FNode *n = ReferencedNodes[ x ];
        CString str;
        if ( n->GetType() == AST::ENodeType::Declaration )
        {
          str = ( ( AST::FDeclaration* )n )->Identifier;
          //if (((AST::FDeclaration*)n)->StructMember) str = "member_" + str;
        }
        if ( n->GetType() == AST::ENodeType::Function ) str = ( ( AST::FFunction* )n )->Identifier;
        if ( n->GetType() == AST::ENodeType::ParameterDeclarator ) str = ( ( AST::FParameterDeclarator* )n )->Identifier;
        if ( n->GetType() == AST::ENodeType::StructSpecifier ) str = "struct_" + ( ( AST::FStructSpecifier* )n )->Name;

        if ( !Dic.HasKey( str ) ) Dic[ str ] = n->References.NumItems();
        else
        {
          TS32 cnt = Dic[ str ] + n->References.NumItems();
          Dic[ str ] = cnt;
        }
      }

      for ( TS32 x = 0; x < Dic.NumItems(); x++ )
      {
        namerefcount n;
        auto *kf = Dic.GetKDPair( x );
        n.Name = kf->Key;
        n.RefCount = kf->Data;
        Out += n;
      }

      Out.Sort( namerefsorter );
    }

    void OptimizeVariableNames( AST::FNode *Node, CArray<namerefcount>& externalNames )
    {
      CArray<AST::FNode*> ReferencedNodes;
      GatherReferencedNodes( Node->Children, ReferencedNodes );
      CArray<namerefcount> Count = externalNames;
      for ( int x = 0; x < Count.NumItems(); x++ )
        Count[ x ].oldRef = x;
      GatherReferencedNames( ReferencedNodes, Count );
      TS32 cnt = 0;
      for ( TS32 x = 0; x < Count.NumItems(); x++ )
      {
        CString name = GenerateSafeVariableName( cnt );
        if ( Count[ x ].oldRef >= 0 )
          externalNames[ Count[ x ].oldRef ].newName = name;
        for ( TS32 y = 0; y < ReferencedNodes.NumItems(); y++ )
        {
          AST::FNode *n = ReferencedNodes[ y ];
          CString str;
          if ( n->GetType() == AST::ENodeType::Declaration )
          {
            str = ( ( AST::FDeclaration* )n )->Identifier;
            //if (((AST::FDeclaration*)n)->StructMember) str = "member_" + str;
          }
          if ( n->GetType() == AST::ENodeType::Function )
          {
            str = ( ( AST::FFunction* )n )->Identifier;
            if ( str == "p" || str == "v" || str == "g" || str == "h" || str == "d" ) continue;
          }
          if ( n->GetType() == AST::ENodeType::ParameterDeclarator ) str = ( ( AST::FParameterDeclarator* )n )->Identifier;
          if ( n->GetType() == AST::ENodeType::StructSpecifier ) str = "struct_" + ( ( AST::FStructSpecifier* )n )->Name;

          if ( str == Count[ x ].Name )
          {
            n->RenameWithReferences( name );
            ReferencedNodes.Delete( n );
            y--;
          }
        }
      }
    }

    TS32 ShortenVariableNames( AST::FNode *Node, TS32 VariableID )
    {
      TS32 InScopeNames = VariableID;

      for ( TS32 x = 0; x < Node->Children.NumItems(); x++ )
      {
        switch ( Node->Children[ x ]->GetType() )
        {
        case AST::ENodeType::DeclaratorList:
        {
          auto *dl = ( AST::FDeclaratorList* )Node->Children[ x ];
          if ( dl->Type && dl->Type->Specifier && dl->Type->Specifier->Structure )
          {
            dl->Type->Specifier->Structure->Name = GenerateSafeVariableName( InScopeNames );
            ShortenVariableNames( dl->Type->Specifier->Structure, 0 );
          }

          for ( TS32 y = 0; y < dl->Declarations.NumItems(); y++ )
          {
            auto *dec = ( AST::FDeclaration* )dl->Declarations[ y ];

            CString NewName;
            if ( dec->CBufferMember )
              NewName = GenerateSafeVariableName( InScopeNames );
            else
              NewName = GenerateVariableName( InScopeNames++ );

            dec->Identifier = NewName;
            for ( TS32 z = 0; z < dec->References.NumItems(); z++ )
              if ( dec->References[ z ]->IsExpression() )
                ( ( AST::FExpression* )dec->References[ z ] )->Identifier = NewName;
          }
        }
        break;
        case AST::ENodeType::FunctionDefinition:
        {
          auto *f = ( AST::FFunctionDefinition* )Node->Children[ x ];
          if ( f->Body && f->Prototype )
          {
            //change function name
            CString CurrentName = f->Prototype->Identifier;

            if ( CurrentName != "p" && CurrentName != "v" && CurrentName != "g" && CurrentName != "d" && CurrentName != "h" )
            {
              CString NewName = GenerateSafeVariableName( InScopeNames );
              f->Prototype->Identifier = NewName;
              for ( TS32 z = 0; z < f->Prototype->References.NumItems(); z++ )
                if ( f->Prototype->References[ z ]->IsExpression() )
                  ( ( AST::FExpression* )f->Prototype->References[ z ] )->Identifier = NewName;
              CurrentName = NewName;
            }

            TS32 ScopeCopy = InScopeNames;
            for ( TS32 y = 0; y < f->Prototype->Parameters.NumItems(); y++ )
            {
              if ( f->Prototype->Parameters[ y ]->GetType() == AST::ENodeType::ParameterDeclarator )
              {
                auto *par = ( AST::FParameterDeclarator* )f->Prototype->Parameters[ y ];

                CString NewName;
                do
                {
                  NewName = GenerateVariableName( ScopeCopy++ );
                } while ( NewName == CurrentName ); //don't name any variables the same as the function
                par->Identifier = NewName;
                for ( TS32 z = 0; z < par->References.NumItems(); z++ )
                  if ( par->References[ z ]->IsExpression() )
                    ( ( AST::FExpression* )par->References[ z ] )->Identifier = NewName;

              }
            }

            ShortenVariableNames( f->Body, ScopeCopy );
          }
        }
        break;

        case AST::ENodeType::IterationStatement:
        {
          auto *f = ( AST::FIterationStatement* )Node->Children[ x ];
          ShortenVariableNames( Node->Children[ x ], InScopeNames );

          if ( f->InitStatement ) InScopeNames += f->InitStatement->CountDeclarations();
          if ( f->RestExpression ) InScopeNames += f->RestExpression->CountDeclarations();
          if ( f->Condition ) InScopeNames += f->Condition->CountDeclarations();

        }
        break;

        case AST::ENodeType::CBufferDeclaration:
        {

          InScopeNames = ShortenVariableNames( Node->Children[ x ], InScopeNames );
        }
        break;

        case AST::ENodeType::ShaderDocument:
        {
          ShortenVariableNames( Node->Children[ x ], InScopeNames );
        }
        break;

        case AST::ENodeType::TypeSpecifier:
        {
          ShortenVariableNames( Node->Children[ x ], InScopeNames );
        }
        break;

        case AST::ENodeType::FunctionExpression:
        {
          ShortenVariableNames( Node->Children[ x ], InScopeNames );
        }
        break;

        default:
          ShortenVariableNames( Node->Children[ x ], InScopeNames );
        }

      }

      return InScopeNames;
    }

    /*TBOOL SeesImpostor(CArray<AST::FNode*> &RootNodes, AST::FNode *eye, AST::FNode *ref, AST::FNode *impostor)
    {
    CArray<AST::FNode*> &CurrentNodes = eye->Parent ? eye->Parent->Children : RootNodes;
    TS32 CurrentNodeID = CurrentNodes.Find(eye);
    while (1)
    {
    for (TS32 x = CurrentNodeID; x >= 0; x--)
    {
    if (CurrentNodes[x] == ref) return false;
    if (CurrentNodes[x] == impostor) return true;

    switch (CurrentNodes[x]->GetType())
    {
    case AST::ENodeType::DeclaratorList:
    {
    auto *d = (AST::FDeclaratorList*)CurrentNodes[x];
    for (TS32 y = 0; y < d->Declarations.NumItems(); y++)
    {
    if (d->Declarations[y] == ref) return false;
    if (d->Declarations[y] == impostor) return true;
    }
    }
    break;
    case AST::ENodeType::FunctionDefinition:
    {
    auto *d = ((AST::FFunctionDefinition*)CurrentNodes[x])->Prototype;
    if (d == ref) return false;
    if (d == impostor) return true;
    for (TS32 y = 0; y < d->Parameters.NumItems(); y++)
    {
    if (d->Parameters[y] == ref) return false;
    if (d->Parameters[y] == impostor) return true;
    }
    }
    break;
    case AST::ENodeType::CBufferDeclaration:
    {
    auto *d = (AST::FCBufferDeclaration*)CurrentNodes[x];
    if (d->FindNodeInChildTree(ref)) return false;
    if (d->FindNodeInChildTree(impostor)) return true;
    }
    break;
    case AST::ENodeType::IterationStatement:
    {
    auto *d = (AST::FIterationStatement*)CurrentNodes[x];
    if (d->InitStatement)
    {
    if (d->InitStatement->FindNodeInChildTree(ref)) return false;
    if (d->InitStatement->FindNodeInChildTree(impostor)) return true;
    }
    //if (d->RestExpression)
    //{
    //	if (d->RestExpression->FindNodeInChildTree(ref)) return false;
    //	if (d->RestExpression->FindNodeInChildTree(impostor)) return true;
    //}
    //if (d->Condition)
    //{
    //	if (d->Condition->FindNodeInChildTree(ref)) return false;
    //	if (d->Condition->FindNodeInChildTree(impostor)) return true;
    //}
    }
    break;
    }
    }

    if (!eye->Parent)
    {
    LOG_ERR("[AST] Declaration not found while looking through scope!");
    return false;
    }

    eye = eye->Parent;
    CurrentNodes = eye->Parent ? eye->Parent->Children : RootNodes;
    CurrentNodeID = CurrentNodes.Find(eye);
    }
    }

    void OptimalVariableNameReduction(CArray<AST::FNode*> &Nodes)
    {
    CArray<AST::FNode*> ReferencedNodes;
    GatherReferencedNodes(Nodes, ReferencedNodes);

    TS32 cnt = 0;
    CString name = GenerateSafeVariableName(cnt);
    TS32 LastNewNameIndex = 0;

    for (TS32 x = 0; x < ReferencedNodes.NumItems(); x++)
    {
    auto *CurrentNode = ReferencedNodes[x];
    TBOOL CanNameBeReused = true;

    for (TS32 y = LastNewNameIndex; y < x - 1; y++)
    {
    AST::FNode *n = ReferencedNodes[y];
    for (TS32 z = 0; z < n->References.NumItems(); z++)
    {
    if (SeesImpostor(Nodes, n->References[z], n, CurrentNode))
    {
    CanNameBeReused = false;
    break;
    }
    }

    if (!CanNameBeReused) break;
    }

    if (!CanNameBeReused)
    {
    name = GenerateSafeVariableName(cnt);
    LastNewNameIndex = x;
    }

    CurrentNode->RenameWithReferences(name);
    }
    }*/

    CDictionary<TCHAR, TS32> NumStats;

    //bool ExpandDocumentTree( const CString& code, AST::FShaderDocument& doc )
    //{
    //  FHlslParser Parser;
    //  if ( !Parser.Scanner.Lex( code ) )
    //  {
    //    return false;
    //  }

    //  bool bSuccess = true;

    //  while ( Parser.Scanner.HasMoreTokens() )
    //  {
    //    auto LastIndex = Parser.Scanner.GetCurrentTokenIndex();

    //    static CString GlobalDeclOrDefinition( _T( "Global declaration or definition" ) );
    //    AST::FNode* Node = nullptr;
    //    auto Result = TryRules( RulesTranslationUnit, Parser, GlobalDeclOrDefinition, true, &Node );
    //    if ( Result == EParseResult::Error )
    //    {
    //      bSuccess = false;
    //      break;
    //    }
    //    else
    //    {
    //      BASEASSERT( Result == EParseResult::Matched );
    //      doc.Children.Add( Node );
    //      Node->Parent = &doc;
    //    }

    //    BASEASSERT( LastIndex != Parser.Scanner.GetCurrentTokenIndex() );
    //  }

    //  return bSuccess;
    //}

    bool Parse( const CString& Input, CString &Result, TBOOL Verbose, TBOOL ResetDictionary, TBOOL RebuildDictionary )
    {
      macroNames.FlushFast();
      Output = "";

      FHlslParser Parser;
      if ( !Parser.Scanner.Lex( Input ) )
      {
        return false;
      }

      bool bSuccess = true;

      FSourceInfo InInfo;
      InInfo.Column = 0;
      InInfo.Line = 0;
      AST::FShaderDocument Shader( InInfo );

      while ( Parser.Scanner.HasMoreTokens() )
      {
        auto LastIndex = Parser.Scanner.GetCurrentTokenIndex();

        static CString GlobalDeclOrDefinition( _T( "Global declaration or definition" ) );
        AST::FNode* Node = nullptr;
        auto Result = TryRules( RulesTranslationUnit, Parser, GlobalDeclOrDefinition, true, &Node );
        if ( Result == EParseResult::Error )
        {
          bSuccess = false;
          break;
        }
        else
        {
          BASEASSERT( Result == EParseResult::Matched );
          Shader.Children.Add( Node );
          Node->Parent = &Shader;
        }

        BASEASSERT( LastIndex != Parser.Scanner.GetCurrentTokenIndex() );
      }

      Shader.BuildChildArray();

      BuildReferences( &Shader );
      BuildReferences( &Shader );
      RemoveNotNeededCBufferMembers( &Shader );
      RemoveNotNeededVariables( &Shader );
      CollapseDeclaratorLists( &Shader );

      if ( ResetDictionary )
        NumStats.Flush();

      //BuildReferences(Nodes, Nodes);
      if ( RebuildDictionary )
        GetAlphaNumStatistics( &Shader, NumStats, Verbose );

      BuildAlphabet( NumStats );

      //for (TS32 x = 0; x < Nodes.NumItems(); x++)
      //	Nodes[x]->RebuildChildren();

      //OptimalVariableNameReduction(Nodes);

      if ( CrossCompiler::RenameIdentifiers )
      {
        ShortenVariableNames( &Shader, 0 );
        OptimizeVariableNames( &Shader, CArray<namerefcount>() );
      }

      Shader.Dump();

      //rebuild here to kill extra parentheses

      CString NewInput = Output;
      Output = "";

      FHlslParser Parser2;
      if ( !Parser2.Scanner.Lex( NewInput ) )
      {
        return false;
      }

      bSuccess = true;
      CArray<AST::FNode*> Nodes2;
      while ( Parser2.Scanner.HasMoreTokens() )
      {
        auto LastIndex = Parser2.Scanner.GetCurrentTokenIndex();

        static CString GlobalDeclOrDefinition( _T( "Global declaration or definition" ) );
        AST::FNode* Node = nullptr;
        auto Result = TryRules( RulesTranslationUnit, Parser2, GlobalDeclOrDefinition, true, &Node );
        if ( Result == EParseResult::Error )
        {
          bSuccess = false;
          break;
        }
        else
        {
          BASEASSERT( Result == EParseResult::Matched );
          Nodes2.Add( Node );
        }

        BASEASSERT( LastIndex != Parser2.Scanner.GetCurrentTokenIndex() );
      }

      for ( TS32 x = 0; x < Nodes2.NumItems(); x++ )
        Nodes2[ x ]->Dump( 0 );

      Nodes2.FreeArray();/**/

      if ( Verbose )
        LOG_DBG( "%s", Output.GetPointer() );

      Result = Output;
      Output = "";
      //LOG_DBG("%s", Output.GetPointer());

      Shader.Children.FreeArray();

      return bSuccess;
    }

    bool CompareTrees( AST::FNode* source, AST::FNode* target );

    bool IsImmutableFunction( AST::FNode* n )
    {
      if ( n->GetType() == AST::ENodeType::Function )
      {
        CString str = ( ( AST::FFunction* )n )->Identifier;
        if ( str == "p" || str == "v" || str == "g" || str == "h" || str == "d" ) return true;
      }

      if ( n->GetType() == AST::ENodeType::FunctionDefinition )
      {
        auto *f = ( AST::FFunctionDefinition* )n;
        if ( f->Body && f->Prototype )
        {
          //change function name
          CString str = f->Prototype->Identifier;
          if ( str == "p" || str == "v" || str == "g" || str == "h" || str == "d" ) return true;
        }
      }
      return false;
    }

    bool ProxyReferences( AST::FNode* n )
    {
      for ( int x = 0; x < n->References.NumItems(); x++ )
      {
        auto *d = n->References[ x ];
        while ( d->duplicateOf )
          d = d->duplicateOf;

        n->References[ x ] = d;
      }

      for ( int x = 0; x < n->Children.NumItems(); x++ )
        ProxyReferences( n->Children[ x ] );

      return true;
    }

    int tempCounter = 0;

    //bool FixTexgenConstantBufferAssignments( AST::FNode* node )
    //{
    //  CArray<AST::FDeclaration*> declarations;

    //  for ( int x = 0; x < node->Children.NumItems(); x++ )
    //  {
    //    if ( node->Children[ x ]->GetType() == AST::DeclaratorList )
    //    {
    //      AST::FDeclaratorList* d1 = ( AST::FDeclaratorList* )node->Children[ x ];

    //      bool remove = false;

    //      for ( int y = 0; y < d1->Declarations.NumItems(); y++ )
    //      {
    //        if ( d1->Declarations[ y ]->GetType() != AST::Declaration )
    //        {
    //          LOG_ERR( "A non-declaration found in a declarator list, WTF" );
    //          return false;
    //        }

    //        AST::FDeclaration* dec = ( AST::FDeclaration* )d1->Declarations[ y ];
    //        if ( dec->Register[ 0 ] == 'c' )
    //        {
    //          if ( !dec->Type )
    //          {
    //            LOG_ERR( "Declaration missing type, WTF" );
    //            return false;
    //          }

    //          if ( !dec->Type->Specifier )
    //          {
    //            LOG_ERR( "Declaration missing type specifier, WTF" );
    //            return false;
    //          }

    //          if ( dec->Type->Specifier->TypeName != "float4" )
    //          {
    //            LOG_ERR( "Raw register reference declaration not a float4 - unsupported operation, aborting." );
    //            return false;
    //          }

    //          for (int z=0; z<declarations.NumItems(); z++ )
    //            if ( !declarations[ z ]->Type->CompareTo( dec->Type ) )
    //            {
    //              LOG_ERR( "Constant buffer declaration type mismatch while minimizing" );
    //              return false;
    //            }

    //          remove = true;

    //          declarations += dec;
    //        }
    //      }

    //      if ( remove )
    //      {
    //        d1->DetachFromParent();
    //        x--;
    //      }
    //    }
    //  }

    //  if ( !declarations.NumItems() )
    //    return true;

    //  declarations.Sort( []( AST::FDeclaration** a, AST::FDeclaration** b ) -> int 
    //  { 
    //    return ( *a )->Register[ 1 ] - ( *b )->Register[ 1 ];
    //  } );

    //  FHlslParser Parser;
    //  Parser.Scanner.Lex( "cbuffer b : register(b0) { float4 placeholder; };" );

    //  auto LastIndex = Parser.Scanner.GetCurrentTokenIndex();

    //  static CString GlobalDeclOrDefinition( _T( "Global declaration or definition" ) );
    //  AST::FNode* BufferNode = nullptr;
    //  auto Result = TryRules( RulesTranslationUnit, Parser, GlobalDeclOrDefinition, true, &BufferNode );

    //  node->Children.InsertFirst( BufferNode );
    //  node->Parent = BufferNode;
    //  //node->BuildChildArray();

    //  int cntr = 0;

    //  if ( BufferNode->GetType() != AST::CBufferDeclaration )
    //  {
    //    LOG_ERR( "Not a cbuffer declaration, wtf" );
    //    return false;
    //  }

    //  AST::FCBufferDeclaration* bufDec = ( AST::FCBufferDeclaration* )BufferNode;
    //  if (bufDec->Declarations[0]->GetType()!= AST::DeclaratorList)
    //  {
    //    LOG_ERR( "Not a DeclaratorList, wtf" );
    //    return false;
    //  }

    //  AST::FDeclaratorList* decList = ( AST::FDeclaratorList* )bufDec->Declarations[ 0 ];

    //  if ( decList->Declarations.NumItems() != 1 || decList->Declarations[ 0 ]->GetType() != AST::Declaration )
    //  {
    //    LOG_ERR( "Missing declaration 0!" );
    //    return false;
    //  }

    //  AST::FDeclaration* baseDec = ( AST::FDeclaration* )decList->Declarations[ 0 ];
    //  baseDec->DetachFromParent();
    //  
    //  decList->Declarations.FlushFast();

    //  for ( int x = 0; x < declarations.NumItems(); x++ )
    //  {
    //    int curr = 0;
    //    declarations[ x ]->Register.Scan( "c%d", &curr );

    //    //add missing registers
    //    while ( cntr < curr )
    //    {
    //      AST::FDeclaration* newDec = new AST::FDeclaration( node->SourceInfo );
    //      *newDec = *baseDec;
    //      newDec->Identifier = CString::Format( "_temp_counter_%d", tempCounter++ );
    //      decList->Declarations += newDec;
    //      cntr++;
    //    }

    //    declarations[ x ]->Register = "";
    //    decList->Declarations += declarations[ x ];
    //    cntr++;
    //  }

    //  return true;
    //}

    CString FixCBufferTexgenProblem( CString& input )
    {
      CStringArray statements = input.Explode( ";" );

      CString buffer = "cbuffer b : register(b0) \n{\n";

      struct BufferAssignment
      {
        CString varName;
        int regIDX;
      };

      CArray<BufferAssignment> regs;

      for ( int x = 0; x < statements.NumItems(); x++ )
      {
        int regpos = 0;
        int floatpos = 0;

        if ( ( regpos = statements[ x ].Find( ":register(c" ) ) < 0 )
          continue;

        if ( ( floatpos = statements[ x ].Find( "float4 " ) ) < 0 )
        {
          LOG_ERR( ":register(c not in the same line as float4!" );
          continue;
        }

        if ( regpos < floatpos )
        {
          LOG_ERR( ":register(c preceding float4!" );
          continue;
        }

        floatpos += 6;
        regpos += 11;

        while ( statements[ x ][ floatpos ] && isspace( statements[ x ][ floatpos ] ) )
          floatpos++;

        int colonpos = statements[ x ].Find( ":", floatpos );
        CString varName = statements[ x ].Substring( floatpos, colonpos - floatpos );

        int regID = 0;
        if ( statements[ x ].Substring( regpos ).Scan( "%d", &regID ) != 1 )
        {
          LOG_ERR( "failed to scan register id!" );
          continue;
        }

        BufferAssignment ass;
        ass.varName = varName;
        ass.regIDX = regID;
        regs += ass;

        statements.DeleteByIndex( x );
        x--;
      }

      if ( !regs.NumItems() )
        return input;

      regs.Sort( []( BufferAssignment* a, BufferAssignment* b )->int { return a->regIDX - b->regIDX; } );

      int cntr = 0;

      for ( int x = 0; x < regs.NumItems(); x++ )
      {
        int curr = regs[ x ].regIDX;

        //add missing registers
        while ( cntr < curr )
        {
          buffer += CString::Format( "float4 _temp_counter_%d;\n", tempCounter++ );
          cntr++;
        }

        buffer += CString::Format( "float4 %s;\n", regs[ x ].varName.GetPointer() );
        cntr++;
      }

      buffer += "};\n";

      CString result = buffer + statements.Implode( ";" );

      return result;
    }

    CString ReplaceKeywords( CString input, CArray<namerefcount>& keywordRenames )
    {
      Output = " ";
      Output += input + " ";

      for ( int x = 0; x < keywordRenames.NumItems(); x++ )
      {
        CStringArray exp = Output.Explode( keywordRenames[ x ].Name );

        if ( exp.NumItems() )
        {
          Output = "";

          int count = 0;

          for ( int y = 0; y < exp.NumItems(); y++ )
          {
            if ( y > 0 )
            {
              if ( isalnum( Output[ Output.Length() - 1 ] ) || isalnum( exp[ y ][ 0 ] ) )
                Output += keywordRenames[ x ].Name;
              else
                Output += keywordRenames[ x ].newName;
            }

            Output += exp[ y ];
          }
        }
      }

      if ( Output[ 0 ] == ' ' )
        Output = Output.Substring( 1 );

      if ( Output[ Output.Length() - 1 ] == ' ' )
        Output = Output.Substring( 0, Output.Length() - 1 );

      return Output;
    }

    bool MinifyMultipleShaders( CArray<CString>& input, CString& include, CArray<CString>& output )
    {
      macroNames.FlushFast();

      tempCounter = 0;

      CArray< AST::FShaderDocument* > trees;

      for ( int x = 0; x < input.NumItems(); x++ )
      {
        input[ x ] = FixCBufferTexgenProblem( input[ x ] );

        FHlslParser Parser;
        if ( !Parser.Scanner.Lex( input[ x ] ) )
        {
          LOG_ERR( "Failed to lex shader %d: %s", x, input[ x ].GetPointer() );
          return false;
        }

        bool bSuccess = true;

        FSourceInfo InInfo;
        InInfo.Column = 0;
        InInfo.Line = 0;
        AST::FShaderDocument* doc = new AST::FShaderDocument( InInfo );

        while ( Parser.Scanner.HasMoreTokens() )
        {
          auto LastIndex = Parser.Scanner.GetCurrentTokenIndex();

          static CString GlobalDeclOrDefinition( _T( "Global declaration or definition" ) );
          AST::FNode* Node = nullptr;
          auto Result = TryRules( RulesTranslationUnit, Parser, GlobalDeclOrDefinition, true, &Node );
          if ( Result == EParseResult::Error )
          {
            bSuccess = false;
            break;
          }
          else
          {
            BASEASSERT( Result == EParseResult::Matched );
            doc->Children.Add( Node );
            Node->Parent = doc;
          }

          BASEASSERT( LastIndex != Parser.Scanner.GetCurrentTokenIndex() );
        }

        if ( !bSuccess )
        {
          LOG_ERR( "Failed to parse shader %d: %s", x, input[ x ].GetPointer() );
          return false;
        }

        doc->BuildChildArray();

        //if ( !FixTexgenConstantBufferAssignments( doc ) )
        //  return false;

        BuildReferences( doc );
        BuildReferences( doc );

        RemoveNotNeededCBufferMembers( doc );
        RemoveNotNeededVariables( doc );
        CollapseDeclaratorLists( doc );

        doc->CountSubTreeSize();

        trees += doc;

        //LOG_NFO( "Shader %d: %d bytes %d root nodes, %d nodes", x, input[ x ].Length(), doc->Children.NumItems(), doc->subTreeSize );
      }

      {
        CString debugStr;
        for ( int x = 0; x < trees.NumItems(); x++ )
        {
          Output = "";
          trees[ x ]->Dump();
          debugStr += Output;
        }
        LOG_WARN( "Shader minification debug step 1: %d %x", debugStr.Length(), debugStr.GetHash() );
      }

      int duplicateSize = 0;

      CString header;

      FSourceInfo InInfo2;
      InInfo2.Column = 0;
      InInfo2.Line = 0;
      AST::FShaderDocument* headerDoc = new AST::FShaderDocument( InInfo2 );

      // mark referenced global declarations as writeable first


      for ( int x = 0; x < trees.NumItems(); x++ )
      {
        //LOG_NFO( "Processing shader %d", x );

        AST::FShaderDocument* docA = trees[ x ];
        for ( int i = 0; i < docA->Children.NumItems(); i++ )
        {
          AST::FNode* nodeA = docA->Children[ i ];

          if ( IsImmutableFunction( nodeA ) )
            continue;

          for ( int y = x + 1; y < trees.NumItems(); y++ )
          {
            AST::FShaderDocument* docB = trees[ y ];

            for ( int j = 0; j < docB->Children.NumItems(); j++ )
            {
              AST::FNode* nodeB = docB->Children[ j ];

              if ( IsImmutableFunction( nodeB ) )
                continue;

              if ( nodeA->duplicateOf != nullptr || nodeB->duplicateOf != nullptr )
                continue;

              if ( nodeA->CompareTo( nodeB ) )
                nodeA->MarkReferencedGlobalsAsRequired();
            }
          }
        }
      }

      // do double tree comparison and write header

      for ( int x = 0; x < trees.NumItems(); x++ )
      {
        //LOG_NFO( "Processing shader %d", x );

        AST::FShaderDocument* docA = trees[ x ];
        for ( int i = 0; i < docA->Children.NumItems(); i++ )
        {
          AST::FNode* nodeA = docA->Children[ i ];
          bool written = false;

          if ( IsImmutableFunction( nodeA ) )
            continue;

          for ( int y = x + 1; y < trees.NumItems(); y++ )
          {
            //LOG_NFO( "Comparing shader %d to shader %d", x, y );

            AST::FShaderDocument* docB = trees[ y ];

            for ( int j = 0; j < docB->Children.NumItems(); j++ )
            {
              AST::FNode* nodeB = docB->Children[ j ];

              if ( IsImmutableFunction( nodeB ) )
                continue;

              // always write out global variable declarations as their lists have already been
              // combined, may differ from shader to shader and won't match with CompareTo()
              if ( nodeA->Parent == docA && nodeA->GetType()==AST::ENodeType::DeclaratorList)
              {
                AST::FDeclaratorList* declList = (AST::FDeclaratorList*)nodeA;

                if ( nodeA->requiredForCommonHeader && !written && (declList->Type->Specifier->TypeName=="Texture2D" || declList->Type->Specifier->TypeName == "Texture3D" ))
                {
                  Output = "";

                  nodeA->Dump( 0, false );
                  CString resultA = Output;
                  header += resultA + "\n";
                  written = true;
                }
              }

              if ( nodeA->duplicateOf != nullptr || nodeB->duplicateOf != nullptr )
                continue;

              if ( nodeA->CompareTo( nodeB ) )
              {
                nodeB->SetDuplicateOf( nodeA );
                nodeB->PointReferencesToOriginal();

                Output = "";
                nodeA->Dump( 0, false );
                CString resultA = Output;

                if ( !written )
                {
                  //ExpandDocumentTree( resultA, headerDoc );
                  header += resultA + "\n";
                }

                written = true;

                Output = "";
                nodeB->Dump( 0, false );
                CString resultB = Output;

                // remove duplicate from root
                nodeB->DetachFromParent();
                //delete nodeB;
                //docB->Children.DeleteByIndex( j );
                j--;

                duplicateSize += resultB.Length();

                //if ( resultA != resultB )
                //{
                //  LOG_NFO( "Root level tree recurrence!" );
                //  LOG_NFO( "%s", resultA.GetPointer() );
                //  LOG_NFO( "%s", resultB.GetPointer() );
                //}
              }
            }
          }

          if ( written ) // move original to header data
          {
            nodeA->DetachFromParent();
            headerDoc->Children.Add( nodeA );
            i--;
          }
        }
      }

      {
        CString debugStr;
        for ( int x = 0; x < trees.NumItems(); x++ )
        {
          Output = "";
          trees[ x ]->Dump();
          debugStr += Output;
        }
        LOG_WARN( "Shader minification debug step 2: %d %x", debugStr.Length(), debugStr.GetHash() );
      }

      // deduplicate common register declarations

      for ( int x = 0; x < headerDoc->Children.NumItems(); x++ )
      {
        if ( headerDoc->Children[ x ]->GetType() == AST::DeclaratorList )
        {
          AST::FDeclaratorList* d1 = ( AST::FDeclaratorList* )headerDoc->Children[ x ];

          for ( int y = x + 1; y < headerDoc->Children.NumItems(); y++ )
          {
            if ( headerDoc->Children[ y ]->GetType() == AST::DeclaratorList )
            {
              AST::FDeclaratorList* d2 = ( AST::FDeclaratorList* )headerDoc->Children[ y ];

              if ( d1->Type->CompareTo( d2->Type ) )
              {
                for ( int i = 0; i < d2->Declarations.NumItems(); i++ )
                {
                  auto *a = d2->Declarations[ i ];
                  d2->Declarations.Delete( a );
                  i--;
                  a->DetachFromParent();
                  d1->Declarations += a;
                  d1->Children += a;
                  a->Parent = d1;
                }

                headerDoc->Children.DeleteByIndex( y );
                y--;
              }

            }
          }

          // at this point all of the same typed declarations are in Children[x]

          for ( int y = 0; y < d1->Declarations.NumItems(); y++ )
          {
            AST::FNode *a = d1->Declarations[ y ];

            for ( int z = y + 1; z < d1->Declarations.NumItems(); z++ )
            {
              AST::FNode *b = d1->Declarations[ z ];

              if ( a->CompareTo( b ) ) //DUPLICATE!
              {
                b->SetDuplicateOf( a );
                b->PointReferencesToOriginal();
                d1->Declarations.DeleteByIndex( z );
                z--;
                b->DetachFromParent();
              }
            }
          }

          // loop through all the other trees and find matches

          for ( int y = 0; y < trees.NumItems(); y++ )
          {
            for ( int z = 0; z < trees[ y ]->Children.NumItems(); z++ )
            {
              if ( trees[ y ]->Children[ z ]->GetType() == AST::DeclaratorList )
              {
                AST::FDeclaratorList* d2 = ( AST::FDeclaratorList* )trees[ y ]->Children[ z ];
                if ( d1->Type->CompareTo( d2->Type ) )
                {
                  for ( int i = 0; i < d2->Declarations.NumItems(); i++ )
                  {
                    AST::FNode *b = d2->Declarations[ i ];

                    for ( int j = 0; j < d1->Declarations.NumItems(); j++ )
                    {
                      AST::FNode *a = d1->Declarations[ j ];

                      if ( a->CompareTo( b ) ) // match found!
                      {
                        b->SetDuplicateOf( a );
                        b->PointReferencesToOriginal();
                        d2->Declarations.DeleteByIndex( i );
                        i--;
                        b->DetachFromParent();
                      }
                    }

                    if ( !d2->Declarations.NumItems() )
                    {
                      d2->DetachFromParent();
                      z--;
                    }

                  }

                }
              }
            }
          }
        }
      }

      {
        CString debugStr;
        for ( int x = 0; x < trees.NumItems(); x++ )
        {
          Output = "";
          trees[ x ]->Dump();
          debugStr += Output;
        }
        LOG_WARN( "Shader minification debug step 3: %d %x", debugStr.Length(), debugStr.GetHash() );
      }

      // do variable renaming

      // add all shaders to main doc for common processing
      for ( int x = 0; x < trees.NumItems(); x++ )
      {
        //for ( int y = 0; y < trees[ x ]->Children.NumItems(); y++ )
        headerDoc->Children += trees[ x ];// ->Children[ y ];
      }

      ProxyReferences( headerDoc );

      CArray<CString> macroableKeywords;
      macroableKeywords += "SamplerComparisonState";
      macroableKeywords += "SampleCmpLevelZero";
      macroableKeywords += "maxvertexcount";
      macroableKeywords += "RestartStrip";
      macroableKeywords += "SamplerState";
      macroableKeywords += "sv_position";
      macroableKeywords += "SampleLevel";
      macroableKeywords += "sv_target0";
      macroableKeywords += "sv_target1";
      macroableKeywords += "SampleBias";
      macroableKeywords += "smoothstep";
      macroableKeywords += "texcoord0";
      macroableKeywords += "texcoord1";
      macroableKeywords += "texcoord2";
      macroableKeywords += "texcoord3";
      macroableKeywords += "texcoord4";
      macroableKeywords += "position0";
      macroableKeywords += "position1";
      macroableKeywords += "normalize";
      macroableKeywords += "transpose";
      macroableKeywords += "Texture2D";
      macroableKeywords += "register";
      macroableKeywords += "float4x4";
      macroableKeywords += "float3x3";
      macroableKeywords += "float2x2";
      macroableKeywords += "saturate";
      macroableKeywords += "reflect";
      macroableKeywords += "discard";
      macroableKeywords += "cbuffer";
      macroableKeywords += "normal0";
      macroableKeywords += "static";
      macroableKeywords += "length";
      macroableKeywords += "color0";
      macroableKeywords += "float2";
      macroableKeywords += "float3";
      macroableKeywords += "float4";
      macroableKeywords += "Append";
      macroableKeywords += "Sample";
      macroableKeywords += "return";
      macroableKeywords += "struct";
      macroableKeywords += "rsqrt";
      macroableKeywords += "floor";
      macroableKeywords += "point";
      macroableKeywords += "cross";
      macroableKeywords += "clamp";
      macroableKeywords += "float";
      macroableKeywords += "break";
      macroableKeywords += "const";
      macroableKeywords += "inout";
      macroableKeywords += "bool";
      macroableKeywords += "else";
      macroableKeywords += "lerp";
      macroableKeywords += "Load";
      macroableKeywords += "void";
      macroableKeywords += "acos";
      macroableKeywords += "sqrt";
      macroableKeywords += "frac";
      macroableKeywords += "fmod";
      macroableKeywords += "int2";
      macroableKeywords += "int3";
      macroableKeywords += "exp2";
      macroableKeywords += "sin";
      macroableKeywords += "cos";
      macroableKeywords += "mul";
      macroableKeywords += "int";
      macroableKeywords += "dot";
      macroableKeywords += "min";
      macroableKeywords += "max";
      macroableKeywords += "abs";
      macroableKeywords += "pow";
      macroableKeywords += "for";
      macroableKeywords += "ddx";
      macroableKeywords += "ddy";
      macroableKeywords += "exp";

      macroableKeywords.Sort( []( CString* a, CString* b ) -> int { return b->Length() - a->Length(); } );

      //////////////////////////////////////////////////////////////////
      // macrify keywords

      CArray<namerefcount> keywordCounts;

      {
        //count keywords

        CString debugStr;
        Output = " ";
        headerDoc->RecursiveBackupAndClearIdentifiers();
        headerDoc->Dump();
        headerDoc->RecursiveRestoreIdentifiers();
        debugStr += Output;
        for ( int x = 0; x < trees.NumItems(); x++ )
        {
          Output = "";
          trees[ x ]->RecursiveBackupAndClearIdentifiers();
          trees[ x ]->Dump();
          trees[ x ]->RecursiveRestoreIdentifiers();
          debugStr += Output;
        }

        debugStr += " ";

        for ( int x = 0; x < macroableKeywords.NumItems(); x++ )
        {
          CStringArray exp = debugStr.Explode( macroableKeywords[ x ] );

          debugStr = "";

          int count = 0;

          for ( int y = 0; y < exp.NumItems(); y++ )
          {
            debugStr += exp[ y ];
            if ( isalnum( debugStr[ debugStr.Length() - 1 ] ) || ( y < exp.NumItems() - 1 && isalnum( exp[ y + 1 ][ 0 ] ) ) )
              debugStr += macroableKeywords[ x ];
            else
              count++;
          }

          // assuming 2 byte name length this will shrink the shaders:
          if ( 12 + macroableKeywords[ x ].Length() < count * ( macroableKeywords[ x ].Length() - 2 ) )
          {
            namerefcount w;
            w.RefCount = count;
            w.Name = macroableKeywords[ x ];
            keywordCounts += w;
          }

        }

        //LOG_WARN( "Stripped shaders:" );
        //LOG_WARN( "%s", debugStr.GetPointer() );
      }


      NumStats.Flush();

      //BuildReferences(Nodes, Nodes);
      GetAlphaNumStatistics( headerDoc, NumStats, false );
      //for ( int x = 0; x < trees.NumItems(); x++ )
      //  GetAlphaNumStatistics( trees[ x ], NumStats, false );

      BuildAlphabet( NumStats );

      ShortenVariableNames( headerDoc, 0 );
      //OptimizeVariableNames( headerDoc, keywordCounts );
      OptimizeVariableNames( headerDoc, CArray<namerefcount>() );

      for ( int x = 0; x < trees.NumItems(); x++ )
      {
        //for ( int y = 0; y < trees[ x ]->Children.NumItems(); y++ )
        headerDoc->Children.Delete( trees[ x ]/*->Children[ y ]*/ );
      }

      //for ( int x = 0; x < keywordCounts.NumItems(); x++ )
      //{
      //  LOG_WARN( "%s renamed to %s", keywordCounts[ x ].Name.GetPointer(), keywordCounts[ x ].newName.GetPointer() );
      //}

      //for ( int x = 0; x < trees.NumItems(); x++ )
      //{
      //  ShortenVariableNames( trees[ x ], 0 );
      //  OptimizeVariableNames( trees[ x ] );
      //}

      // dump!

      //TBOOL pprint = PrettyPrint;
      //PrettyPrint = true;

      //RecursiveBackupAndClearIdentifiers


      // dump

      {
        CString debugStr;
        for ( int x = 0; x < trees.NumItems(); x++ )
        {
          Output = "";
          trees[ x ]->Dump();
          debugStr += Output;
        }
        LOG_WARN( "Shader minification debug step 4: %d %x", debugStr.Length(), debugStr.GetHash() );
      }

      Output = "";
      headerDoc->Dump( 0, false );

      Output += "\n"; //make sure the header doesn't end in a macro


      // MACRO HACKERY - FAILED ATTEMPT
      // add rename macros
      /*
      CString defines;
      for ( int x = 0; x < keywordCounts.NumItems(); x++ )
        defines += CString::Format( "#define %s %s\n", keywordCounts[ x ].newName.GetPointer(), keywordCounts[ x ].Name.GetPointer() );

      include = defines + ReplaceKeywords( Output, keywordCounts );*/

      include = Output;

      LOG_NFO( "Possible bytes saved by deduplicating shader code: %d", duplicateSize );
      LOG_NFO( "Common code part: %s", Output.GetPointer() );

      for ( int x = 0; x < trees.NumItems(); x++ )
      {
        Output = "";
        trees[ x ]->Dump( 0, false );

        // MACRO HACKERY - FAILED ATTEMPT
        //Output = ReplaceKeywords( Output, keywordCounts );

        output += Output;

        //LOG_NFO( "%s", Output.GetPointer() );
      }


      //PrettyPrint = pprint;

      //trees.FreeArray();

      return true;
    }

    int counts[ 255 ];

    struct SubTreeData
    {
      AST::FNode* node;
      //AST::FShaderDocument* parentShader;
    };

    struct SubTreeRecurrence
    {
      int size = 0;
      int count = 0;
      SubTreeData first;
      CArray< SubTreeData > occurences;
    };

    int countSubTreeSize( AST::FNode* node, int count )
    {
      int cnt = 0;
      for ( int x = 0; x < node->Children.NumItems(); x++ )
        cnt += countSubTreeSize( node->Children[ x ], cnt );
      return cnt + 1;
    }

    void RecurseShaderBlobFunctions( AST::FNode* node )
    {
      counts[ node->GetType() ]++;
      for ( int x = 0; x < node->Children.NumItems(); x++ )
        RecurseShaderBlobFunctions( node->Children[ x ] );
    }

    bool CompareTrees( AST::FNode* source, AST::FNode* target )
    {
      if ( source->subTreeSize != target->subTreeSize )
        return false;

      if ( !source->CompareTo( target ) )
        return false;

      if ( source->Children.NumItems() != target->Children.NumItems() )
        return false;

      for ( int x = 0; x < source->Children.NumItems(); x++ )
        if ( !CompareTrees( source->Children[ x ], target->Children[ x ] ) )
          return false;

      return true;
    }

    void FindSubTreeIn( AST::FNode* source, AST::FNode* target, SubTreeRecurrence& duplicates )
    {
      if ( source->duplicateOf )
        return;

      if ( target->duplicateOf )
        return;

      if ( source == target )
        return;

      // don't count leaf nodes... would result in too many hits
      if ( source->Children.NumItems() == 0 )
        return;

      if ( CompareTrees( source, target ) )
      {
        SubTreeData d;
        d.node = target;
        duplicates.occurences.Add( d );
        target->duplicateOf = source;
      }
      else
        for ( int x = 0; x < target->Children.NumItems(); x++ )
          if ( source->subTreeSize <= target->Children[ x ]->subTreeSize )
            FindSubTreeIn( source, target->Children[ x ], duplicates );
    }

    /*
        void FindIdenticalSubTrees(AST::FNode* source, AST::FNode* target, SubTreeRecurrence& duplicates)
        {
          int reccnt = rec.occurences.NumItems();
          FindSubTreeIn(source, target, rec);

          if (rec.occurences.NumItems())
          {
            rec.size = countSubTreeSize(source, 0);
            duplicates += rec;
          }
          else
            for (int x = 0; x < source->Children.NumItems(); x++)
              FindIdenticalSubTrees(source->Children[x], target, duplicates);
        }
    */

    void FindIdenticalSubTrees( AST::FNode* source, CArray<AST::FShaderDocument*> shaders, int startidx, CArray<SubTreeRecurrence>& duplicates )
    {
      if ( source->duplicateOf )
        return;

      SubTreeRecurrence rec;
      rec.first.node = source;

      for ( int x = startidx; x < shaders.NumItems(); x++ )
        FindSubTreeIn( source, shaders[ x ], rec );

      if ( rec.occurences.NumItems() )
      {
        rec.size = countSubTreeSize( source, 0 );
        duplicates += rec;
      }
      else
        for ( int x = 0; x < source->Children.NumItems(); x++ )
          FindIdenticalSubTrees( source->Children[ x ], shaders, startidx, duplicates );
    }

    bool BuildShaderBlob( const CArray<CString>& shaders )
    {
      CArray<AST::FShaderDocument*> shaderDocs;

      for ( int x = 0; x < shaders.NumItems(); x++ )
      {
        LOG_NFO( "Processing shader %d of %d" );
        FHlslParser Parser;
        if ( !Parser.Scanner.Lex( shaders[ x ] ) )
        {
          LOG_ERR( "Failed to Lex shader %d of %d", x, shaders.NumItems() );
          shaderDocs.FreeArray();
          return false;
        }

        bool bSuccess = true;

        FSourceInfo InInfo;
        InInfo.Column = 0;
        InInfo.Line = 0;
        AST::FShaderDocument* Shader = new AST::FShaderDocument( InInfo );

        while ( Parser.Scanner.HasMoreTokens() )
        {
          auto LastIndex = Parser.Scanner.GetCurrentTokenIndex();

          static CString GlobalDeclOrDefinition( _T( "Global declaration or definition" ) );
          AST::FNode* Node = nullptr;
          auto Result = TryRules( RulesTranslationUnit, Parser, GlobalDeclOrDefinition, true, &Node );
          if ( Result == EParseResult::Error )
          {
            bSuccess = false;
            LOG_ERR( "Failed to Parse shader %d of %d", x, shaders.NumItems() );
            break;
          }
          else
          {
            BASEASSERT( Result == EParseResult::Matched );
            Shader->Children.Add( Node );
            Node->Parent = Shader;
          }

          BASEASSERT( LastIndex != Parser.Scanner.GetCurrentTokenIndex() );
        }

        shaderDocs += Shader;

      }

      int shadersSize = 0;
      int shaderPartCount = 0;

      for ( int x = 0; x < shaderDocs.NumItems(); x++ )
      {
        shaderDocs[ x ]->BuildChildArray();
        BuildReferences( shaderDocs[ x ] );
        BuildReferences( shaderDocs[ x ] );
        RemoveNotNeededCBufferMembers( shaderDocs[ x ] );
        RemoveNotNeededVariables( shaderDocs[ x ] );
        CollapseDeclaratorLists( shaderDocs[ x ] );

        Output = "";
        shaderDocs[ x ]->Dump();
        shadersSize += Output.Length();
      }

      memset( counts, 0, 255 * sizeof( int ) );
      for ( int x = 0; x < shaderDocs.NumItems(); x++ )
      {
        shaderDocs[ x ]->CountSubTreeSize();
        RecurseShaderBlobFunctions( shaderDocs[ x ] );
        shaderDocs[ x ]->ClearDuplicateInfo();
      }

      for ( int x = 0; x < 255; x++ )
        shaderPartCount += counts[ x ];

      CArray<SubTreeRecurrence> duplicates;

      for ( int x = 0; x < shaderDocs.NumItems(); x++ )
        FindIdenticalSubTrees( shaderDocs[ 0 ], shaderDocs, x, duplicates );
      //for (int y = 0; y < shaderDocs.NumItems(); y++)
      //  FindIdenticalSubTrees(shaderDocs[0], shaderDocs[y], duplicates);

      for ( int x = 0; x < duplicates.NumItems(); x++ )
        duplicates[ x ].count = duplicates[ x ].occurences.NumItems() + 1;

      for ( int x = 0; x < shaderDocs.NumItems(); x++ )
      {
        LOG_NFO( "-------------------------------------" );
        LOG_NFO( "Shader %d of %d", x, shaderDocs.NumItems() );
        LOG_NFO( "-------------------------------------" );

        for ( int y = 0; y < shaderDocs[ x ]->Children.NumItems(); y++ )
        {
          LOG_NFO( "RootItem %d of %d: %d", y, shaderDocs[ x ]->Children.NumItems(), shaderDocs[ x ]->Children[ y ]->GetType() );
        }
      }

      //shaderDocs.FreeArray();

      return true;

    }

  }
}