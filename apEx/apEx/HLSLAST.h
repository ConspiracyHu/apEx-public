#pragma once
#include "HlslLexer.h"

#define USE_UNREAL_ALLOCATOR 0

namespace CrossCompiler
{
  extern CString Output;
  extern TBOOL PrettyPrint;
  extern TBOOL RenameIdentifiers;

  namespace AST
  {
    enum ENodeType
    {
      Node,
      Expression,
      UnaryExpression,
      BinaryExpression,
      FunctionExpression,
      InitializerListExpression,
      CompoundStatement,
      Declaration,
      TypeQualifier,
      StructSpecifier,
      CBufferDeclaration, //10
      TypeSpecifier,
      FullySpecifiedType,
      DeclaratorList, //13
      Function,
      ExpressionStatement,
      CaseLabel,
      CaseLabelList,
      ParameterDeclarator,
      CaseStatement,
      CaseStatementList,
      SwitchBody,
      SelectionStatement,
      SwitchStatement,
      IterationStatement,
      JumpStatement,
      FunctionDefinition, //26
      AttributeArgument,
      Attribute,
      Macro, //29
      ShaderDocument
    };

    struct FFullySpecifiedType;

    class FNode
    {
    public:
      FSourceInfo SourceInfo;
      FNode* duplicateOf = nullptr;
      CArray<FNode*> duplicates;
      int subTreeSize = 0;
      bool requiredForCommonHeader = false;

      int CountSubTreeSize()
      {
        subTreeSize = 1;

        for ( int x = 0; x < Children.NumItems(); x++ )
          subTreeSize += Children[ x ]->CountSubTreeSize();

        return subTreeSize;
      }

      virtual bool CompareTo( FNode* node );

