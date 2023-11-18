// Written by Fabian "ryg" Giesen.
// I hereby place this code in the public domain.

#include "_types.hpp"
#include "mapfile.hpp"
#include "_startconsole.hpp"
#include <algorithm>

#include <tchar.h>
//#include <dbghelp.h>
#include <stdio.h>
#pragma comment( lib, "dbghelp.lib" )
#pragma comment( lib, "imagehlp.lib" )

namespace rekkrunchy
{

  // HACK: report buffer is 1mb fixed size

  /****************************************************************************/

  void DebugInfo::Init()
  {
    Strings.Init();
    StringData.Init();
    Symbols.Init();
    Files.Init();
    NameSps.Init();

    BaseAddress = 0;
  }

  void DebugInfo::Exit()
  {
    NameSps.Exit();
    Files.Exit();
    Symbols.Exit();
    StringData.Exit();
    Strings.Exit();
  }

  sInt DebugInfo::MakeString( sChar *string )
  {
    // TODO: write a better implementation using hashing

    sInt i;

    for ( i = 0; i < Strings.Count; i++ )
      if ( !sCmpString( string, &StringData[ Strings[ i ] ] ) )
        return Strings[ i ];

    i = StringData.Count;
    while ( *string )
      *StringData.Add() = *string++;

    *StringData.Add() = 0;
    *Strings.Add() = i;

    return i;
  }

  bool virtAddressComp( const DISymbol &a, const DISymbol &b )
  {
    return a.VA < b.VA;
  }

  void DebugInfo::FinishedReading()
  {
    // fix strings
    for ( sInt i = 0; i < Symbols.Count; i++ )
    {
      DISymbol *sym = &Symbols[ i ];
      sym->Name.String = &StringData[ sym->Name.Index ];
      sym->MangledName.String = &StringData[ sym->MangledName.Index ];
    }

    for ( sInt i = 0; i < Files.Count; i++ )
      Files[ i ].Name.String = &StringData[ Files[ i ].Name.Index ];

    for ( sInt i = 0; i < NameSps.Count; i++ )
      NameSps[ i ].Name.String = &StringData[ NameSps[ i ].Name.Index ];

    // sort symbols by virtual address
    std::sort( &Symbols[ 0 ], &Symbols[ Symbols.Count ], virtAddressComp );

    // remove address double-covers
/*
    sInt symCount = Symbols.Count;
    DISymbol *syms = new DISymbol[ symCount ];
    sCopyMem( syms, &Symbols[ 0 ], symCount * sizeof( DISymbol ) );

    Symbols.Count = 0;
    sU32 oldVA = 0;
    sChar *oldName = "";
    sInt oldSize = 0;

    for ( sInt i = 0; i < symCount; i++ )
    {
      DISymbol *in = &syms[ i ];
      sU32 newVA = in->VA;
      sU32 newSize = in->Size;

      if ( oldVA != 0 )
      {
        sInt adjust = newVA - oldVA;
        if ( adjust < 0 ) // we have to shorten
        {
          newVA = oldVA;
          if ( newSize >= -adjust )
            newSize += adjust;
        }
      }

      if ( newSize || in->Class == DIC_END )
      {
        DISymbol *out = Symbols.Add();
        *out = *in;
        out->VA = newVA;
        out->Size = newSize;

        oldVA = newVA + newSize;
        oldSize = newSize;
        oldName = in->Name.String;
      }
    }

    delete[] syms;
*/
  }

  void DebugInfo::Rebase( sU32 newBase )
  {
    sU32 delta = newBase - BaseAddress;

    for ( sInt i = 0; i < Symbols.Count; i++ )
      Symbols[ i ].VA += delta;

    BaseAddress = newBase;
  }

  sInt DebugInfo::GetFile( sInt name )
  {
    for ( sInt i = 0; i < Files.Count; i++ )
      if ( Files[ i ].Name.Index == name )
        return i;

    DISymFile *file = Files.Add();
    file->Name.Index = name;
    file->PackedSize = 0;
    file->Size = 0;

    fileNameDict[ std::string( &StringData[ name ] ) ] = Files.Count - 1;

    return Files.Count - 1;
  }

