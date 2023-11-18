#include "BasePCH.h"
#include "HLSLAST.h"

//////////////////////////////////////////////////////////////////////////
// HLSL Processing lifted from from Unreal Engine 4 v4.7 (c) Epic Games
#include "HlslAST.h"

CStringArray macroNames;

namespace CrossCompiler
{
  CString Output;

  CString TruncateFloatString( CString str )
  {
    CStringArray s = str.Explode( "." );
    if ( s.Last()[ s.Last().Length() - 1 ] == 'f' || s.Last()[ s.Last().Length() - 1 ] == 'F' )
    {
      s.Last()[ s.Last().Length() - 1 ] = 0;
      s.Last() = CString( s.Last() );
    }

    for ( TS32 x = 0; x < s.NumItems(); x++ )
    {
      TBOOL AllZero = true;
      for ( TU32 y = 0; y < s[ x ].Length(); y++ )
        if ( s[ x ][ y ] != '0' ) AllZero = false;
      if ( AllZero )
        s[ x ][ 0 ] = 0;
    }

    TBOOL AllZero = true;
    for ( TS32 x = 0; x < s.NumItems(); x++ )
      if ( s[ x ][ 0 ] != 0 ) AllZero = false;

    if ( AllZero )
      return CString( "0" );

    if ( s.NumItems() == 1 )
      return s[ 0 ] + ".";

    return s.Implode( "." );
  }

  TBOOL PrettyPrint = false;
  TBOOL RenameIdentifiers = true;
  TBOOL ForceAllParentheses = false;

#define EXPANDOUTPUT(x) do { CString v=x; if (!PrettyPrint) v.RemoveNewLines(); Output.Append(v); } while (0)
#define EXPANDOUTPUT2(x,y) do { CString v=CString::Format(x,y); if (!PrettyPrint) v.RemoveNewLines(); Output.Append(v); } while (0)
#define EXPANDOUTPUT3(x,y,z) do { CString v=CString::Format(x,y,z); if (!PrettyPrint) v.RemoveNewLines(); Output.Append(v); } while (0)

  namespace AST
  {
    TS32 GetOperatorPrecedence( EOperators Op )
    {
      switch ( Op )
      {
      case CrossCompiler::AST::EOperators::Assign: return 15;
      case CrossCompiler::AST::EOperators::Plus: return 3;
      case CrossCompiler::AST::EOperators::Neg: return 3;
      case CrossCompiler::AST::EOperators::Add: return 6;
      case CrossCompiler::AST::EOperators::Sub: return 6;
      case CrossCompiler::AST::EOperators::Mul: return 5;
      case CrossCompiler::AST::EOperators::Div: return 5;
      case CrossCompiler::AST::EOperators::Mod: return 5;
      case CrossCompiler::AST::EOperators::LShift: return 7;
      case CrossCompiler::AST::EOperators::RShift: return 7;
      case CrossCompiler::AST::EOperators::Less: return 8;
      case CrossCompiler::AST::EOperators::Greater: return 8;
      case CrossCompiler::AST::EOperators::LEqual: return 8;
      case CrossCompiler::AST::EOperators::GEqual: return 8;
      case CrossCompiler::AST::EOperators::Equal: return 9;
      case CrossCompiler::AST::EOperators::NEqual: return 9;
      case CrossCompiler::AST::EOperators::BitAnd: return 10;
      case CrossCompiler::AST::EOperators::BitXor: return 11;
      case CrossCompiler::AST::EOperators::BitOr: return 12;
      case CrossCompiler::AST::EOperators::BitNot: return 3;
      case CrossCompiler::AST::EOperators::LogicAnd: return 13;
      case CrossCompiler::AST::EOperators::LogicXor: return 13; // ???
      case CrossCompiler::AST::EOperators::LogicOr: return 14;
      case CrossCompiler::AST::EOperators::LogicNot: return 3;
      case CrossCompiler::AST::EOperators::MulAssign: return 15;
      case CrossCompiler::AST::EOperators::DivAssign: return 15;
      case CrossCompiler::AST::EOperators::ModAssign: return 15;
      case CrossCompiler::AST::EOperators::AddAssign: return 15;
      case CrossCompiler::AST::EOperators::SubAssign: return 15;
      case CrossCompiler::AST::EOperators::LSAssign: return 15;
      case CrossCompiler::AST::EOperators::RSAssign: return 15;
      case CrossCompiler::AST::EOperators::AndAssign: return 15;
      case CrossCompiler::AST::EOperators::XorAssign: return 15;
      case CrossCompiler::AST::EOperators::OrAssign: return 15;
      case CrossCompiler::AST::EOperators::Conditional: return 15;
      case CrossCompiler::AST::EOperators::PreInc: return 3;
      case CrossCompiler::AST::EOperators::PreDec: return 3;
      case CrossCompiler::AST::EOperators::PostInc: return 2;
      case CrossCompiler::AST::EOperators::PostDec: return 2;
      case CrossCompiler::AST::EOperators::FieldSelection: return 2;
      case CrossCompiler::AST::EOperators::ArrayIndex: return 2;
      case CrossCompiler::AST::EOperators::FunctionCall: return 2;
      case CrossCompiler::AST::EOperators::InitializerList: return 0;
      case CrossCompiler::AST::EOperators::Identifier: return 0;
      case CrossCompiler::AST::EOperators::UintConstant: return 0;
      case CrossCompiler::AST::EOperators::FloatConstant: return 0;
      case CrossCompiler::AST::EOperators::BoolConstant: return 0;
      case CrossCompiler::AST::EOperators::TypeCast: return 3;
      default:

        return 100; //???
      }
    }

    TBOOL GetOperatorAssociativity(EOperators Op)
    {
    	switch (Op)
    	{
    		case CrossCompiler::AST::EOperators::Assign: return false;
    		case CrossCompiler::AST::EOperators::Plus: return false;
    		case CrossCompiler::AST::EOperators::Neg: return false;
    		case CrossCompiler::AST::EOperators::Add: return true;
    		case CrossCompiler::AST::EOperators::Sub: return true;
    		case CrossCompiler::AST::EOperators::Mul: return true;
    		case CrossCompiler::AST::EOperators::Div: return true;
    		case CrossCompiler::AST::EOperators::Mod: return true;
    		case CrossCompiler::AST::EOperators::LShift: return true;
    		case CrossCompiler::AST::EOperators::RShift: return true;
    		case CrossCompiler::AST::EOperators::Less: return true;
    		case CrossCompiler::AST::EOperators::Greater: return true;
    		case CrossCompiler::AST::EOperators::LEqual: return true;
    		case CrossCompiler::AST::EOperators::GEqual: return true;
    		case CrossCompiler::AST::EOperators::Equal: return true;
    		case CrossCompiler::AST::EOperators::NEqual: return true;
    		case CrossCompiler::AST::EOperators::BitAnd: return true;
    		case CrossCompiler::AST::EOperators::BitXor: return true;
    		case CrossCompiler::AST::EOperators::BitOr: return true;
    		case CrossCompiler::AST::EOperators::BitNot: return false;
    		case CrossCompiler::AST::EOperators::LogicAnd: return true;
    		case CrossCompiler::AST::EOperators::LogicXor: return true;
    		case CrossCompiler::AST::EOperators::LogicOr: return true;
    		case CrossCompiler::AST::EOperators::LogicNot: return false;
    		case CrossCompiler::AST::EOperators::MulAssign: return false;
    		case CrossCompiler::AST::EOperators::DivAssign: return false;
    		case CrossCompiler::AST::EOperators::ModAssign: return false;
    		case CrossCompiler::AST::EOperators::AddAssign: return false;
    		case CrossCompiler::AST::EOperators::SubAssign: return false;
    		case CrossCompiler::AST::EOperators::LSAssign: return false;
    		case CrossCompiler::AST::EOperators::RSAssign: return false;
    		case CrossCompiler::AST::EOperators::AndAssign: return false;
    		case CrossCompiler::AST::EOperators::XorAssign: return false;
    		case CrossCompiler::AST::EOperators::OrAssign: return false;
    		case CrossCompiler::AST::EOperators::Conditional: return false;
    		case CrossCompiler::AST::EOperators::PreInc: return false;
    		case CrossCompiler::AST::EOperators::PreDec: return false;
    		case CrossCompiler::AST::EOperators::PostInc: return true;
    		case CrossCompiler::AST::EOperators::PostDec: return true;
    		case CrossCompiler::AST::EOperators::FieldSelection: return true;
    		case CrossCompiler::AST::EOperators::ArrayIndex: return true;
    		case CrossCompiler::AST::EOperators::FunctionCall: return true;
    		case CrossCompiler::AST::EOperators::InitializerList: return true; //?
    		case CrossCompiler::AST::EOperators::Identifier: return true; //?
    		case CrossCompiler::AST::EOperators::UintConstant: return true;
    		case CrossCompiler::AST::EOperators::FloatConstant: return true;
    		case CrossCompiler::AST::EOperators::BoolConstant: return true;
    		case CrossCompiler::AST::EOperators::TypeCast: return false;
    		default:

    			return true; //?
    	}
    }

    void DumpOptionalArraySize( bool bIsArray, const CArray<FExpression*>& ArraySize )
    {
      if ( bIsArray && ArraySize.NumItems() == 0 )
      {
        EXPANDOUTPUT( _T( "[]" ) );
      }
      else
      {
        for ( TS32 x = 0; x < ArraySize.NumItems(); x++ )
        {
          EXPANDOUTPUT( _T( "[" ) );
          if ( ArraySize[ x ] ) ArraySize[ x ]->Dump( 0 );
          EXPANDOUTPUT( _T( "]" ) );
        }
      }
    }

#if 0
    FNode::FNode()/* :
            Prev(nullptr),
            Next(nullptr)*/
    {
    }
#endif // 0

