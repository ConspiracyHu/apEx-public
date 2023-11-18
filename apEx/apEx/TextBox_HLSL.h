#pragma once
#include "../../Bedrock/WhiteBoard/TextBox.h"

class CWBTextBox_HLSL : public CWBTextBox
{
  virtual void DoSyntaxHighlight();
  TU32 SkipSpace( TU32 Position );

  void OnDraw( CWBDrawAPI *API );
  CWBTextBox *SearchBox;
  virtual TBOOL MessageProc( CWBMessage &Message );
  CColor Palette[ 256 ];
  virtual TBOOL ColoredText() { return true; }
  virtual CColor GetTextColor( TS32 Index, CColor &DefaultColor );
  virtual TBOOL GetTextBackground( TS32 Index, CColor &Result );
  CString ColorString;
  CString SearchHighlightString;
  virtual void OnCursorPosChange( TS32 Cpos );

  TS32 SearchCursorPos;
  virtual void PostCharInsertion( TS32 CursorPos, TS32 Key );

public:

  CWBTextBox_HLSL();
  CWBTextBox_HLSL( CWBItem *Parent, const CRect &Pos, TS32 flags = WB_TEXTBOX_SINGLELINE, const TCHAR *txt = _T( "" ) );
  virtual ~CWBTextBox_HLSL();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "textboxhlsl" ), CWBTextBox );

  void StartSearch();
  void UpdateSearch( CString &str, TBOOL Next );

  virtual void SetPalette( TS32 idx, CColor Color )
  {
    Palette[ idx ] = Color;
  }

};