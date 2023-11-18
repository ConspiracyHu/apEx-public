#include "BasePCH.h"
#include "phxSongV2M.h"

#define V2MPLAYER_SYNC_FUNCTIONS
#include "../../libraries/v2/v2mPlayer.cpp"

#pragma comment(lib,"../libraries/v2/libv2.lib")

void CphxSongV2M::OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & streamOut, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData )
{
  unsigned char * p = new unsigned char[ (TU32) streamSong.GetLength() ];
  streamSong.Read( p, (TU32) streamSong.GetLength() );

  V2MPlayer * v2mPlayer = new V2MPlayer(); // must use new otherwise it blows the stack
  v2mPlayer->Init();
  if ( !v2mPlayer->Open( p ) )
  {
    return;
  }

  long * positions = NULL;
  unsigned int posCount = v2mPlayer->CalcPositions( &positions );

  /*
  // "CalcPositions is kinda deprecated and won’t work" /kb/
  aMarkers.Flush();
  for ( unsigned int i = 0; i < posCount; i++ )
  {
    if ( ( positions[ i * 2 + 1 ] & 0xFF00 ) == 0 )
    {
      aMarkers.Add( positions[ i * 2 ] );
    }
  }
  */

  // reasonable enough for the progress bar though
  unsigned int songLengthInMS = positions[ ( posCount - 1 ) * 2 ];
  delete[] positions;

  v2mPlayer->Play();
  int bufsize = 128 * 1024;
  float * bufFloat = new float[ bufsize * 2 ];
  short * bufShort = new short[ bufsize * 2 ];
  pLoad( pUserData, 0, songLengthInMS );
  unsigned int samplesWritten = 0;
  while ( v2mPlayer->IsPlaying() && MulDiv( samplesWritten, 1000, PHXSOUND_SAMPLERATE ) < songLengthInMS )
  {
    v2mPlayer->Render( bufFloat, bufsize );
    for ( int i = 0; i < bufsize * 2; i++ )
    {
      bufShort[ i ] = (short)max( -32760, min( 32760, bufFloat[ i ] * 32767 ) );
    }
    streamOut.Write( bufShort, bufsize * 2 * sizeof( short ) );
    samplesWritten += bufsize;
    pLoad( pUserData, MulDiv( samplesWritten, 1000, PHXSOUND_SAMPLERATE ), songLengthInMS );
  }
  pLoad( pUserData, songLengthInMS, songLengthInMS );

  delete v2mPlayer;
  delete[] bufFloat;
  delete[] bufShort;
  delete[] p;

}