  sInt DebugInfo::GetFileByName( sChar *name )
  {
    sChar *p;

    // skip path seperators
    while ( ( p = (sChar *)sFindString( name, "\\" ) ) )
      name = p + 1;

    while ( ( p = (sChar *)sFindString( name, "/" ) ) )
      name = p + 1;

    int x = GetFile( MakeString( name ) );

    return x;
  }

  sInt DebugInfo::GetNameSpace( sInt name )
  {
    for ( sInt i = 0; i < NameSps.Count; i++ )
      if ( NameSps[ i ].Name.Index == name )
        return i;

    DISymNameSp *namesp = NameSps.Add();
    namesp->Name.Index = name;
    namesp->PackedSize = 0;
    namesp->Size = 0;

    return NameSps.Count - 1;
  }

  sInt DebugInfo::GetNameSpaceByName( sChar *name )
  {
    sChar *pp = name - 2;
    sChar *p;
    sInt cname;

    while ( ( p = (sChar *)sFindString( pp + 2, "::" ) ) )
      pp = p;

    while ( ( p = (sChar *)sFindString( pp + 1, "." ) ) )
      pp = p;

    if ( pp != name - 2 )
    {
      sChar buffer[ 2048 ];
      sCopyString( buffer, name, 2048 );

      if ( pp - name < 2048 )
        buffer[ pp - name ] = 0;

      cname = MakeString( buffer );
    }
    else
      cname = MakeString( "<global>" );

    return GetNameSpace( cname );
  }

  void DebugInfo::StartAnalyze( sU32 startAddress, ReorderBuffer *reorder )
  {
    sInt i;

    Address = startAddress;
    Reorder = reorder;
    Reorder->Finish();

    for ( i = 0; i < Symbols.Count; i++ )
      Symbols[ i ].PackedSize = 0.0f;

    for ( i = 0; i < Files.Count; i++ )
    {
      Files[ i ].Size = 0;
      Files[ i ].PackedSize = 0.0f;
    }

    for ( i = 0; i < NameSps.Count; i++ )
    {
      NameSps[ i ].Size = 0;
      NameSps[ i ].PackedSize = 0.0f;
    }
  }

  void DebugInfo::TokenizeCallback( void *user, sInt uncompSize, sF32 compSize )
  {
    DebugInfo *info;
    DISymbol *sym;
    sU32 addr;
    sInt leftSize, availSize, len;
    ReorderItem *reord;

    info = (DebugInfo *)user;
    leftSize = uncompSize;

    while ( leftSize )
    {
      // setup
      availSize = leftSize;
      addr = info->Address;

      // find out whether there is a remapping for the current address range
      if ( info->Reorder )
      {
        if ( !info->Reorder->Find( addr, &reord ) ) // no match
        {
          if ( reord )
            len = sMin<sInt>( availSize, reord->NewVA - addr );
          else
            len = availSize;

          info->Address += len;
          leftSize -= len;
        }
        else
        {
          len = sMin<sInt>( availSize, reord->NewVA + reord->NewSize - addr );
          info->Address += len;
          leftSize -= len;

          // just a crude approximation here, gotta fix it somehow
          availSize = reord->OldSize * len / reord->NewSize;
          addr = reord->OldVA + ( reord->OldSize * ( addr - reord->NewVA ) + reord->NewSize / 2 ) / reord->NewSize;
        }
      }
      else
      {
        info->Address += availSize;
        leftSize -= availSize;
      }

      // try to find symbol for current address
      while ( availSize )
      {
        if ( !info->FindSymbol( addr, &sym ) ) // no match
        {
          if ( sym ) // skip to next symbol or everything if there's no next symbol
            len = sMin<sInt>( availSize, sym->VA - addr );
          else
            len = availSize;

          addr += len;
          availSize -= len;
        }
        else // match with symbol
        {
          len = sMin<sInt>( availSize, sym->VA + sym->Size - addr );
          addr += len;
          availSize -= len;

          sym->PackedSize += compSize * 0.125f * len / uncompSize;
        }
      }
    }
  }

