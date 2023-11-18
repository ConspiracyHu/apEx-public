#include "BasePCH.h"
#include "TexgenPage.h"
#include "apxProject.h"

CapexTexGenPage::CapexTexGenPage( APEXPAGEID id )
{
  ID = id;
  Name = CString( "New Page " ) + id;
  BaseXRes = BaseYRes = 8;
  hdr = false;
}

CapexTexGenPage::~CapexTexGenPage()
{
  Operators.FreeArray();
}

void SetStatusbarText( CString Text );

void CapexTexGenPage::DeleteSelected()
{
  while ( 1 )
  {
    int SelectedCount = 0;
    for ( TS32 x = 0; x < Operators.NumItems(); x++ )
      if ( Operators[ x ]->Selected ) SelectedCount++;

    for ( TS32 x = 0; x < Operators.NumItems(); x++ )
    {
      if ( Operators[ x ]->Selected )
      {
        TBOOL Cantdelete = false;
        for ( TS32 y = 0; y < Operators[ x ]->GetChildCount() && !Cantdelete; y++ )
        {
          CphxResource *r = Operators[ x ]->GetChild( y );
          CphxTextureOperator_Tool *to = (CphxTextureOperator_Tool *)r;
          CphxTextureOperator_SubroutineCall *sc = (CphxTextureOperator_SubroutineCall *)r;
          if ( r->GetType() != PHX_TEXTUREOPERATOR || ( r->GetType() == PHX_TEXTUREOPERATOR && to->GetOpType() == TEXGEN_OP_SUBROUTINECALL && sc->Subroutine == Operators[ x ] ) )
          {
            SetStatusbarText( _T( "WARNING: Not all operators were deleted due to dependency issues." ) );
            Cantdelete = true;
            continue;
          }
        }

        if ( Cantdelete ) continue;

        Operators.FreeByIndex( x );
        x--;
      }
    }

    int NewSelectedCount = 0;
    for ( TS32 x = 0; x < Operators.NumItems(); x++ )
      if ( Operators[ x ]->Selected ) NewSelectedCount++;

    if ( NewSelectedCount == SelectedCount ) break;
  }
}

CphxTextureOperator_Tool *CapexTexGenPage::CreateOperator( CphxTextureFilter_Tool *Filter )
{
  CphxTextureOperator_Tool *op = new CphxTextureOperator_Tool( CRect( 5, 5, 10, 6 ), this );
  op->AddParent( Filter );
  op->Filter = Filter;
  op->FilterGuid = Filter->GetGUID();
  Operators += op;

  op->OpData.RandSeed = 0;
  op->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;

  for ( TS32 x = 0; x < Filter->Parameters.NumItems(); x++ )
  {
    TS32 p = Filter->GetParamStart( x );
    TS32 s = Filter->GetParamSize( x );

    for ( TS32 y = 0; y < s; y++ )
      op->OpData.Parameters[ p + y ] = Filter->Parameters[ x ]->Default;
  }

  return op;
}

int OperatorPositionSorter( CphxTextureOperator_Tool **a, CphxTextureOperator_Tool **b )
{
  //sort for bottom row first, column next
  if ( ( *a )->Position.y2 != ( *b )->Position.y2 ) return ( *a )->Position.y2 - ( *b )->Position.y2;
  return ( *a )->Position.x1 - ( *b )->Position.x1;
}

