#pragma once
#include "phxEngine.h"
#include "phxarray.h"

#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")

#include "Arbaro.h"

#define DEFAULTSMOOTHGROUPSEPARATION 0
//(1-sqrtf(3.0f)/2.0f)

enum PHXMESHPRIMITIVE //parameter count array is MeshPrimitiveParameterCounts[]
{
#ifdef PHX_MESH_CUBE
  Mesh_Cube = 0,
#endif
#ifdef PHX_MESH_PLANE
  Mesh_Plane = 1,
#endif
#ifdef PHX_MESH_SPHERE
  Mesh_Sphere = 2,
#endif
#ifdef PHX_MESH_CYLINDER
  Mesh_Cylinder = 3,
#endif
#ifdef PHX_MESH_CONE
  Mesh_Cone = 4,
#endif
#ifdef PHX_MESH_ARC
  Mesh_Arc = 5,
#endif
#ifdef PHX_MESH_LINE
  Mesh_Line = 6,
#endif
#ifdef PHX_MESH_SPLINE
  Mesh_Spline = 7,
#endif
#ifdef PHX_MESH_LOFT
  Mesh_Loft = 8,
#endif
  Mesh_Clone = 9,
#ifdef PHX_MESH_COPY
  Mesh_Copy = 10,
#endif
#ifdef PHX_MESH_CREATEGEOSPHERE
  Mesh_GeoSphere = 11,
#endif
#ifdef PHX_MESH_SCATTER
  Mesh_Scatter = 12,
#endif
#ifdef PHX_MESH_LOADSTOREDMESH
  Mesh_Stored = 13,
#endif
#ifdef PHX_MESH_CREATETREE
  Mesh_Tree = 14,
#endif
#ifdef PHX_MESH_CREATETREELEAVES
  Mesh_TreeLeaves = 15,
#endif
#ifdef PHX_MESH_CREATETEXT
  Mesh_Text = 16,
#endif
#ifdef PHX_MESH_CREATEMARCHINGMESH
  Mesh_Marched = 17,
#endif
#ifdef PHX_MESH_LOADSTOREDMINIMESH
  Mesh_StoredMini = 18,
#endif
#ifdef PHX_MESH_MERGE
  Mesh_Merge = 19,
#endif
};

enum PHXMESHFILTER //if you add one here add it to the ModelFilterNames[] variable in the tool as well
  //parameter count array is MeshFilterParameterCounts[]
{
  ModelFilter_UVMap = 0,
#ifdef PHX_MESH_BEVEL
  ModelFilter_Bevel = 1,
#endif
#ifdef PHX_MESH_MAPXFORM
  ModelFilter_MapXForm = 2,
#endif
#ifdef PHX_MESH_SMOOTH
  ModelFilter_MeshSmooth = 3,
#endif
  ModelFilter_SmoothGroup = 4,
#ifdef PHX_MESH_CALCULATETINT
  ModelFilter_TintMesh = 5,
#endif
#ifdef PHX_MESH_CALCULATETINTSHAPE
  ModelFilter_TintMeshShape = 6,
#endif
#ifdef PHX_MESH_REPLICATE
  ModelFilter_Replicate = 7,
#endif
#ifdef PHX_MESH_NORMALDEFORM
  ModelFilter_NormalDeform = 8,
#endif
#ifdef PHX_MESH_CSG
  ModelFilter_CSG = 9,
#endif
#ifdef PHX_MESH_GREEBLE
  ModelFilter_Greeble = 10,
#endif
#ifdef PHX_MESH_INVERT
  ModelFilter_Invert = 11,
#endif
#ifdef PHX_MESH_SAVEPOS2
  ModelFilter_SavePos2 = 12,
#endif
  //this should always be the last one, only used by the tool
  ModelFilter_NONE,
};

struct PHXVERTEXDATA
{
  D3DXVECTOR3 Position;
  D3DXVECTOR3 Position2;
  D3DXVECTOR3 Normal;
  unsigned int Color;
  D3DXVECTOR4 Texcoord;
  D3DXVECTOR4 Texcoord2;
};

struct PHXMESHFILTERDATA
{
  PHXMESHFILTER Type;
  unsigned char *FilterParams;
  D3DXFLOAT16 *filtertransform;
};

class CphxVertex
{
public:

  D3DXVECTOR3 Position, Position2, StoredPosition;
  D3DXVECTOR3 Normal;
  float Color[ 4 ];
  int ColorCount;

  CphxArray<int> EdgeIDs;
  //bool isSplit;

  CphxVertex();
  CphxVertex( const CphxVertex &v );

  CphxVertex operator +( const CphxVertex &e );
  CphxVertex operator /( const float &f );
  void operator =( const CphxVertex &v );
};

