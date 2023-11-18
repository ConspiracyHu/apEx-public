#include <Windows.h>
#include "MusicPlayer.h"

#ifdef MUSIC_IS_OGG

#include <dsound.h>

//#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_STDIO
#include "..\..\Bedrock\UtilLib\stb_vorbis.c"
//#pragma comment(lib,"wcrt.lib")
#pragma comment(lib,"msvcrt.lib")
//#pragma comment(lib,"libcmt.lib")

LPDIRECTSOUND pDirectSound = NULL;
LPDIRECTSOUNDBUFFER pPrimaryBuffer = NULL;
LPDIRECTSOUNDBUFFER pSecondaryBuffer = NULL;

#define OGGPLAY_NUMCHANNELS 2
#define OGGPLAY_SAMPLERATE 44100
#define OGGPLAY_SAMPLESIZE sizeof(short)
#define OGGPLAY_FRAMESIZE (OGGPLAY_NUMCHANNELS * OGGPLAY_SAMPLESIZE)

#define OGGPLAY_BUFFERSIZE (OGGPLAY_SAMPLERATE * 2)

int oggPlayer_SamplesPlayed = -OGGPLAY_BUFFERSIZE;
int oggPlayer_SyncOffset = 0;
double oggPlayer_CurrentTime = 0.0;
int oggPlayer_LockMutex = 0;
LARGE_INTEGER oggPlayer_LastPCV;
DWORD oggPlayer_LastPosition = 0;

#define OGGPLAY_TIMER_THRESHOLD 500

WAVEFORMATEX wfxFormat = {
  WAVE_FORMAT_PCM,
  OGGPLAY_NUMCHANNELS,
  OGGPLAY_SAMPLERATE,
  OGGPLAY_FRAMESIZE * OGGPLAY_SAMPLERATE,
  OGGPLAY_FRAMESIZE,
  OGGPLAY_SAMPLESIZE * 8,
  0
};

DSBUFFERDESC pBufferDescPrimary = {
  sizeof( DSBUFFERDESC ),
  DSBCAPS_PRIMARYBUFFER
};

int oggPlayer_ExitThread = 0;
DWORD oggPlayer_ThreadID = 0;

stb_vorbis * song = NULL;

#include <stdio.h>

int __cdecl memcmp(
  const void * buf1,
  const void * buf2,
  size_t count
)
{
  if ( !count )
    return( 0 );

#if defined (_M_X64)

  {


    __declspec( dllimport )


      size_t RtlCompareMemory( const void * src1, const void * src2, size_t length );

    size_t length;

    if ( ( length = RtlCompareMemory( buf1, buf2, count ) ) == count ) {
      return( 0 );
    }

    buf1 = (char *)buf1 + length;
    buf2 = (char *)buf2 + length;
  }

#else  /* defined (_M_X64) */

  while ( --count && *(char *)buf1 == *(char *)buf2 ) {
    buf1 = (char *)buf1 + 1;
    buf2 = (char *)buf2 + 1;
  }

#endif  /* defined (_M_X64) */

  return( *( (unsigned char *)buf1 ) - *( (unsigned char *)buf2 ) );
}


int oggPlayer_Init( HWND hWndParent, char* pMusicData, int nMusicDataSize )
{
  int error = 0;
  song = stb_vorbis_open_memory( (unsigned char*)pMusicData, nMusicDataSize, &error, NULL );
  if ( !song )
  {
    //FILE *f = fopen("err.log", "w+b");
    //fprintf(f, "%d", error);
    //fclose(f);
    return 0;
  }

  pDirectSound = NULL;
  DirectSoundCreate( 0, &pDirectSound, 0 );
  if ( !pDirectSound )
    return 0;

  pDirectSound->SetCooperativeLevel( hWndParent, DSSCL_PRIORITY );

  pPrimaryBuffer = NULL;
  pDirectSound->CreateSoundBuffer( &pBufferDescPrimary, &pPrimaryBuffer, NULL );

  if ( !pPrimaryBuffer )
    return 0;

  pPrimaryBuffer->SetFormat( &wfxFormat );

  DSBUFFERDESC bufferDescSecondary = {
    sizeof( DSBUFFERDESC ),
    DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS,
    OGGPLAY_FRAMESIZE * OGGPLAY_BUFFERSIZE,
    NULL,
    &wfxFormat,
    NULL
  };

  pSecondaryBuffer = NULL;
  pDirectSound->CreateSoundBuffer( &bufferDescSecondary, &pSecondaryBuffer, NULL );
  if ( !pSecondaryBuffer )
    return 0;

  return 1;
}

