#pragma once

#include "..\..\..\Bedrock\BaseLib\BaseLib.h"
#include "phxSongBase.h"

class CphxSongV2M : public CphxSongBase
{
public:
  void OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & stream, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData );
  CString GetFileTypeMacro() { return CString( "MUSIC_IS_V2M" ); }
};
