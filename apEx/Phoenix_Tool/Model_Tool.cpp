#include "BasePCH.h"
#include "Model_Tool.h"
#include "apxProject.h"
#include "Material_Tool.h"

void SetStatusbarText( CString Text );
void SetStatusbarError( CString Text );


void CphxModel_Tool::SetName( CString &name )
{
  Name = name;
}

CString CphxModel_Tool::GetName()
{
  return Name;
}

CphxModel_Tool::CphxModel_Tool()
{
  Name = _T( "New Model" );
}

CphxModel_Tool::~CphxModel_Tool()
{
  Objects.FreeArray();
}

void CphxModel_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Object" ) );
    n.SetAttributeFromInteger( _T( "Type" ), Objects[ x ]->GetPrimitive() );
    Objects[ x ]->Export( &n );
  }
}

void CphxModel_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Object" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Object" ), x );
    PHXMESHPRIMITIVE type = Mesh_Cube;
    if ( !n.HasAttribute( _T( "Type" ) ) ) continue;
    n.GetAttributeAsInteger( _T( "Type" ), (TS32*)&type );
    CphxModelObject_Tool *t = AddPrimitive( type );
    t->Import( &n );
  }

  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->UpdateDependencies();

}

TBOOL CphxModel_Tool::GenerateResource( CCoreDevice *Dev )
{
  return true;
}

CphxModelObject_Tool * CphxModel_Tool::AddPrimitive( PHXMESHPRIMITIVE p )
{
  InvalidateUptoDateFlag();
  CphxModelObject_Tool *b;

  if ( p != Mesh_Clone )
  {
    CphxModelObject_Tool_Mesh *me = new CphxModelObject_Tool_Mesh();
    me->ParentModel = this;
    me->GetModelObject()->Mesh.SmoothGroupSeparation = DEFAULTSMOOTHGROUPSEPARATION;
    b = me;
  }
  else
  {
    CphxModelObject_Tool_Clone *mc = new CphxModelObject_Tool_Clone();
    mc->ParentModel = this;
    b = mc;
    for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    {
      if ( Objects[ x ]->Selected )
        mc->AddClonedObject( Objects[ x ] );
    }
  }

  D3DXMATRIX m;
  D3DXMatrixIdentity( &m );
  b->SetMatrix( m );

  b->Primitive = p;
  Objects.Add( b );
  AddParent( b );

  Model.Objects.Add( b->Object );

  switch ( p )
  {
  case Mesh_Cube:
    b->SetName( _T( "Cube" ) );
    break;
  case Mesh_GeoSphere:
    b->SetName( _T( "GeoSphere" ) );
    b->Parameters[ 0 ] = 2;
    break;
  case Mesh_Plane:
    b->SetName( _T( "Plane" ) );
    b->Parameters[ 0 ] = 12;
    b->Parameters[ 1 ] = 12;
    break;
  case Mesh_Sphere:
    b->SetName( _T( "Sphere" ) );
    b->Parameters[ 0 ] = 12;
    b->Parameters[ 1 ] = 12;
    b->Parameters[ 2 ] = 255;
    b->Parameters[ 3 ] = 0;
    b->Parameters[ 4 ] = 1;
    break;
  case Mesh_Cylinder:
    b->SetName( _T( "Cylinder" ) );
    b->Parameters[ 0 ] = 12;
    b->Parameters[ 1 ] = 1;
    b->Parameters[ 2 ] = 1;
    break;
  case Mesh_Cone:
    b->SetName( _T( "Cone" ) );
    b->Parameters[ 0 ] = 12;
    b->Parameters[ 1 ] = 1;
    b->Parameters[ 2 ] = 255;
    b->Parameters[ 3 ] = 1;
    break;
  case Mesh_Arc:
    b->SetName( _T( "Arc" ) );
    b->Parameters[ 0 ] = 12;
    b->Parameters[ 1 ] = 255;
    b->Parameters[ 2 ] = 0;
    break;
  case Mesh_Line:
    b->SetName( _T( "Line" ) );
    b->Parameters[ 0 ] = 2;
    break;
  case Mesh_Spline:
    b->SetName( _T( "Spline" ) );
    b->Parameters[ 0 ] = 5;
    b->Parameters[ 1 ] = 0;
    {
      //flareMeshSplineKey k;
      //k.Front[0] = k.Front[1] = k.Back[0] = k.Back[1] = k.vx[0] = k.vx[1] = 0;
      //k.vx[2] = -0.5;
      //k.Back[1] = -0.25f;
      //k.Front[1] = 0.25f;
      //b->Spline.AddItem(k);
      //k.Front[0] = k.Front[1] = k.Back[0] = k.Back[1] = k.vx[0] = k.vx[1] = 0;
      //k.vx[2] = 0.5;
      //k.Back[1] = -0.25f;
      //k.Front[1] = 0.25f;
      //b->Spline.AddItem(k);
    }
    break;
  case Mesh_Loft:
    b->SetName( _T( "Loft" ) );
    b->Parameters[ 0 ] = 255;
    b->Parameters[ 1 ] = 255;
    b->Parameters[ 2 ] = 1;
    b->Parameters[ 3 ] = 1;
    b->Parameters[ 4 ] = 0;
    b->Parameters[ 5 ] = 255;
    b->Parameters[ 6 ] = 255;
    break;
  case Mesh_Clone:
    b->SetName( _T( "Clone" ) );
    break;
  case Mesh_Copy:
    b->SetName( _T( "Copy" ) );
    b->Parameters[ 0 ] = 255;
    break;
  case Mesh_Merge:
    b->SetName( _T( "Merge" ) );
    break;
  case Mesh_Scatter:
    b->SetName( _T( "Scatter" ) );
    b->Parameters[ 0 ] = 255;
    b->Parameters[ 1 ] = 255;
    b->Parameters[ 2 ] = 0; //randseed
    b->Parameters[ 3 ] = 255; //vx prob
    b->Parameters[ 4 ] = 0; //edge prob
    b->Parameters[ 5 ] = 0; //poly prob
    b->Parameters[ 6 ] = 0; //poly count
    b->Parameters[ 7 ] = 0; //prob tint
    b->Parameters[ 8 ] = 1; //orientation
    b->Parameters[ 9 ] = 0; //scalethreshold
    b->Parameters[ 10 ] = 0; //only y scale
    b->Parameters[ 11 ] = 0; //scaletint
    b->FloatParameter = 0; //transform threshold
    break;
  case Mesh_Stored:
    b->SetName( _T( "Stored" ) );
    break;
  case Mesh_Tree:
    b->SetName( _T( "Tree" ) );
    b->Parameters[ 0 ] = 255; //species
    b->Parameters[ 1 ] = 0; //randseed
    b->Parameters[ 2 ] = 255; //density
    b->Parameters[ 3 ] = 255; //density
    b->Parameters[ 4 ] = 255; //density
    b->Parameters[ 5 ] = 255; //density
    break;
  case Mesh_TreeLeaves:
    b->SetName( _T( "TreeLeaves" ) );
    b->Parameters[ 0 ] = 255; //species
    b->Parameters[ 1 ] = 0; //randseed
    b->Parameters[ 2 ] = 255; //density
    b->Parameters[ 3 ] = 255; //density
    b->Parameters[ 4 ] = 255; //density
    b->Parameters[ 5 ] = 255; //density
    b->Parameters[ 6 ] = 255; //density
    break;
  case Mesh_Text:
    b->SetName( _T( "Text" ) );
    b->Parameters[ 0 ] = 0; // font
    b->Parameters[ 1 ] = 32; // deviation, lowest possible -> high resolution text
    b->Text = _T( "Conspiracy" );
    break;
  }

  return b;

}

TS32 CphxModel_Tool::GetObjectIndex( CphxModelObject_Tool *o )
{
  return Objects.Find( o );
}

TS32 CphxModel_Tool::GetObjectIndex( CphxGUID &g )
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    if ( Objects[ x ]->GetGUID() == g ) return x;
  return -1;
}

TBOOL CphxModel_Tool::DeleteSelected()
{
  int deletedcount = 0;
  do
  {
    deletedcount = 0;
    for ( int x = 0; x < Objects.NumItems(); x++ )
      if ( Objects[ x ]->Selected )
      {
        CphxResource *r = Objects[ x ];

        if ( !r->HasChildrenOfType( PHX_MODELOBJECT ) )
        {
          Objects.FreeByIndex( x );

#ifdef MEMORY_TRACKING
          memTracker.SetMissingIgnore( true );
#endif
          Model.Objects.DeleteByIndex( x );
#ifdef MEMORY_TRACKING
          memTracker.SetMissingIgnore( false );
#endif

          x--;
          deletedcount++;
        }
      }

  } while ( deletedcount != 0 );

  //delete filters
  for ( int x = 0; x < Objects.NumItems(); x++ )
  {
    for ( int y = 0; y < Objects[ x ]->Filters.NumItems(); y++ )
    {
      if ( Objects[ x ]->Filters[ y ]->Selected )
      {
        Objects[ x ]->Filters.FreeByIndex( y );
        Objects[ x ]->InvalidateUptoDateFlag();
        y--;
      }
    }
  }

  int selectedcount = 0;
  for ( int x = 0; x < Objects.NumItems(); x++ )
  {
    Objects[ x ]->UpdateDependencies();
    if ( Objects[ x ]->Selected ) selectedcount++;
  }

  if ( selectedcount > 0 )
  {
    SetStatusbarText( "Not all objects could be deleted due to dependencies" );
    LOG_ERR( "Not all objects could be deleted due to dependencies" );
    return false;
  }

  return true;
}

void CphxModel_Tool::UpdateTextures()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->UpdateMaterialTextures();
}

void CphxModel_Tool::UpdateMaterialStates()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->UpdateMaterialState();
}

void CphxModel_Tool::CopySelected()
{
  int count = Objects.NumItems();
  for ( int x = 0; x < count; x++ )
    if ( Objects[ x ]->Selected )
    {
      Objects[ x ]->Selected = false;
      CphxModelObject_Tool_Mesh *o = (CphxModelObject_Tool_Mesh *)AddPrimitive( Objects[ x ]->Primitive );
      o->SetName( CString( "*" + Objects[ x ]->GetName() ) );
      o->SetMatrix( Objects[ x ]->GetMatrix() );
      o->FloatParameter = Objects[ x ]->FloatParameter;
      o->miniModelTriangles = Objects[ x ]->miniModelTriangles;
      o->miniModelVertices = Objects[ x ]->miniModelVertices;
      for ( int y = 0; y < 12; y++ ) o->Parameters[ y ] = Objects[ x ]->Parameters[ y ];
      o->Selected = true;

      if ( o->GetPrimitive() != Mesh_Clone )
      {
        for ( int y = 0; y < 2; y++ )
          o->ParentGUIDS[ y ] = Objects[ x ]->ParentGUIDS[ y ];
        for ( int y = 0; y < Objects[ x ]->Filters.NumItems(); y++ )
        {
          CphxMeshFilter_Tool *f = o->AddFilter( Objects[ x ]->Filters[ y ]->Filter );
          f->Parameters[ 0 ] = Objects[ x ]->Filters[ y ]->Parameters[ 0 ];
          f->Parameters[ 1 ] = Objects[ x ]->Filters[ y ]->Parameters[ 1 ];
          for ( int z = 0; z < 12; z++ )
          {
            f->srt[ z ] = Objects[ x ]->Filters[ y ]->srt[ z ];
            f->_srt[ z ] = Objects[ x ]->Filters[ y ]->_srt[ z ];
          }
          f->Enabled = Objects[ x ]->Filters[ y ]->Enabled;
          f->Texture = Objects[ x ]->Filters[ y ]->Texture;
          if ( f->Filter == ModelFilter_TintMesh && f->Texture ) o->AddParent( f->Texture );
        }

        o->Text = Objects[ x ]->Text;
        o->SetMaterial( Objects[ x ]->GetMaterial() );

        o->MaterialData.Copy( &( (CphxModelObject_Tool_Mesh *)Objects[ x ] )->MaterialData );
      }
      else
      {
        CphxModelObject_Tool_Clone *c = (CphxModelObject_Tool_Clone*)o;
        CphxModelObject_Tool_Clone *v = (CphxModelObject_Tool_Clone*)Objects[ x ];
        for ( int y = 0; y < v->ClonedObjects.NumItems(); y++ )
          c->AddClonedObject( v->ClonedObjects[ y ] );
      }

      o->UpdateDependencies();
    }
}

