#pragma once

void WinMainCRTStartup();

//#define DEBUGINTOFILE

#ifdef DEBUGINTOFILE
#include <windows.h>
#include <DbgHelp.h>
int sprintf2( char *out, const char *format, ... );
#define OPENDEBUGFILE()  do { HANDLE h=CreateFile("debug.log",GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); CloseHandle(h); } while (0)
#define DEBUGLOG(v,...) do { HANDLE h=CreateFile("debug.log", FILE_APPEND_DATA , FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); char str[1024*3]; for (int yyy=0; yyy<1024*3; yyy++) str[yyy]=0; sprintf2(str, v, __VA_ARGS__); int xxx=0; for (int zzz=0; zzz<1024*3-1; zzz++) if (str[zzz]!=0) xxx++; else break; str[xxx]='\n'; WriteFile(h, str, xxx+1, NULL, NULL); CloseHandle(h); } while (0)

//#define OPENDEBUGFILE()  do {} while (0)
//#define DEBUGLOG(v,...) do { char *str=new char[1024]; for (int yyy=0; yyy<1024; yyy++) str[yyy]=0; sprintf2(str, v, __VA_ARGS__); int xxx=0; for (int zzz=0; zzz<1023; zzz++) if (str[zzz]!=0) xxx++; else break; str[xxx]='\n'; OutputDebugStringA(str); delete[] str;} while (0)

#else
#define OPENDEBUGFILE() do {} while (0)
#define DEBUGLOG(v,...) do {} while (0)
#endif
