#pragma once
#include "../../Bedrock/CoRE2/Core2.h"
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../Phoenix/Texgen.h"
#include "phxResource.h"

class CphxSpline_Tool_float16;

class CphxTextureFilterParameter
{
public:

  CString Name;
  TS32 Type;
  TS32 Minimum, Maximum, Default;
  CArray<CString> ListItems;

  CphxTextureFilterParameter();
};

class CphxTextureFilter_Tool : public CphxResource
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  PHXTEXTUREFILTER Filter;
  CString Name;
  CString Code;
  CString CodeBackup; // for shader minimization purposes
  TS32 Type;
  CArray<CphxTextureFilterParameter*> Parameters;
  CCorePixelShader *Shader;
  TBOOL Minifiable = true;

  TBOOL External; //used to determine whether this filter was loaded from a project or the filter lib
  int useCount; // count during minimal export

  CphxTextureFilter_Tool();
  virtual ~CphxTextureFilter_Tool();

  TS32 GetParamStart( TS32 ParamID );
  TS32 GetParamSize( TS32 ParamID );

  virtual TBOOL ContentReady() { return UpToDate && Shader; }

  TBOOL GenerateResource( CCoreDevice *Dev );

  virtual PHXRESOURCETYPE GetType() { return PHX_TEXTUREFILTER; }

};

class CphxTexturePoolTexture_Tool : public CphxTexturePoolTexture
{
public:

  //CphxResource *Owner;

  CphxTexturePoolTexture_Tool();
  virtual ~CphxTexturePoolTexture_Tool();
  virtual bool Create( unsigned char res, bool hdr );

  TS32 DumpTexture();

};

typedef int APEXOPID;

class CphxTextureOperator_Subroutine;

enum APEXTEXTUREOPTYPE
{
  TEXGEN_OP_NORMAL = 0,
  TEXGEN_OP_SAVE = 1,
  TEXGEN_OP_LOAD = 2,
  TEXGEN_OP_NOP = 3,
  TEXGEN_OP_SUBROUTINE = 4,
  TEXGEN_OP_SUBROUTINECALL = 5,
};

class CphxTextureOperator_Tool : public CphxResource
{
  virtual void LockMechanism( TBOOL Lock );
  virtual void AllocatorMechanism( TBOOL Alloc );

protected:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  TU8 *ImageData;
  TS32 ImageDataSize;
  PHXTEXTDATA TextData;
  char *Text;
  CphxSpline_Tool_float16 *Curves[ 4 ];
  bool UiVisibleCurveChannels[ 4 ];
  float UiCurveZoomX = 1;
  float UiCurveZoomY = 1;
  float UiCurveOffsetX = 0;
  float UiCurveOffsetY = 0;

  CphxTexturePoolTexture_Tool *Result;

  PHXTEXTUREOPERATOR OpData;

  APEXOPID ID;
  CphxGUID FilterGuid;
  CphxTextureFilter_Tool *Filter;

  CRect OldPosition, Position;
  CString Name;
  TBOOL Selected;
  TBOOL NeedsInvalidation;

  TBOOL SubroutineTagged;
  TBOOL TouchedByLoop;

  class CapexTexGenPage *ParentPage = NULL; //only used to be used by the minimalexporter, now it's used by the hdr stuff as well
  CString BuildsTexture = ""; // used by minimalexporter for symbol export to write out which operator belongs to which texture
  void SetBuildsTextureSymbolRecursive( const CString& symbol );

  CphxTextureOperator_Subroutine *SubroutineRoot;

  CphxTextureOperator_Tool( CRect Pos, CapexTexGenPage *parentPage );
  virtual ~CphxTextureOperator_Tool();

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual TBOOL InputsValid();

  virtual PHXRESOURCETYPE GetType() { return PHX_TEXTUREOPERATOR; }

  virtual CphxTextureOperator_Tool *Copy();

  virtual void FreeIfNeeded();

  virtual CphxTextureOperator_Tool *GetContentOp() { return this; }
  virtual CphxResource *GetContentResource() { return GetContentOp(); }

  virtual TU32 GetInputCount();

  virtual CString GetName();
  virtual APEXTEXTUREOPTYPE GetOpType() { return TEXGEN_OP_NORMAL; }

  //virtual TBOOL IsSubroutine() { return false; }
  //virtual TBOOL IsSave() { return false; }
  //virtual TBOOL IsSubroutineCall() { return false; }
  //virtual TBOOL IsLoad() { return false; }
  //virtual TBOOL IsNOP() { return false; }

  virtual void ApplyItemSpecificParents() {}
  virtual void RemoveItemSpecificParents() {}

  void SetSubroutine( CphxTextureOperator_Subroutine *Sub );

  void FreeSubroutineAllocationsandUpdateResolution( CphxTextureOperator_Subroutine *Root, TU8 Res );
  void ClearSubroutineTags( CphxTextureOperator_Subroutine *Root );
  void SetSubroutineInputOperators( CphxTextureOperator_Subroutine *Root, CphxTextureOperator_Tool **Inputs, TS32 NumInputs, TS32 &InputIdx );
  TS32 CountSubInputOps( CphxTextureOperator_Subroutine *Root );
  void ClearSubInputs( CphxTextureOperator_Subroutine *Root );
  void ClearSubRoots( CphxTextureOperator_Subroutine *Root );

