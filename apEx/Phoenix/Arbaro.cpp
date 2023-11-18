#include "Arbaro.h"
#include "Mesh.h"

#if defined PHX_MESH_CREATETREE || defined PHX_MESH_CREATETREELEAVES

#define PI 3.14159265f

namespace Arbaro
{

  int idx = 0;

#define MIN_STEM_LEN (0.00001)
#define MIN_STEM_RADIUS (MIN_STEM_LEN / 10)

  static char leafVertexData[ 4 ][ 2 ] = { { -1, 0 },{ 1, 0 },{ 1, 2 },{ -1, 2 } };
  static float leafUVData[ 6 ] = { 0, 0, 0, 1, 1, 0 };

  float var( float variation )
  {
    return ( rand() / ( (float)( RAND_MAX ) ) * 2 - 1 )*variation;
  }

  int PropagateError( float &err, float Val )
  {
    int eff = (int)( Val + err + 0.5 );
    err -= ( eff - Val );
    return eff;
  }

  float getShapeRatio( float ratio, Params::ShapeType shape )
  {
    switch ( shape )
    {
#ifdef PHX_ARBARO_HAS_SHAPE_CONICAL
    case Params::CONICAL:
      return ratio;
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_SPHERICAL
    case Params::SPHERICAL:
      return 0.2f + 0.8f*(float)sin( PI*ratio );
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_HEMISPHERICAL
    case Params::HEMISPHERICAL:
      return 0.2f + 0.8f*(float)sin( 0.5*PI*ratio );
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_CYLINDRICAL
    case Params::CYLINDRICAL:
      return 1.0f;
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_TAPERED_CYLINDRICAL
    case Params::TAPERED_CYLINDRICAL:
      return 0.5f + 0.5f*ratio;
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_FLAME
    case Params::FLAME:
      return ratio <= 0.7f ? ratio / 0.7f : ( 1 - ratio ) / 0.3f;
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_INVERSE_CONICAL
    case Params::INVERSE_CONICAL:
      return 1 - 0.8f*ratio;
#endif
#ifdef PHX_ARBARO_HAS_SHAPE_TEND_FLAME
    case Params::TEND_FLAME:
      return ratio <= 0.7f ? 0.5f + 0.5f*ratio / 0.7f : 0.5f + 0.5f*( 1 - ratio ) / 0.3f;
#endif
    }
    return 0;
  }

  LevelParams & Params::getLevelParams( int x )
  {
    return lparams[ min( 3, x ) ];
  }

  Params::Params( TREESPECIESDESCRIPTOR& desc, unsigned char Seed )
  {
    leavesErrorValue = 0;

    _0BaseSplits = desc.Parameters._0BaseSplits;
    Levels = desc.Parameters.Levels;
    Leaves = desc.Parameters.Leaves;
    scale_tree = desc.Parameters.Scale;

    BaseSize = desc.Parameters.BaseSize / 255.0f;
    Shape = (ShapeType)desc.Parameters.Shape;

    Ratio = desc.Parameters.Ratio / 2048.0f;
    RatioPower = desc.Parameters.RatioPower / 32.0f;
    Flare = desc.Parameters.Flare / 25.5f - 1.0f;
    Lobes = desc.Parameters.Lobes;
    LobeDepth = desc.Parameters.LobeDepth;
    _0Scale = desc.Parameters._0Scale;
    LeafQuality = desc.Parameters.LeafQuality / 255.0f;
    LeafDistrib = (ShapeType)desc.Parameters.LeafDistrib;
    AttractionUp = desc.Parameters.AttractionUp / 12.7f;
    LeafBend = desc.Parameters.LeafBend / 255.0f;
    LeafScale = desc.Parameters.LeafScale;
    LeafScaleX = desc.Parameters.LeafScaleX;
    LeafStemLen = desc.Parameters.LeafStemLen;
    _0ScaleV = desc.Parameters._0ScaleV;
    Smooth = desc.Parameters.Smooth / 255.0f;

    PruneRatio = desc.Parameters.PruneRatio / 255.0f;
    PruneWidth = desc.Parameters.PruneWidth / 255.0f;
    PruneWidthPeak = desc.Parameters.PruneWidthPeak / 255.0f;
    PrunePowerHigh = desc.Parameters.PrunePowerHigh;
    PrunePowerLow = desc.Parameters.PrunePowerLow;

    for ( int x = 0; x < 4; x++ )
    {
      LevelParams &p = lparams[ x ];
      TREELEVELPARAMETERS& tp = desc.Levels[ x ];
      p.level = x;
      p.nBranches = tp.nBranches;
      p.nBranchDist = tp.nBranchDist / 255.0f;
      p.nRotate = tp.nRotate;
      p.nRotateV = tp.nRotateV;
      p.nDownAngle = tp.nDownAngle;
      p.nDownAngleV = tp.nDownAngleV;
      p.nCurveRes = tp.nCurveRes;
      p.nSegSplits = tp.nSegSplits;
      p.nLength = tp.nLength;
      p.nLengthV = tp.nLengthV;
      p.nTaper = tp.nTaper / 255.0f*3.0f;
      p.nCurve = tp.nCurve;
      p.nCurveV = tp.nCurveV;
      p.nCurveBack = tp.nCurveBack;
      p.nSplitAngle = tp.nSplitAngle;
      p.nSplitAngleV = tp.nSplitAngleV;
      p.mesh_points = 4 - x;
    }

    srand( Seed );

    if ( Lobes > 0 )
      lparams[ 0 ].mesh_points = max( ( (int)( Lobes*( pow( 2.0, (int)( 1 + 2.5*Smooth ) ) ) ) ), (int)( 4 * ( 1 + 2 * Smooth ) ) );

    for ( int i = 1; i < 4; i++ )
      lparams[ i ].mesh_points = max( 3, (int)( lparams[ i ].mesh_points*( 1 + 1.5*Smooth ) ) );
  }