void CphxModel_Tool::SetModelerMode()
{
  for ( TS32 x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->Object->ToolObject = Objects[ x ];
}

CphxModelObject_Tool::CphxModelObject_Tool()
{
  Object = NULL;
  ParentModel = NULL;
  Selected = false;
  ParentGUIDS[ 0 ].SetString( _T( "NONENONENONENONENONENONENONENONE" ) );
  ParentGUIDS[ 1 ].SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  int r, g, b;
  r = ( rand() % 256 );
  g = ( rand() % 255 ) + 1;
  b = ( rand() % 255 ) + 1;

  int mx = max( max( r, g ), b );

  WireframeColor.R() = ( r * 255 ) / mx;
  WireframeColor.G() = ( g * 255 ) / mx;
  WireframeColor.B() = ( b * 255 ) / mx;
  WireframeColor.A() = 255;

  //int mx = max(max(r, g), b);
  //TF32 len = sqrtf((TF32)(r*r + g*g + b*b));

  //WireframeColor.R() = (TS32)(r / len * 255);
  //WireframeColor.G() = (TS32)(g / len * 255);
  //WireframeColor.B() = (TS32)(b / len * 255);

}

CphxModelObject_Tool::~CphxModelObject_Tool()
{
  Filters.FreeArray();
  SAFEDELETE( Object );
}

D3DXMATRIX CphxModelObject_Tool::GetMatrix()
{
  return Object->GetMatrix();
}

void CphxModelObject_Tool::SetName( CString &name )
{
  Name = name;
}

void CphxModelObject_Tool::SetName( TCHAR *name )
{
  SetName( CString( name ) );
}

CString CphxModelObject_Tool::GetName()
{
  return Name;
}

void CphxModelObject_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );

  for ( TS32 x = 0; x < 12; x++ )
  {
    CXMLNode t = Node->AddChild( _T( "transformation" ) );

    TS32 idxx = x % 4;
    TS32 idxy = x / 4;

    t.SetAttributeFromInteger( _T( "index" ), x );
    TS32 v = *( (TU16*)&Object->TransformationF16[ idxy + idxx * 3 ] );
    t.SetAttributeFromInteger( _T( "value" ), v );
  }

  for ( TS32 x = 0; x < 12; x++ )
  {
    CXMLNode t = Node->AddChild( _T( "parameter" ) );
    t.SetAttributeFromInteger( _T( "index" ), x );
    t.SetAttributeFromInteger( _T( "value" ), Parameters[ x ] );
  }

  CXMLNode t = Node->AddChild( _T( "floatparameter" ) );
  TS32 v = *( (TU16*)&FloatParameter );
  t.SetAttributeFromInteger( _T( "value" ), v );

  for ( TS32 x = 0; x < 2; x++ )
  {
    CXMLNode tx = Node->AddChild( _T( "parentguid" ) );
    tx.SetAttributeFromInteger( _T( "index" ), x );
    tx.SetAttribute( _T( "value" ), ParentGUIDS[ x ].GetString() );
  }

  for ( TS32 x = 0; x < Filters.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Filter" ) );
    n.SetAttributeFromInteger( _T( "Type" ), Filters[ x ]->Filter );
    Filters[ x ]->ExportData( &n );
  }

  if ( Text.Length() )
  {
    //Node->AddChild(_T("Text")).SetText(Text.GetPointer());
    CString str = CString::EncodeToBase64( (TU8*)Text.GetPointer(), Text.Length() );
    Node->AddChild( _T( "textblob" ) ).SetText( str.GetPointer() );
  }

  if ( miniModelTriangles.NumItems() && miniModelVertices.NumItems() )
  {
    auto vnode = Node->AddChild( _T( "MiniVerts" ) );
    vnode.SetAttributeFromInteger( "count", miniModelVertices.NumItems() );
    vnode.SetAttribute( "data", CString::EncodeToBase64( miniModelVertices.GetPointer( 0 ), miniModelVertices.NumItems() ).GetPointer() );
    auto tnode = Node->AddChild( _T( "MiniTris" ) );
    tnode.SetAttributeFromInteger( "count", miniModelTriangles.NumItems() );
    tnode.SetAttribute( "data", CString::EncodeToBase64( miniModelTriangles.GetPointer( 0 ), miniModelTriangles.NumItems() ).GetPointer() );
  }

  for ( TS32 x = 0; x < ClonedGuids.NumItems(); x++ )
    Node->AddChild( _T( "clonedobject" ) ).SetText( ClonedGuids[ x ].GetString() );
}

void CphxModelObject_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "transformation" ) ); x++ )
  {
    CXMLNode t = Node->GetChild( _T( "transformation" ), x );
    if ( !t.HasAttribute( _T( "index" ) ) ) continue;
    if ( !t.HasAttribute( _T( "value" ) ) ) continue;
    TS32 idx = 0, v = 0;
    t.GetAttributeAsInteger( _T( "index" ), &idx );
    t.GetAttributeAsInteger( _T( "value" ), &v );
    TU16 v2 = v;

    TS32 idxx = idx % 4;
    TS32 idxy = idx / 4;

    Object->TransformationF16[ idxy + idxx * 3 ] = *( (D3DXFLOAT16*)&v2 );
  }

  //D3DXMATRIX m;
  //D3DXMatrixIdentity(&m);
  //for (int y = 0; y < 3; y++)
  //	for (int x = 0; x < 4; x++)
  //		m.m[x][y] = Object->TransformationF16[x + (y << 2)];
  //Object->Matrix = m;


  for ( TS32 x = 0; x < Node->GetChildCount( _T( "parameter" ) ); x++ )
  {
    CXMLNode t = Node->GetChild( _T( "parameter" ), x );
    if ( !t.HasAttribute( _T( "index" ) ) ) continue;
    if ( !t.HasAttribute( _T( "value" ) ) ) continue;
    TS32 idx = 0, v = 0;
    t.GetAttributeAsInteger( _T( "index" ), &idx );
    t.GetAttributeAsInteger( _T( "value" ), &v );
    Parameters[ idx ] = v;
  }

  if ( Node->GetChildCount( _T( "floatparameter" ) ) )
  {
    CXMLNode t = Node->GetChild( _T( "floatparameter" ) );
    if ( t.HasAttribute( _T( "value" ) ) )
    {
      TS32 v = 0;
      t.GetAttributeAsInteger( _T( "value" ), &v );
      TU16 v2 = v;
      FloatParameter = *( (D3DXFLOAT16*)&v2 );
    }
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "parentguid" ) ); x++ )
  {
    CXMLNode t = Node->GetChild( _T( "parentguid" ), x );
    if ( !t.HasAttribute( _T( "index" ) ) ) continue;
    if ( !t.HasAttribute( _T( "value" ) ) ) continue;
    TS32 idx = 0;
    t.GetAttributeAsInteger( _T( "index" ), &idx );
    ParentGUIDS[ idx ].SetString( t.GetAttributeAsString( _T( "value" ) ).GetPointer() );
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Filter" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Filter" ), x );
    PHXMESHFILTER type = ModelFilter_NONE;
    if ( !n.HasAttribute( _T( "Type" ) ) ) continue;
    n.GetAttributeAsInteger( _T( "Type" ), (TS32*)&type );
    CphxMeshFilter_Tool *t = AddFilter( type );
    if ( !t ) continue;
    t->ImportData( &n );
  }

  if ( Node->GetChildCount( _T( "Text" ) ) )
    Text = Node->GetChild( _T( "Text" ) ).GetText();

  if ( Node->GetChildCount( _T( "textblob" ) ) )
  {
    TU8 *textdata = NULL;
    TS32 textdatasize;
    CString txt = Node->GetChild( _T( "textblob" ) ).GetText();
    txt.DecodeBase64( textdata, textdatasize );

    Text = CString( (TS8*)textdata, textdatasize );
    SAFEDELETEA( textdata );
  }
  
/*
  auto vnode = Node->AddChild( _T( "MiniVerts" ) );
  vnode.SetAttributeFromInteger( "count", miniModelVertices.NumItems() );
  vnode.SetAttributeFromInteger( "data", CString::EncodeToBase64( miniModelVertices.GetPointer( 0 ), miniModelVertices.NumItems() * 3 ).GetPointer() );
  auto vnode = Node->AddChild( _T( "MiniTris" ) );
  vnode.SetAttributeFromInteger( "count", miniModelTriangles.NumItems() );
  vnode.SetAttributeFromInteger( "data", CString::EncodeToBase64( miniModelTriangles.GetPointer( 0 ), miniModelTriangles.NumItems() * 3 ).GetPointer() );
*/

  if ( Node->GetChildCount( _T( "MiniVerts" ) ) && Node->GetChildCount( _T( "MiniTris" ) ) )
  {
    auto vnode = Node->GetChild( _T( "MiniVerts" ) );
    int vcount = 0;
    auto tnode = Node->GetChild( _T( "MiniTris" ) );
    int tcount = 0;

    vnode.GetAttributeAsInteger( "count", &vcount );
    tnode.GetAttributeAsInteger( "count", &tcount );
    if ( vcount && tcount )
    {
      CString vData = vnode.GetAttributeAsString( "data" );
      CString tData = tnode.GetAttributeAsString( "data" );
      
      TU8* vertices = nullptr;
      TS32 vxDataLen = 0;
      vData.DecodeBase64( vertices, vxDataLen );

      TU8* triangles = nullptr;
      TS32 triDataLen = 0;
      tData.DecodeBase64( triangles, triDataLen );

      miniModelVertices.FlushFast();
      for ( int x = 0; x < vxDataLen; x++ )
        miniModelVertices.Add( vertices[ x ] );

      miniModelTriangles.FlushFast();
      for ( int x = 0; x < triDataLen; x++ )
        miniModelTriangles.Add( triangles[ x ] );
    }
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "clonedobject" ) ); x++ )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "clonedobject" ), x ).GetText().GetPointer() );
    ClonedGuids += g;
  }
}

