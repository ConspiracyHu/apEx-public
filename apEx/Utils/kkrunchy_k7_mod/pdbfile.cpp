// Written by Fabian "ryg" Giesen.
// I hereby place this code in the public domain.

#include "_types.hpp"
#include "debuginfo.hpp"
#include "pdbfile.hpp"

#include <malloc.h>
#include "windows.h"
#include "ole2.h"

namespace rekkrunchy
{

#include <dia2.h>

/*
  struct PDBFileReader::SectionContrib
  {
    DWORD Section;
    DWORD Offset;
    DWORD Length;
    DWORD Compiland;
    BOOL CodeFlag;
  };

  sU32 PDBFileReader::CompilandFromSectionOffset( sU32 sec, sU32 offs, sBool &codeFlag )
  {
    sInt l, r, x;

    l = 0;
    r = nContribs;

    while ( l < r )
    {
      x = ( l + r ) / 2;
      const SectionContrib &cur = Contribs[ x ];

      if ( sec < cur.Section || sec == cur.Section && offs < cur.Offset )
        r = x;
      else if ( sec > cur.Section || sec == cur.Section && offs >= cur.Offset + cur.Length )
        l = x + 1;
      else // we got a winner
      {
        codeFlag = cur.CodeFlag;
        return cur.Compiland;
      }
    }

    // normally, this shouldn't happen!
    return 0;
  }

  // helpers
  static sChar *BStrToString( BSTR str, sChar *defString = "" )
  {
    if ( !str )
    {
      sInt len = sGetStringLen( defString );
      sChar *buffer = new sChar[ len + 1 ];
      sCopyString( buffer, defString, len + 1 );

      return buffer;
    }
    else
    {
      sInt len = SysStringLen( str );
      sChar *buffer = new sChar[ len + 1 ];

      for ( sInt i = 0; i < len; i++ )
        buffer[ i ] = ( str[ i ] >= 32 && str[ i ] < 128 ) ? str[ i ] : '?';

      buffer[ len ] = 0;

      return buffer;
    }
  }

  static sInt GetBStr( BSTR str, sChar *defString, DebugInfo &to )
  {
    sChar *normalStr = BStrToString( str );
    sInt result = to.MakeString( normalStr );
    delete[] normalStr;

    return result;
  }

  void PDBFileReader::ProcessSymbol( IDiaSymbol *symbol, DebugInfo &to )
  {
    DWORD section, offset, rva;
    enum SymTagEnum tag;
    ULONGLONG length = 0;
    sU32 compilandId;
    IDiaSymbol *compiland = 0;
    BSTR name = 0, sourceName = 0;
    sBool codeFlag;

    symbol->get_symTag( (DWORD *)&tag );
    symbol->get_relativeVirtualAddress( &rva );
    symbol->get_length( &length );
    symbol->get_addressSection( &section );
    symbol->get_addressOffset( &offset );

    // is the previous symbol desperately looking for a length? we can help!
    if ( DanglingLengthStart )
    {
      to.Symbols[ to.Symbols.Count - 1 ].Size = rva - DanglingLengthStart;
      DanglingLengthStart = 0;
    }

    // get length from type for data
    if ( tag == SymTagData )
    {
      IDiaSymbol *type;
      if ( symbol->get_type( &type ) == S_OK )
      {
        type->get_length( &length );
        type->Release();
      }

      // if length still zero, just guess and take number of bytes between
      // this symbol and the next one.
      if ( !length )
        DanglingLengthStart = rva;
    }

    compilandId = CompilandFromSectionOffset( section, offset, codeFlag );
    Session->symbolById( compilandId, &compiland );

    if ( compiland )
      compiland->get_name( &sourceName );

    symbol->get_name( &name );

    // fill out structure
    sChar *nameStr = BStrToString( name, "<no name>" );
    sChar *sourceStr = BStrToString( sourceName, "<no source>" );

    if ( tag == SymTagPublicSymbol )
    {
      length = 0;
      DanglingLengthStart = rva;
    }

    DISymbol *outSym = to.Symbols.Add();
    outSym->Name.Index = outSym->MangledName.Index = to.MakeString( nameStr );
    outSym->FileNum = to.GetFileByName( sourceStr );
    outSym->VA = rva;
    outSym->Size = (sU32)length;
    outSym->Class = codeFlag ? DIC_CODE : DIC_DATA;
    outSym->NameSpNum = to.GetNameSpaceByName( nameStr );

    // clean up
    delete[] nameStr;
    delete[] sourceStr;

    if ( compiland )   compiland->Release();
    if ( sourceName )  SysFreeString( sourceName );
    if ( name )        SysFreeString( name );
  }

  void PDBFileReader::ReadEverything( DebugInfo &to )
  {
    ULONG celt;

    Contribs = 0;
    nContribs = 0;

    DanglingLengthStart = 0;

    // read section table
    IDiaEnumTables *enumTables;
    if ( Session->getEnumTables( &enumTables ) == S_OK )
    {
      VARIANT vIndex;
      vIndex.vt = VT_BSTR;
      vIndex.bstrVal = SysAllocString( L"Sections" );

      IDiaTable *secTable;
      if ( enumTables->Item( vIndex, &secTable ) == S_OK )
      {
        LONG count;

        secTable->get_Count( &count );
        Contribs = new SectionContrib[ count ];
        nContribs = 0;

        IDiaSectionContrib *item;
        while ( SUCCEEDED( secTable->Next( 1, (IUnknown **)&item, &celt ) ) && celt == 1 )
        {
          SectionContrib &contrib = Contribs[ nContribs++ ];

          item->get_addressOffset( &contrib.Offset );
          item->get_addressSection( &contrib.Section );
          item->get_length( &contrib.Length );
          item->get_compilandId( &contrib.Compiland );
          item->get_execute( &contrib.CodeFlag );

          item->Release();
        }

        secTable->Release();
      }

      SysFreeString( vIndex.bstrVal );
      enumTables->Release();
    }

    // enumerate symbols by (virtual) address
    IDiaEnumSymbolsByAddr *enumByAddr;
    if ( SUCCEEDED( Session->getSymbolsByAddr( &enumByAddr ) ) )
    {
      IDiaSymbol *symbol;
      // get first symbol to get first RVA (argh)
      if ( SUCCEEDED( enumByAddr->symbolByAddr( 1, 0, &symbol ) ) )
      {
        DWORD rva;
        if ( symbol->get_relativeVirtualAddress( &rva ) == S_OK )
        {
          symbol->Release();

          // now, enumerate by rva.
          if ( SUCCEEDED( enumByAddr->symbolByRVA( rva, &symbol ) ) )
          {
            do
            {
              ProcessSymbol( symbol, to );
              symbol->Release();

              if ( FAILED( enumByAddr->Next( 1, &symbol, &celt ) ) )
                break;
            } while ( celt == 1 );
          }
        }
        else
          symbol->Release();
      }

      enumByAddr->Release();
    }

    // clean up
    delete[] Contribs;
  }
*/

  /****************************************************************************/

  sBool PDBFileReader::ReadDebugInfo( sChar *fileName, DebugInfo &to )
  {
/*
    sBool readOk = sFALSE;

    if ( FAILED( CoInitialize( 0 ) ) )
      return sFALSE;

    IDiaDataSource *source = 0;
    CoCreateInstance( __uuidof( DiaSource ), 0, CLSCTX_INPROC_SERVER,
                      __uuidof( IDiaDataSource ), (void**)&source );

    if ( source )
    {
      wchar_t wideFileName[ 260 ];
      mbstowcs( wideFileName, fileName, 260 );
      if ( SUCCEEDED( source->loadDataForExe( wideFileName, 0, 0 ) ) )
      {
        if ( SUCCEEDED( source->openSession( &Session ) ) )
        {
          ReadEverything( to );

          readOk = sTRUE;
          Session->Release();
        }
      }

      source->Release();
    }

    CoUninitialize();

    return readOk;
*/
    return true;
  }

}
/****************************************************************************/
