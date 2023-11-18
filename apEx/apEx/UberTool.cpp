#include "BasePCH.h"
#include "UberTool.h"

CphxModelObject_Tool_Mesh *Cube = NULL;
CphxModelObject_Tool_Mesh *Sphere = NULL;
CphxModelObject_Tool_Mesh *Plane = NULL;
CphxModelObject_Tool_Mesh *Cylinder = NULL;
CCoreBlendState *BlendStateSolid = NULL;
CCoreBlendState *BlendStateAlpha = NULL;
CCoreVertexShader *VertexShader = NULL;
CCorePixelShader *PixelShader = NULL;
CCoreRasterizerState *RasterState = NULL;
CCoreDepthStencilState *DepthState = NULL;
CCoreVertexFormat *VertexFormat = NULL;

TF32 HighlightColor[ 4 ] = { 1, 1, 0, 0.9f };

void InitUberTool( CCoreDevice *Dev )
{
  if ( Cube ) return;

  phxDev = ( (CCoreDX11Device*)Dev )->GetDevice();

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif
  Cube = new CphxModelObject_Tool_Mesh;
  Cube->GetModelObject()->Mesh.CreateCube();
  Cube->GetModelObject()->Mesh.BuildMesh( Cube->GetModelObject()->VxBuffer, Cube->GetModelObject()->IndexBuffer, Cube->GetModelObject()->WireBuffer, Cube->GetModelObject()->VxCount, Cube->GetModelObject()->TriCount, Cube->GetModelObject()->EdgeCount );
  //Cube->Material = DefaultPreviewMaterial;

  Sphere = new CphxModelObject_Tool_Mesh;
  Sphere->GetModelObject()->Mesh.CreateSphere( 12, 12, 0, 1, true );
  Sphere->GetModelObject()->Mesh.BuildMesh( Sphere->GetModelObject()->VxBuffer, Sphere->GetModelObject()->IndexBuffer, Sphere->GetModelObject()->WireBuffer, Sphere->GetModelObject()->VxCount, Sphere->GetModelObject()->TriCount, Sphere->GetModelObject()->EdgeCount );
  //Sphere->Material = DefaultPreviewMaterial;

  Cylinder = new CphxModelObject_Tool_Mesh;
  Cylinder->GetModelObject()->Mesh.CreateCylinder( 12, 3, false );
  Cylinder->GetModelObject()->Mesh.BuildMesh( Cylinder->GetModelObject()->VxBuffer, Cylinder->GetModelObject()->IndexBuffer, Cylinder->GetModelObject()->WireBuffer, Cylinder->GetModelObject()->VxCount, Cylinder->GetModelObject()->TriCount, Cylinder->GetModelObject()->EdgeCount );
  //Cylinder->Material = DefaultPreviewMaterial;

  Plane = new CphxModelObject_Tool_Mesh;
  Plane->GetModelObject()->Mesh.CreatePlane( 2, 2 );
  Plane->GetModelObject()->Mesh.BuildMesh( Plane->GetModelObject()->VxBuffer, Plane->GetModelObject()->IndexBuffer, Plane->GetModelObject()->WireBuffer, Plane->GetModelObject()->VxCount, Plane->GetModelObject()->TriCount, Plane->GetModelObject()->EdgeCount );
  //Plane->Material = DefaultPreviewMaterial;
#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif

  LPCSTR  shader = _T(
    "cbuffer b : register(b0){float4x4 projmat;float4x4 viewmat;float4x4 transmat;}\n"
    "cbuffer c : register(b1){float4 color;}\n"
    "struct VSIN\n"
    "{\n"
    "	float3 Position : POSITION0;\n"
    "	float2 UV : TEXCOORD0;\n"
    "};\n\n"
    "struct VSOUT\n"
    "{\n"
    "	float4 Position : SV_POSITION;\n"
    "};\n"
    "\n"
    "VSOUT v(VSIN x)\n"
    "{\n"
    "	VSOUT k;\n"
    "	k.Position = mul(projmat,mul(viewmat,mul(transmat,float4(x.Position,1))));\n"
    "	return k;\n"
    "}\n"
    "\n"
    "float4 p(VSOUT v) : SV_TARGET0\n"
    "{\n"
    "	return color;\n"
    "}\n" );

  VertexShader = Dev->CreateVertexShader( shader, strlen( shader ), "v", "vs_5_0" );
  PixelShader = Dev->CreatePixelShader( shader, strlen( shader ), "p", "ps_5_0" );
  BlendStateSolid = Dev->CreateBlendState();
  BlendStateSolid->SetBlendEnable( 0, false );
  BlendStateAlpha = Dev->CreateBlendState();
  BlendStateAlpha->SetBlendEnable( 0, true );
  BlendStateAlpha->SetSrcBlend( 0, COREBLEND_SRCALPHA );
  BlendStateAlpha->SetDestBlend( 0, COREBLEND_INVSRCALPHA );
  DepthState = Dev->CreateDepthStencilState();
  RasterState = Dev->CreateRasterizerState();
  RasterState->SetCullMode( CORECULL_NONE );
  RasterState->SetAntialiasedLineEnable( true );

  CArray<COREVERTEXATTRIBUTE> a;
  a += COREVXATTR_POSITION3;
  a += COREVXATTR_TEXCOORD2;
  VertexFormat = Dev->CreateVertexFormat( a, VertexShader );
}

void DeinitUberTool()
{
#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif
  SAFEDELETE( Cube );
  SAFEDELETE( Sphere );
  SAFEDELETE( Cylinder );
  SAFEDELETE( Plane );
  SAFEDELETE( DepthState );
  SAFEDELETE( BlendStateSolid );
  SAFEDELETE( BlendStateAlpha );
  SAFEDELETE( VertexShader );
  SAFEDELETE( PixelShader );
  SAFEDELETE( VertexFormat );
  SAFEDELETE( RasterState );
#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif
}

//////////////////////////////////////////////////////////////////////////
// ubertool gizmo

CUberToolGizmo::CUberToolGizmo( CCoreDevice *dev )
{
  Device = dev;
  Group = 0;
  Visible = true;
  VertexBuffer = NULL;
  WireIndexBuffer = NULL;
  SolidIndexBuffer = NULL;
  LineCount = 0;
  TriCount = 0;
  LineCount = 0;

  ColorData = Device->CreateConstantBuffer();

  D3DXMatrixIdentity( &Transformation );

  for ( int x = 0; x < 4; x++ ) Color[ x ] = 0.5f;
}

CUberToolGizmo::~CUberToolGizmo()
{
  SAFEDELETE( ColorData );
  SAFEDELETE( VertexBuffer );
  SAFEDELETE( WireIndexBuffer );
  SAFEDELETE( SolidIndexBuffer );
}

void CUberToolGizmo::SetWireColor( bool Highlight )
{
  float Col[ 4 ];
  for ( int x = 0; x < 4; x++ )
    Col[ x ] = Highlight ? HighlightColor[ x ] : Color[ x ] * 0.7f;

  ColorData->Reset();
  ColorData->AddData( &Col, 16 );
  ColorData->Upload();

  Device->SetShaderConstants( 1, 1, &ColorData );
  //Device->SetPixelShaderConstantF(0, Col, 1);
}

