#pragma once

#include "..\..\..\Bedrock\BaseLib\BaseLib.h"
#include "phxSongBase.h"
#include "WaveSabrePlayerLib.h"

class CphxSongWaveSabre : public CphxSongBase
{
public:

  CArray<WaveSabrePlayerLib::SongRenderer::DeviceId> devices;

  void OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & stream, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData );
  virtual CString GenerateHeaderContents() override;
  CString GetFileTypeMacro() { return CString( "MUSIC_IS_WAVESABRE" ); }
};
