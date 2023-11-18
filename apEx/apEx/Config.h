#pragma once

class CapexRoot;

namespace Config
{
  extern TBOOL ConfirmExit;
  extern TBOOL VSync;
  extern TBOOL RightDoubleClickCameraReset;
  extern int rendercount;
  extern CString AutoloadMusic;
  extern CString VideoDumpPath;
  extern TS32 TexgenMemPoolSize;
  extern CString skinCSS;
  extern TS32 MonitorAspectX;
  extern TS32 MonitorAspectY;
  extern TF32 UbertoolSnap;

  extern CArray<CString> RecentFiles;
  extern CDictionary<CString, CString> DirectoryList;


  void Load();
  void InitRoot( CapexRoot* root );
  void Save( CapexRoot* root );
}