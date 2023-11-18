#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"

enum CNSArchiveError
{
  AllOk = 0,
  ErrorOpeningArchive,
  InvalidArchive,
  FileNotInArchive,
  ArchiveNotOpen,
  ArchiveAlreadyOpen,
  DecompressionError,
  CorruptArchive,
  CompressionError,
  NoFilesFound,
};

struct CNSArchiveTocEntry
{
  TU32 Hash;
  TU32 Offset;
  TU32 PackedSize;
  TU32 UnpackedSize;
};

class CNSArchive
{
  FILE *Archive;
  CArray<CNSArchiveTocEntry> TOC;

public:
  CNSArchive();
  virtual ~CNSArchive();
  CNSArchiveError OpenArchive( TS8 *ArchiveName );
  CNSArchiveError ReadFile( TS8 *FileName, TU8 *&Result, unsigned long &FileSize );
  CNSArchiveError ReadFileByHash( TU32 Hash, TU8 *&Result, unsigned long &FileSize );
  TS32 GetFileCount();
  CNSArchiveTocEntry GetTOCEntry( TS32 File );
};

class CNSArchiveFileData
{
  TU8 *Data;
  TU32 FileSize;
  TS8 *FileName;

public:
  CNSArchiveFileData();
  virtual ~CNSArchiveFileData();
  void SetData( TU8 *data, TS32 filesize );
  void SetFileName( TS8 *filename );
  TBOOL isValid();
  TU8 *GetData();
  TS32 GetFileSize();
  TS8 *GetFileName();
};

CNSArchiveError BuildNewArchiveFromMemory( TS8 *ArchiveFile, CNSArchiveFileData *FileList[], TS32 FileCount, TBOOL Verbose = false );
CNSArchiveError BuildNewArchiveFromFiles( TS8 *ArchiveFile, TS8 *Path, TBOOL RecurseDirs, TBOOL Verbose );