void CUberToolGizmo::SetSolidColor( bool Highlight )
{
  float Col[ 4 ];
  for ( int x = 0; x < 4; x++ )
    Col[ x ] = Highlight ? HighlightColor[ x ] : Color[ x ] * 0.7f;

  ColorData->Reset();
  ColorData->AddData( &Col, 16 );
  ColorData->Upload();

  Device->SetShaderConstants( 1, 1, &ColorData );
  //Device->SetPixelShaderConstantF(0, Col, 1);
}

void CUberToolGizmo::Display( bool Highlight )
{
  Device->SetPixelShader( PixelShader );
  Device->SetVertexShader( VertexShader );
  Device->SetVertexFormat( VertexFormat );
  Device->SetVertexBuffer( VertexBuffer, 0 );
  Device->SetRenderState( BlendStateSolid );
  Device->SetRenderState( RasterState );
  Device->SetRenderState( DepthState );

  if ( WireIndexBuffer )
  {
    SetWireColor( Highlight );
    Device->SetIndexBuffer( WireIndexBuffer );
    Device->DrawIndexedLines( LineCount, VertexCount );
  }
  if ( SolidIndexBuffer )
  {
    SetSolidColor( Highlight );
    Device->SetIndexBuffer( SolidIndexBuffer );
    Device->DrawIndexedTriangles( TriCount, VertexCount );
  }
}

void CUberToolGizmo::SetType( UBERTOOLGIZMOTYPE t )
{
  Type = t;
  switch ( t )
  {
  case ubertool_MoveX:
  case ubertool_MoveYZ:
  case UberTool_RotateX:
  case UberTool_ScaleX:
    SetColor( 204 / 255.0f / 0.7f, 122 / 255.0f / 0.7f, 0 );
    break;
  case ubertool_MoveY:
  case ubertool_MoveXZ:
  case UberTool_RotateY:
  case UberTool_ScaleY:
    SetColor( 0, 204 / 255.0f / 0.7f, 122 / 255.0f / 0.7f );
    break;
  case ubertool_MoveZ:
  case ubertool_MoveXY:
  case UberTool_RotateZ:
  case UberTool_ScaleZ:
    SetColor( 0, 122 / 255.0f / 0.7f, 204 / 255.0f / 0.7f );
    break;
  case UberTool_ScaleUniform:
    SetColor( 1, 1, 0 );
    break;
  }
}

UBERTOOLGIZMOTYPE CUberToolGizmo::GetType()
{
  return Type;
}

void CUberToolGizmo::SetColor( float r, float g, float b )
{
  Color[ 0 ] = r;
  Color[ 1 ] = g;
  Color[ 2 ] = b;
}

//////////////////////////////////////////////////////////////////////////
// move ubertool

CUberToolGizmo_Move_Axis_Standard::CUberToolGizmo_Move_Axis_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int SegmentCount = 48;
  int VxDataSize = ( SegmentCount + 3 )*VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];

  WireIndexBuffer = Device->CreateIndexBuffer( 2 );
  SolidIndexBuffer = Device->CreateIndexBuffer( SegmentCount * 3 );

  float *VertexData = (float*)VxData;
  short *WireIndexData = NULL;
  short *SolidIndexData = NULL;

  TriCount = 0;

  WireIndexBuffer->Lock( (void**)&WireIndexData );
  *WireIndexData++ = 0;
  *WireIndexData++ = 2;
  WireIndexBuffer->UnLock();

  SolidIndexBuffer->Lock( (void**)&SolidIndexData );

  //vx1
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx2
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 1.5;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 1.4f;
  *VertexData++ = 0;
  *VertexData++ = 0;

  VertexCount = 2;

  for ( int x = 0; x < SegmentCount; x++ )
  {
    float t = x / (float)SegmentCount*3.14159265f*2.0f;
    *VertexData++ = sin( t )*0.05f;
    *VertexData++ = cos( t )*0.05f;
    *VertexData++ = 1.4f;
    *VertexData++ = 0;
    *VertexData++ = 0;
    *SolidIndexData++ = x + 3;
    *SolidIndexData++ = 1;
    *SolidIndexData++ = ( x + 1 ) % SegmentCount + 3;
    VertexCount++;
    TriCount++;
  }

  SolidIndexBuffer->UnLock();

  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );

  LineCount = 1;
}

bool CUberToolGizmo_Move_Axis_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float pickradius = 0.1f;
  int SegmentCount = 48;

  bool picked = false;

  for ( int x = 0; x < SegmentCount; x++ )
  {
    D3DXVECTOR3 v[ 4 ];
    v[ 0 ].z = v[ 1 ].z = 0.3f;
    v[ 2 ].z = v[ 3 ].z = 1.5f;

    float r = x / (float)SegmentCount*3.14159265f*2.0f;
    v[ 3 ].x = v[ 0 ].x = sin( r )*pickradius;
    v[ 3 ].y = v[ 0 ].y = cos( r )*pickradius;

    r = ( ( x + 1 ) % SegmentCount ) / (float)SegmentCount*3.14159265f*2.0f;
    v[ 1 ].x = v[ 2 ].x = sin( r )*pickradius;
    v[ 1 ].y = v[ 2 ].y = cos( r )*pickradius;

    float a, b, c;
    if ( D3DXIntersectTri( &v[ 0 ], &v[ 1 ], &v[ 2 ], &RayPoint, &RayDir, &a, &b, &c ) )
    {
      if ( !picked ) t = c;
      picked = true;
      if ( c < t ) t = c;
    }
    if ( D3DXIntersectTri( &v[ 0 ], &v[ 2 ], &v[ 3 ], &RayPoint, &RayDir, &a, &b, &c ) )
    {
      if ( !picked ) t = c;
      picked = true;
      if ( c < t ) t = c;
    }
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Move_Axis_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  float t;
  if ( fabs( RayDir.y ) > 0.0001f )
    t = -RayPoint.y / RayDir.y;
  else
    t = -RayPoint.x / RayDir.x;
  return RayPoint + t*RayDir;
}

TF32 round( TF32 x, TF32 y )
{
  return y * floor( x / y );
}

void CUberToolGizmo_Move_Axis_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  D3DXVECTOR3 d = ( CurrPos - PickPos )*mult;
  d.x = d.y = 0;
  if ( Snap )
    d.z = round( d.z, SnapValue );
  D3DXVECTOR4 o;
  D3DXVec3Transform( &o, &d, &Transformation );
  srt.Translation = CVector3( o.x, o.y, o.z );
}

