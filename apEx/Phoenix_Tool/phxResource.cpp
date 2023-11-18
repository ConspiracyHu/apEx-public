#include "BasePCH.h"
#include "phxResource.h"

#pragma warning(disable:4073)
#pragma init_seg(lib) //this is needed so the resource database is created before everything else and freed after everything else (except the memory tracking)

CArray<CphxResource*> ResourceDatabase;
CArray<CphxResource*> UpdateQueue;
TS32 TouchCounter = 0;

void ClearRequiredFlagForAllResources()
{
  for ( TS32 x = 0; x < ResourceDatabase.NumItems(); x++ )
  {
    ResourceDatabase[ x ]->SetRequired( false );
    ResourceDatabase[ x ]->SetExportedFlag( false );
    ResourceDatabase[ x ]->ClearRenderRequirement();
  }
}

CphxResource::CphxResource()
{
  Guid = GUIDServer.RequestGUID();
  UpToDate = false;
  Allocated = false;
  LockCount = 0;
  ResourceDatabase.AddUnique( this );
  Touch();
}

CphxResource::~CphxResource()
{
  UpdateQueue -= this;
  ResourceDatabase -= this;
  //Lock(false);
  Allocate( false );
  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
    Parents[ x ]->Children.Delete( this );
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    Children[ x ]->Parents.Delete( this );

  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
    Parents[ x ]->WeakChildren.Delete( this );
  for ( TS32 x = 0; x < WeakChildren.NumItems(); x++ )
    WeakChildren[ x ]->Parents.Delete( this );
}

void CphxResource::InvalidateUptoDateFlag()
{
  //if (!UpToDate) return;

  //LOG_DBG("[resource] Invalidating uptodate flag for resource %x",this);

  UpToDate = false;
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    Children[ x ]->InvalidateUptoDateFlag();
}

void CphxResource::InvalidateChildren()
{
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    Children[ x ]->InvalidateUptoDateFlag();
}

void CphxResource::RequestContent()
{
  if ( ContentReady() ) return;
  if ( !CanBeGenerated() ) return;

  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
    Parents[ x ]->RequestContent();

  CphxResource *ct = GetContentResource();

  if ( UpdateQueue.Find( ct ) >= 0 ) return;
  UpdateQueue += ct;
}

TBOOL CphxResource::CanBeGenerated()
{
  TBOOL r = InputsValid();
  for ( int x = 0; x < Parents.NumItems() && r; x++ )
    r = r&&Parents[ x ]->CanBeGenerated();
  return r;
}

TBOOL CphxResource::InputsValid()
{
  return true;
}

void CphxResource::Generate( CCoreDevice *Dev )
{
  Touch();
  if ( ContentReady() ) return;
  //if (IsUpToDate()) return;
  Allocate( true );
  UpToDate = GenerateResource( Dev );
  //if (Children.NumItems()) Lock(true);
}

void CphxResource::RemoveParents( PHXRESOURCETYPE type )
{
  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
  {
    if ( Parents[ x ]->GetType() == type )
    {
      Parents[ x ]->Children.Delete( this );
      Parents.DeleteByIndex( x );
      x--;
    }
  }
}

TS32 CphxResource::GetParentsOfType( PHXRESOURCETYPE type, CphxResource *Buffer[], TS32 BufferSize )
{
  TS32 idx = 0;
  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
  {
    if ( Parents[ x ]->GetType() == type )
      Buffer[ idx++ ] = Parents[ x ];
    if ( idx == BufferSize ) return idx;
  }
  return idx;
}

void CphxResource::AddParent( CphxResource *r, bool weak )
{
  if ( !r ) return;
  Parents.AddUnique( r );
  if ( !weak )
    r->Children.AddUnique( this );
  else
    r->WeakChildren.AddUnique( this );
}

void CphxResource::RemoveParent( CphxResource *r )
{
  if ( !r ) return;
  Parents.Delete( r );
  r->Children.Delete( this );
  r->WeakChildren.Delete( this );
}

