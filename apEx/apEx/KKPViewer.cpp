#include "BasePCH.h"
#include "KKPViewer.h"
#include "WorkBench.h"
#include "apExRoot.h"
#define WINDOWNAME _T("KKrunchy Pack Data Viewer")
#define WINDOWXML _T("KKPViewer")
#include <ShellAPI.h>
#include <CommDlg.h>

static const char hexMap[] = "0123456789ABCDEF";

static const CColor colors[] =
{
/*
  CColor( 0xff7ffe86 ),
  CColor( 0xff00fe30 ),
  CColor( 0xff00bf21 ),
  CColor( 0xff007f43 ),
  CColor( 0xff0f618f ),
  CColor( 0xff232fed ),
  CColor( 0xff14129e ),
  CColor( 0xffffffff ),
  CColor( 0xff8f0000 ),
  CColor( 0xfffd0000 ),
  CColor( 0xfffd00fd ),
*/
  CColor( 0xff1d4877 ),
  CColor( 0xff196e62 ),
  CColor( 0xff1b8a5a ),
  CColor( 0xffb99e35 ),
  CColor( 0xfffbb021 ),
  CColor( 0xfff89c2d ),
  CColor( 0xfff68838 ),
  CColor( 0xfff48628 ),
  CColor( 0xffee3e32 ),
  CColor( 0xfffd0000 ),
  CColor( 0xfffd00fd ),
};

CColor GetCompressionColorGradient( int t )
{
  float tval = max( 0, min( 1, ( t / 9.0f ) ) );

  tval = powf( tval, 1.5f );

  int count = 5;
  // https://www.color-hex.com/color-palette/27541
  CColor colors[] = { CColor( 0xff24568f ), // CColor( 0xff1d4877 ),
                      CColor( 0xff1a9c67 ), // CColor( 0xff1b8a5a ),
                      CColor( 0xfffbb021 ),
                      CColor( 0xfff68838 ),
                      CColor( 0xffee3e32 )};

  int start = tval * ( count - 1 );
  float interpolation = tval * ( count - 1 ) - start;
  if ( start == count - 1 )
    return colors[ count - 1 ];

  return Lerp( colors[ start ], colors[ start + 1 ], interpolation );

}

CColor GetCompressionRatioColor( double ratio )
{
  if ( ratio == 0 || isinf( ratio ) || isnan( ratio ) )
    return CColor( 0xff808080 );

  double bits = ratio * 8;
  if ( bits < 0.1 )
    return GetCompressionColorGradient( 0 );// colors[ 0 ];
  if ( bits < 0.5 )
    return GetCompressionColorGradient( 1 ); // colors[ 1 ];
  if ( bits < 1 )
    return GetCompressionColorGradient( 2 ); // colors[ 2 ];
  if ( bits < 2 )
    return GetCompressionColorGradient( 3 ); // colors[ 3 ];
  if ( bits < 3 )
    return GetCompressionColorGradient( 4 ); // colors[ 4 ];
  if ( bits < 5 )
    return GetCompressionColorGradient( 5 ); // colors[ 5 ];
  if ( bits < 7 )
    return GetCompressionColorGradient( 6 ); // colors[ 6 ];
  if ( bits < 9 )
    return GetCompressionColorGradient( 7 ); // colors[ 7 ];
  if ( bits < 12 )
    return GetCompressionColorGradient( 8 ); // colors[ 8 ];
  if ( bits <= 16 )
    return GetCompressionColorGradient( 9 ); // colors[ 9 ];
  return CColor( 0xfffd00fd );// GetCompressionColorGradient( 10 ); // colors[ 10 ];
}