void CphxModelObject_Tool::SetMatrix( D3DXMATRIX m )
{
  D3DXFLOAT16 trans[ 16 ];
  memcpy( trans, Object->TransformationF16, 12 * 2 );
  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      Object->TransformationF16[ y + x * 3 ] = m.m[ x ][ y ];

  if ( memcmp( trans, Object->TransformationF16, 12 * 2 ) != 0 )
    for ( TS32 x = 0; x < Filters.NumItems(); x++ )
      if ( Filters[ x ]->Filter == ModelFilter_CSG )
      {
        InvalidateUptoDateFlag();
        break;
      }

  //D3DXMATRIX mm;
  //D3DXMatrixIdentity(&mm);
  //for (int y = 0; y < 3; y++)
  //	for (int x = 0; x < 4; x++)
  //		mm.m[x][y] = Object->TransformationF16[x + (y << 2)];

  //Object->Matrix = mm;
}

TBOOL CphxModelObject_Tool::FindInCloneTree( CphxModelObject_Tool* m )
{
  if ( m == this ) return true;
  TBOOL Found = false;
  for ( TS32 x = 0; x < ClonedObjects.NumItems(); x++ )
    Found |= ClonedObjects[ x ]->FindInCloneTree( m );
  return Found;
}

void CphxModelObject_Tool::DeleteFilter( CphxMeshFilter_Tool *t )
{
  if ( Filters.Find( t ) < 0 ) return;
  Filters.Free( t );
  InvalidateUptoDateFlag();
}

void CphxModelObject_Tool::MoveFilterUp( CphxMeshFilter_Tool *t )
{
  TS32 x = Filters.Find( t );
  if ( x <= 0 ) return;
  CphxMeshFilter_Tool *e = Filters[ x - 1 ];
  Filters[ x - 1 ] = t;
  Filters[ x ] = e;
  InvalidateUptoDateFlag();
}

void CphxModelObject_Tool::MoveFilterDown( CphxMeshFilter_Tool *t )
{
  TS32 x = Filters.Find( t );
  if ( x < 0 ) return;
  if ( x >= Filters.NumItems() - 1 ) return;

  CphxMeshFilter_Tool *e = Filters[ x + 1 ];
  Filters[ x + 1 ] = t;
  Filters[ x ] = e;
  InvalidateUptoDateFlag();
}

D3DXMATRIX CphxModelObject_Tool::GetBoundingBoxMatrix()
{
  D3DXMATRIX m;
  D3DXMatrixIdentity( &m );
  return m;
}

CphxMeshFilter_Tool *CphxModelObject_Tool_Mesh::AddFilter( PHXMESHFILTER fi )
{
  CphxMeshFilter_Tool *f = new CphxMeshFilter_Tool;
  f->Filter = fi;
  f->ParentObject = this;
  f->Parameters[ 0 ] = f->Parameters[ 1 ] = 0;

  switch ( fi )
  {
  case ModelFilter_SmoothGroup:
    f->Parameters[ 0 ] = 255;
    break;
  case ModelFilter_MeshSmooth:
    f->Parameters[ 1 ] = 1;
    break;
  case ModelFilter_Replicate:
    f->Parameters[ 1 ] = 1;
    {
      D3DXMATRIX m;
      D3DXMatrixIdentity( &m );
      f->SetRawMatrix( m );
    }
    break;
  case ModelFilter_NormalDeform:
    f->Parameters[ 0 ] = 12;
    f->Parameters[ 1 ] = 128;
    break;
  case ModelFilter_TintMeshShape:
    f->Parameters[ 0 ] = 0;
    f->Parameters[ 1 ] = 0;
    f->Parameters[ 2 ] = 16;
    break;
  case ModelFilter_Greeble:
    f->Parameters[ 0 ] = 0;
    f->Parameters[ 1 ] = 25;
    f->Parameters[ 2 ] = 200;
    break;
  }

  Filters += f;
  InvalidateUptoDateFlag();
  return f;
}

CphxModelObject_Tool_Mesh::CphxModelObject_Tool_Mesh() : CphxModelObject_Tool()
{
  Object = new CphxModelObject_Mesh();
  Object->ToolObject = this;

  GetModelObject()->Material = &DefaultMaterial->Material; //default display material should be here

  GetModelObject()->VxBuffer = NULL;
  GetModelObject()->IndexBuffer = NULL;
  GetModelObject()->WireBuffer = NULL;
  GetModelObject()->TriCount = 0;
  GetModelObject()->EdgeCount = 0;
  GetModelObject()->VxCount = 0;
  Material = DefaultMaterial;
  MaterialStateSize = 0;
  GetModelObject()->MaterialState = NULL;
  GetModelObject()->StoredPolyCount = 0;
  GetModelObject()->StoredVertexCount = 0;
  GetModelObject()->StoredPolygons = NULL;
  GetModelObject()->StoredVertices = NULL;

  UpdateMaterialState();
}

CphxModelObject_Tool_Mesh::~CphxModelObject_Tool_Mesh()
{
  FreeMaterialState();

  SAFEDELETEA( GetModelObject()->StoredPolygons );
  SAFEDELETEA( GetModelObject()->StoredVertices );

  if ( GetModelObject()->VxBuffer ) GetModelObject()->VxBuffer->Release();
  if ( GetModelObject()->IndexBuffer ) GetModelObject()->IndexBuffer->Release();
  if ( GetModelObject()->WireBuffer ) GetModelObject()->WireBuffer->Release();
  GetModelObject()->VxBuffer = NULL;
  GetModelObject()->IndexBuffer = NULL;
  GetModelObject()->WireBuffer = NULL;
}

CphxModelObject_Mesh * CphxModelObject_Tool_Mesh::GetModelObject()
{
  return (CphxModelObject_Mesh*)Object;
}

struct StoredPolyVertex
{
  int index;
  float normal[ 3 ];
  float uv[ 2 ];
};

void CphxModelObject_Tool_Mesh::ExportData( CXMLNode *Node )
{
  CphxModelObject_Tool::ExportData( Node );
  if ( Material )
  {
    CXMLNode m = Node->AddChild( _T( "Material" ), false );
    m.SetAttribute( _T( "GUID" ), Material->GetGUID().GetString() );
    MaterialData.ExportData( Node );
  }

  if ( GetModelObject()->StoredVertexCount )
  {
    CXMLNode n = Node->AddChild( _T( "Vertices" ), false );
    float *vxData = new float[ GetModelObject()->StoredVertexCount * 6 ];
    for ( TS32 x = 0; x < GetModelObject()->StoredVertexCount; x++ )
    {
      vxData[ x * 6 + 0 ] = GetModelObject()->StoredVertices[ x ].Position.x;
      vxData[ x * 6 + 1 ] = GetModelObject()->StoredVertices[ x ].Position.y;
      vxData[ x * 6 + 2 ] = GetModelObject()->StoredVertices[ x ].Position.z;
      vxData[ x * 6 + 3 ] = GetModelObject()->StoredVertices[ x ].Normal.x;
      vxData[ x * 6 + 4 ] = GetModelObject()->StoredVertices[ x ].Normal.y;
      vxData[ x * 6 + 5 ] = GetModelObject()->StoredVertices[ x ].Normal.z;
    }

    n.SetAttribute( _T( "VertexData" ), CString::EncodeToBase64( (TU8*)vxData, GetModelObject()->StoredVertexCount * 6 * sizeof( float ) ).GetPointer() );
    delete[] vxData;
  }

  //for ( TS32 x = 0; x < GetModelObject()->StoredVertexCount; x++ )
  //{
  //  CXMLNode n = Node->AddChild( _T( "Vertex" ), false );
  //  n.SetAttribute( _T( "Position" ), CString::EncodeToBase64( (TU8*)&GetModelObject()->StoredVertices[ x ].Position, sizeof( D3DXVECTOR3 ) ).GetPointer() );
  //  n.SetAttribute( _T( "Normal" ), CString::EncodeToBase64( (TU8*)&GetModelObject()->StoredVertices[ x ].Normal, sizeof( D3DXVECTOR3 ) ).GetPointer() );
  //}

  if ( GetModelObject()->StoredPolyCount )
  {
    CXMLNode n = Node->AddChild( _T( "Polygons" ), false );
    n.SetAttributeFromInteger( "PolyCount", GetModelObject()->StoredPolyCount );
    for ( int vc = 1; vc <= 4; vc++ )
    {
      int cnt = 0;
      for ( TS32 x = 0; x < GetModelObject()->StoredPolyCount; x++ )
        if ( GetModelObject()->StoredPolygons[ x ].VertexCount == vc )
          cnt++;

      if ( cnt )
      {
        CString name;
        switch ( vc )
        {
        case 1: name = _T( "Poly1s" );
          break;
        case 2: name = _T( "Poly2s" );
          break;
        case 3: name = _T( "Poly3s" );
          break;
        case 4: name = _T( "Poly4s" );
          break;
        }

        StoredPolyVertex *polyData = new StoredPolyVertex[ vc*cnt ];

        cnt = 0;

        for ( TS32 x = 0; x < GetModelObject()->StoredPolyCount; x++ )
          if ( GetModelObject()->StoredPolygons[ x ].VertexCount == vc )
          {
            for ( int y = 0; y < vc; y++ )
            {
              polyData[ cnt ].index = GetModelObject()->StoredPolygons[ x ].VertexIDs[ y ];
              polyData[ cnt ].normal[ 0 ] = GetModelObject()->StoredPolygons[ x ].Normals[ y ].x;
              polyData[ cnt ].normal[ 1 ] = GetModelObject()->StoredPolygons[ x ].Normals[ y ].y;
              polyData[ cnt ].normal[ 2 ] = GetModelObject()->StoredPolygons[ x ].Normals[ y ].z;
              polyData[ cnt ].uv[ 0 ] = GetModelObject()->StoredPolygons[ x ].Texcoords[ y ][ 0 ].x;
              polyData[ cnt ].uv[ 1 ] = GetModelObject()->StoredPolygons[ x ].Texcoords[ y ][ 0 ].y;
              cnt++;
            }
          }

        CXMLNode nn = n.AddChild( name.GetPointer(), false );
        nn.SetAttribute( _T( "PolyData" ), CString::EncodeToBase64( (TU8*)polyData, cnt * sizeof( StoredPolyVertex ) ).GetPointer() );

        delete[] polyData;
      }
    }
  }

  //for ( TS32 x = 0; x < GetModelObject()->StoredPolyCount; x++ )
  //{
  //  CXMLNode n = Node->AddChild( _T( "Poly" ), false );
  //  if ( GetModelObject()->StoredPolygons[ x ].VertexCount >= 1 )
  //    n.SetAttributeFromInteger( _T( "v1" ), GetModelObject()->StoredPolygons[ x ].VertexIDs[ 0 ] );
  //  if ( GetModelObject()->StoredPolygons[ x ].VertexCount >= 2 )
  //    n.SetAttributeFromInteger( _T( "v2" ), GetModelObject()->StoredPolygons[ x ].VertexIDs[ 1 ] );
  //  if ( GetModelObject()->StoredPolygons[ x ].VertexCount >= 3 )
  //    n.SetAttributeFromInteger( _T( "v3" ), GetModelObject()->StoredPolygons[ x ].VertexIDs[ 2 ] );
  //  if ( GetModelObject()->StoredPolygons[ x ].VertexCount >= 4 )
  //    n.SetAttributeFromInteger( _T( "v4" ), GetModelObject()->StoredPolygons[ x ].VertexIDs[ 3 ] );
  //}
}

