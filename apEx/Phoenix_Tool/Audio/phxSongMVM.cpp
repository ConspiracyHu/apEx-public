#include "BasePCH.h"
#include "phxSongMVM.h"
#include "../../MVX/mvx.h"

/*
#ifndef _DEBUG
#pragma comment(lib,"../libraries/mvx/mvxplayerlib_vs2019_releasestatic.lib")
#else
#pragma comment(lib,"../libraries/mvx/mvxplayerlib_vs2015_debug.lib")
#endif
*/

class mvxMachine
{
public:
  mvxMachine() {};
  virtual void Tick() = NULL;
  virtual int Render( float * buffer, int numsamp, float preamp ) = NULL;
};

extern mvxMachine ** mvxSystem_Machines;

void CphxSongMVM::OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & streamOut, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData )
{
  TU8 * pData = new TU8[ (TS32) streamSong.GetLength() ];
  streamSong.SeekFromStart( 0 );
  streamSong.Read( pData, (TS32) streamSong.GetLength() );

  mvxSystem_LoadMusic( (char*) pData, (TS32) streamSong.GetLength() );

  mvxMixer_Init();

  unsigned short fl = 0;
  __asm {
    fstcw[ fl ]
    or [ fl ], 0x0200
    fldcw[ fl ]
  }

  float * clipbuffer32 = new float[ mvxSystem_SamplesPerTick * 2 * 2 ];
  short * clipbuffer16 = new short[ mvxSystem_SamplesPerTick * 2 * 2 ];

  aMarkers.Flush();

  mvxSystem_MasterTick = 0;
  TU32 dwFramesRendered = 0;
  for ( int x = 0; x < mvxSystem_SongLength; x++ ) {
    for ( int y = 0; y < mvxSystem_NumMachines; y++ ) {
      if ( mvxSystem_Machines[ y ] ) mvxSystem_Machines[ y ]->Tick();
    }
    if ( mvxSystem_MasterTick < mvxSystem_NumMasterTicks ) {
      if ( mvxSystem_MasterSeq[ mvxSystem_MasterTick ] != 0xFF )
        mvxSystem_MasterVolume = mvxSystem_MasterSeq[ mvxSystem_MasterTick ];
      if ( mvxSystem_TPBSeq[ mvxSystem_MasterTick ] != 0xFF )
        mvxSystem_TPB = mvxSystem_TPBSeq[ mvxSystem_MasterTick ];
      if ( mvxSystem_BPMSeq[ mvxSystem_MasterTick ] != 0xFF )
        mvxSystem_BPM = mvxSystem_BPMSeq[ mvxSystem_MasterTick ];
      mvxSystem_SamplesPerTick = ( 60 * MVX_SAMPLERATE ) / ( mvxSystem_TPB*mvxSystem_BPM );
    }
    mvxSystem_MasterTick++;

    mvxMixer_Render( clipbuffer32, mvxSystem_SamplesPerTick );
    mvxSystem_ClipBuffer( clipbuffer32, clipbuffer16, mvxSystem_SamplesPerTick * 2 );

    aMarkers.Add( MulDiv( dwFramesRendered, 1000, PHXSOUND_SAMPLERATE ) );
    streamOut.Write( clipbuffer16, mvxSystem_SamplesPerTick * MVX_SAMPLESIZE );

    if ( pLoad( pUserData, x, mvxSystem_SongLength ) == false )
      break;

    dwFramesRendered += mvxSystem_SamplesPerTick;
  }

  delete[] clipbuffer32;
  delete[] clipbuffer16;

  mvxMixer_DeInit();
  mvxSystem_FreeMusic();

  delete[] pData;

  pLoad( pUserData, mvxSystem_SongLength, mvxSystem_SongLength );
}