void CKKPDisplay::OnDraw( CWBDrawAPI* API )
{
  ignoreScrollTeleport = false;

  if ( !kkp )
  {
    CWBItem::OnDraw( API );
    return;
  }

  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );

  DrawBackground( API );
  API->SetCropToClient( this );

  int width = GetClientRect().Width();
  int height = GetClientRect().Height();

  CWBFont* f = GetFont( GetState() );
  int charWidth = f->GetWidth( 'x', true );

  int fittingBlocks = width / charWidth - 8; // 1 char slack for spacer

  // block size is 8*3 for hex and space + 8 for nonhex + 2 for spacer at border

  fittingBlocks /= 8 * 3 + 8 + 2;

  CPoint mouse = ScreenToClient( App->GetMousePos() );
  hasMouseHighlight = false;
  mousehighlightbye = 0;

  for ( int pass = 0; pass < 2; pass++ )
  {
    int start = 0;
    int y = 0;
    for ( int i = start; i < kkp->bytes.NumItems(); )
    {
      if ( y + Offset.y < 0 )
      {
        i += fittingBlocks * 8;
        y += f->GetLineHeight();
        continue;
      }
      if ( pass )
        f->Write( API, CString::Format( "%08x", i ), CPoint( 0, y ) + Offset );
      int x = charWidth * 9;
      for ( int j = 0; j < fittingBlocks; j++ )
      {
        for ( int k = 0; k < 8; k++ )
        {
          if ( i >= kkp->bytes.NumItems() )
            break;

          int nonHexLocation = ( fittingBlocks * ( 8 * 3 + 2 ) + j * 8 + k + 9 ) * charWidth;
          int w = k < 7 ? 3 : ( j < fittingBlocks - 1 ? 5 : 2 );
          CRect r1 = CRect( CPoint( x, y ), CPoint( x + charWidth * w, y + f->GetLineHeight() ) ) + Offset;
          CRect rr = CRect( CPoint( nonHexLocation, y ), CPoint( nonHexLocation + charWidth, y + f->GetLineHeight() ) ) + Offset;

          auto& byte = kkp->bytes[ i ];

          if ( !pass && ( r1.Contains( mouse ) || rr.Contains( mouse ) ) )
          {
            hasMouseHighlight = true;
            mousehighlightbye = i;
            Root->SetTooltip( CString::Format( "%x: %s", i, byte.symbol >= 0 ? kkp->symbols[ byte.symbol ].name.GetPointer() : "<no symbol>" ) );
          }

          if ( pass )
          {
            w = k < 7 ? 3 : 2;
            r1 = CRect( CPoint( x, y ), CPoint( x + charWidth * w, y + f->GetLineHeight() ) ) + Offset;

            bool doSelection = false;
            switch ( selection )
            {
            case HighlightMode::Symbol:
              doSelection = byte.symbol == highlightID;
              break;
            case HighlightMode::File:
              doSelection = byte.file == highlightID;
              break;
            case HighlightMode::Line:
              doSelection = byte.file == highlightID && byte.line == lineID;
              break;
            default:
              break;
            }

            int mouseHiglightLevel = 0;
            if ( hasMouseHighlight )
            {
              auto& mouseByte = kkp->bytes[ mousehighlightbye ];
              if ( byte.symbol == mouseByte.symbol )
                mouseHiglightLevel++;
              if ( byte.line == mouseByte.line && byte.file == mouseByte.file && mouseByte.file >= 0 )
                mouseHiglightLevel++;
            }

            if ( doSelection )
            {
              API->DrawRect( r1, CColor( 0x80808080 ) );
              API->DrawRect( rr, CColor( 0x80808080 ) );
            }

            if ( mouseHiglightLevel > 0 )
            {
              CColor col = CColor( 0x80ffffff );
              col.A() = mouseHiglightLevel * 32;
              API->DrawRect( r1, col );
              API->DrawRect( rr, col );
            }

            CColor col = GetCompressionRatioColor( byte.packed );// 0xff808080 );
            int c1 = hexMap[ byte.data >> 4 ];
            int c2 = hexMap[ byte.data & 0x0f ];
            f->WriteChar( API, c1, CPoint( x, y ) + Offset, col ); x += charWidth;
            f->WriteChar( API, c2, CPoint( x, y ) + Offset, col ); x += charWidth;
            f->WriteChar( API, byte.data ? byte.data : '.', CPoint( nonHexLocation, y ) + Offset, col );
          }

          if ( !pass )
            x += charWidth * 2;

          x += charWidth;

          i++;
        }

        if ( pass )
          f->WriteChar( API, '|', CPoint( x, y ) + Offset ); x += charWidth * 2;
      }
      y += f->GetLineHeight();
      if ( y + Offset.y > height )
        break;
    }
  }

  DrawBorder( API );
}

