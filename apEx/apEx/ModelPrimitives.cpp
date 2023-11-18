#include "BasePCH.h"
#include "ModelPrimitives.h"
#include "ModelParameters.h"
#define WINDOWNAME _T("Model Primitives")
#define WINDOWXML _T("ModelPrimitives")

#include "WorkBench.h"
#include "ModelGraph.h"

CapexModelPrimitives::CapexModelPrimitives() : CapexWindow()
{
}

CapexModelPrimitives::CapexModelPrimitives( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexModelPrimitives::~CapexModelPrimitives()
{

}

void CapexModelPrimitives::UpdateData()
{

}

TBOOL CapexModelPrimitives::MessageProc( CWBMessage &Message )
{
  CapexWorkBench *wb = GetWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();

  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
  {
    switch ( Message.Key )
    {
    case VK_DELETE:
    {
      if ( WorkBench->GetEditedModel() )
      {
        WorkBench->GetEditedModel()->DeleteSelected();
        WorkBench->UpdateWindows( apEx_ModelGraph );
        WorkBench->UpdateWindows( apEx_ModelParameters );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
    }
    return true;
    break;
    case 'C':
    {
      if ( WorkBench->GetEditedModel() )
      {
        WorkBench->GetEditedModel()->CopySelected();
        WorkBench->UpdateWindows( apEx_ModelGraph );
        WorkBench->UpdateWindows( apEx_ModelParameters );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
    }
    return true;
    break;
    }

  }
  break;
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b || !model ) break;

    if ( b->GetID() == _T( "newcube" ) ) { CreatePrimitive( Mesh_Cube ); return true; }
    if ( b->GetID() == _T( "newplane" ) ) { CreatePrimitive( Mesh_Plane ); return true; }
    if ( b->GetID() == _T( "newsphere" ) ) { CreatePrimitive( Mesh_Sphere ); return true; }
    if ( b->GetID() == _T( "newgeosphere" ) ) { CreatePrimitive( Mesh_GeoSphere ); return true; }
    if ( b->GetID() == _T( "newcylinder" ) ) { CreatePrimitive( Mesh_Cylinder ); return true; }
    if ( b->GetID() == _T( "newcone" ) ) { CreatePrimitive( Mesh_Cone ); return true; }
    if ( b->GetID() == _T( "newarc" ) ) { CreatePrimitive( Mesh_Arc ); return true; }
    if ( b->GetID() == _T( "newline" ) ) { CreatePrimitive( Mesh_Line ); return true; }
    if ( b->GetID() == _T( "newspline" ) ) { CreatePrimitive( Mesh_Spline ); return true; }
    if ( b->GetID() == _T( "newloft" ) ) { CreatePrimitive( Mesh_Loft ); return true; }
    if ( b->GetID() == _T( "newscatter" ) ) { CreatePrimitive( Mesh_Scatter ); return true; }
    if ( b->GetID() == _T( "newtree" ) ) { CreatePrimitive( Mesh_Tree ); return true; }
    if ( b->GetID() == _T( "newleaves" ) ) { CreatePrimitive( Mesh_TreeLeaves ); return true; }
    if ( b->GetID() == _T( "newtext" ) ) { CreatePrimitive( Mesh_Text ); return true; }

    if ( b->GetID() == _T( "newstored" ) ) { CreatePrimitive( Mesh_Stored ); return true; }

    if ( b->GetID() == _T( "clone" ) ) { CreatePrimitive( Mesh_Clone ); return true; }
    if ( b->GetID() == _T( "bake" ) ) { CreatePrimitive( Mesh_Copy ); return true; }
    if ( b->GetID() == _T( "merge" ) ) { CreatePrimitive( Mesh_Merge ); return true; }


    if ( b->GetID() == _T( "uvmap" ) ) { CreateFilter( ModelFilter_UVMap ); return true; }
    if ( b->GetID() == _T( "tint" ) ) { CreateFilter( ModelFilter_TintMesh ); return true; }
    if ( b->GetID() == _T( "tintshape" ) ) { CreateFilter( ModelFilter_TintMeshShape ); return true; }
    if ( b->GetID() == _T( "mapxform" ) ) { CreateFilter( ModelFilter_MapXForm ); return true; }
    if ( b->GetID() == _T( "meshsmooth" ) ) { CreateFilter( ModelFilter_MeshSmooth ); return true; }
    if ( b->GetID() == _T( "smoothgroup" ) ) { CreateFilter( ModelFilter_SmoothGroup ); return true; }
    if ( b->GetID() == _T( "bevel" ) ) { CreateFilter( ModelFilter_Bevel ); return true; }
    if ( b->GetID() == _T( "replicate" ) ) { CreateFilter( ModelFilter_Replicate ); return true; }
    if ( b->GetID() == _T( "normaldeform" ) ) { CreateFilter( ModelFilter_NormalDeform ); return true; }
    if ( b->GetID() == _T( "csgfilter" ) ) { CreateFilter( ModelFilter_CSG ); return true; }
    if ( b->GetID() == _T( "greeble" ) ) { CreateFilter( ModelFilter_Greeble ); return true; }

  }
  break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexModelPrimitives::CreatePrimitive( PHXMESHPRIMITIVE p )
{
  CapexWorkBench *wb = GetWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();
  if ( !model ) return;

  if ( p == Mesh_Copy || p == Mesh_Merge )
  {
    if ( WorkBench->GetEditedModelObject() && WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone )
  	{
  		SetStatusbarText(_T("Cannot bake clones."));
  		return;
  	}
  }

  CphxModelObject_Tool *o = model->AddPrimitive( p );

  CapexModelGraph *mg = (CapexModelGraph *)WorkBench->GetWindow( apEx_ModelGraph );
  if ( !mg ) return;

  if ( p == Mesh_Copy )
  {
    if ( !WorkBench->GetEditedModelObject() ) return;
    TS32 i = model->GetObjectIndex( WorkBench->GetEditedModelObject() );
    if ( i >= 0 )
    {
      o->ParentGUIDS[ 0 ] = WorkBench->GetEditedModelObject()->GetGUID();
      o->AddParent( WorkBench->GetEditedModelObject() );
      o->GetParameters()[ 0 ] = i;
      o->SetMatrix( WorkBench->GetEditedModelObject()->GetMatrix() );
      WorkBench->UpdateWindows( apEx_ModelMatrix );

      if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Clone )
      {
        o->SetMaterial( WorkBench->GetEditedModelObject()->GetMaterial() );
        CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)o;
        mesh->MaterialData.Copy( &( (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelObject() )->MaterialData );
      }
    }
  }

  mg->UpdateData();
  CWBItemSelector *b = (CWBItemSelector*)mg->FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
  if ( !b ) return;
  b->SelectItemByIndex( b->NumItems() - 1 );
}

void CapexModelPrimitives::CreateFilter( PHXMESHFILTER p )
{
  CapexWorkBench *wb = GetWorkBench();
  CphxModel_Tool *model = NULL;
  if ( wb ) model = wb->GetEditedModel();
  if ( !model ) return;

  CphxModelObject_Tool *edobj = WorkBench->GetEditedModelObject();
  if ( !edobj && WorkBench->GetEditedModelFilter() ) edobj = WorkBench->GetEditedModelFilter()->ParentObject;

  if ( !edobj ) return;
  if ( edobj->GetPrimitive() == Mesh_Clone ) return;

  edobj->AddFilter( p );

  CapexModelGraph *mg = (CapexModelGraph *)WorkBench->GetWindow( apEx_ModelGraph );
  if ( !mg ) return;

  mg->UpdateData();

  CWBItemSelector *b = (CWBItemSelector*)mg->FindChildByID( _T( "objectlist" ), _T( "itemselector" ) );
  if ( !b ) return;

  for ( TS32 x = 0; x < mg->ModelFilters.NumItems(); x++ )
  {
    if ( mg->ModelFilters.GetByIndex( x ) == edobj->GetFilter( edobj->GetFilterCount() - 1 ) )
      b->SelectItem( mg->ModelFilters.GetKDPair( x )->Key );
  }
}