      void SetDuplicateOf( FNode* original )
      {
        duplicateOf = original;
        original->duplicates.AddUnique( this );

        for ( int x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->SetDuplicateOf( original->Children[ x ] );
      }

      void PointReferencesToOriginal()
      {
        if ( !duplicateOf )
        {
          LOG_ERR( "Inconsistency when repointing references from duplicate to original tree" );
          return;
        }

        CArray<FNode*> refCopy = References;

        for ( int x = 0; x < refCopy.NumItems(); x++ )
        {
          refCopy[ x ]->ReparentReferencesFromAToB( this, duplicateOf );
        }
        for ( int x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->PointReferencesToOriginal();
      }

      virtual void MarkReferencedGlobalsAsRequired()
      {
        for ( int x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->MarkReferencedGlobalsAsRequired();
      }

      void MarkAsRequired()
      {
        requiredForCommonHeader = true;
        if ( Parent )
          Parent->MarkAsRequired();
      }

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
      }

      virtual void ClearDuplicateInfo()
      {
        duplicateOf = nullptr;
        for ( int x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->ClearDuplicateInfo();
      }

      virtual FFullySpecifiedType* GetReferencedType()
      {
        return nullptr;
      }

      //TLinearArray<struct FAttribute*> Attributes;
      CArray<struct FAttribute*> Attributes;

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const = 0;
      virtual ENodeType GetType() {
        return Node;
      }
      //virtual void DumpShort(CString &out) const = 0;

      virtual ~FNode()
      {
        DetachFromParent();
        Children.FreeArray();
      }

      virtual TBOOL FindVariableModification( CString VarName ) = 0;

      virtual void BackupAndClearIdentifier( TBOOL Forced = false ) { }
      virtual void RestoreIdentifier( TBOOL Forced = false ) { }
      virtual void BuildChildArray() = 0;
      virtual void ChangeChildTo( FNode *Source, FNode *Target ) = 0;
      virtual void RemoveChild( FNode *child ) = 0;
      virtual void ReplaceInParentWith( FNode *Node )
      {
        if ( !Parent ) return;

        Node->DetachFromParent();
        Node->Parent = Parent;

        TS32 idx = Parent->Children.Find( this );
        Parent->Children[ idx ] = Node;
        Parent->ChangeChildTo( this, Node );
      }

      void SetParentAndBuildChildArray( FNode *n )
      {
        if ( Parent )
        {
          LOG_ERR( "[AST] Parent Override!" );
        }
        Parent = n;
        Parent->Children += this;
        BuildChildArray();
      }
      void DetachFromParent()
      {
        if ( Parent )
        {
          Parent->RemoveChild( this );
          Parent->Children.Delete( this );
        }
        Parent = NULL;
      }

      void RecursiveBackupAndClearIdentifiers()
      {
        BackupAndClearIdentifier();
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->RecursiveBackupAndClearIdentifiers();
      }

      void RecursiveRestoreIdentifiers()
      {
        RestoreIdentifier();
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->RecursiveRestoreIdentifiers();
      }

      FNode *Parent = NULL;
      CArray<FNode*> Children;
      CArray<FNode*> References; //list of nodes referencing this node

      virtual void FindReferences( FNode *Declaration )
      {
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->FindReferences( Declaration );
      }

      virtual TBOOL IsExpression() { return false; }

      virtual TS32 DeclarationCount() { return 0; }

      TS32 CountDeclarations()
      {
        TS32 cnt = DeclarationCount();
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          cnt += Children[ x ]->CountDeclarations();
        return cnt;
      }

      virtual TBOOL CanBeReferenced() { return false; }

      void GatherReferencedNodes( CArray<FNode*> &Nodes )
      {
        if ( CanBeReferenced() ) Nodes.AddUnique( this );
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->GatherReferencedNodes( Nodes );
      }

      virtual void RenameWithReferences( CString &NewName )
      {

      }

      TBOOL FindNodeInChildTree( FNode *Node )
      {
        if ( this == Node ) return true;
        TBOOL Found = false;
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Found |= Children[ x ]->FindNodeInChildTree( Node );
        return Found;
      }

      void RebuildChildren()
      {
        Children.FlushFast();
        BuildChildArray();
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->RebuildChildren();
      }

    protected:
      //FNode();
      FNode( const FSourceInfo& InInfo );

      static void DumpIndent( TS32 Indent );

      void DumpAttributes() const;
    };

    struct FShaderDocument : FNode
    {
      virtual void Dump( TS32 Indent = 0, TBOOL Parentheses = 0 ) const
      {
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->Dump( 0 );
      }

      virtual TBOOL FindVariableModification( CString VarName )
      {
        TBOOL result = false;
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          result |= Children[ x ]->FindVariableModification( VarName );
        return result;
      }

      virtual void BuildChildArray()
      {
        for ( TS32 x = 0; x < Children.NumItems(); x++ )
          Children[ x ]->BuildChildArray();
      }
      virtual void ChangeChildTo( FNode *Source, FNode *Target ) { }
      virtual void RemoveChild( FNode *child ) { }

      FShaderDocument( const FSourceInfo& InInfo ) : FNode( InInfo ) {}

      virtual ENodeType GetType() 
      {
        return ShaderDocument;
      }
    };

    /**
    * Operators for AST expression nodes.
    */
    enum class EOperators
    {
      Assign,
      Plus,        /**< Unary + operator. */
      Neg,
      Add,
      Sub,
      Mul,
      Div,
      Mod,
      LShift,
      RShift,
      Less,
      Greater,
      LEqual,
      GEqual,
      Equal,
      NEqual,
      BitAnd,
      BitXor,
      BitOr,
      BitNot,
      LogicAnd,
      LogicXor,
      LogicOr,
      LogicNot,

      MulAssign,
      DivAssign,
      ModAssign,
      AddAssign,
      SubAssign,
      LSAssign,
      RSAssign,
      AndAssign,
      XorAssign,
      OrAssign,

      Conditional,

      PreInc,
      PreDec,
      PostInc,
      PostDec,
      FieldSelection,
      ArrayIndex,

      FunctionCall,
      InitializerList,

      Identifier,
      //Int_constant,
      UintConstant,
      FloatConstant,
      BoolConstant,

      //Sequence,

      TypeCast,
    };

    //TBOOL GetOperatorAssociativity(EOperators Op);

    inline EOperators TokenToASTOperator( EHlslToken Token )
    {
      switch ( Token )
      {
      case EHlslToken::Equal:
        return AST::EOperators::Assign;

      case EHlslToken::PlusEqual:
        return AST::EOperators::AddAssign;

      case EHlslToken::MinusEqual:
        return AST::EOperators::SubAssign;

      case EHlslToken::TimesEqual:
        return AST::EOperators::MulAssign;

      case EHlslToken::DivEqual:
        return AST::EOperators::DivAssign;

      case EHlslToken::ModEqual:
        return AST::EOperators::ModAssign;

      case EHlslToken::GreaterGreaterEqual:
        return AST::EOperators::RSAssign;

      case EHlslToken::LowerLowerEqual:
        return AST::EOperators::LSAssign;

      case EHlslToken::AndEqual:
        return AST::EOperators::AndAssign;

      case EHlslToken::OrEqual:
        return AST::EOperators::OrAssign;

      case EHlslToken::XorEqual:
        return AST::EOperators::XorAssign;

      case EHlslToken::Question:
        return AST::EOperators::Conditional;

      case EHlslToken::OrOr:
        return AST::EOperators::LogicOr;

      case EHlslToken::AndAnd:
        return AST::EOperators::LogicAnd;

      case EHlslToken::Or:
        return AST::EOperators::BitOr;

      case EHlslToken::Xor:
        return AST::EOperators::BitXor;

      case EHlslToken::And:
        return AST::EOperators::BitAnd;

      case EHlslToken::EqualEqual:
        return AST::EOperators::Equal;

      case EHlslToken::NotEqual:
        return AST::EOperators::NEqual;

      case EHlslToken::Lower:
        return AST::EOperators::Less;

      case EHlslToken::Greater:
        return AST::EOperators::Greater;

      case EHlslToken::LowerEqual:
        return AST::EOperators::LEqual;

      case EHlslToken::GreaterEqual:
        return AST::EOperators::GEqual;

      case EHlslToken::LowerLower:
        return AST::EOperators::LShift;

      case EHlslToken::GreaterGreater:
        return AST::EOperators::RShift;

      case EHlslToken::Plus:
        return AST::EOperators::Add;

      case EHlslToken::Minus:
        return AST::EOperators::Sub;

      case EHlslToken::Times:
        return AST::EOperators::Mul;

      case EHlslToken::Div:
        return AST::EOperators::Div;

      case EHlslToken::Mod:
        return AST::EOperators::Mod;

      default:
        BASEASSERT( 0 );
        break;
      }

      return AST::EOperators::Plus;
    }

    struct FExpression : public FNode
    {
      FExpression( EOperators InOperator, FExpression* E0, FExpression* E1, FExpression* E2, const FSourceInfo& InInfo );
      ~FExpression();

      virtual bool CompareTo( FNode* node );

      EOperators Operator = EOperators::Assign;
      FExpression* SubExpressions[ 3 ];

      CString OriginalFloatString;

      TBOOL ForceParenthesis = false; //used for moved variable initializations

      union
      {
        TU32 UintConstant;
        float FloatConstant;
        bool BoolConstant;
      };

      CString Identifier;
      struct FTypeSpecifier* TypeSpecifier = NULL;

      //TLinearArray<FExpression*> Expressions;
      CArray<FExpression*> Expressions;

      void DumpOperator() const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return Expression;
      }
      virtual TBOOL IsExpression() { return true; }

      CString BackedupIdentifier;
      virtual void BackupAndClearIdentifier( TBOOL Forced )
      {
        if ( !Forced ) return;
        BackedupIdentifier = Identifier; Identifier = "";
      }
      virtual void RestoreIdentifier( TBOOL Forced )
      {
        if ( !Forced ) return;
        Identifier = BackedupIdentifier;
      }

      FNode *ReferencedNode = NULL; //node referenced by the identifier

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
        if ( ReferencedNode == original )
        {
          if ( ReferencedNode )
            ReferencedNode->References.Delete( this );
          ReferencedNode = target;
          if ( target )
            target->References.Add( this );
        }
      }

      virtual void MarkReferencedGlobalsAsRequired()
      {
        if ( ReferencedNode )
          ReferencedNode->MarkAsRequired();

        FNode::MarkReferencedGlobalsAsRequired();
      }

      //virtual TS32 GetChildCount() { TS32 cnt = 0; if (SubExpressions[0]) cnt++; if (SubExpressions[1]) cnt++; if (SubExpressions[2]) cnt++; return cnt; }
      //virtual FNode *GetChildByIndex(TS32 idx) { return SubExpressions[idx]; }
      virtual void GetUniqueIdentifierList( CArray<CString> &List );

      virtual void FindReferences( FNode *Declaration );

      struct FStructSpecifier *GetStruct();

    };

