#pragma once
#include "apExWindow.h"
#include "../Phoenix_Tool/TreeSpecies.h"

class CapexTreeSpeciesEditor : public CapexWindow
{
  CphxTreeSpecies *EditedSpecies = nullptr;

  TBOOL MessageProc( CWBMessage &Message );

  CphxTreeSpecies *CreateSpecies();
  void DeleteEditedSpecies();
  void SelectSpecies( TS32 index );
  void SetEditedSpecies( CphxTreeSpecies *edited );
  void ShowBoxes( TS32 count );

  void SetItemSelectorToValue( CString name, TS32 value );
  void SetBarToValue( CString name, TS32 value );
  void SetFloatTextToValue( CString name, D3DXFLOAT16 value );
  TBOOL UpdateBarValue( CWBTrackBar *bar, CString barName, CString text, int Value );
  TBOOL UpdateTextValues( CWBTextBox *box );
  TBOOL UpdateTextFloat( CWBTextBox *box, CString boxName, D3DXFLOAT16 &value );

public:

  CapexTreeSpeciesEditor();
  CapexTreeSpeciesEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTreeSpeciesEditor();
  virtual APEXWINDOW GetWindowType() { return apEx_TreeSpeciesEditor; }
  virtual void UpdateData();
};
