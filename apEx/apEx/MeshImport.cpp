#include "BasePCH.h"
#include "MeshImport.h"
#include "..\Phoenix_Tool\apxProject.h"
#include "NvTriStrip\NvTriStrip.h"

float sceneImportScale = 1;

#define ASSIMP_SUPPORT

#ifdef ASSIMP_SUPPORT

#ifndef _DEBUG
#pragma comment(lib,"..\\Libraries\\Assimp\\assimp.lib")
#pragma comment(lib,"..\\Libraries\\Assimp\\zlibstat.lib")
#else
#pragma comment(lib,"..\\Libraries\\Assimp\\assimp_dbg.lib")
#pragma comment(lib,"..\\Libraries\\Assimp\\zlibstat.lib")
#endif

//#pragma push_macro("new")
//#undef new
#define SWIG
#include "../Libraries/Assimp/include/assimp/cimport.h"
#include "../Libraries/Assimp/include/assimp/scene.h"
#include "../Libraries/Assimp/include/assimp/postprocess.h"
//#pragma pop_macro("new")

CphxModel_Tool *ImportSceneNodeModel( const aiScene *Scene, aiNode *Node )
{
  CphxModel_Tool *model = Project.CreateModel();
  model->SetName( CString( Node->mName.data ) );

  if ( !Node->mName.data )
    model->SetName( CString( "Mesh" ) );


  for ( TU32 x = 0; x < Node->mNumMeshes; x++ )
  {
    const aiMesh* Mesh = Scene->mMeshes[ Node->mMeshes[ x ] ];

    CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)model->AddPrimitive( Mesh_Stored );
    mesh->GetModelObject()->StoredVertexCount = Mesh->mNumVertices;
    mesh->GetModelObject()->StoredVertices = new CphxVertex[ Mesh->mNumVertices ];

    CphxVertex *vx = mesh->GetModelObject()->StoredVertices;

    for ( TU32 y = 0; y < Mesh->mNumVertices; y++ )
    {
      vx[ y ].Position.x = Mesh->mVertices[ y ].x;
      vx[ y ].Position.y = Mesh->mVertices[ y ].y;
      vx[ y ].Position.z = Mesh->mVertices[ y ].z;

      if ( Mesh->mNormals )
      {
        vx[ y ].Normal.x = Mesh->mNormals[ y ].x;
        vx[ y ].Normal.y = Mesh->mNormals[ y ].y;
        vx[ y ].Normal.z = Mesh->mNormals[ y ].z;
      }
    }

    mesh->GetModelObject()->StoredPolyCount = 0;// Mesh->mNumFaces;
    mesh->GetModelObject()->StoredPolygons = new CphxPolygon[ Mesh->mNumFaces ];

    //if ( Mesh->mNormals )
    mesh->GetModelObject()->Mesh.SkipNormalCalculation = true;

    CphxPolygon *pl = mesh->GetModelObject()->StoredPolygons;

    for ( TU32 y = 0; y < Mesh->mNumFaces; y++ )
    {
      if ( Mesh->mFaces[ y ].mNumIndices <= 4 && Mesh->mFaces[ y ].mNumIndices > 2 )
      {
        mesh->GetModelObject()->StoredPolyCount++;
        pl[ y ].VertexCount = Mesh->mFaces[ y ].mNumIndices;
        for ( TU32 z = 0; z < Mesh->mFaces[ y ].mNumIndices; z++ )
        {
          pl[ y ].VertexIDs[ z ] = Mesh->mFaces[ y ].mIndices[ z ];

          if ( Mesh->mNormals )
          {
            pl[ y ].Normals[ z ].x = Mesh->mNormals[ pl[ y ].VertexIDs[ z ] ].x;
            pl[ y ].Normals[ z ].y = Mesh->mNormals[ pl[ y ].VertexIDs[ z ] ].y;
            pl[ y ].Normals[ z ].z = Mesh->mNormals[ pl[ y ].VertexIDs[ z ] ].z;
          }

          if ( Mesh->mTextureCoords && Mesh->mTextureCoords[ 0 ] )
          {
            pl[ y ].Texcoords[ z ][ 0 ].x = Mesh->mTextureCoords[ 0 ][ pl[ y ].VertexIDs[ z ] ].x;
            pl[ y ].Texcoords[ z ][ 0 ].y = 1 - Mesh->mTextureCoords[ 0 ][ pl[ y ].VertexIDs[ z ] ].y;
          }
        }
      }
      else
        LOG_ERR( "[assimp] Face %d in object %s has more than 4 vertices!", y, Mesh->mName.data );
    }
  }

  return model;
}

