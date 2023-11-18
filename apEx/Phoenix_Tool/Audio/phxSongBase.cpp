#include "BasePCH.h"
#include "phxSongBase.h"
#include "phxSound.h"

CphxSongBase::CphxSongBase()
{
  pSecondaryBuffer = NULL;
  nBufferSize = 0;
}

CphxSongBase::~CphxSongBase()
{
  OfflineStop();
  if ( pSecondaryBuffer ) pSecondaryBuffer->Release();
  pSecondaryBuffer = NULL;
}

struct PHXSOUND_THREADPROXY
{
  CphxSongBase * pThis;
  SOUND_LOADCALLBACK pLoad;
  void * pUserData;
  CStreamReaderMemory pStreamSong;
};

DWORD __stdcall phxSongBase_Thread( LPVOID lpParam )
{
  PHXSOUND_THREADPROXY * p = (PHXSOUND_THREADPROXY*) lpParam;
  p->pThis->OfflineRender( p->pStreamSong, p->pLoad, p->pUserData );
  delete p;

  return 1;
}

TBOOL CphxSongBase::OffLineLoad( CStreamReader &stream, SOUND_LOADCALLBACK pLoad /*= NULL*/, void * pUserData /*= NULL*/ )
{
  PHXSOUND_THREADPROXY * p = new PHXSOUND_THREADPROXY;
  p->pThis = this;
  p->pLoad = pLoad;
  p->pUserData = pUserData;

  TU8 *data = new TU8[ (TS32) stream.GetLength() ];
  stream.Read( data, (TS32) stream.GetLength() );

  p->pStreamSong.Open( data, (TS32) stream.GetLength() );
  delete[] data;

  DWORD dwThreadID = 0;
  HANDLE h = CreateThread( NULL, 0, phxSongBase_Thread, p, 0, &dwThreadID );

  return true;
}

unsigned __int64 HashBuffer( CStreamReader &streamSong )
{
  char buffer[ 1024 ];
  unsigned __int64 hash = 5381;
  while ( true )
  {
    int out = streamSong.Read( buffer, 1024 );
    for ( int i = 0; i < out; i++ )
    {
      hash = ( ( hash << 5 ) + hash ) + buffer[ i ]; // hash * 33 + c
    }
    if ( out < 1024 )
    {
      break;
    }
  }
  streamSong.SeekFromStart( 0 );
  return hash;
}

void CphxSongBase::OfflineRender( CStreamReader & streamSong, SOUND_LOADCALLBACK pLoad, void * pUserData )
{
  CStreamWriterMemory streamOut;
  aMarkers.Flush();

  unsigned __int64 hash = HashBuffer( streamSong );

  CreateDirectory( _T( "SoundCache" ), 0 );
  CString cacheFileName = CString::Format( _T( "SoundCache\\%I64x.apxsndcache" ), hash );
  
  CStreamReaderFile cache;
  if ( ShouldCache() && cache.Open( cacheFileName.GetPointer() ) && cache.ReadDWord() == '!CSA' )
  {
    unsigned int metadataSize = cache.ReadDWord(); // reserved for metadata
    int metaDataPos = cache.GetOffset();

    if ( metadataSize )
    {
      headerContents = cache.ReadASCIIZ();
    }

    cache.SeekFromStart( metaDataPos + metadataSize );
    unsigned int numMarkers = cache.ReadDWord();
    for ( unsigned int i = 0; i < numMarkers; i++ )
    {
      aMarkers.Add( cache.ReadDWord() );
    }
    unsigned int sampleDataSize = cache.ReadDWord();
    while ( sampleDataSize > 0 )
    {
      char buffer[ 10 * 1024 ];
      int read = min( sampleDataSize, 10 * 1024 );
      cache.Read( buffer, read );
      streamOut.Write( buffer, read );
      sampleDataSize -= read;
    }
  }
  else
  {
    OfflineRenderSong( streamSong, streamOut, aMarkers, pLoad, pUserData );

    CStreamWriterFile cacheWr;
    if ( ShouldCache() && cacheWr.Open( cacheFileName.GetPointer() ) )
    {
      cacheWr.WriteDWord( '!CSA' );
      headerContents = GenerateHeaderContents();
      int metaDataLength = headerContents.Length() + 1;
      cacheWr.WriteDWord( metaDataLength ); // reserved for metadata
      cacheWr.WriteASCIIZ( headerContents );
      cacheWr.WriteDWord( aMarkers.NumItems() );
      for ( int i = 0; i < aMarkers.NumItems(); i++ )
      {
        cacheWr.WriteDWord( aMarkers[ i ] );
      }
      cacheWr.WriteDWord( streamOut.GetLength() );
      cacheWr.Write( streamOut.GetData(), streamOut.GetLength() );
    }
  }

  if ( pSecondaryBuffer ) pSecondaryBuffer->Release();

  DSBUFFERDESC bufferDescSecondary = {
    sizeof( DSBUFFERDESC ),
    DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS,
    streamOut.GetLength(),
    NULL,
    &CphxSound::wfxFormat,
    NULL
  };

  LPDIRECTSOUNDBUFFER pBuf = NULL;
  CphxSound::pDirectSound->CreateSoundBuffer( &bufferDescSecondary, &pBuf, NULL );

  LPVOID pData = NULL;
  DWORD dwLength = 0;
  pBuf->Lock( 0, streamOut.GetLength(), &pData, &dwLength, NULL, NULL, NULL );
  CopyMemory( pData, streamOut.GetData(), streamOut.GetLength() );
  pBuf->Unlock( pData, dwLength, NULL, NULL );

  pSecondaryBuffer = pBuf;
  nBufferSize = streamOut.GetLength();

  pLoad( pUserData, 1, 1 );
}

TBOOL CphxSongBase::OfflinePlay( TU32 nMilliSec )
{
  if ( !pSecondaryBuffer )
    return false;

  pSecondaryBuffer->SetCurrentPosition( MulDiv( nMilliSec, PHXSOUND_SAMPLERATE * PHXSOUND_FRAMESIZE, 1000 ) );
  pSecondaryBuffer->Play( 0, 0, NULL );
  return true;
}

TBOOL CphxSongBase::OfflineStop()
{
  if ( !pSecondaryBuffer )
    return false;

  pSecondaryBuffer->Stop();
  return true;
}

TU32 CphxSongBase::GetOfflinePlayPosition()
{
  if ( !pSecondaryBuffer )
    return false;

  DWORD dwPlayCursor = 0;
  pSecondaryBuffer->GetCurrentPosition( &dwPlayCursor, NULL );

  return MulDiv( dwPlayCursor, 1000, PHXSOUND_SAMPLERATE * PHXSOUND_FRAMESIZE );
}

TU32 CphxSongBase::GetOfflineSongLength()
{
  if ( !pSecondaryBuffer )
    return 0;

  return MulDiv( nBufferSize, 1000, PHXSOUND_SAMPLERATE * PHXSOUND_FRAMESIZE );
}

TBOOL CphxSongBase::IsOfflineLoaded()
{
  return pSecondaryBuffer != NULL;
}

CString CphxSongBase::GetHeaderContents()
{
  return headerContents;
}

CString CphxSongBase::GenerateHeaderContents()
{
  return CString();
}

TBOOL CphxSongBase::GetOfflineMarkers( CArray<TU32> & aMarkers )
{
  aMarkers = this->aMarkers;
  return true;
}
