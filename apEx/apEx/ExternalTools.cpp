#include "BasePCH.h"
#include "ExternalTools.h"

//#include <stdio.h>
#include <ShellAPI.h>
//#include <Windows.h>
//#include <io.h>
//#include <sys/stat.h>


TBOOL Execute( TCHAR *prog, TCHAR *params, TBOOL visible )
{
  SHELLEXECUTEINFO ShExecInfo = { 0 };
  ShExecInfo.cbSize = sizeof( SHELLEXECUTEINFO );
  ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  ShExecInfo.hwnd = NULL;
  ShExecInfo.lpVerb = NULL;
  ShExecInfo.lpFile = prog;
  ShExecInfo.lpParameters = params;
  ShExecInfo.lpDirectory = NULL;
  ShExecInfo.nShow = visible ? SW_SHOW : SW_HIDE;
  ShExecInfo.hInstApp = NULL;
  if ( ShellExecuteEx( &ShExecInfo ) )
  {
    WaitForSingleObject( ShExecInfo.hProcess, INFINITE );
    return true;
  }
  else
  {
    LOG_ERR( "FAILED TO RUN %s!!!\n", prog );
    return false;
  }
}

int ReadFromPipeNoWait( HANDLE hPipe, char* pDest, int nMax )
{
  DWORD nBytesRead = 0;
  DWORD nAvailBytes;
  char cTmp;
  memset( pDest, 0, nMax );
  // -- check for something in the pipe
  PeekNamedPipe( hPipe, &cTmp, 1, NULL, &nAvailBytes, NULL );
  if ( nAvailBytes == 0 ) {
    return( nBytesRead );
  }
  // OK, something there... read it
  if ( !ReadFile( hPipe, pDest, nMax - 1, &nBytesRead, NULL ) )
  {
    LOG_ERR( "[external] Error reading from pipe" );
  }
  return( nBytesRead );
}

#include "apExRoot.h"

void ExecuteWithConsoleLog( TCHAR *prog, TCHAR *params )
{
  extern CapexRoot *Root;
  CWBApplication *App = Root->GetApplication();

  SECURITY_ATTRIBUTES rSA = { 0 };
  rSA.nLength = sizeof( SECURITY_ATTRIBUTES );
  rSA.bInheritHandle = TRUE;

  HANDLE hReadPipe, hWritePipe;
  CreatePipe( &hReadPipe, &hWritePipe, &rSA, 25000 );

  HANDLE hReadPipe2, hWritePipe2;
  CreatePipe( &hReadPipe2, &hWritePipe2, &rSA, 25000 );

  PROCESS_INFORMATION rPI = { 0 };
  STARTUPINFO         rSI = { 0 };
  rSI.cb = sizeof( rSI );
  rSI.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES | STARTF_USEPOSITION;
  rSI.wShowWindow = SW_HIDE;  // or SW_SHOWNORMAL or SW_MINIMIZE
  rSI.hStdOutput = hWritePipe;
  rSI.hStdError = hWritePipe2;
  rSI.dwX = 0;
  rSI.dwY = 10000;

  LOG_NFO( "[apex] Executing: %s %s", prog, params );

  BOOL fRet = CreateProcessA( prog, (LPSTR)params, NULL,
                              NULL, TRUE, 0, 0, 0, &rSI, &rPI );

  if ( !fRet )
  {
    LOG( LOG_ERROR, _T( "[apex] Error executing program!" ) );
    return;
  }

  CString s;

  char dest[ 1000 ];
  CString sProgress = "";
  DWORD dwRetFromWait = WAIT_TIMEOUT;
  while ( dwRetFromWait != WAIT_OBJECT_0 )
  {
    dwRetFromWait = WaitForSingleObject( rPI.hProcess, 100 );
    if ( dwRetFromWait == WAIT_ABANDONED ) break;

    TS32 readcnt = 0;
    while ( ( readcnt = ReadFromPipeNoWait( hReadPipe, dest, sizeof( dest ) ) ) > 0 )
    {
      s.Append( dest, readcnt );
    }

    if ( s.Length() )
    {
      TBOOL NeedsEnd = false;
      if ( s.Length() >= 1 && s.GetPointer()[ s.Length() - 1 ] == '\n' ) NeedsEnd = true;
      CStringArray strs = s.Explode( "\n" );

      for ( TS32 x = 0; x < strs.NumItems() - 1; x++ )
      {
        if ( strs[ x ].GetPointer()[ strs[ x ].Length() - 1 ] == '\r' ) strs[ x ].GetPointer()[ strs[ x ].Length() - 1 ] = 0;
        LOG_NFO( "%s", strs[ x ].GetPointer() );
      }

      s = strs.Last();
      if ( NeedsEnd )
        s += CString( "\n" );
    }

    App->HandleOSMessages();
    App->Display();
  }

  if ( s.Length() )
  {
    CStringArray strs = s.Explode( "\n" );
    for ( TS32 x = 0; x < strs.NumItems(); x++ )
    {
      if ( strs[ x ].GetPointer()[ strs[ x ].Length() - 1 ] == '\r' ) strs[ x ].GetPointer()[ strs[ x ].Length() - 1 ] = 0;
      LOG_NFO( "%s", strs[ x ].GetPointer() );
    }
  }

  CloseHandle( hReadPipe2 );
  CloseHandle( hWritePipe2 );
  CloseHandle( hReadPipe );
  CloseHandle( hWritePipe );
  CloseHandle( rPI.hThread );
  CloseHandle( rPI.hProcess );
}