    bool FNode::CompareTo( FNode* node )
    {
/*
      LOG_WARN( "CompareTo %d vs %d", GetType(), node->GetType() );
      CString o = Output;
      Output = "";
      Dump( 0 );
      CString src = Output;
      Output = "";
      node->Dump( 0 );
      CString dst = Output;
      Output = o;
      LOG_WARN( "src: %s", src.GetPointer() );
      LOG_WARN( "dst: %s", dst.GetPointer() );
*/

      if ( GetType() != node->GetType() )
        return false;

      if ( Children.NumItems() != node->Children.NumItems() )
        return false;

      for ( int x = 0; x < Children.NumItems(); x++ )
        if ( !Children[ x ]->CompareTo( node->Children[ x ] ) )
          return false;

      if ( Attributes.NumItems() != node->Attributes.NumItems() )
        return false;

      for ( int x = 0; x < Attributes.NumItems(); x++ )
        if ( !Attributes[ x ]->CompareTo( node->Attributes[ x ] ) )
          return false;

      return true;
    }

    FNode::FNode( const FSourceInfo& innfo ) :
      SourceInfo( innfo ),
      Attributes()/*,
                 Prev(nullptr),
                 Next(nullptr)*/
    {
    }

    void FNode::DumpIndent( TS32 Indent )
    {
      //while (--Indent >= 0)
      //{
      //	EXPANDOUTPUT(_T("\t"));
      //}
    }

    void FNode::DumpAttributes() const
    {
      if ( Attributes.NumItems() > 0 )
      {
        for ( TS32 x = 0; x < Attributes.NumItems(); x++ )
        {
          Attributes[ x ]->Dump( 0 );
        }

        EXPANDOUTPUT( _T( " " ) );
      }
    }

    FExpression::FExpression( EOperators InOperator, FExpression* E0, FExpression* E1, FExpression* E2, const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Operator( InOperator ),
      Identifier( "" ),
      Expressions()
    {
      SubExpressions[ 0 ] = E0;
      SubExpressions[ 1 ] = E1;
      SubExpressions[ 2 ] = E2;
      TypeSpecifier = 0;
      UintConstant = 0;
    }

    void FExpression::DumpOperator() const
    {
      switch ( Operator )
      {
      case EOperators::Plus:
        EXPANDOUTPUT( _T( "+" ) );
        break;

      case EOperators::Neg:
        EXPANDOUTPUT( _T( "-" ) );
        break;

      case EOperators::Assign:
        EXPANDOUTPUT( _T( "=" ) );
        break;

      case EOperators::AddAssign:
        EXPANDOUTPUT( _T( "+=" ) );
        break;

      case EOperators::SubAssign:
        EXPANDOUTPUT( _T( "-=" ) );
        break;

      case EOperators::MulAssign:
        EXPANDOUTPUT( _T( "*=" ) );
        break;

      case EOperators::DivAssign:
        EXPANDOUTPUT( _T( "/=" ) );
        break;

      case EOperators::ModAssign:
        EXPANDOUTPUT( _T( "%=" ) );
        break;

      case EOperators::RSAssign:
        EXPANDOUTPUT( _T( ">>=" ) );
        break;

      case EOperators::LSAssign:
        EXPANDOUTPUT( _T( "<<=" ) );
        break;

      case EOperators::AndAssign:
        EXPANDOUTPUT( _T( "&=" ) );
        break;

      case EOperators::OrAssign:
        EXPANDOUTPUT( _T( "|=" ) );
        break;

      case EOperators::XorAssign:
        EXPANDOUTPUT( _T( "^=" ) );
        break;

      case EOperators::Conditional:
        EXPANDOUTPUT( _T( "?" ) );
        break;

      case EOperators::LogicOr:
        EXPANDOUTPUT( _T( "||" ) );
        break;

      case EOperators::LogicAnd:
        EXPANDOUTPUT( _T( "&&" ) );
        break;

      case EOperators::LogicNot:
        EXPANDOUTPUT( _T( "!" ) );
        break;

      case EOperators::BitOr:
        EXPANDOUTPUT( _T( "|" ) );
        break;

      case EOperators::BitXor:
        EXPANDOUTPUT( _T( "^" ) );
        break;

      case EOperators::BitAnd:
        EXPANDOUTPUT( _T( "&" ) );
        break;

      case EOperators::Equal:
        EXPANDOUTPUT( _T( "==" ) );
        break;

      case EOperators::NEqual:
        EXPANDOUTPUT( _T( "!=" ) );
        break;

      case EOperators::Less:
        EXPANDOUTPUT( _T( "<" ) );
        break;

      case EOperators::Greater:
        EXPANDOUTPUT( _T( ">" ) );
        break;

      case EOperators::LEqual:
        EXPANDOUTPUT( _T( "<=" ) );
        break;

      case EOperators::GEqual:
        EXPANDOUTPUT( _T( ">=" ) );
        break;

      case EOperators::LShift:
        EXPANDOUTPUT( _T( "<<" ) );
        break;

      case EOperators::RShift:
        EXPANDOUTPUT( _T( ">>" ) );
        break;

      case EOperators::Add:
        EXPANDOUTPUT( _T( "+" ) );
        break;

      case EOperators::Sub:
        EXPANDOUTPUT( _T( "-" ) );
        break;

      case EOperators::Mul:
        EXPANDOUTPUT( _T( "*" ) );
        break;

      case EOperators::Div:
        EXPANDOUTPUT( _T( "/" ) );
        break;

      case EOperators::Mod:
        EXPANDOUTPUT( _T( "%" ) );
        break;

      case EOperators::PreInc:
        EXPANDOUTPUT( _T( "++" ) );
        break;

      case EOperators::PreDec:
        EXPANDOUTPUT( _T( "--" ) );
        break;

      case EOperators::Identifier:
        EXPANDOUTPUT2( _T( "%s" ), Identifier.GetPointer() );
        break;

      case EOperators::UintConstant:
      case EOperators::BoolConstant:
        EXPANDOUTPUT2( _T( "%d" ), UintConstant );
        break;

      case EOperators::FloatConstant:
        //EXPANDOUTPUT2(_T("%f"), FloatConstant);
      {
        CString truncatedfloat = TruncateFloatString( OriginalFloatString );

        EXPANDOUTPUT2( _T( "%s" ), truncatedfloat.GetPointer() );
      }
      break;

      case EOperators::InitializerList:
        // Nothing...
        break;

      case EOperators::PostInc:
      case EOperators::PostDec:
      case EOperators::FieldSelection:
      case EOperators::ArrayIndex:
        break;

      case EOperators::TypeCast:
        EXPANDOUTPUT( _T( "(" ) );
        TypeSpecifier->Dump( 0 );
        EXPANDOUTPUT( _T( ")" ) );
        break;

      default:
        LOG_ERR( _T( "*MISSING_%d*" ), Operator );
        BASEASSERT( 0 );
        break;
      }
    }

    void FExpression::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      switch ( Operator )
      {
      case EOperators::Conditional:
      {
        if ( ForceAllParentheses || NeedsParenthesis || ForceParenthesis ) { 
          EXPANDOUTPUT( _T( "(" ) ); 
        }
        SubExpressions[ 0 ]->Dump( 0, SubExpressions[ 0 ]->Operator == EOperators::Conditional );
        EXPANDOUTPUT( _T( "?" ) );
        SubExpressions[ 1 ]->Dump( 0, false );
        EXPANDOUTPUT( _T( ":" ) );
        SubExpressions[ 2 ]->Dump( 0, false );
        if ( ForceAllParentheses || NeedsParenthesis || ForceParenthesis ) { 
          EXPANDOUTPUT( _T( ")" ) ); 
        }

        break;
      }

      default:
        LOG_ERR( _T( "*MISSING_%d*" ), Operator );
        BASEASSERT( 0 );
        break;
      }
    }

    void FExpression::BuildChildArray()
    {
      for ( TS32 x = 0; x < 3; x++ )
        if ( SubExpressions[ x ] ) SubExpressions[ x ]->SetParentAndBuildChildArray( this );
      for ( TS32 x = 0; x < Expressions.NumItems(); x++ )
        if ( Expressions[ x ] ) Expressions[ x ]->SetParentAndBuildChildArray( this );
    }

    void FExpression::RemoveChild( FNode *child )
    {
      for ( TS32 x = 0; x < 3; x++ )
        if ( SubExpressions[ x ] == child ) SubExpressions[ x ] = NULL;
      Expressions.Delete( (FExpression*)child );
    }

    void FExpression::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( !Target->IsExpression() )
      {
        LOG_ERR( "[AST] Trying to force non-expression node as an expression child" );
        return;
      }

      for ( TS32 x = 0; x < 3; x++ )
        if ( SubExpressions[ x ] == Source )
          SubExpressions[ x ] = ( AST::FExpression* )Target;

      TS32 x = Expressions.Find( (FExpression*)Source );
      if ( x >= 0 ) Expressions[ x ] = (FExpression*)Target;
    }

    TBOOL FExpression::FindVariableModification( CString VarName )
    {
      TBOOL result = false;
      switch ( Operator )
      {
      case EOperators::Conditional:
        result |= SubExpressions[ 0 ]->FindVariableModification( VarName );
        result |= SubExpressions[ 1 ]->FindVariableModification( VarName );
        result |= SubExpressions[ 2 ]->FindVariableModification( VarName );
        break;

      default:
        break;
      }

      return result;
    }

    FExpression::~FExpression()
    {
      if ( TypeSpecifier ) SAFEDELETE( TypeSpecifier );

      Expressions.FreeArray();

      for ( TS32 Index = 0; Index < 3; ++Index )
      {
        if ( SubExpressions[ Index ] )
        {
          delete SubExpressions[ Index ];
        }
        else
        {
          break;
        }
      }
    }

