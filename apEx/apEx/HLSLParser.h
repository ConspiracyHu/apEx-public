#pragma once
#include "HlslLexer.h"

namespace CrossCompiler
{
  enum class EParseResult
  {
    Matched,
    NotMatched,
    Error,
  };

  namespace Parser
  {
    // Returns true if successfully parsed
    extern CDictionary<TCHAR, TS32> NumStats;
    bool Parse( const CString& Input, CString &Result, TBOOL Verbose = false, TBOOL ResetDictionary = true, TBOOL RebuildDictionary = true );

    bool BuildShaderBlob( const CArray<CString>& shaders );

  }
}