TS32 GetKKrunchedSize( CString &FileName )
{
  SECURITY_ATTRIBUTES rSA = { 0 };
  rSA.nLength = sizeof( SECURITY_ATTRIBUTES );
  rSA.bInheritHandle = TRUE;

  HANDLE hReadPipe, hWritePipe;
  CreatePipe( &hReadPipe, &hWritePipe, &rSA, 25000 );

  PROCESS_INFORMATION rPI = { 0 };
  STARTUPINFO         rSI = { 0 };
  rSI.cb = sizeof( rSI );
  rSI.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  rSI.wShowWindow = SW_HIDE;  // or SW_SHOWNORMAL or SW_MINIMIZE
  rSI.hStdOutput = hWritePipe;
  rSI.hStdError = hWritePipe;

  CString sCmd = CString( ".\\kkrunch.exe " ) + FileName;

  LOG_NFO( "[kkrunch] Executing: %s", sCmd.GetPointer() );

  BOOL fRet = CreateProcessA( _T( ".\\kkrunch.exe" ), (LPSTR)sCmd.GetPointer(), NULL,
                              NULL, TRUE, 0, 0, 0, &rSI, &rPI );

  if ( !fRet )
  {
    LOG( LOG_ERROR, _T( "[apex] Error executing kkrunch!" ) );
    return 0;
  }

  CString s;

  char dest[ 1000 ];
  CString sProgress = "";
  DWORD dwRetFromWait = WAIT_TIMEOUT;
  while ( dwRetFromWait != WAIT_OBJECT_0 )
  {
    dwRetFromWait = WaitForSingleObject( rPI.hProcess, 100 );
    if ( dwRetFromWait == WAIT_ABANDONED ) break;

    while ( ReadFromPipeNoWait( hReadPipe, dest, sizeof( dest ) ) > 0 )
      s += dest;
  }

  CloseHandle( hReadPipe );
  CloseHandle( hWritePipe );
  CloseHandle( rPI.hThread );
  CloseHandle( rPI.hProcess );

  TS32 x = s.Find( _T( "=>" ) );
  LOG_DBG( "[kkrunch] %s", s.GetPointer() );

  TS32 size = 0;

  if ( x >= 0 )
  {
    sscanf_s( s.GetPointer() + x + 3, "%d", &size );
    LOG( LOG_DEBUG, "kkrunch result: %d bytes", size );
  }

  return size;
}


TS32 GetKKrunchedSize( TU8 *Data, TS32 Size )
{
  FILE *f;
  if ( !fopen_s( &f, "bloated.dat", "w+b" ) )
  {
    LOG( LOG_ERROR, _T( "[apex] Error writing data for kkrunch compression test" ) );
    return 0;
  }
  if ( !f ) return 0;

  fwrite( Data, Size, 1, f );
  fclose( f );

  TS32 size = GetKKrunchedSize( CString( _T( "bloated.dat" ) ) );

  remove( "bloated.dat" );

  return size;
}