CUberToolGizmo_Move_Plane_Standard::CUberToolGizmo_Move_Plane_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int VxDataSize = 4 * VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];

  WireIndexBuffer = Device->CreateIndexBuffer( 8 );
  SolidIndexBuffer = Device->CreateIndexBuffer( 2 * 3 );

  float *VertexData = (float*)VxData;
  short *WireIndexData = NULL;
  short *SolidIndexData = NULL;

  TriCount = 2;
  LineCount = 4;
  VertexCount = 4;

  WireIndexBuffer->Lock( (void**)&WireIndexData );
  *WireIndexData++ = 0;
  *WireIndexData++ = 1;
  *WireIndexData++ = 1;
  *WireIndexData++ = 2;
  *WireIndexData++ = 2;
  *WireIndexData++ = 3;
  *WireIndexData++ = 3;
  *WireIndexData++ = 0;
  WireIndexBuffer->UnLock();

  SolidIndexBuffer->Lock( (void**)&SolidIndexData );
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 1;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 3;
  SolidIndexBuffer->UnLock();

  float size = 0.3f;

  //vx1
  *VertexData++ = 0;
  *VertexData++ = size;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx2
  *VertexData++ = size;
  *VertexData++ = size;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = size;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );
}

bool CUberToolGizmo_Move_Plane_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float size = 0.3f;

  bool picked = false;

  float a, b, c;
  if ( D3DXIntersectTri( &D3DXVECTOR3( size, size, 0 ), &D3DXVECTOR3( 0, size, 0 ), &D3DXVECTOR3( size, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }
  if ( D3DXIntersectTri( &D3DXVECTOR3( size, 0, 0 ), &D3DXVECTOR3( 0, size, 0 ), &D3DXVECTOR3( 0, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Move_Plane_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  float t = -RayPoint.z / RayDir.z;
  return RayPoint + t*RayDir;
}

void CUberToolGizmo_Move_Plane_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  D3DXVECTOR3 d = ( CurrPos - PickPos )*mult;
  d.z = 0;
  if ( Snap )
  {
    d.x = round( d.x, SnapValue );
    d.y = round( d.y, SnapValue );
  }
  D3DXVECTOR4 o;
  D3DXVec3Transform( &o, &d, &Transformation );
  srt.Translation = CVector3( o.x, o.y, o.z );
}

void CUberToolGizmo_Move_Plane_Standard::SetSolidColor( bool Highlight )
{
  float Col[ 4 ];
  for ( int x = 0; x < 4; x++ )
    Col[ x ] = Highlight ? HighlightColor[ x ] : Color[ x ] * 0.7f;

  Col[ 3 ] = Highlight ? 0.8f : 0.5f;

  Device->SetRenderState( BlendStateAlpha );
  ColorData->Reset();
  ColorData->AddData( &Col, 16 );
  ColorData->Upload();

  Device->SetShaderConstants( 1, 1, &ColorData );
}

//////////////////////////////////////////////////////////////////////////
// rotate ubertool

CUberToolGizmo_Rotation_Standard::CUberToolGizmo_Rotation_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int SegmentCount = 48;
  int VxDataSize = SegmentCount*VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];
  WireIndexBuffer = Device->CreateIndexBuffer( SegmentCount * 2 );

  float *VertexData = (float*)VxData;
  short *IndexData = NULL;

  WireIndexBuffer->Lock( (void**)&IndexData );

  for ( int x = 0; x < SegmentCount; x++ )
  {
    float t = x / (float)SegmentCount*3.14159265f*2.0f;
    *VertexData++ = sin( t );
    *VertexData++ = cos( t );
    *VertexData++ = 0;
    *VertexData++ = 0;
    *VertexData++ = 0;
    *IndexData++ = x;
    *IndexData++ = ( x + 1 ) % SegmentCount;
  }

  WireIndexBuffer->UnLock();
  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );

  LineCount = VertexCount = SegmentCount;
}

bool CUberToolGizmo_Rotation_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float pickradius = 0.1f;
  int SegmentCount = 48;
  int SegmentCount2 = 12;

  t = 10000;
  bool picked = false;

  for ( int x = 0; x < SegmentCount; x++ )
  {
    for ( int z = 0; z < SegmentCount2; z++ )
    {
      D3DXVECTOR3 v[ 4 ];
      for ( int y = 0; y < 2; y++ )
      {
        float t_ = ( ( x + y ) % SegmentCount ) / (float)SegmentCount*3.14159265f*2.0f;

        D3DXVECTOR3 center = D3DXVECTOR3( sin( t_ ), cos( t_ ), 0 );
        D3DXVECTOR3 _x = center;
        D3DXVec3Normalize( &_x, &_x );
        D3DXVECTOR3 _y = D3DXVECTOR3( 0, 0, 1 );

        for ( int k = 0; k < 2; k++ )
        {
          float t2 = ( ( z + k ) % SegmentCount2 ) / (float)SegmentCount2*3.14159265f*2.0f;
          v[ y * 2 + k ] = center + _x*pickradius*sin( t2 ) + _y*pickradius*cos( t2 );
        }
      }

      float a, b, c;
      if ( D3DXIntersectTri( &v[ 0 ], &v[ 1 ], &v[ 3 ], &RayPoint, &RayDir, &a, &b, &c ) )
      {
        if ( !picked ) t = c;
        picked = true;
        if ( c < t ) t = c;
      }
      if ( D3DXIntersectTri( &v[ 0 ], &v[ 3 ], &v[ 2 ], &RayPoint, &RayDir, &a, &b, &c ) )
      {
        if ( !picked ) t = c;
        picked = true;
        if ( c < t ) t = c;
      }
    }
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Rotation_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  float t;
  if ( StartPos && Pick( RayPoint, RayDir, t ) )
  {
    //this should perfectly pick on the circle each time
    D3DXVECTOR3 PickPos = RayPoint + t*RayDir;
    PickPos.z = 0;
    D3DXVec3Normalize( &PickPos, &PickPos );
    return PickPos;
  }

  if ( fabs( RayDir.z ) < 0.001 )
  {
    D3DXVECTOR3 sp, res;
    D3DXPLANE p;
    res = StartPickPosition;
    D3DXVec3Normalize( &sp, &StartPickPosition );
    D3DXPlaneFromPointNormal( &p, &sp, &sp );

    if ( D3DXPlaneIntersectLine( &res, &p, &RayPoint, &( RayPoint + RayDir ) ) )
    {
      res.z = 0;
      return res;
    }
  }

  //if we're not parallel to the plane this should suffice
  t = -RayPoint.z / RayDir.z;

  D3DXVECTOR3 v = RayPoint + t*RayDir;
  v.z = 0;
  return v;
}

