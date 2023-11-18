#include "BasePCH.h"
#include "BuildInfo.h"
#include "BuildCount.h"

#ifdef _DEBUG
CString apexBuild = CString::Format( "%.4d_%dd", RELEASECOUNT, BUILDCOUNT );
#else
CString apexBuild = CString::Format( "%.4d_%dr", RELEASECOUNT, BUILDCOUNT );
#endif

CString buildDateTime = CString::Format( __DATE__ " " __TIME__ );

TS32 apexRelease = RELEASECOUNT;
TS32 apexBuildCount = BUILDCOUNT;