int oggPlayer_SamplesToRender( unsigned int lastpos )
{
  DWORD cursor = 0;
  pSecondaryBuffer->GetCurrentPosition( &cursor, NULL );

  int frameCursor = cursor / OGGPLAY_FRAMESIZE;
  int s2r = ( OGGPLAY_BUFFERSIZE + frameCursor - lastpos ) % OGGPLAY_BUFFERSIZE;
  if ( s2r > OGGPLAY_BUFFERSIZE / 2 )
    return 0;
  return s2r;
}

short __inline clamp( float f )
{
  int temp = (int)( f * 32767 );
  if ( temp > SHRT_MAX ) temp = SHRT_MAX;
  if ( temp < SHRT_MIN ) temp = SHRT_MIN;
  return temp;
}

void oggPlayer_ClipBuffer( float * source, short * dest, int nums ) {
  for ( int x = 0; x < nums; x++ ) dest[ x ] = clamp( source[ x ] );
}

DWORD __stdcall oggPlayer_Thread( LPVOID lpParam )
{
  if ( pSecondaryBuffer->Play( NULL, NULL, DSBPLAY_LOOPING ) ) return 0;

  // switch to 53-bit precision
  // http://www.nacad.ufrj.br/online/intel/Documentation/en_US/compiler_f/main_for/fpops/fortran/fpops_contw_f.htm
  unsigned short fl = 0;
  __asm {
    fstcw[ fl ]
    or [ fl ], 0x0200
    fldcw[ fl ]
  }

  oggPlayer_ExitThread = 0;
  while ( !oggPlayer_ExitThread )
  {
    int samplesToRender = oggPlayer_SamplesToRender( oggPlayer_LastPosition );

    if ( samplesToRender > 0 )
    {
      short * lpBuf1; DWORD dwSize1;
      short * lpBuf2; DWORD dwSize2;

      oggPlayer_LockMutex = 1;

      pSecondaryBuffer->Lock(
        OGGPLAY_FRAMESIZE * oggPlayer_LastPosition,
        OGGPLAY_FRAMESIZE * samplesToRender,
        (void**)&lpBuf1, &dwSize1,
        (void**)&lpBuf2, &dwSize2, NULL );

      int offs = dwSize1 / OGGPLAY_SAMPLESIZE;

      stb_vorbis_get_samples_short_interleaved( song, 2, lpBuf1, dwSize1 / sizeof( short ) );
      if ( lpBuf2 )
        stb_vorbis_get_samples_short_interleaved( song, 2, lpBuf2, dwSize2 / sizeof( short ) );

      pSecondaryBuffer->Unlock( lpBuf1, dwSize1, lpBuf2, dwSize2 );

      oggPlayer_LockMutex = 0;

      oggPlayer_LastPosition = ( oggPlayer_LastPosition + samplesToRender ) % OGGPLAY_BUFFERSIZE;
      oggPlayer_SamplesPlayed += samplesToRender;
    }

    Sleep( 3 );
  }
  pSecondaryBuffer->Stop();
  oggPlayer_ExitThread = -1;
  return 0;
}
void oggPlayer_Play()
{
  QueryPerformanceCounter( &oggPlayer_LastPCV );
  HANDLE h = CreateThread( NULL, 0, oggPlayer_Thread, NULL, 0, &oggPlayer_ThreadID );
  SetThreadPriority( h, THREAD_PRIORITY_TIME_CRITICAL );
}