void CUberToolGizmo_Rotation_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  //the process is as follows:
  // -the pick position is determined on the ubertool circle
  // -the tangent vector is calculated at that point and is added to the pick position (to represent a second point on the tangent line)
  // -the two points are projected back into screen space
  // -distance calculation is done in screen space along the screen space projection of the tangent line

  //DragRotateHelperPos - DragStartMousePos : tangent vector in screenspace (clientarea relative)

  D3DXVECTOR2 vd = DragRotateHelperPos - DragRotateStartMousePos;
  D3DXVECTOR2 vcurr = D3DXVECTOR2( (TF32)MousePos.x, (TF32)MousePos.y ) - DragRotateStartMousePos;

  //D3DXVECTOR3 dir = PickPos;
  //D3DXVec3Normalize(&dir, &dir);

  //D3DXVECTOR3 linenorm;
  //D3DXVec3Cross(&linenorm, &dir, &D3DXVECTOR3(0, 0, 1));

  //float distance = D3DXVec3Dot(&CurrPos, &linenorm);

  D3DXVec2Normalize( &vd, &vd );
  //D3DXVec2Normalize(&vcurr, &vcurr);

  float distance = D3DXVec2Dot( &vcurr, &vd ) / 60.0f;

  //bool neg=distance<0;

  //if (fabs(distance)>1)
  //{
  //	distance=pow((fabs(distance)),0.7f);
  //	if (neg) distance=-distance;
  //}

  if ( Snap )
  {
    distance = round( distance * 180 / 3.14152965f, SnapValue ) / 180.0f*3.14152965f;
  }
  else
    distance = ( (int)( distance * 360 / 3.14152965f ) ) / 360.0f*3.14152965f;

  D3DXVECTOR4 axis = D3DXVECTOR4( 0, 0, 1, 1 );
  D3DXVec4Transform( &axis, &axis, &Transformation );

  D3DXQUATERNION q;

  D3DXQuaternionRotationAxis( &q, &D3DXVECTOR3( axis.x, axis.y, axis.z ), -distance );
  srt.Rotation.x = q.x;
  srt.Rotation.y = q.y;
  srt.Rotation.z = q.z;
  srt.Rotation.s = q.w;
}


//SetCameraView(&View->Cam,4/3.0f,0,0,0);
//VECTOR3 Dir= CalcXYDir((lx-View->x1)/(float)(View->x2-View->x1),(ly-View->y1)/(float)(View->y2-View->y1),View->Cam.Fov,4/3.0f);
//VECTOR3 Dir2=CalcXYDir((mx-View->x1)/(float)(View->x2-View->x1),(my-View->y1)/(float)(View->y2-View->y1),View->Cam.Fov,4/3.0f);
//float matrix[16];
//MATRIX CameraMatrix;
//M_Identity(CameraMatrix);
//glGetFloatv(GL_MODELVIEW_MATRIX,matrix);
//for (int x=0; x<4; x++)
//	for (int y=0; y<4; y++)
//		CameraMatrix[y][x]=matrix[x+y*4];
//
//M_Xform3(CameraMatrix,Dir,Dir);
//M_Xform3(CameraMatrix,Dir2,Dir2);
//
//Dir: pick start irany world spaceben
//Dir2: pick pos irany world spaceben
//
//VECTOR3 mask=V3_Make(0,0,0);
//mask.a[buffer[3]]=1;
//
//float t1=distRayPlane(View->Cam.Eye,Dir,mask,Pivot.a[buffer[3]]);
//float t2=distRayPlane(View->Cam.Eye,Dir2,mask,Pivot.a[buffer[3]]);
//
//VECTOR3 P1,P2;
//
//P1=V3_Add(View->Cam.Eye,V3_Mults(Dir,t1)); //pont a koron
//P2=V3_Add(View->Cam.Eye,V3_Mults(Dir2,t2)); //kurzorpozicio
//
//VECTOR3 B=V3_Normalize(V3_Sub(P1,Pivot));
//VECTOR3 Planenormal=V3_Cross(mask,B);
//Displacement=(int)((Planenormal.x*(P2.x-Pivot.x)+Planenormal.y*(P2.y-Pivot.y)+Planenormal.z*(P2.z-Pivot.z))/radtheta*180.0f*180.0f/f);
//
//M_Rotate(mask.x,mask.y,mask.z,Displacement*(float)radtheta,m);
//RotateAxis=V3_Make(mask.x,mask.y,mask.z);
//RotateAngle=Displacement*(float)radtheta/f;
//

//////////////////////////////////////////////////////////////////////////
// scale ubertool

CUberToolGizmo_Scale_Plane_Standard::CUberToolGizmo_Scale_Plane_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int VxDataSize = 4 * VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];
  WireIndexBuffer = Device->CreateIndexBuffer( 8 );
  SolidIndexBuffer = Device->CreateIndexBuffer( 2 * 3 );

  float *VertexData = (float*)VxData;
  short *WireIndexData = NULL;
  short *SolidIndexData = NULL;

  TriCount = 2;
  LineCount = 4;
  VertexCount = 4;

  WireIndexBuffer->Lock( (void**)&WireIndexData );
  *WireIndexData++ = 0;
  *WireIndexData++ = 1;
  *WireIndexData++ = 1;
  *WireIndexData++ = 2;
  *WireIndexData++ = 2;
  *WireIndexData++ = 3;
  *WireIndexData++ = 3;
  *WireIndexData++ = 0;
  WireIndexBuffer->UnLock();

  SolidIndexBuffer->Lock( (void**)&SolidIndexData );
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 1;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 3;
  SolidIndexBuffer->UnLock();

  float size1 = 0.6f;
  float size2 = 0.9f;

  //vx1
  *VertexData++ = 0;
  *VertexData++ = size1;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx2
  *VertexData++ = 0;
  *VertexData++ = size2;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = size2;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = size1;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );
}

bool CUberToolGizmo_Scale_Plane_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float size1 = 0.6f;
  float size2 = 0.9f;

  bool picked = false;

  float a, b, c;
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, size1, 0 ), &D3DXVECTOR3( 0, size2, 0 ), &D3DXVECTOR3( size2, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, size1, 0 ), &D3DXVECTOR3( size2, 0, 0 ), &D3DXVECTOR3( size1, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Scale_Plane_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  return D3DXVECTOR3( (float)mx, (float)my, 0 ) / 30.0f;
}

void CUberToolGizmo_Scale_Plane_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  D3DXVECTOR3 d = CurrPos - PickPos;

  if ( Snap )
  {
    d.x = round( d.x, SnapValue );
    d.y = round( d.y, SnapValue );
    d.z = round( d.z, SnapValue );
  }

  D3DXVECTOR4 o;
  D3DXVec3Transform( &o, &d, &Transformation );
  srt.Scale = CVector3( 1 + o.x, 1 + o.y, 1 + o.z );
}

void CUberToolGizmo_Scale_Plane_Standard::SetSolidColor( bool Highlight )
{
  float Col[ 4 ];
  for ( int x = 0; x < 4; x++ )
    Col[ x ] = Highlight ? HighlightColor[ x ] : Color[ x ] * 0.7f;

  Col[ 3 ] = Highlight ? 0.8f : 0.5f;

  Device->SetRenderState( BlendStateAlpha );
  ColorData->Reset();
  ColorData->AddData( &Col, 16 );
  ColorData->Upload();

  Device->SetShaderConstants( 1, 1, &ColorData );
}

/////////////////////////////////////////////////////////////////////////////////////////
//scale plane uniform standard