  Transformation::Transformation( D3DXMATRIX& m, D3DXVECTOR3& v )
  {
    Rotation = m;
    Position = v;
  }

  Transformation::Transformation()
  {
    D3DXMatrixIdentity( &Rotation );
    Position = *(D3DXVECTOR3*)leafUVData;
  }

  D3DXVECTOR3 Transformation::apply( D3DXVECTOR3& p )
  {
    D3DXVECTOR3 v;
    D3DXMATRIX rm;
    D3DXMatrixTranspose( &rm, &Rotation );
    D3DXVec3TransformNormal( &v, &p, &rm );
    return v + Position;
  }

  D3DXVECTOR3 Transformation::getZ()
  {
    return D3DXVECTOR3( Rotation.m[ 0 ][ 2 ], Rotation.m[ 1 ][ 2 ], Rotation.m[ 2 ][ 2 ] );
  }

  void Transformation::rotx( float angle )
  {
    float radAngle = angle*PI / 180;
    D3DXMATRIX rm;
    D3DXMatrixMultiply( &Rotation, &Rotation, D3DXMatrixRotationX( &rm, -radAngle ) );
  }

#ifdef PHX_ARBARO_HAVE_FAN_LEAVES
  Transformation Transformation::roty( float angle )
  {
    float radAngle = angle*PI / 180;

    D3DXMATRIX rm;
    D3DXMatrixRotationY( &rm, radAngle );
    return Transformation( *D3DXMatrixMultiply( &rm, &Rotation, &rm ), Position );
  }
#endif

  Transformation Transformation::translate( D3DXVECTOR3& v )
  {
    return Transformation( Rotation, Position + v );
  }

  void Transformation::rotaxisz( float delta, float rho )
  {
    float radDelta = delta*PI / 180;
    float radRho = rho*PI / 180;

    D3DXMATRIX rm;
    D3DXMatrixMultiply( &Rotation, &Rotation, D3DXMatrixRotationAxis( &rm, &D3DXVECTOR3( cosf( radRho ), sinf( radRho ), 0 ), -radDelta ) );
  }

  void Transformation::rotaxis( float angle, D3DXVECTOR3& axis )
  {
    float radAngle = angle*PI / 180;
    D3DXVECTOR3 normAxis;
    D3DXVec3Normalize( &normAxis, &axis );
    D3DXMATRIX rm;
    D3DXMatrixMultiply( &Rotation, D3DXMatrixRotationAxis( &rm, &normAxis, -radAngle ), &Rotation );
  }

  Transformation Transformation::rotxz( float delta, float rho )
  {
    float radDelta = delta*PI / 180;
    float radRho = rho*PI / 180;

    D3DXMATRIX rm;
    D3DXMATRIX rx, rz;
    D3DXMatrixMultiply( &rm, D3DXMatrixRotationZ( &rx, -radRho ), D3DXMatrixRotationX( &rz, -radDelta ) );
    return Transformation( *D3DXMatrixMultiply( &rm, &Rotation, &rm ), Position );
  }

  LeafImpl::LeafImpl( Transformation& trf )
  {
    transf = trf;
  }

  void LeafImpl::make( Params& par )
  {
    if ( par.LeafBend == 0 )
      return;

    transf.rotaxis( par.LeafBend*( atan2( transf.Position.y, transf.Position.x ) - atan2( transf.Rotation.m[ 1 ][ 1 ], transf.Rotation.m[ 0 ][ 1 ] ) ) * 180 / PI, *(D3DXVECTOR3*)( leafUVData + 1 ) );
    transf.rotx( par.LeafBend*atan2( sqrt( transf.Rotation.m[ 0 ][ 1 ] * transf.Rotation.m[ 0 ][ 1 ] + transf.Rotation.m[ 1 ][ 1 ] * transf.Rotation.m[ 1 ][ 1 ] ), transf.Rotation.m[ 2 ][ 1 ] ) * 180 / PI );
  }

  void LeafImpl::BuildMesh( int branchIdx, CphxMesh *mesh, float leafScale, float leafScaleX, float leafStemLen )
  {
    int vxc = mesh->Vertices.NumItems();

    D3DXVECTOR3 root = transf.apply( D3DXVECTOR3( 0, 0, 0 ) );
    D3DXVECTOR3 dir = transf.apply( D3DXVECTOR3( 0, 0, 1 ) ) - root;

    for ( int x = 0; x < 4; x++ )
    {
      D3DXVECTOR3 &vx = transf.apply( D3DXVECTOR3( leafVertexData[ x ][ 0 ] * leafScaleX, 0, leafVertexData[ x ][ 1 ] + leafStemLen * 2 )*leafScale*0.5f );

      mesh->AddVertex( vx.x, vx.z, vx.y ); //XZY!!
      CphxVertex &vert = mesh->Vertices[ mesh->Vertices.NumItems() - 1 ];
      vert.Position2 = D3DXVECTOR3( root.x, root.z, root.y );
      //vert.Position2.x = (float)branchIdx;
    }

    mesh->AddPolygon( vxc + 0, vxc + 1, vxc + 2, vxc + 3, *(D3DXVECTOR2*)leafUVData, *(D3DXVECTOR2*)( leafUVData + 4 ), *(D3DXVECTOR2*)( leafUVData + 3 ), *(D3DXVECTOR2*)( leafUVData + 2 ) );

    for ( int x = 0; x < 4; x++ )
    {
      CphxPolygon &p = mesh->Polygons[ mesh->Polygons.NumItems() - 1 ];
      p.Texcoords[ x ][ 2 ].x = dir.x;
      p.Texcoords[ x ][ 2 ].y = dir.z;
      p.Texcoords[ x ][ 3 ].x = dir.y;
    }
  }

