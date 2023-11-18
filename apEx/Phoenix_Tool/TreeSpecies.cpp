#include "BasePCH.h"
#include "TreeSpecies.h"
#include "apxProject.h"

void CphxTreeSpecies::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "Levels" ), false ).SetInt( Tree.Parameters.Levels );

  Node->AddChild( _T( "Lobes" ), false ).SetInt( Tree.Parameters.Lobes );
  Node->AddChild( _T( "_0BaseSplits" ), false ).SetInt( Tree.Parameters._0BaseSplits );
  Node->AddChild( _T( "LeafShape" ), false ).SetInt( Tree.Parameters.LeafShape );
  Node->AddChild( _T( "Shape" ), false ).SetInt( Tree.Parameters.Shape );
  Node->AddChild( _T( "LeafDistrib" ), false ).SetInt( Tree.Parameters.LeafDistrib );

  Node->AddChild( _T( "BaseSize" ), false ).SetInt( Tree.Parameters.BaseSize );
  Node->AddChild( _T( "LeafQuality" ), false ).SetInt( Tree.Parameters.LeafQuality );
  Node->AddChild( _T( "LeafBend" ), false ).SetInt( Tree.Parameters.LeafBend );
  Node->AddChild( _T( "Smooth" ), false ).SetInt( Tree.Parameters.Smooth );
  Node->AddChild( _T( "Flare" ), false ).SetInt( Tree.Parameters.Flare );
  Node->AddChild( _T( "Ratio" ), false ).SetInt( Tree.Parameters.Ratio );

  Node->AddChild( _T( "AttractionUp" ), false ).SetInt( Tree.Parameters.AttractionUp );
  Node->AddChild( _T( "Leaves" ), false ).SetInt( Tree.Parameters.Leaves );
  Node->AddChild( _T( "RatioPower" ), false ).SetInt( Tree.Parameters.RatioPower );

  Node->AddChild( _T( "PruneRatio" ), false ).SetInt( Tree.Parameters.PruneRatio );
  Node->AddChild( _T( "PruneWidth" ), false ).SetInt( Tree.Parameters.PruneWidth );
  Node->AddChild( _T( "PruneWidthPeak" ), false ).SetInt( Tree.Parameters.PruneWidthPeak );

  TS32 v;
  Node->AddChild( _T( "LobeDepth" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.LobeDepth ) );
  Node->AddChild( _T( "Scale" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.Scale ) );
  Node->AddChild( _T( "_0Scale" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters._0Scale ) );
  Node->AddChild( _T( "_0ScaleV" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters._0ScaleV ) );
  Node->AddChild( _T( "LeafScale" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.LeafScale ) );
  Node->AddChild( _T( "LeafScaleX" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.LeafScaleX ) );
  Node->AddChild( _T( "LeafStemLen" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.LeafStemLen ) );
  Node->AddChild( _T( "PrunePowerLow" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.PrunePowerLow ) );
  Node->AddChild( _T( "PrunePowerHigh" ), false ).SetInt( v = *( (TU16*)&Tree.Parameters.PrunePowerHigh ) );

  for ( TS32 x = 0; x <= Tree.Parameters.Levels; x++ )
  {
    CXMLNode level = Node->AddChild( "Level" );
    level.SetAttributeFromInteger( "Level", x );

    level.AddChild( _T( "nBranches" ), false ).SetInt( Tree.Levels[ x ].nBranches );
    level.AddChild( _T( "nCurveRes" ), false ).SetInt( Tree.Levels[ x ].nCurveRes );
    level.AddChild( _T( "nTaper" ), false ).SetInt( Tree.Levels[ x ].nTaper );
    level.AddChild( _T( "nSplitAngle" ), false ).SetInt( Tree.Levels[ x ].nSplitAngle );
    level.AddChild( _T( "nSplitAngleV" ), false ).SetInt( Tree.Levels[ x ].nSplitAngleV );
    level.AddChild( _T( "nBranchDist" ), false ).SetInt( Tree.Levels[ x ].nBranchDist );
    level.AddChild( _T( "nRotate" ), false ).SetInt( Tree.Levels[ x ].nRotate );
    level.AddChild( _T( "nRotateV" ), false ).SetInt( Tree.Levels[ x ].nRotateV );
    level.AddChild( _T( "nDownAngle" ), false ).SetInt( Tree.Levels[ x ].nDownAngle );
    level.AddChild( _T( "nDownAngleV" ), false ).SetInt( Tree.Levels[ x ].nDownAngleV );
    level.AddChild( _T( "nCurve" ), false ).SetInt( Tree.Levels[ x ].nCurve );
    level.AddChild( _T( "nCurveV" ), false ).SetInt( Tree.Levels[ x ].nCurveV );
    level.AddChild( _T( "nCurveBack" ), false ).SetInt( Tree.Levels[ x ].nCurveBack );

    level.AddChild( _T( "nLength" ), false ).SetInt( v = *( (TU16*)&Tree.Levels[ x ].nLength ) );
    level.AddChild( _T( "nLengthV" ), false ).SetInt( v = *( (TU16*)&Tree.Levels[ x ].nLengthV ) );
    level.AddChild( _T( "nSegSplits" ), false ).SetInt( v = *( (TU16*)&Tree.Levels[ x ].nSegSplits ) );
  }
}