unsigned int oggPlayer_GetSync()
{
  int rs = oggPlayer_SamplesPlayed;
  if ( !oggPlayer_LockMutex )
    rs += oggPlayer_SamplesToRender( oggPlayer_LastPosition );
  int r = (int)( rs / ( OGGPLAY_SAMPLERATE / 1000.0f ) );
  r = ( r < 0 ) ? 0 : r;
  if ( !r ) return r;

  LARGE_INTEGER count, freq;
  QueryPerformanceCounter( &count );
  QueryPerformanceFrequency( &freq );

  oggPlayer_CurrentTime += (double)( count.QuadPart - oggPlayer_LastPCV.QuadPart ) / (double)( freq.QuadPart );

  oggPlayer_LastPCV = count;
  int z = (int)( oggPlayer_CurrentTime * 1000 - ( OGGPLAY_BUFFERSIZE / OGGPLAY_SAMPLERATE * 1000 ) + oggPlayer_SyncOffset );
  z = ( z < 0 ) ? 0 : z;
  if ( abs( z - r ) > OGGPLAY_TIMER_THRESHOLD ) oggPlayer_SyncOffset += r - z;

  return z;
}

void oggPlayer_Stop()
{
  oggPlayer_ExitThread = 1;
  while ( oggPlayer_ExitThread != -1 ) { Sleep( 1 ); }
}

void oggPlayer_Deinit()
{
  pSecondaryBuffer->Release(); pSecondaryBuffer = NULL;
  pPrimaryBuffer->Release(); pPrimaryBuffer = NULL;
  pDirectSound->Release(); pDirectSound = NULL;
  stb_vorbis_close( song ); song = NULL;
}

#endif

#ifdef MUSIC_IS_WAVESABRE

#include "WaveSabreCore.h"
#include "WaveSabrePlayerLib.h"
//#pragma comment(lib,"../Libraries/WaveSabre/WaveSabreCore_release.lib")
//#pragma comment(lib,"../Libraries/WaveSabre/WaveSabrePlayerLib_release.lib")

using namespace WaveSabrePlayerLib;
static IPlayer *player = nullptr;

/*WaveSabreCore::Device *SongFactory( WaveSabrePlayerLib::SongRenderer::DeviceId id )
{
  switch ( id )
  {
  case SongRenderer::DeviceId::DeviceId_Falcon: return new WaveSabreCore::Falcon();
  case SongRenderer::DeviceId::DeviceId_Slaughter: return new WaveSabreCore::Slaughter();
  case SongRenderer::DeviceId::DeviceId_Thunder: return new WaveSabreCore::Thunder();
  case SongRenderer::DeviceId::DeviceId_Scissor: return new WaveSabreCore::Scissor();
  case SongRenderer::DeviceId::DeviceId_Leveller: return new WaveSabreCore::Leveller();
  case SongRenderer::DeviceId::DeviceId_Crusher: return new WaveSabreCore::Crusher();
  case SongRenderer::DeviceId::DeviceId_Echo: return new WaveSabreCore::Echo();
  case SongRenderer::DeviceId::DeviceId_Smasher: return new WaveSabreCore::Smasher();
  case SongRenderer::DeviceId::DeviceId_Chamber: return new WaveSabreCore::Chamber();
  case SongRenderer::DeviceId::DeviceId_Twister: return new WaveSabreCore::Twister();
  case SongRenderer::DeviceId::DeviceId_Cathedral: return new WaveSabreCore::Cathedral();
  case SongRenderer::DeviceId::DeviceId_Adultery: return new WaveSabreCore::Adultery();
  case SongRenderer::DeviceId::DeviceId_Specimen: return new WaveSabreCore::Specimen();
  }

  return nullptr;
}*/

#include "SynthConfig.inc"

int wsPlayer_Init( HWND hWndParent, char* pMusicData, int nMusicDataSize )
{
  SongRenderer::Song* Song = new SongRenderer::Song;
  Song->factory = SongFactory;
  Song->blob = (unsigned char*)pMusicData;
  int numRenderThreads = 3;

  player = new RealtimePlayer( Song, numRenderThreads );

  return 1;
}

void wsPlayer_Play()
{
  player->Play();
}

unsigned int wsPlayer_GetSync()
{
  return (unsigned int)( player->GetSongPos() * 1000 );
}

void wsPlayer_Stop()
{
}

void wsPlayer_Deinit()
{
  delete player;
  player = nullptr;
}

#endif