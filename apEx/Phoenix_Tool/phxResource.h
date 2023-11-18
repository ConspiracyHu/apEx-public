#pragma once
#include "../../Bedrock/CoRE2/Core2.h"
#include "../../Bedrock/UtilLib/XMLDocument.h"
#include "GUID.h"

enum PHXRESOURCETYPE
{
  PHX_TEXTUREFILTER = 0,
  PHX_TEXTUREOPERATOR = 1,
  PHX_MATERIALTECH = 2,
  PHX_RENDERTARGET = 3,
  PHX_EVENT = 4,
  PHX_MATERIALPARAM = 5,
  PHX_MODEL = 6,
  PHX_MODELOBJECT = 7,
  PHX_MATERIAL = 8,
  PHX_RENDERLAYERDESCRIPTOR = 9,
  PHX_SCENE = 10,
  PHX_OBJECT = 11,
  PHX_CLIP = 12,
  PHX_TREESPECIES = 13,
};

class CphxResource
{

  CphxGUID Guid;

  virtual TBOOL GenerateResource( CCoreDevice *Dev ) = 0;
  virtual void LockMechanism( TBOOL Lock ) {};
  virtual void AllocatorMechanism( TBOOL Alloc ) {};
  virtual void ExportData( CXMLNode *Node ) = 0;
  virtual void ImportData( CXMLNode *Node ) = 0;

  TBOOL Allocated;
  TS32 LockCount;
  TS32 LastTouched;

  CArray<CphxResource*> Parents;	//dependencies
  CArray<CphxResource*> Children;	//these items require this one
  CArray<CphxResource*> WeakChildren;	//these items require this one, but don't need to be re-generated if this changes

protected:

  TBOOL UpToDate;
  TBOOL Required;
  TBOOL Exported;
  TS32 ResourceIndex;
  TBOOL NeedsRender; //texgen export info to mark endpoint operators

  void ExportGUID( CXMLNode *Node );

public:

  int useCount = 0;

  CphxResource();
  virtual ~CphxResource();

  virtual void Export( CXMLNode *Node );
  virtual void Import( CXMLNode *Node );
  void ImportGUID( CXMLNode *Node );

  CphxGUID &GetGUID() { return Guid; }
  TBOOL HasDependants() { return Children.NumItems() > 0; }

  //void AddDependency(CphxResource *Resource) { Parents+=Resource; Resource->Children+=this; }

  void InvalidateUptoDateFlag();
  void InvalidateChildren();

  virtual void RequestContent();
  virtual TBOOL ContentReady() { return UpToDate && Allocated; }
  virtual TBOOL ParentContentReady();
  virtual TBOOL CanBeGenerated();
  virtual TBOOL InputsValid();
  virtual void Generate( CCoreDevice *Dev );
  virtual TBOOL HasChildrenOfType( PHXRESOURCETYPE type );

  virtual PHXRESOURCETYPE GetType() = 0;

  void RemoveParents( PHXRESOURCETYPE type );
  TS32 GetParentsOfType( PHXRESOURCETYPE type, CphxResource *Buffer[], TS32 BufferSize );

  void AddParent( CphxResource *r, bool weak = false );
  void RemoveParent( CphxResource *r );
  CphxResource *GetParent( PHXRESOURCETYPE type, TS32 idx );
  CphxResource *GetChild( PHXRESOURCETYPE type, TS32 idx );
  CphxResource* GetWeakChild( PHXRESOURCETYPE type, TS32 idx );

  TS32 GetParentCount() { return Parents.NumItems(); }
  TS32 GetChildCount() { return Children.NumItems(); }
  TS32 GetParentCount( PHXRESOURCETYPE type );
  TS32 GetChildCount( PHXRESOURCETYPE type );
  TS32 GetWeakChildCount( PHXRESOURCETYPE type );

  CphxResource *GetParent( TS32 x ) { return Parents[ x ]; }
  CphxResource *GetChild( TS32 x ) { return Children[ x ]; }
  CphxResource* GetWeakChild( TS32 x ) { return WeakChildren[ x ]; }

  virtual void Allocate( TBOOL Alloc );
  virtual void Lock( TBOOL val ) { LockCount = val ? 1 : 0; }
  TBOOL IsLocked() { return LockCount > 0; }

  virtual void FreeIfNeeded() {};
  TBOOL IsAllocated() { return Allocated; }

  void ForceUptodateFlag( TBOOL value ) { UpToDate = value; }

  TS32 GetLastTouchTime() { return LastTouched; }

  virtual TBOOL IsRequiredForGenerator();

  virtual CphxResource *GetContentResource() { return this; }

  void Touch();

  virtual void UpdateDependencies() {}

  void SetRequired( TBOOL req ) { Required = req; }
  TBOOL IsRequired() { return Required; }
  void MarkAsRequired();
  virtual void MarkSpecialRequired() {};
  void SetResourceIndex( TS32 idx ) { ResourceIndex = idx; }
  TS32 GetResourceIndex();
  TBOOL AlreadyExported() { return Exported; }
  void SetExportedFlag( TBOOL e = true ) { Exported = e; }
  void SetRenderRequirement() { NeedsRender = true; }
  void ClearRenderRequirement() { NeedsRender = false; }
  TBOOL IsRendered() { return NeedsRender; }

};

TS32 ProcessGeneratorQueue( TU32 Timelimit, CCoreDevice *Dev );
void ClearRequiredFlagForAllResources();
void GenerateAll( CCoreDevice *Dev );
