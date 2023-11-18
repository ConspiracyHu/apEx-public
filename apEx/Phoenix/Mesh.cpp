#include "Mesh.h"
#include "phxMath.h"
//#include "flareMath.h"
//#include "flareConfig.h"
#include "../LibCTiny/libcminimal.h"
#include "Model.h"

static const float vecdata[] = { 0, 0, 0, 1, 0, 0, 0 };

#define nullvector3 (*(D3DXVECTOR3*)vecdata)
#define nullvector2 (*(D3DXVECTOR2*)vecdata)
#define unitquaternion (*(D3DXQUATERNION*)vecdata)
#define zvector (*(D3DXVECTOR3*)(vecdata+1))
#define yvector (*(D3DXVECTOR3*)(vecdata+2))
#define xvector4 (*(D3DXVECTOR4*)(vecdata+3))

void D3DXVec3Transform3( D3DXVECTOR3 *out, D3DXVECTOR3 *in, D3DXMATRIX *m )
{
  D3DXVECTOR4 v;
  D3DXVec3Transform( &v, in, m );
  *out = *(D3DXVECTOR3*)&v;
}

CphxVertex::CphxVertex()
{
  for ( int x = 0; x < 4; x++ ) Color[ x ] = 0;
  //#ifdef COMPILED_FOR_TOOL
  ColorCount = 0;
  //isSplit = 0;
  StoredPosition = Normal = Position = Position2 = nullvector3;
  //#endif
}

CphxVertex::CphxVertex( const CphxVertex &v )
{
  *this = v;
}

CphxVertex CphxVertex::operator+( const CphxVertex &e )
{
  CphxVertex r;
  r.Position = Position + e.Position;
  r.Position2 = Position2 + e.Position2;
  for ( int x = 0; x < 4; x++ )
    r.Color[ x ] = Color[ x ] + e.Color[ x ];
  //r.Texcoord=Texcoord+e.Texcoord;
  return r;
}

CphxVertex CphxVertex::operator/( const float &v )
{
  CphxVertex r;
  r.Position = Position / v;
  r.Position2 = Position2 / v;
  for ( int x = 0; x < 4; x++ )
    r.Color[ x ] = Color[ x ] / v;
  //r.Texcoord=Texcoord/v;
  return r;
}

void CphxVertex::operator=( const CphxVertex &v )
{
  Position = v.Position;
  Position2 = v.Position2;
  for ( int x = 0; x < 4; x++ )
    Color[ x ] = v.Color[ x ];
  //Texcoord=v.Texcoord;
  //for (int x=0; x<v.EdgeIDs.NumItems(); x++)
  //	EdgeIDs+=v.EdgeIDs[x][x];
  EdgeIDs = v.EdgeIDs;
  ColorCount = v.ColorCount;
  StoredPosition = v.StoredPosition;
  Normal = v.Normal;
  //isSplit = v.isSplit;
}

__inline int CphxEdge::GetOtherPoly( int p )
{
  return PolyIDs[ 0 ] == p ? PolyIDs[ 1 ] : PolyIDs[ 0 ];
}

void CphxPolygon::AddToVertexNormal( int VertexID, D3DXVECTOR3 &N )
{
  for ( int x = 0; x < VertexCount; x++ )
    if ( VertexIDs[ x ] == VertexID )
      Normals[ x ] += N;
}

void CphxMesh::AddVertex( const float *v )
{
  CphxVertex vx;
  vx.Position = *(D3DXVECTOR3*)v;
  vx.Position2 = *(D3DXVECTOR3*)v;
  Vertices.Add( vx );
}

void CphxMesh::AddVertex( const float x, const float y, const float z )
{
  float f[ 3 ] = { x, y, z };
  AddVertex( f );
}

void CphxMesh::AddPolygon( int a, int b, int c )
{
  AddPolygon( a, b, c, c, nullvector2, nullvector2, nullvector2, nullvector2 );
}

void CphxMesh::AddPolygon( int a, int b, int c, int d )
{
  AddPolygon( a, b, c, d, nullvector2, nullvector2, nullvector2, nullvector2 );
}

void CphxMesh::AddPolygon( int a, int b, int c, int d, const float *i, const float *j, const float *k, const float *l )
{
  CphxPolygon p;
  p.TouchedByNormalCalculator = false;
  p.VertexCount = c == d ? 3 : 4; //size hack so this single function is used to add polygons
  p.VertexIDs[ 0 ] = a;
  p.VertexIDs[ 1 ] = b;
  p.VertexIDs[ 2 ] = c;
  p.VertexIDs[ 3 ] = d;
  for ( int x = 0; x < 4; x++ )
  {
    p.Texcoords[ 0 ][ x ] = i;
    p.Texcoords[ 1 ][ x ] = j;
    p.Texcoords[ 2 ][ x ] = k;
    p.Texcoords[ 3 ][ x ] = l;
  }
  Polygons.Add( p );
}

//int CphxMesh::FindEdge(int v1, int v2)
//{
//	for (int x = 0; x < Edges.NumItems(); x++)
//	{
//		if ((Edges[x].VertexIDs[0] == v1 && Edges[x].VertexIDs[1] == v2) ||
//			(Edges[x].VertexIDs[0] == v2 && Edges[x].VertexIDs[1] == v1))
//			return x;
//	}
//	return -1;
//}

int _cdecl EdgeSorter( const void *e1, const void *e2 )
{
  CphxEdgeSortStruct *a = (CphxEdgeSortStruct*)e1;
  CphxEdgeSortStruct *b = (CphxEdgeSortStruct*)e2;

  if ( a->Vx1 != b->Vx1 ) return a->Vx1 - b->Vx1;
  return a->Vx2 - b->Vx2;
}

void CphxMesh::RebuildEdgeList( bool BuildVertexEdgeList )
{
  //reset everything
  Edges.FlushFast();
  for ( int x = 0; x < Vertices.NumItems(); x++ )
    Vertices[ x ].EdgeIDs.FlushFast();

  int EdgeCount = 0;

  for ( int x = 0; x < Polygons.NumItems(); x++ )
    EdgeCount += Polygons[ x ].VertexCount;

  CphxEdgeSortStruct *SortArray = new CphxEdgeSortStruct[ EdgeCount ];
  int Position = 0;
  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];
    for ( int y = 0; y < p->VertexCount; y++ )
    {
      int v1 = p->VertexIDs[ y ];
      int v2 = p->VertexIDs[ ( y + 1 ) % p->VertexCount ];
      SortArray[ Position ].Vx1 = min( v1, v2 );
      SortArray[ Position ].Vx2 = max( v1, v2 );
      SortArray[ Position ].SourcePoly = x;
      SortArray[ Position ].SourceVertex = y;
      Position++;
    }
  }

  qsort( SortArray, EdgeCount, sizeof( CphxEdgeSortStruct ), EdgeSorter );

  int LastVx1 = -1;
  int LastVx2 = -1;

  for ( int x = 0; x < EdgeCount; x++ )
  {
    CphxEdgeSortStruct *s = SortArray + x;

    //duplicate edge found
    if ( s->Vx1 == LastVx1 && s->Vx2 == LastVx2 )
    {
      Edges[ Edges.NumItems() - 1 ].PolyIDs[ 1 ] = s->SourcePoly;
      Polygons[ s->SourcePoly ].EdgeIDs[ s->SourceVertex ] = Edges.NumItems() - 1;
      continue;
    }

    //new edge found, add it
    CphxEdge e;
    LastVx1 = e.VertexIDs[ 0 ] = s->Vx1;
    LastVx2 = e.VertexIDs[ 1 ] = s->Vx2;
    e.PolyIDs[ 0 ] = s->SourcePoly;
    e.PolyIDs[ 1 ] = -1;
    e.NewVertexIDs[ 0 ][ 0 ] = e.NewVertexIDs[ 0 ][ 1 ] = e.NewVertexIDs[ 1 ][ 0 ] = e.NewVertexIDs[ 1 ][ 1 ] = -1;
    Polygons[ s->SourcePoly ].EdgeIDs[ s->SourceVertex ] = Edges.NumItems();
    Edges.Add( e );
  }

  delete[] SortArray;

  //for (int x=0; x<Polygons.NumItems(); x++)
  //{
  //	flarePolygon *p=&Polygons[x];
  //	for (int y=0; y<p->VertexCount; y++)
  //	{
  //		int v1=p->VertexIDs[y];
  //		int v2=p->VertexIDs[(y+1)%p->VertexCount];

  //		int EdgeID=FindEdge(v1,v2);

  //		if (EdgeID<0)
  //		{
  //			//new edge
  //			flareEdge e;
  //			e.VertexIDs[0]=v1;
  //			e.VertexIDs[1]=v2;
  //			e.PolyIDs[0]=x;
  //			e.PolyIDs[1]=-1;
  //			e.NewVertexIDs[0][0]=e.NewVertexIDs[0][1]=e.NewVertexIDs[1][0]=e.NewVertexIDs[1][1]=-1;

  //			EdgeID=Edges.NumItems();
  //			//add to pool
  //			Edges.Add(e);
  //		}
  //		else
  //		{
  //			//add polygon to existing edge
  //			Edges[EdgeID].PolyIDs[1]=x;
  //		}

  //		p->EdgeIDs[y]=EdgeID;
  //	}
  //}

  if ( BuildVertexEdgeList )
    for ( int x = 0; x < Edges.NumItems(); x++ )
    {
      Vertices[ Edges[ x ].VertexIDs[ 0 ] ].EdgeIDs.Add( x );
      Vertices[ Edges[ x ].VertexIDs[ 1 ] ].EdgeIDs.Add( x );
    }
}

D3DXVECTOR3 CphxMesh::CalculatePolyNormal( int pl )
{
  D3DXVECTOR3 Normal = nullvector3;
  CphxPolygon *p = &Polygons[ pl ];

  //this is an edge:
  if ( p->VertexCount < 3 ) return Normal;

  //n-poly: (triangles could be a special case but this handles them just as well, - with three times the performance cost but who cares)
  for ( int x = 0; x < p->VertexCount; x++ )
  {
    D3DXVECTOR3 &_a = Vertices[ p->VertexIDs[ ( x + 1 ) % p->VertexCount ] ].Position;
    D3DXVECTOR3 &_b = Vertices[ p->VertexIDs[ x ] ].Position;
    D3DXVECTOR3 &_c = Vertices[ p->VertexIDs[ ( x + 2 ) % p->VertexCount ] ].Position;

    D3DXVECTOR3 a;
    D3DXVECTOR3 b;
    D3DXVec3Normalize( &a, &( _a - _b ) );
    D3DXVec3Normalize( &b, &( _a - _c ) );
    D3DXVec3Cross( &a, &b, &a );
    //D3DXVec3Cross( &a, &( _a - _c ), &( _a - _b ) );

    //this normalization might not be needed
    //D3DXVec3Normalize(&a,&a);

    Normal += a;
  }

  //this normalization might not be needed
  //D3DXVec3Normalize(&Normal,&Normal);
  return Normal;
}

void CphxMesh::CalculatePolyVertexNormal( int poly, int VertexID, D3DXVECTOR3 &Vector )
{
  CphxPolygon *p = &Polygons[ poly ];

  Vector += p->PolyNormal;

  p->TouchedByNormalCalculator = true;

  for ( int x = 0; x < p->VertexCount; x++ )
  {
    CphxEdge &edge = Edges[ p->EdgeIDs[ x ] ];

    //check if edge has the vertex we seek
    if ( edge.VertexIDs[ 0 ] == VertexID || edge.VertexIDs[ 1 ] == VertexID )
    {
      //if it does check if the other poly exists or has been touched already

      int otherpoly = edge.GetOtherPoly( poly );
      if ( otherpoly >= 0 && !Polygons[ otherpoly ].TouchedByNormalCalculator )
      {
        D3DXVECTOR3 v1, v2;
        //if everything is ok, check for edge hardness
        if ( 1 - D3DXVec3Dot( D3DXVec3Normalize( &v1, &p->PolyNormal ), D3DXVec3Normalize( &v2, &Polygons[ otherpoly ].PolyNormal ) ) <= SmoothGroupSeparation )
        {
          //soft edge
          CalculatePolyVertexNormal( otherpoly, VertexID, Vector );
        }
      }
    }
  }
}

void CphxMesh::CalculateNormals()
{
  RebuildEdgeList();

  if ( SkipNormalCalculation ) return;

  for ( int x = 0; x < Vertices.NumItems(); x++ )
    Vertices[ x ].Normal = nullvector3;

  //calculate and reset face normals
  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    Polygons[ x ].PolyNormal = CalculatePolyNormal( x );
    for ( int y = 0; y < Polygons[ x ].VertexCount; y++ )
      Vertices[ Polygons[ x ].VertexIDs[ y ] ].Normal += Polygons[ x ].PolyNormal;
  }

  //for ( int x = 0; x < Polygons.NumItems(); x++ )
  //  D3DXVec3Normalize( &Polygons[ x ].PolyNormal, &Polygons[ x ].PolyNormal );

  for ( int x = 0; x < Vertices.NumItems(); x++ )
    D3DXVec3Normalize( &Vertices[ x ].Normal, &Vertices[ x ].Normal );

  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];

    //if edge is soft we need to take the normal from there and add it to the two vertexnormals on this end
    for ( int y = 0; y < p->VertexCount; y++ )
    {
      if ( SmoothGroupSeparation == 0 )
      {
        p->Normals[ y ] = Polygons[ x ].PolyNormal; //flat shade
      }
      else
        if ( SmoothGroupSeparation < 2 )
        {
          for ( int z = 0; z < Polygons.NumItems(); z++ )
            Polygons[ z ].TouchedByNormalCalculator = false;

          p->Normals[ y ] = nullvector3;
          CalculatePolyVertexNormal( x, p->VertexIDs[ y ], p->Normals[ y ] );
          D3DXVec3Normalize( &p->Normals[ y ], &p->Normals[ y ] );
        }
        else
          p->Normals[ y ] = Vertices[ p->VertexIDs[ y ] ].Normal; //gouraud shade
    }
  }
}