void CphxModelObject_Tool_Mesh::ImportData( CXMLNode *Node )
{
  CphxModelObject_Tool::ImportData( Node );
  if ( Node->GetChildCount( _T( "Material" ) ) )
  {
    CXMLNode m = Node->GetChild( _T( "Material" ) );
    if ( m.HasAttribute( _T( "GUID" ) ) )
    {
      CphxGUID g;
      g.SetString( m.GetAttributeAsString( _T( "GUID" ) ).GetPointer() );
      CphxMaterial_Tool *mat = Project.GetMaterial( g );
      SetMaterial( mat );
    }
    MaterialData.ImportData( Node );
    UpdateMaterialState();
  }

  TS32 vxc = Node->GetChildCount( _T( "Vertex" ) );
  if ( vxc )
  {
    GetModelObject()->StoredVertexCount = vxc;
    GetModelObject()->StoredVertices = new CphxVertex[ vxc ];

    for ( TS32 x = 0; x < vxc; x++ )
    {
      CXMLNode &n = Node->GetChild( _T( "Vertex" ), x );
      if ( n.HasAttribute( _T( "Position" ) ) && n.HasAttribute( _T( "Normal" ) ) )
      {
        CString s = n.GetAttribute( _T( "Position" ) );
        CString sn = n.GetAttribute( _T( "Normal" ) );
        D3DXVECTOR3 v, n;
        TU8 *dta = nullptr;
        TS32 siz = 0;
        TU8 *dtan = nullptr;
        TS32 sizn = 0;
        s.DecodeBase64( dta, siz );
        sn.DecodeBase64( dtan, sizn );

        if ( siz == sizeof( D3DXVECTOR3 ) && sizn == sizeof( D3DXVECTOR3 ) )
        {
          memcpy( &v, dta, sizeof( D3DXVECTOR3 ) );
          memcpy( &n, dtan, sizeof( D3DXVECTOR3 ) );
          GetModelObject()->StoredVertices[ x ].Position = v;
          GetModelObject()->StoredVertices[ x ].Normal = n;
        }

        SAFEDELETEA( dta );
        SAFEDELETEA( dtan );
      }
    }
  }

  if ( Node->GetChildCount( _T( "Vertices" ) ) )
  {
    CXMLNode &n = Node->GetChild( _T( "Vertices" ) );
    CString s = n.GetAttribute( _T( "VertexData" ) );

    TU8 *dta = nullptr;
    TS32 siz = 0;
    s.DecodeBase64( dta, siz );

    if ( dta && !( siz % ( 6 * sizeof( float ) ) ) )
    {
      int vxcount = siz / ( 6 * sizeof( float ) );
      GetModelObject()->StoredVertexCount = 0;
      GetModelObject()->StoredVertices = new CphxVertex[ vxcount ];
      for ( int x = 0; x < vxcount; x++ )
      {
        GetModelObject()->StoredVertices[ x ].Position.x = ( (float*)dta )[ x * 6 + 0 ];
        GetModelObject()->StoredVertices[ x ].Position.y = ( (float*)dta )[ x * 6 + 1 ];
        GetModelObject()->StoredVertices[ x ].Position.z = ( (float*)dta )[ x * 6 + 2 ];
        GetModelObject()->StoredVertices[ x ].Normal.x = ( (float*)dta )[ x * 6 + 3 ];
        GetModelObject()->StoredVertices[ x ].Normal.y = ( (float*)dta )[ x * 6 + 4 ];
        GetModelObject()->StoredVertices[ x ].Normal.z = ( (float*)dta )[ x * 6 + 5 ];
        GetModelObject()->StoredVertexCount++;
      }
    }

    SAFEDELETEA( dta );
  }

  TS32 plc = Node->GetChildCount( _T( "Poly" ) );
  if ( plc )
  {
    GetModelObject()->StoredPolyCount = plc;
    GetModelObject()->StoredPolygons = new CphxPolygon[ plc ];
    for ( TS32 x = 0; x < plc; x++ )
    {
      CXMLNode &n = Node->GetChild( _T( "Poly" ), x );
      TS32 vcx = 0;
      if ( n.HasAttribute( _T( "v1" ) ) )
      {
        vxc = 1;
        if ( n.HasAttribute( _T( "v2" ) ) )
        {
          vxc = 2;
          if ( n.HasAttribute( _T( "v3" ) ) )
          {
            vxc = 3;
            if ( n.HasAttribute( _T( "v4" ) ) )
            {
              vxc = 4;
            }
          }
        }
      }

      GetModelObject()->StoredPolygons[ x ].VertexCount = vxc;
      if ( vxc >= 1 )
        n.GetAttributeAsInteger( _T( "v1" ), &GetModelObject()->StoredPolygons[ x ].VertexIDs[ 0 ] );
      if ( vxc >= 2 )
        n.GetAttributeAsInteger( _T( "v2" ), &GetModelObject()->StoredPolygons[ x ].VertexIDs[ 1 ] );
      if ( vxc >= 3 )
        n.GetAttributeAsInteger( _T( "v3" ), &GetModelObject()->StoredPolygons[ x ].VertexIDs[ 2 ] );
      if ( vxc >= 4 )
        n.GetAttributeAsInteger( _T( "v4" ), &GetModelObject()->StoredPolygons[ x ].VertexIDs[ 3 ] );
    }
  }

  if ( Node->GetChildCount( _T( "Polygons" ) ) )
  {
    CXMLNode &n = Node->GetChild( _T( "Polygons" ) );
    int pcnt;
    n.GetAttributeAsInteger( "PolyCount", &pcnt );

    GetModelObject()->StoredPolyCount = 0;
    GetModelObject()->StoredPolygons = new CphxPolygon[ pcnt ];

    int cnt = 0;

    for ( int vc = 1; vc <= 4; vc++ )
    {
      CString name;
      switch ( vc )
      {
      case 1: name = _T( "Poly1s" );
        break;
      case 2: name = _T( "Poly2s" );
        break;
      case 3: name = _T( "Poly3s" );
        break;
      case 4: name = _T( "Poly4s" );
        break;
      }

      if ( n.GetChildCount( name.GetPointer() ) )
      {
        CXMLNode &nn = n.GetChild( name.GetPointer() );

        CString s = nn.GetAttribute( _T( "PolyData" ) );

        TU8 *dta = nullptr;
        TS32 siz = 0;
        s.DecodeBase64( dta, siz );
        StoredPolyVertex *pd = (StoredPolyVertex*)dta;

        if ( dta && !( siz % ( sizeof( StoredPolyVertex )*vc ) ) )
        {
          for ( TU32 x = 0; x < siz / ( sizeof( StoredPolyVertex )*vc ); x++ )
          {
            CphxPolygon *p = GetModelObject()->StoredPolygons + cnt;
            p->VertexCount = vc;

            for ( int y = 0; y < vc; y++ )
            {
              p->VertexIDs[ y ] = pd[ x*vc + y ].index;
              p->Normals[ y ].x = pd[ x*vc + y ].normal[ 0 ];
              p->Normals[ y ].y = pd[ x*vc + y ].normal[ 1 ];
              p->Normals[ y ].z = pd[ x*vc + y ].normal[ 2 ];
              p->Texcoords[ y ][ 0 ].x = pd[ x*vc + y ].uv[ 0 ];
              p->Texcoords[ y ][ 0 ].y = pd[ x*vc + y ].uv[ 1 ];
            }

            GetModelObject()->StoredPolyCount++;
            cnt++;
          }
        }

        SAFEDELETEA( dta );
      }

    }
  }
}