void CapexTexGenPage::BuildOperatorConnections()
{
  Operators.Sort( OperatorPositionSorter );

  for ( int x = 0; x < Operators.NumItems(); x++ )
  {
    CphxTextureOperator_Tool *opx = Operators[ x ];
    opx->SubroutineRoot = NULL;

    TS32 OriginalParentCount = opx->GetParentCount( PHX_TEXTUREOPERATOR );

    CphxResource **bck = new CphxResource*[ OriginalParentCount ];
    memset( bck, 0, OriginalParentCount * sizeof( CphxResource* ) );

    TS32 oldincnt = opx->GetParentsOfType( PHX_TEXTUREOPERATOR, bck, OriginalParentCount );

    opx->RemoveParents( PHX_TEXTUREOPERATOR );

    TS32 inputcnt = 0;

    for ( int y = 0; y < x; y++ ) //only need to go until x as ops after that are below the current op
    {
      CphxTextureOperator_Tool *opy = Operators[ y ];
      ConnectOperators( opx, opy, inputcnt );
    }

    if ( opx->GetOpType() == TEXGEN_OP_SUBROUTINE )
      opx->SetSubroutine( (CphxTextureOperator_Subroutine*)opx );

    TS32 maxincnt = opx->GetInputCount();
    for ( TS32 y = maxincnt; y < opx->GetParentCount( PHX_TEXTUREOPERATOR ); y++ )
    {
      if ( y >= 0 && opx->GetParent( PHX_TEXTUREOPERATOR, y ) )
      {
        opx->RemoveParent( opx->GetParent( PHX_TEXTUREOPERATOR, y ) );
        y--;
      }
    }

    opx->ApplyItemSpecificParents();
    opx->NeedsInvalidation = OriginalParentCount != opx->GetParentCount( PHX_TEXTUREOPERATOR );

    for ( int y = 0; y < OriginalParentCount; y++ )
    {
      CphxResource *r = opx->GetParent( PHX_TEXTUREOPERATOR, y );
      if ( bck[ y ] != r )
        opx->NeedsInvalidation = true;
    }

    delete[] bck;
  }

  //for (int x=0; x<Operators.NumItems(); x++)
  //{
  //	if (Operators[x]->IsSubroutine())
  //		Operators[x]->SetSubroutine((CphxTextureOperator_Subroutine*)Operators[x]);
  //}

  //kill endless loops
  Project.KillTextureLoops();

  for ( int x = 0; x < Operators.NumItems(); x++ )
    if ( Operators[ x ]->NeedsInvalidation )
    {
      Operators[ x ]->InvalidateUptoDateFlag();
      Operators[ x ]->NeedsInvalidation = false;
    }
}

void CapexTexGenPage::Export( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "xres" ), false ).SetInt( BaseXRes );
  Node->AddChild( _T( "yres" ), false ).SetInt( BaseYRes );
  Node->AddChild( _T( "hdr" ), false ).SetInt( hdr );
  for ( TS32 x = 0; x < Operators.NumItems(); x++ )
  {
    CXMLNode n;
    switch ( Operators[ x ]->GetOpType() )
    {
    case TEXGEN_OP_LOAD:
      n = Node->AddChild( _T( "Load" ) );
      break;
    case TEXGEN_OP_SAVE:
      n = Node->AddChild( _T( "Save" ) );
      break;
    case TEXGEN_OP_NOP:
      n = Node->AddChild( _T( "NOP" ) );
      break;
    case TEXGEN_OP_SUBROUTINE:
      n = Node->AddChild( _T( "Subroutine" ) );
      break;
    case TEXGEN_OP_SUBROUTINECALL:
      n = Node->AddChild( _T( "SubroutineCall" ) );
      break;
    default:
      n = Node->AddChild( _T( "Operator" ) );
      break;
    }

    Operators[ x ]->Export( &n );
  }
}

void CapexTexGenPage::Import( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  if ( Node->GetChildCount( _T( "xres" ) ) ) Node->GetChild( _T( "xres" ) ).GetValue( BaseXRes );
  if ( Node->GetChildCount( _T( "yres" ) ) ) Node->GetChild( _T( "yres" ) ).GetValue( BaseYRes );
  if ( Node->GetChildCount( _T( "hdr" ) ) ) { int i = 0; Node->GetChild( _T( "hdr" ) ).GetValue( i ); hdr = i; }

  TU8 res = ( BaseXRes << 4 ) + BaseYRes;

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Operator" ) ); x++ )
  {
    CphxTextureOperator_Tool *t = new CphxTextureOperator_Tool( CRect( 0, 0, 1, 1 ), this );
    t->Import( &Node->GetChild( _T( "Operator" ), x ) );
    t->OpData.Resolution = res;
    Operators += t;
  }
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Load" ) ); x++ )
  {
    CphxTextureOperator_Tool *t = new CphxTextureOperator_Load( CRect( 0, 0, 1, 1 ), this );
    t->Import( &Node->GetChild( _T( "Load" ), x ) );
    t->OpData.Resolution = res;
    Operators += t;
  }
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Save" ) ); x++ )
  {
    CphxTextureOperator_Tool *t = new CphxTextureOperator_Save( CRect( 0, 0, 1, 1 ), this );
    t->Import( &Node->GetChild( _T( "Save" ), x ) );
    t->OpData.Resolution = res;
    Operators += t;
  }
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "NOP" ) ); x++ )
  {
    CphxTextureOperator_Tool *t = new CphxTextureOperator_NOP( CRect( 0, 0, 1, 1 ), this );
    t->Import( &Node->GetChild( _T( "NOP" ), x ) );
    t->OpData.Resolution = res;
    Operators += t;
  }
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Subroutine" ) ); x++ )
  {
    CphxTextureOperator_Tool *t = new CphxTextureOperator_Subroutine( CRect( 0, 0, 1, 1 ), this );
    t->Import( &Node->GetChild( _T( "Subroutine" ), x ) );
    t->OpData.Resolution = res;
    Operators += t;
  }
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "SubroutineCall" ) ); x++ )
  {
    CphxTextureOperator_Tool *t = new CphxTextureOperator_SubroutineCall( CRect( 0, 0, 1, 1 ), this );
    t->Import( &Node->GetChild( _T( "SubroutineCall" ), x ) );
    t->OpData.Resolution = res;
    Operators += t;
  }
  BuildOperatorConnections();
}

