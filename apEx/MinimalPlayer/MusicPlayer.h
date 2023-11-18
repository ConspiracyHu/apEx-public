#include "MinimalPlayer.h"

#ifdef MUSIC_IS_OGG
int oggPlayer_Init( HWND hWndParent, char* pMusicData, int nMusicDataSize );
void oggPlayer_Play();
unsigned int oggPlayer_GetSync();
void oggPlayer_Stop();
void oggPlayer_Deinit();
#endif

#ifdef MUSIC_IS_WAVESABRE
int wsPlayer_Init( HWND hWndParent, char* pMusicData, int nMusicDataSize );
void wsPlayer_Play();
unsigned int wsPlayer_GetSync();
void wsPlayer_Stop();
void wsPlayer_Deinit();
#endif

#ifdef MUSIC_IS_OGG
#define MUSIC_INIT(a,b,c) oggPlayer_Init(a,b,c)
#define MUSIC_PLAY oggPlayer_Play
#define MUSIC_GETSYNC oggPlayer_GetSync
#define MUSIC_STOP oggPlayer_Stop
#define MUSIC_DEINIT oggPlayer_Deinit
#elif defined MUSIC_IS_WAVESABRE
#define MUSIC_INIT(a,b,c) wsPlayer_Init(a,b,c)
#define MUSIC_PLAY wsPlayer_Play
#define MUSIC_GETSYNC wsPlayer_GetSync
#define MUSIC_STOP wsPlayer_Stop
#define MUSIC_DEINIT wsPlayer_Deinit
#else // MVX is the default, of course
#define MUSIC_INIT(a,b,c) mvxSystem_Init(a,b,c)
#define MUSIC_PLAY mvxSystem_Play
#define MUSIC_GETSYNC mvxSystem_GetSync
#define MUSIC_STOP mvxSystem_Stop
#define MUSIC_DEINIT mvxSystem_DeInit
#endif