void ImportSceneNodeModel_OnlyModel( CphxModel_Tool *model, const aiScene *Scene, aiNode *Node, D3DXMATRIX m, bool importMini )
{
  for ( TU32 x = 0; x < Node->mNumMeshes; x++ )
  {
    const aiMesh* Mesh = Scene->mMeshes[ Node->mMeshes[ x ] ];

    if ( !importMini )
    {
      CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)model->AddPrimitive( Mesh_Stored );
      mesh->GetModelObject()->StoredVertexCount = Mesh->mNumVertices;
      mesh->GetModelObject()->StoredVertices = new CphxVertex[ Mesh->mNumVertices ];
      mesh->SetMatrix( m );
      mesh->SetName( CString( Node->mName.data ) );

      CphxVertex *vx = mesh->GetModelObject()->StoredVertices;

      for ( TU32 y = 0; y < Mesh->mNumVertices; y++ )
      {
        vx[ y ].Position.x = Mesh->mVertices[ y ].x;
        vx[ y ].Position.y = Mesh->mVertices[ y ].y;
        vx[ y ].Position.z = Mesh->mVertices[ y ].z;

        if ( Mesh->mNormals )
        {
          vx[ y ].Normal.x = Mesh->mNormals[ y ].x;
          vx[ y ].Normal.y = Mesh->mNormals[ y ].y;
          vx[ y ].Normal.z = Mesh->mNormals[ y ].z;
        }
      }

      mesh->GetModelObject()->StoredPolyCount = 0;// Mesh->mNumFaces;
      mesh->GetModelObject()->StoredPolygons = new CphxPolygon[ Mesh->mNumFaces ];

      //if ( Mesh->mNormals )
      mesh->GetModelObject()->Mesh.SkipNormalCalculation = true;

      CphxPolygon *pl = mesh->GetModelObject()->StoredPolygons;

      for ( TU32 y = 0; y < Mesh->mNumFaces; y++ )
      {
        if ( Mesh->mFaces[ y ].mNumIndices <= 4 && Mesh->mFaces[ y ].mNumIndices > 2 )
        {
          mesh->GetModelObject()->StoredPolyCount++;
          pl[ y ].VertexCount = Mesh->mFaces[ y ].mNumIndices;
          for ( TU32 z = 0; z < Mesh->mFaces[ y ].mNumIndices; z++ )
          {
            pl[ y ].VertexIDs[ z ] = Mesh->mFaces[ y ].mIndices[ z ];

            if ( Mesh->mNormals )
            {
              pl[ y ].Normals[ z ].x = Mesh->mNormals[ pl[ y ].VertexIDs[ z ] ].x;
              pl[ y ].Normals[ z ].y = Mesh->mNormals[ pl[ y ].VertexIDs[ z ] ].y;
              pl[ y ].Normals[ z ].z = Mesh->mNormals[ pl[ y ].VertexIDs[ z ] ].z;
            }

            if ( Mesh->mTextureCoords && Mesh->mTextureCoords[ 0 ] )
            {
              pl[ y ].Texcoords[ z ][ 0 ].x = Mesh->mTextureCoords[ 0 ][ pl[ y ].VertexIDs[ z ] ].x;
              pl[ y ].Texcoords[ z ][ 0 ].y = 1 - Mesh->mTextureCoords[ 0 ][ pl[ y ].VertexIDs[ z ] ].y;
            }
          }
        }
        else
          LOG_ERR( "[assimp] Face %d in object %s has more than 4 vertices!", y, Mesh->mName.data );
      }
    }
    else
    {
      // miniMesh magic!!!!
      DisableRestart();
      SetCacheSize( 65536 );
      //SetStitchStrips( false );

      D3DXVECTOR3 bbMin, bbMax;

      if ( Mesh->mNumVertices )
      {
        CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)model->AddPrimitive( Mesh_StoredMini );
        mesh->SetName( CString( Node->mName.data ) );

        bbMin = bbMax = D3DXVECTOR3( Mesh->mVertices[ 0 ].x, Mesh->mVertices[ 0 ].y, Mesh->mVertices[ 0 ].z );

        for ( TU32 y = 0; y < Mesh->mNumVertices; y++ )
        {
          bbMin.x = min( bbMin.x, Mesh->mVertices[ y ].x );
          bbMin.y = min( bbMin.y, Mesh->mVertices[ y ].y );
          bbMin.z = min( bbMin.z, Mesh->mVertices[ y ].z );
          bbMax.x = max( bbMax.x, Mesh->mVertices[ y ].x );
          bbMax.y = max( bbMax.y, Mesh->mVertices[ y ].y );
          bbMax.z = max( bbMax.z, Mesh->mVertices[ y ].z );
        }
        D3DXVECTOR3 bbSize = bbMax - bbMin;

        for ( TU32 y = 0; y < Mesh->mNumVertices; y++ )
        {
          D3DXVECTOR3 v = D3DXVECTOR3( Mesh->mVertices[ y ].x,
                                       Mesh->mVertices[ y ].y,
                                       Mesh->mVertices[ y ].z );
          mesh->miniModelVertices.Add( unsigned char( ( ( v.y - bbMin.y ) / bbSize.y ) * 255 ) );
          mesh->miniModelVertices.Add( unsigned char( ( ( v.x - bbMin.x ) / bbSize.x ) * 255 ) );
          mesh->miniModelVertices.Add( unsigned char( ( ( v.z - bbMin.z ) / bbSize.z ) * 255 ) );
        }

        D3DXMATRIX mx;
        D3DXMatrixScaling( &mx, bbSize.y, bbSize.x, bbSize.z );
        D3DXMatrixIdentity( &m );
        D3DXMatrixMultiply( &m, &mx, &m );

        mesh->SetMatrix( m );

        CArray<unsigned short> indices;

        for ( TU32 y = 0; y < Mesh->mNumFaces; y++ )
        {
          if ( Mesh->mFaces[ y ].mNumIndices == 3 )
          {
            for ( TS32 z = Mesh->mFaces[ y ].mNumIndices - 1; z >= 0; z-- )
            {
              indices += Mesh->mFaces[ y ].mIndices[ z ];
              mesh->miniModelTriangles.Add( Mesh->mFaces[ y ].mIndices[ z ] );
            }
          }
        }

        PrimitiveGroup* groupOutput = nullptr;
        unsigned short numGroups = 0;
        GenerateStrips( indices.GetPointer( 0 ), indices.NumItems(), &groupOutput, &numGroups, false );

        /*for ( size_t x = 0; x < groupOutput->numIndices; x++ )
        {
          D3DXVECTOR3 v = D3DXVECTOR3( Mesh->mVertices[ groupOutput->indices[ x ] ].x,
                                       Mesh->mVertices[ groupOutput->indices[ x ] ].y,
                                       Mesh->mVertices[ groupOutput->indices[ x ] ].z );
          mesh->miniModelVertices.Add( unsigned char( ( ( v.x - bbMin.x ) / bbSize.x ) * 255 ) );
          mesh->miniModelVertices.Add( unsigned char( ( ( v.y - bbMin.y ) / bbSize.y ) * 255 ) );
          mesh->miniModelVertices.Add( unsigned char( ( ( v.z - bbMin.z ) / bbSize.z ) * 255 ) );
        }*/

        delete[] groupOutput;
      }

    }
  }
}