void CphxMesh::BuildMesh( ID3D11Buffer *&VxBuffer, ID3D11Buffer *&TriIndexBuffer, ID3D11Buffer *&WireIndexBuffer, int &VxCount, int &TriCount, int &EdgeCount )
{
  CalculateNormals();

  //this is here to make sure line objects don't crash:
  if ( !Polygons.NumItems() ) return;

  VxCount = TriCount = 0;
  EdgeCount = 0;
  int VxPos = 0;

  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];
    VxCount += p->VertexCount;
    TriCount += p->VertexCount - 2;
  }

  PHXVERTEXDATA *LockedVertices = new PHXVERTEXDATA[ VxCount ];
  int *LockedIndices = new int[ TriCount * 3 ];
  int *LockedWireIndices = new int[ VxCount * 2 ];
  PHXVERTEXDATA *lv = LockedVertices;
  int *li = LockedIndices;
  int *lwi = LockedWireIndices;

  //flareDevice->CreateVertexBuffer(VxCount*FLAREVERTEXFORMATSIZE,D3DUSAGE_WRITEONLY,FLAREVERTEXFORMAT,D3DPOOL_MANAGED,&VxBuffer,NULL);
  //flareDevice->CreateIndexBuffer(TriCount*3*sizeof(int),D3DUSAGE_WRITEONLY,D3DFMT_INDEX32,D3DPOOL_MANAGED,&TriIndexBuffer,NULL);
  //flareDevice->CreateIndexBuffer(VxCount*2*sizeof(int),D3DUSAGE_WRITEONLY,D3DFMT_INDEX32,D3DPOOL_MANAGED,&WireIndexBuffer,NULL);
  //WireIndexBuffer->Lock(0,VxCount*2*sizeof(int),(void**)&LockedWireIndices,0);
  //VxBuffer->Lock(0,VxCount*FLAREVERTEXFORMATSIZE,(void**)&LockedVertices,0);
  //TriIndexBuffer->Lock(0,TriCount*3*sizeof(int),(void**)&LockedIndices,0);

  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];
    //if (Polygons[x].VertexCount>2)
    for ( int y = 0; y < p->VertexCount; y++ )
    {
      unsigned char Color[ 4 ];
      for ( int z = 0; z < 4; z++ )
        Color[ z ] = (unsigned char)max( 0, min( 255, Vertices[ p->VertexIDs[ y ] ].Color[ z ] * 255 ) );

      LockedVertices->Position = Vertices[ p->VertexIDs[ y ] ].Position;
      LockedVertices->Position2 = Vertices[ p->VertexIDs[ y ] ].Position2;
      LockedVertices->Normal = p->Normals[ y ];
      LockedVertices->Color = ( (unsigned int*)Color )[ 0 ];
      memcpy( LockedVertices->Texcoord, p->Texcoords[ y ], sizeof( D3DXVECTOR2 ) * 4 );

      //memcpy(LockedVertices, Vertices[p->VertexIDs[y]].Position, sizeof(D3DXVECTOR3));
      //LockedVertices += 3;
      //memcpy(LockedVertices, p->Normals[y], sizeof(D3DXVECTOR3));
      //LockedVertices += 3;

      //memcpy(LockedVertices, Color, 4);
      //LockedVertices += 1;

      //memcpy(LockedVertices, p->Texcoords[y], sizeof(D3DXVECTOR2)* 4);
      //LockedVertices += 8;

      LockedVertices++;
    }
  }

  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];
    //if (p->VertexCount>2)
    {
      LockedIndices[ 0 ] = VxPos;
      LockedIndices[ 1 ] = VxPos + 2;
      LockedIndices[ 2 ] = VxPos + 1;
      LockedIndices += 3;

      //forced one way triangulation of quads
      if ( p->VertexCount == 4 )
      {
        LockedIndices[ 0 ] = VxPos;
        LockedIndices[ 1 ] = VxPos + 3;
        LockedIndices[ 2 ] = VxPos + 2;
        LockedIndices += 3;
      }

      //build wireframe
      for ( int y = 0; y < p->VertexCount; y++ )
        if ( Edges[ p->EdgeIDs[ y ] ].PolyIDs[ 0 ] == x )
        {
          LockedWireIndices[ 0 ] = VxPos + y;
          LockedWireIndices[ 1 ] = VxPos + ( y + 1 ) % p->VertexCount;
          LockedWireIndices += 2;
          EdgeCount++;
        }

      VxPos += p->VertexCount;
    }
  }

  D3D11_BUFFER_DESC bd = { (unsigned char*)LockedVertices - (unsigned char*)lv, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
  D3D11_SUBRESOURCE_DATA bufData = { lv, 0, 0 };
  //memset(&bd, 0, sizeof(bd));
  //memset(&bufData, 0, sizeof(bufData));

  //bd.Usage = D3D11_USAGE_DEFAULT;
  //bd.ByteWidth = (unsigned char*)LockedVertices - (unsigned char*)lv;
  //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  //bufData.pSysMem = lv;

#ifdef DEBUGINTOFILE
  HRESULT res =
#endif
    phxDev->CreateBuffer( &bd, &bufData, &VxBuffer );
#ifdef DEBUGINTOFILE
  if ( res != S_OK )
    DEBUGLOG( "*** Failed to create vertex buffer of size %d", bd.ByteWidth );
#endif

  bd.ByteWidth = (unsigned char*)LockedIndices - (unsigned char*)li;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufData.pSysMem = li;

#ifdef DEBUGINTOFILE
  res =
#endif
    phxDev->CreateBuffer( &bd, &bufData, &TriIndexBuffer );
#ifdef DEBUGINTOFILE
  if ( res != S_OK )
    DEBUGLOG( "*** Failed to create index buffer of size %d", bd.ByteWidth );
#endif

  bd.ByteWidth = (unsigned char*)LockedWireIndices - (unsigned char*)lwi;
  //bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bufData.pSysMem = lwi;

#ifdef DEBUGINTOFILE
  res =
#endif
    phxDev->CreateBuffer( &bd, &bufData, &WireIndexBuffer );
#ifdef DEBUGINTOFILE
  if ( res != S_OK )
    DEBUGLOG( "*** Failed to create wire index buffer of size %d", bd.ByteWidth );
#endif

  delete[] lv;
  delete[] li;
  delete[] lwi;


  //VxBuffer->Unlock();
  //TriIndexBuffer->Unlock();
  //WireIndexBuffer->Unlock();
}

#ifdef PHX_MESH_SMOOTH
void CphxMesh::CatmullClark( bool Linear, int Iterations )
{
  for ( int i = 0; i < Iterations; i++ )
  {
    RebuildEdgeList( true );

    int OriginalVertexCount = Vertices.NumItems();
    int OriginalPolyCount = Polygons.NumItems();

    //calculate and add face centroids
    for ( int x = 0; x < OriginalPolyCount; x++ )
    {
      CphxPolygon *p = &Polygons[ x ];
      CphxVertex v = Vertices[ p->VertexIDs[ 0 ] ];

      for ( int y = 1; y < p->VertexCount; y++ )
        v = v + Vertices[ p->VertexIDs[ y ] ];

      v = v / (float)p->VertexCount;

      Vertices.Add( v );
    }

    //calculate and add edge points
    for ( int x = 0; x < Edges.NumItems(); x++ )
    {
      CphxEdge *e = &Edges[ x ];
      if ( !Linear && e->PolyIDs[ 0 ] != -1 && e->PolyIDs[ 1 ] != -1 )
        Vertices.Add( ( Vertices[ e->VertexIDs[ 0 ] ] +
                        Vertices[ e->VertexIDs[ 1 ] ] +
                        Vertices[ e->PolyIDs[ 0 ] + OriginalVertexCount ] +
                        Vertices[ e->PolyIDs[ 1 ] + OriginalVertexCount ] ) / 4.0f );
      else
        Vertices.Add( ( Vertices[ e->VertexIDs[ 0 ] ] +
                        Vertices[ e->VertexIDs[ 1 ] ] ) / 2.0f );
    }

    //store positions
    for ( int x = 0; x < Vertices.NumItems(); x++ )
    {
      CphxVertex *v = &Vertices[ x ]; //size optim
      v->StoredPosition = v->Position;
    }

    //calculate new vertex positions
    if ( !Linear )
      for ( int x = 0; x < OriginalVertexCount; x++ )
      {
        CphxVertex *v = &Vertices[ x ];
        int EdgeCount = v->EdgeIDs.NumItems();

        D3DXVECTOR3 R2F = nullvector3;
        int edgeCnt = 0;

        bool skip = false;

        for ( int y = 0; y < EdgeCount; y++ )
        {
          CphxEdge *e = &Edges[ v->EdgeIDs[ y ] ];
          if ( e->PolyIDs[ 0 ] == -1 || e->PolyIDs[ 1 ] == -1 )
          {
            skip = true;
            continue;
          }

          edgeCnt++;

          R2F += 0.5*( Vertices[ e->PolyIDs[ 0 ] + OriginalVertexCount ].StoredPosition +
                       Vertices[ e->PolyIDs[ 1 ] + OriginalVertexCount ].StoredPosition ) + //face points (F), but each face is added twice -> we need a 0.5 multiplier
                       ( Vertices[ e->VertexIDs[ 0 ] ].StoredPosition +
                         Vertices[ e->VertexIDs[ 1 ] ].StoredPosition ); //edge midpoints are added twice here but that's what we need (R*2)
        }

        if ( skip )
          continue;

        R2F /= (float)edgeCnt;

        v->Position2 = v->Position = ( R2F + ( edgeCnt - 3.0f )*v->StoredPosition ) / (float)edgeCnt;
      }

    //recalculate topology and add new quads
    for ( int x = 0; x < OriginalPolyCount; x++ )
    {
      int PolyVertexCount = Polygons[ x ].VertexCount;
      for ( int y = 0; y < PolyVertexCount; y++ )
      {
        //this needs to be here because the pointer can change when new polys are added
        CphxPolygon *pl = &Polygons[ x ];

        //code reorganized for size optimization
        CphxPolygon p;
        int m = ( y + PolyVertexCount - 1 ) % PolyVertexCount;
        p.VertexIDs[ 0 ] = OriginalVertexCount + x; //central vertex added for this poly
        p.VertexIDs[ 1 ] = OriginalVertexCount + OriginalPolyCount + pl->EdgeIDs[ m ]; //new vertex added for edge[y-1]
        p.VertexIDs[ 3 ] = OriginalVertexCount + OriginalPolyCount + pl->EdgeIDs[ y ]; //new vertex added for edge[y]


        p.VertexIDs[ 2 ] = pl->VertexIDs[ y ];

        for ( int j = 0; j < 4; j++ )
        {
          p.Texcoords[ 0 ][ j ] = nullvector2;
          for ( int z = 0; z < pl->VertexCount; z++ )
            p.Texcoords[ 0 ][ j ] += pl->Texcoords[ z ][ j ];
          p.Texcoords[ 0 ][ j ] /= (float)pl->VertexCount;
          p.Texcoords[ 1 ][ j ] = ( pl->Texcoords[ m ][ j ] + pl->Texcoords[ y ][ j ] ) / 2.0f;
          p.Texcoords[ 2 ][ j ] = pl->Texcoords[ y ][ j ];
          p.Texcoords[ 3 ][ j ] = ( pl->Texcoords[ y ][ j ] + pl->Texcoords[ ( y + 1 ) % PolyVertexCount ][ j ] ) / 2.0f;
        }


        p.VertexCount = 4;

        if ( y < PolyVertexCount - 1 )
          Polygons.Add( p ); //add new polygons as needed
        else
          Polygons[ x ] = p; //the last poly simply replaces the original one
      }
    }

    //normals and edge list will be fucked up at this point.
  }
}

#endif

#ifdef PHX_MESH_NORMALDEFORM
void CphxMesh::NormalDeform( float factor )
{
  CalculateNormals();

  for ( int x = 0; x < Vertices.NumItems(); x++ )
  {
    float col = Vertices[ x ].Color[ 0 ];
    Vertices[ x ].Position += Vertices[ x ].Normal * factor * col;
  }
}

#endif

#ifdef PHX_MESH_CUBE
static float CubeVertexData[] =
{
  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, 0.5f, -0.5f,
  -0.5f, 0.5f, -0.5f,
  0.5f, -0.5f, 0.5f,
  0.5f, 0.5f, 0.5f,
  -0.5f, 0.5f, 0.5f,
  -0.5f, -0.5f, 0.5f
};

static char CubePolyData[] =
{
  0, 1, 2, 3,
  1, 4, 5, 2,
  5, 6, 3, 2,
  5, 4, 7, 6,
  0, 7, 4, 1,
  0, 3, 6, 7
};

void CphxMesh::CreateCube()
{
  for ( int x = 0; x < 8; x++ )
    AddVertex( CubeVertexData + x * 3 );

  char *data = (char*)CubePolyData;
  for ( int y = 0; y < 6; y++ )
  {
    AddPolygon( data[ 3 ], data[ 2 ], data[ 1 ], data[ 0 ] );
    data += 4;
  }

  for ( int x = 0; x < 4; x++ )
    CalculateTextureCoordinates( flareTextureMap_Box, x );

}
#endif

void CphxMesh::AddCapsBuildCylindricalTopologyAndCalculateUV( float Cap1Y, float Cap2Y, int XRes, int YRes, bool BottomCap, bool TopCap, PHXTEXTUREMAPTYPE MapType )
{
  int vxc = Vertices.NumItems();

  AddVertex( 0, Cap1Y, 0 ); //moved here due to sizeoptim
  AddVertex( 0, Cap2Y, 0 );

  for ( int x = 0; x < XRes; x++ )
  {
    if ( TopCap )
      AddPolygon( vxc + 1, vxc - ( x + 1 ) % XRes - 1, vxc - x - 1 );
    if ( BottomCap )
      AddPolygon( vxc, ( x + 1 ) % XRes, x );
  }

  for ( int y = 0; y < YRes - 1; y++ )
    for ( int x = 0; x < XRes; x++ )
      AddPolygon( x + y*XRes,
      ( x + 1 ) % XRes + y*XRes,
                  ( x + 1 ) % XRes + ( ( y + 1 ) % YRes )*XRes,
                  x + ( ( y + 1 ) % YRes )*XRes );

  //this has been moved here due to size optimizations
  for ( int x = 0; x < 4; x++ )
    CalculateTextureCoordinates( MapType, x );
}