  void DebugInfo::FinishAnalyze()
  {
    sInt i;

    for ( i = 0; i < Symbols.Count; i++ )
    {
      if ( Symbols[ i ].Class != DIC_END )
      {
        Files[ Symbols[ i ].FileNum ].Size += Symbols[ i ].Size;
        Files[ Symbols[ i ].FileNum ].PackedSize += Symbols[ i ].PackedSize;
        NameSps[ Symbols[ i ].NameSpNum ].Size += Symbols[ i ].Size;
        NameSps[ Symbols[ i ].NameSpNum ].PackedSize += Symbols[ i ].PackedSize;
      }
    }
  }

  sBool DebugInfo::FindSymbol( sU32 VA, DISymbol **sym )
  {
    sInt l, r, x;

    l = 0;
    r = Symbols.Count;
    while ( l < r )
    {
      x = ( l + r ) / 2;

      if ( VA < Symbols[ x ].VA )
        r = x; // continue in left half
      else if ( VA >= Symbols[ x ].VA + Symbols[ x ].Size )
        l = x + 1; // continue in left half
      else
      {
        *sym = &Symbols[ x ]; // we found a match
        return sTRUE;
      }
    }

    *sym = ( l + 1 < Symbols.Count ) ? &Symbols[ l + 1 ] : 0;
    return sFALSE;
  }

  static sChar ReportBuffer[ 1024 * 1024 ];

  bool symPackedSizeComp( const DISymbol &a, const DISymbol &b )
  {
    return a.PackedSize > b.PackedSize;
  }

  bool namePackedSizeComp( const DISymNameSp &a, const DISymNameSp &b )
  {
    return a.PackedSize > b.PackedSize;
  }

  bool filePackedSizeComp( const DISymFile &a, const DISymFile &b )
  {
    return a.PackedSize > b.PackedSize;
  }

  struct CImageHlpLine64 : public IMAGEHLP_LINE64
  {
    CImageHlpLine64()
    {
      SizeOfStruct = sizeof( IMAGEHLP_LINE64 );
    }
  };

  void DebugInfo::SetLineData( int symbolIdx, int symbolOffset, int imageStart )
  {
    int pos = Symbols[ symbolIdx ].sourcePos + symbolOffset;
    int VA = Symbols[ symbolIdx ].VA + symbolOffset - imageStart;

    DWORD LineDisplacement = 0;
    CImageHlpLine64 LineInfo;
    if ( SymGetLineFromAddr64( GetCurrentProcess(), VA, &LineDisplacement, &LineInfo ) )
    {
      lineMap[ pos ] = LineInfo.LineNumber;
      fileNameMap[ pos ] = fileNameDict[ std::string( LineInfo.FileName ) ];
    }
  }

