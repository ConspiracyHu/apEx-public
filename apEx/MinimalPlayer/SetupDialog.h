#pragma once
#include <windows.h>
#include "resource.h"
#ifdef PHX_MINIMAL_BUILD
#include "..\Phoenix\PhoenixConfig.h"
#else
#include "..\Phoenix\PhoenixConfig_Full.h"
#endif

typedef struct
{
  char *ReleaseName;
  char *GroupName;

#ifdef SETUPBOX_HAS_SOCIAL
  char *urls[ 5 ];
#endif

  int fullscreen;
  DEVMODE mode;
  int vsync;
  float HardwareAspectRatio;
} SETUPCFG;

extern SETUPCFG setupcfg;

int OpenSetupDialog( char *Strings[ 7 ] );