TBOOL CKKPDisplay::MessageProc( CWBMessage& Message )
{
  CWBFont* f = GetFont( GetState() );

  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
  {
    TBOOL b = CWBItem::MessageProc( Message );
    TS32 mi, ma, vi;

    if ( !kkp || !f )
      break;

    UpdateLineCount();

    GetHScrollbarParameters( mi, ma, vi );
    SetHScrollbarParameters( mi, ma, GetClientRect().Width() );
    GetVScrollbarParameters( mi, ma, vi );
    SetVScrollbarParameters( mi, lineCount * f->GetLineHeight(), GetClientRect().Height() );

    if ( GetClientRect().Height() >= f->GetLineHeight() * lineCount )
      SetVScrollbarPos( 0, true );

    return b;
  }
  break;
  case WBM_MOUSEWHEEL:
    SetVScrollbarPos( GetVScrollbarPos() - Message.Data * 3 * f->GetLineHeight(), true );
    return true;
  }

  return CWBItem::MessageProc( Message );
}

void CKKPDisplay::UpdateLineCount()
{
  CWBFont* f = GetFont( GetState() );
  if ( !f || !kkp )
    return;
  int charWidth = f->GetWidth( 'x', true );
  int width = GetClientRect().Width();

  int fittingBlocks = width / charWidth - 8; // 1 char slack for spacer
  fittingBlocks /= 8 * 3 + 8 + 2;

  lineCount = kkp->bytes.NumItems() / ( fittingBlocks * 8 );
  lineCount += kkp->bytes.NumItems() % ( fittingBlocks * 8 ) ? 1 : 0;
}

CKKPDisplay::CKKPDisplay( CWBItem* Parent, const CRect& Pos )
  : CWBItem( Parent, Pos )
{
  Initialize( Parent, Pos );
}

CKKPDisplay::~CKKPDisplay()
{
}

TBOOL CKKPDisplay::Initialize( CWBItem* Parent, const CRect& Position )
{
  EnableVScrollbar( true, true );
  return true;
}

CWBItem* CKKPDisplay::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new CKKPDisplay( Root, Pos );
}

void CKKPDisplay::SetKKP( KKP* _kkp )
{
  kkp = _kkp;
  if ( !kkp )
    return;

  CWBFont* f = GetFont( GetState() );
  if ( !f )
    return;
  UpdateLineCount();
  SetVScrollbarParameters( 0, lineCount * f->GetLineHeight(), GetClientRect().Height() );
}

void CKKPDisplay::SetHighlight( HighlightMode mode, int id )
{
  selection = mode;
  highlightID = id;

  if ( !kkp )
    return;

  if ( mode == HighlightMode::Symbol )
    GoToByte( kkp->symbols[ id ].sourcePos );
}

void CKKPDisplay::SetHighlightLine( int _lineID )
{
  selection = HighlightMode::Line;
  lineID = _lineID;

  if ( !kkp )
    return;

  for (int x=0; x<kkp->bytes.NumItems(); x++ )
    if ( kkp->bytes[ x ].line == _lineID )
    {
      GoToByte( x );
      return;
    }
}

void CKKPDisplay::GoToByte( int byte )
{
  if ( ignoreScrollTeleport )
    return;

  CWBFont* f = GetFont( GetState() );
  if ( !f )
    return;

  int charWidth = f->GetWidth( 'x', true );
  int width = GetClientRect().Width();

  int fittingBlocks = width / charWidth - 8; // 1 char slack for spacer
  fittingBlocks /= 8 * 3 + 8 + 2;

  int line = byte / ( fittingBlocks * 8 );
  SetVScrollbarPos( line * f->GetLineHeight(), true );
}