    struct FUnaryExpression : public FExpression
    {
      FUnaryExpression( EOperators InOperator, FExpression* Expr, const FSourceInfo& InInfo );

      bool CompareTo( FNode* node )
      {
        //LOG_WARN( "FUnaryExpression::CompareTo enter" );
        bool result = FExpression::CompareTo( node );
        //LOG_WARN( "FUnaryExpression::CompareTo exit" );
        return result;
      }

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return UnaryExpression;
      }
    };

    struct FBinaryExpression : public FExpression
    {
      FBinaryExpression( EOperators InOperator, FExpression* E0, FExpression* E1, const FSourceInfo& InInfo );

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return BinaryExpression;
      }
    };

    struct FFunctionExpression : public FExpression
    {
      FFunctionExpression( const FSourceInfo& InInfo, FExpression* Callee );

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return FunctionExpression;
      }
      void GetUniqueIdentifierList( CArray<CString> &List );
    };

    struct FInitializerListExpression : public FExpression
    {
      FInitializerListExpression( const FSourceInfo& InInfo );

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return InitializerListExpression;
      }
    };

    struct FCompoundStatement : public FNode
    {
      //virtual bool CompareTo( FNode* node )
      //{
      //  if ( !FNode::CompareTo( node ) )
      //    return false;

      //  FCompoundStatement* other = (FCompoundStatement*)node;

      //  LOG_WARN( "CompareTo not implemented for FCompoundStatement!" );
      //  return false;
      //}

      FCompoundStatement( const FSourceInfo& InInfo );
      ~FCompoundStatement();

      //TLinearArray<FNode*> Statements;
      CArray<FNode*> Statements;

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return CompoundStatement;
      }
      //virtual TS32 GetChildCount() { return Statements.NumItems(); }
      //virtual FNode *GetChildByIndex(TS32 idx) { return Statements[idx]; }
      //virtual void DeleteChild(FNode *ch) { Statements.Delete(ch); }
    };

    struct FDeclaration : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FDeclaration* other = (FDeclaration*)node;

        if ( Semantic != other->Semantic )
          return false;
        if ( Register != other->Register )
          return false;
        if ( bIsArray != other->bIsArray )
          return false;

        //LOG_WARN( "CompareTo not implemented for FDeclaration!" );
        return true;
      }

      FDeclaration( const FSourceInfo& InInfo );
      ~FDeclaration();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return Declaration;
      }
      virtual TBOOL CanBeReferenced() { return true; }

      CString Identifier;

      CString Semantic;
      CString Register;

      bool bIsArray = false;

      TBOOL CBufferMember = false;
      TBOOL StructMember = false;
      FStructSpecifier *ParentStruct = NULL;
      //bool bIsUnsizedArray;

      //TLinearArray<FExpression*> ArraySize;
      CArray<FExpression*> ArraySize;

      FExpression* Initializer = nullptr;
      struct FFullySpecifiedType *Type = NULL;

      virtual FFullySpecifiedType* GetReferencedType()
      {
        return Type;
      }

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
        if ( (FNode*)Type == original )
        {
          if ( Type )
            ((FNode*)Type)->References.Delete( this );
          Type = (FFullySpecifiedType*)target;
          if ( target )
            target->References.Add( this );
        }
      }

      CString BackedupIdentifier;
      virtual void BackupAndClearIdentifier( TBOOL Forced )
      {
        BackedupIdentifier = Identifier; Identifier = "";
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->BackupAndClearIdentifier( true );
      }
      virtual void RestoreIdentifier( TBOOL Forced )
      {
        Identifier = BackedupIdentifier;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->RestoreIdentifier( true );
      }

      virtual TS32 DeclarationCount() { return 1; }

      virtual void RenameWithReferences( CString &NewName )
      {
        Identifier = NewName;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
        {
          if ( References[ x ]->IsExpression() )
          {
            auto *e = (FExpression*)References[ x ];
            e->Identifier = NewName;
          }
          else
          {
            LOG_ERR( "[AST] Non expression reference found!" );
          }
        }
      }

      //virtual TS32 GetChildCount() { if (Initializer) return 1 + ArraySize.NumItems(); return ArraySize.NumItems(); }
      //virtual FNode *GetChildByIndex(TS32 idx) { if (Initializer && !idx) return Initializer; if (Initializer) return ArraySize[idx - 1]; return ArraySize[idx]; }
    };

    struct FTypeQualifier
    {
      union
      {
        struct
        {
          TU32 bIsStatic : 1;
          TU32 bConstant : 1;
          TU32 bIn : 1;
          TU32 bOut : 1;
          TU32 bRowMajor : 1;
          TU32 bShared : 1;
        };
        TU32 Raw;
      };

      FTypeQualifier();

      void Dump() const;
      virtual ENodeType GetType() {
        return TypeQualifier;
      }
    };

    struct FStructSpecifier : public FNode
    {
      //virtual bool CompareTo( FNode* node )
      //{
      //  if ( !FNode::CompareTo( node ) )
      //    return false;

      //  FStructSpecifier* other = (FStructSpecifier*)node;

      //  LOG_WARN( "CompareTo not implemented for FStructSpecifier!" );
      //  return false;
      //}

      FStructSpecifier( const FSourceInfo& InInfo );
      ~FStructSpecifier();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return StructSpecifier;
      }

      virtual TBOOL CanBeReferenced() { return true; }

      CString Name;
      CString ParentName;
      //TLinearArray<FNode*> Declarations;
      CArray<FNode*> Declarations;
      //virtual TS32 GetChildCount() { return Declarations.NumItems(); }
      //virtual FNode *GetChildByIndex(TS32 idx) { return Declarations[idx]; }
      //virtual void DeleteChild(FNode *ch) { Declarations.Delete(ch); }

      CString BackedupIdentifier;
      virtual void BackupAndClearIdentifier( TBOOL Forced )
      {
        BackedupIdentifier = Name; Name = "";
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->BackupAndClearIdentifier( true );
      }
      virtual void RestoreIdentifier( TBOOL Forced )
      {
        Name = BackedupIdentifier;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->RestoreIdentifier( true );
      }

      virtual void RenameWithReferences( CString &NewName );

    };

    struct FCBufferDeclaration : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FCBufferDeclaration* other = (FCBufferDeclaration*)node;

        if ( Register != other->Register )
          return false;

        //if ( Declarations.NumItems() != other->Declarations.NumItems() )
        //  return false;

        //for ( TS32 x = 0; x < Declarations.NumItems(); x++ )
        //  if ( !Declarations[ x ]->CompareTo( other->Declarations[ x ] ) )
        //    return false;

        //LOG_WARN( "CompareTo not implemented for FBufferDeclaration!" );
        return true;
      }

      FCBufferDeclaration( const FSourceInfo& InInfo );
      ~FCBufferDeclaration();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return CBufferDeclaration;
      }

      CString Name;
      CString Register;
      //TLinearArray<FNode*> Declarations;
      CArray<FNode*> Declarations;
      //virtual TS32 GetChildCount() { return Declarations.NumItems(); }
      //virtual FNode *GetChildByIndex(TS32 idx) { return Declarations[idx]; }
      //virtual void DeleteChild(FNode *ch) { Declarations.Delete(ch); }
    };

    struct FTypeSpecifier : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FTypeSpecifier* other = (FTypeSpecifier*)node;

        if ( InnerType != other->InnerType )
        {
          LOG_WARN( "InnerType mismatch: %s %s", other->InnerType.GetPointer(), InnerType.GetPointer() );
        }

        if ( TextureMSNumSamples != other->TextureMSNumSamples )
          return false;
        if ( PatchSize != other->PatchSize )
          return false;
        if ( bIsArray != other->bIsArray )
          return false;

        if ( ( Structure == nullptr ) != ( other->Structure == nullptr ) )
          return false;
        if ( ( ReferencedStruct == nullptr ) != ( other->ReferencedStruct == nullptr ) )
          return false;
        if ( ( ReferencedInnerType == nullptr ) != ( other->ReferencedInnerType == nullptr ) )
          return false;

        if ( Structure && !Structure->CompareTo( other->Structure ) )
          return false;
        if ( ReferencedStruct && !ReferencedStruct->CompareTo( other->ReferencedStruct ) )
          return false;
        if ( ReferencedInnerType && !ReferencedInnerType->CompareTo( other->ReferencedInnerType ) )
          return false;
        
        if ( !ReferencedStruct && !ReferencedInnerType )
        {
          if ( TypeName != other->TypeName )
            return false;
          if ( InnerType != other->InnerType )
            return false;
        }

        //LOG_WARN( "CompareTo not implemented for FTypeSpecifier!" );
        return true;
      }

      FTypeSpecifier( const FSourceInfo& InInfo );
      ~FTypeSpecifier();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return TypeSpecifier;
      }

      CString TypeName;
      CString InnerType;

      FStructSpecifier* Structure = nullptr;
      FStructSpecifier* ReferencedStruct = NULL;
      FStructSpecifier* ReferencedInnerType = NULL;

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
        if ( (FNode*)ReferencedStruct == original )
        {
          if ( ReferencedStruct )
            ReferencedStruct->References.Delete( this );
          ReferencedStruct = (FStructSpecifier*)target;
          if ( target )
            target->References.Add( this );
        }
        if ( (FNode*)ReferencedInnerType == original )
        {
          if ( ReferencedInnerType )
            ReferencedInnerType->References.Delete( this );
          ReferencedInnerType = (FStructSpecifier*)target;
          if ( target )
            target->References.Add( this );
        }
      }

      TS32 TextureMSNumSamples = 0;

      TS32 PatchSize = 0;

      bool bIsArray = false;
      //bool bIsUnsizedArray;
      FExpression* ArraySize = nullptr;

      CString BackedupIdentifier;
      CString BackedupInnerType;
      TBOOL Restored = true;

      virtual void BackupAndClearIdentifier( TBOOL Forced )
      {
        if ( !Restored ) return;
        if ( !Forced ) return;
        Restored = false;
        BackedupIdentifier = TypeName; TypeName = "";
        BackedupInnerType = InnerType; InnerType = "";
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->BackupAndClearIdentifier( true );
      }
      virtual void RestoreIdentifier( TBOOL Forced )
      {
        if ( Restored ) return;
        if ( !Forced ) return;
        Restored = true;
        TypeName = BackedupIdentifier;
        InnerType = BackedupInnerType;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->RestoreIdentifier( true );
      }

      //virtual TS32 GetChildCount() { if (Structure) return 1; return 0; }
      //virtual FNode *GetChildByIndex(TS32 idx) { return Structure; }
    };

    struct FFullySpecifiedType : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FFullySpecifiedType* other = (FFullySpecifiedType*)node;

        if ( Primitive != other->Primitive )
          return false;

        if ( Qualifier.Raw != other->Qualifier.Raw )
          return false;

        if ( ( Specifier == nullptr ) != ( other->Specifier == nullptr ) )
          return false;

        if ( !Specifier )
          return true;

        return Specifier->CompareTo( other->Specifier );
      }

      FFullySpecifiedType( const FSourceInfo& InInfo );
      ~FFullySpecifiedType();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return FullySpecifiedType;
      }

      CString Primitive;
      FTypeQualifier Qualifier;
      FTypeSpecifier* Specifier = nullptr;

      virtual void FindReferences( FNode *Declaration );

      virtual FFullySpecifiedType* GetReferencedType()
      {
        return this;
      }

      //virtual TS32 GetChildCount() { return Specifier?1:0; }
      //virtual FNode *GetChildByIndex(TS32 idx) { return Specifier; }
    };

    struct FDeclaratorList : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FDeclaratorList* other = (FDeclaratorList*)node;

        if ( ( Type == nullptr ) != ( other->Type == nullptr ) )
          return false;

        if ( Type && !Type->CompareTo( other->Type ) )
          return false;
        
        //if ( other->Declarations.NumItems() != Declarations.NumItems() )
        //  return false;

        return true;
      }

      FDeclaratorList( const FSourceInfo& InInfo );
      ~FDeclaratorList();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return DeclaratorList;
      }

      virtual FFullySpecifiedType* GetReferencedType()
      {
        return Type;
      }

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
        if ( (FNode*)Type == original )
        {
          if ( Type )
            Type->References.Delete( this );
          Type = (FFullySpecifiedType*)target;
          if ( target )
            target->References.Add( this );
        }
      }

      FFullySpecifiedType* Type = NULL;
      //TLinearArray<FNode*> Declarations;
      CArray<FNode*> Declarations;
      //virtual TS32 GetChildCount() { return Declarations.NumItems()+1; }
      //virtual FNode *GetChildByIndex(TS32 idx) { if (!idx) return Type; return Declarations[idx-1]; }
      //virtual void DeleteChild(FNode *ch) { Declarations.Delete(ch); }
    };

    struct FParameterDeclarator : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FParameterDeclarator* other = (FParameterDeclarator*)node;

        if ( Semantic != other->Semantic )
          return false;

        if ( bIsArray != other->bIsArray )
          return false;

        //LOG_WARN( "CompareTo not implemented for FParameterDeclarator!" );
        return true;
      }

      FParameterDeclarator( const FSourceInfo& InInfo );
      ~FParameterDeclarator();

      static FParameterDeclarator* CreateFromDeclaratorList( FDeclaratorList* List );

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return ParameterDeclarator;
      }
      virtual TBOOL CanBeReferenced() { return true; }

      virtual FFullySpecifiedType* GetReferencedType()
      {
        return Type;
      }

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
        if ( (FNode*)Type == original )
        {
          if ( Type )
            Type->References.Delete( this );
          Type = (FFullySpecifiedType*)target;
          if ( target )
            target->References.Add( this );
        }
      }

      FFullySpecifiedType* Type = nullptr;
      CString Identifier;
      CString Semantic;
      bool bIsArray = false;

      CString BackedupIdentifier;
      virtual void BackupAndClearIdentifier( TBOOL Forced )
      {
        BackedupIdentifier = Identifier; Identifier = "";
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->BackupAndClearIdentifier( true );
      }
      virtual void RestoreIdentifier( TBOOL Forced )
      {
        Identifier = BackedupIdentifier;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->RestoreIdentifier( true );
      }

      virtual void RenameWithReferences( CString &NewName )
      {
        Identifier = NewName;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
        {
          if ( References[ x ]->IsExpression() )
          {
            auto *e = (FExpression*)References[ x ];
            e->Identifier = NewName;
          }
          else
          {
            LOG_ERR( "[AST] Non expression reference found!" );
          }
        }
      }
      //TLinearArray<FExpression*> ArraySize;
      CArray<FExpression*> ArraySize;
      FExpression* DefaultValue = nullptr;
      virtual TS32 DeclarationCount() { return 1; }
    };

    struct FFunction : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FFunction* other = (FFunction*)node;

        if ( ReturnSemantic != other->ReturnSemantic )
          return false;

        if ( !ReturnType->CompareTo( other->ReturnType ) )
          return false;

        //LOG_WARN( "CompareTo not implemented for FFunction!" );
        return true;
      }

      FFunction( const FSourceInfo& InInfo );
      ~FFunction();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return Function;
      }

      virtual FFullySpecifiedType* GetReferencedType()
      {
        return ReturnType;
      }

      virtual void ReparentReferencesFromAToB( FNode* original, FNode* target )
      {
        if ( (FNode*)ReturnType == original )
        {
          if ( ReturnType )
            ReturnType->References.Delete( this );
          ReturnType = (FFullySpecifiedType*)target;
          if ( target )
            target->References.Add( this );
        }
      }

      FFullySpecifiedType* ReturnType = nullptr;
      CString Identifier;
      CString ReturnSemantic;
      virtual TBOOL CanBeReferenced() { return true; }

      //TLinearArray<FNode*> Parameters;
      CArray<FNode*> Parameters;

      bool bIsDefinition = false;

      CString BackedupIdentifier;
      virtual void BackupAndClearIdentifier( TBOOL Forced )
      {
        BackedupIdentifier = Identifier; Identifier = "";
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->BackupAndClearIdentifier( true );
      }
      virtual void RestoreIdentifier( TBOOL Forced )
      {
        Identifier = BackedupIdentifier;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
          References[ x ]->RestoreIdentifier( true );
      }

      virtual void RenameWithReferences( CString &NewName )
      {
        Identifier = NewName;
        for ( TS32 x = 0; x < References.NumItems(); x++ )
        {
          if ( References[ x ]->IsExpression() )
          {
            auto *e = (FExpression*)References[ x ];
            e->Identifier = NewName;
          }
          else
          {
            LOG_ERR( "[AST] Non expression reference found!" );
          }
        }
      }
      //Signature
    };

    struct FExpressionStatement : public FNode
    {
      //virtual bool CompareTo( FNode* node )
      //{
      //  if ( !FNode::CompareTo( node ) )
      //    return false;

      //  FExpressionStatement* other = (FExpressionStatement*)node;

      //  LOG_WARN( "CompareTo not implemented for FExpressionStatement!" );
      //  return false;
      //}

      FExpressionStatement( FExpression* InExpr, const FSourceInfo& InInfo );
      ~FExpressionStatement();

      FExpression* Expression = nullptr;

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return ExpressionStatement;
      }
    };

    struct FCaseLabel : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FCaseLabel* other = (FCaseLabel*)node;

        LOG_WARN( "CompareTo not implemented for FCaseLabel!" );
        return false;
      }

      FCaseLabel( const FSourceInfo& InInfo, AST::FExpression* InExpression );
      ~FCaseLabel();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return CaseLabel;
      }

      FExpression* TestExpression = nullptr;
    };

    struct FCaseLabelList : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FCaseLabelList* other = (FCaseLabelList*)node;

        LOG_WARN( "CompareTo not implemented for FCaseLabelList!" );
        return false;
      }

      FCaseLabelList( const FSourceInfo& InInfo );
      ~FCaseLabelList();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return CaseLabelList;
      }

      //TLinearArray<FCaseLabel*> Labels;
      CArray<FCaseLabel*> Labels;
    };

    struct FCaseStatement : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FCaseStatement* other = (FCaseStatement*)node;

        LOG_WARN( "CompareTo not implemented for FCaseStatemenet!" );
        return false;
      }

      FCaseStatement( const FSourceInfo& InInfo, FCaseLabelList* InLabels );
      ~FCaseStatement();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return CaseStatement;
      }

      FCaseLabelList* Labels;
      //TLinearArray<FNode*> Statements;
      CArray<FNode*> Statements;
    };

    struct FCaseStatementList : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FCaseStatementList* other = (FCaseStatementList*)node;

        LOG_WARN( "CompareTo not implemented for FCaseStatementList!" );
        return false;
      }

      FCaseStatementList( const FSourceInfo& InInfo );
      ~FCaseStatementList();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return CaseStatementList;
      }

      //TLinearArray<FCaseStatement*> Cases;
      CArray<FCaseStatement*> Cases;
    };

    struct FSwitchBody : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FSwitchBody* other = (FSwitchBody*)node;

        LOG_WARN( "CompareTo not implemented for FSwitchBody!" );
        return false;
      }

      FSwitchBody( const FSourceInfo& InInfo );
      ~FSwitchBody();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return SwitchBody;
      }

      FCaseStatementList* CaseList = nullptr;
    };

    struct FSelectionStatement : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FSelectionStatement* other = (FSelectionStatement*)node;

        if ( ( Condition == nullptr ) != ( other->Condition == nullptr ) )
          return false;

        if ( ( ThenStatement == nullptr ) != ( other->ThenStatement == nullptr ) )
          return false;

        if ( ( ElseStatement == nullptr ) != ( other->ElseStatement == nullptr ) )
          return false;

        if ( Condition && !Condition->CompareTo( other->Condition ) )
          return false;

        if ( ThenStatement && !ThenStatement->CompareTo( other->ThenStatement ) )
          return false;

        if ( ElseStatement && !ElseStatement->CompareTo( other->ElseStatement ) )
          return false;

        return true;
      }

      FSelectionStatement( const FSourceInfo& InInfo );
      ~FSelectionStatement();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return SelectionStatement;
      }

      FExpression* Condition = nullptr;

      FNode* ThenStatement = nullptr;
      FNode* ElseStatement = nullptr;

      //virtual TS32 GetChildCount() { int cnt = 0; if (Condition) cnt++; if (ThenStatement) cnt++; if (ElseStatement) cnt++; return cnt; }
      //virtual FNode *GetChildByIndex(TS32 idx)
      //{
      //	AST::FNode *Nodes[3];
      //	int cnt = 0;
      //	if (Condition) Nodes[cnt++] = Condition;
      //	if (ThenStatement) Nodes[cnt++] = ThenStatement;
      //	if (ElseStatement) Nodes[cnt++] = ElseStatement;
      //	return Nodes[idx];
      //}
    };

    struct FSwitchStatement : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FSwitchStatement* other = (FSwitchStatement*)node;

        LOG_WARN( "CompareTo not implemented for FSwitchStatement!" );
        return false;
      }

      FSwitchStatement( const FSourceInfo& InInfo, FExpression* InCondition, FSwitchBody* InBody );
      ~FSwitchStatement();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return SwitchStatement;
      }

      FExpression* Condition = nullptr;
      FSwitchBody* Body = nullptr;
    };

    enum class EIterationType
    {
      For,
      While,
      DoWhile,
    };

    struct FIterationStatement : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FIterationStatement* other = (FIterationStatement*)node;

        if ( Type != other->Type )
          return false;

        //LOG_WARN( "CompareTo not implemented for FIterationStatement!" );
        return true;
      }

      FIterationStatement( const FSourceInfo& InInfo, EIterationType InType );
      ~FIterationStatement();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return IterationStatement;
      }

      EIterationType Type = EIterationType::For;

      AST::FNode* InitStatement = nullptr;
      AST::FNode* Condition = nullptr;
      FExpression* RestExpression = nullptr;

      AST::FNode* Body = nullptr;

      //virtual TS32 GetChildCount() { int cnt = 0; if (InitStatement) cnt++; if (Condition) cnt++; if (RestExpression) cnt++; if (Body) cnt++; return cnt; }
      //virtual FNode *GetChildByIndex(TS32 idx) 
      //{ 
      //	AST::FNode *Nodes[4];
      //	int cnt = 0;
      //	if (InitStatement) Nodes[cnt++] = InitStatement;
      //	if (Condition) Nodes[cnt++] = Condition;
      //	if (RestExpression) Nodes[cnt++] = RestExpression;
      //	if (Body) Nodes[cnt++] = Body;
      //	return Nodes[idx];
      //}

    };

    enum class EJumpType
    {
      Continue,
      Break,
      Return,
      //Discard,
    };

    struct FJumpStatement : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FJumpStatement* other = (FJumpStatement*)node;

        if ( Type != other->Type )
          return false;

        //LOG_WARN( "CompareTo not implemented for FJumpStatement!" );
        return true;
      }

      FJumpStatement( EJumpType InType, const FSourceInfo& InInfo );
      ~FJumpStatement();

      EJumpType Type = EJumpType::Continue;
      FExpression* OptionalExpression = nullptr;

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return JumpStatement;
      }
    };

    struct FFunctionDefinition : public FNode
    {
      FFunctionDefinition( const FSourceInfo& InInfo );
      ~FFunctionDefinition();

      FFunction* Prototype = nullptr;
      FCompoundStatement* Body = nullptr;

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return FunctionDefinition;
      }
      //virtual TS32 GetChildCount() { return 2; }
      //virtual FNode *GetChildByIndex(TS32 idx) { if (!idx) return Body; else return Prototype; }
    };

    struct FAttributeArgument : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FAttributeArgument* other = (FAttributeArgument*)node;

        LOG_WARN( "CompareTo not implemented for FAttributeArgument!" );
        return false;
      }

      FAttributeArgument( const FSourceInfo& InInfo );
      ~FAttributeArgument();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return AttributeArgument;
      }

      CString StringArgument;
      FExpression* ExpressionArgument = nullptr;
    };

    struct FAttribute : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FAttribute* other = (FAttribute*)node;

        LOG_WARN( "CompareTo not implemented for FAttribute!" );
        return false;
      }

      FAttribute( const FSourceInfo& InInfo, const CString &InName );
      ~FAttribute();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray();
      virtual void ChangeChildTo( FNode *Source, FNode *Target );
      virtual void RemoveChild( FNode *child );
      virtual TBOOL FindVariableModification( CString VarName );
      virtual ENodeType GetType() {
        return Attribute;
      }

      CString Name;
      //TLinearArray<FAttributeArgument*> Arguments;
      CArray<FAttributeArgument*> Arguments;
    };

    struct FMacro : public FNode
    {
      virtual bool CompareTo( FNode* node )
      {
        if ( !FNode::CompareTo( node ) )
          return false;

        FMacro* other = (FMacro*)node;

        return ( MacroString == other->MacroString ) != 0;
      }

      FMacro( const FSourceInfo& InInfo, CString InStr );
      ~FMacro();

      virtual void Dump( TS32 Indent, TBOOL NeedsParenthesis = true ) const;
      virtual void BuildChildArray() { }
      virtual void ChangeChildTo( FNode *Source, FNode *Target ) { }
      virtual void RemoveChild( FNode *child ) { }
      virtual TBOOL FindVariableModification( CString VarName ) { return false; }
      virtual ENodeType GetType() {
        return Macro;
      }

      CString MacroString;
    };

  }
}