  SegmentImpl::~SegmentImpl()
  {
    subsegments.FreeArray();
  }

  void SegmentImpl::make()
  {
    int cnt = 10;

#ifdef PHX_ARBARO_HAVE_HELIX_SUPPORT
    //helix
    if ( lpar.nCurveV < 0 )
    {
      float angle = cosf( fabs( lpar.nCurveV ) / 180 * PI );
      float rad = sqrtf( 1.0f / ( angle*angle ) - 1 )*length / PI / 2.0f;

      for ( int i = 1; i < cnt + 1; i++ )
      {
        D3DXVECTOR3 pos( (float)( rad*cos( 2 * PI*i / cnt ) - rad ),
          (float)( rad*sin( 2 * PI*i / cnt ) ),
                         (float)( i*length / cnt ) );
        subsegments.Add( new SubsegmentImpl( transf.apply( pos ), stem->stemRadius( index*length + i*length / cnt ), i*length / cnt, this ) );
      }
      return;
    }
#endif

    D3DXVECTOR3 dir = transf.getZ()*(float)length;
    D3DXVECTOR3 upperPos = transf.Position + dir;

    //spherical end
    if ( lpar.nTaper > 1 && lpar.nTaper <= 2 && ( index == stem->stemdata.segmentCount - 1 ) )
    {
      for ( int i = 1; i < cnt; i++ )
      {
        float pos = length - length / powf( 2.0f, (float)i );
        subsegments.Add( new SubsegmentImpl( transf.Position + ( dir*(float)( pos / length ) ), stem->stemRadius( index*length + pos ), pos, this ) );
      }
      subsegments.Add( new SubsegmentImpl( upperPos, _rad2, length, this ) );
      return;
    }

    cnt = 1;

    if ( lpar.nTaper <= 2 )
    {
      //flare
      if ( lpar.level == 0 && par.Flare != 0 && index == 0 )
      {
        for ( int i = 9; i >= 0; i-- )
        {
          float pos = length / powf( 2.0f, (float)i );
          subsegments.Add( new SubsegmentImpl( transf.Position + ( dir*(float)( pos / length ) ), stem->stemRadius( index*length + pos ), pos, this ) );
        }
        return;
      }
    }
    else
      cnt = 20; //normal

    for ( int i = 1; i < cnt + 1; i++ )
    {
      float pos = i*length / cnt;
      subsegments.Add( new SubsegmentImpl( transf.Position + ( dir*(float)( pos / length ) ), stem->stemRadius( index*length + pos ), pos, this ) );
    }
  }

  __forceinline Transformation SegmentImpl::substemPosition( Transformation& trf, float _where )
  {
#ifdef PHX_ARBARO_HAVE_HELIX_SUPPORT
    if ( lpar.nCurveV >= 0 )
#endif
      return trf.translate( transf.getZ()*(float)( _where*length ) );

#ifdef PHX_ARBARO_HAVE_HELIX_SUPPORT    // helix
    int i = (int)( _where*( subsegments.NumItems() - 1 ) );
    D3DXVECTOR3 p1 = subsegments[ i ]->pos;
    D3DXVECTOR3 p2 = subsegments[ i + 1 ]->pos;
    D3DXVECTOR3 pos = p1 + ( ( p2 - p1 )*(float)( _where - i / ( subsegments.NumItems() - 1 ) ) );
    return trf.translate( pos - transf.Position );
#endif
  }

  SegmentImpl::SegmentImpl( StemImpl* stm, int inx, Transformation& trf, float r1, float r2 )
    : lpar( stm->lpar ),
    par( stm->par )
  {
    index = inx;
    transf = trf;
    _rad1 = r1;
    _rad2 = r2;
    stem = stm;

    //par = stem->par;
    length = stem->stemdata.segmentLength;
  }