void CphxTreeSpecies::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  TS32 v;
  if ( Node->GetChildCount( _T( "Levels" ) ) ) { if ( Node->GetChild( _T( "Levels" ) ).GetValue( v ) ) Tree.Parameters.Levels = v; }
  if ( Node->GetChildCount( _T( "Lobes" ) ) ) { if ( Node->GetChild( _T( "Lobes" ) ).GetValue( v ) ) Tree.Parameters.Lobes = v; }
  if ( Node->GetChildCount( _T( "_0BaseSplits" ) ) ) { if ( Node->GetChild( _T( "_0BaseSplits" ) ).GetValue( v ) ) Tree.Parameters._0BaseSplits = v; }
  if ( Node->GetChildCount( _T( "LeafShape" ) ) ) { if ( Node->GetChild( _T( "LeafShape" ) ).GetValue( v ) ) Tree.Parameters.LeafShape = v; }
  if ( Node->GetChildCount( _T( "Shape" ) ) ) { if ( Node->GetChild( _T( "Shape" ) ).GetValue( v ) ) Tree.Parameters.Shape = (TREESHAPE)v; }
  if ( Node->GetChildCount( _T( "LeafDistrib" ) ) ) { if ( Node->GetChild( _T( "LeafDistrib" ) ).GetValue( v ) ) Tree.Parameters.LeafDistrib = (TREESHAPE)v; }
  if ( Node->GetChildCount( _T( "BaseSize" ) ) ) { if ( Node->GetChild( _T( "BaseSize" ) ).GetValue( v ) ) Tree.Parameters.BaseSize = v; }
  if ( Node->GetChildCount( _T( "LeafQuality" ) ) ) { if ( Node->GetChild( _T( "LeafQuality" ) ).GetValue( v ) ) Tree.Parameters.LeafQuality = v; }
  if ( Node->GetChildCount( _T( "LeafBend" ) ) ) { if ( Node->GetChild( _T( "LeafBend" ) ).GetValue( v ) ) Tree.Parameters.LeafBend = v; }
  if ( Node->GetChildCount( _T( "Smooth" ) ) ) { if ( Node->GetChild( _T( "Smooth" ) ).GetValue( v ) ) Tree.Parameters.Smooth = v; }
  if ( Node->GetChildCount( _T( "Flare" ) ) ) { if ( Node->GetChild( _T( "Flare" ) ).GetValue( v ) ) Tree.Parameters.Flare = v; }
  if ( Node->GetChildCount( _T( "Ratio" ) ) ) { if ( Node->GetChild( _T( "Ratio" ) ).GetValue( v ) ) Tree.Parameters.Ratio = v; }
  if ( Node->GetChildCount( _T( "AttractionUp" ) ) ) { if ( Node->GetChild( _T( "AttractionUp" ) ).GetValue( v ) ) Tree.Parameters.AttractionUp = v; }
  if ( Node->GetChildCount( _T( "Leaves" ) ) ) { if ( Node->GetChild( _T( "Leaves" ) ).GetValue( v ) ) Tree.Parameters.Leaves = v; }
  if ( Node->GetChildCount( _T( "RatioPower" ) ) ) { if ( Node->GetChild( _T( "RatioPower" ) ).GetValue( v ) ) Tree.Parameters.RatioPower = v; }

  if ( Node->GetChildCount( _T( "PruneRatio" ) ) ) { if ( Node->GetChild( _T( "PruneRatio" ) ).GetValue( v ) ) Tree.Parameters.PruneRatio = v; }
  if ( Node->GetChildCount( _T( "PruneWidth" ) ) ) { if ( Node->GetChild( _T( "PruneWidth" ) ).GetValue( v ) ) Tree.Parameters.PruneWidth = v; }
  if ( Node->GetChildCount( _T( "PruneWidthPeak" ) ) ) { if ( Node->GetChild( _T( "PruneWidthPeak" ) ).GetValue( v ) ) Tree.Parameters.PruneWidthPeak = v; }

  TU16 v16;
  if ( Node->GetChildCount( _T( "LobeDepth" ) ) ) { if ( Node->GetChild( _T( "LobeDepth" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.LobeDepth = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "Scale" ) ) ) { if ( Node->GetChild( _T( "Scale" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.Scale = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "_0Scale" ) ) ) { if ( Node->GetChild( _T( "_0Scale" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters._0Scale = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "_0ScaleV" ) ) ) { if ( Node->GetChild( _T( "_0ScaleV" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters._0ScaleV = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "LeafScale" ) ) ) { if ( Node->GetChild( _T( "LeafScale" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.LeafScale = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "LeafScaleX" ) ) ) { if ( Node->GetChild( _T( "LeafScaleX" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.LeafScaleX = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "LeafStemLen" ) ) ) { if ( Node->GetChild( _T( "LeafStemLen" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.LeafStemLen = *( (D3DXFLOAT16*)&v16 ); } }

  if ( Node->GetChildCount( _T( "PrunePowerLow" ) ) ) { if ( Node->GetChild( _T( "PrunePowerLow" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.PrunePowerLow = *( (D3DXFLOAT16*)&v16 ); } }
  if ( Node->GetChildCount( _T( "PrunePowerHigh" ) ) ) { if ( Node->GetChild( _T( "PrunePowerHigh" ) ).GetValue( v ) ) { v16 = v; Tree.Parameters.PrunePowerHigh = *( (D3DXFLOAT16*)&v16 ); } }

  for ( TS32 x = 0; x < Node->GetChildCount( "Level" ); x++ )
  {
    CXMLNode level = Node->GetChild( "Level", x );
    TS32 l = 0;
    if ( !level.HasAttribute( "Level" ) )
      continue;
    level.GetAttributeAsInteger( "Level", &l );
    if ( l < 0 || l>4 )
      continue;

    if ( level.GetChildCount( _T( "nBranches" ) ) ) { if ( level.GetChild( _T( "nBranches" ) ).GetValue( v ) ) Tree.Levels[ l ].nBranches = v; }
    if ( level.GetChildCount( _T( "nCurveRes" ) ) ) { if ( level.GetChild( _T( "nCurveRes" ) ).GetValue( v ) ) Tree.Levels[ l ].nCurveRes = v; }
    if ( level.GetChildCount( _T( "nTaper" ) ) ) { if ( level.GetChild( _T( "nTaper" ) ).GetValue( v ) ) Tree.Levels[ l ].nTaper = v; }
    if ( level.GetChildCount( _T( "nSplitAngle" ) ) ) { if ( level.GetChild( _T( "nSplitAngle" ) ).GetValue( v ) ) Tree.Levels[ l ].nSplitAngle = v; }
    if ( level.GetChildCount( _T( "nSplitAngleV" ) ) ) { if ( level.GetChild( _T( "nSplitAngleV" ) ).GetValue( v ) ) Tree.Levels[ l ].nSplitAngleV = v; }
    if ( level.GetChildCount( _T( "nBranchDist" ) ) ) { if ( level.GetChild( _T( "nBranchDist" ) ).GetValue( v ) ) Tree.Levels[ l ].nBranchDist = v; }
    if ( level.GetChildCount( _T( "nRotate" ) ) ) { if ( level.GetChild( _T( "nRotate" ) ).GetValue( v ) ) Tree.Levels[ l ].nRotate = v; }
    if ( level.GetChildCount( _T( "nRotateV" ) ) ) { if ( level.GetChild( _T( "nRotateV" ) ).GetValue( v ) ) Tree.Levels[ l ].nRotateV = v; }
    if ( level.GetChildCount( _T( "nDownAngle" ) ) ) { if ( level.GetChild( _T( "nDownAngle" ) ).GetValue( v ) ) Tree.Levels[ l ].nDownAngle = v; }
    if ( level.GetChildCount( _T( "nDownAngleV" ) ) ) { if ( level.GetChild( _T( "nDownAngleV" ) ).GetValue( v ) ) Tree.Levels[ l ].nDownAngleV = v; }
    if ( level.GetChildCount( _T( "nCurve" ) ) ) { if ( level.GetChild( _T( "nCurve" ) ).GetValue( v ) ) Tree.Levels[ l ].nCurve = v; }
    if ( level.GetChildCount( _T( "nCurveV" ) ) ) { if ( level.GetChild( _T( "nCurveV" ) ).GetValue( v ) ) Tree.Levels[ l ].nCurveV = v; }
    if ( level.GetChildCount( _T( "nCurveBack" ) ) ) { if ( level.GetChild( _T( "nCurveBack" ) ).GetValue( v ) ) Tree.Levels[ l ].nCurveBack = v; }

    if ( level.GetChildCount( _T( "nLength" ) ) ) { if ( level.GetChild( _T( "nLength" ) ).GetValue( v ) ) { v16 = v; Tree.Levels[ l ].nLength = *( (D3DXFLOAT16*)&v16 ); } }
    if ( level.GetChildCount( _T( "nLengthV" ) ) ) { if ( level.GetChild( _T( "nLengthV" ) ).GetValue( v ) ) { v16 = v; Tree.Levels[ l ].nLengthV = *( (D3DXFLOAT16*)&v16 ); } }
    if ( level.GetChildCount( _T( "nSegSplits" ) ) ) { if ( level.GetChild( _T( "nSegSplits" ) ).GetValue( v ) ) { v16 = v; Tree.Levels[ l ].nSegSplits = *( (D3DXFLOAT16*)&v16 ); } }
  }
}

TBOOL CphxTreeSpecies::GenerateResource( CCoreDevice *Dev )
{
  return true;
}

CphxTreeSpecies::CphxTreeSpecies() : CphxResource()
{
  Name = "New Species";
  memset( &Tree, 0, sizeof( Tree ) );
}

CphxTreeSpecies::~CphxTreeSpecies()
{

}

CString CphxTreeSpecies::GetName()
{
  return Name;
}
