#include "BasePCH.h"
#include "phxSongOGG.h"

#define MUSIC_IS_OGG
#define STB_VORBIS_HEADER_ONLY
#include "../../../Bedrock/UtilLib/stb_vorbis.c"

CphxSongOGG::CphxSongOGG()
{
}

CphxSongOGG::~CphxSongOGG()
{
}

void CphxSongOGG::OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & stream, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData )
{
  int error = 0;
  unsigned char * p = new unsigned char[ (TU32) streamSong.GetLength() ];
  streamSong.Read( p, (TU32) streamSong.GetLength() );
  stb_vorbis * song = stb_vorbis_open_memory( p, (TS32) streamSong.GetLength(), &error, NULL );

  unsigned int total = stb_vorbis_stream_length_in_samples( song );
  int bufsize = 1024 * 1024;
  short * buf = new short[ bufsize ];
  unsigned int samplesWritten = 0;
  do {
    int n = stb_vorbis_get_samples_short_interleaved( song, 2, buf, bufsize );
    stream.Write( buf, n * 2 * sizeof( short ) );
    samplesWritten += n;
    pLoad( pUserData, samplesWritten, total );
    if ( n == 0 ) break;
  } while ( samplesWritten < total );

  pLoad( pUserData, total, total );

  stb_vorbis_close( song );

  delete[] buf;
  delete[] p;
}