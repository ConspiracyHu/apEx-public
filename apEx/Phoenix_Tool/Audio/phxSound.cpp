#include "BasePCH.h"
#include "phxSound.h"

LPDIRECTSOUND CphxSound::pDirectSound = NULL;
LPDIRECTSOUNDBUFFER CphxSound::pPrimaryBuffer = NULL;

WAVEFORMATEX CphxSound::wfxFormat = {
  WAVE_FORMAT_PCM,
  PHXSOUND_NUMCHANNELS,
  PHXSOUND_SAMPLERATE,
  PHXSOUND_FRAMESIZE * PHXSOUND_SAMPLERATE,
  PHXSOUND_FRAMESIZE,
  PHXSOUND_SAMPLESIZE * 8,
  0
};

DSBUFFERDESC CphxSound::pBufferDescPrimary = {
  sizeof( DSBUFFERDESC ),
  DSBCAPS_PRIMARYBUFFER
};

//////////////////////////////////////////////////////////////////////////

TBOOL CphxSound::Init( HWND hWnd )
{
  DirectSoundCreate( 0, &pDirectSound, 0 );
  if ( !pDirectSound )
    return false;

  pDirectSound->SetCooperativeLevel( hWnd, DSSCL_PRIORITY );

  pDirectSound->CreateSoundBuffer( &pBufferDescPrimary, &pPrimaryBuffer, NULL );

  if ( !pPrimaryBuffer )
    return false;

  pPrimaryBuffer->SetFormat( &wfxFormat );

  return true;
}

TBOOL CphxSound::Deinit()
{
  if ( pPrimaryBuffer ) pPrimaryBuffer->Release();
  pPrimaryBuffer = NULL;

  if ( pDirectSound ) pDirectSound->Release();
  pDirectSound = NULL;

  return true;
}

CphxSound::CphxSound()
{
  pSong = NULL;
}

CphxSound::~CphxSound()
{
  if ( pSong ) delete pSong;
  pSong = NULL;
}

TBOOL CphxSound::Load( CStreamReader &stream, SOUND_LOADCALLBACK pLoad /*= 0*/, void * pUserData/*=0*/ )
{
  stream.SeekFromStart( 0 );
  switch ( stream.ReadDWord() )
  {
    case 'SggO':
    {
      stream.SeekFromStart( 0 );
      pSong = new CphxSongOGG();
      return pSong->OffLineLoad( stream, pLoad, pUserData );
    } 
    break;
    case '\0MVM':
    {
      stream.SeekFromStart( 0 );
      pSong = new CphxSongMVM();
      return pSong->OffLineLoad( stream, pLoad, pUserData );
    } 
    break;
    default:
    {
      stream.SeekFromStart( 4 );
      TU32 sampleRate = stream.ReadDWord();
      stream.SeekFromStart( 0 );
      if ( 22050 <= sampleRate && sampleRate <= 48000 )
      {
        pSong = new CphxSongWaveSabre();
      }
      else
      {
        pSong = new CphxSongV2M();
      }
      return pSong->OffLineLoad( stream, pLoad, pUserData );
    }
    break;
  }
  return false;
}

TBOOL CphxSound::Play( TU32 nMilliSec )
{
  if ( !pSong )
    return false;

  return pSong->OfflinePlay( nMilliSec );
}

TBOOL CphxSound::Stop()
{
  if ( !pSong )
    return false;

  return pSong->OfflineStop();
}

TU32 CphxSound::GetSongLength()
{
  if ( !pSong )
    return 0;

  return pSong->GetOfflineSongLength();
}

TBOOL CphxSound::GetMarkers( CArray<TU32> & aMarkers )
{
  if ( !pSong )
    return false;

  return pSong->GetOfflineMarkers( aMarkers );
}

TU32 CphxSound::GetPlayPosition()
{
  if ( !pSong )
    return 0;

  return pSong->GetOfflinePlayPosition();
}

CString CphxSound::GetFileTypeMacro()
{
  if ( !pSong )
    return CString();
  
  return pSong->GetFileTypeMacro();
}

CString CphxSound::GetSynthConfig()
{
  if ( !pSong )
    return CString();

  return pSong->GetHeaderContents();
}

TBOOL CphxSound::IsLoaded()
{
  if ( !pSong )
    return false;

  return pSong->IsOfflineLoaded();
}