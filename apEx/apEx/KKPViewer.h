#pragma once
#include "apExWindow.h"

class KKP
{
public:

  struct KKPFile
  {
    CString name;
    int size{};
    float packedSize{};
  };

  struct KKPSymbol
  {
    CString name;
    int size{};
    double packedSize{};
    bool isCode{};
    int fileID;
    int sourcePos;
    double cumulativeSize{};
    int cumulativeUnpackedSize{};
  };

  struct KKPByteData
  {
    unsigned char data;
    short symbol;
    double packed;
    short line;
    short file;
  };

  CArray<KKPFile> files;
  CArray<KKPSymbol> symbols;
  CArray<KKPByteData> bytes;

  void Load( CStreamReader& reader );
};

class CKKPDisplay : public CWBItem
{
public:
  enum class HighlightMode
  {
    None,
    Symbol,
    File,
    Line
  };

private:
  KKP* kkp = nullptr;
  int lineCount = 0;

  HighlightMode selection{};
  int highlightID{};
  int lineID{};

  virtual void OnDraw( CWBDrawAPI* API );
  virtual TBOOL MessageProc( CWBMessage& Message );
  void UpdateLineCount();

public:

  bool hasMouseHighlight = false;
  int mousehighlightbye = 0;
  bool ignoreScrollTeleport = false;

  CKKPDisplay( CWBItem* Parent, const CRect& Pos );
  virtual ~CKKPDisplay();

  virtual TBOOL Initialize( CWBItem* Parent, const CRect& Position );

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "kkpdisplay" ), CWBItem );

  void SetKKP( KKP* kkp );
  void SetHighlight( HighlightMode mode, int id );
  void SetHighlightLine( int lineID );
  void GoToByte( int byte );
};

class CKKPViewer : public CapexWindow
{
  KKP kkp;

  TBOOL MessageProc( CWBMessage& Message );
  int currFileIndex = -1;
  int clickedLine = -1;

  int sortType = 0;
  int sortOrder = 0;

public:

  void ExportSymbolList();
  void LoadKKP( const CString& fname );
  void OpenKKP();
  void LoadSYM( const CString& fname );
  void OpenSYM();
  void RebuildSymbolList();

  CKKPViewer();
  CKKPViewer( CWBItem* Parent, const CRect& Pos, CapexWorkBench* WorkBench );
  virtual ~CKKPViewer();
  virtual APEXWINDOW GetWindowType() { return apEx_KKPViewer; }

  virtual void UpdateData();
  void OpenSourceFile( int fileIndex );
};