  sChar* DebugInfo::WriteReport()
  {
    sChar *Report;
    sInt i, j;
    sU32 size;
    sF32 pSize;

    Report = ReportBuffer;
    sSPrintF( Report, 512, sAPPNAME " " sVERSION " >> pack ratio report\n\n" );
    Report += sGetStringLen( Report );

    sSPrintF( Report, 512, "Functions by packed size:\n" );
    Report += sGetStringLen( Report );

    std::sort( &Symbols[ 0 ], &Symbols[ Symbols.Count ], symPackedSizeComp );

    for ( i = 0; i < Symbols.Count; i++ )
    {
      if ( Symbols[ i ].Class == DIC_CODE )
      {
        sSPrintF( Report, 512, "%5d.%02d/%8d: %8x %-70s %s\n",
                  (sInt)Symbols[ i ].PackedSize, sInt( Symbols[ i ].PackedSize * 100 ) % 100,
                  Symbols[ i ].Size, Symbols[ i ].sourcePos, Symbols[ i ].Name, Files[ Symbols[ i ].FileNum ].Name );

        Report += sGetStringLen( Report );
      }
    }

    sSPrintF( Report, 512, "\nData by packed size:\n" );
    Report += sGetStringLen( Report );
    for ( i = 0; i < Symbols.Count; i++ )
    {
      if ( Symbols[ i ].Class == DIC_DATA )
      {
        sSPrintF( Report, 512, "%5d.%02d/%8d: %8x %-70s %s\n",
                  (sInt)Symbols[ i ].PackedSize, sInt( Symbols[ i ].PackedSize * 100 ) % 100,
                  Symbols[ i ].Size, Symbols[ i ].sourcePos, Symbols[ i ].Name, Files[ Symbols[ i ].FileNum ].Name );

        Report += sGetStringLen( Report );
      }
    }

    sSPrintF( Report, 512, "\nFunctions by object file and packed size:\n" );
    Report += sGetStringLen( Report );

    for ( i = 1; i < Symbols.Count; i++ )
      for ( j = i; j > 0; j-- )
      {
        sInt f1 = Symbols[ j ].FileNum;
        sInt f2 = Symbols[ j - 1 ].FileNum;

        if ( f1 == -1 || f2 != -1 && sCmpStringI( Files[ f1 ].Name.String, Files[ f2 ].Name.String ) < 0 )
          sSwap( Symbols[ j ], Symbols[ j - 1 ] );
      }

    for ( i = 0; i < Symbols.Count; i++ )
    {
      if ( Symbols[ i ].Class == DIC_CODE )
      {
        sSPrintF( Report, 512, "%5d.%02d/%8d: %-50s %s\n",
                  (sInt)Symbols[ i ].PackedSize, sInt( Symbols[ i ].PackedSize * 100 ) % 100,
                  Symbols[ i ].Size, Symbols[ i ].Name, Files[ Symbols[ i ].FileNum ].Name );

        Report += sGetStringLen( Report );
      }
    }

    sSPrintF( Report, 512, "\nClasses/Namespaces by packed size:\n" );
    Report += sGetStringLen( Report );
    std::sort( &NameSps[ 0 ], &NameSps[ NameSps.Count ], namePackedSizeComp );

    for ( i = 0; i < NameSps.Count; i++ )
    {
      sSPrintF( Report, 512, "%5d.%02d/%8d: %s\n", sInt( NameSps[ i ].PackedSize ),
                sInt( NameSps[ i ].PackedSize * 100 ) % 100, NameSps[ i ].Size, NameSps[ i ].Name );
      Report += sGetStringLen( Report );
    }

    sSPrintF( Report, 512, "\nObject files by packed size:\n" );
    Report += sGetStringLen( Report );
    std::sort( &Files[ 0 ], &Files[ Files.Count ], filePackedSizeComp );

    for ( i = 0; i < Files.Count; i++ )
    {
      sSPrintF( Report, 512, "%5d.%02d/%8d: %s\n", sInt( Files[ i ].PackedSize ),
                sInt( Files[ i ].PackedSize * 100 ) % 100, Files[ i ].Size, Files[ i ].Name );
      Report += sGetStringLen( Report );
    }

    size = 0;
    pSize = 0.0f;
    for ( i = 0; i < Symbols.Count; i++ )
    {
      if ( Symbols[ i ].Class == DIC_CODE )
      {
        size += Symbols[ i ].Size;
        pSize += Symbols[ i ].PackedSize;
      }
    }

    sSPrintF( Report, 512, "\nOverall code: %5d.%02d/%8d\n", sInt( pSize ),
              sInt( pSize * 100 ) % 100, size );
    Report += sGetStringLen( Report );

    size = 0;
    pSize = 0.0f;
    for ( i = 0; i < Symbols.Count; i++ )
    {
      if ( Symbols[ i ].Class == DIC_DATA )
      {
        size += Symbols[ i ].Size;
        pSize += Symbols[ i ].PackedSize;
      }
    }

    sSPrintF( Report, 512, "Overall data: %5d.%02d/%8d\n", sInt( pSize ),
              sInt( pSize * 100 ) % 100, size );
    Report += sGetStringLen( Report );

    return sDupString( ReportBuffer, 1 );
  }

