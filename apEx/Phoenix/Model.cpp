#include "Model.h"
#include "Scene.h"

//model

void CphxModel::CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *ToolData )
{
  for ( int x = 0; x < Objects.NumItems(); x++ )
    Objects[ x ]->CreateRenderDataInstances( Clip, m, RootScene, ToolData );
}

//modelobjects

D3DXMATRIX CphxModelObject::GetMatrix()
{
  //return Matrix;
  D3DXMATRIX m;
  D3DXMatrixIdentity( &m );
  for ( int x = 0; x < 4; x++ )
    for ( int y = 0; y < 3; y++ )
      m.m[ x ][ y ] = TransformationF16[ y + x * 3 ];
  return m;
}

void CphxModelObject_Mesh::CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *CloneData )
{
  if ( !Material ) return;

  D3DXMatrixMultiply( &phxWorldMatrix, &GetMatrix(), &m );
  D3DXMatrixInverse( &phxITWorldMatrix, NULL, &phxWorldMatrix );
  D3DXMatrixTranspose( &phxITWorldMatrix, &phxITWorldMatrix );

  // load animated data into object material
  Clip->MaterialSplines->ApplyToParameters( this );

  //update dynamic material variables

  int passid = 0;

  for ( int x = 0; x < Material->TechCount; x++ )
    for ( int y = 0; y < Material->Techniques[ x ]->PassCount; y++ )
      Material->Techniques[ x ]->CollectAnimatedData( MaterialState[ passid++ ], y );

  //create render data instance here
  Material->CreateRenderDataInstances( this, RootScene, CloneData ? CloneData : ToolObject );
}

void CphxModelObject_Clone::CreateRenderDataInstances( CphxObjectClip *Clip, const D3DXMATRIX &m, CphxScene *RootScene, void *ToolData )
{
  D3DXMATRIX newm;
  D3DXMatrixMultiply( &newm, &GetMatrix(), &m );

  for ( int x = 0; x < ClonedObjects.NumItems(); x++ )
    ClonedObjects[ x ]->CreateRenderDataInstances( Clip, newm, RootScene, ToolData ? ToolData : ToolObject );
}