#ifdef PHX_MESH_SPHERE
void CphxMesh::CreateSphere( int XRes, int YRes, float TopCut, float BottomCut, bool Caps )
{
  int v1 = BottomCut > 0 ? 0 : 1;
  int v2 = TopCut < 1 ? 0 : 1;

  if ( YRes - v1 - v2 == 0 ) YRes++;

  for ( int y = v1; y < YRes - v2; y++ )
  {
    float theta = ( lerp( BottomCut, TopCut, ( y ) / (float)( YRes - 1 ) ) - 0.5f )*pi;
    for ( int x = 0; x < XRes; x++ )
    {
      float phi = x / (float)XRes * 2 * pi;
      AddVertex( cos( theta )*sin( phi )*0.5f, sin( theta )*0.5f, cos( theta )*cos( phi )*0.5f );
    }
  }

  AddCapsBuildCylindricalTopologyAndCalculateUV( sin( ( BottomCut - 0.5f )*pi )*0.5f, sin( ( TopCut - 0.5f )*pi )*0.5f, XRes, YRes - v1 - v2, Caps || v1 == 1, Caps || v2 == 1, flareTextureMap_Spherical );
}
#endif

#ifdef PHX_MESH_PLANE
void CphxMesh::CreatePlane( int XRes, int YRes )
{
  for ( int y = 0; y <= YRes; y++ )
    for ( int x = 0; x <= XRes; x++ )
      AddVertex( x / (float)XRes - 0.5f, 0, y / (float)YRes - 0.5f );

  for ( int y = 0; y < YRes; y++ )
    for ( int x = 0; x < XRes; x++ )
      AddPolygon( x + ( y + 1 )*( XRes + 1 ),
                  x + 1 + ( y + 1 )*( XRes + 1 ),
                  x + 1 + y   *( XRes + 1 ),
                  x + y   *( XRes + 1 ) );

  for ( int x = 0; x < 4; x++ )
    CalculateTextureCoordinates( flareTextureMap_Planar, x );
}
#endif

#ifdef PHX_MESH_CONE
void CphxMesh::CreateCone( int XRes, int YRes, float TopCut, bool Caps )
{
  int v2 = TopCut < 1 ? 1 : 0;

  if ( YRes + v2 == 0 ) YRes++;

  for ( int y = 0; y < YRes + v2; y++ )
    for ( int x = 0; x < XRes; x++ )
    {
      float h = y / (float)(YRes)*TopCut;
      float r = 1 - h;
      float t = x / (float)XRes * 2 * pi;
      AddVertex( sin( t )*0.5f*r, h - 0.5f, cos( t )*0.5f*r );
    }

  AddCapsBuildCylindricalTopologyAndCalculateUV( -0.5f, TopCut - 0.5f, XRes, YRes + v2, Caps, Caps || v2 == 0, flareTextureMap_Cylindrical );
}
#endif

#ifdef PHX_MESH_CYLINDER
void CphxMesh::CreateCylinder( int XRes, int YRes, bool Caps )
{
  for ( int y = 0; y <= YRes; y++ )
    for ( int x = 0; x < XRes; x++ )
    {
      float t = x / (float)XRes * 2 * pi;
      AddVertex( sin( t )*0.5f, y / (float)YRes - 0.5f, cos( t )*0.5f );
    }

  AddCapsBuildCylindricalTopologyAndCalculateUV( -0.5f, 0.5f, XRes, YRes + 1, Caps, Caps, flareTextureMap_Cylindrical );
}
#endif

#ifdef PHX_MESH_LINE
void CphxMesh::CreateLine( int Res )
{
  ArcStartDir = ArcEndDir = zvector;

  for ( int x = 0; x < Res; x++ )
  {
    AddVertex( 0, 0, -0.5f + x / (float)( Res - 1 ) );
  }

  //#ifdef COMPILED_FOR_TOOL

  //display polys for tool:
  for ( int x = 0; x < Res - 1; x++ )
  {
    AddPolygon( x, x + 1, x, x + 1 );
    CphxPolygon &p = Polygons[ x ];
    for ( int y = 0; y < 4; y++ )
    {
      p.Texcoords[ 0 ][ y ].x = x / (float)Res;
      p.Texcoords[ 1 ][ y ].x = ( x + 1 ) / (float)Res;
    }
  }

  //#endif
}
#endif

#ifdef PHX_MESH_ARC
void CphxMesh::CreateArc( int Res, float Degree, bool HaveLastSegment )
{
  int r = Res;
  if ( HaveLastSegment ) r++;

  ArcStartDir = zvector;
  float phase = ( r - 1 )*Degree * 2 * pi / Res;
  ArcEndDir = D3DXVECTOR3( -sin( phase ), 0, cos( phase ) );

  for ( int x = 0; x < r; x++ )
  {
    float phs = x*Degree * 2 * pi / Res;
    AddVertex( 0.5f*cos( phs ), 0, 0.5f*sin( phs ) );
  }

  //#ifdef COMPILED_FOR_TOOL

  //display polys for tool:
  for ( int x = 0; x < r - 1; x++ )
  {
    AddPolygon( x, x + 1, x, x + 1 );
    CphxPolygon &p = Polygons[ x ];
    for ( int y = 0; y < 4; y++ )
    {
      p.Texcoords[ 0 ][ y ].x = x / (float)r;
      p.Texcoords[ 1 ][ y ].x = ( x + 1 ) / (float)r;
    }
  }

  //#endif

}
#endif

#ifdef PHX_MESH_SPLINE
void CphxMesh::Spline( CphxMeshSplineKey *Keys, int KeyCount, int Resolution, bool Loop )
{
  int KeyCnt = KeyCount - 1;
  if ( Loop ) KeyCnt++;

  D3DXVec3Normalize( &ArcStartDir, &D3DXVECTOR3( Keys[ 0 ].Front ) );
  D3DXVec3Normalize( &ArcEndDir, &D3DXVECTOR3( Keys[ KeyCnt%KeyCount ].Back ) );

  for ( int x = 0; x < KeyCnt; x++ )
  {
    D3DXVECTOR3 k[ 4 ];
    k[ 0 ] = Keys[ x ].vx;
    k[ 1 ] = k[ 0 ] + D3DXVECTOR3( Keys[ x ].Front );
    k[ 3 ] = D3DXVECTOR3( Keys[ ( x + 1 ) % KeyCount ].vx );
    k[ 2 ] = k[ 3 ] + D3DXVECTOR3( Keys[ ( x + 1 ) % KeyCount ].Back );

    int cnt = Resolution;
    if ( !Loop && x == KeyCnt - 1 ) cnt++;

    for ( int y = 0; y < cnt; y++ )
    {
      AddVertex( bezier( k[ 0 ], k[ 1 ], k[ 2 ], k[ 3 ], y / (float)Resolution ) );
    }
  }

  //#ifdef COMPILED_FOR_TOOL

  //display polys for tool:
  for ( int x = 0; x < KeyCnt*Resolution; x++ )
    AddPolygon( x, ( x + 1 ) % ( KeyCnt*Resolution + ( Loop ? 0 : 1 ) ), x );

  //#endif

}
#endif

#ifdef PHX_MESH_LOFT
void CphxMesh::Loft( CphxMesh *Path, D3DXMATRIX *PathPos, CphxMesh *Slice, D3DXMATRIX *SlicePos, bool PathClosed, bool SliceClosed, float Rotation, float StartScale, float EndScale )
{
  int PathVxCount = Path->Vertices.NumItems();
  int SliceVxCount = Slice->Vertices.NumItems();

  D3DXVECTOR3 nx, /*ny, */dir, up;
  D3DXVECTOR4 v, va, vb, vc;

  up = yvector;

  D3DXMATRIX InvTransp;
  D3DXMatrixInverse( &InvTransp, NULL, PathPos );
  D3DXMatrixTranspose( &InvTransp, &InvTransp );

  for ( int z = 0; z < 101; z++ )
  {
    for ( int x = 0; x < PathVxCount; x++ )
    {
      D3DXVec3Transform( &va, &Path->Vertices[ ( x + PathVxCount - 1 ) % PathVxCount ].Position, PathPos );
      D3DXVec3Transform( &vb, &Path->Vertices[ x ].Position, PathPos );
      D3DXVec3Transform( &vc, &Path->Vertices[ ( x + 1 ) % PathVxCount ].Position, PathPos );
      D3DXVec3Normalize( &dir, ( D3DXVECTOR3* )&( vc - va ) );

      if ( z && ( !PathClosed && ( x == 0 || x == PathVxCount - 1 ) ) )
      {
        dir = Path->ArcEndDir;
        if ( x == 0 ) dir = Path->ArcStartDir;
        D3DXVec3Transform3( &dir, &dir, &InvTransp );
        //dir = *(D3DXVECTOR3*)&v;
      }

      D3DXVec3Normalize( &dir, &dir );

      D3DXVec3Cross( &nx, &up, &dir );
      D3DXVec3Normalize( &nx, &nx );
      D3DXVec3Cross( &up, &nx, &dir );
      up = up*-1;

      if ( z==100 )
      {
        D3DXMATRIX RotationMatrix;
        D3DXMatrixMultiply( &RotationMatrix, D3DXMatrixRotationAxis( &RotationMatrix, &up, x / (float)PathVxCount * 2 * pi*Rotation ), SlicePos );

        for ( int y = 0; y < SliceVxCount; y++ )
        {
          D3DXVec3Transform( &v, &Slice->Vertices[ y ].Position, &RotationMatrix );
          AddVertex( ( ( nx*v.x ) + ( up*v.z ) )*lerp( StartScale, EndScale, x / (float)( PathVxCount - 1 ) ) + *(D3DXVECTOR3*)&vb );
        }
      }
    }
  }

  for ( int x = 0; x < SliceVxCount - 1 + ( SliceClosed ? 1 : 0 ); x++ )
    for ( int y = 0; y < PathVxCount - 1 + ( PathClosed ? 1 : 0 ); y++ )
    {
      float u1 = x / (float)SliceVxCount;
      float v1 = y / (float)PathVxCount;
      float u2 = ( x + 1 ) / (float)SliceVxCount;
      float v2 = ( y + 1 ) / (float)PathVxCount;

      float a[ 2 ] = { u1, v2 };
      float b[ 2 ] = { u2, v2 };
      float c[ 2 ] = { u2, v1 };
      float d[ 2 ] = { u1, v1 };

      AddPolygon( y*SliceVxCount + x,
                  y*SliceVxCount + ( ( x + 1 ) % SliceVxCount ),
                  ( ( y + 1 ) % PathVxCount )*SliceVxCount + ( ( x + 1 ) % SliceVxCount ),
                  ( ( y + 1 ) % PathVxCount )*SliceVxCount + x,
                  d, c, b, a );
    }
}
#endif

#ifdef PHX_MESH_COPY
void CphxMesh::Copy( CphxMesh *a )
{
  D3DXMATRIX m;
  CopyInstance( a, D3DXMatrixIdentity( &m ) );
}
#endif

#ifdef PHX_MESH_MERGE
void CphxMesh::Merge( CphxModelObject_Mesh** objects, int count )
{
  for ( int x = 0; x < count; x++ )
    CopyInstance( &objects[ x ]->Mesh, &objects[ x ]->GetMatrix() );
}
#endif

void CphxMesh::CalculateTextureCoordinates( PHXTEXTUREMAPTYPE Type, int Channel )
{
  static float one[ 3 ] = { 1, 1, 1 };
  CalculateTextureCoordinates( Type, Channel, false, *(D3DXVECTOR3*)one, unitquaternion, nullvector3 );
}

void CphxMesh::CalculateTextureCoordinates( PHXTEXTUREMAPTYPE Type, int Channel, bool clipUV, D3DXVECTOR3 Scale, D3DXQUATERNION Rotate, D3DXVECTOR3 Translate )
{
  CalculateNormals();

  D3DXMATRIX Transform;
  D3DXMatrixInverse( &Transform, NULL, D3DXMatrixTransformation( &Transform, NULL, NULL, &Scale, NULL, &Rotate, &Translate ) );

  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];

    bool XOver0, XBelow0, ZBelow0, TangentNotCalculated;
    int BuggyTangent = 0;
    XOver0 = XBelow0 = ZBelow0 = TangentNotCalculated = false;

    //first pass calculates texture coordinates
    for ( int y = 0; y < p->VertexCount; y++ )
    {
      D3DXVECTOR4 vx;
      D3DXVECTOR3 vn;
      D3DXVec3Transform( &vx, &Vertices[ p->VertexIDs[ y ] ].Position, &Transform );
      D3DXVec3Normalize( &vn, (D3DXVECTOR3*)&vx );

      D3DXVECTOR4 pn = D3DXVECTOR4( p->PolyNormal.x, p->PolyNormal.y, p->PolyNormal.z, 0 );
      D3DXVec4Transform( &pn, &pn, &Transform );

      float _ax = fabs( pn.x );
      float _ay = fabs( pn.y );
      float _az = fabs( pn.z );
      float xd = pn.x / _ax;
      float yd = pn.y / _ay;
      float zd = pn.z / _az;

      float tanval = -atan2( vn.x, vn.z ) / pi / 2.0f + 0.5f;
      if ( Type == flareTextureMap_Spherical )
        if ( vn.x == 0 && vn.z == 0 )
        {
          tanval = 0;
          TangentNotCalculated = true;
          BuggyTangent = y;
        }

      switch ( Type )
      {
      case flareTextureMap_Planar:
        p->Texcoords[ y ][ Channel ] = D3DXVECTOR2( vx.x + 0.5f, vx.z + 0.5f );
        break;
      case flareTextureMap_Spherical:
        p->Texcoords[ y ][ Channel ] = D3DXVECTOR2( -tanval, acos( vn.y ) / pi );
        if ( vn.x >= 0 ) XOver0 = true; else XBelow0 = true;
        if ( vn.z < 0 ) ZBelow0 = true;
        break;
      case flareTextureMap_Cylindrical:
        p->Texcoords[ y ][ Channel ] = D3DXVECTOR2( -tanval, -vx.y + 0.5f );

        if ( !( _ay > _ax && _ay > _az ) )
        {
          if ( vn.x > 0 ) XOver0 = true; else XBelow0 = true;
          if ( vn.z < 0 ) ZBelow0 = true;
          break;
        }
        //top and bottom are box mapped for cylindrical, fall through

      case flareTextureMap_Box:
        p->Texcoords[ y ][ Channel ] = D3DXVECTOR2( vx.x*zd, -vx.y );

        if ( _ax > _ay && _ax > _az )
          p->Texcoords[ y ][ Channel ] = D3DXVECTOR2( -vx.z*xd, -vx.y );
        else
          if ( _ay > _ax && _ay > _az )
            p->Texcoords[ y ][ Channel ] = D3DXVECTOR2( vx.x*yd, vx.z );

        p->Texcoords[ y ][ Channel ] += D3DXVECTOR2( 0.5f, 0.5f );

        break;
      case flareTextureMap_TransformOriginal:
      {
        p->Texcoords[ y ][ Channel ].x = p->Texcoords[ y ][ Channel ].x*Scale.x + Translate.x;
        p->Texcoords[ y ][ Channel ].y = p->Texcoords[ y ][ Channel ].y*Scale.z + Translate.z;
      }
      break;
      }
    }

    //second pass corrects them
    for ( int y = 0; y < p->VertexCount; y++ )
    {
      D3DXVECTOR4 vx;
      D3DXVec3Transform( &vx, &Vertices[ p->VertexIDs[ y ] ].Position, &Transform );
      if ( XOver0 && XBelow0 && ZBelow0 && vx.x < 0 ) p->Texcoords[ y ][ Channel ].x += 1.0f;

#ifdef PHX_MESH_UV_HAS_CLIPPING
      if ( clipUV )
      {
        auto& uv = p->Texcoords[ y ][ Channel ];
        uv.x = max( 0, min( 1, uv.x ) );
        uv.y = max( 0, min( 1, uv.y ) );
      }
#endif
    }

    //third pass fixes poles
    if ( TangentNotCalculated ) //we have a buggy tangent
    {
      float xv = 0;
      for ( int i = 0; i < p->VertexCount; i++ )
        xv += p->Texcoords[ i ][ Channel ].x;
      p->Texcoords[ BuggyTangent ][ Channel ].x = xv / (float)( p->VertexCount - 1 );
    }

  }
}