CUberToolGizmo_Scale_Plane_Uniform_Standard::CUberToolGizmo_Scale_Plane_Uniform_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int VxDataSize = 4 * VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];
  WireIndexBuffer = Device->CreateIndexBuffer( 8 );
  SolidIndexBuffer = Device->CreateIndexBuffer( 2 * 3 );

  float *VertexData = (float*)VxData;
  short *WireIndexData = NULL;
  short *SolidIndexData = NULL;

  TriCount = 2;
  LineCount = 4;
  VertexCount = 4;

  WireIndexBuffer->Lock( (void**)&WireIndexData );
  *WireIndexData++ = 0;
  *WireIndexData++ = 1;
  *WireIndexData++ = 1;
  *WireIndexData++ = 2;
  *WireIndexData++ = 2;
  *WireIndexData++ = 3;
  *WireIndexData++ = 3;
  *WireIndexData++ = 0;
  WireIndexBuffer->UnLock();

  SolidIndexBuffer->Lock( (void**)&SolidIndexData );
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 1;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 3;
  SolidIndexBuffer->UnLock();

  float size1 = 0.3f;
  float size2 = 0.6f;

  //vx1
  *VertexData++ = 0;
  *VertexData++ = size1;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx2
  *VertexData++ = 0;
  *VertexData++ = size2;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = size2;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = size1;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );
}

bool CUberToolGizmo_Scale_Plane_Uniform_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float size1 = 0.3f;
  float size2 = 0.6f;

  bool picked = false;

  float a, b, c;
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, size1, 0 ), &D3DXVECTOR3( 0, size2, 0 ), &D3DXVECTOR3( size2, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, size1, 0 ), &D3DXVECTOR3( size2, 0, 0 ), &D3DXVECTOR3( size1, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Scale_Plane_Uniform_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  return D3DXVECTOR3( (float)mx, 0, 0 );
}

void CUberToolGizmo_Scale_Plane_Uniform_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  float v = ( CurrPos.x - PickPos.x ) / 30.0f;
  D3DXVECTOR3 d = D3DXVECTOR3( v, v, v );
  d.z = 0;
  if ( Snap )
  {
    d.x = round( d.x, SnapValue );
    d.y = round( d.y, SnapValue );
    d.z = round( d.z, SnapValue );
  }
  D3DXVECTOR4 o;
  D3DXVec3Transform( &o, &d, &Transformation );

  if ( v > 0 )
    srt.Scale = CVector3( 1 + o.x, 1 + o.y, 1 + o.z );
  else
    srt.Scale = CVector3( 1 / ( 1 - o.x ), 1 / ( 1 - o.y ), 1 / ( 1 - o.z ) );

  //srt.Scale = CVector3(1 + o.x, 1 + o.y, 1 + o.z);
}

void CUberToolGizmo_Scale_Plane_Uniform_Standard::SetSolidColor( bool Highlight )
{
  float Col[ 4 ];
  for ( int x = 0; x < 4; x++ )
    Col[ x ] = Highlight ? HighlightColor[ x ] : Color[ x ] * 0.7f;

  Col[ 3 ] = Highlight ? 0.8f : 0.5f;

  Device->SetRenderState( BlendStateAlpha );
  ColorData->Reset();
  ColorData->AddData( &Col, 16 );
  ColorData->Upload();

  Device->SetShaderConstants( 1, 1, &ColorData );
}

/////////////////////////////////////////////////////////////////////////////////////////
//scale uniform standard

CUberToolGizmo_Scale_Uniform_Standard::CUberToolGizmo_Scale_Uniform_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int VxDataSize = 4 * VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];
  WireIndexBuffer = Device->CreateIndexBuffer( 6 );
  SolidIndexBuffer = Device->CreateIndexBuffer( 9 );

  float *VertexData = (float*)VxData;
  short *WireIndexData = NULL;
  short *SolidIndexData = NULL;

  TriCount = 3;
  LineCount = 3;
  VertexCount = 4;

  WireIndexBuffer->Lock( (void**)&WireIndexData );
  *WireIndexData++ = 1;
  *WireIndexData++ = 2;
  *WireIndexData++ = 2;
  *WireIndexData++ = 3;
  *WireIndexData++ = 3;
  *WireIndexData++ = 1;
  WireIndexBuffer->UnLock();

  SolidIndexBuffer->Lock( (void**)&SolidIndexData );
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 1;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 2;
  *SolidIndexData++ = 3;
  *SolidIndexData++ = 0;
  *SolidIndexData++ = 3;
  *SolidIndexData++ = 1;
  SolidIndexBuffer->UnLock();

  float size = 0.3f;

  //vx0
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx1
  *VertexData++ = 0;
  *VertexData++ = size;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx2
  *VertexData++ = size;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = size;
  *VertexData++ = 0;
  *VertexData++ = 0;

  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );
}

bool CUberToolGizmo_Scale_Uniform_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float size = 0.3f;

  bool picked = false;

  float a, b, c;
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, 0, 0 ), &D3DXVECTOR3( 0, size, 0 ), &D3DXVECTOR3( size, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, 0, 0 ), &D3DXVECTOR3( 0, 0, size ), &D3DXVECTOR3( size, 0, 0 ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }
  if ( D3DXIntersectTri( &D3DXVECTOR3( 0, 0, 0 ), &D3DXVECTOR3( 0, size, 0 ), &D3DXVECTOR3( 0, 0, size ), &RayPoint, &RayDir, &a, &b, &c ) )
  {
    if ( !picked ) t = c;
    if ( c < t ) t = c;
    picked = true;
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Scale_Uniform_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  return D3DXVECTOR3( (float)mx, 0, 0 );
}

void CUberToolGizmo_Scale_Uniform_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  float d = ( CurrPos.x - PickPos.x ) / 30.0f;
  if ( Snap )
    d = round( d, SnapValue );

  if ( d > 0 )
    srt.Scale = CVector3( 1 + d, 1 + d, 1 + d );
  else
    srt.Scale = CVector3( 1 / ( 1 - d ), 1 / ( 1 - d ), 1 / ( 1 - d ) );
}

void CUberToolGizmo_Scale_Uniform_Standard::SetSolidColor( bool Highlight )
{
  float Col[ 4 ];
  for ( int x = 0; x < 4; x++ )
    Col[ x ] = Highlight ? HighlightColor[ x ] : Color[ x ] * 0.7f;

  Col[ 3 ] = Highlight ? 0.8f : 0.5f;

  Device->SetRenderState( BlendStateAlpha );
  ColorData->Reset();
  ColorData->AddData( &Col, 16 );
  ColorData->Upload();

  Device->SetShaderConstants( 1, 1, &ColorData );
}

/////////////////////////////////////////////////////////////////////////////////////////
//scale axis standard