    bool FExpression::CompareTo( FNode* node )
    {
      if ( !FNode::CompareTo( node ) )
      {
        //LOG_WARN( "FExpression::CompareTo returned false. #1" );
        return false;
      }

      FExpression* other = (FExpression*)node;

      if ( Operator != other->Operator )
      {
        //LOG_WARN( "FExpression::CompareTo returned false. #2" );
        return false;
      }

      if ( OriginalFloatString != other->OriginalFloatString )
      {
        //LOG_WARN( "FExpression::CompareTo returned false. #3" );
        return false;
      }

      if ( !ReferencedNode && !other->ReferencedNode )
      {
        if ( UintConstant != other->UintConstant )
        {
          //LOG_WARN( "FExpression::CompareTo returned false. #4" );
          return false;
        }

        if ( FloatConstant != other->FloatConstant )
        {
          //LOG_WARN( "FExpression::CompareTo returned false. #4" );
          return false;
        }

        if ( BoolConstant != other->BoolConstant )
        {
          //LOG_WARN( "FExpression::CompareTo returned false. #4" );
          return false;
        }

        if ( Identifier != other->Identifier )
        {
          //LOG_WARN( "FExpression::CompareTo returned false. #4" );
          return false;
        }
      }

      if ( ( TypeSpecifier == nullptr ) != ( other->TypeSpecifier == nullptr ) )
      {
        //LOG_WARN( "FExpression::CompareTo returned false. #5" );
        return false;
      }

      if ( TypeSpecifier && !TypeSpecifier->CompareTo( other->TypeSpecifier ) )
      {
        //LOG_WARN( "FExpression::CompareTo returned false. #6" );
        return false;
      }

      if ( ( ReferencedNode != nullptr ) != ( other->ReferencedNode != nullptr ) )
        return false;

      if ( ReferencedNode )
      {
        if ( ReferencedNode->GetType() == ENodeType::Function )
        {
          if ( ( ReferencedNode->Parent != nullptr ) != ( other->ReferencedNode->Parent != nullptr ) )
          {
            return false;
          }
          if ( ReferencedNode->Parent )
          {
            return ReferencedNode->Parent->CompareTo( other->ReferencedNode->Parent );
          }
        }
        else
        {
          int x = 0;
        }
      }
      else
      {
        if ( Operator == EOperators::FunctionCall )
        {
          int x = 0;
        }
      }

      //LOG_WARN( "CompareTo not implemented for FCompoundStatement!" );
      //LOG_WARN( "FExpression::CompareTo returned true" );
      return true;
    }

    //bool FExpression::CompareTo( FNode* node )
    //{
    //  if ( !FNode::CompareTo( node ) )
    //    return false;

    //  FExpression* target = (FExpression*)node;

    //  if ( Operator != target->Operator )
    //    return false;

    //  switch ( Operator )
    //  {
    //  case CrossCompiler::AST::EOperators::FunctionCall:
    //  {
    //    int x = 0;
    //  }
    //  break;
    //  case CrossCompiler::AST::EOperators::Identifier:
    //  {
    //    if ( ReferencedNode == nullptr && target->ReferencedNode == nullptr )
    //      return true;

    //    if ( ReferencedNode == nullptr || target->ReferencedNode == nullptr )
    //      return false;

    //    FFullySpecifiedType* srcType = ReferencedNode->GetReferencedType();
    //    FFullySpecifiedType* trgType = target->ReferencedNode->GetReferencedType();

    //    if ( srcType == nullptr && trgType == nullptr )
    //      return true;

    //    if ( srcType == nullptr || trgType == nullptr )
    //      return false;

    //    if ( srcType->Primitive != trgType->Primitive )
    //      return false;

    //    if ( srcType->Qualifier.Raw != trgType->Qualifier.Raw )
    //      return false;

    //    if ( srcType->Specifier->TypeName != trgType->Specifier->TypeName )
    //      return false;

    //    if ( srcType->Specifier->InnerType != trgType->Specifier->InnerType )
    //      return false;

    //    if ( !srcType->Specifier->CompareTo( trgType->Specifier ) )
    //      return false;

    //    //if ( srcType->Specifier->Structure != trgType->Specifier->Structure ) // BAD BAD BAD BAD, NEED PROPER COMPARISON
    //    //  return false;

    //    if ( srcType->Specifier->TextureMSNumSamples != trgType->Specifier->TextureMSNumSamples )
    //      return false;

    //    if ( srcType->Specifier->PatchSize != trgType->Specifier->PatchSize )
    //      return false;

    //    if ( srcType->Specifier->bIsArray != trgType->Specifier->bIsArray )
    //      return false;

    //    if ( srcType->Specifier->ArraySize != trgType->Specifier->ArraySize )
    //      return false;

    //    //CString Primitive;
    //    //FTypeQualifier Qualifier;
    //    //FTypeSpecifier* Specifier;

    //    return true;
    //  }
    //  break;
    //  case CrossCompiler::AST::EOperators::FloatConstant:
    //    return FloatConstant == target->FloatConstant;
    //    break;
    //  case CrossCompiler::AST::EOperators::UintConstant:
    //  case CrossCompiler::AST::EOperators::BoolConstant:
    //    return UintConstant == target->UintConstant;
    //    break;
    //  case CrossCompiler::AST::EOperators::TypeCast:
    //  {
    //    int x = 0;
    //  }
    //  break;
    //  }

    //  return true;
    //}

    void FExpression::GetUniqueIdentifierList( CArray<CString> &List )
    {
      if ( Identifier.Length() )
        List.AddUnique( Identifier );
      for ( TS32 x = 0; x < 3; x++ )
        if ( SubExpressions[ x ] ) SubExpressions[ x ]->GetUniqueIdentifierList( List );
    }

    void FExpression::FindReferences( FNode *Declaration )
    {
      CString name;

      if ( Operator == EOperators::Identifier )
      {
        if ( Declaration->GetType() == AST::ENodeType::Declaration )
        {
          auto *fd = (FDeclaration*)Declaration;
          if ( !fd->StructMember ) //struct members can only be accessed via fieldselection
            name = fd->Identifier;
        }

        if ( Declaration->GetType() == AST::ENodeType::ParameterDeclarator )
        {
          auto *fd = (FParameterDeclarator*)Declaration;
          name = fd->Identifier;
        }

        if ( Declaration->GetType() == AST::ENodeType::Function && Parent )
        {
          if ( Parent->IsExpression() )
          {
            auto *p = (FExpression*)Parent;
            if ( p->Operator == AST::EOperators::FunctionCall && p->SubExpressions[ 0 ] == this )
            {
              auto *fd = (FFunction*)Declaration;
              name = fd->Identifier;
            }
          }
        }
      }

      if ( Operator == EOperators::FieldSelection )
      {
        auto *typ = SubExpressions[ 0 ]->GetStruct();
        if ( Declaration->GetType() == AST::ENodeType::Declaration )
        {
          auto *fd = (FDeclaration*)Declaration;
          if ( fd->StructMember ) //only struct members can be accessed via fieldselection
          {
            if ( Identifier == fd->Identifier )
            {
              FStructSpecifier *inc = fd->ParentStruct;
              if ( typ && typ == inc )
              {
                if ( ReferencedNode )
                {
                  LOG_ERR( "[AST] Duplicate struct member reference, wtf" );
                  ReferencedNode->References.Delete( this );
                }
                ReferencedNode = Declaration;
                Declaration->References += this;
              }
            }
          }
        }
      }


      //if found, add the reference
      if ( name.Length() && name == Identifier )
      {
        if ( ReferencedNode )
        {
          //duplicate name found, redefinition of higher scope variable name, remove original reference
          ReferencedNode->References.Delete( this );
        }
        ReferencedNode = Declaration;
        Declaration->References += this;
      }

      for ( TS32 x = 0; x < Children.NumItems(); x++ )
        Children[ x ]->FindReferences( Declaration );
    }

    struct FStructSpecifier * FExpression::GetStruct()
    {
      if ( Operator == EOperators::Conditional )
      {
        auto *a = SubExpressions[ 1 ]->GetStruct();
        auto *b = SubExpressions[ 2 ]->GetStruct();
        if ( a != b ) return NULL;
        return a;
      }

      if ( SubExpressions[ 0 ] && Operator == EOperators::FieldSelection )
      {
        FStructSpecifier *fs = SubExpressions[ 0 ]->GetStruct();
        if ( !fs ) return NULL;
        for ( TS32 x = 0; x < fs->Declarations.NumItems(); x++ )
          if ( fs->Declarations[ x ]->GetType() == ENodeType::DeclaratorList )
          {
            auto *dec = ( AST::FDeclaratorList* )fs->Declarations[ x ];
            for ( TS32 y = 0; y < dec->Declarations.NumItems(); y++ )
              if ( dec->Declarations[ y ]->GetType() == ENodeType::Declaration )
              {
                if ( Identifier == ( ( AST::FDeclaration* )dec->Declarations[ y ] )->Identifier )
                {
                  //type found!
                  auto *typ = dec->Type;
                  if ( dec->Type->Specifier ) return NULL;
                  return dec->Type->Specifier->ReferencedStruct;
                }
              }
          }
        return false;
      }

      if ( !SubExpressions[ 0 ] && Operator != EOperators::Identifier ) return NULL;
      if ( SubExpressions[ 0 ] ) return SubExpressions[ 0 ]->GetStruct();

      //this is an identifier
      if ( !ReferencedNode ) return NULL; //unknown reference

