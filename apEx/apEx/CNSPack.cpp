#include "BasePCH.h"
#include "CNSPack.h"
#include "../../Bedrock/UtilLib/miniz.c"

#define TOCSIZEMASK   0x2E33B4FD 
#define TOCMASK       0xB3398B42
#define TOCBUFFERSIZE 1024*1024

#define HASHSEED 0x811C9DC5
#define HASHMULT 0x1000193

struct CNSPackedTOCEntry
{
  TU32 Hash;
  TU32 PackedSize;
  TU32 UnpackedSize;
};

TU32 HashFunct( TS8 *String )
{
  TU32 x = 0;

  TU32 Hash = HASHSEED;
  do
  {
    Hash = HASHMULT * ( tolower( String[ x++ ] ) ^ Hash );
  } while ( x < strlen( String ) );

  return Hash;
}

CNSArchive::CNSArchive()
{
  Archive = NULL;
}

CNSArchive::~CNSArchive()
{
  if ( Archive ) fclose( Archive );
}

CNSArchiveError CNSArchive::OpenArchive( TS8 *ArchiveName )
{
  if ( Archive ) return ArchiveAlreadyOpen;

  if ( fopen_s( &Archive, ArchiveName, "rb" ) ) return ErrorOpeningArchive;
  if ( !Archive ) return ErrorOpeningArchive;

  TU32 cons, pira;
  TU16 cy;

  if ( fread( &cons, 4, 1, Archive ) != 1 )
  {
    fclose( Archive );
    return InvalidArchive;
  }
  if ( fread( &pira, 4, 1, Archive ) != 1 )
  {
    fclose( Archive );
    return InvalidArchive;
  }
  if ( fread( &cy, 2, 1, Archive ) != 1 )
  {
    fclose( Archive );
    return InvalidArchive;
  }

  if ( !( cons == 'SNOC' && pira == 'ARIP' && cy == 'YC' ) )
  {
    fclose( Archive );
    return InvalidArchive;
  }

  TU32 PackedTocSize;
  if ( fread( &PackedTocSize, 4, 1, Archive ) != 1 )
  {
    fclose( Archive );
    return InvalidArchive;
  }
  PackedTocSize ^= TOCSIZEMASK;

  if ( !PackedTocSize ) return AllOk; //empty archive

  TU8 *PackedToc = new TU8[ PackedTocSize ];

  if ( fread( PackedToc, PackedTocSize, 1, Archive ) != 1 )
  {
    delete[] PackedToc;
    fclose( Archive );
    return InvalidArchive;
  }

  TU32 *pt = (TU32*)PackedToc;
  for ( TU32 x = 0; x < PackedTocSize / 4; x++ )
  {
    *pt ^= TOCMASK;
    pt++;
  }

  TU8 *cTOC = new TU8[ TOCBUFFERSIZE ];
  memset( cTOC, 0, TOCBUFFERSIZE );

  unsigned long TocSize = TOCBUFFERSIZE;

  if ( uncompress( cTOC, &TocSize, PackedToc, PackedTocSize ) != Z_OK )
  {
    delete[] PackedToc;
    delete[] cTOC;
    fclose( Archive );
    return DecompressionError;
  }

  delete[] PackedToc;

  CNSPackedTOCEntry *rTOC = (CNSPackedTOCEntry*)cTOC;

  TS32 FileCount = TocSize / sizeof( CNSPackedTOCEntry );

  TS32 Offset = 14 + PackedTocSize;

  for ( TS32 x = 0; x < FileCount; x++ )
  {
    CNSArchiveTocEntry TOCEntry;
    TOCEntry.Hash = rTOC[ x ].Hash;
    TOCEntry.PackedSize = rTOC[ x ].PackedSize;
    TOCEntry.UnpackedSize = rTOC[ x ].UnpackedSize;
    TOCEntry.Offset = Offset;
    Offset += TOCEntry.PackedSize;
    TOC.Add( TOCEntry );
  }

  delete[] cTOC;
  return AllOk;
}

CNSArchiveError CNSArchive::ReadFile( TS8 *FileName, TU8 *&Result, unsigned long &FileSize )
{
  if ( !Archive ) return ArchiveNotOpen;
  TU32 Hash = HashFunct( FileName );
  return ReadFileByHash( Hash, Result, FileSize );
}

