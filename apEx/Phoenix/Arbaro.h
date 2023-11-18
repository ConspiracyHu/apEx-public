#pragma once
#include "phxarray.h"
#include "phxEngine.h"
#include "phxMath.h"
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

#if defined PHX_MESH_CREATETREE || defined PHX_MESH_CREATETREELEAVES

#define HAS_ARBARO

class CphxMesh;

enum TREESHAPE : unsigned char
{
  flareTreeShape_Conical = 0,
  flareTreeShape_Spherical = 1,
  flareTreeShape_Hemispherical = 2,
  flareTreeShape_Cylindrical = 3,
  flareTreeShape_Tapered_Cylindrical = 4,
  flareTreeShape_Flame = 5,
  flareTreeShape_Inverse_Conical = 6,
  flareTreeShape_Tend_Flame = 7,
  flareTreeShape_Envelope = 8,
};

struct TREELEVELPARAMETERS
{
  unsigned char nBranches;	//0..inf
  unsigned char nCurveRes;	//1..inf

  unsigned char nTaper;		//0..2.99999
  unsigned char nSplitAngle;	//0..180
  unsigned char nSplitAngleV;	//0..180

  unsigned char nBranchDist;	//0..1
  short nRotate;				//-360..360
  short nRotateV;				//-360..360
  short nDownAngle;			//-179.999..179.999
  short nDownAngleV;			//-179.999..179.999

  short nCurve;				//-inf..inf
  short nCurveV;				//-90..inf
  short nCurveBack;			//-inf..inf

  D3DXFLOAT16 nLength;		//0.00001..inf
  D3DXFLOAT16 nLengthV;		//0..inf
  D3DXFLOAT16 nSegSplits;		//0..inf
};

struct TREEPARAMETERS
{
  unsigned char Levels;		//0..9
  unsigned char Lobes;		//0..inf
  unsigned char _0BaseSplits;	//0..inf
  unsigned char LeafShape;	//enum
  TREESHAPE Shape;			//enum
  TREESHAPE LeafDistrib;		//enum

  unsigned char BaseSize;		//0..1
  unsigned char LeafQuality;	//0.00001..1
  unsigned char LeafBend;		//0..1
  unsigned char Smooth;		//0..1
  unsigned char Flare;		//-1..inf realFlare=Flare/25.5-1
  unsigned char Ratio;		//0.00001..inf realRatio=Ratio/2048

  char AttractionUp;			//-10..10
  short Leaves;				//-inf..inf
  char RatioPower;			//-inf..inf realRatioPower=RatioPower/32

  unsigned char PruneRatio;
  unsigned char PruneWidth;
  unsigned char PruneWidthPeak;
  D3DXFLOAT16 PrunePowerLow;
  D3DXFLOAT16 PrunePowerHigh;

  D3DXFLOAT16 LobeDepth;		//0..inf

  D3DXFLOAT16 Scale;			//0.00001..inf
  D3DXFLOAT16 _0Scale;		//0.00001..inf
  D3DXFLOAT16 _0ScaleV;		//0..inf
  D3DXFLOAT16 LeafScale;		//0.00001..inf
  D3DXFLOAT16 LeafScaleX;		//0.00001..inf
  D3DXFLOAT16 LeafStemLen;	//-inf..inf
};

struct TREESPECIESDESCRIPTOR
{
  TREELEVELPARAMETERS Levels[ 4 ];
  TREEPARAMETERS Parameters;
};

namespace Arbaro
{
  struct BranchData
  {
    D3DXMATRIX Rotation;
    D3DXVECTOR3 Position;
    int   parentIndex;
    float parentPosition;
    float baseThickness;
    float endThickness;
  };

  struct Transformation
  {
    D3DXMATRIX Rotation;
    D3DXVECTOR3 Position;

    Transformation();
    Transformation( D3DXMATRIX& m, D3DXVECTOR3& v );

    D3DXVECTOR3 apply( D3DXVECTOR3& p );
    D3DXVECTOR3 getZ();
    void rotx( float angle );
#ifdef PHX_ARBARO_HAVE_FAN_LEAVES
    Transformation roty( float angle );
#endif
    Transformation translate( D3DXVECTOR3& v );
    void rotaxisz( float delta, float rho );
    void rotaxis( float angle, D3DXVECTOR3& axis );
    Transformation rotxz( float delta, float rho );
  };

  struct LevelParams
  {
    int level;

    int nBranches;
    float nBranchDist;
    float nRotate;
    float nRotateV;
    float nDownAngle;
    float nDownAngleV;
    int nCurveRes;
    float nSegSplits;
    float nLength;
    float nLengthV;
    float nTaper;
    float nCurve;
    float nCurveV;
    float nCurveBack;
    float nSplitAngle;
    float nSplitAngleV;

    float substemErrorValue;
    float splitErrorValue;
    int mesh_points;

    LevelParams();

    long randstate;
    float spliterrval;

    void saveState();
    void restoreState();
  };

  struct Params
  {
    enum ShapeType
    {
      CONICAL = 0,
      SPHERICAL = 1,
      HEMISPHERICAL = 2,
      CYLINDRICAL = 3,
      TAPERED_CYLINDRICAL = 4,
      FLAME = 5,
      INVERSE_CONICAL = 6,
      TEND_FLAME = 7,
      ENVELOPE = 8
    };