int uvmodulo( int a, int b )
{
  return ( ( a%b ) + b ) % b;
}

#ifdef PHX_MESH_CALCULATETINT
void CphxMesh::CalculateTint( int TextureChannel, ID3D11Texture2D *SrcTexture, int Saturation )
{
  //first read back the texture data

  ID3D10Blob *Blob = NULL;
  D3DX11SaveTextureToMemory( phxContext, SrcTexture, D3DX11_IFF_DDS, &Blob, NULL );

  if ( !Blob ) return; //THIS SHOULD NEVER HAPPEN!

  struct DDSHEAD
  {
    unsigned int DDS;
    int dwSize;
    int dwFlags;
    int dwHeight;
    int dwWidth;
    int dwPitchOrLinearSize;
    int dwDepth;
    int dwMipMapCount;
    int dwReserved1[ 11 ];

    int _dwSize;
    int _dwFlags;
    int dwFourCC;
    int dwRGBBitCount;
    int dwRBitMask;
    int dwGBitMask;
    int dwBBitMask;
    int dwABitMask;

    int dwCaps;
    int dwCaps2;
    int dwCaps3;
    int dwCaps4;
    int dwReserved2;
  };

  unsigned char *Data = (unsigned char*)Blob->GetBufferPointer();
  DDSHEAD head;
  memcpy( &head, Data, sizeof( DDSHEAD ) );
  Data += 128;

  unsigned short *inimg = (unsigned short*)Data;

  //reset color info
  for ( int x = 0; x < Vertices.NumItems(); x++ )
  {
    for ( int y = 0; y < 4; y++ )
      Vertices[ x ].Color[ y ] = 0;
    Vertices[ x ].ColorCount = 0;
  }

  //accumulate vertex colors
  for ( int x = 0; x < Polygons.NumItems(); x++ )
    for ( int y = 0; y < Polygons[ x ].VertexCount; y++ )
    {
      D3DXVECTOR2 &uv = Polygons[ x ].Texcoords[ y ][ TextureChannel ];
      int p = ( uvmodulo( (int)( uv.x*head.dwWidth ), head.dwWidth ) + uvmodulo( (int)( uv.y*head.dwHeight ), head.dwHeight )*head.dwWidth ) * 4;

      for ( int z = 0; z < 4; z++ )
        Vertices[ Polygons[ x ].VertexIDs[ y ] ].Color[ z ] += ( (int)( inimg[ p + z ] / 256 ) ) / 256.0f; //this could be changed to simple /65536.0f but it'll change some old releases

      Vertices[ Polygons[ x ].VertexIDs[ y ] ].ColorCount++;
    }

  //calculate average
  for ( int x = 0; x < Vertices.NumItems(); x++ )
  {
    for ( int y = 0; y < 4; y++ )
    {
      Vertices[ x ].Color[ y ] /= (float)Vertices[ x ].ColorCount;
      Vertices[ x ].Color[ y ] = min( 1, Vertices[ x ].Color[ y ] * ( 1 + Saturation ) );
    }
  }

  Blob->Release();
}
#endif

#ifdef PHX_MESH_CALCULATETINTSHAPE
void CphxMesh::CalculateTintShape( int shape, int op, int power, D3DXVECTOR3 Scale, D3DXQUATERNION Rotate, D3DXVECTOR3 Translate )
{
  D3DXMATRIX m;
  D3DXMatrixTransformation( &m, NULL, NULL, &( D3DXVECTOR3( 1, 1, 1 ) + Scale ), NULL, &Rotate, &Translate );
  D3DXMatrixInverse( &m, nullptr, &m );

  for ( int x = 0; x < Vertices.NumItems(); x++ )
  {
    D3DXVECTOR4 pos;
    D3DXVec3Transform( &pos, &Vertices[ x ].Position, &m );
    pos /= pos.w;

    float p = float( power );

    float val;

    switch ( shape )
    {
    case 0: //box
      val = 1 - max( max( fabs( pos.x ), fabs( pos.y ) ), fabs( pos.z ) ) * 2.0f;
      break;
    case 1: //sphere
      val = 1 - D3DXVec3Length( (D3DXVECTOR3*)&pos ) * 2.0f;
      break;
    case 2: //plane
      val = pos.y;
      break;
    }

    val /= p / 16.0f;

    for ( int y = 0; y < 4; y++ )
    {
      switch ( op )
      {
      case 0:
        Vertices[ x ].Color[ y ] = val;
        break;
      case 1:
        Vertices[ x ].Color[ y ] += val;
        break;
      case 2:
        Vertices[ x ].Color[ y ] -= val;
        break;
      case 3:
        Vertices[ x ].Color[ y ] *= val;
        break;
      }

      if ( Vertices[ x ].Color[ y ] < 0 )
        Vertices[ x ].Color[ y ] = 0;
      if ( Vertices[ x ].Color[ y ] > 1 )
        Vertices[ x ].Color[ y ] = 1;

    }

  }

}
#endif

#ifdef PHX_MESH_MAPXFORM
void CphxMesh::MapXForm( D3DXVECTOR3 Scale, D3DXQUATERNION Rotate, D3DXVECTOR3 Translate )
{
  D3DXVECTOR3 Axis;
  float Angle;
  D3DXQuaternionToAxisAngle( &Rotate, &Axis, &Angle );

  for ( int x = 0; x < Vertices.NumItems(); x++ )
  {
    float col = Vertices[ x ].Color[ 0 ];
    D3DXMATRIX m;
    D3DXQUATERNION q;
    D3DXQuaternionRotationAxis( &q, &Axis, Angle*col );
    D3DXMatrixTransformation( &m, NULL, NULL, &( D3DXVECTOR3( 1, 1, 1 ) + Scale*col ), NULL, &q, &( Translate*col ) );
    D3DXVec3Transform3( &Vertices[ x ].Position, &Vertices[ x ].Position, &m );
  }
}
#endif

#ifdef PHX_MESH_BEVEL
void CphxMesh::Bevel( float Percent )
{
  float bck = SmoothGroupSeparation;
  SmoothGroupSeparation = 2.0f;
  SkipNormalCalculation = false;

  CalculateNormals();

  SmoothGroupSeparation = bck;

  // normal calculation built the edge list already but not for vertices, doing that here:
  for ( int x = 0; x < Edges.NumItems(); x++ )
  {
    Vertices[ Edges[ x ].VertexIDs[ 0 ] ].EdgeIDs.Add( x );
    Vertices[ Edges[ x ].VertexIDs[ 1 ] ].EdgeIDs.Add( x );
  }

  int OriginalVertexCount = Vertices.NumItems();

  for ( int x = 0; x < OriginalVertexCount; x++ )
  {
    Vertices[ x ].StoredPosition = nullvector3;
    Vertices[ x ].ColorCount = 0;
  }

  //shrink polygons first

  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Polygons[ x ];

    //calculate polygon center
    D3DXVECTOR3 center = nullvector3;
    for ( int y = 0; y < p->VertexCount; y++ )
      center += Vertices[ p->VertexIDs[ y ] ].Position;
    center /= (float)p->VertexCount;

    //create shrinked polygon vertices
    for ( int y = 0; y < p->VertexCount; y++ )
    {
      D3DXVECTOR3 nv = ( Vertices[ p->VertexIDs[ y ] ].Position - center )*( 1 - Percent ) + center;

      int vxID = p->VertexIDs[ y ];

      CphxEdge *e1 = &Edges[ p->EdgeIDs[ y ] ];

      D3DXVECTOR3 dir = Vertices[ e1->VertexIDs[ 1 ] ].Position - Vertices[ e1->VertexIDs[ 0 ] ].Position;
      D3DXVECTOR3 dir2 = nv - Vertices[ e1->VertexIDs[ 0 ] ].Position;
      D3DXVECTOR3 cross;
      D3DXVec3Cross( &cross, &dir, &dir2 );

      int ep = ( e1->VertexIDs[ 0 ] == vxID ) ? 0 : 1;
      int pp = ( D3DXVec3Dot( &cross, &p->PolyNormal ) < 0 ) ? 0 : 1;
      e1->NewVertexIDs[ ep ][ pp ] = Vertices.NumItems();

      e1 = &Edges[ p->EdgeIDs[ ( y + p->VertexCount - 1 ) % p->VertexCount ] ];
      dir = Vertices[ e1->VertexIDs[ 1 ] ].Position - Vertices[ e1->VertexIDs[ 0 ] ].Position;
      dir2 = nv - Vertices[ e1->VertexIDs[ 0 ] ].Position;
      D3DXVec3Cross( &cross, &dir, &dir2 );
      ep = ( e1->VertexIDs[ 0 ] == vxID ) ? 0 : 1;
      pp = ( D3DXVec3Dot( &cross, &p->PolyNormal ) < 0 ) ? 0 : 1;
      e1->NewVertexIDs[ ep ][ pp ] = Vertices.NumItems();

      Vertices[ p->VertexIDs[ y ] ].StoredPosition += nv;
      Vertices[ p->VertexIDs[ y ] ].ColorCount++;
      AddVertex( nv );
      p->VertexIDs[ y ] = Vertices.NumItems() - 1;
    }
  }

  //move original vertices

  for ( int x = 0; x < OriginalVertexCount; x++ )
    Vertices[ x ].Position2 = Vertices[ x ].Position = Vertices[ x ].StoredPosition / (float)Vertices[ x ].ColorCount;

  //build edge polys
  for ( int x = 0; x < Edges.NumItems(); x++ )
  {
    CphxEdge *e = &Edges[ x ];
    if ( !( e->NewVertexIDs[ 0 ][ 0 ] == -1 || e->NewVertexIDs[ 1 ][ 0 ] == -1 || e->NewVertexIDs[ 0 ][ 1 ] == -1 || e->NewVertexIDs[ 1 ][ 1 ] == -1 ) )
      AddPolygon( e->NewVertexIDs[ 0 ][ 0 ], e->NewVertexIDs[ 1 ][ 0 ], e->NewVertexIDs[ 1 ][ 1 ], e->NewVertexIDs[ 0 ][ 1 ] );
  }

  for ( int x = 0; x < OriginalVertexCount; x++ )
  {
    int eCount = Vertices[ x ].EdgeIDs.NumItems();

    for ( int y = 0; y < eCount; y++ )
    {
      CphxEdge *e1 = &Edges[ Vertices[ x ].EdgeIDs[ y ] ];
      int eid = ( e1->VertexIDs[ 0 ] == x ) ? 0 : 1;

      if ( e1->NewVertexIDs[ eid ][ 0 ] == -1 || e1->NewVertexIDs[ eid ][ 1 ] == -1 )
        continue;

      D3DXVECTOR3 v1 = Vertices[ e1->NewVertexIDs[ eid ][ 0 ] ].Position - Vertices[ x ].Position;
      D3DXVECTOR3 v2 = Vertices[ e1->NewVertexIDs[ eid ][ 1 ] ].Position - Vertices[ x ].Position;
      D3DXVECTOR3 cross;
      D3DXVec3Cross( &cross, &v1, &v2 );
      float d = D3DXVec3Dot( &cross, &Vertices[ x ].Normal );
      int p = ( d < 0 ) ? 1 : 0;

      if ( e1->NewVertexIDs[ eid ][ p ] == -1 || e1->NewVertexIDs[ eid ][ ( p + 1 ) & 1 ] == -1 )
        continue;

      AddPolygon( x, e1->NewVertexIDs[ eid ][ p ], e1->NewVertexIDs[ eid ][ ( p + 1 ) & 1 ] );
    }
  }

}
#endif

#ifdef PHX_MESH_CREATEGEOSPHERE

#define _v1 0.42532540417602f
#define _v2 0.26286555605956f

static float IcosaVertexData[] =
{
  _v1, -_v2, 0,
  _v1, _v2, 0,
  _v2, 0, _v1,
  0, -_v1, _v2,
  0, -_v1, -_v2,
  _v2, 0, -_v1,
  0, _v1, _v2,
  -_v2, 0, _v1,
  -_v1, -_v2, 0,
  -_v2, 0, -_v1,
  0, _v1, -_v2,
  -_v1, _v2, 0
};

static char IcosaPolyData[] =
{
  0, 1, 2,
  0, 2, 3,
  0, 3, 4,
  0, 4, 5,
  0, 5, 1,
  1, 10, 6,
  2, 6, 7,
  3, 7, 8,
  4, 8, 9,
  5, 9, 10,
  6, 2, 1,
  7, 3, 2,
  8, 4, 3,
  9, 5, 4,
  10, 1, 5,
  11, 7, 6,
  11, 8, 7,
  11, 9, 8,
  11, 10, 9,
  11, 6, 10,
};