void CKKPViewer::OpenSourceFile( int fileIndex )
{
  auto* sourceView = FindChildByID<CWBList>( "codeview" );
  if ( !sourceView )
    return;

  if ( currFileIndex == fileIndex )
  {
    if ( clickedLine != sourceView->GetCursorPosition() )
      sourceView->SelectItemByIndex( clickedLine - 1 );
    return;
  }
  currFileIndex = fileIndex;

  sourceView->Flush();

  CStreamReaderMemory file;
  if ( !file.Open( kkp.files[ fileIndex ].name.GetPointer() ) )
    return;

  int lineCount = 0;

  struct LineSizeInfo
  {
    double packed{};
    int unpacked{};
  };

  CDictionary<int, LineSizeInfo> lineSizes;
  for ( int x = 0; x < kkp.bytes.NumItems(); x++ )
  {
    if ( kkp.bytes[ x ].file == fileIndex )
    {
      if ( !lineSizes.HasKey( kkp.bytes[ x ].line ) )
      {
        lineSizes[ kkp.bytes[ x ].line ].packed = 0;
        lineSizes[ kkp.bytes[ x ].line ].unpacked = 0;
      }
      lineSizes[ kkp.bytes[ x ].line ].packed += kkp.bytes[ x ].packed;
      lineSizes[ kkp.bytes[ x ].line ].unpacked++;
    }
  }

  while ( 1 )
  {
    if ( file.eof() )
      break;

    CString line = file.ReadLine();
    lineCount++;

    LineSizeInfo lineSize;
    double packRatio = 0;

    if ( lineSizes.HasKey( lineCount ) )
    {
      lineSize = lineSizes[ lineCount ];
      packRatio = lineSize.packed / (double)lineSize.unpacked;
    }

    if ( lineSize.unpacked > 0 )
      line = CString::Format( "%4d %7.3f ", lineCount, (float)lineSize.packed ) + line;
    else
      line = CString::Format( "%4d         ", lineCount, (float)lineSize.packed ) + line;
    auto id = sourceView->AddItem( line );
    sourceView->GetItem( id )->SetColor( GetCompressionRatioColor( packRatio ) );
  }

  if ( clickedLine >= 0 )
    sourceView->SelectItemByIndex( clickedLine - 1 );

  clickedLine = -1;
}

void CKKPViewer::ExportSymbolList()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "Symbol export\0*.txt\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Export Symbol Pack Info";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "txt";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = Root->GetTargetDirectory( "exportsymbolinfo" );
  if ( GetSaveFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "exportsymbolinfo" );
    App->SelectMouseCursor( CM_WAIT );
    App->FinalizeMouseCursor();
    SetCurrentDirectory( dir );

    CStreamWriterFile f;
    if ( f.Open( opf.lpstrFile ) )
    {

      auto* symbolList = FindChildByID<CWBList>( "symbollist" );
      if ( symbolList )
      {
        for ( int x = 0; x < symbolList->NumItems(); x++ )
        {
          CString text = symbolList->GetItemByIndex( x ).GetText();
          f.Write( text.GetPointer(), text.Length() );
          f.Write( "\r\n", 2 );
        }
      }
    }
  }
  SetCurrentDirectory( dir );
}

void CKKPViewer::LoadKKP( const CString& fname )
{
  CStreamReaderMemory reader;
  if ( !reader.Open( fname.GetPointer() ) )
  {
    LOG_ERR( "Failed to open KKP!" );
    return;
  }

  kkp.Load( reader );

  auto* display = FindChildByID<CKKPDisplay>( "kkpview" );
  if ( !display )
    return;
  display->SetKKP( &kkp );

  RebuildSymbolList();

  auto* fileList = FindChildByID<CWBList>( "filelist" );
  if ( fileList )
  {
    fileList->Flush();
    for ( int x = 0; x < kkp.files.NumItems(); x++ )
      fileList->AddItem( kkp.files[ x ].name, x );
    fileList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                      {
                        CString as = a->GetText();
                        as.ToLower();
                        CString bs = b->GetText();
                        bs.ToLower();
                        if ( as == bs )
                          return 0;
                        return as > bs ? 1 : -1;
                      } );
  }
}

void CKKPViewer::OpenKKP()
{
  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "KKrunchy pack info files\0*.kkp\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open KKP";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "kkp";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = Root->GetTargetDirectory( "openkkp" );

  if ( GetOpenFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "openkkp" );
    SetCurrentDirectory( dir );

    LoadKKP( CString( opf.lpstrFile ) );
  }

  SetCurrentDirectory( dir );
}