    int _0BaseSplits;
    int Levels;
    int Leaves;
    float scale_tree;
    float BaseSize;
    ShapeType Shape;
    float PruneWidthPeak;
    float PrunePowerHigh;
    float PrunePowerLow;
    float Ratio;
    float RatioPower;
    float Flare;
    int Lobes;
    float LobeDepth;
    float _0Scale;
    float PruneRatio;
    float LeafQuality;
    ShapeType LeafDistrib;
    float AttractionUp;
    float PruneWidth;
    float LeafBend;
    float LeafScaleX;
    float LeafScale;
    float LeafStemLen;
    float _0ScaleV;
    float Smooth;

    LevelParams lparams[ 4 ];

    float leavesErrorValue;

    Params();

    LevelParams &getLevelParams( int x );
    Params( TREESPECIESDESCRIPTOR& desc, unsigned char Seed );
  };

  class LeafImpl
  {
  public:

    Transformation transf;

    LeafImpl( Transformation& trf );
    void make( Params& par );
    void BuildMesh( int branchIdx, CphxMesh *mesh, float leafScale, float leafScaleX, float leafStemLen );
  };

  class SubsegmentImpl
  {
  public:

    D3DXVECTOR3 pos;
    float rad;
    float dist;
    class SegmentImpl *segment;

    SubsegmentImpl( D3DXVECTOR3& p, float r, float h, SegmentImpl* seg );
  };

  class SegmentImpl
  {
  public:

    float length;
    float _rad1;
    float _rad2;
    class StemImpl *stem;
    D3DXVECTOR3 pos;

    Params par;
    LevelParams& lpar;

    int index;
    Transformation transf;
    CphxArray<SubsegmentImpl*> subsegments;

    ~SegmentImpl();

    void make();
    __forceinline Transformation substemPosition( Transformation& trf, float _where );

    SegmentImpl( StemImpl* stm, int inx, Transformation& trf, float r1, float r2 );

    void getSectionPoints( int branchIdx, CphxMesh *mesh, float rad, Transformation& trf, int &counter, float branchDist1, float branchDist2, float uvscale );
    void BuildMesh( int branchIdx, CphxMesh *mesh, int &cntr, float &branchDist, float uvscale, bool isClone );
  };

  class StemImpl
  {
  public:
    class Tree *tree;
    int stemlevel;
    Transformation transf;
    float offset;

    StemImpl *parent;
    StemImpl *clonedFrom;

    Params par;
    LevelParams& lpar;

    CphxArray<SegmentImpl*> segments;
    CphxArray<StemImpl*> clones;
    CphxArray<StemImpl*> substems;
    CphxArray<LeafImpl*> leaves;

    float splitRotation;

    ~StemImpl();

    struct StemData
    {
#ifdef PHX_ARBARO_HAVE_PRUNING
      bool pruneTest;
#endif
      float leavesPerSegment;
      float splitCorrection;
      int index;
      int segmentCount;
      float length;
      float lengthChildMax;
      float segmentLength;
      float baseRadius;
      float substemsPerSegment;
      float substemRotangle;
    } stemdata;

    bool isClone;
    int startSegment;

    StemImpl( Tree* tr, StemImpl* growsOutOf, int stlev, Transformation& trf, float offs );

    bool make();

    __forceinline float stemLength();
    float stemBaseRadius();
    float stemRadius( float h );
#ifdef PHX_ARBARO_HAVE_PRUNING
    void pruning();
#endif
    void prepareSubstemParams();
    float leavesPerBranch();
    int makeSegments( int start_seg, int end_seg );
    __forceinline Transformation newDirection( Transformation trf, int nsegm );
    __forceinline void makeSubstems( SegmentImpl* segment );
    Transformation substemDirection( Transformation& trf, float offset );
    __forceinline void makeLeaves( SegmentImpl* segment );
#ifdef PHX_ARBARO_HAVE_PRUNING
    bool isInsideEnvelope( D3DXVECTOR3& vector );
#endif
    int makeClones( Transformation trf, int nseg );
    StemImpl *clone( Transformation& trf, int start_segm );
    Transformation split( Transformation trf, float s_angle, int nseg, int nsplits );

    void BuildMesh( int parentID, CphxMesh *mesh, unsigned char* levelDensities, BranchData *&data );
    void BuildLeaves( int parentID, CphxMesh *mesh, unsigned char* levelDensities, BranchData *&data );
  };

  class Tree
  {
  public:

    Params params;
    float trunk_rotangle;

    Transformation trunkDirection( Transformation& trf, LevelParams lpar );

    CphxArray<StemImpl*> trunks;

    Tree( TREESPECIESDESCRIPTOR& desc, unsigned char seed );
    ~Tree();

    void make();

    int BuildTree( CphxMesh *mesh, unsigned char* levelDensities, BranchData* branchOutput );
    int BuildLeaves( CphxMesh *mesh, unsigned char* levelDensities, BranchData* branchOutput );
  };

}

#endif