void CphxMesh::CreateGeoSphere( int Iterations )
{
  for ( int x = 0; x < 12; x++ )
    AddVertex( IcosaVertexData + x * 3 );

  char *data = (char*)IcosaPolyData;
  for ( int y = 0; y < 20; y++ )
  {
    AddPolygon( data[ 0 ], data[ 1 ], data[ 2 ] );
    data += 3;
  }

  //refine geosphere

  for ( int i = 0; i < Iterations; i++ )
  {
    RebuildEdgeList();

    int OriginalVertexCount = Vertices.NumItems();

    for ( int x = 0; x < Edges.NumItems(); x++ )
    {
      D3DXVECTOR3 v = Vertices[ Edges[ x ].VertexIDs[ 0 ] ].Position + Vertices[ Edges[ x ].VertexIDs[ 1 ] ].Position;
      D3DXVec3Normalize( &v, &v );
      AddVertex( v / 2.0f );
    }

    for ( int z = Polygons.NumItems() - 1; z >= 0; z-- )
    {
      for ( int x = 0; x < 3; x++ )
      {
        AddPolygon( Polygons[ z ].VertexIDs[ x ], OriginalVertexCount + Polygons[ z ].EdgeIDs[ x ], OriginalVertexCount + Polygons[ z ].EdgeIDs[ ( x + 2 ) % 3 ] );
        Polygons[ z ].VertexIDs[ x ] = OriginalVertexCount + Polygons[ z ].EdgeIDs[ x ];
      }
    }
  }

  for ( int x = 0; x < 4; x++ )
    CalculateTextureCoordinates( flareTextureMap_Spherical, x );
}
#endif

#ifdef PHX_MESH_SCATTER
D3DXMATRIX GetScatterTransformation( PHXSCATTERORIENTATIONTYPE Orientation, D3DXMATRIX *ObjPos, D3DXMATRIX *ShapePos, D3DXMATRIX *NormalPos, D3DXVECTOR3 Position, D3DXVECTOR3 Normal, D3DXVECTOR3 Up, float ScaleThreshold, bool ForceYScale, float OffsetThreshold, float Tint )
{
  //D3DXVec3Transform(&v,&pos,ShapePos);
  D3DXVECTOR4 n;
  D3DXVec3Transform3( &Position, &Position, ShapePos );
  //Position = *(D3DXVECTOR3*)&v;
  D3DXVec4Transform( &n, &( D3DXVECTOR4( Normal.x, Normal.y, Normal.z, 0 ) ), NormalPos );
  D3DXVec3Normalize( &Normal, (D3DXVECTOR3*)&n );

  D3DXVec4Transform( &n, &( D3DXVECTOR4( Up.x, Up.y, Up.z, 0 ) ), NormalPos );
  D3DXVec3Normalize( &Up, (D3DXVECTOR3*)&n );

  D3DXMATRIX m, t;
  D3DXMatrixIdentity( &m );

  //D3DXVECTOR3 y = Normal;
  D3DXVECTOR3 x, z;
  D3DXVec3Cross( &x, &Normal, &Up );

  if ( D3DXVec3Length( &x ) == 0 )
  {
    D3DXVec4Transform( &n, &xvector4, NormalPos );
    x = *(D3DXVECTOR3*)&n;
  }

  D3DXVec3Normalize( &x, &x );
  //at this point y and x are perpendicular
  D3DXVec3Cross( &z, &x, &Normal );

  //build base orientation matrix
  *( (D3DXVECTOR3*)m.m[ 0 ] ) = x;
  *( (D3DXVECTOR3*)m.m[ 1 ] ) = Normal;
  *( (D3DXVECTOR3*)m.m[ 2 ] ) = z;

  //scale
  float fScale = lerp( ScaleThreshold, 1 / ScaleThreshold, ( rand() / (float)RAND_MAX )*Tint );
  D3DXVECTOR3 Scale = D3DXVECTOR3( 1, 1, 1 )*fScale;
  if ( ForceYScale ) Scale.x = Scale.z = 1;

#ifdef PHX_SCATTER_HAS_ORIENTATION_FULLROTATE
  D3DXVECTOR3 Axis = GetRandomVertexOnSphere(); // needs to stay here to keep randseed
  if ( Orientation != flareScatterOrientation_FullRotate )
    Axis = yvector;
#else
  D3DXVECTOR3 Axis = yvector;
#endif
  float Angle = (float)rand();
#if defined PHX_SCATTER_HAS_ORIENTATION_NORMAL || defined PHX_SCATTER_HAS_ORIENTATION_ORIGINAL
  if ( Orientation == flareScatterOrientation_Normal || Orientation == flareScatterOrientation_Original )
    Angle = 0;
#endif

  D3DXVECTOR3 Pos = Position + Normal*OffsetThreshold*( rand() / (float)RAND_MAX )*Tint;

  D3DXQUATERNION q;
  D3DXMatrixTransformation( &t, NULL, NULL, &Scale, NULL, D3DXQuaternionRotationAxis( &q, &Axis, Angle ), NULL );
  D3DXMatrixMultiply( &m, &t, &m );

#ifdef PHX_SCATTER_HAS_ORIENTATION_ORIGINAL
  if ( Orientation == flareScatterOrientation_Original )
    m = t;
#endif
  //D3DXMatrixIdentity(&m);

  D3DXMatrixMultiply( &m, ObjPos, D3DXMatrixMultiply( &m, &m, D3DXMatrixTranslation( &t, Pos.x, Pos.y, Pos.z ) ) );

  return m;
}

void CphxMesh::Scatter( CphxMesh *Object, D3DXMATRIX *ObjPos, CphxMesh *Shape, D3DXMATRIX *ShapePos, unsigned char RandSeed, float VertexProbability, float EdgeProbability, float PolyProbability, unsigned char MaxPerPoly, unsigned char ProbabilityTint, PHXSCATTERORIENTATIONTYPE Orientation, float ScaleThreshold, bool ForceYScale, float OffsetThreshold, unsigned char ScaleOffsetTint )
{
  Shape->CalculateNormals();
  srand( RandSeed );

  D3DXVECTOR3 pos, norm, up;
  float probtint, offstint;
  D3DXMATRIX Transformation;

  D3DXMATRIX InvTransp;
  D3DXMatrixInverse( &InvTransp, NULL, ShapePos );
  D3DXMatrixTranspose( &InvTransp, &InvTransp );

  if ( EdgeProbability > 0 || VertexProbability > 0 )
    Shape->RebuildEdgeList( VertexProbability > 0 );

  //vertex copies
#ifdef PHX_SCATTER_HAS_VERTEX_SUPPORT
  for ( int x = 0; x < Shape->Vertices.NumItems(); x++ )
  {
    CphxVertex *vx = &Shape->Vertices[ x ];
    pos = vx->Position;
    up = yvector;

    if ( vx->EdgeIDs.NumItems() )
    {
      int *vxids = Shape->Edges[ vx->EdgeIDs[ 0 ] ].VertexIDs;
      D3DXVec3Normalize( &up, &( Shape->Vertices[ vxids[ 1 ] ].Position - Shape->Vertices[ vxids[ 0 ] ].Position ) );
    }

    Transformation = GetScatterTransformation( Orientation, ObjPos, ShapePos, &InvTransp, pos, vx->Normal, up, 1 - ScaleThreshold, ForceYScale, OffsetThreshold, 1 - vx->Color[ ScaleOffsetTint ] );

    if ( rand() / (float)RAND_MAX < VertexProbability*( 1 - vx->Color[ ProbabilityTint ] ) )
      CopyInstance( Object, &Transformation );
  }
#endif

  //edge copies
#ifdef PHX_SCATTER_HAS_EDGE_SUPPORT
  for ( int x = 0; x < Shape->Edges.NumItems(); x++ )
  {
    int *vxids = Shape->Edges[ x ].VertexIDs;

    norm = pos = nullvector3;
    probtint = offstint = 0;
    D3DXVECTOR3 edge = up = Shape->Vertices[ vxids[ 1 ] ].Position - Shape->Vertices[ vxids[ 0 ] ].Position;
    float length = D3DXVec3Length( &edge );
    //D3DXVec3Normalize( &up, &edge );

    for ( int y = 0; y < 2; y++ )
    {
      CphxVertex &v = Shape->Vertices[ vxids[ y ] ];
      pos += v.Position / 2.0f;
      if ( Shape->Edges[ x ].PolyIDs[ y ] >= 0 )
        norm += Shape->Polygons[ Shape->Edges[ x ].PolyIDs[ y ] ].PolyNormal / 2.0f;
      probtint += v.Color[ ProbabilityTint ] / 2.0f;
      offstint += v.Color[ ScaleOffsetTint ] / 2.0f;
    }

    D3DXMATRIX scaleMat;
    //D3DXMATRIX objMat = *ObjPos;
    //D3DXVec3Normalize( &norm, &norm );

    Transformation = GetScatterTransformation( Orientation, ObjPos, ShapePos, &InvTransp, pos, norm, up, 1 - ScaleThreshold, ForceYScale, OffsetThreshold, 1 - offstint );
    D3DXMatrixMultiply( &Transformation, D3DXMatrixScaling( &scaleMat, 1, 1, length ), &Transformation );

    if ( rand() / (float)RAND_MAX < EdgeProbability*( 1 - probtint ) )
      CopyInstance( Object, &Transformation );
  }
#endif

#ifdef PHX_SCATTER_HAS_POLY_SUPPORT
  //poly copies
  for ( int x = 0; x < Shape->Polygons.NumItems(); x++ )
  {
    CphxPolygon *p = &Shape->Polygons[ x ];
    int *vxids = p->VertexIDs;

    pos = nullvector3;
    probtint = offstint = 0;
    D3DXVec3Normalize( &up, &( Shape->Vertices[ vxids[ 1 ] ].Position - Shape->Vertices[ vxids[ 0 ] ].Position ) );

    norm = p->PolyNormal;

    for ( int y = 0; y < p->VertexCount; y++ )
    {
      CphxVertex &vx = Shape->Vertices[ vxids[ y ] ];
      pos += vx.Position / (float)p->VertexCount;
      probtint += vx.Color[ ProbabilityTint ] / (float)p->VertexCount;
      offstint += vx.Color[ ScaleOffsetTint ] / (float)p->VertexCount;
    }

    Transformation = GetScatterTransformation( Orientation, ObjPos, ShapePos, &InvTransp, pos, norm, up, 1 - ScaleThreshold, ForceYScale, OffsetThreshold, 1 - offstint );

    if ( rand() / (float)RAND_MAX < PolyProbability*( 1 - probtint ) )
      CopyInstance( Object, &Transformation );

# ifdef PHX_SCATTER_HAS_POLY_MORE_THAN_1_INSTANCE_SUPPORT
    int count = (int)( rand() / (float)RAND_MAX*( MaxPerPoly ) );
    for ( int y = 0; y < count; y++ )
    {
      float vxweights[ 4 ];
      float w = 0;
      for ( int z = 0; z < p->VertexCount; z++ )
      {
        vxweights[ z ] = rand() / (float)RAND_MAX;
        w += vxweights[ z ];
      }
      pos = nullvector3;
      for ( int z = 0; z < p->VertexCount; z++ )
        pos += Shape->Vertices[ vxids[ z ] ].Position*vxweights[ z ] / w;

      CopyInstance( Object, &GetScatterTransformation( Orientation, ObjPos, ShapePos, &InvTransp, pos, norm, up, 1 - ScaleThreshold, ForceYScale, OffsetThreshold, 1 - offstint ) );
    }
# endif
  }
#endif

}
#endif

#if defined(PHX_MESH_COPY) || defined(PHX_MESH_SCATTER)
void CphxMesh::CopyInstance( CphxMesh *a, D3DXMATRIX *Transformation )
{
  int VxCount = Vertices.NumItems();
  for ( int x = 0; x < a->Vertices.NumItems(); x++ )
  {
    Vertices.Add( a->Vertices[ x ] );
    CphxVertex *vx = &Vertices[ Vertices.NumItems() - 1 ];
    D3DXVec3Transform3( &vx->Position, &vx->Position, Transformation );
    D3DXVec3Transform3( &vx->Position2, &nullvector3, Transformation );
  }
  for ( int x = 0; x < a->Polygons.NumItems(); x++ )
  {
    Polygons.Add( a->Polygons[ x ] );
    CphxPolygon *p = &Polygons[ Polygons.NumItems() - 1 ];
    for ( int y = 0; y < p->VertexCount; y++ )
      p->VertexIDs[ y ] += VxCount;
  }
}
#endif

#ifdef PHX_MESH_LOADSTOREDMESH
void CphxMesh::LoadStoredMesh( CphxVertex *vx, int vxc, CphxPolygon *pl, int plc )
{
  for ( int x = 0; x < vxc; x++ )
    Vertices.Add( vx[ x ] );
  for ( int x = 0; x < plc; x++ )
    Polygons.Add( pl[ x ] );

  SkipNormalCalculation = true;
}
#endif

#ifdef PHX_MESH_LOADSTOREDMINIMESH
void CphxMesh::LoadStoredMiniMesh( unsigned char* vertices, int vxc, unsigned char* tris, int tricount )
{
  for ( int x = 0; x < vxc; x++ )
    AddVertex( vertices[ x * 3 ] / 255.0f, vertices[ x * 3 + 1 ] / 255.0f, vertices[ x * 3 + 2 ] / 255.0f );
  for ( int x = 0; x < vxc; x++ )
    AddVertex( -vertices[ x * 3 ] / 255.0f, vertices[ x * 3 + 1 ] / 255.0f, vertices[ x * 3 + 2 ] / 255.0f );

  for ( int x = 0; x < tricount; x++ )
  {
    int xa = x * 3;
    int na = tris[ xa ];
    int nb = tris[ xa + 1 ];
    int nc = tris[ xa + 2 ];
    AddPolygon( na, nb, nc );
    if ( Vertices[ na ].Position != Vertices[ na + vxc ].Position )
      na += vxc;
    if ( Vertices[ nb ].Position != Vertices[ nb + vxc ].Position )
      nb += vxc;
    if ( Vertices[ nc ].Position != Vertices[ nc + vxc ].Position )
      nc += vxc;
    AddPolygon( na, nc, nb );
  }
}
#endif

#ifdef PHX_MESH_CREATETREE
#include "Arbaro.h"
Arbaro::BranchData treeData[ 100000 ];