CNSArchiveError CNSArchive::ReadFileByHash( TU32 Hash, TU8 *&Result, unsigned long &FileSize )
{
  if ( !Archive ) return ArchiveNotOpen;
  for ( TS32 x = 0; x < TOC.NumItems(); x++ )
  {
    if ( TOC[ x ].Hash == Hash )
    {
      Result = NULL;

      if ( fseek( Archive, TOC[ x ].Offset, SEEK_SET ) )
      {
        return CorruptArchive;
      }

      TU8 *CompressedFile = new TU8[ TOC[ x ].PackedSize ];

      if ( fread( CompressedFile, TOC[ x ].PackedSize, 1, Archive ) != 1 )
      {
        delete[] CompressedFile;
        return CorruptArchive;
      }

      Result = new TU8[ TOC[ x ].UnpackedSize ];

      FileSize = TOC[ x ].UnpackedSize;
      if ( uncompress( Result, &FileSize, CompressedFile, TOC[ x ].PackedSize ) != Z_OK )
      {
        delete[] CompressedFile;
        delete[] Result;
        Result = NULL;
        FileSize = 0;
        return DecompressionError;
      }

      delete[] CompressedFile;

      return AllOk;
    }
  }

  return FileNotInArchive;
}

TS32 CNSArchive::GetFileCount()
{
  return TOC.NumItems();
}

CNSArchiveTocEntry CNSArchive::GetTOCEntry( TS32 File )
{
  return TOC[ File ];
}

CNSArchiveFileData::CNSArchiveFileData()
{
  Data = NULL;
  FileName = NULL;
  FileSize = 0;
}

CNSArchiveFileData::~CNSArchiveFileData()
{
  if ( Data ) delete[] Data;
  if ( FileName ) delete[] FileName;
  Data = NULL;
  FileName = NULL;
  FileSize = 0;
}

void CNSArchiveFileData::SetData( TU8 *data, TS32 filesize )
{
  if ( !data ) return;
  if ( filesize <= 0 ) return;
  Data = new TU8[ filesize ];
  memcpy( Data, data, filesize );
  FileSize = filesize;
}

void CNSArchiveFileData::SetFileName( TS8 *filename )
{
  if ( !filename ) return;
  if ( !strlen( filename ) ) return;
  FileName = new TS8[ strlen( filename ) + 1 ];
  memset( FileName, 0, strlen( filename ) + 1 );
  memcpy( FileName, filename, strlen( filename ) );
}

TBOOL CNSArchiveFileData::isValid()
{
  return ( FileSize > 0 && FileName && Data );
}

TU8 *CNSArchiveFileData::GetData()
{
  return Data;
}

TS32 CNSArchiveFileData::GetFileSize()
{
  return FileSize;
}

TS8 *CNSArchiveFileData::GetFileName()
{
  return FileName;
}


//////////////////////////////////////////////////////////////////////////
// packer

class cnspackCFileEntry
{
public:
  TS8 *FullPath;
  TS8 *ShortPath;

  cnspackCFileEntry()
  {
    FullPath = ShortPath = NULL;
  }
  ~cnspackCFileEntry()
  {
    if ( FullPath ) delete[] FullPath;
    if ( ShortPath ) delete[] ShortPath;
  }
};

class OpenedFile
{
public:

  unsigned long PackedSize;
  TU8 *PackedData;

  OpenedFile()
  {
    PackedSize = 0;
    PackedData = NULL;
  }
  ~OpenedFile()
  {
    if ( PackedData ) delete[] PackedData;
  }
};

TU8 *GeCCo_ReadFile( TS8 *Filename, unsigned long &Size )
{
  Size = 0;
  FILE *f = NULL;
  if ( fopen_s( &f, Filename, "rb" ) ) return NULL;
  if ( !f ) return NULL;
  fseek( f, 0, SEEK_END );
  long s;
  s = ftell( f );
  Size = (TS32)s;
  fseek( f, 0, SEEK_SET );
  TU8 *c = new TU8[ Size ];
  fread( c, Size, 1, f );
  fclose( f );
  return c;
}

CNSArchiveError BuildNewArchiveFromFiles( TS8 *ArchiveFile, TS8 *Path, TBOOL RecurseDirs, TBOOL Verbose )
{
  CFileList *FileList = new CFileList( "*.*", Path, RecurseDirs );
  if ( !FileList->Files.NumItems() )
  {
    delete FileList;
    return NoFilesFound;
  }

  if ( Verbose )
    LOG( LOG_INFO, _T( "[pack] Reading files..." ) );

  CArray<CNSArchiveFileData*> Files;
  for ( TS32 x = 0; x < FileList->Files.NumItems(); x++ )
  {
    unsigned long UnpackedSize = 0;

    if ( Verbose )
      LOG( LOG_INFO, _T( "[pack] Reading file %s... " ), CString( FileList->Files[ x ].FileName ).GetPointer() );

    CStreamReaderMemory m;
    if ( m.Open( ( FileList->Files[ x ].Path + FileList->Files[ x ].FileName ).GetPointer() ) )
    {
      CNSArchiveFileData *f = new CNSArchiveFileData();
      f->SetData( m.GetData(), (TS32)m.GetLength() );
      f->SetFileName( FileList->Files[ x ].FileName.GetPointer() );
      Files.Add( f );
      if ( Verbose )
        LOG( LOG_INFO, _T( "[pack] OK" ) );
    }
    else
    {
      if ( Verbose )
        LOG( LOG_INFO, _T( "[pack] SKIPPED" ) );
    }
  }

  CNSArchiveError result = BuildNewArchiveFromMemory( ArchiveFile, Files.GetPointer( 0 ), Files.NumItems(), Verbose );

  Files.FreeArray();
  delete FileList;
  return result;
}