TBOOL CphxModelObject_Tool_Mesh::GenerateResource( CCoreDevice *Dev )
{
  CphxMesh &Mesh = GetModelObject()->Mesh;

  Mesh.Vertices.FlushFast();
  Mesh.Polygons.FlushFast();

  if ( GetModelObject()->VxBuffer ) GetModelObject()->VxBuffer->Release();
  if ( GetModelObject()->IndexBuffer ) GetModelObject()->IndexBuffer->Release();
  if ( GetModelObject()->WireBuffer ) GetModelObject()->WireBuffer->Release();
  if ( GetModelObject()->dataBuffer ) GetModelObject()->dataBuffer->Release();
  if ( GetModelObject()->dataBufferView ) GetModelObject()->dataBufferView->Release();

  GetModelObject()->VxBuffer = NULL;
  GetModelObject()->IndexBuffer = NULL;
  GetModelObject()->WireBuffer = NULL;
  GetModelObject()->dataBuffer = NULL;
  GetModelObject()->dataBufferView = NULL;
  Mesh.SmoothGroupSeparation = DEFAULTSMOOTHGROUPSEPARATION;
  Mesh.SkipNormalCalculation = false;

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif

  try
  {

    switch ( Primitive )
    {
    case Mesh_Cube:
      Mesh.CreateCube();
      break;
    case Mesh_GeoSphere:
      Mesh.CreateGeoSphere( Parameters[ 0 ] );
      break;
    case Mesh_Plane:
      Mesh.CreatePlane( Parameters[ 0 ], Parameters[ 1 ] );
      break;
    case Mesh_Sphere:
      Mesh.CreateSphere( Parameters[ 0 ], Parameters[ 1 ], ( Parameters[ 2 ] + 1 ) / 256.0f, Parameters[ 3 ] / 256.0f, Parameters[ 4 ] != 0 );
      break;
    case Mesh_Cylinder:
      Mesh.CreateCylinder( Parameters[ 0 ], Parameters[ 1 ], Parameters[ 2 ] != 0 );
      break;
    case Mesh_Cone:
      Mesh.CreateCone( Parameters[ 0 ], Parameters[ 1 ], ( Parameters[ 2 ] + 1 ) / 256.0f, Parameters[ 3 ] != 0 );
      break;
    case Mesh_Arc:
      Mesh.CreateArc( Parameters[ 0 ], ( Parameters[ 1 ] ) / 255.0f, Parameters[ 2 ] != 0 );
      //{
      //	int r = Parameters[0];
      //	if (Parameters[2] != 0) r++;

      //	for (int x = 0; x < r - 1; x++)
      //		Mesh.AddPolygon(x, x + 1, x);
      //}
      break;
    case Mesh_Line:
      Mesh.CreateLine( Parameters[ 0 ] );
      //{
      //	for (int x = 0; x < Parameters[0] - 1; x++)
      //		Mesh.AddPolygon(x, x + 1, x);
      //}
      break;
    case Mesh_Spline:
      //Mesh.Spline(Spline.Items, Spline.ItemCount, Parameters[0], Parameters[1] != 0);
      break;
    case Mesh_Loft:
      if ( ParentModel && Parameters[ 0 ] < ParentModel->GetObjectCount() && Parameters[ 1 ] < ParentModel->GetObjectCount() )
        Mesh.Loft( &( (CphxModelObject_Tool_Mesh*)ParentModel->GetObjectByIndex( Parameters[ 0 ] ) )->GetModelObject()->Mesh,
                   &ParentModel->GetObjectByIndex( Parameters[ 0 ] )->GetMatrix(),
                   &( (CphxModelObject_Tool_Mesh*)ParentModel->GetObjectByIndex( Parameters[ 1 ] ) )->GetModelObject()->Mesh,
                   &ParentModel->GetObjectByIndex( Parameters[ 1 ] )->GetMatrix(),
                   Parameters[ 2 ] != 0,
                   Parameters[ 3 ] != 0,
                   Parameters[ 4 ] ? ( Parameters[ 4 ] + 1 ) / 32.0f : 0,
                   Parameters[ 5 ] / 255.0f,
                   Parameters[ 6 ] / 255.0f );
      break;
    case Mesh_Clone:
      break;
    case Mesh_Copy:
      if ( ParentModel && Parameters[ 0 ] < ParentModel->GetObjectCount() )
        Mesh.Copy( &( (CphxModelObject_Tool_Mesh*)ParentModel->GetObjectByIndex( Parameters[ 0 ] ) )->GetModelObject()->Mesh );
      break;
    case Mesh_Merge:
      if ( ParentModel )
      {
        CArray<CphxModelObject_Mesh*> models;
        for ( int x = 0; x < ClonedObjects.NumItems(); x++ )
          models += ((CphxModelObject_Tool_Mesh*)ClonedObjects[ x ])->GetModelObject();

        Mesh.Merge( models.GetPointer( 0 ), models.NumItems() );
      }
      break;
    case Mesh_Scatter:
      if ( ParentModel && Parameters[ 0 ] < ParentModel->GetObjectCount() && Parameters[ 1 ] < ParentModel->GetObjectCount() )
      {
        Mesh.Scatter( &( (CphxModelObject_Tool_Mesh*)ParentModel->GetObjectByIndex( Parameters[ 0 ] ) )->GetModelObject()->Mesh,
                      &ParentModel->GetObjectByIndex( Parameters[ 0 ] )->GetMatrix(),
                      &( (CphxModelObject_Tool_Mesh*)ParentModel->GetObjectByIndex( Parameters[ 1 ] ) )->GetModelObject()->Mesh,
                      &ParentModel->GetObjectByIndex( Parameters[ 1 ] )->GetMatrix(),
                      Parameters[ 2 ],
                      Parameters[ 3 ] / 255.0f,
                      Parameters[ 4 ] / 255.0f,
                      Parameters[ 5 ] / 255.0f,
                      Parameters[ 6 ],
                      Parameters[ 7 ],
                      (PHXSCATTERORIENTATIONTYPE)Parameters[ 8 ],
                      Parameters[ 9 ] / 255.0f,
                      Parameters[ 10 ] != 0,
                      FloatParameter,
                      Parameters[ 11 ] );

      }
      break;
    case Mesh_Stored:
      if ( GetModelObject()->StoredVertices && GetModelObject()->StoredPolygons )
        Mesh.LoadStoredMesh( GetModelObject()->StoredVertices, GetModelObject()->StoredVertexCount, GetModelObject()->StoredPolygons, GetModelObject()->StoredPolyCount );
      break;
    case Mesh_StoredMini:
      Mesh.LoadStoredMiniMesh( miniModelVertices.GetPointer( 0 ), miniModelVertices.NumItems() / 3, miniModelTriangles.GetPointer( 0 ), miniModelTriangles.NumItems() / 3 );
      break;
    case Mesh_Tree:
    {
      if ( Project.GetTreeSpecies( ParentGUIDS[ 0 ] ) )
        Mesh.CreateTree( Parameters[ 1 ], Parameters + 2, Project.GetTreeSpecies( ParentGUIDS[ 0 ] )->Tree );
    }
    break;
    case Mesh_TreeLeaves:
    {
      if ( Project.GetTreeSpecies( ParentGUIDS[ 0 ] ) )
        Mesh.CreateTreeLeaves( Parameters[ 1 ], Parameters + 2, Project.GetTreeSpecies( ParentGUIDS[ 0 ] )->Tree );
    }
    break;
    case Mesh_Text:
    {
      Mesh.CreateText( Parameters[ 0 ], Text.GetPointer(), Parameters[ 1 ] );
    }
    break;
    }

    for ( int x = 0; x < Filters.NumItems(); x++ )
      if ( Filters[ x ]->Enabled )
      {
        switch ( Filters[ x ]->Filter )
        {
        case ModelFilter_UVMap:
          Mesh.CalculateTextureCoordinates( (PHXTEXTUREMAPTYPE)Filters[ x ]->Parameters[ 0 ], Filters[ x ]->Parameters[ 1 ] & 0x0f, Filters[ x ]->Parameters[ 1 ] & 0xf0,
                                            D3DXVECTOR3( Filters[ x ]->srt[ 0 ], Filters[ x ]->srt[ 1 ], Filters[ x ]->srt[ 2 ] ) + D3DXVECTOR3( 1, 1, 1 ),
                                            D3DXQUATERNION( Filters[ x ]->srt[ 3 ], Filters[ x ]->srt[ 4 ], Filters[ x ]->srt[ 5 ], Filters[ x ]->srt[ 6 ] ),
                                            D3DXVECTOR3( Filters[ x ]->srt[ 7 ], Filters[ x ]->srt[ 8 ], Filters[ x ]->srt[ 9 ] ) );
          break;
        case ModelFilter_MapXForm:
          Mesh.MapXForm( D3DXVECTOR3( Filters[ x ]->srt[ 0 ], Filters[ x ]->srt[ 1 ], Filters[ x ]->srt[ 2 ] ),
                         D3DXQUATERNION( Filters[ x ]->srt[ 3 ], Filters[ x ]->srt[ 4 ], Filters[ x ]->srt[ 5 ], Filters[ x ]->srt[ 6 ] ),
                         D3DXVECTOR3( Filters[ x ]->srt[ 7 ], Filters[ x ]->srt[ 8 ], Filters[ x ]->srt[ 9 ] ) );
          break;
        case ModelFilter_MeshSmooth:
          Mesh.CatmullClark( Filters[ x ]->Parameters[ 0 ] != 0, Filters[ x ]->Parameters[ 1 ] );
          break;
        case ModelFilter_SmoothGroup:
          Mesh.SmoothGroupSeparation = Filters[ x ]->Parameters[ 0 ] / 255.0f*2.0f;
          Mesh.SkipNormalCalculation = false;
          break;
        case ModelFilter_TintMesh:
          if ( Filters[ x ]->Texture && Filters[ x ]->Texture->GetContentOp() && Filters[ x ]->Texture->ContentReady() )
            Mesh.CalculateTint( Filters[ x ]->Parameters[ 0 ], Filters[ x ]->Texture->GetContentOp()->Result->Texture, Filters[ x ]->Parameters[ 1 ] );
          break;
        case ModelFilter_TintMeshShape:
          Mesh.CalculateTintShape( Filters[ x ]->Parameters[ 0 ], Filters[ x ]->Parameters[ 1 ], Filters[ x ]->Parameters[ 2 ], 
                         D3DXVECTOR3( Filters[ x ]->srt[ 0 ], Filters[ x ]->srt[ 1 ], Filters[ x ]->srt[ 2 ] ),
                         D3DXQUATERNION( Filters[ x ]->srt[ 3 ], Filters[ x ]->srt[ 4 ], Filters[ x ]->srt[ 5 ], Filters[ x ]->srt[ 6 ] ),
                         D3DXVECTOR3( Filters[ x ]->srt[ 7 ], Filters[ x ]->srt[ 8 ], Filters[ x ]->srt[ 9 ] ) );
          break;
        case ModelFilter_Bevel:
          Mesh.Bevel( Filters[ x ]->Parameters[ 0 ] / 255.0f );
          break;
        case ModelFilter_Replicate:
        {
          Mesh.Replicate( Filters[ x ]->Parameters[ 0 ], Filters[ x ]->srt );
        }
        break;
        case ModelFilter_NormalDeform:
          Mesh.NormalDeform( Filters[ x ]->Parameters[ 0 ] / 255.0f * ( Filters[ x ]->Parameters[ 1 ] - 127 ) );
          break;
        case ModelFilter_CSG:
        {
          CphxModelObject_Tool_Mesh* o = (CphxModelObject_Tool_Mesh*)Project.GetModelObject( Filters[ x ]->ReferencedGUID );
          if ( o )
          {
            D3DXMATRIX currObjMatInv;
            D3DXMatrixInverse( &currObjMatInv, nullptr, &GetMatrix() );

            D3DXMATRIX result;
            D3DXMatrixMultiply( &result, &o->GetMatrix(), &currObjMatInv );

            Mesh.CSG( &o->GetModelObject()->Mesh, &result , Filters[ x ]->Parameters[ 1 ] );
          }
          //data += 24;
          break;
        }
        case ModelFilter_Greeble:
          Mesh.Greeble( Filters[ x ]->Parameters[ 0 ], Filters[ x ]->Parameters[ 1 ] / 255.0f, Filters[ x ]->Parameters[ 2 ] / 255.0f );
          break;
        }
      }

    Mesh.BuildMesh( GetModelObject()->VxBuffer, GetModelObject()->IndexBuffer, GetModelObject()->WireBuffer, GetModelObject()->VxCount, GetModelObject()->TriCount, GetModelObject()->EdgeCount );
    GetModelObject()->dataBuffer = Mesh.dataBuffer;
    GetModelObject()->dataBufferView = Mesh.dataBufferView;
  }
  catch ( std::exception& /*e*/ )
  {
    LOG_ERR( "Memory exception thrown!" );
    SetStatusbarError( CString::Format( _T( "%s FAILED TO GENERATE DUE TO OUT OF MEMORY ERROR!" ), GetName().GetPointer() ) );

    Mesh.Vertices.FlushFast();
    Mesh.Polygons.FlushFast();

    if ( GetModelObject()->VxBuffer ) GetModelObject()->VxBuffer->Release();
    if ( GetModelObject()->IndexBuffer ) GetModelObject()->IndexBuffer->Release();
    if ( GetModelObject()->WireBuffer ) GetModelObject()->WireBuffer->Release();
    if ( GetModelObject()->dataBuffer ) GetModelObject()->dataBuffer->Release();
    if ( GetModelObject()->dataBufferView ) GetModelObject()->dataBufferView->Release();

    GetModelObject()->VxBuffer = NULL;
    GetModelObject()->IndexBuffer = NULL;
    GetModelObject()->WireBuffer = NULL;
    GetModelObject()->dataBuffer = NULL;
    GetModelObject()->dataBufferView = NULL;
    Mesh.SmoothGroupSeparation = DEFAULTSMOOTHGROUPSEPARATION;
    Mesh.SkipNormalCalculation = false;

    Mesh.BuildMesh( GetModelObject()->VxBuffer, GetModelObject()->IndexBuffer, GetModelObject()->WireBuffer, GetModelObject()->VxCount, GetModelObject()->TriCount, GetModelObject()->EdgeCount );

  }

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif

  UpdateMaterialState();
  return true;
}

bool CphxModelObject_Tool_Mesh::Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t )
{
  extern bool pickingEnabled;
  if ( !pickingEnabled )
    return false;

  if ( !ClientRect.Contains( MousePos ) ) return false;
  D3D10_VIEWPORT vp;
  vp.TopLeftX = ClientRect.TopLeft().x;
  vp.TopLeftY = ClientRect.TopLeft().y;
  vp.Width = ClientRect.Width();
  vp.Height = ClientRect.Height();
  vp.MinDepth = 0;
  vp.MaxDepth = 1;

  D3DXMATRIX m;
  D3DXMatrixMultiply( &m, &GetMatrix(), &TransformationMatrix );

  D3DXMATRIX i;
  D3DXMatrixIdentity( &i );

  D3DXVECTOR3 v1, v2, n;
  D3DXVec3Unproject( &v1, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 0 ), &vp, &ProjectionMatrix, &CameraMatrix, &i );
  D3DXVec3Unproject( &v2, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 1 ), &vp, &ProjectionMatrix, &CameraMatrix, &i );
  D3DXVec3Normalize( &n, &( v2 - v1 ) );

  bool picked = false;

  CphxMesh &Mesh = GetModelObject()->Mesh;

  for ( int x = 0; x < Mesh.Polygons.NumItems(); x++ )
    if ( Mesh.Polygons[ x ].VertexCount > 2 )
    {
      float a, b, c;

      D3DXVECTOR3 vx[ 4 ];
      for ( int y = 0; y < Mesh.Polygons[ x ].VertexCount; y++ )
      {
        D3DXVECTOR4 vt;
        D3DXVec3Transform( &vt, &Mesh.Vertices[ Mesh.Polygons[ x ].VertexIDs[ y ] ].Position, &m );
        vx[ y ] = D3DXVECTOR3( vt.x, vt.y, vt.z );
      }

      if ( D3DXIntersectTri( &vx[ 0 ], &vx[ 1 ], &vx[ 2 ], &v1, &n, &a, &b, &c ) )
      {
        if ( !picked ) t = c;
        picked = true;
        if ( c < t ) t = c;
      }

      if ( Mesh.Polygons[ x ].VertexCount > 3 )
        if ( D3DXIntersectTri( &vx[ 0 ], &vx[ 2 ], &vx[ 3 ], &v1, &n, &a, &b, &c ) )
        {
          if ( !picked ) t = c;
          picked = true;
          if ( c < t ) t = c;
        }
    }

  return picked;
}

