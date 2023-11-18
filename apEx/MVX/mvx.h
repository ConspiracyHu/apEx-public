/*********************************************************************

                          #   # #   # #   #
                          ## ## #   #  # # 
                          # # # #   #   # 
                          #   #  # #   # # 
                          #   #   #   #   #

                  REALTIME BUZZ-BASED SYNTHESIZER

        Written by Gargaj/Conspiracy ^ Ümlaüt Design ^ CoolPHat

  "You've successfully proven that you can write a working softsynth, 
   now prove that you can write a good one :)"
                                                  /kb^farbrausch/

**********************************************************************/

#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>

// #define MVX_PURE_C_EXPORT

#ifndef DWORD_PTR
#define DWORD_PTR unsigned int
#endif

#include <dsound.h>

#define MVX_SAMPLERATE 44100
#define MVX_SAMPLESIZE (sizeof(short)*2)
#define MVX_BUFFERSIZE (MVX_SAMPLERATE * 2)

//////////////////////////////////////////////////////////////////////////

extern int mvxSystem_NumMachines;
extern BYTE * mvxSystem_MachineIDs;
extern BYTE * mvxSystem_MachineOrder;
extern int mvxSystem_NumConnections;

extern BYTE * mvxSystem_MasterSeq;
extern BYTE * mvxSystem_TPBSeq;
extern BYTE * mvxSystem_BPMSeq;
extern int mvxSystem_BPM;
extern int mvxSystem_TPB;
extern BYTE mvxSystem_MasterVolume;
extern int mvxSystem_MasterTick;
extern int mvxSystem_NumMasterTicks;

extern int * mvxSystem_ClipStart;
extern int * mvxSystem_ClipEnd;

extern int mvxSystem_SongLength; // in ticks
extern int mvxSystem_SamplesPerTick;

//////////////////////////////////////////////////////////////////////////

#ifdef MVX_PURE_C_EXPORT

#define MVXCALL __cdecl
extern "C"
{
#else

#define MVXCALL __fastcall

#endif


// MIXING ROUTINES
void MVXCALL mvxMixer_Init();
void MVXCALL mvxMixer_Render(float *,int);
void MVXCALL mvxMixer_DeInit();

// SYSTEM
void MVXCALL mvxSystem_ClipBuffer(float * source, short * dest, int nums);
int MVXCALL mvxSystem_LoadMusic(char * musicdata, int musicsize);
void MVXCALL mvxSystem_FreeMusic();

int MVXCALL mvxSystem_InitWithGUID( GUID * deviceGUID, HWND hWndParent, char* pMusicData, int nMusicDataSize );
int MVXCALL mvxSystem_Init( HWND hWndParent, char* pMusicData, int nMusicDataSize );
void MVXCALL mvxSystem_Play();
int MVXCALL mvxSystem_GetSync();
void MVXCALL mvxSystem_Stop();
void MVXCALL mvxSystem_DeInit();
void MVXCALL mvxSystem_Pause();
void MVXCALL mvxSystem_Resume();

//////////////////////////////////////////////////////////////////////////

typedef void (MVXCALL * MVX_RENDERCALLBACK)(void * userdata, short * buffer, unsigned int numsamples);

void MVXCALL mvxRenderOffline(MVX_RENDERCALLBACK callback, void * userdata);

int MVXCALL mvxSystem_RawInit(HWND hWndParent,short * pRawMusicData,int nRawMusicDataSizeInSamples);
void MVXCALL mvxSystem_RawPlay(int timeInMs);
int MVXCALL mvxSystem_RawGetSync();
void MVXCALL mvxSystem_RawSetSync(int timeInMs);
void MVXCALL mvxSystem_RawStop();
void MVXCALL mvxSystem_RawDeInit();

#ifdef MVX_PURE_C_EXPORT
}
#endif