void BuildTreeData( int bufferSize, ID3D11Buffer **buffer, ID3D11ShaderResourceView **bufferView )
{
  D3D11_BUFFER_DESC treeDataBuffer;
  memset( &treeDataBuffer, 0, sizeof( treeDataBuffer ) );

  treeDataBuffer.ByteWidth = sizeof( Arbaro::BranchData ) * bufferSize;
  treeDataBuffer.Usage = D3D11_USAGE_IMMUTABLE;
  treeDataBuffer.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  treeDataBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  treeDataBuffer.StructureByteStride = sizeof( Arbaro::BranchData );

  D3D11_SUBRESOURCE_DATA subData;
  subData.pSysMem = treeData;

  phxDev->CreateBuffer( &treeDataBuffer, &subData, buffer );

  D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
  memset( &resourceViewDesc, 0, sizeof( resourceViewDesc ) );

  resourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
  resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  resourceViewDesc.Buffer.ElementWidth = sizeof( Arbaro::BranchData );
  resourceViewDesc.Buffer.NumElements = bufferSize;

  phxDev->CreateShaderResourceView( *buffer, &resourceViewDesc, bufferView );
}

void CphxMesh::CreateTree( unsigned char Seed, unsigned char* levelDensities, TREESPECIESDESCRIPTOR &Species )
{
  Arbaro::Tree t( Species, Seed );
  t.make();
  int branchCount = t.BuildTree( this, levelDensities, treeData );
  BuildTreeData( branchCount, &dataBuffer, &dataBufferView );

  //CphxTreeGenerator g;
  //g.MakeTree(Seed, Species);
  //g.BuildTree(this);
}
#endif

#ifdef PHX_MESH_CREATETREELEAVES
void CphxMesh::CreateTreeLeaves( unsigned char Seed, unsigned char* levelDensities, TREESPECIESDESCRIPTOR &Species )
{
  Arbaro::Tree t( Species, Seed );
  t.make();
  int branchCount = t.BuildLeaves( this, levelDensities, treeData );
  BuildTreeData( branchCount, &dataBuffer, &dataBufferView );

  // CphxTreeGenerator g;
   //g.MakeTree(Seed, Species);
   //g.BuildLeaves(this);
}
#endif

#ifdef PHX_MESH_CREATETEXT
#include <gl\gl.h>
#include <gl\glu.h>

#define MAX_Q_POINTS 10
#define DEFAULT_EXTRUSION 1.0

static int nVertexCount;
static bool bEdge;
static int triindices[ 3 ];
static bool triedges[ 3 ];

void CALLBACK glyphBegin( GLenum type, void *polygon_data )
{
  nVertexCount = 0;
}

void CALLBACK glyphVertex( GLdouble *vertex_data, void *polygon_data )
{
  CphxMesh * s = (CphxMesh*)polygon_data;

  triedges[ nVertexCount ] = bEdge;
  triindices[ nVertexCount++ ] = (int)vertex_data;

  if ( nVertexCount == 3 )
  {
    s->AddPolygon( triindices[ 2 ], triindices[ 1 ], triindices[ 0 ] );
    s->AddPolygon( triindices[ 0 ] + 1, triindices[ 1 ] + 1, triindices[ 2 ] + 1 );
    for ( int x = 0; x < 3; x++ )
      if ( triedges[ x ] )
      {
        int a = triindices[ x ];
        int b = triindices[ ( x + 1 ) % 3 ];
        s->AddPolygon( b, b + 1, a + 1, a );
      }
    nVertexCount = 0;
  }
}

#ifndef PHX_MINIMAL_BUILD
void CALLBACK glyphCombine( GLdouble coords[ 3 ], void *vertex_data[ 4 ], GLfloat weight[ 4 ], void **outData, void * polygon_data )
{
  CphxMesh * s = (CphxMesh*)polygon_data;
  ( *outData ) = (void *)s->Vertices.NumItems();
  s->AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], 0 );
  s->AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], DEFAULT_EXTRUSION );
}
#endif

void CALLBACK glyphEdge( bool Edge )
{
  bEdge = Edge;
}

void CphxMesh::BInterp( D3DXVECTOR3 * pPoints, int * nPoints, D3DXVECTOR2 p1, D3DXVECTOR2 p2, D3DXVECTOR2 p3, float fDeviation )
{
  D3DXVECTOR2 s1, s2, m;
  s1.x = ( p1.x + p2.x )*0.5f;
  s1.y = ( p1.y + p2.y )*0.5f;
  s2.x = ( p2.x + p3.x )*0.5f;
  s2.y = ( p2.y + p3.y )*0.5f;
  m.x = ( s1.x + s2.x )*0.5f;
  m.y = ( s1.y + s2.y )*0.5f;

  //s1.z = s2.z = m.z = 0; //these are assumed to be 0

  float distance = ( p2.x - m.x )*( p2.x - m.x ) + ( p2.y - m.y )*( p2.y - m.y );

  if ( distance < fDeviation*fDeviation )
  {
    *( (D3DXVECTOR2*)&pPoints[ ( *nPoints )++ ] ) = p2;
  }
  else
  {
    BInterp( pPoints, nPoints, p1, s1, m, fDeviation );
    BInterp( pPoints, nPoints, m, s2, p3, fDeviation );
  }
}

extern char* EngineFontList[];

void CphxMesh::CreateText( unsigned char Font, char *Text, unsigned char Deviation )
{
  float fDeviation = Deviation / 255.0f + 0.001f; // 0 could cause infinite recursion / problems

  extern HDC hdc; //already created in the texgen
  HFONT hFont = CreateFontA( 32, 0, 0, 0, 0, 0, 0, 0, EngineFontList[ Font ][ 0 ] == 'W' ? SYMBOL_CHARSET : 0, 0, 0, 0, 0, EngineFontList[ Font ] );

  SelectObject( hdc, hFont );

  GLYPHMETRICS gm;
  static MAT2 m = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

#define TOFLOAT(x) ( (*(int*)&(x)) / (float)0x10000 )

  GLUtesselator * tess = gluNewTess();

  gluTessCallback( tess, GLU_TESS_BEGIN_DATA, ( GLvoid( CALLBACK * )( ) )glyphBegin );
  gluTessCallback( tess, GLU_TESS_VERTEX_DATA, ( GLvoid( CALLBACK * )( ) )glyphVertex );
  gluTessCallback( tess, GLU_TESS_EDGE_FLAG, ( GLvoid( CALLBACK * )( ) )glyphEdge );
#ifndef PHX_MINIMAL_BUILD
  gluTessCallback( tess, GLU_TESS_COMBINE_DATA, ( GLvoid( CALLBACK * )( ) )glyphCombine );
#endif
  ////gluTessCallback(tess, GLU_TESS_END_DATA,  (GLvoid (CALLBACK *)())glyphEnd);

  float x = 0;
  while ( *Text )
  {
    int nSize = GetGlyphOutline( hdc, *Text, GGO_NATIVE, &gm, 0, NULL, &m );

    char *p = new char[ nSize ];
    GetGlyphOutline( hdc, *Text, GGO_NATIVE, &gm, nSize, p, &m );
    Text++;

    gluTessBeginPolygon( tess, this );
    gluTessNormal( tess, 0, 0, 1 );

    GLdouble coords[ 3 ];
    coords[ 2 ] = 0.0;

    for ( char * pp = p; pp - p < nSize;)
    {
      TTPOLYGONHEADER * hdr = (TTPOLYGONHEADER*)pp;

      gluTessBeginContour( tess );

      coords[ 0 ] = ( TOFLOAT( hdr->pfxStart.x ) + x );
      coords[ 1 ] = ( TOFLOAT( hdr->pfxStart.y ) );

      gluTessVertex( tess, coords, (void*)Vertices.NumItems() );

      AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], 0 );
      AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], DEFAULT_EXTRUSION );

      int n = 0;
      for ( char * ppp = pp + sizeof( TTPOLYGONHEADER ); ppp - pp < (int)hdr->cb;)
      {
        TTPOLYCURVE * curve = (TTPOLYCURVE*)( ppp );

        if ( curve->wType == TT_PRIM_LINE )
        {
          for ( int j = 0; j < curve->cpfx; j++ )
          {
            coords[ 0 ] = ( TOFLOAT( curve->apfx[ j ].x ) + x );
            coords[ 1 ] = ( TOFLOAT( curve->apfx[ j ].y ) );

            gluTessVertex( tess, coords, (void*)Vertices.NumItems() );
            AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], 0 );
            AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], DEFAULT_EXTRUSION );
          }
        }
        else if ( curve->wType == TT_PRIM_QSPLINE )
        {
          D3DXVECTOR3 points[ MAX_Q_POINTS ];
          points[ 0 ].x = (float)coords[ 0 ];
          points[ 0 ].y = (float)coords[ 1 ];
          //points[0].z = 0;
          for ( int j = 0; j < curve->cpfx; j++ )
          {
            points[ j + 1 ].x = TOFLOAT( curve->apfx[ j ].x ) + x;
            points[ j + 1 ].y = TOFLOAT( curve->apfx[ j ].y );
            //points[j + 1].z = 0;
          }

          D3DXVECTOR3 start = points[ 0 ];
          for ( int j = 1; j < curve->cpfx; j++ )
          {
            static D3DXVECTOR3 pointTess[ MAX_Q_POINTS * 100 ];
            int nPointTess = 0;

            D3DXVECTOR3 end;
            if ( j == curve->cpfx - 1 )
            {
              end = points[ j + 1 ];
            }
            else
            {
              end.x = ( points[ j ].x + points[ j + 1 ].x )*0.5f;
              end.y = ( points[ j ].y + points[ j + 1 ].y )*0.5f;
            }

            //end.z = 0;

            BInterp( pointTess, &nPointTess, *(D3DXVECTOR2*)&start, *(D3DXVECTOR2*)&points[ j ], *(D3DXVECTOR2*)&end, fDeviation );
            start = end;
            for ( int k = 0; k < nPointTess; k++ )
            {
              D3DXVECTOR3 &s = pointTess[ k ];

              coords[ 0 ] = s.x;
              coords[ 1 ] = s.y;

              gluTessVertex( tess, coords, (void*)Vertices.NumItems() );
              AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], 0 );
              AddVertex( (float)coords[ 0 ], (float)coords[ 1 ], DEFAULT_EXTRUSION );
            }
          }
        }
        ppp += sizeof( TTPOLYCURVE ) + sizeof( POINTFX )* ( curve->cpfx - 1 );

      }

      gluTessEndContour( tess );

      pp += hdr->cb;
    }
    gluTessEndPolygon( tess );

    x += gm.gmCellIncX;

    delete[] p;
  }

  gluDeleteTess( tess );

  DeleteObject( hFont );
  //DeleteDC(hDC);

  float minimum, maximum;
  minimum = maximum = 0;
  for ( int y = 0; y < Vertices.NumItems(); y++ )
  {
    Vertices[ y ].Position.x *= 0.1f;
    Vertices[ y ].Position.y *= 0.1f;
    minimum = min( minimum, Vertices[ y ].Position.x );
    maximum = max( maximum, Vertices[ y ].Position.x );
  }

  for ( int y = 0; y < Vertices.NumItems(); y++ )
    Vertices[ y ].Position.x -= ( maximum + minimum ) / 2.0f;
}
#endif

#ifdef PHX_MESH_REPLICATE
void CphxMesh::Replicate( unsigned char Count, D3DXFLOAT16 *Srt )
{
  int vxcount = Vertices.NumItems();
  int polycount = Polygons.NumItems();

  D3DXMATRIX Transformation;
  D3DXMatrixIdentity( &Transformation );

  for ( int y = 0; y < 3; y++ )
    for ( int x = 0; x < 4; x++ )
      Transformation.m[ x ][ y ] = Srt[ x + ( y << 2 ) ];

  D3DXMATRIX transform = Transformation;

  //transform base object
  for ( int x = 0; x < vxcount; x++ )
  {
    CphxVertex *vx = &Vertices[ x ];
    D3DXVec3Transform3( &vx->Position, &vx->Position, &transform );
    D3DXVec3Transform3( &vx->Position2, &vx->Position2, &transform );
  }

  //create new copies
  for ( int z = 0; z < Count; z++ )
  {
    int VxBase = Vertices.NumItems();
    for ( int x = 0; x < vxcount; x++ )
    {
      CphxVertex vv = Vertices[ x ]; //need to create a copy of this - not a good idea to add a member of the same dynamic array back to itself
      Vertices.Add( vv );
      CphxVertex *vx = &Vertices[ Vertices.NumItems() - 1 ];
      D3DXVec3Transform3( &vx->Position, &vx->Position, &transform );
      D3DXVec3Transform3( &vx->Position2, &vx->Position2, &transform );
    }
    for ( int x = 0; x < polycount; x++ )
    {
      CphxPolygon pp = Polygons[ x ]; //need to create a copy of this - not a good idea to add a member of the same dynamic array back to itself
      Polygons.Add( pp );
      CphxPolygon *p = &Polygons[ Polygons.NumItems() - 1 ];
      for ( int y = 0; y < p->VertexCount; y++ )
        p->VertexIDs[ y ] += VxBase;
    }

    D3DXMatrixMultiply( &transform, &transform, &Transformation );
  }
}
#endif

#ifdef PHX_MESH_CREATEMARCHINGMESH