  void DebugInfo::DumpPackData( char* fileName, unsigned char* sourceData, int sourceSize )
  {
    FILE* f = fopen( fileName, "w+b" );
    if ( !f )
      return;

    fwrite( "KK64", 4, 1, f );
    fwrite( &sourceSize, 4, 1, f );
    fwrite( &Files.Count, 4, 1, f );
    for ( int x = 0; x < Files.Count; x++ )
    {
      char* fName = *(char**)&Files[ x ].Name;
      do
      {
        fwrite( fName, 1, 1, f );
      } while ( *fName++ );

      fwrite( &Files[ x ].PackedSize, 4, 1, f );
      fwrite( &Files[ x ].Size, 4, 1, f );
    }
    fwrite( &Symbols.Count, 4, 1, f );
    for ( int x = 0; x < Symbols.Count; x++ )
    {
      auto& s = Symbols[ x ];
      char* sName = *(char**)&s.Name;
      do
      {
        fwrite( sName, 1, 1, f );
      } while ( *sName++ );
      fwrite( &s.PackedSize, 8, 1, f );
      fwrite( &s.Size, 4, 1, f );
      bool isCode = s.Class == DIC_CODE;
      fwrite( &isCode, 1, 1, f );
      fwrite( &s.FileNum, 4, 1, f );
      fwrite( &s.sourcePos, 4, 1, f );
    }

    for ( int x = 0; x < sourceSize; x++ )
    {
      struct ByteInfo
      {
        unsigned char data;
        short symbol;
        double packed;
        short line;
        short file;
      } data;
      data.data = sourceData[ x ];
      data.symbol = symbolMap[ x ];
      data.packed = packedSizeMap[ x ];
      data.file = fileNameMap[ x ];
      data.line = lineMap[ x ];
      fwrite( &data, sizeof( ByteInfo ), 1, f );
    }

    fclose( f );
  }

  int DebugInfo::ImageRvaToVa( int rva )
  {
    DWORD* dNameRVAs( 0 );
    int res = (int)::ImageRvaToVa( LoadedImage.FileHeader,
                                      LoadedImage.MappedAddress,
                                      rva, NULL );

    if ( res )
      res -= (int)LoadedImage.MappedAddress;
    return res;
  }

  struct CSymbolInfoPackage : public SYMBOL_INFO_PACKAGE
  {
    CSymbolInfoPackage()
    {
      si.SizeOfStruct = sizeof( SYMBOL_INFO );
      si.MaxNameLen = sizeof( name );
    }
  };

  BOOL CALLBACK enum_proc( SYMBOL_INFO* info, ULONG size, void* param )
  {
    DebugInfo* nfo = (DebugInfo*)param;

    auto sym = nfo->Symbols.Add();
    sym->Name.Index = sym->MangledName.Index = nfo->MakeString( info->Name );
    sym->NameSpNum = nfo->GetNameSpaceByName( info->Name );
    sym->FileNum = 0;
    sym->VA = info->Address;
    sym->Size = info->Size;
    sym->Class = 0;
    if ( info->Tag == SymTagData || info->Tag == SymTagPublicSymbol )
      sym->Class = DIC_DATA;
    if ( info->Tag == SymTagFunction )
      sym->Class = DIC_CODE;

    if ( sym->Class == 0 )
      int x = 0;

    if ( !strcmp( info->Name, "demo" ) )
      int x = 0;

    if ( !strcmp( info->Name, "affectors" ) )
      int x = 0;

    if ( sym->Size == 0 )
    {
      ULONG64 typeLength;
      if ( SymGetTypeInfo( GetCurrentProcess(), info->ModBase, info->TypeIndex, TI_GET_LENGTH, &typeLength ) )
        sym->Size = typeLength;

    }

    sym->FileNum = -1;

    CImageHlpLine64 LineInfo;
    DWORD LineDisplacement = 0;
    if ( SymGetLineFromAddr64( GetCurrentProcess(), info->Address, &LineDisplacement, &LineInfo ) )
    {
      if ( LineInfo.FileName )
        sym->FileNum = nfo->GetFile( nfo->MakeString( LineInfo.FileName ) );
    }

    if ( sym->FileNum == -1 )
      sym->FileNum = nfo->GetFile( nfo->MakeString( "<no source>" ) );

/*
    sym_info_t* sym_info;

    sym_info = (sym_info_t*)pool_alloc( &g_symbol_pool, sizeof( sym_info_t ) );
    dbghelp_to_sym_info( info, sym_info );

    if ( !( g_sym_count % 100 ) )
    {
      print_info( "\r%d", g_sym_count );
    }
    ++g_sym_count;
*/

    return TRUE;
  }