CUberToolGizmo_Scale_Axis_Standard::CUberToolGizmo_Scale_Axis_Standard( CCoreDevice *dev ) : CUberToolGizmo( dev )
{
  int SegmentCount = 48;
  int VxDataSize = ( SegmentCount + 3 )*VertexFormat->GetSize();
  TU8 *VxData = new TU8[ VxDataSize ];
  WireIndexBuffer = Device->CreateIndexBuffer( 2 );
  SolidIndexBuffer = Device->CreateIndexBuffer( SegmentCount * 3 );

  float *VertexData = (float*)VxData;
  short *WireIndexData = NULL;
  short *SolidIndexData = NULL;

  TriCount = 0;

  WireIndexBuffer->Lock( (void**)&WireIndexData );
  *WireIndexData++ = 0;
  *WireIndexData++ = 2;
  WireIndexBuffer->UnLock();

  SolidIndexBuffer->Lock( (void**)&SolidIndexData );

  //vx1
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx2
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 1.5;
  *VertexData++ = 0;
  *VertexData++ = 0;

  //vx3
  *VertexData++ = 0;
  *VertexData++ = 0;
  *VertexData++ = 1.4f;
  *VertexData++ = 0;
  *VertexData++ = 0;

  VertexCount = 2;

  for ( int x = 0; x < SegmentCount; x++ )
  {
    float t = x / (float)SegmentCount*3.14159265f*2.0f;
    *VertexData++ = sin( t )*0.05f;
    *VertexData++ = cos( t )*0.05f;
    *VertexData++ = 1.4f;
    *VertexData++ = 0;
    *VertexData++ = 0;
    *SolidIndexData++ = x + 3;
    *SolidIndexData++ = 1;
    *SolidIndexData++ = ( x + 1 ) % SegmentCount + 3;
    VertexCount++;
    TriCount++;
  }

  SolidIndexBuffer->UnLock();
  VertexBuffer = Device->CreateVertexBuffer( VxData, VxDataSize );
  SAFEDELETEA( VxData );

  LineCount = 1;
}

bool CUberToolGizmo_Scale_Axis_Standard::Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t )
{
  //pick with triangles instead of exact method - it worked in addict2 well enough ;)

  float pickradius = 0.1f;
  int SegmentCount = 48;

  bool picked = false;

  for ( int x = 0; x < SegmentCount; x++ )
  {
    D3DXVECTOR3 v[ 4 ];
    v[ 0 ].z = v[ 1 ].z = 0.3f;
    v[ 2 ].z = v[ 3 ].z = 1.5f;

    float r = x / (float)SegmentCount*3.14159265f*2.0f;
    v[ 3 ].x = v[ 0 ].x = sin( r )*pickradius;
    v[ 3 ].y = v[ 0 ].y = cos( r )*pickradius;

    r = ( ( x + 1 ) % SegmentCount ) / (float)SegmentCount*3.14159265f*2.0f;
    v[ 1 ].x = v[ 2 ].x = sin( r )*pickradius;
    v[ 1 ].y = v[ 2 ].y = cos( r )*pickradius;

    float a, b, c;
    if ( D3DXIntersectTri( &v[ 0 ], &v[ 1 ], &v[ 2 ], &RayPoint, &RayDir, &a, &b, &c ) )
    {
      if ( !picked ) t = c;
      picked = true;
      if ( c < t ) t = c;
    }
    if ( D3DXIntersectTri( &v[ 0 ], &v[ 2 ], &v[ 3 ], &RayPoint, &RayDir, &a, &b, &c ) )
    {
      if ( !picked ) t = c;
      picked = true;
      if ( c < t ) t = c;
    }
  }

  return picked;
}

D3DXVECTOR3 CUberToolGizmo_Scale_Axis_Standard::GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx, int my )
{
  float t;
  if ( fabs( RayDir.y ) > 0.0001f )
    t = -RayPoint.y / RayDir.y;
  else
    t = -RayPoint.x / RayDir.x;
  return RayPoint + t*RayDir;
}

void CUberToolGizmo_Scale_Axis_Standard::GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir )
{
  //the process is as follows:
  // -the pick position is determined on the axis
  // -axis vector is added to the pick position (to represent a second point on along the axis)
  // -the two points are projected back into screen space
  // -distance calculation is done in screen space by measuring the distance to the plane given by the 2d transformed axis

  D3DXVECTOR2 vcurr = D3DXVECTOR2( (TF32)MousePos.x, (TF32)MousePos.y ) - DragRotateStartMousePos;

  TF32 dist = vcurr.x*DragAxisDir.x + vcurr.y*DragAxisDir.y;

  D3DXVECTOR3 d = CurrPos - PickPos;
  d.x = d.y = 0;

  d.z = dist / 30.0f;

  if ( Snap )
  {
    d.x = round( d.x, SnapValue );
    d.y = round( d.y, SnapValue );
    d.z = round( d.z, SnapValue );
  }
  D3DXVECTOR4 o;
  D3DXVec3Transform( &o, &d, &Transformation );
  srt.Scale = CVector3( 1 + o.x, 1 + o.y, 1 + o.z );
}

//////////////////////////////////////////////////////////////////////////
// ubertool

CUberTool::CUberTool( CCoreDevice *dev )
{
  HelperMesh = 0;
  Device = dev;
  DraggedGizmo = NULL;
  HighlightedGizmo = NULL;
  DragCurrentPosition = DragStartPosition = Position = D3DXVECTOR3( 0, 0, 0 );
  Scale = 1;
  NativeScale = 1;
  Undo = false;
  OrthoZ = D3DXVECTOR3( 0, 0, 0 );
  D3DXMatrixIdentity( &BaseTransform );
  D3DXMatrixIdentity( &DisplayTransform );
  Matrices = Device->CreateConstantBuffer();
  HelperColor = Device->CreateConstantBuffer();
}

CUberTool::~CUberTool()
{
  SAFEDELETE( HelperColor );
  SAFEDELETE( Matrices );
}