void CphxMesh::CreateMarchingMesh( D3DXVECTOR3 dimensions, int objCount, char *objType, D3DXMATRIX *objPositions, float surface, char resolution )
{

  // marching tetraeders algo
  typedef struct
  {
    float Value;
    D3DXVECTOR3 Pos;
    float Normal[ 3 ];
    int Inside;
  } GRIDPOINT;

  //yeah the tetraeder lookup tables are a bit smaller than the marching cubes ones ;)
  static const unsigned char Tetraeders[ 6 ][ 4 ] =
  {
    //6 tetraeders that make up the cube (cube vertex indices)
    { 3,7,2,1 },
    { 3,0,4,1 },
    { 3,4,7,1 },
    { 1,4,5,7 },
    { 1,5,2,7 },
    { 2,5,6,7 }
  };

  static unsigned char TetraMap[ 16 ][ 4 ] =
  {
    //tetraeder vertex: 0 1 2 3     - 0 = inside, 1= outside
    { 0,0,0,0 }, //       0 0 0 0
    { 3,5,6,0 }, //       0 0 0 1
    { 2,4,6,0 }, //       0 0 1 0
    { 2,3,5,4 }, //       0 0 1 1
    { 1,4,5,0 }, //       0 1 0 0
    { 1,4,6,3 }, //       0 1 0 1
    { 1,2,6,5 }, //       0 1 1 0
    { 1,2,3,0 }  //       0 1 1 1     next 8 are the same but in reverse order
                 // mappos = vertex0.inside*8+vertex1.inside*4+vertex2.inside*2+vertex3.inside;
  };

  for ( int x = 0; x < 8; x++ )
    for ( int y = 0; y < 4; y++ )
      TetraMap[ 15 - x ][ y ] = TetraMap[ x ][ y ];

  //unsigned char TriMap[ 6 ] = { 0,1,2,0,2,3 };

  float TetraederEdges[ 6 ][ 3 ]; //6 possible intersection points of the edges. mapped by TetraMap[value]-1, if TetraMap[value] is 0, end
  float TetraederNormals[ 6 ][ 3 ];
  GRIDPOINT Cube[ 8 ]; //8 grid points of the cube

  int res[ 3 ];
  res[ 0 ] = resolution;
  res[ 1 ] = resolution;
  res[ 2 ] = resolution;

  int gCnt = res[ 0 ] * res[ 1 ] * res[ 2 ];
  GRIDPOINT* Grid = new GRIDPOINT[ gCnt ];
  memset( Grid, 0, sizeof( GRIDPOINT )*gCnt );

  for ( int i = 0; i < gCnt; i++ )
  {
    int p[ 3 ];
    p[ 0 ] = i%res[ 0 ];
    p[ 1 ] = ( i / res[ 0 ] ) % res[ 1 ];
    p[ 2 ] = ( i / res[ 0 ] ) / res[ 1 ];

    for ( int j = 0; j < 3; j++ )
      Grid[ i ].Pos[ j ] = ( ( p[ j ] / (float)res[ j ] ) * 2 - 1 )*dimensions[ j ];

    for ( int j = 0; j < objCount; j++ )
    {
      D3DXVECTOR4 f;
      D3DXVec3Transform( &f, &Grid[ i ].Pos, &objPositions[ j ] );
      Grid[ i ].Value += 1 / D3DXVec3Length( &D3DXVECTOR3( f ) );
    }

    Grid[ i ].Inside = Grid[ i ].Value < surface;
  }

  for ( int x = 1; x < res[ 0 ] - 1; x++ )
    for ( int y = 1; y < res[ 1 ] - 1; y++ )
      for ( int z = 1; z < res[ 2 ] - 1; z++ )
      {
        int p = x*res[ 0 ] + y*res[ 1 ] + z*res[ 2 ];
        Grid[ p ].Normal[ 0 ] = ( Grid[ p + 1 ].Value - Grid[ p - 1 ].Value ) / 2.0f;
        Grid[ p ].Normal[ 1 ] = ( Grid[ p + res[ 0 ] ].Value - Grid[ p - res[ 0 ] ].Value ) / 2.0f;
        Grid[ p ].Normal[ 2 ] = ( Grid[ p + res[ 0 ] * res[ 1 ] ].Value - Grid[ p - res[ 0 ] * res[ 1 ] ].Value ) / 2.0f;
      }

  for ( int x = 1; x < res[ 0 ] - 1; x++ )
    for ( int y = 1; y < res[ 1 ] - 1; y++ )
      for ( int z = 1; z < res[ 2 ] - 1; z++ )
      {
        for ( int i = 0; i < 8; i++ )
        {
          int dif = ( i & 1 ) + ( i & 2 )*res[ 0 ] + ( i & 4 )*res[ 1 ];
          Cube[ i ] = Grid[ i + dif ];
        }

        //calculate cube
        for ( int j = 0; j < 6; j++ ) //for each tetraeder
        {
          unsigned char Map = ( Cube[ Tetraeders[ j ][ 0 ] ].Inside << 3 )
            + ( Cube[ Tetraeders[ j ][ 1 ] ].Inside << 2 )
            + ( Cube[ Tetraeders[ j ][ 2 ] ].Inside << 1 )
            + Cube[ Tetraeders[ j ][ 3 ] ].Inside;
          int e = 0;

          for ( int e1 = 0; e1 < 3; e1++ )
            for ( int e2 = e1 + 1; e2 < 4; e2++ )
            {
              GRIDPOINT p1, p2;
              p1 = Cube[ Tetraeders[ j ][ e1 ] ];
              p2 = Cube[ Tetraeders[ j ][ e2 ] ];
              if ( p1.Inside != p2.Inside )
              {
                float t = ( surface - p1.Value ) / ( p2.Value - p1.Value );
                int k;
                for ( k = 0; k < 3; k++ )
                {
                  TetraederEdges[ e ][ k ] = lerp( p1.Pos[ k ], p2.Pos[ k ], t );
                  TetraederNormals[ e ][ k ] = lerp( p1.Normal[ k ], p2.Normal[ k ], t );
                }
              }
              e++;
            }

          int start = Vertices.NumItems();

          for ( int k = 0; k < 4; k++ )
            if ( TetraMap[ Map ][ k ] )
            {
              AddVertex( TetraederEdges[ TetraMap[ Map ][ k ] - 1 ] );
              Vertices[ start + k ].Normal = TetraederNormals[ TetraMap[ Map ][ k ] - 1 ];
            }

          if ( !TetraMap[ Map ][ 3 ] )
            AddPolygon( start, start + 1, start + 2 );
          else
            AddPolygon( start, start + 1, start + 2, start + 3 );

          //for ( int k = 0; k < 6; k++ )
          //  if ( ( TetraMap[ Map ][ 0 ] && k < 3 ) || ( TetraMap[ Map ][ 3 ] && k >= 3 ) )
          //  {
          //    int t = TetraMap[ Map ][ TriMap[ k ] ] - 1;
          //    //glTexCoord2f( TetraederEdges[ t ][ 0 ] / (float)GRIDX * 3, ( ( -TetraederEdges[ t ][ 2 ] + TunnelPos ) / (float)GRIDX * 3 ) );
          //    //glNormal3fv( TetraederNormals[ t ] );
          //    //glVertex3fv( TetraederEdges[ t ] );
          //  }
        }
      }

  delete[] Grid;

  SkipNormalCalculation = true;
}

#endif

#ifdef PHX_MESH_CSG

struct CSGPlane
{
  D3DXVECTOR3 normal;
  float w;

  CSGPlane()
  {
    normal = D3DXVECTOR3( 0, 0, 0 );
    w = 0;
  }

  bool ok()
  {
    return D3DXVec3Length( &normal ) > 0;
  }

  void Flip()
  {
    normal = -normal;
    w = -w;
  }
};

struct CSGPoly
{
  struct CphxUV
  {
    D3DXVECTOR2 uv[ 4 ];
    D3DXVECTOR3 vertNormal;
    CphxUV() {};
    CphxUV( const D3DXVECTOR2 _uv[ 4 ], const D3DXVECTOR3 norm )
    {
      for ( int x = 0; x < 4; x++ )
        uv[ x ] = _uv[ x ];
      vertNormal = norm;
    }
  };

  CphxArray<int> vertexIDs;
  CphxArray<CphxVertex> *vertices;
  CphxArray<CphxUV> UVs;
  CSGPlane plane;

  CSGPoly() {};
  CSGPoly( CphxArray<CphxVertex> *vxData, int a, int b, int c, CphxUV& uva, CphxUV& uvb, CphxUV& uvc )
  {
    vertices = vxData;
    vertexIDs += a;
    vertexIDs += b;
    vertexIDs += c;
    UVs += uva;
    UVs += uvb;
    UVs += uvc;
    CalcNormal();
  }

  void CalcNormal()
  {
    D3DXVec3Cross( &plane.normal, &( ( *vertices )[ vertexIDs[ 1 ] ].Position - ( *vertices )[ vertexIDs[ 0 ] ].Position ), &( ( *vertices )[ vertexIDs[ 2 ] ].Position - ( *vertices )[ vertexIDs[ 0 ] ].Position ) );
    D3DXVec3Normalize( &plane.normal, &plane.normal );
    plane.w = D3DXVec3Dot( &plane.normal, &( ( *vertices )[ vertexIDs[ 0 ] ].Position ) );
  }

  void Flip()
  {
    CphxArray<int> v2;
    for ( int x = vertexIDs.NumItems() - 1; x >= 0; x-- )
      v2.Add( vertexIDs[ x ] );

    CphxArray<CphxUV> uv2;
    for ( int x = UVs.NumItems() - 1; x >= 0; x-- )
    {
      UVs[ x ].vertNormal = -UVs[ x ].vertNormal;
      uv2.Add( UVs[ x ] );
    }

    vertexIDs = v2;
    UVs = uv2;
    plane.Flip();
  }
};

static const float csgEpsilon = 0.0001f;

void SplitPoly( const CSGPlane &plane, const CSGPoly &polygon, CphxArray<CSGPoly>& coplanarFront, CphxArray<CSGPoly>& coplanarBack, CphxArray<CSGPoly>& front, CphxArray<CSGPoly>& back )
{
  enum
  {
    COPLANAR = 0,
    FRONT = 1,
    BACK = 2,
    SPANNING = 3
  };

  CphxArray<CphxVertex> &vertices = *polygon.vertices;

  int polygonType = 0;
  int* types = new int[ polygon.vertexIDs.NumItems() ];

  for ( int i = 0; i < polygon.vertexIDs.NumItems(); i++ )
  {
    float t = D3DXVec3Dot( &plane.normal, &vertices[ polygon.vertexIDs[ i ] ].Position ) - plane.w;
    int type = ( t < -csgEpsilon ) ? BACK : ( ( t > csgEpsilon ) ? FRONT : COPLANAR );
    polygonType |= type;
    types[ i ] = type;
  }

  if ( polygonType == SPANNING )
  {
    CphxArray<int> f, b;
    CphxArray<CSGPoly::CphxUV> fuv, buv;

    for ( int i = 0; i < polygon.vertexIDs.NumItems(); i++ )
    {
      int j = ( i + 1 ) % polygon.vertexIDs.NumItems();
      int ti = types[ i ];
      int tj = types[ j ];
      CphxVertex vi = vertices[ polygon.vertexIDs[ i ] ];
      CphxVertex vj = vertices[ polygon.vertexIDs[ j ] ];
      CSGPoly::CphxUV ui = polygon.UVs[ i ];
      CSGPoly::CphxUV uj = polygon.UVs[ j ];

      if ( ti != BACK )
      {
        f.Add( polygon.vertexIDs[ i ] );
        fuv.Add( ui );
      }
      if ( ti != FRONT )
      {
        b.Add( polygon.vertexIDs[ i ] );
        buv.Add( ui );
      }

      if ( ( ti | tj ) == SPANNING )
      {
        float t = ( plane.w - D3DXVec3Dot( &plane.normal, &vi.Position ) ) / D3DXVec3Dot( &plane.normal, &( vj.Position - vi.Position ) );

        CphxVertex v;
        v.Position2 = v.Position = ( vj.Position - vi.Position )*t + vi.Position;
        CSGPoly::CphxUV vuv;
        for ( int x = 0; x < 4; x++ )
          vuv.uv[ x ] = ( uj.uv[ x ] - ui.uv[ x ] )*t + ui.uv[ x ];

        vuv.vertNormal = ( uj.vertNormal - ui.vertNormal )*t + ui.vertNormal;
        D3DXVec3Normalize( &vuv.vertNormal, &vuv.vertNormal );

        f.Add( vertices.NumItems() );
        fuv.Add( vuv );
        b.Add( vertices.NumItems() );
        buv.Add( vuv );
        //v.isSplit = true;
        vertices.Add( v );
      }
    }

    if ( f.NumItems() >= 3 )
    {
      CSGPoly n;
      n.vertices = polygon.vertices;
      n.vertexIDs = f;
      n.CalcNormal();
      n.UVs = fuv;
      front.Add( n );
    }

    if ( b.NumItems() >= 3 )
    {
      CSGPoly n;
      n.vertices = polygon.vertices;
      n.vertexIDs = b;
      n.CalcNormal();
      n.UVs = buv;
      back.Add( n );
    }
  }
  else
  {
    CphxArray<CSGPoly>* target;

    switch ( polygonType )
    {
    case COPLANAR:
    {
      target = ( D3DXVec3Dot( &plane.normal, &polygon.plane.normal ) > 0 ) ? ( &coplanarFront ) : ( &coplanarBack );
      break;
    }
    case FRONT:
    {
      target = &front;
      break;
    }
    case BACK:
    {
      target = &back;
      break;
    }
    }
    target->Add( polygon );
  }

  delete[] types;
}


struct CSGMesh
{
  CphxArray<CphxVertex> vertices;
  CphxArray<CSGPoly> polys;

  CSGMesh( CphxMesh* src, D3DXMATRIX* m )
  {
    vertices = src->Vertices;

    D3DXMATRIX invMat;
    D3DXMatrixInverse( &invMat, NULL, m );
    D3DXMatrixTranspose( &invMat, &invMat );

    for ( int x = 0; x < vertices.NumItems(); x++ )
      D3DXVec3Transform3( &vertices[ x ].Position, &vertices[ x ].Position, m );

    for ( int x = 0; x < src->Polygons.NumItems(); x++ )
    {
      CphxPolygon* p = &src->Polygons[ x ];
      
      D3DXVECTOR3 normals[ 4 ];
      for ( int y = 0; y < p->VertexCount; y++ )
        D3DXVec3Transform3( &normals[ y ], &p->Normals[ y ], &invMat );

      // triangulate to make sure every poly is flat
      if ( p->VertexCount >= 3 )
      {
        polys.Add( CSGPoly( &vertices,
                            p->VertexIDs[ 0 ],
                            p->VertexIDs[ 1 ],
                            p->VertexIDs[ 2 ],
                            CSGPoly::CphxUV( p->Texcoords[ 0 ], normals[ 0 ] ),
                            CSGPoly::CphxUV( p->Texcoords[ 1 ], normals[ 1 ] ),
                            CSGPoly::CphxUV( p->Texcoords[ 2 ], normals[ 2 ] ) ) );
      }
      if ( p->VertexCount == 4 )
      {
        polys.Add( CSGPoly( &vertices,
                            p->VertexIDs[ 0 ],
                            p->VertexIDs[ 2 ],
                            p->VertexIDs[ 3 ],
                            CSGPoly::CphxUV( p->Texcoords[ 0 ], normals[ 0 ] ),
                            CSGPoly::CphxUV( p->Texcoords[ 2 ], normals[ 2 ] ),
                            CSGPoly::CphxUV( p->Texcoords[ 3 ], normals[ 3 ] ) ) );
      }
    }
  }
};

struct BSPNode
{
  BSPNode* nodes[ 2 ];

  CSGPlane plane;

  CphxArray<CSGPoly> polys;