void ImportSceneHierarchy( const aiScene *AIScene, aiNode *Node, CphxScene_Tool *Scene, CphxObject_Tool *Root, CArray<CphxModel_Tool*> &Models )
{
  CphxObject_Tool *RootObj;

  if ( !Node->mNumMeshes )
  {
    RootObj = Scene->AddObject( Object_Dummy, NULL );
  }
  else
  {
    CphxModel_Tool *model = ImportSceneNodeModel( AIScene, Node );

    RootObj = Scene->AddObject( Object_Model, model );// Models[ Node->mMeshes[ 0 ] ] );
  }

  RootObj->SetParent( Root, Scene );
  D3DXMATRIX m;
  for ( TS32 x = 0; x < 4; x++ )
    for ( TS32 y = 0; y < 4; y++ )
      m.m[ y ][ x ] = Node->mTransformation[ x ][ y ];

  D3DXVECTOR3 Scale, Position;
  D3DXQUATERNION Rotation;

  D3DXMatrixDecompose( &Scale, &Rotation, &Position, &m );
  RootObj->GetSpline( 0, Spline_Scale_x )->Spline->Value[ 0 ] = Scale.x;
  RootObj->GetSpline( 0, Spline_Scale_y )->Spline->Value[ 0 ] = Scale.y;
  RootObj->GetSpline( 0, Spline_Scale_z )->Spline->Value[ 0 ] = Scale.z;

  RootObj->GetSpline( 0, Spline_Position_x )->Spline->Value[ 0 ] = Position.x;
  RootObj->GetSpline( 0, Spline_Position_y )->Spline->Value[ 0 ] = Position.y;
  RootObj->GetSpline( 0, Spline_Position_z )->Spline->Value[ 0 ] = Position.z;

  RootObj->GetSpline( 0, Spline_Rotation )->Spline->Value[ 0 ] = Rotation.x;
  RootObj->GetSpline( 0, Spline_Rotation )->Spline->Value[ 1 ] = Rotation.y;
  RootObj->GetSpline( 0, Spline_Rotation )->Spline->Value[ 2 ] = Rotation.z;
  RootObj->GetSpline( 0, Spline_Rotation )->Spline->Value[ 3 ] = Rotation.w;

  //if ( Node->mNumMeshes != 1 )
  //  for ( TU32 x = 0; x < Node->mNumMeshes; x++ )
  //  {
  //    CphxObject_Tool *tm = Scene->AddObject( Object_Model, Models[ Node->mMeshes[ x ] ] );
  //    tm->SetParent( RootObj, Scene );
  //  }

  for ( TU32 x = 0; x < Node->mNumChildren; x++ )
  {
    ImportSceneHierarchy( AIScene, Node->mChildren[ x ], Scene, RootObj, Models );
  }
}