void CphxModelObject_Tool_Mesh::SetMatrix( D3DXMATRIX m )
{
  D3DXMATRIX mx = GetMatrix();
  TBOOL different = false;
  for ( int x = 0; x < 4; x++ )
    for ( int y = 0; y < 4; y++ )
      if ( ( (D3DXFLOAT16)mx.m[ x ][ y ] ) != ( (D3DXFLOAT16)m.m[ x ][ y ] ) )
        different = true;

  CphxModelObject_Tool::SetMatrix( m );
  if ( different && HasChildrenOfType( PHX_MODELOBJECT ) )
    InvalidateChildren();
}

void CphxModelObject_Tool_Mesh::UpdateDependencies()
{
  RemoveParents( PHX_MODELOBJECT );

  switch ( Primitive )
  {
  case Mesh_Scatter:
  case Mesh_Loft:
  {
    CphxModelObject_Tool *m;
    if ( m = Project.GetModelObject( ParentGUIDS[ 0 ] ) )
    {
      for ( int x = 0; x < ParentModel->GetObjectCount(); x++ )
        if ( ParentModel->GetObjectByIndex( x ) == m )
        {
          Parameters[ 0 ] = x;
          AddParent( m );
        }
    }
    if ( m = Project.GetModelObject( ParentGUIDS[ 1 ] ) )
    {
      for ( int x = 0; x < ParentModel->GetObjectCount(); x++ )
        if ( ParentModel->GetObjectByIndex( x ) == m )
        {
          Parameters[ 1 ] = x;
          AddParent( m );
        }
    }
  }
  break;
  case Mesh_Tree:
  case Mesh_TreeLeaves:
  {
    RemoveParents( PHX_TREESPECIES );
    CphxTreeSpecies *t;
    if ( t = Project.GetTreeSpecies( ParentGUIDS[ 0 ] ) )
    {
      Parameters[ 0 ] = Project.GetTreeSpeciesIndex( ParentGUIDS[ 0 ] );
      AddParent( t );
    }
  }
  break;
  case Mesh_Copy:
  {
    CphxModelObject_Tool *m;
    if ( m = Project.GetModelObject( ParentGUIDS[ 0 ] ) )
    {
      for ( int x = 0; x < ParentModel->GetObjectCount(); x++ )
        if ( ParentModel->GetObjectByIndex( x ) == m )
        {
          Parameters[ 0 ] = x;
          AddParent( m );
        }
    }
  }
  break;
  }

  for ( int x = 0; x < Filters.NumItems(); x++ )
  {
    if ( Filters[ x ]->Filter == ModelFilter_CSG )
    {
      CphxModelObject_Tool *m;
      if ( m = Project.GetModelObject( Filters[ x ]->ReferencedGUID ) )
      {
        for ( int y = 0; y < ParentModel->GetObjectCount(); y++ )
          if ( ParentModel->GetObjectByIndex( y ) == m )
          {
            Filters[ x ]->Parameters[ 0 ] = y;
            AddParent( m );
          }
      }
    }
  }

  ClonedObjects.FlushFast();
  GetModelObject()->ClonedObjects.FlushFast();

  for ( int x = 0; x < ClonedGuids.NumItems(); x++ )
  {
    CphxModelObject_Tool* m = Project.GetModelObject( ClonedGuids[ x ] );
    if ( m ) AddClonedObject( m );
  }
}

void CphxModelObject_Tool_Mesh::SetMaterial( CphxMaterial_Tool *m, TBOOL Update )
{
  auto *oldMat = Material;

  CphxMaterialDataStorage_Tool oldData;
  oldData.Copy( &MaterialData );

  RemoveParent( Material );
  Material = m;
  AddParent( Material );
  GetModelObject()->Material = &m->Material;
  InvalidateUptoDateFlag();

  if ( m )
  {
    if ( !Update )
      MaterialData.SetDefaultValues( m );
    else
      MaterialData.SetMissingDefaultValues( m );
  }

  if ( oldMat && !Update )
  {
    MaterialData.TryImport( &oldData, oldMat, m );
  }

  UpdateMaterialState();
}

void CphxModelObject_Tool_Mesh::UpdateMaterialState()
{
  AllocateMaterialState();

  if ( !Material ) return;
  CphxModelObject_Mesh *m = GetModelObject();

  MaterialData.UpdateParams( Material );
  UpdateMaterialTextures();

  //apply parameters to material instance

  phxMeshDataShaderView = m->dataBufferView;

  TS32 passcnt = 0;
  for ( TS32 x = 0; x < Material->Techniques.NumItems(); x++ )
    Material->Techniques[ x ]->CreateInstancedData( m->MaterialState, passcnt );

}

void CphxModelObject_Tool_Mesh::UpdateMaterialTextures()
{
  if ( !Material ) return;
  CphxModelObject_Mesh *m = GetModelObject();

  phxMeshDataShaderView = m->dataBufferView;

  MaterialData.UpdateTextures( Material );

  TS32 passcnt = 0;
  for ( TS32 x = 0; x < Material->Techniques.NumItems(); x++ )
    Material->Techniques[ x ]->CreateInstancedData_Textures( m->MaterialState, passcnt );
}


void CphxModelObject_Tool_Mesh::FreeMaterialState()
{
  CphxModelObject_Mesh *m = GetModelObject();
  if ( !m->MaterialState ) return;
  if ( !MaterialStateSize ) return;

  for ( TS32 x = 0; x < MaterialStateSize; x++ )
  {
    //need to recast here because the minimal engine version of this class has no destructor - hence no virtual destructor call
    CphxMaterialPassConstantState_Tool *t = (CphxMaterialPassConstantState_Tool*)m->MaterialState[ x ];
    SAFEDELETE( t );
    m->MaterialState[ x ] = NULL;
  }

  SAFEDELETEA( m->MaterialState );
  MaterialStateSize = 0;
}

void CphxModelObject_Tool_Mesh::AllocateMaterialState()
{
  FreeMaterialState();
  CphxModelObject_Mesh *m = GetModelObject();
  if ( !Material ) return;

  MaterialStateSize = 0;
  m->MaterialState = NULL;

  for ( TS32 x = 0; x < m->Material->TechCount; x++ )
    MaterialStateSize += m->Material->Techniques[ x ]->PassCount;

  if ( MaterialStateSize )
  {
    m->MaterialState = new CphxMaterialPassConstantState*[ MaterialStateSize ];
    for ( TS32 x = 0; x < MaterialStateSize; x++ )
      m->MaterialState[ x ] = new CphxMaterialPassConstantState_Tool();
  }

}

CphxModelObject_Tool_Clone::CphxModelObject_Tool_Clone() : CphxModelObject_Tool()
{
  Object = new CphxModelObject_Clone();
  Object->ToolObject = this;
}

CphxModelObject_Tool_Clone::~CphxModelObject_Tool_Clone()
{
}

CphxModelObject_Clone * CphxModelObject_Tool_Clone::GetModelObject()
{
  return (CphxModelObject_Clone*)Object;
}

void CphxModelObject_Tool_Clone::ExportData( CXMLNode *Node )
{
  CphxModelObject_Tool::ExportData( Node );
}

void CphxModelObject_Tool_Clone::ImportData( CXMLNode *Node )
{
  CphxModelObject_Tool::ImportData( Node );
}

TBOOL CphxModelObject_Tool_Clone::GenerateResource( CCoreDevice *Dev )
{
  return true;
}

bool CphxModelObject_Tool_Clone::Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t )
{
  D3DXMATRIX m;
  D3DXMatrixMultiply( &m, &GetMatrix(), &TransformationMatrix );

  bool Picked = false;

  for ( int x = 0; x < ClonedObjects.NumItems(); x++ )
    Picked |= ClonedObjects[ x ]->Pick( ClientRect, MousePos, ProjectionMatrix, CameraMatrix, m, t );

  return Picked;

}

void CphxModelObject_Tool_Clone::AddClonedObject( CphxModelObject_Tool *c )
{
  if ( ClonedObjects.Find( c ) >= 0 ) return;
  ClonedObjects += c;
  ClonedGuids.AddUnique( c->GetGUID() );
  AddParent( c );
#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif

  GetModelObject()->ClonedObjects.Add( c->GetObject() );
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
}

void CphxModelObject_Tool_Clone::RemoveClonedObject( CphxModelObject_Tool *c )
{
  if ( ClonedObjects.Find( c ) < 0 ) return;
  ClonedObjects -= c;
  ClonedGuids.Delete( c->GetGUID() );
  RemoveParent( c );
  GetModelObject()->ClonedObjects.Delete( c->GetObject() );
}

void CphxModelObject_Tool_Mesh::AddClonedObject( CphxModelObject_Tool* c )
{
  if ( ClonedObjects.Find( c ) >= 0 ) return;
  ClonedObjects += c;
  ClonedGuids.AddUnique( c->GetGUID() );
  AddParent( c );
#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif

  GetModelObject()->ClonedObjects.Add( c->GetObject() );
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
}

void CphxModelObject_Tool_Mesh::RemoveClonedObject( CphxModelObject_Tool* c )
{
  if ( ClonedObjects.Find( c ) < 0 ) return;
  ClonedObjects -= c;
  ClonedGuids.Delete( c->GetGUID() );
  RemoveParent( c );
  GetModelObject()->ClonedObjects.Delete( c->GetObject() );
}

TBOOL CphxModelObject_Tool_Clone::FindInCloneTree( CphxModelObject_Tool*m )
{
  if ( m == this ) return true;
  TBOOL Found = false;
  for ( TS32 x = 0; x < ClonedObjects.NumItems(); x++ )
    Found |= ClonedObjects[ x ]->FindInCloneTree( m );
  return Found;
}

void CphxModelObject_Tool_Clone::UpdateDependencies()
{
  RemoveParents( PHX_MODELOBJECT );

  ClonedObjects.FlushFast();
  GetModelObject()->ClonedObjects.FlushFast();

  for ( int x = 0; x < ClonedGuids.NumItems(); x++ )
  {
    CphxModelObject_Tool *m = Project.GetModelObject( ClonedGuids[ x ] );
    if ( m ) AddClonedObject( m );
  }
}