  virtual void CallRender( CphxTexturePoolTexture *&Target, CphxTexturePoolTexture *&SwapBuffer, CphxTexturePoolTexture *Inputs[ TEXGEN_MAX_PARENTS ], unsigned char RandSeed, unsigned char Parameters[ TEXGEN_MAX_PARAMS ] );
  void CopyLookupData( CphxTextureOperator_Tool *src );

  virtual void KillLoops();
  virtual void KillLoopCause() {};

  virtual TBOOL UsedByMaterial();

  TBOOL SubTreeMatch( CphxTextureOperator_Tool *Op );

};

class CphxTextureOperator_NOP : public CphxTextureOperator_Tool
{
  virtual void LockMechanism( TBOOL Lock );
  virtual void AllocatorMechanism( TBOOL Alloc );
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:
  CphxTextureOperator_NOP( CRect Pos, CapexTexGenPage *parentPage );
  virtual ~CphxTextureOperator_NOP();

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual TBOOL InputsValid();
  virtual CphxTextureOperator_Tool *Copy();

  virtual CphxTextureOperator_Tool *GetContentOp();
  virtual TU32 GetInputCount();
  virtual TBOOL ContentReady();
  virtual CString GetName();

  //virtual TBOOL IsNOP() { return true; }
  virtual APEXTEXTUREOPTYPE GetOpType() { return TEXGEN_OP_NOP; }
  virtual TBOOL IsRequiredForGenerator();
};

class CphxTextureOperator_Save : public CphxTextureOperator_NOP
{

public:
  CphxTextureOperator_Save( CRect Pos, CapexTexGenPage *parentPage );
  virtual ~CphxTextureOperator_Save();
  virtual CphxTextureOperator_Tool *Copy();

  virtual CString GetName();
  //virtual TBOOL IsSave() { return true; }
  virtual APEXTEXTUREOPTYPE GetOpType() { return TEXGEN_OP_SAVE; }
  virtual TBOOL IsRequiredForGenerator();

};

class CphxTextureOperator_Load : public CphxTextureOperator_NOP
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:
  CphxGUID LoadTime_TargetOpGUID;

  CphxTextureOperator_Tool *LoadedOp;

  CphxTextureOperator_Load( CRect Pos, CapexTexGenPage *parentPage );
  virtual ~CphxTextureOperator_Load();
  virtual CphxTextureOperator_Tool *Copy();

  virtual CString GetName();
  virtual APEXTEXTUREOPTYPE GetOpType() { return TEXGEN_OP_LOAD; }
  //virtual TBOOL IsLoad() { return true; }
  virtual void ApplyItemSpecificParents();
  virtual void RemoveItemSpecificParents();
  virtual TBOOL InputsValid() { return LoadedOp != NULL; }
  virtual void KillLoopCause();
  virtual TBOOL IsRequiredForGenerator();

};

class CphxTextureOperator_SubroutineCall;

class CphxSubroutineParamConnection
{
public:
  APEXOPID TargetID;
  CphxGUID TargetGUID;
  TS32 OpParam;

  CphxSubroutineParamConnection();
};

class CphxSubroutineParam
{
public:
  CString Name;
  TS32 Type;
  TS32 Minimum;
  TS32 Maximum;
  TS32 Default;
  CArray<CphxSubroutineParamConnection*> Targets;

  CphxSubroutineParam();
  virtual ~CphxSubroutineParam();
};

class CphxTextureOperator_Subroutine : public CphxTextureOperator_NOP
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  CArray<CphxSubroutineParam*> Parameters;

  CphxTextureOperator_Subroutine( CRect Pos, CapexTexGenPage *parentPage );
  virtual ~CphxTextureOperator_Subroutine();
  virtual CphxTextureOperator_Tool *Copy();

  virtual CString GetName();
  //virtual TBOOL IsSubroutine() { return true; }
  virtual APEXTEXTUREOPTYPE GetOpType() { return TEXGEN_OP_SUBROUTINE; }
  virtual TBOOL InputsValid();
  virtual TBOOL CanBeGenerated();

  virtual void SetCaller( CphxTextureOperator_SubroutineCall *Caller );

  TS32 GetParamStart( TS32 ParamID );
  TS32 GetParamSize( TS32 ParamID );
  virtual TBOOL IsRequiredForGenerator();

};

class CphxTextureOperator_SubroutineCall : public CphxTextureOperator_Tool
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:
  CphxGUID Loadtime_SubroutineGUID; //only used during import to store subroutines that are loaded at a later time

  CphxTextureOperator_Subroutine *Subroutine;

  CphxTextureOperator_SubroutineCall( CRect Pos, CapexTexGenPage *parentPage );
  virtual ~CphxTextureOperator_SubroutineCall();

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual TBOOL InputsValid();
  virtual CphxTextureOperator_Tool *Copy();

  virtual TU32 GetInputCount();

  virtual CString GetName();

  virtual void ApplyItemSpecificParents();
  virtual void RemoveItemSpecificParents();
  //virtual TBOOL IsSubroutineCall() { return true; }
  virtual APEXTEXTUREOPTYPE GetOpType() { return TEXGEN_OP_SUBROUTINECALL; }
  virtual void KillLoopCause();
  virtual void RequestContent();
  virtual TBOOL ParentContentReady();
};


class CphxTexturePool_Tool : public CphxTexturePool
{
public:
  CphxArray<CphxTexturePoolTexture_Tool *> Pool;
  CphxTexturePool_Tool();
  virtual CphxTexturePoolTexture *GetTexture( unsigned char Resolution, bool hdr );
  void Clear();

#ifndef PHX_MINIMAL_BUILD
  virtual ~CphxTexturePool_Tool() {};
#endif
};