TBOOL MinifyShader( char *Data, CString &Result, TBOOL Verbose )
{
  Result = _T( "" );

  if ( !strlen( Data ) )
  {
    LOG( LOG_WARNING, _T( "[apex] Empty shader. Skipping minification." ) );
    return false;
  }

  {
    CStreamWriterFile f;
    f.Open( _T( "in.hlsl" ) );
    f.Write( Data, strlen( Data ) );
  }

  SECURITY_ATTRIBUTES rSA = { 0 };
  rSA.nLength = sizeof( SECURITY_ATTRIBUTES );
  rSA.bInheritHandle = TRUE;

  HANDLE hReadPipe, hWritePipe, hWriteIN, hWriteINtmp, hReadIN;;
  CreatePipe( &hReadPipe, &hWritePipe, &rSA, 25000 );
  CreatePipe( &hReadIN, &hWriteINtmp, &rSA, strlen( Data ) );

  DuplicateHandle( GetCurrentProcess(), hWriteINtmp, GetCurrentProcess(), &hWriteIN, 0, FALSE, DUPLICATE_SAME_ACCESS );

  CloseHandle( hWriteINtmp );

  PROCESS_INFORMATION rPI = { 0 };
  STARTUPINFO         rSI = { 0 };
  rSI.cb = sizeof( rSI );
  rSI.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  rSI.wShowWindow = SW_HIDE;
  rSI.hStdOutput = hWritePipe;
  rSI.hStdError = hWritePipe;
  rSI.hStdInput = hReadIN;

  CString sCmd = CString::Format( ".\\shader_minifier.exe --hlsl --format none --no-renaming-list p,v,g,h,d -o out.hlsl in.hlsl" );

  BOOL fRet = CreateProcess( NULL, (LPSTR)(LPCSTR)sCmd.GetPointer(), NULL, NULL, TRUE, 0, 0, 0, &rSI, &rPI );

  if ( !fRet )
  {
    LOG( LOG_ERROR, _T( "[apex] Error executing shader minifier!" ) );
    CloseHandle( hWriteIN );
    CloseHandle( hReadIN );
    CloseHandle( hReadPipe );
    CloseHandle( hWritePipe );
    CloseHandle( rPI.hThread );
    CloseHandle( rPI.hProcess );
    remove( "in.hlsl" );
    remove( "out.hlsl" );
    return 0;
  }

  TS8 *Out = Data;
  while ( strlen( Out ) )
  {
    TS32 Wr = 0;
    WriteFile( hWriteIN, Out, strlen( Out ), (LPDWORD)&Wr, NULL );
    Out += Wr;
  }
  CloseHandle( hWriteIN );

  char dest[ 1000 ];
  CString sProgress = "";
  DWORD dwRetFromWait = WAIT_TIMEOUT;
  while ( dwRetFromWait != WAIT_OBJECT_0 )
  {
    dwRetFromWait = WaitForSingleObject( rPI.hProcess, 100 );
    if ( dwRetFromWait == WAIT_ABANDONED ) break;

    while ( ReadFromPipeNoWait( hReadPipe, dest, sizeof( dest ) ) > 0 )
      Result += dest;
  }

  CloseHandle( hReadIN );
  CloseHandle( hReadPipe );
  CloseHandle( hWritePipe );
  CloseHandle( rPI.hThread );
  CloseHandle( rPI.hProcess );

  if ( Result.Length() )
  {
    LOG_ERR( "[minifier] %s", Result.GetPointer() );
    remove( "in.hlsl" );
    remove( "out.hlsl" );
    return false;
  }

  CStreamReaderFile f;
  f.Open( _T( "out.hlsl" ) );

  TS8 *text = new TS8[ (TS32)( f.GetLength() ) + 1 ];
  memset( text, 0, (size_t)f.GetLength() + 1 );
  f.Read( text, (size_t)f.GetLength() );

  Result = CString( text );

  SAFEDELETEA( text );

  if ( Verbose )
    LOG_DBG( "[minifier] %s", Result.GetPointer() );

  remove( "in.hlsl" );
  remove( "out.hlsl" );
  return true;
}