CphxMeshFilter_Tool::CphxMeshFilter_Tool()
{
  ParentObject = NULL;
  Texture = NULL;
  Parameters[ 0 ] = Parameters[ 1 ] = 0;
  Enabled = true;
  for ( int x = 0; x < 12; x++ )
    srt[ x ] = 0;
  Selected = false;

  //rotate
  srt[ 6 ] = 1;
}

void CphxMeshFilter_Tool::ApplyTransformation( CPRS &f )
{
  srt[ 0 ] = _srt[ 0 ] + f.Scale.x - 1;
  srt[ 1 ] = _srt[ 1 ] + f.Scale.y - 1;
  srt[ 2 ] = _srt[ 2 ] + f.Scale.z - 1;

  D3DXQUATERNION q = D3DXQUATERNION( _srt[ 3 ], _srt[ 4 ], _srt[ 5 ], _srt[ 6 ] );
  D3DXQUATERNION b;
  b.x = f.Rotation.x;
  b.y = f.Rotation.y;
  b.z = f.Rotation.z;
  b.w = f.Rotation.s;
  D3DXQuaternionMultiply( &q, &b, &q );
  srt[ 3 ] = q.x;
  srt[ 4 ] = q.y;
  srt[ 5 ] = q.z;
  srt[ 6 ] = q.w;

  srt[ 7 ] = _srt[ 7 ] + f.Translation.x;
  srt[ 8 ] = _srt[ 8 ] + f.Translation.y;
  srt[ 9 ] = _srt[ 9 ] + f.Translation.z;
}

D3DXMATRIX CphxMeshFilter_Tool::GetTransformationMatrix()
{
  D3DXMATRIX m;
  D3DXMatrixTransformation( &m, NULL, NULL, &D3DXVECTOR3( srt[ 0 ] + 1, srt[ 1 ] + 1, srt[ 2 ] + 1 ), NULL, &D3DXQUATERNION( srt[ 3 ], srt[ 4 ], srt[ 5 ], srt[ 6 ] ), &D3DXVECTOR3( srt[ 7 ], srt[ 8 ], srt[ 9 ] ) );
  return m;
}

D3DXMATRIX CphxMeshFilter_Tool::GetTransformationMatrix_()
{
  D3DXMATRIX m;
  D3DXMatrixTransformation( &m, NULL, NULL, &D3DXVECTOR3( _srt[ 0 ] + 1, _srt[ 1 ] + 1, _srt[ 2 ] + 1 ), NULL, &D3DXQUATERNION( _srt[ 3 ], _srt[ 4 ], _srt[ 5 ], _srt[ 6 ] ), &D3DXVECTOR3( _srt[ 7 ], _srt[ 8 ], _srt[ 9 ] ) );
  return m;
}

void CphxMeshFilter_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );

  for ( TS32 x = 0; x < 12; x++ )
  {
    CXMLNode t = Node->AddChild( _T( "transformation" ) );
    t.SetAttributeFromInteger( _T( "index" ), x );
    TS32 v = *( (TU16*)&srt[ x ] );
    t.SetAttributeFromInteger( _T( "value" ), v );
  }

  for ( TS32 x = 0; x < 3; x++ )
  {
    CXMLNode t = Node->AddChild( _T( "parameter" ) );
    t.SetAttributeFromInteger( _T( "index" ), x );
    t.SetAttributeFromInteger( _T( "value" ), Parameters[ x ] );
  }

  if ( Texture )
    Node->AddChild( _T( "texture" ) ).SetText( Texture->GetGUID().GetString() );

  if ( Filter == PHXMESHFILTER::ModelFilter_CSG )
  {
    CXMLNode tx = Node->AddChild( _T( "referencedguid" ) );
    tx.SetAttribute( _T( "value" ), ReferencedGUID.GetString() );
  }

  Node->AddChild( _T( "enabled" ) ).SetInt( Enabled );
}

void CphxMeshFilter_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "transformation" ) ); x++ )
  {
    CXMLNode t = Node->GetChild( _T( "transformation" ), x );
    if ( !t.HasAttribute( _T( "index" ) ) ) continue;
    if ( !t.HasAttribute( _T( "value" ) ) ) continue;
    TS32 idx = 0, v = 0;
    t.GetAttributeAsInteger( _T( "index" ), &idx );
    t.GetAttributeAsInteger( _T( "value" ), &v );
    TU16 v2 = v;
    srt[ idx ] = *( (D3DXFLOAT16*)&v2 );
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "parameter" ) ); x++ )
  {
    CXMLNode t = Node->GetChild( _T( "parameter" ), x );
    if ( !t.HasAttribute( _T( "index" ) ) ) continue;
    if ( !t.HasAttribute( _T( "value" ) ) ) continue;
    TS32 idx = 0, v = 0;
    t.GetAttributeAsInteger( _T( "index" ), &idx );
    t.GetAttributeAsInteger( _T( "value" ), &v );
    Parameters[ idx ] = v;
  }

  if ( Node->GetChildCount( _T( "texture" ) ) )
  {
    CphxGUID g;
    g.SetString( Node->GetChild( _T( "texture" ) ).GetText().GetPointer() );
    Texture = Project.GetTexgenOp( g );
    if ( Texture ) ParentObject->AddParent( Texture );
  }

  if ( Node->GetChildCount( _T( "referencedguid" ) ) )
  {
    CXMLNode t = Node->GetChild( _T( "referencedguid" ) );
    if ( t.HasAttribute( _T( "value" ) ) )
      ReferencedGUID.SetString( t.GetAttributeAsString( _T( "value" ) ).GetPointer() );
  }

  if ( Node->GetChildCount( _T( "enabled" ) ) )
  {
    TS32 v = 1;
    Node->GetChild( _T( "enabled" ) ).GetValue( v );
    Enabled = v != 0;
  }

}

D3DXMATRIX CphxMeshFilter_Tool::GetRawMatrix()
{
  D3DXMATRIX Transformation;
  D3DXMatrixIdentity( &Transformation );

  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      Transformation.m[ x ][ y ] = srt[ x + ( y << 2 ) ];
  return Transformation;
}

D3DXMATRIX CphxMeshFilter_Tool::GetRawMatrix_()
{
  D3DXMATRIX Transformation;
  D3DXMatrixIdentity( &Transformation );

  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      Transformation.m[ x ][ y ] = _srt[ x + ( y << 2 ) ];
  return Transformation;
}

void CphxMeshFilter_Tool::SetRawMatrix( D3DXMATRIX &m )
{
  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      srt[ x + ( y << 2 ) ] = m.m[ x ][ y ];
}


void CphxMeshFilter_Tool::SetTransformationMatrix( D3DXMATRIX &m )
{
  CMatrix4x4 mx( &m._11 );
  ApplyTransformation( CPRS( mx ) );
}

void CphxMeshFilter_Tool::ApplyMatrix( D3DXMATRIX &m, TBOOL Invert )
{
  D3DXMATRIX old, newm;
  D3DXMatrixIdentity( &old );
  D3DXMatrixIdentity( &newm );
  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      old.m[ x ][ y ] = _srt[ x + ( y << 2 ) ];

  if ( !Invert )
    D3DXMatrixMultiply( &newm, &old, &m );
  else
    D3DXMatrixMultiply( &newm, &m, &old );

  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      srt[ x + ( y << 2 ) ] = newm.m[ x ][ y ];
}

CphxMaterialPassConstantState_Tool::CphxMaterialPassConstantState_Tool()
{
  BlendState = NULL;
  RasterizerState = NULL;
  DepthStencilState = NULL;
  //ConstData = NULL;
  //DynamicData = NULL;
  ConstantDataSize = 0;
  AnimatedDataSize = 0;

  RenderPriority = 127;
  Wireframe = false;
  RenderTarget = NULL;
  for ( TS32 x = 0; x < 8; x++ )
    Textures[ x ] = NULL;
}

CphxMaterialPassConstantState_Tool::~CphxMaterialPassConstantState_Tool()
{
  if ( BlendState ) BlendState->Release();
  if ( RasterizerState ) RasterizerState->Release();
  if ( DepthStencilState ) DepthStencilState->Release();
  //if (ConstData) ConstData->Release();
  //if (DynamicData) DynamicData->Release();

  BlendState = NULL;
  RasterizerState = NULL;
  DepthStencilState = NULL;
  //ConstData = NULL;
  //DynamicData = NULL;
}

void CphxMaterialDataStorage_Tool::UpdateParams( CphxMaterial_Tool *Material )
{
  if ( !Material ) return;

  for ( TS32 x = 0; x < MaterialParams.NumItems(); x++ )
  {
    CDictionary<CphxGUID, MATERIALVALUE>::KDPair *kdp = MaterialParams.GetKDPair( x );
    CphxMaterialParameter_Tool *param = Material->GetParameter( kdp->Key );
    if ( !param )
    {
      //LOG_ERR( "[phx] Parameter %s not found in material %s", kdp->Key.GetString(), Material->Name.GetPointer() );
      continue;
    }
    param->Parameter.Value = kdp->Data;
  }
}

void CphxMaterialDataStorage_Tool::UpdateTextures( CphxMaterial_Tool *Material )
{
  if ( !Material ) return;

  for ( TS32 x = 0; x < MaterialTextures.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CphxGUID>::KDPair *kdp = MaterialTextures.GetKDPair( x );
    CphxMaterialParameter_Tool *param = Material->GetParameter( kdp->Key );
    if ( !param )
    {
      //LOG_ERR( "[phx] Parameter %s not found in material %s", kdp->Key.GetString(), Material->Name.GetPointer() );
      continue;
    }
    param->TextureGUID = kdp->Data;
    param->Parameter.Value.TextureView = Project.GetTextureView( kdp->Data );
  }
}

void CphxMaterialDataStorage_Tool::UpdateParams( CphxMaterialTechnique_Tool *Tech )
{
  if ( !Tech ) return;

  for ( TS32 x = 0; x < MaterialParams.NumItems(); x++ )
  {
    CDictionary<CphxGUID, MATERIALVALUE>::KDPair *kdp = MaterialParams.GetKDPair( x );
    CphxMaterialParameter_Tool *param = Tech->GetParameter( kdp->Key );
    if ( !param )
    {
      LOG_ERR( "[phx] Parameter %s not found in tech %s", kdp->Key.GetString(), Tech->Name.GetPointer() );
      continue;
    }
    param->Parameter.Value = kdp->Data;
  }
}

void CphxMaterialDataStorage_Tool::UpdateTextures( CphxMaterialTechnique_Tool *Tech )
{
  if ( !Tech ) return;

  for ( TS32 x = 0; x < MaterialTextures.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CphxGUID>::KDPair *kdp = MaterialTextures.GetKDPair( x );
    CphxMaterialParameter_Tool *param = Tech->GetParameter( kdp->Key );
    if ( !param )
    {
      //LOG_ERR( "[phx] Parameter %s not found in material %s", kdp->Key.GetString(), Tech->Name.GetPointer() );
      continue;
    }
    param->TextureGUID = kdp->Data;

    if ( param->Parameter.Type >= PARAM_TEXTURE0 && param->Parameter.Type <= PARAM_TEXTURE7 )
      param->Parameter.Value.TextureView = Project.GetTextureView( kdp->Data );
    else
      if ( param->Parameter.Type == PARAM_RENDERTARGET )
      {
        CphxRenderTarget_Tool *rt = Project.GetRenderTarget( kdp->Data );
        if ( rt )
          param->Parameter.Value.RenderTarget = &rt->rt;
        else
        {
          CphxGUID none;
          none.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );
          if ( kdp->Data != none )
            LOG_ERR( "[phx] Rendertarget %s not found!", kdp->Data.GetString() );
          else
            LOG_WARN( "[phx] Rendertarget for technique '%s' parameter '%s' not set (yet?)", Tech->Name.GetPointer(), param->Name.GetPointer() );
        }
      }
      else
        LOG_ERR( "[phx] Non texture parameter in material data storage! (%s)", param->GetGUID().GetString() );
  }
}