void CUberTool::Display( D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX HelperTransform )
{
  float VertexShaderData[ 48 ];
  memcpy( VertexShaderData, ProjectionMatrix, 16 * sizeof( float ) );
  memcpy( VertexShaderData + 16, CameraMatrix, 16 * sizeof( float ) );
  //Device->SetVertexShaderConstantF(0, VertexShaderData, 8);

  for ( int y = 0; y < 2; y++ )
    for ( int x = 0; x < Gizmos.NumItems(); x++ )
    {
      if ( ( ( y == 0 && Gizmos[ x ] != HighlightedGizmo ) || ( y == 1 && Gizmos[ x ] == HighlightedGizmo ) ) && ( ( !DraggedGizmo && Gizmos[ x ]->Visible ) || DraggedGizmo == Gizmos[ x ] ) )
      {
        D3DXMATRIX Transformation;
        D3DXMATRIX s;
        D3DXMatrixScaling( &s, Scale, Scale, Scale );
        D3DXMATRIX p;
        D3DXVECTOR3 pos = GetDisplayPos( Position );
        D3DXMatrixTranslation( &p, pos.x, pos.y, pos.z );
        D3DXMatrixMultiply( &Transformation, &s, &p );

        D3DXMATRIX dt;
        D3DXMatrixMultiply( &dt, &DisplayTransform, &BaseTransform );

        D3DXMatrixMultiply( &Transformation, &dt, &Transformation );
        D3DXMatrixMultiply( &Transformation, &Gizmos[ x ]->Transformation, &Transformation );

        memcpy( VertexShaderData + 32, Transformation, 16 * sizeof( float ) );
        //Device->SetVertexShaderConstantF(0, VertexShaderData, 12);
        Matrices->Reset();
        Matrices->AddData( VertexShaderData, 48 * 4 );
        Matrices->Upload();
        Device->SetShaderConstants( 0, 1, &Matrices );

        Gizmos[ x ]->Display( HighlightedGizmo == Gizmos[ x ] || DraggedGizmo == Gizmos[ x ] );
      }
    }

  //display helper mesh
  if ( HelperMesh )
  {
    memcpy( VertexShaderData + 32, HelperTransform, 16 * sizeof( float ) );
    Matrices->Reset();
    Matrices->AddData( VertexShaderData, 48 * 4 );
    Matrices->Upload();
    Device->SetShaderConstants( 0, 1, &Matrices );

    float helperColor[ 4 ];
    helperColor[ 0 ] = 1;
    helperColor[ 1 ] = 0;
    helperColor[ 2 ] = 1;
    helperColor[ 3 ] = 0.4f;

    Device->SetRenderState( BlendStateAlpha );
    Device->SetRenderState( BlendStateAlpha );
    HelperColor->Reset();
    HelperColor->AddData( helperColor, 16 );
    HelperColor->Upload();
    Device->SetShaderConstants( 1, 1, &HelperColor );

    Device->SetPixelShader( PixelShader );
    Device->SetVertexShader( VertexShader );
    Device->SetVertexFormat( VertexFormat );
    Device->SetRenderState( RasterState );
    Device->SetRenderState( DepthState );

    CphxModelObject_Tool_Mesh *Helper = NULL;
    switch ( HelperMesh )
    {
    case 1: Helper = Plane; break;
    case 2: Helper = Sphere; break;
    case 3: Helper = Cylinder; break;
    case 4: Helper = Cube; break;
    }

    if ( Helper )
    {
      unsigned int offset = 0;
      unsigned int stride = PHXVERTEXFORMATSIZE;

      Device->ApplyRequestedRenderState();

      phxContext->IASetVertexBuffers( 0, 1, &Helper->GetModelObject()->VxBuffer, &stride, &offset );
      phxContext->IASetIndexBuffer( Helper->GetModelObject()->WireBuffer, DXGI_FORMAT_R32_UINT, 0 );
      phxContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
      phxContext->DrawIndexed( Helper->GetModelObject()->EdgeCount * 2, 0, 0 );
    }

  }
}

