#pragma once
#include "../../Bedrock/WhiteBoard/WhiteBoard.h"
#include "apExWindow.h"

extern CRingBuffer<CString> ConsoleLog;

class CapexConsoleLog : public CLoggerOutput
{
public:

  virtual void Process( LOGVERBOSITY v, TCHAR *String )
  {
    ConsoleLog += String;
  }
};

class CapexConsole : public CWBItem
{
  void OnDraw( CWBDrawAPI *API );

  TBOOL ClickThrough;

  void UpdateScrollParameters();
  TBOOL MessageProc( CWBMessage &Message );
  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

public:

  CapexConsole();
  CapexConsole( CWBItem *Parent, const CRect &Pos, TBOOL ClickThrough );
  virtual ~CapexConsole();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, TBOOL ClickThrough );

  WB_DECLARE_GUIITEM( _T( "console" ), CWBItem );

  static CWBItem * Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

};

class CapexConsoleWindow : public CapexWindow
{
  CapexConsole *Console;

  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CapexConsoleWindow();
  CapexConsoleWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench );
  virtual ~CapexConsoleWindow();
  virtual APEXWINDOW GetWindowType() { return apEx_Console; }
  virtual void UpdateData();
};
