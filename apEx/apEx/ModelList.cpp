#include "BasePCH.h"
#include "ModelList.h"
#define WINDOWNAME _T("Model List")
#define WINDOWXML _T("ModelList")
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"

CapexModelList::CapexModelList() : CapexWindow()
{
}

CapexModelList::CapexModelList( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexModelList::~CapexModelList()
{

}

void CapexModelList::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( !List ) return;

  TS32 Cursor = List->GetCursorPosition();

  List->Flush();

  bool found = false;

  for ( TS32 x = 0; x < Project.GetModelCount(); x++ )
  {
    auto *model = Project.GetModelByIndex( x );
    bool usedModel = model->GetChildCount( PHX_OBJECT ) != 0 || model->GetWeakChildCount( PHX_OBJECT ) != 0;

    auto itemId = List->AddItem( model->GetName() );

    if ( usedModel )
      List->SetItemColor( itemId, CColor( 255, 255, 154, 255 ) );
    
    if ( GetWorkBench()->GetEditedModel() == Project.GetModelByIndex( x ) )
    {
      List->SelectItemByIndex( x );
      found = true;
    }
  }

  if ( !found )
    List->SelectItemByIndex( Cursor );
}

TBOOL CapexModelList::MessageProc( CWBMessage &Message )
{

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "newmodel" ) )
    {
      CreateModel();

      return true;
    }

    if ( b->GetID() == _T( "deletemodel" ) )
    {
      CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
      if ( !List ) return true;

      DeleteModel( WorkBench->GetEditedModel() );

      return true;
    }

    if ( b->GetID() == _T( "copymodel" ) )
    {
      CopyModel();
      return true;
    }

  }
  break;

  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      CphxModel_Tool *t = Project.GetModelByIndex( List->GetCursorPosition() );

      for ( int x = 0; x < Project.GetSceneCount(); x++ )
      {
        for ( int y = 0; y < Project.GetSceneByIndex( x )->GetObjectCount(); y++ )
        {
          auto *obj = Project.GetSceneByIndex( x )->GetObjectByIndex( y );
          if ( obj->GetObjectType() == Object_Model )
          {
            CphxObject_Model_Tool* modObj = (CphxObject_Model_Tool*)obj;
            if ( modObj->Model && modObj->Model == t && modObj->GetName() == modObj->Model->GetName() )
            {
              modObj->SetName( List->GetCursorItem()->GetText() );
            }
          }
        }
      }

      if ( t ) t->SetName( List->GetCursorItem()->GetText() );

      WorkBench->UpdateWindows( apEx_ScenePrimitives );
      WorkBench->UpdateWindows( apEx_SceneSplineEditor );
      WorkBench->UpdateWindows( apEx_SceneGraph );
      return true;
    }
  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
    if ( !List ) break;
    if ( List->GetID() == Message.GetTargetID() )
    {
      WorkBench->SetEditedModel( Project.GetModelByIndex( List->GetCursorPosition() ) );
      return true;
    }

  }
  break;

  default:
    break;
  }


  return CapexWindow::MessageProc( Message );
}

void CapexModelList::CreateModel()
{
  CphxModel_Tool *t = Project.CreateModel();
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->AddItem( t->GetName() );
    SelectModel( List->NumItems() - 1 );
  }
  GetWorkBench()->UpdateWindows( apEx_ScenePrimitives );
}

