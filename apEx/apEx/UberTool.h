#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../../Bedrock/CoRE2/Core2.h"
#include "../Phoenix_Tool/apxPhoenix.h"

enum UBERTOOLGIZMOTYPE
{
  ubertool_MoveX,
  ubertool_MoveY,
  ubertool_MoveZ,
  ubertool_MoveXY,
  ubertool_MoveYZ,
  ubertool_MoveXZ,
  UberTool_RotateX,
  UberTool_RotateY,
  UberTool_RotateZ,
  UberTool_ScaleX,
  UberTool_ScaleY,
  UberTool_ScaleZ,
  UberTool_ScaleUniform,
};

class CUberToolGizmo
{
protected:
  CCoreDevice *Device;

  UBERTOOLGIZMOTYPE Type;

  CCoreConstantBuffer *ColorData;
  CCoreVertexBuffer *VertexBuffer;
  CCoreIndexBuffer *WireIndexBuffer;
  CCoreIndexBuffer *SolidIndexBuffer;

  int LineCount, TriCount, VertexCount;
  float Color[ 4 ];

public:

  D3DXMATRIX Transformation;

  int Group;
  bool Visible;

  CUberToolGizmo( CCoreDevice *Device );
  virtual ~CUberToolGizmo();

  virtual void Display( bool Highlight );
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t ) = 0;
  void SetType( UBERTOOLGIZMOTYPE Type );
  UBERTOOLGIZMOTYPE GetType();
  virtual void SetWireColor( bool Highlight );
  virtual void SetSolidColor( bool Highlight );
  virtual D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 ) = 0;
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir ) = 0;
  void SetColor( float r, float g, float b );
};

class CUberToolGizmo_Move_Axis_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Move_Axis_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Move_Axis_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
};

class CUberToolGizmo_Move_Plane_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Move_Plane_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Move_Plane_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  virtual void SetSolidColor( bool Highlight );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
};

class CUberToolGizmo_Rotation_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Rotation_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Rotation_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
};

class CUberToolGizmo_Scale_Plane_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Scale_Plane_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Scale_Plane_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
  void SetSolidColor( bool Highlight );
};

class CUberToolGizmo_Scale_Plane_Uniform_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Scale_Plane_Uniform_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Scale_Plane_Uniform_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
  void SetSolidColor( bool Highlight );
};

class CUberToolGizmo_Scale_Uniform_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Scale_Uniform_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Scale_Uniform_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
  void SetSolidColor( bool Highlight );
};

class CUberToolGizmo_Scale_Axis_Standard : public CUberToolGizmo
{
public:
  CUberToolGizmo_Scale_Axis_Standard( CCoreDevice *Device );
  virtual ~CUberToolGizmo_Scale_Axis_Standard() {};
  virtual bool Pick( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, float &t );
  D3DXVECTOR3 GetPickPosition( D3DXVECTOR3 RayPoint, D3DXVECTOR3 RayDir, bool StartPos, D3DXVECTOR3 StartPickPosition, int mx = 0, int my = 0 );
  virtual void GetDragResult( D3DXVECTOR3 PickPos, D3DXVECTOR3 CurrPos, CPRS &srt, TBOOL Snap, TF32 SnapValue, TF32 mult, CPoint MousePos, D3DXVECTOR2 DragRotateStartMousePos, D3DXVECTOR2 DragRotateHelperPos, D3DXVECTOR2 DragStartMousePos, D3DXVECTOR2 DragAxisDir );
};

class CUberTool
{
  CCoreDevice *Device;

  D3DXMATRIX dragProj, dragCam;
  CRect dragClient;

  D3DXVECTOR3 camEye, camTarget, camUp;

  CUberToolGizmo *DraggedGizmo;
  CUberToolGizmo *HighlightedGizmo;
  CPoint CurrentMousePos;

  D3DXVECTOR3 DragStartPosition;
  D3DXVECTOR3 DragCurrentPosition;

  D3DXVECTOR2 DragStartMousePos;
  D3DXVECTOR2 DragAxisDir;
  D3DXVECTOR2 DragRotateStartMousePos;
  D3DXVECTOR2 DragRotateHelperPos;

  float UberToolScale_DragStart;

  D3DXVECTOR3 Position;
  float Scale;
  float NativeScale;

  bool Undo;

  D3DXVECTOR3 OrthoZ;

  D3DXVECTOR3 GetDisplayPos( D3DXVECTOR3 v );

  D3DXMATRIX BaseTransform;
  D3DXMATRIX DisplayTransform;
  CCoreConstantBuffer *Matrices;
  CCoreConstantBuffer *HelperColor;

  TS32 HelperMesh;

public:

  D3DXVECTOR3 UberToolPos_DragStart;
  CArray<CUberToolGizmo*> Gizmos;

  CUberTool( CCoreDevice *Device );
  virtual ~CUberTool();

  virtual void Display( D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX HelperTransform );
  virtual void Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix );
  virtual void StartDrag( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix );
  virtual void Drag( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix );
  virtual void EndDrag();
  virtual void SelectUberTool( int Group, bool XAxis = true, bool YAxis = true, bool ZAxis = true );

  CPRS GetDragResult( TBOOL Snap, TF32 SnapValue );
  void SetScale( D3DXVECTOR3 CamEye, D3DXVECTOR3 CamTarget, float NativeScale );
  void SetPosition( D3DXVECTOR3 CamEye, D3DXVECTOR3 CamTarget, D3DXVECTOR3 Pos );
  void SetUndo( bool Undo );
  bool IsDragged();
  bool IsPicked();

  void SetOrthoZ( D3DXVECTOR3 v );
  void SetBaseTransform( D3DXMATRIX m );
  void SetDisplayTransform( D3DXMATRIX m );
  void EnableHelperDisplay( TS32 MeshType );
  D3DXVECTOR3 GetPosition() { return Position; }
};

extern CphxModelObject_Tool_Mesh *Cube;
extern CphxModelObject_Tool_Mesh *Sphere;
extern CphxModelObject_Tool_Mesh *Plane;
extern CphxModelObject_Tool_Mesh *Cylinder;

void InitUberTool( CCoreDevice *Dev );
void DeinitUberTool();