      switch ( ReferencedNode->GetType() )
      {
      case ENodeType::Declaration:
      {
        auto *d = ( AST::FDeclaration* )ReferencedNode;
        if ( !d->Type ) return NULL;
        if ( !d->Type->Specifier ) return NULL;
        return d->Type->Specifier->ReferencedStruct;
      }
      break;

      case ENodeType::ParameterDeclarator:
      {
        auto *d = ( AST::FParameterDeclarator* )ReferencedNode;
        if ( !d->Type ) return NULL;
        if ( !d->Type->Specifier ) return NULL;
        return d->Type->Specifier->ReferencedStruct;
      }
      break;

      case ENodeType::Function:
      {
        auto *d = ( AST::FFunction* )ReferencedNode;
        if ( !d->ReturnType ) return NULL;
        if ( !d->ReturnType->Specifier ) return NULL;
        return d->ReturnType->Specifier->ReferencedStruct;
      }
      break;

      default:
        return NULL;
        break;
      }
    }

    FUnaryExpression::FUnaryExpression( EOperators InOperator, FExpression* Expr, const FSourceInfo& InInfo ) :
      FExpression( InOperator, Expr, nullptr, nullptr, InInfo )
    {
    }

    void FUnaryExpression::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpOperator();
      if ( SubExpressions[ 0 ] )
      {
        bool needsparentheses = SubExpressions[ 0 ]->SubExpressions[ 0 ] && NeedsParenthesis;
        if ( ForceAllParentheses || needsparentheses || ForceParenthesis ) { EXPANDOUTPUT( _T( "(" ) ); }
        SubExpressions[ 0 ]->Dump( Indent, GetOperatorPrecedence( SubExpressions[ 0 ]->Operator ) > GetOperatorPrecedence( Operator ) );
        if ( ForceAllParentheses || needsparentheses || ForceParenthesis ) { EXPANDOUTPUT( _T( ")" ) ); }
      }