CNSArchiveError BuildNewArchiveFromMemory( TS8 *ArchiveFile, CNSArchiveFileData *FileList[], TS32 FileCount, TBOOL Verbose )
{
  if ( FileCount <= 0 ) return NoFilesFound;

  FILE *Archive = NULL;
  if ( fopen_s( &Archive, ArchiveFile, "w+b" ) )
  {
    return ErrorOpeningArchive;
  }
  if ( !Archive ) return ErrorOpeningArchive;

  if ( Verbose )
    LOG( LOG_INFO, _T( "[pack] Processing files..." ) );

  CArray<CNSPackedTOCEntry> NewTOC;
  CArray<OpenedFile*> Files;

  TU32 ErrorCount = 0;

  for ( TS32 x = 0; x < FileCount; x++ )
    if ( !FileList[ x ]->isValid() )
    {
      ErrorCount++;
    }
    else
    {
      CNSPackedTOCEntry TocEntry;
      TocEntry.Hash = HashFunct( FileList[ x ]->GetFileName() );

      for ( TS32 y = 0; y < NewTOC.NumItems(); y++ )
        if ( NewTOC[ y ].Hash == TocEntry.Hash )
        {
          if ( Verbose )
            LOG( LOG_WARNING, _T( "[pack] Hash Collision on %s!" ), FileList[ x ]->GetFileName() );
          ErrorCount++;
          continue;
        }

      OpenedFile *f = new OpenedFile();

      unsigned long UnpackedSize = FileList[ x ]->GetFileSize();
      TU8 *UnpackedData = FileList[ x ]->GetData();

      f->PackedSize = compressBound( UnpackedSize );
      f->PackedData = new TU8[ f->PackedSize ];

      if ( compress( f->PackedData, &f->PackedSize, UnpackedData, UnpackedSize ) )
      {
        if ( Verbose )
          LOG( LOG_WARNING, _T( "[pack] Compression error on %s!" ), FileList[ x ]->GetFileName() );
        ErrorCount++;
        delete f;
        delete[] UnpackedData;
        continue;
      }

      if ( Verbose )
        LOG( LOG_INFO, _T( "[pack] %s OK! - %.8x" ), FileList[ x ]->GetFileName(), TocEntry.Hash );
      TocEntry.UnpackedSize = UnpackedSize;
      TocEntry.PackedSize = f->PackedSize;

      Files.Add( f );
      NewTOC.Add( TocEntry );
    }

  if ( Verbose )
    LOG( LOG_INFO, _T( "[pack] Building archive..." ) );

  fwrite( "CONSPIRACY", 10, 1, Archive );

  unsigned long UnpackedTOCSize = NewTOC.NumItems() * sizeof( CNSPackedTOCEntry );
  unsigned long PackedTOCSize = compressBound( UnpackedTOCSize );
  TU8 *PackedTOC = new TU8[ PackedTOCSize ];
  if ( compress( PackedTOC, &PackedTOCSize, (TU8*)NewTOC.GetPointer( 0 ), UnpackedTOCSize ) )
  {
    Files.FreeArray();
    fclose( Archive );
    delete[] PackedTOC;
    return CompressionError;
  }

  TU32 *pt = (TU32*)PackedTOC;
  for ( TU32 x = 0; x < PackedTOCSize / 4; x++ )
  {
    *pt ^= TOCMASK;
    pt++;
  }

  TU32 PTOC = PackedTOCSize;
  PTOC ^= TOCSIZEMASK;
  fwrite( &PTOC, 4, 1, Archive );
  fwrite( PackedTOC, PackedTOCSize, 1, Archive );
  for ( TS32 x = 0; x < Files.NumItems(); x++ )
    fwrite( Files[ x ]->PackedData, Files[ x ]->PackedSize, 1, Archive );

  fclose( Archive );

  if ( Verbose )
  {
    if ( ErrorCount > 0 )
      LOG( LOG_INFO, _T( "[pack] %d skipped from %d files!" ), ErrorCount, Files.NumItems() );
  }
  delete[] PackedTOC;
  Files.FreeArray();
  return AllOk;
}