#pragma once
#include "../Phoenix/phxEngine.h"

void InitWindow( HINSTANCE h, int XRes, int YRes, int refreshrate, bool FullScreen, HICON Icon, bool antialias );
extern bool phxDone;
extern HWND hWnd;
extern IDXGISwapChain *SwapChain;
void SetDenominator( int denom ); //set frequency denominator - this is a hackhackhack for the 120hz build
