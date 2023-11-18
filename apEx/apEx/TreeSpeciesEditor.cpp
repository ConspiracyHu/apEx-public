#include "BasePCH.h"
#include "TreeSpeciesEditor.h"
#include "../Phoenix_Tool/apxProject.h"

#define WINDOWNAME _T("Tree Species Editor")
#define WINDOWXML _T("TreeSpeciesEditor")

TBOOL CapexTreeSpeciesEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );

    if ( b )
    {
      if ( b->GetID() == _T( "newbutton" ) )
      {
        CreateSpecies();
        return true;
      }

      if ( b->GetID() == _T( "delbutton" ) )
      {
        DeleteEditedSpecies();
        return true;
      }
    }

    CWBTextBox *t = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( UpdateTextValues( t ) ) return true;
  }
  break;

  case WBM_FOCUSLOST:
  {
    CWBTextBox *t = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( UpdateTextValues( t ) ) return true;
  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *b = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !b )
      break;

    if ( b->GetID() == _T( "treelist" ) )
    {
      SetEditedSpecies( Project.GetTreeSpeciesByIndex( b->GetItemIndex( Message.Data ) ) );
      return true;
    }

    if ( b->GetID() == _T( "treeshape" ) )
    {
      if ( EditedSpecies )
      {
        if ( EditedSpecies->Tree.Parameters.Shape != Message.Data )
        {
          EditedSpecies->Tree.Parameters.Shape = (TREESHAPE)b->GetItemIndex( Message.Data );
          EditedSpecies->InvalidateUptoDateFlag();
          return true;
        }
      }
    }

    if ( b->GetID() == _T( "leafdistrib" ) )
    {
      if ( EditedSpecies )
      {
        if ( EditedSpecies->Tree.Parameters.LeafDistrib != Message.Data )
        {
          EditedSpecies->Tree.Parameters.LeafDistrib = (TREESHAPE)b->GetItemIndex( Message.Data );
          EditedSpecies->InvalidateUptoDateFlag();
          return true;
        }
      }
    }

  }
  break;

  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "treelist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      CphxTreeSpecies *t = Project.GetTreeSpeciesByIndex( List->GetCursorPosition() );
      if ( t ) t->Name = List->GetCursorItem()->GetText();
      return true;
    }

  }
  break;

  case WBM_VALUECHANGED:
  {
    CWBTrackBar *b = (CWBTrackBar*)App->FindItemByGuid( Message.GetTarget(), _T( "trackbar" ) );
    if ( !b )
      break;

    if ( UpdateBarValue( b, "levelcount", "Level Count", Message.Data ) )
    {
      if ( EditedSpecies->Tree.Parameters.Levels != Message.Data )
        EditedSpecies->InvalidateUptoDateFlag();
      EditedSpecies->Tree.Parameters.Levels = Message.Data;
      ShowBoxes( Message.Data + 1 );
      return true;
    }
    if ( UpdateBarValue( b, "basesize", "Base Size", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.BaseSize != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.BaseSize = Message.Data; return true; }
    if ( UpdateBarValue( b, "basesplits", "Base Splits", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters._0BaseSplits != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters._0BaseSplits = Message.Data; return true; }
    if ( UpdateBarValue( b, "ratiopower", "Ratio Power", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.RatioPower != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.RatioPower = Message.Data; return true; }
    if ( UpdateBarValue( b, "attractionup", "Attraction Up", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.AttractionUp != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.AttractionUp = Message.Data; return true; }
    if ( UpdateBarValue( b, "ratio", "Ratio", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.Ratio != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.Ratio = Message.Data; return true; }
    if ( UpdateBarValue( b, "flare", "Flare", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.Flare != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.Flare = Message.Data; return true; }
    if ( UpdateBarValue( b, "lobes", "Lobes", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.Lobes != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.Lobes = Message.Data; return true; }
    if ( UpdateBarValue( b, "smooth", "Smooth", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.Smooth != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.Smooth = Message.Data; return true; }
    if ( UpdateBarValue( b, "leaves", "Leaves", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.Leaves != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.Leaves = Message.Data; return true; }
    if ( UpdateBarValue( b, "leafbend", "Leaf Bend", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.LeafBend != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.LeafBend = Message.Data; return true; }
    if ( UpdateBarValue( b, "leafquality", "Leaf Quality", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.LeafQuality != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.LeafQuality = Message.Data; return true; }

    if ( UpdateBarValue( b, "pruneratio", "Prune Ratio", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.PruneRatio != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.PruneRatio = Message.Data; return true; }
    if ( UpdateBarValue( b, "prunewidth", "Prune Width", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.PruneWidth != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.PruneWidth = Message.Data; return true; }
    if ( UpdateBarValue( b, "prunewidthpeak", "Prune Width Peak", Message.Data ) ) { if ( EditedSpecies->Tree.Parameters.PruneWidthPeak != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Parameters.PruneWidthPeak = Message.Data; return true; }

    for ( int x = 0; x < 4; x++ )
    {
      CString preString = CString::Format( "l%d", x + 1 );
      if ( UpdateBarValue( b, preString + "nbranches", "Branch Count", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nBranches != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nBranches = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "curveres", "Curve Resolution", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nCurveRes != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nCurveRes = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "ntaper", "Taper", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nTaper != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nTaper = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "nsplitangle", "Split Angle", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nSplitAngle != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nSplitAngle = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "nsplitanglev", "Split Angle Chaos", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nSplitAngleV != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nSplitAngleV = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "nbranchdist", "Branch Dist", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nBranchDist != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nBranchDist = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "rotate", "Rotate", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nRotate != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nRotate = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "rotatev", "Rotate Chaos", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nRotateV != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nRotateV = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "downangle", "Down Angle", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nDownAngle != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nDownAngle = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "downanglev", "Down Angle Chaos", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nDownAngleV != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nDownAngleV = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "curve", "Curve", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nCurve != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nCurve = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "curvev", "Curve Chaos", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nCurveV != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nCurveV = Message.Data; return true; }
      if ( UpdateBarValue( b, preString + "curveback", "Curveback", Message.Data ) ) { if ( EditedSpecies->Tree.Levels[ x ].nCurveBack != Message.Data ) EditedSpecies->InvalidateUptoDateFlag(); EditedSpecies->Tree.Levels[ x ].nCurveBack = Message.Data; return true; }
    }

  }
  break;

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

CphxTreeSpecies *CapexTreeSpeciesEditor::CreateSpecies()
{
  CphxTreeSpecies *species = Project.CreateTreeSpecies();

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "treelist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->AddItem( species->GetName() );
    SelectSpecies( List->NumItems() - 1 );
  }

  return species;
}

void CapexTreeSpeciesEditor::DeleteEditedSpecies()
{

}

void CapexTreeSpeciesEditor::SelectSpecies( TS32 index )
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "treelist" ), _T( "itemselector" ) );
  if ( !List ) return;

  List->SelectItemByIndex( index );
  EditedSpecies = Project.GetTreeSpeciesByIndex( index );
}

void CapexTreeSpeciesEditor::SetEditedSpecies( CphxTreeSpecies *edited )
{
  EditedSpecies = edited;

  ShowBoxes( 0 );

  if ( !EditedSpecies )
    return;

  ShowBoxes( EditedSpecies->Tree.Parameters.Levels + 1 );

  SetItemSelectorToValue( "treeshape", EditedSpecies->Tree.Parameters.Shape );

  SetBarToValue( "levelcount", EditedSpecies->Tree.Parameters.Levels );
  SetBarToValue( "basesize", EditedSpecies->Tree.Parameters.BaseSize );
  SetBarToValue( "basesplits", EditedSpecies->Tree.Parameters._0BaseSplits );
  SetBarToValue( "ratiopower", EditedSpecies->Tree.Parameters.RatioPower );
  SetBarToValue( "attractionup", EditedSpecies->Tree.Parameters.AttractionUp );
  SetBarToValue( "ratio", EditedSpecies->Tree.Parameters.Ratio );
  SetBarToValue( "flare", EditedSpecies->Tree.Parameters.Flare );
  SetBarToValue( "lobes", EditedSpecies->Tree.Parameters.Lobes );
  SetBarToValue( "smooth", EditedSpecies->Tree.Parameters.Smooth );
  SetBarToValue( "leaves", EditedSpecies->Tree.Parameters.Leaves );
  SetBarToValue( "leafbend", EditedSpecies->Tree.Parameters.LeafBend );
  SetBarToValue( "leafquality", EditedSpecies->Tree.Parameters.LeafQuality );

  SetBarToValue( "pruneratio", EditedSpecies->Tree.Parameters.PruneRatio );
  SetBarToValue( "prunewidth", EditedSpecies->Tree.Parameters.PruneWidth );
  SetBarToValue( "prunewidthpeak", EditedSpecies->Tree.Parameters.PruneWidthPeak );

  SetItemSelectorToValue( "leafdistrib", EditedSpecies->Tree.Parameters.LeafDistrib );

  SetFloatTextToValue( "scale", EditedSpecies->Tree.Parameters.Scale );
  SetFloatTextToValue( "lobedepth", EditedSpecies->Tree.Parameters.LobeDepth );
  SetFloatTextToValue( "0scale", EditedSpecies->Tree.Parameters._0Scale );
  SetFloatTextToValue( "0scalev", EditedSpecies->Tree.Parameters._0ScaleV );
  SetFloatTextToValue( "leafscale", EditedSpecies->Tree.Parameters.LeafScale );
  SetFloatTextToValue( "leafscalex", EditedSpecies->Tree.Parameters.LeafScaleX );
  SetFloatTextToValue( "leafstemlength", EditedSpecies->Tree.Parameters.LeafStemLen );

  SetFloatTextToValue( "prunepowerlow", EditedSpecies->Tree.Parameters.PrunePowerLow );
  SetFloatTextToValue( "prunepowerhigh", EditedSpecies->Tree.Parameters.PrunePowerHigh );

  for ( int x = 0; x < 4; x++ )
  {
    CString preString = CString::Format( "l%d", x + 1 );
    SetBarToValue( preString + "nbranches", EditedSpecies->Tree.Levels[ x ].nBranches );
    SetBarToValue( preString + "curveres", EditedSpecies->Tree.Levels[ x ].nCurveRes );
    SetBarToValue( preString + "ntaper", EditedSpecies->Tree.Levels[ x ].nTaper );
    SetBarToValue( preString + "nsplitangle", EditedSpecies->Tree.Levels[ x ].nSplitAngle );
    SetBarToValue( preString + "nsplitanglev", EditedSpecies->Tree.Levels[ x ].nSplitAngleV );
    SetBarToValue( preString + "nbranchdist", EditedSpecies->Tree.Levels[ x ].nBranchDist );
    SetBarToValue( preString + "rotate", EditedSpecies->Tree.Levels[ x ].nRotate );
    SetBarToValue( preString + "rotatev", EditedSpecies->Tree.Levels[ x ].nRotateV );
    SetBarToValue( preString + "downangle", EditedSpecies->Tree.Levels[ x ].nDownAngle );
    SetBarToValue( preString + "downanglev", EditedSpecies->Tree.Levels[ x ].nDownAngleV );
    SetBarToValue( preString + "curve", EditedSpecies->Tree.Levels[ x ].nCurve );
    SetBarToValue( preString + "curvev", EditedSpecies->Tree.Levels[ x ].nCurveV );
    SetBarToValue( preString + "curveback", EditedSpecies->Tree.Levels[ x ].nCurveBack );

    SetFloatTextToValue( preString + "length", EditedSpecies->Tree.Levels[ x ].nLength );
    SetFloatTextToValue( preString + "lengthv", EditedSpecies->Tree.Levels[ x ].nLengthV );
    SetFloatTextToValue( preString + "nsegsplits", EditedSpecies->Tree.Levels[ x ].nSegSplits );
  }

}

void CapexTreeSpeciesEditor::ShowBoxes( TS32 count )
{
  CString boxes[] = { "treebox", "l1box", "l2box", "l3box", "l4box", "l5box" };

  for ( TS32 x = 0; x < 5; x++ )
  {
    CWBItem *it = FindChildByID( boxes[ x ].GetPointer() );
    if ( it )
    {
      it->Hide( x > count );
      if ( x > 0 )
      {
        CWBFieldSet *f = (CWBFieldSet*)it;
        f->SetText( CString::Format( "Level %d", x - 1 ) );
        if ( x == 1 )
          f->SetText( "Trunk" );
        if ( x == count )
          f->SetText( "Leaves" );
      }
    }
  }
}

void CapexTreeSpeciesEditor::SetItemSelectorToValue( CString name, TS32 value )
{
  CWBItemSelector *selector;
  selector = (CWBItemSelector*)FindChildByID( name.GetPointer(), _T( "itemselector" ) );
  if ( selector )
    selector->SelectItemByIndex( value );
}

void CapexTreeSpeciesEditor::SetBarToValue( CString name, TS32 value )
{
  CWBTrackBar *bar;

  bar = (CWBTrackBar*)FindChildByID( name.GetPointer(), _T( "trackbar" ) );
  if ( bar )
    bar->SetValue( value );
}

void CapexTreeSpeciesEditor::SetFloatTextToValue( CString name, D3DXFLOAT16 value )
{
  CWBTextBox *textBox;

  textBox = (CWBTextBox*)FindChildByID( name.GetPointer(), _T( "textbox" ) );
  if ( textBox )
    textBox->SetText( CString::Format( "%g", (float)value ) );
}

TBOOL CapexTreeSpeciesEditor::UpdateBarValue( CWBTrackBar *bar, CString barName, CString text, int Value )
{
  if ( bar->GetID() != barName )
    return false;

  TS32 val;
  bar->GetValue( val );

  if ( Value != val && EditedSpecies )
    EditedSpecies->InvalidateUptoDateFlag();

  bar->SetText( text + ": " + CString::Format( "%d", Value ) );

  if ( !EditedSpecies )
    return false;

  return true;
}

TBOOL CapexTreeSpeciesEditor::UpdateTextValues( CWBTextBox *t )
{
  if ( t && EditedSpecies )
  {
    if ( UpdateTextFloat( t, "scale", EditedSpecies->Tree.Parameters.Scale ) ) return true;
    if ( UpdateTextFloat( t, "lobedepth", EditedSpecies->Tree.Parameters.LobeDepth ) ) return true;
    if ( UpdateTextFloat( t, "0scale", EditedSpecies->Tree.Parameters._0Scale ) ) return true;
    if ( UpdateTextFloat( t, "0scalev", EditedSpecies->Tree.Parameters._0ScaleV ) ) return true;
    if ( UpdateTextFloat( t, "leafscale", EditedSpecies->Tree.Parameters.LeafScale ) ) return true;
    if ( UpdateTextFloat( t, "leafscalex", EditedSpecies->Tree.Parameters.LeafScaleX ) ) return true;
    if ( UpdateTextFloat( t, "leafstemlength", EditedSpecies->Tree.Parameters.LeafStemLen ) ) return true;

    if ( UpdateTextFloat( t, "prunepowerlow", EditedSpecies->Tree.Parameters.PrunePowerLow ) ) return true;
    if ( UpdateTextFloat( t, "prunepowerhigh", EditedSpecies->Tree.Parameters.PrunePowerHigh ) ) return true;

    for ( int x = 0; x < 4; x++ )
    {
      CString preString = CString::Format( "l%d", x + 1 );
      if ( UpdateTextFloat( t, preString + "length", EditedSpecies->Tree.Levels[ x ].nLength ) ) return true;
      if ( UpdateTextFloat( t, preString + "lengthv", EditedSpecies->Tree.Levels[ x ].nLengthV ) ) return true;
      if ( UpdateTextFloat( t, preString + "nsegsplits", EditedSpecies->Tree.Levels[ x ].nSegSplits ) ) return true;
    }
  }
  return false;
}

TBOOL CapexTreeSpeciesEditor::UpdateTextFloat( CWBTextBox *box, CString boxName, D3DXFLOAT16 &value )
{
  if ( box->GetID() != boxName )
    return false;

  TF32 fl = 0;
  if ( box->GetText().Scan( "%f", &fl ) == 1 )
  {
    D3DXFLOAT16 f16 = fl;
    if ( f16 != value )
    {
      value = f16;
      EditedSpecies->InvalidateUptoDateFlag();
    }
  }

  box->SetText( CString::Format( "%g", (float)value ) );

  return true;
}

CapexTreeSpeciesEditor::CapexTreeSpeciesEditor() : CapexWindow()
{
}

CapexTreeSpeciesEditor::CapexTreeSpeciesEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexTreeSpeciesEditor::~CapexTreeSpeciesEditor()
{

}

void CapexTreeSpeciesEditor::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( "treelist", "itemselector" );

  TS32 Cursor = List->GetCursorPosition();

  List->Flush();

  for ( TS32 x = 0; x < Project.GetTreeSpeciesCount(); x++ )
    List->AddItem( Project.GetTreeSpeciesByIndex( x )->GetName() );

  List->SelectItemByIndex( Cursor );
}

