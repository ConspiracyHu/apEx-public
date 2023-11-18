#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../Phoenix/Model.h"
#include "phxResource.h"

class CphxModelObject_Tool_Mesh;
class CphxTextureOperator_Tool;
class CphxMaterial_Tool;
class CphxMaterialTechnique_Tool;

class CphxMeshFilter_Tool
{
  CString Name;
public:

  TBOOL Selected;

  PHXMESHFILTER Filter;
  CphxModelObject_Tool_Mesh *ParentObject;
  CphxTextureOperator_Tool *Texture;
  unsigned char Parameters[ 3 ];
  D3DXFLOAT16 srt[ 12 ];
  D3DXFLOAT16 _srt[ 12 ];
  bool Enabled;

  CphxGUID ReferencedGUID;

  CphxMeshFilter_Tool();
  void ApplyTransformation( CPRS &f );
  void ApplyMatrix( D3DXMATRIX &m, TBOOL Invert );
  D3DXMATRIX GetTransformationMatrix();
  D3DXMATRIX GetTransformationMatrix_();
  D3DXMATRIX GetRawMatrix();
  D3DXMATRIX GetRawMatrix_();
  void SetRawMatrix( D3DXMATRIX &m );
  void SetTransformationMatrix( D3DXMATRIX &m );

  void SetName( TCHAR *name ) { Name = Name; }
  CString &GetName() { return Name; }

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

};

class CphxModelObject_Tool : public CphxResource
{
  friend class CphxModel_Tool;
  friend class CapexModelList;
protected:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CString Name;

  CphxModelObject *Object;

  PHXMESHPRIMITIVE Primitive;
  TU8 Parameters[ 14 ];
  CString Text;

  //CDynamicArray<flareMeshSplineKey> Spline;
  //CDynamicArray<flareVertex> StoredVertices;
  //CDynamicArray<flarePolygon> StoredPolygons;

  CphxModel_Tool *ParentModel;
  CArray<CphxMeshFilter_Tool*> Filters;

public:
  CArray<unsigned char> miniModelVertices;
  CArray<unsigned char> miniModelTriangles;
  int miniModelExportIndex;
  D3DXFLOAT16 FloatParameter;

  TU8 *GetParameters() { return Parameters; }

  TBOOL Selected;
  D3DXMATRIX _Transformation;
  CColor WireframeColor;

  CphxGUID ParentGUIDS[ 2 ];

  CArray<CphxGUID> ClonedGuids;
  CArray<CphxModelObject_Tool*> ClonedObjects;

  virtual PHXRESOURCETYPE GetType() { return PHX_MODELOBJECT; }

  CphxModelObject_Tool();
  virtual ~CphxModelObject_Tool();

  D3DXMATRIX GetMatrix();
  virtual void SetMatrix( D3DXMATRIX m );

  void SetName( CString &name );
  void SetName( TCHAR *Name );
  CString GetName();
  PHXMESHPRIMITIVE GetPrimitive() { return Primitive; }
  CphxModel_Tool *GetParentModel() { return ParentModel; }
  CphxModelObject *GetObject() { return Object; }

  virtual bool Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t ) { return false; }
  TS32 GetFilterCount() { return Filters.NumItems(); }
  CphxMeshFilter_Tool *GetFilter( TS32 x ) { return x >= 0 && x < Filters.NumItems() ? Filters[ x ] : NULL; }

  virtual TBOOL FindInCloneTree( CphxModelObject_Tool* );
  virtual CphxMeshFilter_Tool *AddFilter( PHXMESHFILTER p ) { return NULL; };
  virtual void DeleteFilter( CphxMeshFilter_Tool *t );
  virtual void MoveFilterUp( CphxMeshFilter_Tool *t );
  virtual void MoveFilterDown( CphxMeshFilter_Tool *t );
  virtual void SetMaterial( CphxMaterial_Tool *m, TBOOL Update = false ) {}
  virtual CphxMaterial_Tool *GetMaterial() { return NULL; }
  virtual void UpdateMaterialState() {}
  virtual void UpdateMaterialTextures() {}
  CString GetText() { return Text; }
  void SetText( CString &t ) { Text = t; }

  D3DXMATRIX GetBoundingBoxMatrix();

  virtual void AddClonedObject( CphxModelObject_Tool* c ) = 0;
  virtual void RemoveClonedObject( CphxModelObject_Tool* c ) = 0;
};