CphxTextureOperator_Tool * CapexTexGenPage::CreateNOPOperator()
{
  CphxTextureOperator_Tool *op = new CphxTextureOperator_NOP( CRect( 5, 5, 10, 6 ), this );
  op->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;
  Operators += op;
  return op;
}

CphxTextureOperator_Tool * CapexTexGenPage::CreateLoadOperator()
{
  CphxTextureOperator_Tool *op = new CphxTextureOperator_Load( CRect( 5, 5, 10, 6 ), this );
  op->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;
  Operators += op;
  return op;
}

CphxTextureOperator_Tool * CapexTexGenPage::CreateSaveOperator()
{
  CphxTextureOperator_Tool *op = new CphxTextureOperator_Save( CRect( 5, 5, 10, 6 ), this );
  op->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;
  Operators += op;
  return op;
}

CphxTextureOperator_Tool * CapexTexGenPage::CreateSubroutineOperator()
{
  CphxTextureOperator_Tool *op = new CphxTextureOperator_Subroutine( CRect( 5, 5, 10, 6 ), this );
  op->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;
  Operators += op;
  return op;
}

CphxTextureOperator_Tool * CapexTexGenPage::CreateSubroutineCall( CphxTextureOperator_Subroutine *Sub )
{
  CphxTextureOperator_SubroutineCall *op = new CphxTextureOperator_SubroutineCall( CRect( 5, 5, 10, 6 ), this );
  op->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;
  op->Subroutine = Sub;
  for ( TS32 x = 0; x < Sub->Parameters.NumItems(); x++ )
  {
    for ( TS32 y = 0; y < Sub->GetParamSize( x ); y++ )
      op->OpData.Parameters[ Sub->GetParamStart( x ) + y ] = Sub->Parameters[ x ]->Default;
  }
  Operators += op;
  return op;
}

void CapexTexGenPage::ConnectOperators( CphxTextureOperator_Tool *opx, CphxTextureOperator_Tool *opy, TS32 &inputcnt )
{
  int opbottomrow = opy->Position.y2 - 1;
  if ( opbottomrow + 1 == opx->Position.y1 && opy->GetOpType() != TEXGEN_OP_SUBROUTINE )
  {
    //opx is just below opy
    if ( IntervalIntersection( opx->Position.x1, opx->Position.x2 - 1, opy->Position.x1, opy->Position.x2 - 1 ) )
    {
      //the ops intersect in the x dir
      //if (inputcnt<opx->GetInputCount()) //excess is removed later
      opx->AddParent( opy );
    }
  }
}

void CapexTexGenPage::SetBaseXRes( TS32 v )
{
  if ( BaseXRes == v ) return;
  BaseXRes = v;
  UpdateResolution();
}

void CapexTexGenPage::SetBaseYRes( TS32 v )
{
  if ( BaseYRes == v ) return;
  BaseYRes = v;
  UpdateResolution();
}

void CapexTexGenPage::UpdateResolution()
{
  for ( TS32 x = 0; x < Operators.NumItems(); x++ )
  {
    Operators[ x ]->OpData.Resolution = ( BaseXRes << 4 ) + BaseYRes;
    Operators[ x ]->Allocate( false );
  }
}

TS32 OperatorFilterSorter( CphxTextureOperator_Tool **a, CphxTextureOperator_Tool **b )
{
  return Project.GetTextureFilterIndex( ( *a )->FilterGuid ) - Project.GetTextureFilterIndex( ( *b )->FilterGuid );
}

void CapexTexGenPage::SortOpsByFilter()
{
  Operators.Sort( OperatorFilterSorter );
}

void CapexTexGenPage::ToggleHDR()
{
  hdr = !hdr;
  UpdateResolution();
}

void DeinitTexgen()
{
  extern ID3D11Buffer *TexgenBufferPS;
  TexgenBufferPS->Release();
  extern ID3D11SamplerState *TexgenSampler;
  TexgenSampler->Release();
  extern ID3D11SamplerState *TexgenSampler_NoWrap;
  TexgenSampler_NoWrap->Release();
  extern ID3D11SamplerState *TexgenSampler_ShadowCompare;
  TexgenSampler_ShadowCompare->Release();
}