  void SegmentImpl::getSectionPoints( int branchIdx, CphxMesh *mesh, float rad, Transformation& trf, int &counter, float branchDist1, float branchDist2, float uvscale )
  {
    int pt_cnt = lpar.mesh_points;
    int vxb = mesh->Vertices.NumItems() - pt_cnt;

    if ( rad < 0.000001 )
    {
      D3DXVECTOR3 vx = trf.apply( *(D3DXVECTOR3*)leafUVData );
      mesh->AddVertex( vx.x, vx.z, vx.y );
      CphxVertex &vert = mesh->Vertices[ mesh->Vertices.NumItems() - 1 ];
      //vert.Position2.x = (float)branchIdx;

      if ( counter )
        for ( int i = 0; i < pt_cnt; i++ )
        {
          float xc1 = i / (float)pt_cnt*uvscale;
          float xc2 = ( i + 1 ) / (float)pt_cnt*uvscale;
          mesh->AddPolygon( vxb + pt_cnt, vxb + ( i + 1 ) % pt_cnt, vxb + i, vxb + i, D3DXVECTOR2( ( xc1 + xc2 ) / 2.0f, branchDist2 ), D3DXVECTOR2( xc1, branchDist1 ), D3DXVECTOR2( xc2, branchDist1 ), D3DXVECTOR2( 0, 0 ) );
        }
    }
    else
    {
      for ( int i = 0; i < pt_cnt; i++ )
      {
        float angle = i*360.0f / pt_cnt;
        if ( lpar.level == 0 && par.Lobes != 0 )
          angle -= 10.0f / par.Lobes;

        D3DXVECTOR3 pt( cos( (float)( angle*PI / 180 ) ), sin( (float)( angle*PI / 180 ) ), 0 );

        float multiplier = rad;

        if ( lpar.level == 0 && ( par.Lobes != 0 || par._0ScaleV != 0 ) )
          multiplier = (float)( ( rad * ( 1 + var( par._0ScaleV ) / subsegments.NumItems() ) )*( 1.0 + par.LobeDepth*cos( par.Lobes*angle*PI / 180.0 ) ) );

        pt = trf.apply( pt*multiplier );
        mesh->AddVertex( pt.x, pt.z, pt.y );
        CphxVertex &vert = mesh->Vertices[ mesh->Vertices.NumItems() - 1 ];
        //vert.Position2.x = (float)branchIdx;

        if ( counter )
        {
          float xc1 = i / (float)pt_cnt*uvscale;
          float xc2 = ( i + 1 ) / (float)pt_cnt*uvscale;
          mesh->AddPolygon( vxb + i + pt_cnt, vxb + ( i + 1 ) % pt_cnt + pt_cnt, vxb + ( i + 1 ) % pt_cnt, vxb + i, D3DXVECTOR2( xc1, branchDist2 ), D3DXVECTOR2( xc2, branchDist2 ), D3DXVECTOR2( xc2, branchDist1 ), D3DXVECTOR2( xc1, branchDist1 ) );
        }
      }
    }
    counter++;
  }

  void SegmentImpl::BuildMesh( int branchIdx, CphxMesh *mesh, int &cntr, float &branchDist, float uvscale, bool isClone )
  {
    if ( !subsegments.NumItems() || cntr == 0 )
      getSectionPoints( branchIdx, mesh, (float)_rad1*( isClone ? 0.9f : 1.0f ), transf, cntr, branchDist, branchDist, uvscale );

    float last = branchDist;
    for ( int x = 0; x < subsegments.NumItems(); x++ )
    {
      D3DXVECTOR3 d = subsegments[ x ]->pos - transf.Position;
      float l = branchDist + D3DXVec3Length( &d );// / uvscale;
      getSectionPoints( branchIdx, mesh, (float)subsegments[ x ]->rad, transf.translate( d ), cntr, last, l, uvscale );
      last = l;
    }
    branchDist = last;
  }

  StemImpl::~StemImpl()
  {
    segments.FreeArray();
    clones.FreeArray();
    substems.FreeArray();
    leaves.FreeArray();
  }

  StemImpl::StemImpl( class Tree* tr, StemImpl* growsOutOf, int stlev, Transformation& trf, float offs )
    : lpar( tr->params.getLevelParams( stlev ) ),
    par( tr->params )
  {
    isClone = false;
    startSegment = 0;
    tree = tr;
    stemlevel = stlev;
    transf = trf;
    offset = offs;

    parent = growsOutOf;
    clonedFrom = growsOutOf;

    if ( growsOutOf && growsOutOf->stemlevel >= stemlevel )
      parent = growsOutOf->parent;

    //par = tree->params;

    //leavesPerSegment = 0;
    stemdata.splitCorrection = 0;

    stemdata.index = 0;

#ifdef PHX_ARBARO_HAVE_PRUNING
    stemdata.pruneTest = false;
#endif
  }

  bool StemImpl::make()
  {
    stemdata.segmentCount = lpar.nCurveRes;
    stemdata.length = stemLength();
    stemdata.segmentLength = stemdata.length / lpar.nCurveRes;
    stemdata.baseRadius = stemBaseRadius();

#ifdef PHX_ARBARO_HAVE_PRUNING
    if ( stemlevel > 0 && par.PruneRatio > 0 )
      pruning();
#endif

    if ( stemdata.length > MIN_STEM_LEN && stemdata.baseRadius > MIN_STEM_RADIUS )
    {
      prepareSubstemParams();
      makeSegments( 0, stemdata.segmentCount );
      return true;
    }

    return false;
  }

  __forceinline float StemImpl::stemLength()
  {
    if ( stemlevel == 0 )
      return ( lpar.nLength + var( lpar.nLengthV ) ) * par.scale_tree;

    if ( stemlevel == 1 )
      return parent->stemdata.length * parent->stemdata.lengthChildMax * getShapeRatio( ( parent->stemdata.length - offset ) / ( parent->stemdata.length - par.BaseSize*par.scale_tree ), par.Shape );

    return parent->stemdata.lengthChildMax*( parent->stemdata.length - 0.6f*offset );
  }

  float StemImpl::stemBaseRadius()
  {
    if ( stemlevel == 0 )
      return stemdata.length * par.Ratio;

    float max_radius = parent->stemRadius( offset );
    float radius = parent->stemdata.baseRadius * pow( stemdata.length / parent->stemdata.length, par.RatioPower );
    return min( radius, max_radius );
  }