void CKKPViewer::LoadSYM( const CString& fname )
{
  if ( !kkp.bytes.NumItems() )
    return;

  CStreamReaderMemory reader;
  if ( !reader.Open( fname.GetPointer() ) )
  {
    LOG_ERR( "Failed to open KKP!" );
    return;
  }

  bool found = false;
  int demoStart = 0;
  int demoSize = 0;

  for ( int x = 0; x < kkp.symbols.NumItems(); x++ )
  {
    if ( kkp.symbols[ x ].name == "demo" )
    {
      found = true;
      demoStart = kkp.symbols[ x ].sourcePos;
      demoSize = kkp.symbols[ x ].size;
      break;
    }
  }

  if ( !found )
  {
    LOG_ERR( "Demo symbol not found in KKP!" );
    return;
  }

  if ( reader.ReadDWord() != 'PXHP' )
    return;

  int dataSize = reader.ReadDWord();

  if ( dataSize != demoSize )
  {
    LOG_ERR( "Demo data size mismatch between .kkp and .sym!" );
    return;
  }

  int symbolCount = reader.ReadDWord();

  int symbolOffset = kkp.symbols.NumItems();

  CArray<CString> symbols;
  for ( int x = 0; x < symbolCount; x++ )
  {
    KKP::KKPSymbol symbol;
    symbol.name = reader.ReadASCIIZ();;
    symbol.isCode = false;
    symbol.fileID = -1;
    symbol.sourcePos = -1;
    kkp.symbols += symbol;
  }

  for ( int x = demoStart; x < demoStart + dataSize; x++ )
  {
    unsigned short symSymbol = reader.ReadWord() + symbolOffset;
    KKP::KKPSymbol& symbol = kkp.symbols[ symSymbol ];

    kkp.bytes[ x ].symbol = symSymbol;
    if ( symbol.sourcePos == -1 )
      symbol.sourcePos = x;
    symbol.size++;
    symbol.packedSize += kkp.bytes[ x ].packed;
    symbol.cumulativeSize = symbol.packedSize;
    symbol.cumulativeUnpackedSize = symbol.size;
  }

  bool* endMatches = new bool[ kkp.symbols.NumItems() ];
  double* endMatchPackedValues = new double[ kkp.symbols.NumItems() ];
  double modelObjectDataPacked = 0;
  double sceneObjectDataPacked = 0;
  int modelObjectDataUnpacked = 0;
  int sceneObjectDataUnpacked = 0;
  int* endMatchUnPackedValues = new int[ kkp.symbols.NumItems() ];
  memset( endMatches, 0, kkp.symbols.NumItems() );
  memset( endMatchPackedValues, 0, kkp.symbols.NumItems() * sizeof( double ) );
  memset( endMatchUnPackedValues, 0, kkp.symbols.NumItems() * sizeof( int ) );

  for ( int x = symbolOffset; x < kkp.symbols.NumItems(); x++ )
  {
    bool endMatch = false;
    CStringArray nameSpaces = kkp.symbols[ x ].name.Explode( "::" );
    endMatchPackedValues[ x ] = kkp.symbols[ x ].packedSize;

    if ( nameSpaces.Last()[ 0 ] == '(' && nameSpaces[ 1 ] == "Models" )
    {
      modelObjectDataPacked += kkp.symbols[ x ].packedSize;
      modelObjectDataUnpacked += kkp.symbols[ x ].size;
    }

    if ( nameSpaces.Last()[ 0 ] == '(' && nameSpaces[ 1 ] == "Scenes" )
    {
      sceneObjectDataPacked += kkp.symbols[ x ].packedSize;
      sceneObjectDataUnpacked += kkp.symbols[ x ].size;
    }

    for ( int y = x + 1; y < kkp.symbols.NumItems(); y++ )
    {
      if ( !endMatches[ x ] && !endMatches[ y ] )
      {
        CStringArray nameSpaces2 = kkp.symbols[ y ].name.Explode( "::" );
        if ( nameSpaces.Last() == nameSpaces2.Last() )
        {
          endMatchPackedValues[ x ] += kkp.symbols[ y ].packedSize;
          endMatchUnPackedValues[ x ] += kkp.symbols[ y ].size;
          endMatch++;
          endMatches[ y ] = true;
        }
      }

      if ( kkp.symbols[ y ].name.Find( kkp.symbols[ x ].name + "::" ) == 0 )
      {
        kkp.symbols[ x ].cumulativeSize += kkp.symbols[ y ].packedSize;
        kkp.symbols[ x ].cumulativeUnpackedSize += kkp.symbols[ y ].size;
      }
    }
  }

  int y = kkp.symbols.NumItems();

  for ( int x = symbolOffset; x < y; x++ )
  {
    if ( !endMatchUnPackedValues[ x ] )
      continue;

    CStringArray nameSpaces = kkp.symbols[ x ].name.Explode( "::" );

    if ( nameSpaces.Last()[ 0 ] == '(' )
      continue;

    KKP::KKPSymbol cummulativeSymbol;
    cummulativeSymbol.cumulativeSize = cummulativeSymbol.packedSize = endMatchPackedValues[ x ];
    cummulativeSymbol.cumulativeUnpackedSize = cummulativeSymbol.size = endMatchUnPackedValues[ x ];
    cummulativeSymbol.name = CString( "64k::CUMULATIVE_END_MATCHES::" ) + nameSpaces.Last();
    kkp.symbols += cummulativeSymbol;
  }

  KKP::KKPSymbol cummulativeSymbol;
  cummulativeSymbol.cumulativeSize = cummulativeSymbol.packedSize = modelObjectDataPacked;
  cummulativeSymbol.cumulativeUnpackedSize = cummulativeSymbol.size = modelObjectDataUnpacked;
  cummulativeSymbol.name = CString( "64k::CUMULATIVE_END_MATCHES::ModelObjects" );
  kkp.symbols += cummulativeSymbol;

  cummulativeSymbol.cumulativeSize = cummulativeSymbol.packedSize = sceneObjectDataPacked;
  cummulativeSymbol.cumulativeUnpackedSize = cummulativeSymbol.size = sceneObjectDataUnpacked;
  cummulativeSymbol.name = CString( "64k::CUMULATIVE_END_MATCHES::SceneObjects" );
  kkp.symbols += cummulativeSymbol;

  delete[] endMatches;
  delete[] endMatchPackedValues;
  delete[] endMatchUnPackedValues;

  RebuildSymbolList();
}