void ImportSceneHierarchy_Model( const aiScene *AIScene, aiNode *Node, CphxModel_Tool *Scene, D3DXMATRIX parent, bool importMini )
{
  D3DXMATRIX m;
  for ( TS32 x = 0; x < 4; x++ )
    for ( TS32 y = 0; y < 4; y++ )
      m.m[ y ][ x ] = Node->mTransformation[ x ][ y ];

  D3DXMatrixMultiply( &m, &m, &parent );

  ImportSceneNodeModel_OnlyModel( Scene, AIScene, Node, m, importMini );

  for ( TU32 x = 0; x < Node->mNumChildren; x++ )
  {
    ImportSceneHierarchy_Model( AIScene, Node->mChildren[ x ], Scene, m, importMini );
  }
}

void ImportWithAssimp( CString &File )
{
#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif

  const aiScene *Scene = aiImportFile( File.GetPointer(), aiProcessPreset_TargetRealtime_MaxQuality );

  if ( !Scene )
  {
    LOG_ERR( "[assimp] Import error: %s", aiGetErrorString() );
#ifdef MEMORY_TRACKING
    memTracker.SetMissingIgnore( false );
#endif
    return;
  }

  CArray<CphxModel_Tool*> Models;

  CphxScene_Tool *sc = Project.CreateScene();
  sc->AddClip();

  ImportSceneHierarchy( Scene, Scene->mRootNode, sc, NULL, Models );

  aiReleaseImport( Scene );

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif
}

void ImportToSingleModel( CString &File, bool miniMesh )
{
#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif

#define miniQuality ( \
	aiProcess_JoinIdenticalVertices			|  \
	aiProcess_Triangulate					|  \
	aiProcess_FindDegenerates               |  \
	aiProcess_FindInvalidData               |  \
	0 )

  const aiScene *Scene = aiImportFile( File.GetPointer(), miniMesh? miniQuality : aiProcessPreset_TargetRealtime_MaxQuality );

  if ( !Scene )
  {
    LOG_ERR( "[assimp] Import error: %s", aiGetErrorString() );
#ifdef MEMORY_TRACKING
    memTracker.SetMissingIgnore( false );
#endif
    return;
  }

  CArray<CphxModel_Tool*> Models;

  CphxModel_Tool *sc = Project.CreateModel();

  D3DXMATRIX m;
  //D3DXMatrixIdentity( &m );
  D3DXMatrixScaling( &m, sceneImportScale, sceneImportScale, sceneImportScale );

  ImportSceneHierarchy_Model( Scene, Scene->mRootNode, sc, m, miniMesh );

  aiReleaseImport( Scene );

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif
}