CphxResource * CphxResource::GetParent( PHXRESOURCETYPE type, TS32 idx )
{
  TS32 id = 0;
  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
  {
    if ( Parents[ x ]->GetType() == type )
    {
      if ( idx == id ) return Parents[ x ];
      id++;
    }
  }
  return NULL;
}

CphxResource * CphxResource::GetChild( PHXRESOURCETYPE type, TS32 idx )
{
  TS32 id = 0;
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
  {
    if ( Children[ x ]->GetType() == type )
    {
      if ( idx == id ) return Children[ x ];
      id++;
    }
  }
  return NULL;
}


CphxResource* CphxResource::GetWeakChild( PHXRESOURCETYPE type, TS32 idx )
{
  TS32 id = 0;
  for ( TS32 x = 0; x < WeakChildren.NumItems(); x++ )
  {
    if ( WeakChildren[ x ]->GetType() == type )
    {
      if ( idx == id ) return WeakChildren[ x ];
      id++;
    }
  }
  return NULL;
}

//void CphxResource::Lock( TBOOL Loc )
//{
//	Locked=Loc; 
//	LockMechanism(Loc);
//}

void CphxResource::Allocate( TBOOL Alloc )
{
  if ( IsLocked() && Alloc == false )
  {
    LOG_DBG( "[resource] Trying to deallocate locked resource!" );
    return;
  }

  if ( Allocated == Alloc ) return;
  AllocatorMechanism( Alloc );
  Allocated = Alloc;
}

void CphxResource::Export( CXMLNode *Node )
{
  ExportGUID( Node );
  ExportData( Node );
}

void CphxResource::Import( CXMLNode *Node )
{
  ImportGUID( Node );
  ImportData( Node );
}

void CphxResource::ExportGUID( CXMLNode *Node )
{
  Node->AddChild( _T( "GUID" ), false ).SetText( Guid.GetString() );
}

void CphxResource::ImportGUID( CXMLNode *Node )
{
  if ( !Node->GetChildCount( _T( "GUID" ) ) )
  {
    LOG_ERR( "[resource] GUID Missing from resource node in XML!" );
    Guid.SetString( "BADGUID!BADGUID!BADGUID!BADGUID!" );
    return;
  }
  Guid.SetString( Node->GetChild( _T( "GUID" ) ).GetText().GetPointer() );
}

TS32 CphxResource::GetParentCount( PHXRESOURCETYPE type )
{
  TS32 cnt = 0;
  for ( TS32 x = 0; x < Parents.NumItems(); x++ )
    if ( Parents[ x ]->GetType() == type ) cnt++;
  return cnt;
}

TS32 CphxResource::GetChildCount( PHXRESOURCETYPE type )
{
  TS32 cnt = 0;
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    if ( Children[ x ]->GetType() == type ) cnt++;
  return cnt;
}

TS32 CphxResource::GetWeakChildCount( PHXRESOURCETYPE type )
{
  TS32 cnt = 0;
  for ( TS32 x = 0; x < WeakChildren.NumItems(); x++ )
    if ( WeakChildren[ x ]->GetType() == type ) cnt++;
  return cnt;
}

TBOOL CphxResource::IsRequiredForGenerator()
{
  for ( TS32 x = 0; x < UpdateQueue.NumItems(); x++ )
  {
    if ( UpdateQueue[ x ] == this ) return true;
    for ( TS32 y = 0; y < UpdateQueue[ x ]->GetParentCount(); y++ )
      if ( UpdateQueue[ x ]->GetParent( y )->GetContentResource() == this || UpdateQueue[ x ]->GetParent( y )->GetContentResource() == GetContentResource() ) return true;
  }
  return false;
}

TBOOL CphxResource::ParentContentReady()
{
  TBOOL r = true;
  for ( int x = 0; x < Parents.NumItems(); x++ )
    r = r&&Parents[ x ]->ContentReady();
  return r;
}

void CphxResource::Touch()
{
  LastTouched = TouchCounter++;
}

TBOOL CphxResource::HasChildrenOfType( PHXRESOURCETYPE type )
{
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    if ( Children[ x ]->GetType() == type ) return true;
  return false;
}