void CKKPViewer::OpenSYM()
{
  if ( !kkp.bytes.NumItems() )
    return;

  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx minimal export sym files\0*.sym\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open SYM";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "sym";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = Root->GetTargetDirectory( "opensym" );

  if ( GetOpenFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "opensym" );
    SetCurrentDirectory( dir );

    LoadSYM( CString( opf.lpstrFile ) );
  }

  SetCurrentDirectory( dir );
}

void CKKPViewer::RebuildSymbolList()
{
  auto* symbolList = FindChildByID<CWBList>( "symbollist" );
  if ( symbolList )
  {
    symbolList->Flush();
    int maxLen = 0;

    for ( int x = 0; x < kkp.symbols.NumItems(); x++ )
    {
      maxLen = max( maxLen, kkp.symbols[ x ].name.Length() + ( kkp.symbols[ x ].isCode ? 6 : 0 ) );
    }

    sortType = 0;
    sortOrder = 0;

    for ( int x = 0; x < kkp.symbols.NumItems(); x++ )
    {
      CString fName = kkp.symbols[ x ].name;
      if ( kkp.symbols[ x ].isCode )
        fName = "Code::" + fName;

      CString name;

      if ( kkp.symbols[ x ].packedSize != kkp.symbols[ x ].cumulativeSize )
        name = CString::Format( CString::Format( "%%-%ds %%6d %%9.3f %%9.3f", maxLen ).GetPointer(), fName.GetPointer(), kkp.symbols[ x ].size, kkp.symbols[ x ].packedSize, kkp.symbols[ x ].cumulativeSize );
      else
        name = CString::Format( CString::Format( "%%-%ds %%6d %%9.3f", maxLen ).GetPointer(), fName.GetPointer(), kkp.symbols[ x ].size, kkp.symbols[ x ].packedSize );

      auto id = symbolList->AddItem( name, x );
      symbolList->GetItem( id )->SetColor( GetCompressionRatioColor( kkp.symbols[ x ].cumulativeSize / (double)kkp.symbols[ x ].cumulativeUnpackedSize ) );
    }
    symbolList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                      {
                        CString as = a->GetText();
                        as.ToLower();
                        CString bs = b->GetText();
                        bs.ToLower();
                        if ( as == bs )
                          return 0;
                        return as > bs ? 1 : -1;
                      } );
  }
}