  float StemImpl::stemRadius( float h )
  {
    float Z = min( h / stemdata.length, 1.0f );

    float unit_taper = 0;

    if ( lpar.nTaper <= 1 )
    {
      unit_taper = lpar.nTaper;
    }
    else if ( lpar.nTaper <= 2 )
    {
      unit_taper = 2 - lpar.nTaper;
    }

    float radius = stemdata.baseRadius * ( 1 - unit_taper * Z );

    if ( lpar.nTaper > 1 )
    {
      float depth;
      float Z2 = ( 1 - Z )*stemdata.length;

      if ( lpar.nTaper < 2 || Z2 < radius )
        depth = 1;
      else
        depth = lpar.nTaper - 2;

      float Z3;

      if ( lpar.nTaper < 2 )
        Z3 = Z2;
      else
        Z3 = fabs( Z2 - 2 * radius*(int)( Z2 / 2 / radius + 0.5f ) );

      if ( lpar.nTaper > 2 || Z3 < radius )
        radius = ( 1 - depth )*radius + depth*sqrt( radius*radius - ( Z3 - radius )*( Z3 - radius ) );
    }

    //flare
    if ( stemlevel == 0 )
      radius *= par._0Scale*( 1 + par.Flare * ( pow( 100, max( 0, 1 - 8 * Z ) ) - 1 ) / 100.0f );

    return radius;
  }

#ifdef PHX_ARBARO_HAVE_PRUNING
  void StemImpl::pruning()
  {
    lpar.spliterrval = lpar.splitErrorValue;
    float splitcorr = stemdata.splitCorrection;
    float origlen = stemdata.length;

    stemdata.pruneTest = true;

    int segm = makeSegments( 0, stemdata.segmentCount );

    while ( segm >= 0 && stemdata.length > 0.001*par.scale_tree )
    {
      lpar.splitErrorValue = lpar.spliterrval;
      stemdata.splitCorrection = splitcorr;

      clones.FreeArray();
      segments.FreeArray();

      stemdata.length = min( max( stemdata.segmentLength*segm, stemdata.length / 2 ), stemdata.length - origlen / 15 );

      stemdata.segmentLength = stemdata.length / lpar.nCurveRes;
      stemdata.baseRadius = stemBaseRadius();

      if ( stemdata.length > MIN_STEM_LEN )
        segm = makeSegments( 0, stemdata.segmentCount );
    }

    stemdata.length = origlen - ( origlen - stemdata.length )*par.PruneRatio;

    lpar.splitErrorValue = lpar.spliterrval;
    stemdata.splitCorrection = splitcorr;

    clones.FreeArray();
    segments.FreeArray();

    stemdata.pruneTest = false;
  }
#endif

  void StemImpl::prepareSubstemParams()
  {
    LevelParams& lpar_1 = par.getLevelParams( stemlevel + 1 );

    stemdata.lengthChildMax = lpar_1.nLength + var( lpar_1.nLengthV );

    float stems_max = (float)lpar_1.nBranches;

    float substem_cnt;
    if ( stemlevel == 0 )
    {
      substem_cnt = stems_max;
      stemdata.substemsPerSegment = substem_cnt / (float)stemdata.segmentCount / ( 1 - par.BaseSize );
    }
    else
    {
      if ( stemlevel == 1 )
      {
        substem_cnt = (float)( (int)( stems_max * ( 0.2f + 0.8f*stemdata.length / parent->stemdata.length / parent->stemdata.lengthChildMax ) ) );
        stemdata.substemsPerSegment = substem_cnt / (float)stemdata.segmentCount;
      }
      else
      {
        substem_cnt = (float)( (int)( stems_max * ( 1.0f - 0.5f * offset / parent->stemdata.length ) ) );
        stemdata.substemsPerSegment = substem_cnt / (float)stemdata.segmentCount;
      }
    }

    stemdata.substemRotangle = 0;

    if ( lpar.level == par.Levels - 1 )
      stemdata.leavesPerSegment = leavesPerBranch() / stemdata.segmentCount;
  }

  float StemImpl::leavesPerBranch()
  {
    if ( par.Leaves == 0 || stemlevel == 0 )
      return 0;

    return ( abs( par.Leaves )
             * getShapeRatio( offset / parent->stemdata.length, par.LeafDistrib )
             * par.LeafQuality );
  }

  int StemImpl::makeSegments( int start_seg, int end_seg )
  {
    Transformation trf = transf;

    for ( int s = start_seg; s < end_seg; s++ )
    {
      if ( s != 0 )
        trf = newDirection( trf, s );

      SegmentImpl* segment = new SegmentImpl( this, s, trf, stemRadius( s*stemdata.segmentLength ), stemRadius( ( s + 1 )*stemdata.segmentLength ) );
      segment->make();
      segments.Add( segment );

#ifdef PHX_ARBARO_HAVE_PRUNING
      if ( !stemdata.pruneTest )
#endif
      {
        if ( lpar.level < par.Levels - 1 )
          makeSubstems( segment );
        else //this should only happen when lpar.level==par.levels-1
          makeLeaves( segment );
      }

      trf = trf.translate( trf.getZ()*stemdata.segmentLength );

#ifdef PHX_ARBARO_HAVE_PRUNING
      if ( stemdata.pruneTest && !isInsideEnvelope( trf.Position ) )
        return s;
#endif

      if ( s < end_seg - 1 )
      {
        int segm = makeClones( trf, s );
        if ( segm >= 0 )
          return segm;
      }
    }

    return -1;
  }