struct CphxEdgeSortStruct
{
  int Vx1, Vx2;
  int SourcePoly;
  int SourceVertex;
};

struct CphxEdge
{
  int VertexIDs[ 2 ];
  int PolyIDs[ 2 ];
  int NewVertexIDs[ 2 ][ 2 ];
  __forceinline int GetOtherPoly( int p );
};

class CphxPolygon
{
public:
  int VertexCount;
  int VertexIDs[ 4 ];
  int EdgeIDs[ 4 ];

  D3DXVECTOR3 PolyNormal;
  D3DXVECTOR3 Normals[ 4 ];
  D3DXVECTOR2 Texcoords[ 4 ][ 4 ];

  bool TouchedByNormalCalculator; // also used for Greeble split mark
  void AddToVertexNormal( int VertexID, D3DXVECTOR3 &N );
};

enum PHXTEXTUREMAPTYPE
{
  flareTextureMap_Planar = 0,
  flareTextureMap_Spherical,
  flareTextureMap_Cylindrical,
  flareTextureMap_Box,
  flareTextureMap_TransformOriginal,
};

enum PHXSCATTERORIENTATIONTYPE
{
  flareScatterOrientation_Original = 0,
  flareScatterOrientation_Normal,
  flareScatterOrientation_NormalRotate,
  flareScatterOrientation_FullRotate,
};

struct CphxMeshSplineKey
{
  D3DXFLOAT16 vx[ 3 ], Front[ 3 ], Back[ 3 ];
};

class CphxMesh
{
  D3DXVECTOR3 CalculatePolyNormal( int p );

  void CalculatePolyVertexNormal( int CurrentPoly, int VertexID, D3DXVECTOR3 &Vector );

public:
  void AddCapsBuildCylindricalTopologyAndCalculateUV( float Cap1Y, float Cap2Y, int XRes, int YRes, bool TopCap, bool BottomCap, PHXTEXTUREMAPTYPE MapType );

  //this value should go between 0 and 2, 0 for 0 degrees, 2 for 180 degrees
  float SmoothGroupSeparation;

  CphxArray<CphxVertex> Vertices;
  CphxArray<CphxPolygon> Polygons;
  CphxArray<CphxEdge> Edges;

  bool SkipNormalCalculation;

  ID3D11Buffer *dataBuffer;
  ID3D11ShaderResourceView *dataBufferView;

  void AddVertex( const float * );
  void AddVertex( const float x, const float y, const float z );
  //void AddVertex(const D3DXVECTOR3 &v, const float *UV);

  void AddPolygon( int a, int b, int c );
  void AddPolygon( int a, int b, int c, int d );
  //void AddPolygon(int a, int b, int c, const float *i, const float *j, const float *k);
  void AddPolygon( int a, int b, int c, int d, const float *i, const float *j, const float *k, const float *l );
  //void AddPolygon(const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const D3DXVECTOR3 &c);

  //int FindEdge(int v1, int v2);
  void RebuildEdgeList( bool BuildVertexEdgeList = false );
  void CalculateNormals();

  void BuildMesh( ID3D11Buffer *&VxBuffer, ID3D11Buffer *&TriIndexBuffer, ID3D11Buffer *&WireIndexBuffer, int &VxCount, int &TriCount, int &EdgeCount );