CKKPViewer::CKKPViewer() : CapexWindow()
{
}

CKKPViewer::CKKPViewer( CWBItem* Parent, const CRect& Pos, CapexWorkBench* WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CKKPViewer::~CKKPViewer()
{

}

KKP* sortKKP = nullptr;
bool gSortOrder;

TBOOL CKKPViewer::MessageProc( CWBMessage& Message )
{
  auto* display = FindChildByID<CKKPDisplay>( "kkpview" );

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton* b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( b )
    {
      if ( b->GetID() == _T( "openkkp" ) )
      {
        OpenKKP();
      }
      if ( b->GetID() == _T( "opensym" ) )
      {
        OpenSYM();
      }
      if ( b->GetID() == _T( "exportsymbollist" ) )
      {
        ExportSymbolList();
      }

      auto* symbolList = FindChildByID<CWBList>( "symbollist" );
      if ( symbolList )
      {

        if ( b->GetID() == _T( "sortbyname" ) )
        {
          if ( sortType == 0 )
            sortOrder = !sortOrder;
          sortType = 0;
          gSortOrder = sortOrder;

          symbolList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                            {
                              CString as = a->GetText();
                              as.ToLower();
                              CString bs = b->GetText();
                              bs.ToLower();
                              if ( as == bs )
                                return 0;
                              return ( as > bs ? 1 : -1 ) * ( gSortOrder ? -1 : 1 );
                            } );
        }
        if ( b->GetID() == _T( "sortbyoffset" ) )
        {
          if ( sortType == 1 )
            sortOrder = !sortOrder;
          sortType = 1;
          gSortOrder = sortOrder;

          sortKKP = &kkp;
          symbolList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                            {
                              return ( sortKKP->symbols[ a->GetData() ].sourcePos - sortKKP->symbols[ b->GetData() ].sourcePos ) * ( gSortOrder ? 1 : -1 );
                            } );
          sortKKP = nullptr;
        }
        if ( b->GetID() == _T( "sortbysize" ) )
        {
          if ( sortType == 2 )
            sortOrder = !sortOrder;
          sortType = 2;
          gSortOrder = sortOrder;

          sortKKP = &kkp;
          symbolList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                            {
                              return ( ( ( sortKKP->symbols[ a->GetData() ].packedSize - sortKKP->symbols[ b->GetData() ].packedSize ) ) * ( gSortOrder ? 1 : -1 ) ) < 0 ? -1 : 1;
                            } );
          sortKKP = nullptr;
        }
        if ( b->GetID() == _T( "sortbyfullsize" ) )
        {
          if ( sortType == 3 )
            sortOrder = !sortOrder;
          sortType = 3;
          gSortOrder = sortOrder;

          sortKKP = &kkp;
          symbolList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                            {
                              return ( ( ( sortKKP->symbols[ a->GetData() ].cumulativeSize - sortKKP->symbols[ b->GetData() ].cumulativeSize ) ) * ( gSortOrder ? 1 : -1 ) ) < 0 ? -1 : 1;
                            } );
          sortKKP = nullptr;
        }
        if ( b->GetID() == _T( "sortbypackratio" ) )
        {
          if ( sortType == 4 )
            sortOrder = !sortOrder;
          sortType = 4;
          gSortOrder = sortOrder;

          sortKKP = &kkp;
          symbolList->Sort( []( CWBSelectableItem* a, CWBSelectableItem* b )
                            {
                              return   ( ( sortKKP->symbols[ a->GetData() ].cumulativeSize/(double)sortKKP->symbols[ a->GetData() ].cumulativeUnpackedSize -
                                           sortKKP->symbols[ b->GetData() ].cumulativeSize/(double)sortKKP->symbols[ b->GetData() ].cumulativeUnpackedSize ) * ( gSortOrder ? 1 : -1 ) ) < 0 ? -1 : 1;
                            } );
          sortKKP = nullptr;
        }
      }

    }
  }
  break;
  case WBM_ITEMSELECTED:
  {
    CWBList* s = (CWBList*)App->FindItemByGuid( Message.GetTarget(), _T( "list" ) );
    if ( s )
    {
      if ( s->GetID() == _T( "symbollist" ) )
      {
        if ( display )
        {
          int symbol = s->GetItem( Message.Data )->GetData();
/*
          if ( kkp.symbols[ symbol ].isCode )
          {
            auto& byte = kkp.bytes[ kkp.symbols[ symbol ].sourcePos ];
            clickedLine = -1;// byte.line;
            OpenSourceFile( byte.file );
          }
*/

          display->SetHighlight( CKKPDisplay::HighlightMode::Symbol, symbol );
        }
      }

      if ( s->GetID() == _T( "filelist" ) )
      {
        if ( display )
        {
          display->SetHighlight( CKKPDisplay::HighlightMode::File, s->GetItem( Message.Data )->GetData() );
          OpenSourceFile( s->GetItem( Message.Data )->GetData() );
        }
      }

      if ( s->GetID() == _T( "codeview" ) )
      {
        if ( display )
          display->SetHighlightLine( s->GetItemIndex( Message.Data ) + 1 );
      }
    }
  }
    break;
  case WBM_LEFTBUTTONDOWN:
  {
    if ( CWBItem::MessageProc( Message ) ) return true;

    if ( App->GetMouseItem() == display && display->hasMouseHighlight )
    {
      auto& byte = kkp.bytes[ display->mousehighlightbye ];
      display->ignoreScrollTeleport = true;
      auto* symbolList = FindChildByID<CWBList>( "symbollist" );
      auto* fileList = FindChildByID<CWBList>( "filelist" );
      auto* codeView = FindChildByID<CWBList>( "codeview" );

      if ( symbolList )
      {
        auto* itm = symbolList->GetItemByUserData( byte.symbol );
        if ( itm )
          symbolList->SelectItem( itm->GetID() );
      }

      if ( byte.file >= 0 )
      {
        auto* itm = fileList->GetItemByUserData( byte.file );
        if ( itm )
          fileList->SelectItem( itm->GetID() );

        clickedLine = byte.line;
      }
    }
      break;
  }
  }

  return CapexWindow::MessageProc( Message );
}