  __forceinline Transformation StemImpl::newDirection( Transformation trf, int nsegm )
  {
    float delta;
    if ( lpar.nCurveBack == 0 )
    {
      delta = lpar.nCurve;
    }
    else
    {
      if ( nsegm < ( lpar.nCurveRes + 1 ) / 2 )
      {
        delta = lpar.nCurve * 2;
      }
      else
      {
        delta = lpar.nCurveBack * 2;
      }
    }
    delta = delta / lpar.nCurveRes + stemdata.splitCorrection;

    trf.rotx( delta );

    if ( lpar.nCurveV > 0 )
    {
      delta = var( lpar.nCurveV ) / lpar.nCurveRes;
      trf.rotaxisz( delta, 180 + var( 180 ) );
    }

    if ( par.AttractionUp != 0 && stemlevel >= 2 )
    {
      D3DXVECTOR3 z = trf.getZ();
      float declination = acos( z.z );
      float curve_up = par.AttractionUp * fabs( declination * sinf( declination ) ) / lpar.nCurveRes;
      trf.rotaxis( -curve_up * 180 / PI, D3DXVECTOR3( -z.y, z.x, 0 ) );
    }
    return trf;
  }

  __forceinline void StemImpl::makeSubstems( SegmentImpl* segment )
  {
    LevelParams& lpar_1 = par.getLevelParams( stemlevel + 1 );

    float subst_per_segm;
    float offs = 0;

    if ( stemlevel > 0 )
    {
      subst_per_segm = stemdata.substemsPerSegment;

      if ( segment->index == 0 )
        offs = parent->stemRadius( offset ) / stemdata.segmentLength;
    }
    else if ( segment->index*stemdata.segmentLength > par.BaseSize*stemdata.length )
    {
      subst_per_segm = stemdata.substemsPerSegment;
    }
    else if ( ( segment->index + 1 )*stemdata.segmentLength <= par.BaseSize*stemdata.length )
    {
      return;
    }
    else
    {
      offs = ( par.BaseSize*stemdata.length - segment->index*stemdata.segmentLength ) / stemdata.segmentLength;
      subst_per_segm = stemdata.substemsPerSegment*( 1 - offs );
    }

    int substems_eff = PropagateError( lpar.substemErrorValue, subst_per_segm );
    if ( substems_eff <= 0 ) return;

    float dist = ( 1.0f - offs ) / substems_eff*lpar_1.nBranchDist;

    for ( int s = 0; s < substems_eff; s++ )
    {
      float where = offs + dist / 2 + s*dist + var( dist*0.25f );
      float offset = ( segment->index + where ) * stemdata.segmentLength;

      Transformation trf = substemDirection( segment->transf, offset );
      trf = segment->substemPosition( trf, where );

      StemImpl* substem = new StemImpl( tree, this, stemlevel + 1, trf, offset );
      substem->stemdata.index = substems.NumItems();

      if ( substem->make() )
        substems.Add( substem );
    }
  }

  Transformation StemImpl::substemDirection( Transformation& trf, float offset )
  {
    LevelParams& lpar_1 = par.getLevelParams( stemlevel + 1 );

    float rotangle;
    if ( lpar_1.nRotate >= 0 )
    {
      stemdata.substemRotangle = fmod( ( stemdata.substemRotangle + lpar_1.nRotate + var( lpar_1.nRotateV ) + 360 ), 360 );
      rotangle = stemdata.substemRotangle;
    }
    else
    {
      if ( fabs( stemdata.substemRotangle ) != 1 ) stemdata.substemRotangle = 1;
      stemdata.substemRotangle = -stemdata.substemRotangle;
      rotangle = stemdata.substemRotangle * ( 180 + lpar_1.nRotate + var( lpar_1.nRotateV ) );
    }

    float downangle;
    if ( lpar_1.nDownAngleV >= 0 )
    {
      downangle = lpar_1.nDownAngle + var( lpar_1.nDownAngleV );
    }
    else
    {
      float len = ( stemlevel == 0 ) ? stemdata.length*( 1 - par.BaseSize ) : stemdata.length;
      downangle = lpar_1.nDownAngle + lpar_1.nDownAngleV*( 1 - 2 * getShapeRatio( ( stemdata.length - offset ) / len, ( Params::ShapeType )0 ) );
    }

    return trf.rotxz( downangle, rotangle );
  }

