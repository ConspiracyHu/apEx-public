#pragma once
#include "Application.h"

enum VALUEINPUTTYPE
{
  WBVI_INT,
  WBVI_FLOAT,
  WBVI_UINT,
  WBVI_INT64,
  WBVI_DOUBLE,
  WBVI_UINT64,
};

class CWBValueInput
{
  TBOOL ValueInitialized = false;
  VALUEINPUTTYPE InputType;

  TS32 ValueTS32, MinTS32, MaxTS32, StepTS32;
  TF32 ValueTF32, MinTF32, MaxTF32, StepTF32;
  TU32 ValueTU32, MinTU32, MaxTU32, StepTU32;
  TS64 ValueTS64, MinTS64, MaxTS64, StepTS64;
  TF64 ValueTF64, MinTF64, MaxTF64, StepTF64;
  TU64 ValueTU64, MinTU64, MaxTU64, StepTU64;

  virtual void SendValueChangeMessage() = 0;

  static const CString &GetClassName()
  {
    static const CString type = _T( "valueinput" );
    return type;
  }

public:

  CWBValueInput();
  CWBValueInput( VALUEINPUTTYPE inputtype );
  virtual ~CWBValueInput();
  virtual TBOOL Initialize( VALUEINPUTTYPE inputtype );

  virtual TBOOL InstanceOf( const CString &name ) const
  {
    return name == GetClassName();
  }

  virtual VALUEINPUTTYPE GetValueType();

  virtual void SetValue( TS32 Value );
  virtual void SetValue( TF32 Value );
  virtual void SetValue( TU32 Value );
  virtual void SetValue( TS64 Value );
  virtual void SetValue( TF64 Value );
  virtual void SetValue( TU64 Value );

  virtual void SetMinConstraint( TS32 Min );
  virtual void SetMinConstraint( TF32 Min );
  virtual void SetMinConstraint( TU32 Min );
  virtual void SetMinConstraint( TS64 Min );
  virtual void SetMinConstraint( TF64 Min );
  virtual void SetMinConstraint( TU64 Min );

  virtual void SetMaxConstraint( TS32 Max );
  virtual void SetMaxConstraint( TF32 Max );
  virtual void SetMaxConstraint( TU32 Max );
  virtual void SetMaxConstraint( TS64 Max );
  virtual void SetMaxConstraint( TF64 Max );
  virtual void SetMaxConstraint( TU64 Max );

  virtual void SetConstraints( TS32 Min, TS32 Max );
  virtual void SetConstraints( TF32 Min, TF32 Max );
  virtual void SetConstraints( TU32 Min, TU32 Max );
  virtual void SetConstraints( TS64 Min, TS64 Max );
  virtual void SetConstraints( TF64 Min, TF64 Max );
  virtual void SetConstraints( TU64 Min, TU64 Max );
  virtual void ClearConstraints();

  virtual TBOOL GetValue( TS32 &Value );
  virtual TBOOL GetValue( TF32 &Value );
  virtual TBOOL GetValue( TU32 &Value );
  virtual TBOOL GetValue( TS64 &Value );
  virtual TBOOL GetValue( TF64 &Value );
  virtual TBOOL GetValue( TU64 &Value );

  virtual TBOOL GetValueSafe( TS32 &Value, TS32 Min, TS32 Max );
  virtual TBOOL GetValueSafe( TF32 &Value, TF32 Min, TF32 Max );
  virtual TBOOL GetValueSafe( TU32 &Value, TU32 Min, TU32 Max );
  virtual TBOOL GetValueSafe( TS64 &Value, TS64 Min, TS64 Max );
  virtual TBOOL GetValueSafe( TF64 &Value, TF64 Min, TF64 Max );
  virtual TBOOL GetValueSafe( TU64 &Value, TU64 Min, TU64 Max );

  virtual TS32 GetValueAsInt();
  virtual TF32 GetValueAsFloat();
  virtual TU32 GetValueAsUnsignedInt();
  virtual TS64 GetValueAsInt64();
  virtual TF64 GetValueAsDouble();
  virtual TU64 GetValueAsUnsignedInt64();

  virtual TS32 GetValueAsIntSafe( TS32 Min, TS32 Max );
  virtual TF32 GetValueAsFloatSafe( TF32 Min, TF32 Max );
  virtual TU32 GetValueAsUnsignedIntSafe( TU32 Min, TU32 Max );
  virtual TS64 GetValueAsInt64Safe( TS64 Min, TS64 Max );
  virtual TF64 GetValueAsDoubleSafe( TF64 Min, TF64 Max );
  virtual TU64 GetValueAsUnsignedInt64Safe( TU64 Min, TU64 Max );

  virtual TF32 GetValuePositionRelativeToConstraints(); //returns a 0..1 normalized t value between the set constraints
  virtual void SetInterpolatedValue( TF32 t ); //set value between the two constraints based on a 0..1 normalized t value
  virtual void ApplyStep( TS32 StepCount );
};