void CKKPViewer::UpdateData()
{

}

void KKP::Load( CStreamReader& reader )
{
  if ( reader.ReadDWord() != '46KK' )
  {
    LOG_ERR( "Invalid KKP File!" );
    return;
  }

  files.Flush();
  symbols.Flush();
  bytes.FlushFast();

  int sourceSize = reader.ReadDWord();
  int fileCount = reader.ReadDWord();
  for ( int x = 0; x < fileCount; x++ )
  {
    KKPFile f;
    f.name = reader.ReadASCIIZ();
    reader.Read( &f.packedSize, 4 );
    f.size = reader.ReadDWord();
    files += f;
  }
  int symbolCount = reader.ReadDWord();
  for ( int x = 0; x < symbolCount; x++ )
  {
    KKPSymbol s;
    s.name = reader.ReadASCIIZ();
    reader.Read( &s.packedSize, 8 );
    s.cumulativeSize = s.packedSize;
    s.size = reader.ReadDWord();
    s.cumulativeUnpackedSize = s.size;
    s.isCode = reader.ReadByte();
    s.fileID = reader.ReadDWord();
    s.sourcePos = reader.ReadDWord();
    symbols += s;
  }

  bytes.AllocateNewUninitialized( sourceSize );

  reader.Read( bytes.GetPointer( 0 ), sourceSize * sizeof( KKPByteData ) );
}