  void CalculateTextureCoordinates( PHXTEXTUREMAPTYPE Type, int Channel );
  //void CalculateTextureCoordinates(PHXTEXTUREMAPTYPE Type, int Channel, D3DXVECTOR3 Scale = D3DXVECTOR3(1, 1, 1), D3DXQUATERNION Rotate = D3DXQUATERNION(0, 0, 0, 1), D3DXVECTOR3 Translate = D3DXVECTOR3(0, 0, 0));
  void CalculateTextureCoordinates( PHXTEXTUREMAPTYPE Type, int Channel, bool clipUV, D3DXVECTOR3 Scale, D3DXQUATERNION Rotate, D3DXVECTOR3 Translate );

#ifdef PHX_MESH_CUBE
  void CreateCube();
#endif
#ifdef PHX_MESH_SPHERE
  void CreateSphere( int XRes, int YRes, float TopCut, float BottomCut, bool Caps );
#endif
#ifdef PHX_MESH_PLANE
  void CreatePlane( int XRes, int YRes );
#endif
#ifdef PHX_MESH_CONE
  void CreateCone( int XRes, int YRes, float TopCut, bool Caps );
#endif
#ifdef PHX_MESH_CYLINDER
  void CreateCylinder( int XRes, int YRes, bool Caps );
#endif

#if defined(PHX_MESH_ARC) || defined(PHX_MESH_LINE) || defined(PHX_MESH_SPLINE) || defined(PHX_MESH_LOFT)
  D3DXVECTOR3 ArcStartDir, ArcEndDir;
#endif

#ifdef PHX_MESH_ARC
  void CreateArc( int Res, float Degree, bool HaveLastSegment );
#endif
#ifdef PHX_MESH_LINE
  void CreateLine( int Res );
#endif
#ifdef PHX_MESH_SPLINE
  void Spline( CphxMeshSplineKey *Keys, int KeyCount, int Resolution, bool Loop );
#endif
#ifdef PHX_MESH_LOFT
  void Loft( CphxMesh *Path, D3DXMATRIX *PathPos, CphxMesh *Slice, D3DXMATRIX *SlicePos, bool PathClosed, bool SliceClosed, float Rotation, float StartScale, float EndScale );
#endif
#ifdef PHX_MESH_COPY
  void Copy( CphxMesh *a );
#endif
#ifdef PHX_MESH_MERGE
  void Merge( class CphxModelObject_Mesh** objects, int count );
#endif
#ifdef PHX_MESH_CALCULATETINT
  void CalculateTint( int TextureChannel, ID3D11Texture2D *Texture, int Saturation );
#endif
#ifdef PHX_MESH_CALCULATETINTSHAPE
  void CalculateTintShape( int shape, int op, int power, D3DXVECTOR3 Scale, D3DXQUATERNION Rotate, D3DXVECTOR3 Translate );
#endif
#ifdef PHX_MESH_MAPXFORM
  void MapXForm( D3DXVECTOR3 Scale, D3DXQUATERNION Rotate, D3DXVECTOR3 Translate );
#endif
#ifdef PHX_MESH_BEVEL
  void Bevel( float Percent );
#endif
#ifdef PHX_MESH_CREATEGEOSPHERE
  void CreateGeoSphere( int Iterations );
#endif
#ifdef PHX_MESH_CREATEMARCHINGMESH

  //struct
  //{
  //  char type;
  //  D3DXFLOAT16 power;
  //};
  void CreateMarchingMesh( D3DXVECTOR3 dimensions, int objCount, char *objType, D3DXMATRIX *objPositions, float surface, char resolution );
#endif
#ifdef PHX_MESH_SCATTER
  void Scatter( CphxMesh *Object, D3DXMATRIX *ObjPos, CphxMesh *Shape, D3DXMATRIX *ShapePos, unsigned char RandSeed, float VertexProbability, float EdgeProbability, float PolyProbability, unsigned char MaxPerPoly, unsigned char ProbabilityTint, PHXSCATTERORIENTATIONTYPE Orientation, float ScaleThreshold, bool ForceYScale, float OffsetThreshold, unsigned char ScaleOffsetTint );
#endif

#if defined(PHX_MESH_COPY) || defined(PHX_MESH_SCATTER) || defined(PHX_MESH_MERGE)
  void CopyInstance( CphxMesh *a, D3DXMATRIX *Transformation );
#endif

#ifdef PHX_MESH_LOADSTOREDMESH
  void LoadStoredMesh( CphxVertex *vx, int vxc, CphxPolygon *pl, int plc );
#endif
#ifdef PHX_MESH_LOADSTOREDMINIMESH
  void LoadStoredMiniMesh( unsigned char* vertices, int vxc, unsigned char* tris, int tricount );
#endif
#ifdef PHX_MESH_CREATETREE
  void CreateTree( unsigned char Seed, unsigned char* levelDensities, TREESPECIESDESCRIPTOR &Species );
#endif
#ifdef PHX_MESH_CREATETREELEAVES
  void CreateTreeLeaves( unsigned char Seed, unsigned char* levelDensities, TREESPECIESDESCRIPTOR &Species );
#endif
#ifdef PHX_MESH_CREATETEXT
  void BInterp( D3DXVECTOR3 * pPoints, int * nPoints, D3DXVECTOR2 p1, D3DXVECTOR2 p2, D3DXVECTOR2 p3, float fDeviation );
  void CreateText( unsigned char Font, char *Text, unsigned char Deviation );
#endif
#ifdef PHX_MESH_REPLICATE
  void Replicate( unsigned char Count, D3DXFLOAT16 *Srt );
#endif
#ifdef PHX_MESH_SMOOTH
  void CatmullClark( bool Linear, int Iterations );
#endif
#ifdef PHX_MESH_NORMALDEFORM
  void NormalDeform( float factor );
#endif
#ifdef PHX_MESH_CSG
  void CSG( CphxMesh *object, D3DXMATRIX *objPos, int operation );
#endif

#ifdef PHX_MESH_GREEBLE
  void Greeble_SplitPoly( int ply );
  void Greeble( int seed, float extrude, float taper );
#endif

#ifdef PHX_MESH_INVERT
  void Invert();
#endif

#ifdef PHX_MESH_SAVEPOS2
  void SavePos2();
#endif
};
