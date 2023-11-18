#pragma once

#include <mmsystem.h>
#include <dsound.h>
#include "..\..\..\Bedrock\BaseLib\BaseLib.h"

#define PHXSOUND_NUMCHANNELS 2
#define PHXSOUND_SAMPLERATE 44100
#define PHXSOUND_SAMPLESIZE sizeof(short)
#define PHXSOUND_FRAMESIZE (PHXSOUND_NUMCHANNELS * PHXSOUND_SAMPLESIZE)

typedef TBOOL( __fastcall * SOUND_LOADCALLBACK )( void * pUserData, TU32 nCurrentProgress, TU32 nTotalProgress );

class CphxSongBase
{
public:
  CphxSongBase();
  virtual ~CphxSongBase();

  TBOOL OffLineLoad( CStreamReader &stream, SOUND_LOADCALLBACK pLoad = NULL, void * pUserData = NULL );
  TBOOL OfflinePlay( TU32 nMilliSec );
  TBOOL OfflineStop();
  TU32 GetOfflinePlayPosition();
  TU32 GetOfflineSongLength(); // millisec
  TBOOL GetOfflineMarkers( CArray<TU32> & aMarkers );

  void OfflineRender( CStreamReader & streamSong, SOUND_LOADCALLBACK pLoad, void * pUserData );
  TBOOL IsOfflineLoaded();
  CString GetHeaderContents();
  virtual CString GetFileTypeMacro() = NULL;

protected:
  virtual CString GenerateHeaderContents();
  virtual void OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & streamOut, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData ) = NULL;
  virtual TBOOL ShouldCache() { return true; }
  LPDIRECTSOUNDBUFFER pSecondaryBuffer;
  CArray<TU32> aMarkers;
  unsigned int nBufferSize;
  CString headerContents;
};
