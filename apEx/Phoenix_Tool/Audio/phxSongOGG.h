#pragma once

#include "..\..\..\Bedrock\BaseLib\BaseLib.h"
#include "phxSongBase.h"

class CphxSongOGG : public CphxSongBase
{
private:
public:
  CphxSongOGG();
  virtual ~CphxSongOGG();
  void OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & stream, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData );
  TBOOL ShouldCache() { return false; }
  CString GetFileTypeMacro() { return CString("MUSIC_IS_OGG"); }
};