void ImportRIP( CString &File )
{
  CStreamReaderFile f;
  f.Open( File.GetPointer() );

  CphxModel_Tool *model = Project.GetModelByIndex( 0 );

  if ( !model )
    model = Project.CreateModel();

  model->SetName( File );

  int sig = f.ReadDWord();
  int ver = f.ReadDWord();
  int numFaces = f.ReadDWord();
  int numVertices = f.ReadDWord();
  int vxSize = f.ReadDWord();
  int numTextures = f.ReadDWord();
  int numShaders = f.ReadDWord();
  int numVertexAttribs = f.ReadDWord();

  CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)model->AddPrimitive( Mesh_Stored );
  mesh->SetName( File );

  CArray<int> vertexAttribTypes;

  for ( TS32 x = 0; x < numVertexAttribs; x++ )
  {
    CString attrib = f.ReadASCIIZ();
    int index = f.ReadDWord();
    int offset = f.ReadDWord();
    int size = f.ReadDWord();
    int typeMap = f.ReadDWord();
    for ( int y = 0; y < typeMap; y++ )
    {
      unsigned int typ = f.ReadDWord();
      vertexAttribTypes.Add( typ );
    }
  }

  for ( TS32 x = 0; x < numTextures; x++ )
  {
    CString attrib = f.ReadASCIIZ();
    int z = 0;
  }

  for ( TS32 x = 0; x < numShaders; x++ )
  {
    CString attrib = f.ReadASCIIZ();
    int z = 0;
  }

  TS32 *faces = new TS32[ numFaces * 3 ];
  TS32 cnt = 0;

  for ( TS32 x = 0; x < numFaces; x++ )
  {
    int a = f.ReadDWord();
    int b = f.ReadDWord();
    int c = f.ReadDWord();

    if ( a != b && b != c && a != c )
    {
      faces[ cnt ] = a; cnt++;
      faces[ cnt ] = b; cnt++;
      faces[ cnt ] = c; cnt++;
    }
  }

  numFaces = cnt / 3;

  mesh->GetModelObject()->StoredVertexCount = numVertices;
  mesh->GetModelObject()->StoredVertices = new CphxVertex[ numVertices ];

  for ( TS32 x = 0; x < numVertices; x++ )
  {
    CphxVertex &vx = mesh->GetModelObject()->StoredVertices[ x ];
    for ( int y = 0; y < vertexAttribTypes.NumItems(); y++ )
    {
      unsigned int in = f.ReadDWord();
      vx.Position[ y ] = *( (float*)&in );
    }

    vx.Position = D3DXVECTOR3( vx.Position.x, -vx.Position.y, -vx.Position.z );

  }

  mesh->GetModelObject()->StoredPolyCount = numFaces;
  mesh->GetModelObject()->StoredPolygons = new CphxPolygon[ numFaces ];

  CphxPolygon *pl = mesh->GetModelObject()->StoredPolygons;

  cnt = 0;
  for ( TS32 x = 0; x < numFaces; x++ )
  {
    CphxPolygon &pl = mesh->GetModelObject()->StoredPolygons[ x ];
    pl.VertexCount = 3;
    pl.VertexIDs[ 0 ] = faces[ cnt ]; cnt++;
    pl.VertexIDs[ 1 ] = faces[ cnt ]; cnt++;
    pl.VertexIDs[ 2 ] = faces[ cnt ]; cnt++;
  }

  SAFEDELETEA( faces );

  //mesh->GetModelObject()->StoredVertexCount = Mesh->mNumVertices;
  //mesh->GetModelObject()->StoredVertices = new CphxVertex[Mesh->mNumVertices];
  //
  //CphxVertex *vx = mesh->GetModelObject()->StoredVertices;
  //
  //for (TU32 y = 0; y < Mesh->mNumVertices; y++)
  //{
  //	vx[y].Position.x = Mesh->mVertices[y].x;
  //	vx[y].Position.y = Mesh->mVertices[y].y;
  //	vx[y].Position.z = Mesh->mVertices[y].z;
  //
  //	vx[y].Normal.x = Mesh->mNormals[y].x;
  //	vx[y].Normal.y = Mesh->mNormals[y].y;
  //	vx[y].Normal.z = Mesh->mNormals[y].z;
  //}

  //#ifdef MEMORY_TRACKING
  //	memTracker.SetMissingIgnore(true);
  //#endif
  //
  //	const aiScene *Scene=aiImportFile(File.GetPointer(), aiProcessPreset_TargetRealtime_MaxQuality);
  //
  //	if (!Scene)
  //	{
  //		LOG_ERR("[assimp] Import error: %s",aiGetErrorString());
  //#ifdef MEMORY_TRACKING
  //		memTracker.SetMissingIgnore(false);
  //#endif
  //		return;
  //	}
  //
  //	CArray<CphxModel_Tool*> Models;
  //
  //	//import meshes
  //	for (TU32 x = 0; x < Scene->mNumMeshes; x++)
  //	{
  //		const aiMesh* Mesh = Scene->mMeshes[x];
  //
  //		CphxModel_Tool *model = Project.CreateModel();
  //		Models += model;
  //		model->SetName(CString(Mesh->mName.data));
  //
  //		if (!model->GetName().Length()) model->SetName(CString::Format(_T("Mesh %3d"),x));
  //
  //		CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)model->AddPrimitive(Mesh_Stored);
  //		mesh->GetModelObject()->StoredVertexCount = Mesh->mNumVertices;
  //		mesh->GetModelObject()->StoredVertices = new CphxVertex[Mesh->mNumVertices];
  //
  //		CphxVertex *vx = mesh->GetModelObject()->StoredVertices;
  //
  //		for (TU32 y = 0; y < Mesh->mNumVertices; y++)
  //		{
  //			vx[y].Position.x = Mesh->mVertices[y].x;
  //			vx[y].Position.y = Mesh->mVertices[y].y;
  //			vx[y].Position.z = Mesh->mVertices[y].z;
  //
  //			vx[y].Normal.x = Mesh->mNormals[y].x;
  //			vx[y].Normal.y = Mesh->mNormals[y].y;
  //			vx[y].Normal.z = Mesh->mNormals[y].z;
  //		}
  //
  //		mesh->GetModelObject()->StoredPolyCount = 0;// Mesh->mNumFaces;
  //		mesh->GetModelObject()->StoredPolygons = new CphxPolygon[Mesh->mNumFaces];
  //
  //		CphxPolygon *pl = mesh->GetModelObject()->StoredPolygons;
  //
  //		for (TU32 y = 0; y < Mesh->mNumFaces; y++)
  //		{
  //			if (Mesh->mFaces[y].mNumIndices <= 4 && Mesh->mFaces[y].mNumIndices>2)
  //			{
  //				mesh->GetModelObject()->StoredPolyCount++;
  //				pl[y].VertexCount = Mesh->mFaces[y].mNumIndices;
  //				for (TU32 z = 0; z < Mesh->mFaces[y].mNumIndices; z++)
  //				{
  //					pl[y].VertexIDs[z] = Mesh->mFaces[y].mIndices[z];
  //					pl[y].Normals[z].x = Mesh->mNormals[pl[y].VertexIDs[z]].x;
  //					pl[y].Normals[z].y = Mesh->mNormals[pl[y].VertexIDs[z]].y;
  //					pl[y].Normals[z].z = Mesh->mNormals[pl[y].VertexIDs[z]].z;
  //				}
  //
  //			}
  //			else
  //				LOG_ERR("[assimp] Face %d in object %s has more than 4 vertices!",y,Mesh->mName.data);
  //		}
  //	}
  //
  //	CphxScene_Tool *sc = Project.CreateScene();
  //	sc->AddClip();
  //
  //	ImportSceneHierarchy(Scene->mRootNode, sc, NULL, Models);
  //
  //	aiReleaseImport(Scene);
  //
  //#ifdef MEMORY_TRACKING
  //	memTracker.SetMissingIgnore(false);
  //#endif
}

#else

void ImportWithAssimp( CString& File )
{

}

void ImportRIP( CString& File )
{

}

void ImportToSingleModel( CString& File, bool miniMesh )
{

}

#endif