  void Build( CphxArray<CSGPoly>& polyList )
  {
    if ( !polyList.NumItems() )
      return;

    if ( !plane.ok() )
      plane = polyList[ 0 ].plane;

    CphxArray<CSGPoly> lists[ 2 ];

    for ( int x = 0; x < polyList.NumItems(); x++ )
      SplitPoly( plane, polyList[ x ], polys, polys, lists[ 0 ], lists[ 1 ] );

    for ( int x = 0; x < 2; x++ )
    {
      if ( !lists[ x ].NumItems() )
        continue;
      if ( !nodes[ x ] )
        nodes[ x ] = new BSPNode();
      nodes[ x ]->Build( lists[ x ] );
    }
  }

  void Invert()
  {
    for ( int x = 0; x < polys.NumItems(); x++ )
      polys[ x ].Flip();
    plane.Flip();
    BSPNode *a = nodes[ 0 ];
    nodes[ 0 ] = nodes[ 1 ];
    nodes[ 1 ] = a;

    for ( int x = 0; x < 2; x++ )
      if ( nodes[ x ] )
        nodes[ x ]->Invert();
  }

  void ClipTo( BSPNode* other )
  {
    CphxArray<CSGPoly> result;
    other->ClipPolygons( result, polys );
    polys = result;

    for ( int x = 0; x < 2; x++ )
      if ( nodes[ x ] )
        nodes[ x ]->ClipTo( other );
  }

  void AllPolygons( CphxArray<CSGPoly>& result )
  {
    result += polys;

    for ( int x = 0; x < 2; x++ )
      if ( nodes[ x ] )
        nodes[ x ]->AllPolygons( result );
  }

  void ClipPolygons( CphxArray<CSGPoly>& result, const CphxArray<CSGPoly>& polyList )
  {
    if ( !plane.ok() )
    {
      result += polyList;
      return;
    }

    CphxArray<CSGPoly> lists[ 2 ];

    for ( int x = 0; x < polyList.NumItems(); x++ )
      SplitPoly( plane, polyList[ x ], lists[ 0 ], lists[ 1 ], lists[ 0 ], lists[ 1 ] );

    if ( nodes[ 0 ] )
      nodes[ 0 ]->ClipPolygons( result, lists[ 0 ] );
    else
      result += lists[ 0 ];

    if ( nodes[ 1 ] )
      nodes[ 1 ]->ClipPolygons( result, lists[ 1 ] );
  }

  /*
    BSPNode* clone()
    {
      BSPNode *clone = new BSPNode();
      clone->plane = plane;
      clone->polys = polys;
      for (int x=0; x<2; x++ )
        if (nodes[x] )
          clone->nodes[x]=nodes[x]->clone();

      return clone;
    }
  */

  BSPNode()
  {
    nodes[ 0 ] = nodes[ 1 ] = nullptr;
  }

  BSPNode( CSGMesh& mesh )
  {
    nodes[ 0 ] = nodes[ 1 ] = nullptr;
    Build( mesh.polys );
  }

  ~BSPNode()
  {
    if ( nodes[ 0 ] )
      delete nodes[ 0 ];
    if ( nodes[ 1 ] )
      delete nodes[ 1 ];
  }
};

void CphxMesh::CSG( CphxMesh *object, D3DXMATRIX *objPos, int operation )
{
  if ( !object->Vertices.NumItems() )
    return;

  SkipNormalCalculation = true;

  D3DXMATRIX m;
  D3DXMatrixIdentity( &m );
  CSGMesh meshA( this, &m );
  CSGMesh meshB( object, objPos );

  BSPNode* a = new BSPNode( meshA );
  BSPNode* b = new BSPNode( meshB );

  CphxArray<CSGPoly> result;

  switch ( operation )
  {
  case 0: // union
    a->ClipTo( b );
    b->ClipTo( a );
    b->Invert();
    b->ClipTo( a );
    b->Invert();
    b->AllPolygons( result );
    a->Build( result );
    break;
  case 1: // difference
    a->Invert();
    a->ClipTo( b );
    b->ClipTo( a );
    b->Invert();
    b->ClipTo( a );
    b->Invert();
    b->AllPolygons( result );
    a->Build( result );
    a->Invert();
    break;
  case 2: // intersection
    a->Invert();
    b->ClipTo( a );
    b->Invert();
    a->ClipTo( b );
    b->ClipTo( a );
    b->AllPolygons( result );
    a->Build( result );
    a->Invert();
    break;
  }

  CphxArray<CSGPoly> finalPolys;
  a->AllPolygons( finalPolys );
  Vertices = meshA.vertices;

  //int* vertexMap = new int[ meshB.vertices.NumItems() ];

  //for ( int x = 0; x < meshB.vertices.NumItems(); x++ )
  //{
  //  vertexMap[ x ] = -1;

  //  if ( meshB.vertices[ x ].isSplit )
  //  {
  //    D3DXVECTOR3 &v2 = meshB.vertices[ x ].Position;
  //    for ( int y = 0; y < meshA.vertices.NumItems(); y++ )
  //    {
  //      D3DXVECTOR3 &v1 = meshA.vertices[ y ].Position;
  //      if ( abs( v2.x - v1.x ) <= csgEpsilon &&
  //           abs( v2.y - v1.y ) <= csgEpsilon &&
  //           abs( v2.z - v1.z ) <= csgEpsilon )
  //      {
  //        vertexMap[ x ] = y;
  //        break;
  //      }
  //    }
  //  }
  //}

  Vertices += meshB.vertices;

  Polygons.Flush();

  for ( int x = 0; x < finalPolys.NumItems(); x++ )
  {
    CSGPoly& p = finalPolys[ x ];

    for ( int y = 2; y < p.vertexIDs.NumItems(); y++ )
    {
      int ids[ 3 ];
      ids[ 0 ] = p.vertexIDs[ 0 ];
      ids[ 1 ] = p.vertexIDs[ y - 1 ];
      ids[ 2 ] = p.vertexIDs[ y ];

      if ( p.vertices == &meshB.vertices )
      {
        for ( int z = 0; z < 3; z++ )
        {
          //if ( vertexMap[ ids[ z ] ] == -1 )
          ids[ z ] += meshA.vertices.NumItems();
          //else
          //  ids[ z ] = vertexMap[ ids[ z ] ];
        }
      }

      AddPolygon( ids[ 0 ],
                  ids[ 1 ],
                  ids[ 2 ],
                  ids[ 2 ], // size hack
                  (float*)p.UVs[ 0 ].uv,
                  (float*)p.UVs[ y - 1 ].uv,
                  (float*)p.UVs[ y ].uv,
                  (float*)p.UVs[ y ].uv );

      auto &poly = Polygons[ Polygons.NumItems() - 1 ];
      poly.Normals[ 0 ] = p.UVs[ 0 ].vertNormal;
      poly.Normals[ 1 ] = p.UVs[ y - 1 ].vertNormal;
      poly.Normals[ 2 ] = p.UVs[ y ].vertNormal;

    }
  }

  //delete[] vertexMap;
  delete a;
  delete b;
}

#endif

#ifdef PHX_MESH_GREEBLE

void CphxMesh::Greeble_SplitPoly( int ply )
{
  CphxPolygon* poly = &Polygons[ ply ];

  int vxc = poly->VertexCount;

  if ( vxc < 3 )
    return;

  poly->TouchedByNormalCalculator = true; // is the poly split?

  int startEdge = rand() % vxc;
  int s1 = ( startEdge + 1 ) % vxc;
  int s2 = ( startEdge + 2 ) % vxc;
  int destEdge = ( startEdge + vxc - 2 ) % vxc;
  int d1 = ( destEdge + 1 ) % vxc;
  int d2 = ( destEdge + 2 ) % vxc;
  int vids[ 4 ];
  memcpy( vids, poly->VertexIDs, 16 );

  int v = Vertices.NumItems();

  CphxVertex &a = Vertices[ vids[ startEdge ] ];
  CphxVertex &b = Vertices[ vids[ s1 ] ];
  D3DXVECTOR2 startUVs[ 4 ];
  for ( int x = 0; x < 4; x++ )
    startUVs[ x ] = ( poly->Texcoords[ startEdge ][ x ] + poly->Texcoords[ s1 ][ x ] ) / 2.0f;

  Vertices.Add( ( a + b ) / 2.0f );

  CphxVertex &c = Vertices[ vids[ destEdge ] ];
  CphxVertex &d = Vertices[ vids[ d1 ] ];
  D3DXVECTOR2 destUVs[ 4 ];
  for ( int x = 0; x < 4; x++ )
    destUVs[ x ] = ( poly->Texcoords[ destEdge ][ x ] + poly->Texcoords[ d1 ][ x ] ) / 2.0f;

  Vertices.Add( ( c + d ) / 2.0f );

  // triangle tip or first poly half
  if ( vxc == 3 )
    AddPolygon( v, vids[ s1 ], v + 1, v + 1, (float*)startUVs, (float*)Polygons[ ply ].Texcoords[ s1 ], (float*)destUVs, (float*)destUVs );
  else
    AddPolygon( v, vids[ s1 ], vids[ s2 ], v + 1, (float*)startUVs, (float*)Polygons[ ply ].Texcoords[ s1 ], (float*)Polygons[ ply ].Texcoords[ s2 ], (float*)destUVs );

  // quad part
  AddPolygon( v + 1, vids[ d1 ], vids[ d2 ], v, (float*)destUVs, (float*)Polygons[ ply ].Texcoords[ d1 ], (float*)Polygons[ ply ].Texcoords[ d2 ], (float*)startUVs );
}

void CphxMesh::Greeble( int seed, float extrude, float taper )
{
  srand( seed );

  int polyCount = Polygons.NumItems();
  int x = 0;

  for ( int x = 0; x < polyCount; x++ )
    Polygons[ x ].TouchedByNormalCalculator = false; // is the poly split?

  // split pass

  for ( int y = 0; y < 2; y++ )  // this ensures that we go through the original polys once and then the first level of splits too
  {
    for ( ; x < polyCount; x++ )
    {
      if ( rand() & 1 )
        Greeble_SplitPoly( x );
    }

    polyCount = Polygons.NumItems(); // move the target
  }

  // drop original polys

  CphxArray<CphxPolygon> newPolys;

  for ( int x = 0; x < Polygons.NumItems(); x++ )
    if ( !Polygons[ x ].TouchedByNormalCalculator )
      newPolys.Add( Polygons[ x ] );

  Polygons = newPolys;

  // extrude pass

  polyCount = Polygons.NumItems();

  for ( int x = 0; x < polyCount; x++ )
  {
    float dist = ( rand() & 0xff ) / 255.0f * extrude;

    int v = Polygons[ x ].VertexCount;
    CphxVertex center = Vertices[ Polygons[ x ].VertexIDs[ 0 ] ];

    D3DXVECTOR2 uvCenter[ 4 ];
    memcpy( uvCenter, Polygons[ x ].Texcoords[ 0 ], sizeof( D3DXVECTOR2 ) * 4 );
    
    D3DXVECTOR2 texCoords[ 4 ][ 4 ];
    for ( int y = 0; y < 4; y++ )
      memcpy( texCoords[ y ], Polygons[ x ].Texcoords[ y ], sizeof( D3DXVECTOR2 ) * 4 );

    for ( int y = 1; y < v; y++ )
    {
      center = center + Vertices[ Polygons[ x ].VertexIDs[ y ] ];
      for ( int z = 0; z < 4; z++ )
        uvCenter[ z ] = uvCenter[ z ] + Polygons[ x ].Texcoords[ y ][ z ];
    }

    center = center / (float)v;
    for ( int y = 0; y < 4; y++ )
      uvCenter[ y ] = uvCenter[ y ] / (float)v;

    D3DXVECTOR3 off = CalculatePolyNormal( x );
    D3DXVec3Normalize( &off, &off );

    int vx = Vertices.NumItems();

    for ( int y = 0; y < v; y++ )
    {
      Vertices.Add( ( Vertices[ Polygons[ x ].VertexIDs[ y ] ] + ( center / -1.0f ) ) / ( 1.0f / taper ) + center );
      Vertices[ Vertices.NumItems() - 1 ].Position2 = ( Vertices[ Polygons[ x ].VertexIDs[ y ] ].Position2 + ( center.Position2 / -1.0f ) ) / ( 1.0f / taper ) + center.Position2;

      float cd = D3DXVec3Length( &( Vertices[ Vertices.NumItems() - 1 ].Position - center.Position ) );

      Vertices[ Vertices.NumItems() - 1 ].Position += off*dist;

      float di = D3DXVec3Length( &( Vertices[ Vertices.NumItems() - 1 ].Position - Vertices[ Polygons[ x ].VertexIDs[ y ] ].Position ) );

      for ( int z = 0; z < 4; z++ )
      {
        texCoords[ y ][ z ] = ( uvCenter[ z ] - texCoords[ y ][ z ] )*( di / ( cd + di ) ) + texCoords[ y ][ z ];
      }
    }

    for ( int y = 0; y < v; y++ )
      AddPolygon( Polygons[ x ].VertexIDs[ y ],         Polygons[ x ].VertexIDs[ ( y + 1 ) % v ],               vx + ( y + 1 ) % v,                 vx + y,
          (float*)Polygons[ x ].Texcoords[ y ], (float*)Polygons[ x ].Texcoords[ ( y + 1 ) % v ], (float*)texCoords[ ( y + 1 ) % v ], (float*)texCoords[ y ] );

    for ( int y = 0; y < v; y++ )
    {
      Polygons[ x ].VertexIDs[ y ] = vx + y;
      memcpy( Polygons[ x ].Texcoords[ y ], texCoords[ y ], sizeof( D3DXVECTOR2 ) * 4 );
    }
  }

}

#ifdef PHX_MESH_INVERT
void CphxMesh::Invert()
{
  for ( int x = 0; x < Polygons.NumItems(); x++ )
  {
    for ( int y = 0; y < Polygons[ x ].VertexCount; y++ )
    {
      auto& p = Polygons[ x ];
      int n1 = p.VertexIDs[ y ];
      p.VertexIDs[ y ] = p.VertexIDs[ p.VertexCount - y - 1 ];
      p.VertexIDs[ p.VertexCount - y - 1 ] = n1;
    }
  }
}
#endif

#ifdef PHX_MESH_SAVEPOS2
void CphxMesh::SavePos2()
{
  for ( int x = 0; x < Vertices.NumItems(); x++ )
    Vertices[ x ].Position2 = Vertices[ x ].Position;
}
#endif

#endif