  __forceinline void StemImpl::makeLeaves( SegmentImpl* segment )
  {
#ifdef PHX_ARBARO_HAVE_FAN_LEAVES
    if ( par.Leaves > 0 )
#endif
    {
      int leaves_eff = PropagateError( par.leavesErrorValue, stemdata.leavesPerSegment );

      if ( leaves_eff <= 0 )
        return;

      float offs;
      if ( segment->index == 0 )
        offs = parent->stemRadius( offset ) / stemdata.segmentLength;
      else
        offs = 0;

      float dist = ( 1.0f - offs ) / leaves_eff;

      for ( int s = 0; s < leaves_eff; s++ )
      {
        float where = offs + dist / 2 + s*dist + var( dist / 2 );
        Transformation& trf = substemDirection( segment->transf, ( segment->index + where )*stemdata.segmentLength ).translate( segment->transf.getZ()*(float)( where*stemdata.segmentLength ) );

        LeafImpl* leaf = new LeafImpl( trf );
        leaf->make( par );
        leaves.Add( leaf );
      }
    }
#ifdef PHX_ARBARO_HAVE_FAN_LEAVES
    else
      if ( segment->index == stemdata.segmentCount - 1 )
      {
        LevelParams& lpar_1 = par.getLevelParams( stemlevel + 1 );
        int cnt = (int)( leavesPerBranch() + 0.5f );

        Transformation trf = segment->transf.translate( segment->transf.getZ()*(float)( stemdata.segmentLength ) );
        float distangle = lpar_1.nRotate / cnt;
        float varangle = lpar_1.nRotateV / cnt;
        float downangle = lpar_1.nDownAngle;
        float vardown = lpar_1.nDownAngleV;
        float offsetangle;

        if ( cnt % 2 == 1 )
        {
          LeafImpl* leaf = new LeafImpl( trf );
          leaf->make( par );
          leaves.Add( leaf );
          offsetangle = distangle;
        }
        else
        {
          offsetangle = distangle / 2;
        }

        for ( int s = 0; s < cnt / 2; s++ )
        {
          for ( int rot = 1; rot >= -1; rot -= 2 )
          {
            Transformation transf1 = trf.roty( rot*( offsetangle + s*distangle + var( varangle ) ) );
            transf1.rotx( downangle + var( vardown ) );
            LeafImpl* leaf = new LeafImpl( transf1 );
            leaf->make( par );
            leaves.Add( leaf );
          }
        }
      }
#endif
  }

#ifdef PHX_ARBARO_HAVE_PRUNING
  bool StemImpl::isInsideEnvelope( D3DXVECTOR3& vector )
  {
    float r = sqrt( vector.x*vector.x + vector.y*vector.y );
    float ratio = ( par.scale_tree - vector.z ) / ( par.scale_tree*( 1 - par.BaseSize ) );

    float envelopeRatio;

    if ( ratio < 0 || ratio>1 )
      envelopeRatio = 0;
    else
      if ( ratio < ( 1 - par.PruneWidthPeak ) )
        envelopeRatio = pow( ratio / ( 1 - par.PruneWidthPeak ), par.PrunePowerHigh );
      else
        envelopeRatio = pow( ( 1 - ratio ) / ( 1 - par.PruneWidthPeak ), par.PrunePowerLow );

    return ( r / par.scale_tree ) < ( par.PruneWidth * envelopeRatio );
  }
#endif

  int StemImpl::makeClones( Transformation trf, int nseg )
  {
    int seg_splits_eff;

    if ( stemlevel == 0 && nseg == 0 && par._0BaseSplits > 0 )
      seg_splits_eff = par._0BaseSplits;
    else
      seg_splits_eff = PropagateError( lpar.splitErrorValue, lpar.nSegSplits );

    if ( seg_splits_eff < 1 ) return -1;

    float s_angle = 360 / (float)( seg_splits_eff + 1 );

    for ( int i = 0; i < seg_splits_eff; i++ )
    {
      StemImpl* newclone = clone( trf, nseg + 1 );

      newclone->transf = newclone->split( trf, s_angle*( 1 + i ), nseg, seg_splits_eff );

      int segm = newclone->makeSegments( nseg + 1, newclone->stemdata.segmentCount );
      if ( segm >= 0 )
        return segm;

      clones.Add( newclone );
    }

    trf = split( trf, 0, nseg, seg_splits_eff );
    return -1;
  }

  StemImpl * StemImpl::clone( Transformation& trf, int start_segm )
  {
    StemImpl *clone = new StemImpl( tree, this, stemlevel, trf, offset );
    clone->stemdata = stemdata;
    clone->isClone = true;
    clone->startSegment = start_segm;

#ifdef PHX_ARBARO_HAVE_PRUNING
    if ( !stemdata.pruneTest )
#endif
      clone->stemdata.substemRotangle += 180;
    return clone;
  }

  Transformation StemImpl::split( Transformation trf, float s_angle, int nseg, int nsplits )
  {
    int remaining_seg = stemdata.segmentCount - nseg - 1;

    float declination = acos( trf.getZ().z ) * 180 / PI;
    float split_angle = max( 0, ( lpar.nSplitAngle + var( lpar.nSplitAngleV ) - declination ) );

    trf.rotx( split_angle );

    stemdata.splitCorrection -= split_angle / remaining_seg;

    if ( s_angle > 0 )
    {
      float split_diverge;
      if ( par._0BaseSplits > 0 && stemlevel == 0 && nseg == 0 )
      {
        split_diverge = s_angle + var( lpar.nSplitAngleV );
      }
      else
      {
        split_diverge = 20 + 0.75f * ( 30 + fabs( declination - 90 ) ) * (float)pow( ( var( 1 ) + 1 ) / 2.0, 2 );
        if ( var( 1 ) >= 0 ) split_diverge = -split_diverge;
      }

      splitRotation = split_diverge;
      trf.rotaxis( split_diverge, *(D3DXVECTOR3*)( leafUVData + 1 ) );
    }

#ifdef PHX_ARBARO_HAVE_PRUNING
    if ( !stemdata.pruneTest )
#endif
      stemdata.substemsPerSegment /= (float)( nsplits + 1 );

    return trf;
  }

  static long aholdrand = 1L;

  int __cdecl arand()
  {
    return( ( ( aholdrand = aholdrand * 214013L + 2531011L ) >> 16 ) & 0x7fff );
  }