void CphxResource::MarkAsRequired()
{
  for ( int y = 0; y < 2; y++ ) // ugly hack to avoid ordering issues with special required stuff
  {
    MarkSpecialRequired();

    Required = true;
    for ( TS32 x = 0; x < Parents.NumItems(); x++ )
      Parents[ x ]->MarkAsRequired();

    MarkSpecialRequired();
  }
}

TS32 CphxResource::GetResourceIndex()
{
  if ( !Required )
    LOG_ERR( "[resource] Minimal export error! Reference request for not flagged resource!" );
  return ResourceIndex;
}

#include <MMSystem.h>

void StepGeneratorQueue( CCoreDevice *Dev )
{
  if ( !UpdateQueue.NumItems() ) return; //nothing to do
  CphxResource *f = UpdateQueue[ 0 ]->GetContentResource();

  if ( f->ContentReady() )
  {
    //something got fucked up, this shouldn't ever happen
    UpdateQueue.Delete( f );
    LOG_ERR( "[resource] Something weird happened during generation" );
    return;
  }

  if ( !f->CanBeGenerated() )
  {
    //the conditions have been changed while the item was waiting in queue and this item can't be generated anymore
    UpdateQueue.Delete( f );
  }

  if ( !f->ParentContentReady() )
  {
    //some parent got invalidated while this item waited to be processed, move to the end of the queue
    UpdateQueue.Delete( f );
    f->RequestContent();
    return;
  }

  //everything ok, generate the resource

  f->Generate( Dev );

  UpdateQueue.Delete( f );

  if ( !f->ContentReady() )
    LOG_ERR( "[resource] Allocation error!" );

  //mark unneeded parents data as unused
  //for (TS32 pc=0; pc<f->GetParentCount(); pc++)
  //{
  //	CphxResource *parent=f->GetParent(pc)->GetContentResource();

  //	TBOOL ParentNeeded=false;
  //	for (TS32 y=0; y<UpdateQueue.NumItems() && !ParentNeeded; y++)
  //	{
  //		for (TS32 z=0; z<UpdateQueue[y]->GetParentCount(); z++)
  //			if (UpdateQueue[y]->GetParent(z)->GetContentResource()==parent)
  //			{
  //				ParentNeeded=true;
  //				break;
  //			}
  //	}

  //	if (!ParentNeeded) 
  //	{
  //		parent->Allocate(false);
  //		//parent->FreeIfNeeded();
  //	}
  //}

}

TS32 ProcessGeneratorQueue( TU32 Timelimit, CCoreDevice *Dev )
{
  TU32 original = timeGetTime();
  TS32 pos = 0;

  while ( UpdateQueue.NumItems() )
  {
    StepGeneratorQueue( Dev );
    pos++;
    if ( timeGetTime() - original > Timelimit ) break;
  }

  //for (TS32 x=0; x<UpdateQueue.NumItems(); x++)
  //{
  //	UpdateQueue[x]->Generate(Dev);
  //	pos=x;

  //	//mark unneeded parents data as unused
  //	for (TS32 pc=0; pc<UpdateQueue[x]->GetParentCount(); pc++)
  //	{
  //		CphxResource *parent=UpdateQueue[x]->GetParent(pc)->GetContentResource();

  //		TBOOL ParentNeeded=false;
  //		for (TS32 y=pos+1; y<UpdateQueue.NumItems() && !ParentNeeded; y++)
  //		{
  //			for (TS32 z=0; z<UpdateQueue[y]->GetParentCount(); z++)
  //				if (UpdateQueue[y]->GetParent(z)->GetContentResource()==parent)
  //				{
  //					ParentNeeded=true;
  //					break;
  //				}
  //		}

  //		if (!ParentNeeded) 
  //		{
  //			//parent->Allocate(false);
  //			parent->FreeIfNeeded();
  //		}
  //	}

  //	if (timeGetTime()-original>Timelimit) break;
  //}
  //UpdateQueue.TrimHead(pos+1);

  Dev->ForceStateReset();
  //API->SetUIRenderState();
  return pos + 1;
}

void GenerateAll( CCoreDevice *Dev )
{
  while ( UpdateQueue.NumItems() )
    StepGeneratorQueue( Dev );
}