void CapexModelList::CopyModel()
{
  CphxModel_Tool *EditedModel = GetWorkBench()->GetEditedModel();
  if ( !EditedModel ) return;

  CphxModel_Tool *t = Project.CreateModel();
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( List )
  {
    t->SetName( CString( _T( "Copy of " ) ) + EditedModel->GetName() );

    for ( TS32 x = 0; x < EditedModel->GetObjectCount(); x++ )
    {
      CphxModelObject_Tool *eo = EditedModel->GetObjectByIndex( x );

      CphxModelObject_Tool_Mesh *o = (CphxModelObject_Tool_Mesh *)t->AddPrimitive( eo->Primitive );
      o->SetName( eo->GetName() );
      o->SetMatrix( eo->GetMatrix() );
      o->FloatParameter = eo->FloatParameter;
      o->miniModelTriangles = eo->miniModelTriangles;
      o->miniModelVertices = eo->miniModelVertices;
      for ( int y = 0; y < 12; y++ ) o->Parameters[ y ] = eo->Parameters[ y ];

      if ( o->GetPrimitive() != Mesh_Clone )
      {
        for ( int y = 0; y < 2; y++ )
          o->ParentGUIDS[ y ] = eo->ParentGUIDS[ y ];
        for ( int y = 0; y < eo->Filters.NumItems(); y++ )
        {
          CphxMeshFilter_Tool *f = o->AddFilter( eo->Filters[ y ]->Filter );
          f->Parameters[ 0 ] = eo->Filters[ y ]->Parameters[ 0 ];
          f->Parameters[ 1 ] = eo->Filters[ y ]->Parameters[ 1 ];
          for ( int z = 0; z < 12; z++ )
          {
            f->srt[ z ] = eo->Filters[ y ]->srt[ z ];
            f->_srt[ z ] = eo->Filters[ y ]->_srt[ z ];
          }
          f->Enabled = eo->Filters[ y ]->Enabled;
          f->Texture = eo->Filters[ y ]->Texture;
          if ( f->Filter == ModelFilter_TintMesh && f->Texture ) o->AddParent( f->Texture );
        }

        o->Text = eo->Text;

        o->SetMaterial( eo->GetMaterial() );

        o->MaterialData.Copy( &( (CphxModelObject_Tool_Mesh *)eo )->MaterialData );
      }
      else
      {
        CphxModelObject_Tool_Clone *c = (CphxModelObject_Tool_Clone*)o;
        CphxModelObject_Tool_Clone *v = (CphxModelObject_Tool_Clone*)eo;
        //for (int y = 0; y < v->ClonedObjects.NumItems(); y++)
        //	c->AddClonedObject(v->ClonedObjects[y]);
      }

      //o->UpdateDependencies();


      // t->AddPrimitive(o->GetPrimitive());
    }

    for ( TS32 x = 0; x < EditedModel->GetObjectCount(); x++ )
    {
      CphxModelObject_Tool *eo = EditedModel->GetObjectByIndex( x );
      CphxModelObject_Tool *no = t->GetObjectByIndex( x );

      for ( int y = 0; y < 2; y++ )
      {
        TS32 idx = EditedModel->GetObjectIndex( eo->ParentGUIDS[ y ] );
        if ( idx >= 0 && idx < t->GetObjectCount() )
          no->ParentGUIDS[ y ] = t->GetObjectByIndex( idx )->GetGUID();
      }

      if ( no->GetPrimitive() == Mesh_Clone )
      {
        CphxModelObject_Tool_Clone *ec = (CphxModelObject_Tool_Clone*)eo;
        CphxModelObject_Tool_Clone *nc = (CphxModelObject_Tool_Clone*)no;

        for ( int y = 0; y < ec->ClonedObjects.NumItems(); y++ )
        {
          TS32 idx = EditedModel->GetObjectIndex( ec->ClonedObjects[ y ]->GetGUID() );
          if ( idx >= 0 && idx < t->GetObjectCount() )
            nc->AddClonedObject( t->GetObjectByIndex( idx ) );
        }

      }

      no->UpdateDependencies();
    }

    List->AddItem( t->GetName() );
    SelectModel( List->NumItems() - 1 );
  }
  GetWorkBench()->UpdateWindows( apEx_ScenePrimitives );
}


void CapexModelList::DeleteModel( CphxModel_Tool *m )
{
  if ( !m ) return;
  if ( m->HasDependants() )
  {
    SetStatusbarText( _T( "Could not delete model as it is being used somewhere." ) );
    return;
  }

  Project.DeleteModel( m->GetGUID() );

  GetWorkBench()->UpdateWindows( apEx_ModelList );
  GetWorkBench()->UpdateWindows( apEx_ScenePrimitives );

  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( !List ) return;

  SelectModel( max( 0, min( Project.GetModelCount() - 1, List->GetCursorPosition() ) ) );
}

void CapexModelList::SelectModel( TS32 ID )
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "modellist" ), _T( "itemselector" ) );
  if ( !List ) return;

  List->SelectItemByIndex( ID );
  //List->SetCursorPosition(ID);
  WorkBench->SetEditedModel( Project.GetModelByIndex( ID ) );
}
