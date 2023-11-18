#pragma once
#define _WINSOCKAPI_

#pragma push_macro("new")
#undef new
#include <d3d10_1.h>
#include <D3DX10Math.h>
#pragma pop_macro("new")

#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../../Bedrock/UtilLib/XMLDocument.h"
#include "../Phoenix/phxSpline.h"

enum SPLINETYPE
{
  Spline_F16,
  Spline_Quaternion,
};

class CphxSplineKey_Tool
{
public:
  CphxSplineKey Key;
  TBOOL Selected;
  CphxSplineKey Stored;

  CphxSplineKey_Tool();
};

class CphxSpline_Tool
{
protected:

  //virtual void _dummy() = 0;
  void ExportKeys( CXMLNode &node );
  void ImportKeys( CXMLNode &node );

public:
  CArray<CphxSplineKey_Tool*> Keys;
  TF32 ValueBackup[ 4 ];

  CphxSpline *Spline;

  CphxSpline_Tool();
  virtual ~CphxSpline_Tool();

  void UpdateSplineKeys();
  void GetValue( float t, float Value[ 4 ] );

  void Sort();

  virtual void ExportData( CXMLNode &node ) = 0;
  virtual void ImportData( CXMLNode &node ) = 0;

  virtual CphxSpline_Tool *Copy() = 0;
  virtual void CopyTo( CphxSpline_Tool *Target ) = 0;

  void Backup();
  void StoreKeyData();

  virtual void ApplySplineTransformation( TF32 Timestamp, TF32 Value, TBOOL AutoKey );
  virtual void ApplySplineTransformation( TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey );
  virtual SPLINETYPE GetSplineType() = 0;

  void AddKey( TF32 TimeStamp, TF32 Value );

  virtual bool IsConstant( D3DXFLOAT16 constValue[ 4 ] ) = 0;
  virtual bool HasIntValue( int v ) = 0;

};

class CphxSpline_Tool_float16 : public CphxSpline_Tool
{
  //virtual void _dummy() {};

public:
  CphxSpline_Tool_float16();

  virtual void ExportData( CXMLNode &node );
  virtual void ImportData( CXMLNode &node );
  virtual CphxSpline_Tool *Copy();
  virtual void CopyTo( CphxSpline_Tool *Target );

  virtual SPLINETYPE GetSplineType() { return Spline_F16; }

  const TBOOL operator==( const CphxSpline_Tool_float16& ) const;

  bool IsZeroToOne();

  virtual bool IsConstant( D3DXFLOAT16 constValue[ 4 ] );
  virtual bool HasIntValue( int v );
};

class CphxSpline_Tool_Quaternion16 : public CphxSpline_Tool
{
  //virtual void _dummy() {};

public:
  CphxSpline_Tool_Quaternion16();

  virtual void ExportData( CXMLNode &node );
  virtual void ImportData( CXMLNode &node );
  virtual CphxSpline_Tool *Copy();
  virtual void CopyTo( CphxSpline_Tool *Target );

  virtual SPLINETYPE GetSplineType() { return Spline_Quaternion; }

  const TBOOL operator==( const CphxSpline_Tool_Quaternion16& ) const;

  virtual bool IsConstant( D3DXFLOAT16 constValue[ 4 ] );
  virtual bool HasIntValue( int v ) { return false; }
};