class CphxMaterialPassConstantState_Tool : public CphxMaterialPassConstantState
{

public:

  CphxMaterialPassConstantState_Tool();
  virtual ~CphxMaterialPassConstantState_Tool();

};

class CphxMaterialParameterBatch_Tool;

class CphxMaterialDataStorage_Tool
{
public:

  CDictionary<CphxGUID, MATERIALVALUE> MaterialParams;
  CDictionary<CphxGUID, CphxGUID> MaterialTextures;

  void UpdateParams( CphxMaterial_Tool *Material );
  void UpdateParams( CphxMaterialTechnique_Tool *Tech );
  void UpdateTextures( CphxMaterial_Tool *Material );
  void UpdateTextures( CphxMaterialTechnique_Tool *Material );
  void SetDefaultValues( CphxMaterialParameterBatch_Tool *Batch );
  void SetMissingDefaultValues( CphxMaterialParameterBatch_Tool *Batch );
  void SetDefaultValues( CphxMaterial_Tool *Material );
  void SetMissingDefaultValues( CphxMaterial_Tool *Material );
  void SetDefaultValues( CphxMaterialTechnique_Tool *Tech );
  void SetMissingDefaultValues( CphxMaterialTechnique_Tool *Tech );
  void Copy( CphxMaterialDataStorage_Tool *Original );

  void TryImport( CphxMaterialDataStorage_Tool *Original, CphxMaterial_Tool *oldMat, CphxMaterial_Tool *newMat );

  void ExportData( CXMLNode *Node );
  void ImportData( CXMLNode *Node );

  virtual ~CphxMaterialDataStorage_Tool();
};

class CphxModelObject_Tool_Mesh : public CphxModelObject_Tool
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );
  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  void FreeMaterialState();
  void AllocateMaterialState();

  TS32 MaterialStateSize;

public:

  CphxMaterial_Tool *Material;
  CphxMaterialDataStorage_Tool MaterialData;

  CphxModelObject_Tool_Mesh();
  virtual ~CphxModelObject_Tool_Mesh();
  virtual void SetMatrix( D3DXMATRIX m );

  CphxModelObject_Mesh *GetModelObject();

  virtual bool Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t );
  virtual CphxMeshFilter_Tool *AddFilter( PHXMESHFILTER p );
  virtual void UpdateDependencies();

  virtual void SetMaterial( CphxMaterial_Tool *m, TBOOL Update = false );
  virtual CphxMaterial_Tool *GetMaterial() { return Material; }
  virtual void UpdateMaterialState();
  virtual void UpdateMaterialTextures();

  virtual void AddClonedObject( CphxModelObject_Tool* c );
  virtual void RemoveClonedObject( CphxModelObject_Tool* c );
};

class CphxModelObject_Tool_Clone : public CphxModelObject_Tool
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );
  virtual TBOOL GenerateResource( CCoreDevice *Dev );

public:

  CphxModelObject_Tool_Clone();
  virtual ~CphxModelObject_Tool_Clone();

  CphxModelObject_Clone *GetModelObject();
  virtual TBOOL FindInCloneTree( CphxModelObject_Tool* );

  virtual bool Pick( CRect ClientRect, CPoint MousePos, D3DXMATRIX ProjectionMatrix, D3DXMATRIX CameraMatrix, D3DXMATRIX TransformationMatrix, float &t );
  virtual void UpdateDependencies();

  virtual void AddClonedObject( CphxModelObject_Tool* c );
  virtual void RemoveClonedObject( CphxModelObject_Tool* c );
};


class CphxModel_Tool : public CphxResource
{
  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  CString Name;
  CphxModel Model;

public:

  CArray<CphxModelObject_Tool*> Objects;

  CphxModel_Tool();
  virtual ~CphxModel_Tool();

  void SetName( CString &name );
  CString GetName();

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType() { return PHX_MODEL; }

  CphxModelObject_Tool *AddPrimitive( PHXMESHPRIMITIVE p );

  TS32 GetObjectCount() { return Objects.NumItems(); }
  CphxModelObject_Tool *GetObjectByIndex( TS32 x ) { return Objects[ x ]; }
  TS32 GetObjectIndex( CphxModelObject_Tool *x );
  TS32 GetObjectIndex( CphxGUID &g );

  CphxModel *GetModel() { return &Model; }

  TBOOL DeleteSelected();
  void CopySelected();

  void UpdateTextures();
  void UpdateMaterialStates();

  void SetModelerMode();

};