#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"

TS32 GetKKrunchedSize( TU8 *Data, TS32 Size );
TBOOL MinifyShader( char *Data, CString &Result, TBOOL Verbose = true );
TS32 GetKKrunchedSize( CString &FileName );
TBOOL Execute( TCHAR *prog, TCHAR *params, TBOOL visible = true );
void ExecuteWithConsoleLog( TCHAR *prog, TCHAR *params );