void CphxMaterialDataStorage_Tool::SetDefaultValues( CphxMaterial_Tool *Material )
{
  MaterialParams.Flush();
  MaterialTextures.Flush();
  SetMissingDefaultValues( Material );
}

void CphxMaterialDataStorage_Tool::SetMissingDefaultValues( CphxMaterial_Tool *Material )
{
  if ( !Material ) return;
  for ( TS32 x = 0; x < Material->Techniques.NumItems(); x++ )
    SetMissingDefaultValues( Material->Techniques[ x ] );
}


void CphxMaterialDataStorage_Tool::SetDefaultValues( CphxMaterialTechnique_Tool *Tech )
{
  MaterialParams.Flush();
  MaterialTextures.Flush();
  SetMissingDefaultValues( Tech );
}

void CphxMaterialDataStorage_Tool::SetMissingDefaultValues( CphxMaterialTechnique_Tool *Tech )
{
  if ( !Tech ) return;
  SetMissingDefaultValues( &Tech->TechParameters );
  for ( TS32 y = 0; y < Tech->Passes.NumItems(); y++ )
    SetMissingDefaultValues( &Tech->Passes[ y ]->PassParameters );
}

void CphxMaterialDataStorage_Tool::SetDefaultValues( CphxMaterialParameterBatch_Tool *Batch )
{
  for ( TS32 x = 0; x < Batch->Parameters.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool *p = Batch->Parameters[ x ];
    if ( p->Parameter.Scope != PARAM_VARIABLE ) continue;
    if ( ( p->Parameter.Type >= PARAM_TEXTURE0 && p->Parameter.Type <= PARAM_TEXTURE7 ) || p->Parameter.Type == PARAM_RENDERTARGET )
    {
      MaterialTextures[ p->GetGUID() ].SetString( "NONENONENONENONENONENONENONENONE" );
      continue;
    }

    MaterialParams[ p->GetGUID() ] = p->DefaultValue;
    //if (p->Parameter.Type == PARAM_DEPTHBUFFER)
    //	p->Parameter.Value.TextureView = phxDepthBufferShaderView;

    //if (p->Parameter.Type >= PARAM_BLENDMODE0 && p->Parameter.Type <= PARAM_BLENDMODE7)
    //{
    //	MaterialParams[p->GetGUID()].BlendMode = phxSrcBlend_ONE | phxDstBlend_ZERO;
    //	continue;
    //}
    //switch (p->Parameter.Type)
    //{
    //	case PARAM_CULLMODE: 
    //		MaterialParams[p->GetGUID()].CullMode = D3D11_CULL_BACK;
    //		break;
    //	case PARAM_FILLMODE:
    //		MaterialParams[p->GetGUID()].Wireframe = false;
    //		break;
    //	case PARAM_ZFUNCTION:
    //		MaterialParams[p->GetGUID()].ZFunction = D3D11_COMPARISON_LESS_EQUAL;
    //		break;
    //	case PARAM_ZMODE:
    //		MaterialParams[p->GetGUID()].ZMode = 0;
    //		break;
    //	default:
    //		break;
    //}
  }
}

void CphxMaterialDataStorage_Tool::SetMissingDefaultValues( CphxMaterialParameterBatch_Tool *Batch )
{
  for ( TS32 x = 0; x < Batch->Parameters.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool *p = Batch->Parameters[ x ];

    if ( MaterialTextures.HasKey( p->GetGUID() ) || MaterialParams.HasKey( p->GetGUID() ) ) continue;

    if ( p->Parameter.Scope != PARAM_VARIABLE ) continue;
    if ( ( p->Parameter.Type >= PARAM_TEXTURE0 && p->Parameter.Type <= PARAM_TEXTURE7 ) || p->Parameter.Type == PARAM_RENDERTARGET )
    {
      MaterialTextures[ p->GetGUID() ].SetString( "NONENONENONENONENONENONENONENONE" );
      continue;
    }

    MaterialParams[ p->GetGUID() ] = p->DefaultValue;
  }
}


void CphxMaterialDataStorage_Tool::ExportData( CXMLNode *Node )
{
  for ( TS32 x = 0; x < MaterialParams.NumItems(); x++ )
  {
    CDictionary<CphxGUID, MATERIALVALUE>::KDPair *kdp = MaterialParams.GetKDPair( x );
    CXMLNode n = Node->AddChild( _T( "materialparamdata" ) );
    n.SetAttribute( _T( "paramid" ), kdp->Key.GetString() );

    CString s = CString::EncodeToBase64( (TU8*)&kdp->Data, sizeof( kdp->Data ) );
    n.SetAttribute( _T( "valuebase64" ), s.GetPointer() );
  }
  for ( TS32 x = 0; x < MaterialTextures.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CphxGUID>::KDPair *kdp = MaterialTextures.GetKDPair( x );
    CXMLNode n = Node->AddChild( _T( "materialtexturedata" ) );
    n.SetAttribute( _T( "paramid" ), kdp->Key.GetString() );
    n.SetAttribute( _T( "textureid" ), kdp->Data.GetString() );
  }
}

void CphxMaterialDataStorage_Tool::ImportData( CXMLNode *Node )
{
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "materialparamdata" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "materialparamdata" ), x );
    if ( n.HasAttribute( _T( "paramid" ) ) )
    {
      CphxGUID p;
      p.SetString( n.GetAttributeAsString( _T( "paramid" ) ).GetPointer() );
      if ( n.HasAttribute( _T( "value" ) ) )
      {
        TS32 v = 0;
        n.GetAttributeAsInteger( _T( "value" ), &v );
        MaterialParams[ p ].ZFunction = (D3D11_COMPARISON_FUNC)v;
      }
      if ( n.HasAttribute( _T( "valuebase64" ) ) )
      {
        CString t = n.GetAttributeAsString( _T( "valuebase64" ) );

        TU8 *Data;
        TS32 DataSize = 0;

        t.DecodeBase64( Data, DataSize );

        if ( DataSize == sizeof( MATERIALVALUE ) )
          memcpy( &MaterialParams[ p ], Data, DataSize );

        delete[] Data;
      }
    }
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "materialtexturedata" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "materialtexturedata" ), x );
    if ( n.HasAttribute( _T( "paramid" ) ) )
    {
      CphxGUID p;
      p.SetString( n.GetAttributeAsString( _T( "paramid" ) ).GetPointer() );
      if ( n.HasAttribute( _T( "textureid" ) ) )
      {
        CphxGUID t;
        t.SetString( n.GetAttributeAsString( _T( "textureid" ) ).GetPointer() );
        MaterialTextures[ p ] = t;
      }
    }
  }
}

void CphxMaterialDataStorage_Tool::Copy( CphxMaterialDataStorage_Tool *Original )
{
  for ( TS32 x = 0; x < Original->MaterialParams.NumItems(); x++ )
  {
    CDictionary<CphxGUID, MATERIALVALUE>::KDPair *p = Original->MaterialParams.GetKDPair( x );
    MATERIALVALUE g = p->Data;
    MaterialParams[ p->Key ] = g;
  }

  for ( TS32 x = 0; x < Original->MaterialTextures.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CphxGUID>::KDPair *p = Original->MaterialTextures.GetKDPair( x );
    CphxGUID g = p->Data;
    MaterialTextures[ p->Key ] = g;
  }
}

void CphxMaterialDataStorage_Tool::TryImport( CphxMaterialDataStorage_Tool *Original, CphxMaterial_Tool *oldMat, CphxMaterial_Tool *newMat )
{
  if ( !oldMat || !newMat )
    return;

  for ( int x = 0; x < MaterialParams.NumItems(); x++ )
  {
    auto kdp = MaterialParams.GetKDPair( x );
    CphxMaterialParameter_Tool* newParam = newMat->GetParameter( kdp->Key );

    if ( !newParam )
    {
      LOG_WARN( "Parameter %s not found in new material... wtf", kdp->Key.GetString() );
      continue;
    }

    // first try to find by guid
    CphxMaterialParameter_Tool* oldParam = oldMat->GetParameter( kdp->Key );

    // then try to find by name and type
    if ( !oldParam )
    {
      for ( int y = 0; y < Original->MaterialParams.NumItems(); y++ )
      {
        auto kdp = Original->MaterialParams.GetKDPair( y );
        CphxMaterialParameter_Tool* oldMatParam = oldMat->GetParameter( kdp->Key );
        if ( !oldMatParam )
        {
          LOG_WARN( "Parameter %s not found in old material... wtf", kdp->Key.GetString() );
          continue;
        }

        if ( newParam->Name == oldMatParam->Name && newParam->Parameter.Type == oldMatParam->Parameter.Type && newParam->Parameter.Scope == oldMatParam->Parameter.Scope )        
        {
          // type name and scope match
          oldParam = oldMatParam;
          break;
        }
      }
    }

    if ( oldParam )
    {
      // match found, replace with old value

      if ( MaterialParams.HasKey( newParam->GetGUID() ) && Original->MaterialParams.HasKey( oldParam->GetGUID() ) )
      {
        MaterialParams[ newParam->GetGUID() ] = Original->MaterialParams[ oldParam->GetGUID() ];
      }
    }
  }

  // repeat for textures
  for ( int x = 0; x < MaterialTextures.NumItems(); x++ )
  {
    auto kdp = MaterialTextures.GetKDPair( x );
    CphxMaterialParameter_Tool* newParam = newMat->GetParameter( kdp->Key );

    if ( !newParam )
    {
      LOG_WARN( "Parameter %s not found in new material... wtf", kdp->Key.GetString() );
      continue;
    }

    // first try to find by guid
    CphxMaterialParameter_Tool* oldParam = oldMat->GetParameter( kdp->Key );

    // then try to find by name and type
    if ( !oldParam )
    {
      for ( int y = 0; y < Original->MaterialTextures.NumItems(); y++ )
      {
        auto kdp = Original->MaterialTextures.GetKDPair( y );
        CphxMaterialParameter_Tool* oldMatParam = oldMat->GetParameter( kdp->Key );
        if ( !oldMatParam )
        {
          LOG_WARN( "Parameter %s not found in old material... wtf", kdp->Key.GetString() );
          continue;
        }

        if ( newParam->Name == oldMatParam->Name && newParam->Parameter.Type == oldMatParam->Parameter.Type && newParam->Parameter.Scope == oldMatParam->Parameter.Scope )
        {
          // type name and scope match
          oldParam = oldMatParam;
          break;
        }
      }
    }

    if ( oldParam )
    {
      // match found, replace with old value

      if ( MaterialTextures.HasKey( newParam->GetGUID() ) && Original->MaterialTextures.HasKey( oldParam->GetGUID() ) )
      {
        MaterialTextures[ newParam->GetGUID() ] = Original->MaterialTextures[ oldParam->GetGUID() ];
      }
    }
  }

}

CphxMaterialDataStorage_Tool::~CphxMaterialDataStorage_Tool()
{
  TS32 x = 0;

}