  void StemImpl::BuildMesh( int parentID, CphxMesh *mesh, unsigned char* levelDensities, BranchData *&data )
  {
    int currIdx = idx;

    data->parentIndex = parentID;
    data->Rotation = transf.Rotation;
    data->Position = transf.Position;

    data++;
    idx++;

    int cntr = 0;
    long ssrand = aholdrand;

    for ( int x = 0; x < substems.NumItems(); x++ )
      if ( arand() % 255 <= *levelDensities )
        substems[ x ]->BuildMesh( currIdx, mesh, levelDensities + 1, data );

    aholdrand = ssrand;

    for ( int x = 0; x < clones.NumItems(); x++ )
      if ( arand() % 255 <= *levelDensities )
        clones[ x ]->BuildMesh( currIdx, mesh, levelDensities + 1, data );

    aholdrand = ssrand;

    float branchDist = 0;
    float uvscale;
    if ( segments.NumItems() )
      uvscale = max( 1.0f, (int)( segments[ 0 ]->_rad1 * PI * 2 ) );

    if ( isClone && clonedFrom )
    {
      uvscale = max( 1.0f, (int)( clonedFrom->segments[ 0 ]->_rad1 * PI * 2 ) );

      //Transformation loctrf = clonedFrom->segments[ startSegment ]->transf;
      //loctrf.rotaxis( splitRotation, *(D3DXVECTOR3*)( leavUVData + 1 ) );
      //clonedFrom->segments[ startSegment ]->getSectionPoints( mesh, clonedFrom->segments[ startSegment ]->_rad1*0.8, loctrf, cntr, branchDist, branchDist, uvscale );
      //cntr++;
    }

    for ( int x = 0; x < segments.NumItems(); x++ )
      segments[ x ]->BuildMesh( currIdx, mesh, cntr, branchDist, uvscale, isClone );

  }

  void StemImpl::BuildLeaves( int parentID, CphxMesh *mesh, unsigned char* levelDensities, BranchData *&data )
  {
    int currIdx = idx;

    data->parentIndex = parentID;
    data->Rotation = transf.Rotation;
    data->Position = transf.Position;

    data++;
    idx++;

    long ssrand = aholdrand;

    for ( int x = 0; x < substems.NumItems(); x++ )
      if ( arand() % 255 <= *levelDensities )
        substems[ x ]->BuildLeaves( currIdx, mesh, levelDensities + 1, data );

    aholdrand = ssrand;

    for ( int x = 0; x < clones.NumItems(); x++ )
      if ( arand() % 255 <= *levelDensities )
        clones[ x ]->BuildLeaves( currIdx, mesh, levelDensities + 1, data );

    aholdrand = ssrand;

    for ( int x = 0; x < leaves.NumItems(); x++ )
      if ( arand() % 255 <= *levelDensities )
        leaves[ x ]->BuildMesh( currIdx, mesh, par.LeafScale, par.LeafScaleX, par.LeafStemLen );
  }

  Transformation Tree::trunkDirection( Transformation& trf, LevelParams lpar )
  {
    float rotangle;
#ifdef PHX_ARBARO_HAVE_NEGATIVE_TRUNK_ROTANGLE
    if ( lpar.nRotate >= 0 )
#endif
    {
      trunk_rotangle = fmod( ( trunk_rotangle + lpar.nRotate + var( lpar.nRotateV ) + 360 ), 360 );
      rotangle = trunk_rotangle;
    }
#ifdef PHX_ARBARO_HAVE_NEGATIVE_TRUNK_ROTANGLE
    else
    {
      if ( fabs( trunk_rotangle ) != 1 )
        trunk_rotangle = 1;
      trunk_rotangle = -trunk_rotangle;
      rotangle = trunk_rotangle * ( 180 + lpar.nRotate + var( lpar.nRotateV ) );
    }
#endif

    float downangle = lpar.nDownAngle + var( lpar.nDownAngleV );
    return trf.rotxz( downangle, rotangle );
  }

  Tree::Tree( TREESPECIESDESCRIPTOR& desc, unsigned char seed )
    : params( desc, seed )
  {

  }

  Tree::~Tree()
  {
    trunks.FreeArray();
  }

  void Tree::make()
  {
    trunk_rotangle = 0;

    Transformation transf;
    Transformation trf;

    LevelParams& lpar = params.getLevelParams( 0 );
    for ( int i = 0; i < lpar.nBranches; i++ )
    {
      float angle = var( 360 );
      float dist = var( lpar.nBranchDist );
      trf = trunkDirection( transf, lpar ).translate( D3DXVECTOR3( (float)( dist*sin( angle ) ), (float)( dist*cos( angle ) ), 0 ) );
      StemImpl* trunk = new StemImpl( this, nullptr, 0, trf, 0 );
      trunks.Add( trunk );
      trunk->make();
    }
  }

  int Tree::BuildTree( CphxMesh *Mesh, unsigned char* levelDensities, BranchData* branchOutput )
  {
    idx = 0;

    BranchData* data = branchOutput;

    aholdrand = 0;
    for ( int x = 0; x < trunks.NumItems(); x++ )
      trunks[ x ]->BuildMesh( -1, Mesh, levelDensities, data );
    Mesh->SmoothGroupSeparation = 2.0f;

    return idx;
  }

  int Tree::BuildLeaves( CphxMesh *Mesh, unsigned char* levelDensities, BranchData* branchOutput )
  {
    idx = 0;

    BranchData* data = branchOutput;

    aholdrand = 0;
    for ( int x = 0; x < trunks.NumItems(); x++ )
      trunks[ x ]->BuildLeaves( -1, Mesh, levelDensities, data );
    Mesh->SmoothGroupSeparation = DEFAULTSMOOTHGROUPSEPARATION;

    return idx;
  }

  SubsegmentImpl::SubsegmentImpl( D3DXVECTOR3& p, float r, float h, SegmentImpl* seg )
    : pos( p ),
    rad( r ),
    dist( h ),
    segment( seg )
  {
  }

  LevelParams::LevelParams()
    : substemErrorValue( 0 ),
    splitErrorValue( 0 )
  {
  }
}

#endif