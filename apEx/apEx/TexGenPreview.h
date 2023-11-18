#pragma once
#include "apExWindow.h"
#include "..\Phoenix_Tool\Texgen_tool.h"

class CapexTexGenPreview : public CapexWindow
{

  APEXOPID EditedOp;
  APEXOPID DisplayedOpID;
  CCoreTexture2D *Texture;
  CCoreBlendState *AlphaBlend, *SolidBlend;
  CCoreSamplerState *WrapSampler;

  TBOOL Generating;
  TBOOL CopyFromOp;

  TBOOL MessageProc( CWBMessage &Message );
  void OpenExportMenu();

  TBOOL Maximized;
  CRect MaximizedPosition, NormalPosition;
  TBOOL JustBeenMaximized;

  CVector2 Pan, _Pan;

  TBOOL Panning;

  void DrawCheckerboard( CWBDrawAPI *API, CRect Rect );

public:

  CapexTexGenPreview();
  CapexTexGenPreview( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexTexGenPreview();
  virtual APEXWINDOW GetWindowType() { return apEx_TexGenPreview; }

  void SetEditedOperator( APEXOPID ID );
  APEXOPID GetEditedOperator() { return EditedOp; }

  virtual void OnDraw( CWBDrawAPI *API );
  virtual void UpdateData();

  void ExportWindow( CXMLNode *node );
  virtual void ImportConfig( CXMLNode *node, CRect &Pos );

};