      // Suffix
      switch ( Operator )
      {
      case EOperators::PostInc:
        EXPANDOUTPUT( _T( "++" ) );
        break;

      case EOperators::PostDec:
        EXPANDOUTPUT( _T( "--" ) );
        break;

      case EOperators::FieldSelection:
        EXPANDOUTPUT2( _T( ".%s" ), Identifier.GetPointer() );
        break;

      default:
        break;
      }
    }

    TBOOL FUnaryExpression::FindVariableModification( CString VarName )
    {
      TBOOL result = false;
      if ( SubExpressions[ 0 ] ) result |= SubExpressions[ 0 ]->FindVariableModification( VarName );
      if ( Operator == EOperators::PostDec || Operator == EOperators::PostInc )
      {
        CArray<CString> Names;
        SubExpressions[ 0 ]->GetUniqueIdentifierList( Names );
        for ( TS32 x = 0; x < Names.NumItems(); x++ )
          if ( Names[ x ] == VarName ) return true;
        return true;
      }
      return result;
    }

    FBinaryExpression::FBinaryExpression( EOperators InOperator, FExpression* E0, FExpression* E1, const FSourceInfo& InInfo ) :
      FExpression( InOperator, E0, E1, nullptr, InInfo )
    {
    }

    void FBinaryExpression::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      switch ( Operator )
      {
      case EOperators::ArrayIndex:
      {
        bool needsparentheses = SubExpressions[ 0 ]->SubExpressions[ 0 ] && NeedsParenthesis;
        if ( ForceAllParentheses || needsparentheses || ForceParenthesis ) { EXPANDOUTPUT( _T( "(" ) ); }
        SubExpressions[ 0 ]->Dump( Indent, GetOperatorPrecedence( SubExpressions[ 0 ]->Operator ) > GetOperatorPrecedence( Operator ) );
        if ( ForceAllParentheses || needsparentheses || ForceParenthesis ) { EXPANDOUTPUT( _T( ")" ) ); }

        EXPANDOUTPUT( _T( "[" ) );
        SubExpressions[ 1 ]->Dump( Indent, false );
        EXPANDOUTPUT( _T( "]" ) );
        break;
      }

      default:
      {
        TBOOL LeftAssociative = GetOperatorAssociativity(Operator);
        if ( ForceAllParentheses || NeedsParenthesis || ForceParenthesis ) { EXPANDOUTPUT( _T( "(" ) ); }
        SubExpressions[ 0 ]->Dump( Indent, LeftAssociative ? GetOperatorPrecedence( SubExpressions[ 0 ]->Operator ) > GetOperatorPrecedence( Operator ) : GetOperatorPrecedence( SubExpressions[ 0 ]->Operator ) >= GetOperatorPrecedence( Operator ) );
        DumpOperator();
        SubExpressions[ 1 ]->Dump( Indent, LeftAssociative ? GetOperatorPrecedence( SubExpressions[ 1 ]->Operator ) >= GetOperatorPrecedence( Operator ) : GetOperatorPrecedence( SubExpressions[ 1 ]->Operator ) > GetOperatorPrecedence( Operator ) );
        if ( ForceAllParentheses || NeedsParenthesis || ForceParenthesis ) { EXPANDOUTPUT( _T( ")" ) ); }
        break;
      }
      }
    }

    TBOOL FBinaryExpression::FindVariableModification( CString VarName )
    {
      TBOOL result = false;
      switch ( Operator )
      {
      case EOperators::ArrayIndex:
      {
        result |= SubExpressions[ 0 ]->FindVariableModification( VarName );
        result |= SubExpressions[ 1 ]->FindVariableModification( VarName );
        break;
      }

      default:
      {
        result |= SubExpressions[ 0 ]->FindVariableModification( VarName );
        result |= SubExpressions[ 1 ]->FindVariableModification( VarName );

        switch ( Operator )
        {
        case EOperators::Assign:
        case EOperators::MulAssign:
        case EOperators::DivAssign:
        case EOperators::ModAssign:
        case EOperators::AddAssign:
        case EOperators::SubAssign:
        case EOperators::LSAssign:
        case EOperators::RSAssign:
        case EOperators::AndAssign:
        case EOperators::XorAssign:
        case EOperators::OrAssign:
        {
          if ( SubExpressions[ 0 ]->Operator == EOperators::Identifier )
            if ( SubExpressions[ 0 ]->Identifier == VarName ) return true;
          if ( SubExpressions[ 0 ]->Operator == EOperators::FieldSelection )
          {
            CArray<CString> Names;
            SubExpressions[ 0 ]->SubExpressions[ 0 ]->GetUniqueIdentifierList( Names );
            for ( TS32 x = 0; x < Names.NumItems(); x++ )
              if ( Names[ x ] == VarName ) return true;
          }
        }
        break;
        default:
          break;
        }
        break;
      }
      }
      return result;
    }

    FExpressionStatement::FExpressionStatement( FExpression* InExpr, const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Expression( InExpr )
    {
    }

    void FExpressionStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      Expression->Dump( Indent, false );
      EXPANDOUTPUT( _T( ";\n" ) );
    }

    void FExpressionStatement::BuildChildArray()
    {
      Expression->SetParentAndBuildChildArray( this );
    }

    void FExpressionStatement::ChangeChildTo( FNode *Src, FNode *Trg )
    {
      if ( !Trg->IsExpression() )
      {
        LOG_ERR( "[AST] Trying to force non-expression node as an expression child" );
        return;
      }
      if ( Expression == Src ) Expression = (FExpression*)Trg;
    }

    void FExpressionStatement::RemoveChild( FNode *child )
    {
      if ( Expression == child ) Expression = NULL;
    }

    TBOOL FExpressionStatement::FindVariableModification( CString VarName )
    {
      return Expression->FindVariableModification( VarName );
    }

    FExpressionStatement::~FExpressionStatement()
    {
      if ( Expression )
      {
        delete Expression;
      }
    }

    FCompoundStatement::FCompoundStatement( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Statements()
    {
    }

    void FCompoundStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );

      bool forceBrackets = true;

      if ( Statements.NumItems() != 1 || forceBrackets )
        EXPANDOUTPUT( _T( "{\n" ) );

      for ( TS32 x = 0; x < Statements.NumItems(); x++ )
      {
        bool needParenthesis = false;
/*

        //NESTED TERNARY PARENTHESIS TWEAK
        if ( x < Statements.NumItems() - 1 )
        {
          if ( Statements[ x ]->GetType() == ExpressionStatement )
          {
            if ( ( ( AST::FExpressionStatement* )Statements[ x ] )->Expression->Operator == EOperators::Conditional )
            {
              if ( Statements[ x + 1 ]->GetType() == ExpressionStatement )
              {
                if ( ( ( AST::FExpressionStatement* )Statements[ x + 1 ] )->Expression->Operator == EOperators::Conditional )
                  needParenthesis = true;
              }
            }
          }
        }

*/
        Statements[ x ]->Dump( Indent + 1, needParenthesis );
      }
      DumpIndent( Indent );
      
      if ( Statements.NumItems() != 1 || forceBrackets )
        EXPANDOUTPUT( _T( "}\n" ) );
    }

    void FCompoundStatement::BuildChildArray()
    {
      for ( TS32 x = 0; x < Statements.NumItems(); x++ )
        Statements[ x ]->SetParentAndBuildChildArray( this );
    }

    void FCompoundStatement::ChangeChildTo( FNode *Source, FNode *Target )
    {
      TS32 x = Statements.Find( Source );
      if ( x >= 0 ) Statements[ x ] = Target;
    }

    void FCompoundStatement::RemoveChild( FNode *child )
    {
      Statements.Delete( child );
    }

    TBOOL FCompoundStatement::FindVariableModification( CString VarName )
    {
      TBOOL result = false;
      for ( TS32 x = 0; x < Statements.NumItems(); x++ )
      {
        result |= Statements[ x ]->FindVariableModification( VarName );
      }
      return result;
    }

    FCompoundStatement::~FCompoundStatement()
    {
      Statements.FreeArray();
    }

    FFunctionDefinition::FFunctionDefinition( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Prototype( nullptr ),
      Body( nullptr )
    {
    }

    void FFunctionDefinition::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpAttributes();
      Prototype->Dump( 0 );
      if ( Body )
      {
        Body->Dump( Indent, false );
      }
    }

    void FFunctionDefinition::BuildChildArray()
    {
      Prototype->SetParentAndBuildChildArray( this );
      if ( Body ) Body->SetParentAndBuildChildArray( this );
    }

    void FFunctionDefinition::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Prototype == Source && Target->GetType() == ENodeType::Function ) Prototype = (FFunction*)Target;
      if ( Body == Source&& Target->GetType() == ENodeType::CompoundStatement ) Body = (FCompoundStatement*)Target;
    }

    void FFunctionDefinition::RemoveChild( FNode *child )
    {
      if ( Prototype == child ) Prototype = NULL;
      if ( Body == child ) Body = NULL;
    }

    TBOOL FFunctionDefinition::FindVariableModification( CString VarName )
    {
      TBOOL result = Prototype->FindVariableModification( VarName );
      if ( Body ) result |= Body->FindVariableModification( VarName );
      return result;
    }

    FFunctionDefinition::~FFunctionDefinition()
    {
      delete Prototype;
      delete Body;
    }

    FFunction::FFunction( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      ReturnType( nullptr ),
      Identifier( "" ),
      ReturnSemantic( "" ),
      Parameters()
    {
    }

    void FFunction::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      //LOG_DBG("Function '%s': %d references", Identifier.GetPointer(), References.NumItems());
      DumpAttributes();
      EXPANDOUTPUT( _T( "\n" ) );
      ReturnType->Dump( 0 );
      EXPANDOUTPUT2( _T( " %s(" ), Identifier.GetPointer() );
      //LOG_DBG("function: %s", Identifier.GetPointer());
      bool bFirst = true;
      for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
      {
        if ( bFirst )
        {
          bFirst = false;
        }
        else
        {
          EXPANDOUTPUT( _T( "," ) );
        }
        Parameters[ x ]->Dump( 0 );
      }

      EXPANDOUTPUT( _T( ")" ) );

      if ( ReturnSemantic.Length() )
      {
        CString rs = ReturnSemantic;
        rs.ToLower();
        EXPANDOUTPUT2( _T( ":%s\n" ), rs.GetPointer() );
      }
    }

    void FFunction::BuildChildArray()
    {
      ReturnType->SetParentAndBuildChildArray( this );
      for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
        Parameters[ x ]->SetParentAndBuildChildArray( this );
    }

    void FFunction::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( ReturnType == Source && Target->GetType() == ENodeType::FullySpecifiedType ) ReturnType = (FFullySpecifiedType*)Target;
      TS32 x = Parameters.Find( Source );
      if ( x >= 0 ) Parameters[ x ] = Target;
    }

    void FFunction::RemoveChild( FNode *child )
    {
      if ( ReturnType == child ) ReturnType = NULL;
      Parameters.Delete( child );
    }

    TBOOL FFunction::FindVariableModification( CString VarName )
    {
      return false;
    }

    FFunction::~FFunction()
    {
      SAFEDELETE( ReturnType );
      Parameters.FreeArray();
    }

    FJumpStatement::FJumpStatement( EJumpType InType, const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Type( InType ),
      OptionalExpression( nullptr )
    {
    }

    void FJumpStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );

      switch ( Type )
      {
      case EJumpType::Return:
        EXPANDOUTPUT( _T( "return" ) );
        break;

      case EJumpType::Break:
        EXPANDOUTPUT( _T( "break" ) );
        break;

      case EJumpType::Continue:
        EXPANDOUTPUT( _T( "continue" ) );
        break;

      default:
        LOG_ERR( _T( "*MISSING_%d*" ), Type );
        BASEASSERT( 0 );
        break;
      }

      if ( OptionalExpression )
      {
        EXPANDOUTPUT( _T( " " ) );
        OptionalExpression->Dump( Indent, false );
      }
      EXPANDOUTPUT( _T( ";\n" ) );
    }

    void FJumpStatement::BuildChildArray()
    {
      if ( OptionalExpression )
        OptionalExpression->SetParentAndBuildChildArray( this );
    }

    void FJumpStatement::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Source == OptionalExpression && Target->IsExpression() )
        OptionalExpression = (FExpression*)Target;
    }

    void FJumpStatement::RemoveChild( FNode *child )
    {
      if ( child == OptionalExpression )
        OptionalExpression = NULL;
    }

    TBOOL FJumpStatement::FindVariableModification( CString VarName )
    {
      return false;
    }

    FJumpStatement::~FJumpStatement()
    {
      if ( OptionalExpression )
      {
        delete OptionalExpression;
      }
    }

    FSelectionStatement::FSelectionStatement( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Condition( nullptr ),
      ThenStatement( nullptr ),
      ElseStatement( nullptr )
    {
    }

    void FSelectionStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      DumpAttributes();
      EXPANDOUTPUT( _T( "if(" ) );
      Condition->Dump( Indent, false );
      EXPANDOUTPUT( _T( ")\n" ) );
      ThenStatement->Dump( Indent, false );
      if ( ElseStatement )
      {
        DumpIndent( Indent );
        if ( ElseStatement->GetType() == AST::ENodeType::CompoundStatement )
          EXPANDOUTPUT( _T( "else\n" ) );
        else
          EXPANDOUTPUT( _T( "else \n" ) );
        ElseStatement->Dump( Indent, false );
      }
    }

    void FSelectionStatement::BuildChildArray()
    {
      Condition->SetParentAndBuildChildArray( this );
      ThenStatement->SetParentAndBuildChildArray( this );
      if ( ElseStatement ) ElseStatement->SetParentAndBuildChildArray( this );
    }

    void FSelectionStatement::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Condition == Source && Target->IsExpression() ) Condition = (FExpression*)Target;
      if ( ThenStatement == Source ) ThenStatement = Target;
      if ( ElseStatement == Source ) ElseStatement = Target;
    }

    void FSelectionStatement::RemoveChild( FNode *child )
    {
      if ( Condition == child ) Condition = NULL;
      if ( ThenStatement == child ) ThenStatement = NULL;
      if ( ElseStatement == child ) ElseStatement = NULL;
    }

    TBOOL FSelectionStatement::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      r |= Condition->FindVariableModification( VarName );
      r |= ThenStatement->FindVariableModification( VarName );
      if ( ElseStatement ) r |= ElseStatement->FindVariableModification( VarName );
      return r;
    }

    FSelectionStatement::~FSelectionStatement()
    {
      delete Condition;
      delete ThenStatement;
      if ( ElseStatement )
      {
        delete ElseStatement;
      }
    }

    FTypeSpecifier::FTypeSpecifier( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Structure( nullptr ),
      TextureMSNumSamples( 1 ),
      PatchSize( 0 ),
      bIsArray( false ),
      //bIsUnsizedArray(false),
      ArraySize( nullptr )
    {
    }

    void FTypeSpecifier::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      if ( Structure )
      {
        Structure->Dump( Indent );
      }
      else
      {
        EXPANDOUTPUT( CString( TypeName ).GetPointer() );
        if ( TextureMSNumSamples > 1 )
        {
          EXPANDOUTPUT3( _T( "<%s,%d>" ), InnerType.GetPointer(), TextureMSNumSamples );
        }
        else if ( InnerType.Length() )
        {
          EXPANDOUTPUT2( _T( "<%s>" ), InnerType.GetPointer() );
        }
      }

      if ( bIsArray )
      {
        printf( "[ " );

        if ( ArraySize )
        {
          ArraySize->Dump( Indent );
        }

        printf( "]" );
      }
    }

    void FTypeSpecifier::BuildChildArray()
    {
      if ( Structure ) Structure->SetParentAndBuildChildArray( this );
      if ( bIsArray )
        ArraySize->SetParentAndBuildChildArray( this );
    }

    void FTypeSpecifier::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Structure == Source && Target->GetType() == ENodeType::StructSpecifier ) Structure = (FStructSpecifier*)Target;
      if ( ArraySize == Source && Target->IsExpression() ) ArraySize = (FExpression*)Target;
    }

    void FTypeSpecifier::RemoveChild( FNode *child )
    {
      if ( Structure == child ) Structure = NULL;
      if ( ArraySize == child ) ArraySize = NULL;
    }

    TBOOL FTypeSpecifier::FindVariableModification( CString VarName )
    {
      return false;
    }


    FTypeSpecifier::~FTypeSpecifier()
    {
      if ( Structure )
      {
        delete Structure;
      }

      if ( ArraySize )
      {
        delete ArraySize;
      }
    }

    FCBufferDeclaration::FCBufferDeclaration( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Name( "" ),
      Register( "" ),
      Declarations()
    {
    }

    void FCBufferDeclaration::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      EXPANDOUTPUT2( _T( "cbuffer %s\n" ), Name.GetPointer() );

      if ( Register.Length() )
      {
        EXPANDOUTPUT2( _T( ":register(%s)\n" ), Register.GetPointer() );
      }

      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "{\n" ) );

      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
      {
        Declarations[ x ]->Dump( Indent + 1 );
      }

      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "}\n" ) );
    }

    void FCBufferDeclaration::BuildChildArray()
    {
      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
        Declarations[ x ]->SetParentAndBuildChildArray( this );
    }

    void FCBufferDeclaration::ChangeChildTo( FNode *Source, FNode *Target )
    {
      TS32 x = Declarations.Find( Source );
      if ( x >= 0 ) Declarations[ x ] = Target;
    }

    void FCBufferDeclaration::RemoveChild( FNode *child )
    {
      Declarations.Delete( child );
    }

    TBOOL FCBufferDeclaration::FindVariableModification( CString VarName )
    {
      return false;
    }

    FCBufferDeclaration::~FCBufferDeclaration()
    {
      Declarations.FreeArray();
    }

    FTypeQualifier::FTypeQualifier()
    {
      Raw = 0;
    }

    void FTypeQualifier::Dump() const
    {
      if ( bConstant )
      {
        EXPANDOUTPUT( _T( "const " ) );
      }

      if ( bIsStatic )
      {
        EXPANDOUTPUT( _T( "static " ) );
      }

      if ( bIn && bOut )
      {
        EXPANDOUTPUT( _T( "inout " ) );
      }
      else if ( bIn )
      {
        EXPANDOUTPUT( _T( "in " ) );
      }
      else if ( bOut )
      {
        EXPANDOUTPUT( _T( "out " ) );
      }

      //bRowMajor?
      //bShared?
    }

    FFullySpecifiedType::FFullySpecifiedType( const FSourceInfo& innfo ) :
      FNode( innfo ),
      Specifier( nullptr )
    {
    }

    void FFullySpecifiedType::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      if ( Primitive.Length() )
      {
        EXPANDOUTPUT2( _T( "%s " ), Primitive.GetPointer() );
      }
      Qualifier.Dump();
      Specifier->Dump( Indent );
    }

    void FFullySpecifiedType::FindReferences( FNode *Declaration )
    {
      if ( !Declaration ) return;

      if ( Declaration->GetType() == AST::StructSpecifier )
      {
        if ( Specifier && Specifier->TypeName == ( ( AST::FStructSpecifier* )Declaration )->Name )
        {
          Specifier->ReferencedStruct = ( ( AST::FStructSpecifier* )Declaration );
          Declaration->References += Specifier;
        }

        if ( Specifier && Specifier->InnerType == ( ( AST::FStructSpecifier* )Declaration )->Name )
        {
          Specifier->ReferencedInnerType = ( ( AST::FStructSpecifier* )Declaration );
          Declaration->References += Specifier;
        }
      }
    }

    void FFullySpecifiedType::BuildChildArray()
    {
      Specifier->SetParentAndBuildChildArray( this );
    }

    void FFullySpecifiedType::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Specifier == Source && Target->GetType() == ENodeType::TypeSpecifier ) Specifier = (FTypeSpecifier*)Target;
    }

    void FFullySpecifiedType::RemoveChild( FNode *child )
    {
      if ( Specifier == child ) Specifier = NULL;
    }

    TBOOL FFullySpecifiedType::FindVariableModification( CString VarName )
    {
      return false;
    }

    FFullySpecifiedType::~FFullySpecifiedType()
    {
      delete Specifier;
    }

    FDeclaration::FDeclaration( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Identifier( "" ),
      Semantic( "" ),
      Register( "" ),
      bIsArray( false ),
      ArraySize(),
      Initializer( nullptr )
    {
    }

    void FDeclaration::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      //LOG_DBG("Declaration '%s': %d references", Identifier.GetPointer(), References.NumItems());

      DumpAttributes();
      EXPANDOUTPUT2( _T( "%s" ), Identifier.GetPointer() );
      //LOG_DBG("variable: %s", Identifier.GetPointer());

      DumpOptionalArraySize( bIsArray, ArraySize );

      if ( Initializer )
      {
        EXPANDOUTPUT( _T( "=" ) );
        Initializer->Dump( Indent, false );
      }

      if ( Semantic.Length() )
      {
        CString sem = Semantic;
        sem.ToLower();
        EXPANDOUTPUT2( _T( ":%s" ), sem.GetPointer() );
      }

      if ( Register.Length() )
      {
        EXPANDOUTPUT2( _T( ":register(%s)" ), Register.GetPointer() );
      }
    }

    void FDeclaration::BuildChildArray()
    {
      if ( Initializer ) Initializer->SetParentAndBuildChildArray( this );
      if ( bIsArray )
        for ( TS32 x = 0; x < ArraySize.NumItems(); x++ )
          if ( ArraySize[ x ] ) ArraySize[ x ]->SetParentAndBuildChildArray( this );
    }

    void FDeclaration::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( !Target->IsExpression() )
      {
        LOG_ERR( "[AST] Forcing Expression child" );
        return;
      }
      if ( Initializer == Source ) Initializer = (FExpression*)Target;
      TS32 x = ArraySize.Find( (FExpression*)Source );
      if ( x >= 0 ) ArraySize[ x ] = (FExpression*)Target;
    }

    void FDeclaration::RemoveChild( FNode *child )
    {
      if ( Initializer == child ) Initializer = NULL;
      ArraySize.Delete( (FExpression*)child );
    }

    TBOOL FDeclaration::FindVariableModification( CString VarName )
    {
      if ( Identifier == VarName ) return true;
      return false;
    }

    FDeclaration::~FDeclaration()
    {
      ArraySize.FreeArray();

      if ( Initializer )
      {
        delete Initializer;
      }
    }

    FDeclaratorList::FDeclaratorList( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Type( nullptr ),
      Declarations()
    {
    }

    void FDeclaratorList::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      DumpAttributes();
      if ( Type )
      {
        Type->Dump( 0 );
      }

      bool bFirst = true;
      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
      {
        if ( bFirst )
        {
          EXPANDOUTPUT( _T( " " ) );
          bFirst = false;
        }
        else
        {
          EXPANDOUTPUT( _T( "," ) );
        }

        Declarations[ x ]->Dump( 0 );
      }

      EXPANDOUTPUT( _T( ";\n" ) );
    }

    void FDeclaratorList::BuildChildArray()
    {
      if ( Type ) Type->SetParentAndBuildChildArray( this );
      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
        Declarations[ x ]->SetParentAndBuildChildArray( this );
    }

    void FDeclaratorList::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Type == Source && Target->GetType() == ENodeType::FullySpecifiedType ) Type = (FFullySpecifiedType*)Target;
      TS32 x = Declarations.Find( Source );
      if ( x >= 0 ) Declarations[ x ] = Target;
    }

    void FDeclaratorList::RemoveChild( FNode *child )
    {
      if ( Type == child ) Type = NULL;
      Declarations.Delete( child );
    }

    TBOOL FDeclaratorList::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
        r |= Declarations[ x ]->FindVariableModification( VarName );
      return r;
    }

    FDeclaratorList::~FDeclaratorList()
    {
      delete Type;
      Declarations.FreeArray();
    }

    FInitializerListExpression::FInitializerListExpression( const FSourceInfo& InInfo ) :
      FExpression( EOperators::InitializerList, nullptr, nullptr, nullptr, InInfo )
    {
    }

    void FInitializerListExpression::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      EXPANDOUTPUT( _T( "{" ) );
      bool bFirst = true;
      for ( TS32 x = 0; x < Expressions.NumItems(); x++ )
      {
        if ( bFirst )
        {
          bFirst = false;
        }
        else
        {
          EXPANDOUTPUT( _T( "," ) );
        }

        Expressions[ x ]->Dump( 0, false );
      }
      EXPANDOUTPUT( _T( "}" ) );
    }

    TBOOL FInitializerListExpression::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      for ( TS32 x = 0; x < Expressions.NumItems(); x++ )
        r |= Expressions[ x ]->FindVariableModification( VarName );
      return r;
    }

    FParameterDeclarator::FParameterDeclarator( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Type( nullptr ),
      bIsArray( false ),
      ArraySize(),
      DefaultValue( nullptr )
    {
    }

    void FParameterDeclarator::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      //LOG_DBG("Function Parameter '%s': %d references", Identifier.GetPointer(), References.NumItems());
      DumpAttributes();
      Type->Dump( Indent );
      EXPANDOUTPUT2( _T( " %s" ), Identifier.GetPointer() );

      //LOG_DBG("function parameter: %s", Identifier.GetPointer());

      DumpOptionalArraySize( bIsArray, ArraySize );

      if ( Semantic.Length() )
      {
        CString sem = Semantic;
        sem.ToLower();
        EXPANDOUTPUT2( _T( ":%s" ), sem.GetPointer() );
      }

      if ( DefaultValue )
      {
        EXPANDOUTPUT( _T( "=" ) );
        DefaultValue->Dump( Indent );
      }
    }

    void FParameterDeclarator::BuildChildArray()
    {
      Type->SetParentAndBuildChildArray( this );
      for ( TS32 x = 0; x < ArraySize.NumItems(); x++ )
        ArraySize[ x ]->SetParentAndBuildChildArray( this );
      if ( DefaultValue )
        DefaultValue->SetParentAndBuildChildArray( this );
    }

    void FParameterDeclarator::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Type == Source && Target->GetType() == ENodeType::FullySpecifiedType ) Type = (FFullySpecifiedType*)Target;

      TS32 x = ArraySize.Find( (FExpression*)Source );
      if ( x >= 0 && Target->IsExpression() ) ArraySize[ x ] = (FExpression*)Target;
      if ( DefaultValue == Source && Target->IsExpression() ) DefaultValue = (FExpression*)Target;
    }

    void FParameterDeclarator::RemoveChild( FNode *child )
    {
      if ( Type == child ) Type = NULL;
      ArraySize.Delete( (FExpression*)child );
      if ( DefaultValue == child ) DefaultValue = NULL;
    }

    TBOOL FParameterDeclarator::FindVariableModification( CString VarName )
    {
      return false;
    }

    FParameterDeclarator* FParameterDeclarator::CreateFromDeclaratorList( FDeclaratorList* List )
    {
      BASEASSERT( List );
      BASEASSERT( List->Declarations.NumItems() == 1 );

      auto* Source = (FDeclaration*)List->Declarations[ 0 ];
      auto* New = new FParameterDeclarator( Source->SourceInfo );
      New->Type = List->Type;
      New->Identifier = Source->Identifier;
      New->Semantic = Source->Semantic;
      New->bIsArray = Source->bIsArray;
      New->ArraySize = Source->ArraySize;
      New->DefaultValue = Source->Initializer;
      return New;
    }

    FParameterDeclarator::~FParameterDeclarator()
    {
      delete Type;

      ArraySize.FreeArray();

      if ( DefaultValue )
      {
        delete DefaultValue;
      }
    }

    FIterationStatement::FIterationStatement( const FSourceInfo& InInfo, EIterationType InType ) :
      FNode( InInfo ),
      Type( InType ),
      InitStatement( nullptr ),
      Condition( nullptr ),
      RestExpression( nullptr ),
      Body( nullptr )
    {
    }

    void FIterationStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      DumpAttributes();
      switch ( Type )
      {
      case EIterationType::For:
        EXPANDOUTPUT( _T( "for (" ) );
        if ( InitStatement )
        {
          InitStatement->Dump( 0, false );
          //DumpIndent(Indent + 1);
        }
        else
        {
          EXPANDOUTPUT( _T( ";" ) );
        }
        if ( Condition )
        {
          Condition->Dump( 0, false );
        }
        EXPANDOUTPUT( _T( ";" ) );
        if ( RestExpression )
        {
          RestExpression->Dump( 0, false );
        }
        EXPANDOUTPUT( _T( ")\n" ) );
        DumpIndent( Indent );

        if ( !Body ) { EXPANDOUTPUT( _T( ";\n" ) ); }
        else
        {
          if ( Body->GetType() != CompoundStatement ) { EXPANDOUTPUT( _T( "{\n" ) ); }
          Body->Dump( Indent + 1, false );
          DumpIndent( Indent );
          if ( Body->GetType() != CompoundStatement ) { EXPANDOUTPUT( _T( "}\n" ) ); }
        }
        break;

      case EIterationType::While:
        EXPANDOUTPUT( _T( "while(" ) );
        Condition->Dump( 0, false );
        EXPANDOUTPUT( _T( ")\n" ) );
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "{\n" ) );
        if ( Body )
        {
          Body->Dump( Indent + 1, false );
        }
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "}\n" ) );
        break;

      case EIterationType::DoWhile:
        EXPANDOUTPUT( _T( "do\n" ) );
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "{\n" ) );
        if ( Body )
        {
          Body->Dump( Indent + 1, false );
        }
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "}\n" ) );
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "while(" ) );
        Condition->Dump( 0, false );
        EXPANDOUTPUT( _T( ");\n" ) );
        break;

      default:
        BASEASSERT( 0 );
        break;
      }
    }

    void FIterationStatement::BuildChildArray()
    {
      if ( InitStatement ) InitStatement->SetParentAndBuildChildArray( this );
      if ( Condition ) Condition->SetParentAndBuildChildArray( this );
      if ( RestExpression ) RestExpression->SetParentAndBuildChildArray( this );
      if ( Body ) Body->SetParentAndBuildChildArray( this );
    }

    void FIterationStatement::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( InitStatement == Source ) InitStatement = Target;
      if ( Condition == Source ) Condition = Target;
      if ( RestExpression == Source && Target->IsExpression() ) RestExpression = (FExpression*)Target;
      if ( Body == Source ) Body = Target;
    }

    void FIterationStatement::RemoveChild( FNode *child )
    {
      if ( InitStatement == child ) InitStatement = NULL;
      if ( Condition == child ) Condition = NULL;
      if ( RestExpression == child ) RestExpression = NULL;
      if ( Body == child ) Body = NULL;
    }

    TBOOL FIterationStatement::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      if ( InitStatement ) r |= InitStatement->FindVariableModification( VarName );
      if ( Condition ) r |= Condition->FindVariableModification( VarName );
      if ( RestExpression ) r |= RestExpression->FindVariableModification( VarName );
      if ( Body ) r |= Body->FindVariableModification( VarName );
      return r;
    }


    FIterationStatement::~FIterationStatement()
    {
      if ( InitStatement )
      {
        delete InitStatement;
      }
      if ( Condition )
      {
        delete Condition;
      }

      if ( RestExpression )
      {
        delete RestExpression;
      }

      if ( Body )
      {
        delete Body;
      }
    }

    FFunctionExpression::FFunctionExpression( const FSourceInfo& InInfo, FExpression* Callee ) :
      FExpression( EOperators::FunctionCall, Callee, nullptr, nullptr, InInfo )
    {
    }

    void FFunctionExpression::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      SubExpressions[ 0 ]->Dump( 0 );
      EXPANDOUTPUT( _T( "(" ) );
      bool bFirst = true;
      for ( TS32 x = 0; x < Expressions.NumItems(); x++ )
      {
        if ( bFirst )
        {
          bFirst = false;
        }
        else
        {
          EXPANDOUTPUT( _T( "," ) );
        }
        Expressions[ x ]->Dump( 0, false );
      }
      EXPANDOUTPUT( _T( ")" ) );
    }

    TBOOL FFunctionExpression::FindVariableModification( CString VarName )
    {
      if ( SubExpressions[ 0 ]->ReferencedNode ) // function reference
      {
        if ( SubExpressions[ 0 ]->ReferencedNode->GetType() == Function )
        {
          FFunction* function = (FFunction*)SubExpressions[ 0 ]->ReferencedNode;
          for ( int x = 0; x < function->Parameters.NumItems(); x++ )
          {
            FParameterDeclarator* parameter = (FParameterDeclarator*)function->Parameters[ x ];
            if ( parameter->Type->Qualifier.bOut )
            {
              // This here is a simplistic hack for the most common case of a variable simply being put in as an out parameter
              // it's possible that more complicated expressions somehow slip through this!
              // for example if a field is passed to the out parameter like testytest(variable.x)
              // also we may want to verify that the out variable is actually modified by the function and if not we can simply remove the out qualifier...

              if ( Expressions.NumItems() > x && Expressions[ x ]->ReferencedNode && Expressions[ x ]->ReferencedNode->GetType() == Declaration )
              {
                FDeclaration* decl = (FDeclaration*)Expressions[ x ]->ReferencedNode;
                if ( decl->Identifier == VarName )
                  return true;
              }
            }
          }
        }
      }

      TBOOL r = false;
      for ( TS32 x = 0; x < Expressions.NumItems(); x++ )
        r |= Expressions[ x ]->FindVariableModification( VarName );
      return r;
    }

    void FFunctionExpression::GetUniqueIdentifierList( CArray<CString> &List )
    {
      for ( TS32 x = 0; x < Expressions.NumItems(); x++ )
        Expressions[ x ]->GetUniqueIdentifierList( List );
    }

    FSwitchStatement::FSwitchStatement( const FSourceInfo& InInfo, FExpression* InCondition, FSwitchBody* InBody ) :
      FNode( InInfo ),
      Condition( InCondition ),
      Body( InBody )
    {
    }

    void FSwitchStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "switch(" ) );
      Condition->Dump( 0, false );
      EXPANDOUTPUT( _T( ")\n" ) );
      Body->Dump( Indent );
    }

    void FSwitchStatement::BuildChildArray()
    {
      Condition->SetParentAndBuildChildArray( this );
      Body->SetParentAndBuildChildArray( this );
    }

    void FSwitchStatement::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Condition == Source && Target->IsExpression() ) Condition = (FExpression*)Target;
      if ( Body == Source && Target->GetType() == ENodeType::SwitchBody ) Body = (FSwitchBody*)Target;
    }

    void FSwitchStatement::RemoveChild( FNode *child )
    {
      if ( Condition == child ) Condition = NULL;
      if ( Body == child ) Body = NULL;
    }

    TBOOL FSwitchStatement::FindVariableModification( CString VarName )
    {
      return Condition->FindVariableModification( VarName ) | Body->FindVariableModification( VarName );
    }

    FSwitchStatement::~FSwitchStatement()
    {
      delete Condition;
      delete Body;
    }

    FSwitchBody::FSwitchBody( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      CaseList( nullptr )
    {
    }

    void FSwitchBody::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "{\n" ) );
      CaseList->Dump( Indent + 1 );
      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "}\n" ) );
    }

    void FSwitchBody::BuildChildArray()
    {
      CaseList->SetParentAndBuildChildArray( this );
    }

    void FSwitchBody::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( CaseList == Source && Target->GetType() == ENodeType::CaseStatementList ) CaseList = (FCaseStatementList*)Target;
    }

    void FSwitchBody::RemoveChild( FNode *child )
    {
      if ( CaseList == child ) CaseList = NULL;
    }

    TBOOL FSwitchBody::FindVariableModification( CString VarName )
    {
      return CaseList->FindVariableModification( VarName );
    }

    FSwitchBody::~FSwitchBody()
    {
      delete CaseList;
    }

    FCaseLabel::FCaseLabel( const FSourceInfo& InInfo, AST::FExpression* InExpression ) :
      FNode( InInfo ),
      TestExpression( InExpression )
    {
    }

    void FCaseLabel::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      if ( TestExpression )
      {
        EXPANDOUTPUT( _T( "case " ) );
        TestExpression->Dump( 0 );
      }
      else
      {
        EXPANDOUTPUT( _T( "default" ) );
      }

      EXPANDOUTPUT( _T( ":\n" ) );
    }

    void FCaseLabel::BuildChildArray()
    {
      if ( TestExpression ) TestExpression->SetParentAndBuildChildArray( this );
    }

    void FCaseLabel::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( TestExpression == Source && Target->IsExpression() ) TestExpression = (FExpression*)Target;
    }

    void FCaseLabel::RemoveChild( FNode *child )
    {
      if ( TestExpression == child ) TestExpression = NULL;
    }

    TBOOL FCaseLabel::FindVariableModification( CString VarName )
    {
      return false;
    }

    FCaseLabel::~FCaseLabel()
    {
      if ( TestExpression )
      {
        delete TestExpression;
      }
    }


    FCaseStatement::FCaseStatement( const FSourceInfo& InInfo, FCaseLabelList* InLabels ) :
      FNode( InInfo ),
      Labels( InLabels ),
      Statements()
    {
    }

    void FCaseStatement::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      Labels->Dump( Indent );

      if ( Statements.NumItems() > 1 )
      {
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "{\n" ) );
        for ( TS32 x = 0; x < Statements.NumItems(); x++ )
        {
          Statements[ x ]->Dump( Indent + 1, false );
        }
        DumpIndent( Indent );
        EXPANDOUTPUT( _T( "}\n" ) );
      }
      else if ( Statements.NumItems() > 0 )
      {
        Statements[ 0 ]->Dump( Indent + 1 );
      }
    }

    void FCaseStatement::BuildChildArray()
    {
      Labels->SetParentAndBuildChildArray( this );
      for ( TS32 x = 0; x < Statements.NumItems(); x++ )
        Statements[ x ]->SetParentAndBuildChildArray( this );
    }

    void FCaseStatement::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( Labels == Source && Target->GetType() == ENodeType::CaseLabelList ) Labels = (FCaseLabelList*)Target;
      TS32 x = Statements.Find( Source );
      if ( x >= 0 ) Statements[ x ] = Target;
    }

    void FCaseStatement::RemoveChild( FNode *child )
    {
      if ( Labels == child ) Labels = NULL;
      Statements.Delete( child );
    }

    TBOOL FCaseStatement::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      for ( TS32 x = 0; x < Statements.NumItems(); x++ )
        r |= Statements[ x ]->FindVariableModification( VarName );
      return r;
    }

    FCaseStatement::~FCaseStatement()
    {
      delete Labels;
      Statements.FreeArray();
    }

    FCaseLabelList::FCaseLabelList( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Labels()
    {
    }

    void FCaseLabelList::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      for ( TS32 x = 0; x < Labels.NumItems(); x++ )
      {
        Labels[ x ]->Dump( Indent );
      }
    }

    void FCaseLabelList::BuildChildArray()
    {
      for ( TS32 x = 0; x < Labels.NumItems(); x++ )
        Labels[ x ]->SetParentAndBuildChildArray( this );
    }

    void FCaseLabelList::ChangeChildTo( FNode *Source, FNode *Target )
    {
      TS32 x = Labels.Find( (FCaseLabel*)Source );
      if ( x >= 0 && Target->GetType() == ENodeType::CaseLabel ) Labels[ x ] = (FCaseLabel*)Target;
    }

    void FCaseLabelList::RemoveChild( FNode *child )
    {
      Labels.Delete( (FCaseLabel*)child );
    }

    TBOOL FCaseLabelList::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      for ( TS32 x = 0; x < Labels.NumItems(); x++ )
        r |= Labels[ x ]->FindVariableModification( VarName );
      return r;
    }

    FCaseLabelList::~FCaseLabelList()
    {
      Labels.FreeArray();
    }

    FCaseStatementList::FCaseStatementList( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Cases()
    {
    }

    void FCaseStatementList::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      for ( TS32 x = 0; x < Cases.NumItems(); x++ )
      {
        Cases[ x ]->Dump( Indent );
      }
    }

    void FCaseStatementList::BuildChildArray()
    {
      for ( TS32 x = 0; x < Cases.NumItems(); x++ )
        Cases[ x ]->SetParentAndBuildChildArray( this );
    }

    void FCaseStatementList::ChangeChildTo( FNode *Source, FNode *Target )
    {
      TS32 x = Cases.Find( (FCaseStatement*)Source );
      if ( x >= 0 && Target->GetType() == ENodeType::CaseStatement ) Cases[ x ] = (FCaseStatement*)Target;
    }

    void FCaseStatementList::RemoveChild( FNode *child )
    {
      Cases.Delete( (FCaseStatement*)child );
    }

    TBOOL FCaseStatementList::FindVariableModification( CString VarName )
    {
      TBOOL r = false;
      for ( TS32 x = 0; x < Cases.NumItems(); x++ )
        r |= Cases[ x ]->FindVariableModification( VarName );
      return r;
    }

    FCaseStatementList::~FCaseStatementList()
    {
      Cases.FreeArray();
    }

    FStructSpecifier::FStructSpecifier( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      Name( "" ),
      ParentName( "" ),
      Declarations()
    {
    }

    void FStructSpecifier::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      DumpIndent( Indent );
      EXPANDOUTPUT2( _T( "struct %s" ), Name.GetPointer() );
      if ( ParentName.Length() )
      {
        EXPANDOUTPUT2( _T( ":%s" ), ParentName.GetPointer() );
      }
      EXPANDOUTPUT( _T( "\n" ) );

      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "{\n" ) );

      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
      {
        Declarations[ x ]->Dump( Indent + 1 );
      }

      DumpIndent( Indent );
      EXPANDOUTPUT( _T( "}" ) );
    }

    void FStructSpecifier::BuildChildArray()
    {
      for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
        Declarations[ x ]->SetParentAndBuildChildArray( this );
    }

    void FStructSpecifier::ChangeChildTo( FNode *Source, FNode *Target )
    {
      TS32 x = Declarations.Find( Source );
      if ( x >= 0 ) Declarations[ x ] = Target;
    }

    void FStructSpecifier::RemoveChild( FNode *child )
    {
      Declarations.Delete( child );
    }

    TBOOL FStructSpecifier::FindVariableModification( CString VarName )
    {
      return false;
    }

    FStructSpecifier::~FStructSpecifier()
    {
      Declarations.FreeArray();
    }

    void FStructSpecifier::RenameWithReferences( CString &NewName )
    {
      Name = NewName;
      for ( TS32 x = 0; x < References.NumItems(); x++ )
        if ( References[ x ]->GetType() == ENodeType::TypeSpecifier )
        {
          auto *typ = ( ( AST::FTypeSpecifier* )References[ x ] );
          if ( typ->ReferencedStruct == this ) typ->TypeName = NewName;
          if ( typ->ReferencedInnerType == this ) typ->InnerType = NewName;
        }
    }

    FAttribute::FAttribute( const FSourceInfo& InInfo, const CString &InName ) :
      FNode( InInfo ),
      Name( InName ),
      Arguments()
    {
    }

    void FAttribute::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      EXPANDOUTPUT2( _T( "[%s" ), Name.GetPointer() );

      bool bFirst = true;
      for ( TS32 x = 0; x < Arguments.NumItems(); x++ )
      {
        if ( bFirst )
        {
          EXPANDOUTPUT( _T( "(" ) );
          bFirst = false;
        }
        else
        {
          EXPANDOUTPUT( _T( "," ) );
        }

        Arguments[ x ]->Dump( 0 );
      }

      if ( !bFirst )
      {
        EXPANDOUTPUT( _T( ")" ) );
      }

      EXPANDOUTPUT( _T( "]" ) );
    }

    void FAttribute::BuildChildArray()
    {
      for ( TS32 x = 0; x < Arguments.NumItems(); x++ )
        Arguments[ x ]->SetParentAndBuildChildArray( this );
    }

    void FAttribute::ChangeChildTo( FNode *Source, FNode *Target )
    {
      TS32 x = Arguments.Find( (FAttributeArgument*)Source );
      if ( x >= 0 && Target->GetType() == ENodeType::AttributeArgument ) Arguments[ x ] = (FAttributeArgument*)Target;
    }

    void FAttribute::RemoveChild( FNode *child )
    {
      Arguments.Delete( (FAttributeArgument*)child );
    }

    TBOOL FAttribute::FindVariableModification( CString VarName )
    {
      return false;
    }

    FAttribute::~FAttribute()
    {
      Arguments.FreeArray();
    }

    FAttributeArgument::FAttributeArgument( const FSourceInfo& InInfo ) :
      FNode( InInfo ),
      StringArgument( "" ),
      ExpressionArgument( nullptr )
    {
    }

    void FAttributeArgument::Dump( TS32 Indent, TBOOL NeedsParenthesis ) const
    {
      if ( ExpressionArgument )
      {
        ExpressionArgument->Dump( 0 );
      }
      else
      {
        EXPANDOUTPUT2( _T( "\"%s\"" ), StringArgument.GetPointer() );
      }
    }

    void FAttributeArgument::BuildChildArray()
    {
      if ( ExpressionArgument )
        ExpressionArgument->SetParentAndBuildChildArray( this );
    }

    void FAttributeArgument::ChangeChildTo( FNode *Source, FNode *Target )
    {
      if ( ExpressionArgument == Source && Target->IsExpression() ) ExpressionArgument = (FExpression*)Target;
    }

    void FAttributeArgument::RemoveChild( FNode *child )
    {
      if ( ExpressionArgument == child ) ExpressionArgument = NULL;
    }

    TBOOL FAttributeArgument::FindVariableModification( CString VarName )
    {
      return false;
    }

    FAttributeArgument::~FAttributeArgument()
    {
      if ( ExpressionArgument )
      {
        delete ExpressionArgument;
      }
    }

    void FMacro::Dump( TS32 Indent, TBOOL NeedsParenthesis /*= true*/ ) const
    {
      if ( Output.Length() && Output[ Output.Length() - 1 ] != '\n' )
      {
        Output.Append( _T( "\n" ) );
      }

      Output.Append( MacroString ); //macro strings have \n on the end by default
    }

    FMacro::FMacro( const FSourceInfo& InInfo, CString InStr )
      : FNode( InInfo )
    {
      MacroString = InStr;
      auto& result = InStr.ExplodeByWhiteSpace();
      if ( result.NumItems() >= 2 )
        macroNames.AddUnique(result[ 1 ]);
    }

    FMacro::~FMacro()
    {

    }

  }
}