  bool DebugInfo::OpenPDB( char* fileName )
  {
      Init();
      SymInitialize( GetCurrentProcess(), NULL, FALSE );
      ModBase = SymLoadModule64( GetCurrentProcess(), NULL, fileName, NULL, 0, 0 );
      if ( !ModBase )
        return false;

      CSymbolInfoPackage sip; // it contains SYMBOL_INFO structure plus additional 
                                  // space for the name of the symbol 

      DWORD64 Displacement = 0;

      if ( !SymEnumSymbols( GetCurrentProcess(), ModBase, "*!*", enum_proc, this ) )
        return false;

      if ( !Symbols.Count )
        return false;

      if ( !MapAndLoad( fileName, NULL, &LoadedImage, TRUE, TRUE ) )
        return false;

      int symStart = 0;

      for ( int i = symStart + 1; i < Symbols.Count; i++ )
        for ( int j = i; j > symStart; j-- )
          if ( Symbols[ j ].VA < Symbols[ j - 1 ].VA )
            sSwap( Symbols[ j ], Symbols[ j - 1 ] );

      for ( int i = symStart; i < Symbols.Count; i++ )
      {
        DISymbol* sym = &Symbols[ i ];

        if ( sym->Class != DIC_END && i < Symbols.Count - 1 )
        {
          sVERIFY( i != Symbols.Count - 1 );
          if ( !sym->Size )
            sym->Size = sym[ 1 ].VA - sym->VA;
        }
      }

      return true;
  }

  /****************************************************************************/

  ReorderBuffer::ReorderBuffer()
  {
    Reorder.Init();
  }

  ReorderBuffer::~ReorderBuffer()
  {
    Reorder.Exit();
  }

  void ReorderBuffer::Add( sU32 newVA, sU32 newSize, sU32 oldVA, sU32 oldSize )
  {
    ReorderItem *item;

    item = Reorder.Add();
    item->NewVA = newVA;
    item->NewSize = newSize;
    item->OldVA = oldVA;
    item->OldSize = oldSize;
  }

  static bool reorderComp( const ReorderItem &a, const ReorderItem &b )
  {
    return a.NewVA < b.NewVA;
  }

  void ReorderBuffer::Finish()
  {
    // sort by new start address
    // (these are *lots* of records, so use a decent sort)
    std::sort( &Reorder[ 0 ], &Reorder[ Reorder.Count ], reorderComp );

    // just to be sure, assert we have no overlaps
    for ( sInt i = 0; i < Reorder.Count - 1; i++ )
      sVERIFY( Reorder[ i ].NewVA + Reorder[ i ].NewSize <= Reorder[ i + 1 ].NewVA );
  }

  sBool ReorderBuffer::Find( sU32 addr, ReorderItem **reord )
  {
    sInt l, r, x;

    l = 0;
    r = Reorder.Count;
    while ( l < r )
    {
      x = ( l + r ) / 2;
      if ( addr < Reorder[ x ].NewVA )
        r = x; // continue in left half
      else if ( addr >= Reorder[ x ].NewVA + Reorder[ x ].NewSize )
        l = x + 1; // continue in right half
      else
      {
        *reord = &Reorder[ x ]; // we got a match
        return sTRUE;
      }
    }

    *reord = ( l + 1 < Reorder.Count ) ? &Reorder[ l + 1 ] : 0;
    return sFALSE;
  }

}
/****************************************************************************/
