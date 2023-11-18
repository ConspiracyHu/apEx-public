#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"

extern int UsedModelPrimitives[ 256 ];
extern int UsedModelFilters[ 256 ];
extern TBOOL MinimalExportSuccessful;

void ExportMinimal( CString TargetFile, bool zip, bool exportHeader );
void CreateMinimalConfig( TCHAR *TargetFile );

extern TS32 SplineCount;
extern TS32 SplineDataSize;

extern TS32 TexgenOpCount;
extern TS32 TexgenOpDataSize;

extern TS32 MaterialParamCount;
extern TS32 MaterialParamSize;

extern TS32 MaterialParameterDataCount;
extern TS32 MaterialParameterDataSize;

extern TS32 ModelFilterCount;
extern TS32 ModelFilterSize;

extern TS32 MaterialDataPackCount;

extern TS32 ModelObjectCount;
extern TS32 ModelObjectDataSize;

extern TS32 ClipSplineCount;

extern TS32 SceneObjectCount;
extern TS32 SceneObjectDataSize;

extern TS32 EventCount;
extern TS32 EventDataSize;

extern TS32 MiscDataSize;
extern TS32 SetupDataSize;

extern TS32 RenderTargetDataSize;
extern TS32 RenderLayerDataSize;

extern TS32 TextureFilterDataSize;
extern TS32 ShaderSize;

extern TS32 TexgenPageDataSize;

extern TS32 MaterialTechCount;
extern TS32 MaterialTechDataSize;

extern TS32 MaterialDescriptionSize;
extern TS32 ModelSetupSize;
extern TS32 SceneSetupSize;

extern TS32 TimelineSetupSize;
extern TS32 MinimalDataSize;