void CUberTool::StartDrag( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix )
{
  dragProj = ProjectionMatrix;
  dragCam = CameraMatrix;
  dragClient = ClientRect;
  DragCurrentPosition = DragStartPosition = D3DXVECTOR3( 0, 0, 0 );
  //Pick(ViewPort, ProjectionMatrix, CameraMatrix, MouseX, MouseY);
  DraggedGizmo = HighlightedGizmo;
  if ( !DraggedGizmo ) return;

  D3D10_VIEWPORT ViewPort;
  ViewPort.TopLeftX = ClientRect.x1;
  ViewPort.TopLeftY = ClientRect.y1;
  ViewPort.Width = ClientRect.Width();
  ViewPort.Height = ClientRect.Height();
  ViewPort.MinDepth = 0;
  ViewPort.MaxDepth = 1;

  UberToolPos_DragStart = Position;
  UberToolScale_DragStart = Scale;

  D3DXMATRIX Transformation;
  D3DXMATRIX s;
  D3DXMatrixScaling( &s, UberToolScale_DragStart, UberToolScale_DragStart, UberToolScale_DragStart );
  D3DXMATRIX p;
  D3DXVECTOR3 pos = GetDisplayPos( UberToolPos_DragStart );
  D3DXMatrixTranslation( &p, pos.x, pos.y, pos.z );
  D3DXMatrixMultiply( &Transformation, &s, &p );
  D3DXMatrixMultiply( &Transformation, &BaseTransform, &Transformation );
  D3DXMatrixMultiply( &Transformation, &DraggedGizmo->Transformation, &Transformation );

  D3DXVECTOR3 p1;
  D3DXVec3Unproject( &p1, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 0 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );

  D3DXVECTOR3 p2;
  D3DXVec3Unproject( &p2, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 1 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );

  DragCurrentPosition = DragStartPosition = DraggedGizmo->GetPickPosition( p1, p2 - p1, true, D3DXVECTOR3( 0, 0, 0 ), MousePos.x, MousePos.y );
  CurrentMousePos = MousePos;

  D3DXVECTOR3 p3, p4;

  D3DXVECTOR3 dir = DragCurrentPosition;

  D3DXVec3Project( &p3, &dir, &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );
  DragStartMousePos = D3DXVECTOR2( p3 );

  D3DXVec3Project( &p4, &D3DXVECTOR3( 0, 0, 0 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );
  D3DXVec3Project( &p3, &D3DXVECTOR3( 0, 0, 1 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );
  DragAxisDir = D3DXVECTOR2( p3 - p4 );
  D3DXVec2Normalize( &DragAxisDir, &DragAxisDir );

  D3DXVec3Normalize( &dir, &dir );

  D3DXVECTOR3 linenorm;
  D3DXVec3Cross( &linenorm, &dir, &D3DXVECTOR3( 0, 0, 1 ) );
  linenorm += dir;

  D3DXVec3Project( &p3, &dir, &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );
  DragRotateStartMousePos = D3DXVECTOR2( p3 );

  D3DXVec3Project( &p3, &linenorm, &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );
  DragRotateHelperPos = D3DXVECTOR2( p3 );

  //compensate for not exact picking:
  D3DXVECTOR2 v = D3DXVECTOR2( (TF32)MousePos.x, (TF32)MousePos.y ) - DragRotateStartMousePos;
  DragRotateStartMousePos += v;
  DragRotateHelperPos += v;

  v = D3DXVECTOR2( (TF32)MousePos.x, (TF32)MousePos.y ) - DragStartMousePos;
  DragStartMousePos += v;

  int x = 0;
}

void CUberTool::Drag( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix )
{
  if ( !DraggedGizmo ) return;

  D3D10_VIEWPORT ViewPort;
  ViewPort.TopLeftX = ClientRect.x1;
  ViewPort.TopLeftY = ClientRect.y1;
  ViewPort.Width = ClientRect.Width();
  ViewPort.Height = ClientRect.Height();
  ViewPort.MinDepth = 0;
  ViewPort.MaxDepth = 1;

  D3DXMATRIX Transformation;
  D3DXMATRIX s;
  D3DXMatrixScaling( &s, UberToolScale_DragStart, UberToolScale_DragStart, UberToolScale_DragStart );
  D3DXMATRIX p;
  D3DXVECTOR3 pos = GetDisplayPos( UberToolPos_DragStart );
  D3DXMatrixTranslation( &p, pos.x, pos.y, pos.z );
  D3DXMatrixMultiply( &Transformation, &s, &p );
  D3DXMatrixMultiply( &Transformation, &BaseTransform, &Transformation );
  D3DXMatrixMultiply( &Transformation, &DraggedGizmo->Transformation, &Transformation );

  D3DXVECTOR3 p1;
  D3DXVec3Unproject( &p1, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 0 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );

  D3DXVECTOR3 p2;
  D3DXVec3Unproject( &p2, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 1 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );

  DragCurrentPosition = DraggedGizmo->GetPickPosition( p1, p2 - p1, false, DragStartPosition, MousePos.x, MousePos.y );
  CurrentMousePos = MousePos;
}

void CUberTool::EndDrag()
{
  SetUndo( false );
  DraggedGizmo = NULL;
}

void CUberTool::Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix )
{
  D3D10_VIEWPORT ViewPort;
  ViewPort.TopLeftX = ClientRect.x1;
  ViewPort.TopLeftY = ClientRect.y1;
  ViewPort.Width = ClientRect.Width();
  ViewPort.Height = ClientRect.Height();
  ViewPort.MinDepth = 0;
  ViewPort.MaxDepth = 1;
  float t = -2000;
  HighlightedGizmo = NULL;
  for ( int x = 0; x < Gizmos.NumItems(); x++ )
  {
    D3DXMATRIX Transformation;
    D3DXMATRIX s;
    D3DXMatrixScaling( &s, Scale, Scale, Scale );
    D3DXMATRIX p;

    D3DXVECTOR3 pos = GetDisplayPos( Position );
    D3DXMatrixTranslation( &p, pos.x, pos.y, pos.z );
    D3DXMatrixMultiply( &Transformation, &s, &p );

    D3DXMATRIX dt;
    D3DXMatrixMultiply( &dt, &DisplayTransform, &BaseTransform );

    D3DXMatrixMultiply( &Transformation, &dt, &Transformation );
    D3DXMatrixMultiply( &Transformation, &Gizmos[ x ]->Transformation, &Transformation );

    D3DXVECTOR3 p1;
    D3DXVec3Unproject( &p1, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 0 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );

    D3DXVECTOR3 p2;
    D3DXVec3Unproject( &p2, &D3DXVECTOR3( (float)MousePos.x, (float)MousePos.y, 1 ), &ViewPort, &ProjectionMatrix, &CameraMatrix, &Transformation );

    D3DXVECTOR3 d = p2 - p1;
    D3DXVec3Normalize( &d, &d );

    float t2 = 2000;
    if ( Gizmos[ x ]->Visible && Gizmos[ x ]->Pick( p1, d, t2 ) )
    {
      if ( t2 > t )
      {
        t = t2;
        HighlightedGizmo = Gizmos[ x ];
      }
    }
  }
}

void CUberTool::SelectUberTool( int Group, bool XAxis, bool YAxis, bool ZAxis )
{
  for ( int x = 0; x < Gizmos.NumItems(); x++ )
    Gizmos[ x ]->Visible = Gizmos[ x ]->Group == Group;
}

CPRS CUberTool::GetDragResult( TBOOL Snap, TF32 SnapValue )
{
  CPRS srt;
  srt.Scale = CVector3( 1, 1, 1 );
  srt.Translation = CVector3( 0, 0, 0 );
  srt.Rotation = CQuaternion( 0, 0, 0, 1 );

  if ( DraggedGizmo && !Undo )
  {
    DraggedGizmo->GetDragResult( DragStartPosition, DragCurrentPosition, srt, Snap, SnapValue, UberToolScale_DragStart, CurrentMousePos, DragRotateStartMousePos, DragRotateHelperPos, DragStartMousePos, DragAxisDir );
    //srt.Translation *= UberToolScale_DragStart;
  }

  return srt;
}

void CUberTool::SetScale( D3DXVECTOR3 CamEye, D3DXVECTOR3 CamTarget, float Native )
{
  NativeScale = Native;
  SetPosition( CamEye, CamTarget, Position );
}

void CUberTool::SetPosition( D3DXVECTOR3 CamEye, D3DXVECTOR3 CamTarget, D3DXVECTOR3 Pos )
{
  Position = Pos;
  D3DXVECTOR3 Normal = CamTarget - CamEye;
  D3DXVec3Normalize( &Normal, &Normal );
  float d = -( Normal.x*CamEye.x + Normal.y*CamEye.y + Normal.z*CamEye.z );

  float distance = D3DXVec3Dot( &GetDisplayPos( Pos ), &Normal ) + d;

  Scale = distance*NativeScale;

  //LOG_DBG("[ubertool] Position %f %f %f", Position.x, Position.y, Position.z);
}

void CUberTool::SetUndo( bool u )
{
  Undo = u;
}

bool CUberTool::IsDragged()
{
  return DraggedGizmo != NULL;
}

bool CUberTool::IsPicked()
{
  return HighlightedGizmo != NULL;
}

void CUberTool::SetOrthoZ( D3DXVECTOR3 v )
{
  OrthoZ = v;
}

D3DXVECTOR3 CUberTool::GetDisplayPos( D3DXVECTOR3 v )
{
  return v - OrthoZ*D3DXVec3Dot( &v, &OrthoZ );
}

void CUberTool::SetBaseTransform( D3DXMATRIX m )
{
  D3DXMATRIX mat = m;
  //D3DXMatrixInverse(&mat,NULL,&mat);

  D3DXVECTOR4 x, y, z;
  D3DXVec4Transform( &x, &D3DXVECTOR4( 1, 0, 0, 0 ), &m );
  D3DXVec4Transform( &y, &D3DXVECTOR4( 0, 1, 0, 0 ), &m );
  D3DXVec4Transform( &z, &D3DXVECTOR4( 0, 0, 1, 0 ), &m );
  D3DXVECTOR3 _x, _y, _z;
  D3DXVec3Normalize( &_x, &( D3DXVECTOR3( x.x, x.y, x.z ) ) );
  D3DXVec3Normalize( &_y, &( D3DXVECTOR3( y.x, y.y, y.z ) ) );
  D3DXVec3Normalize( &_z, &( D3DXVECTOR3( z.x, z.y, z.z ) ) );

  D3DXMatrixIdentity( &BaseTransform );
  BaseTransform.m[ 0 ][ 0 ] = _x.x;
  BaseTransform.m[ 0 ][ 1 ] = _x.y;
  BaseTransform.m[ 0 ][ 2 ] = _x.z;

  BaseTransform.m[ 1 ][ 0 ] = _y.x;
  BaseTransform.m[ 1 ][ 1 ] = _y.y;
  BaseTransform.m[ 1 ][ 2 ] = _y.z;

  BaseTransform.m[ 2 ][ 0 ] = _z.x;
  BaseTransform.m[ 2 ][ 1 ] = _z.y;
  BaseTransform.m[ 2 ][ 2 ] = _z.z;
}

void CUberTool::EnableHelperDisplay( TS32 MeshType )
{
  HelperMesh = MeshType;
}

void CUberTool::SetDisplayTransform( D3DXMATRIX m )
{
  DisplayTransform = m;
}
