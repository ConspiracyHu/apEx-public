#include "BasePCH.h"
#include "ValueInput.h"

CWBValueInput::CWBValueInput()
{
  Initialize( WBVI_INT );
}

CWBValueInput::CWBValueInput( VALUEINPUTTYPE inputtype )
{
  Initialize( inputtype );
}

CWBValueInput::~CWBValueInput()
{

}

TBOOL CWBValueInput::Initialize( VALUEINPUTTYPE inputtype )
{
  InputType = inputtype;

  ValueTS32 = 0;
  ValueTF32 = 0;
  ValueTU32 = 0;
  ValueTS64 = 0;
  ValueTF64 = 0;
  ValueTU64 = 0;

  StepTS32 = 1;
  StepTF32 = 1;
  StepTU32 = 1;
  StepTS64 = 1;
  StepTF64 = 1;
  StepTU64 = 1;

  ClearConstraints();
  return true;
}


VALUEINPUTTYPE CWBValueInput::GetValueType()
{
  return InputType;
}

void CWBValueInput::SetValue( TS32 Value )
{
  TS32 bckTS32 = ValueTS32;
  TF32 bckTF32 = ValueTF32;
  TU32 bckTU32 = ValueTU32;
  TS64 bckTS64 = ValueTS64;
  TF64 bckTF64 = ValueTF64;
  TU64 bckTU64 = ValueTU64;
  switch ( GetValueType() )
  {
  case WBVI_INT: ValueTS32 = max( MinTS32, min( MaxTS32, (TS32)Value ) );	if ( !ValueInitialized || ValueTS32 != bckTS32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_FLOAT: ValueTF32 = max( MinTF32, min( MaxTF32, (TF32)Value ) ); if ( !ValueInitialized || ValueTF32 != bckTF32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT: ValueTU32 = max( MinTU32, min( MaxTU32, (TU32)Value ) ); if ( !ValueInitialized || ValueTU32 != bckTU32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_INT64: ValueTS64 = max( MinTS64, min( MaxTS64, (TS64)Value ) ); if ( !ValueInitialized || ValueTS64 != bckTS64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_DOUBLE: ValueTF64 = max( MinTF64, min( MaxTF64, (TF64)Value ) ); if ( !ValueInitialized || ValueTF64 != bckTF64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT64: ValueTU64 = max( MinTU64, min( MaxTU64, (TU64)Value ) ); if ( !ValueInitialized || ValueTU64 != bckTU64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  }
}

void CWBValueInput::SetValue( TF32 Value )
{
  TS32 bckTS32 = ValueTS32;
  TF32 bckTF32 = ValueTF32;
  TU32 bckTU32 = ValueTU32;
  TS64 bckTS64 = ValueTS64;
  TF64 bckTF64 = ValueTF64;
  TU64 bckTU64 = ValueTU64;
  switch ( GetValueType() )
  {
  case WBVI_INT: ValueTS32 = max( MinTS32, min( MaxTS32, (TS32)Value ) );	if ( !ValueInitialized || ValueTS32 != bckTS32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_FLOAT: ValueTF32 = max( MinTF32, min( MaxTF32, (TF32)Value ) ); if ( !ValueInitialized || ValueTF32 != bckTF32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT: ValueTU32 = max( MinTU32, min( MaxTU32, (TU32)Value ) ); if ( !ValueInitialized || ValueTU32 != bckTU32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_INT64: ValueTS64 = max( MinTS64, min( MaxTS64, (TS64)Value ) ); if ( !ValueInitialized || ValueTS64 != bckTS64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_DOUBLE: ValueTF64 = max( MinTF64, min( MaxTF64, (TF64)Value ) ); if ( !ValueInitialized || ValueTF64 != bckTF64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT64: ValueTU64 = max( MinTU64, min( MaxTU64, (TU64)Value ) ); if ( !ValueInitialized || ValueTU64 != bckTU64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  }
}

void CWBValueInput::SetValue( TU32 Value )
{
  TS32 bckTS32 = ValueTS32;
  TF32 bckTF32 = ValueTF32;
  TU32 bckTU32 = ValueTU32;
  TS64 bckTS64 = ValueTS64;
  TF64 bckTF64 = ValueTF64;
  TU64 bckTU64 = ValueTU64;
  switch ( GetValueType() )
  {
  case WBVI_INT: ValueTS32 = max( MinTS32, min( MaxTS32, (TS32)Value ) );	if ( !ValueInitialized || ValueTS32 != bckTS32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_FLOAT: ValueTF32 = max( MinTF32, min( MaxTF32, (TF32)Value ) ); if ( !ValueInitialized || ValueTF32 != bckTF32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT: ValueTU32 = max( MinTU32, min( MaxTU32, (TU32)Value ) ); if ( !ValueInitialized || ValueTU32 != bckTU32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_INT64: ValueTS64 = max( MinTS64, min( MaxTS64, (TS64)Value ) ); if ( !ValueInitialized || ValueTS64 != bckTS64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_DOUBLE: ValueTF64 = max( MinTF64, min( MaxTF64, (TF64)Value ) ); if ( !ValueInitialized || ValueTF64 != bckTF64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT64: ValueTU64 = max( MinTU64, min( MaxTU64, (TU64)Value ) ); if ( !ValueInitialized || ValueTU64 != bckTU64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  }
}

void CWBValueInput::SetValue( TS64 Value )
{
  TS32 bckTS32 = ValueTS32;
  TF32 bckTF32 = ValueTF32;
  TU32 bckTU32 = ValueTU32;
  TS64 bckTS64 = ValueTS64;
  TF64 bckTF64 = ValueTF64;
  TU64 bckTU64 = ValueTU64;
  switch ( GetValueType() )
  {
  case WBVI_INT: ValueTS32 = max( MinTS32, min( MaxTS32, (TS32)Value ) );	if ( !ValueInitialized || ValueTS32 != bckTS32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_FLOAT: ValueTF32 = max( MinTF32, min( MaxTF32, (TF32)Value ) ); if ( !ValueInitialized || ValueTF32 != bckTF32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT: ValueTU32 = max( MinTU32, min( MaxTU32, (TU32)Value ) ); if ( !ValueInitialized || ValueTU32 != bckTU32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_INT64: ValueTS64 = max( MinTS64, min( MaxTS64, (TS64)Value ) ); if ( !ValueInitialized || ValueTS64 != bckTS64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_DOUBLE: ValueTF64 = max( MinTF64, min( MaxTF64, (TF64)Value ) ); if ( !ValueInitialized || ValueTF64 != bckTF64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT64: ValueTU64 = max( MinTU64, min( MaxTU64, (TU64)Value ) ); if ( !ValueInitialized || ValueTU64 != bckTU64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  }
}

void CWBValueInput::SetValue( TF64 Value )
{
  TS32 bckTS32 = ValueTS32;
  TF32 bckTF32 = ValueTF32;
  TU32 bckTU32 = ValueTU32;
  TS64 bckTS64 = ValueTS64;
  TF64 bckTF64 = ValueTF64;
  TU64 bckTU64 = ValueTU64;
  switch ( GetValueType() )
  {
  case WBVI_INT: ValueTS32 = max( MinTS32, min( MaxTS32, (TS32)Value ) );	if ( !ValueInitialized || ValueTS32 != bckTS32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_FLOAT: ValueTF32 = max( MinTF32, min( MaxTF32, (TF32)Value ) ); if ( !ValueInitialized || ValueTF32 != bckTF32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT: ValueTU32 = max( MinTU32, min( MaxTU32, (TU32)Value ) ); if ( !ValueInitialized || ValueTU32 != bckTU32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_INT64: ValueTS64 = max( MinTS64, min( MaxTS64, (TS64)Value ) ); if ( !ValueInitialized || ValueTS64 != bckTS64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_DOUBLE: ValueTF64 = max( MinTF64, min( MaxTF64, (TF64)Value ) ); if ( !ValueInitialized || ValueTF64 != bckTF64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT64: ValueTU64 = max( MinTU64, min( MaxTU64, (TU64)Value ) ); if ( !ValueInitialized || ValueTU64 != bckTU64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  }
}

void CWBValueInput::SetValue( TU64 Value )
{
  TS32 bckTS32 = ValueTS32;
  TF32 bckTF32 = ValueTF32;
  TU32 bckTU32 = ValueTU32;
  TS64 bckTS64 = ValueTS64;
  TF64 bckTF64 = ValueTF64;
  TU64 bckTU64 = ValueTU64;
  switch ( GetValueType() )
  {
  case WBVI_INT: ValueTS32 = max( MinTS32, min( MaxTS32, (TS32)Value ) );	if ( !ValueInitialized || ValueTS32 != bckTS32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_FLOAT: ValueTF32 = max( MinTF32, min( MaxTF32, (TF32)Value ) ); if ( !ValueInitialized || ValueTF32 != bckTF32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT: ValueTU32 = max( MinTU32, min( MaxTU32, (TU32)Value ) ); if ( !ValueInitialized || ValueTU32 != bckTU32 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_INT64: ValueTS64 = max( MinTS64, min( MaxTS64, (TS64)Value ) ); if ( !ValueInitialized || ValueTS64 != bckTS64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_DOUBLE: ValueTF64 = max( MinTF64, min( MaxTF64, (TF64)Value ) ); if ( !ValueInitialized || ValueTF64 != bckTF64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  case WBVI_UINT64: ValueTU64 = max( MinTU64, min( MaxTU64, (TU64)Value ) ); if ( !ValueInitialized || ValueTU64 != bckTU64 ) SendValueChangeMessage(); ValueInitialized = true; break;
  }
}

void CWBValueInput::SetMinConstraint( TS32 Min )
{
  SetConstraints( Min, MaxTS32 );
}

void CWBValueInput::SetMinConstraint( TF32 Min )
{
  SetConstraints( Min, MaxTF32 );
}

void CWBValueInput::SetMinConstraint( TU32 Min )
{
  SetConstraints( Min, MaxTU32 );
}

void CWBValueInput::SetMinConstraint( TS64 Min )
{
  SetConstraints( Min, MaxTS64 );
}

void CWBValueInput::SetMinConstraint( TF64 Min )
{
  SetConstraints( Min, MaxTF64 );
}

void CWBValueInput::SetMinConstraint( TU64 Min )
{
  SetConstraints( Min, MaxTU64 );
}

void CWBValueInput::SetMaxConstraint( TS32 Max )
{
  SetConstraints( MinTS32, Max );
}

void CWBValueInput::SetMaxConstraint( TF32 Max )
{
  SetConstraints( MinTF32, Max );
}

void CWBValueInput::SetMaxConstraint( TU32 Max )
{
  SetConstraints( MinTU32, Max );
}

void CWBValueInput::SetMaxConstraint( TS64 Max )
{
  SetConstraints( MinTS64, Max );
}

void CWBValueInput::SetMaxConstraint( TF64 Max )
{
  SetConstraints( MinTF64, Max );
}

void CWBValueInput::SetMaxConstraint( TU64 Max )
{
  SetConstraints( MinTU64, Max );
}

void CWBValueInput::SetConstraints( TS32 Min, TS32 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: MinTS32 = (TS32)min( Min, Max ); MaxTS32 = (TS32)max( Min, Max ); break;
  case WBVI_FLOAT: MinTF32 = (TF32)min( Min, Max ); MaxTF32 = (TF32)max( Min, Max ); break;
  case WBVI_UINT: MinTU32 = (TU32)min( Min, Max ); MaxTU32 = (TU32)max( Min, Max ); break;
  case WBVI_INT64: MinTS64 = (TS64)min( Min, Max ); MaxTS64 = (TS64)max( Min, Max ); break;
  case WBVI_DOUBLE: MinTF64 = (TF64)min( Min, Max ); MaxTF64 = (TF64)max( Min, Max ); break;
  case WBVI_UINT64: MinTU64 = (TU64)min( Min, Max ); MaxTU64 = (TU64)max( Min, Max ); break;
  }
}

void CWBValueInput::SetConstraints( TF32 Min, TF32 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: MinTS32 = (TS32)min( Min, Max ); MaxTS32 = (TS32)max( Min, Max ); break;
  case WBVI_FLOAT: MinTF32 = (TF32)min( Min, Max ); MaxTF32 = (TF32)max( Min, Max ); break;
  case WBVI_UINT: MinTU32 = (TU32)min( Min, Max ); MaxTU32 = (TU32)max( Min, Max ); break;
  case WBVI_INT64: MinTS64 = (TS64)min( Min, Max ); MaxTS64 = (TS64)max( Min, Max ); break;
  case WBVI_DOUBLE: MinTF64 = (TF64)min( Min, Max ); MaxTF64 = (TF64)max( Min, Max ); break;
  case WBVI_UINT64: MinTU64 = (TU64)min( Min, Max ); MaxTU64 = (TU64)max( Min, Max ); break;
  }
}

void CWBValueInput::SetConstraints( TU32 Min, TU32 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: MinTS32 = (TS32)min( Min, Max ); MaxTS32 = (TS32)max( Min, Max ); break;
  case WBVI_FLOAT: MinTF32 = (TF32)min( Min, Max ); MaxTF32 = (TF32)max( Min, Max ); break;
  case WBVI_UINT: MinTU32 = (TU32)min( Min, Max ); MaxTU32 = (TU32)max( Min, Max ); break;
  case WBVI_INT64: MinTS64 = (TS64)min( Min, Max ); MaxTS64 = (TS64)max( Min, Max ); break;
  case WBVI_DOUBLE: MinTF64 = (TF64)min( Min, Max ); MaxTF64 = (TF64)max( Min, Max ); break;
  case WBVI_UINT64: MinTU64 = (TU64)min( Min, Max ); MaxTU64 = (TU64)max( Min, Max ); break;
  }
}

void CWBValueInput::SetConstraints( TS64 Min, TS64 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: MinTS32 = (TS32)min( Min, Max ); MaxTS32 = (TS32)max( Min, Max ); break;
  case WBVI_FLOAT: MinTF32 = (TF32)min( Min, Max ); MaxTF32 = (TF32)max( Min, Max ); break;
  case WBVI_UINT: MinTU32 = (TU32)min( Min, Max ); MaxTU32 = (TU32)max( Min, Max ); break;
  case WBVI_INT64: MinTS64 = (TS64)min( Min, Max ); MaxTS64 = (TS64)max( Min, Max ); break;
  case WBVI_DOUBLE: MinTF64 = (TF64)min( Min, Max ); MaxTF64 = (TF64)max( Min, Max ); break;
  case WBVI_UINT64: MinTU64 = (TU64)min( Min, Max ); MaxTU64 = (TU64)max( Min, Max ); break;
  }
}

void CWBValueInput::SetConstraints( TF64 Min, TF64 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: MinTS32 = (TS32)min( Min, Max ); MaxTS32 = (TS32)max( Min, Max ); break;
  case WBVI_FLOAT: MinTF32 = (TF32)min( Min, Max ); MaxTF32 = (TF32)max( Min, Max ); break;
  case WBVI_UINT: MinTU32 = (TU32)min( Min, Max ); MaxTU32 = (TU32)max( Min, Max ); break;
  case WBVI_INT64: MinTS64 = (TS64)min( Min, Max ); MaxTS64 = (TS64)max( Min, Max ); break;
  case WBVI_DOUBLE: MinTF64 = (TF64)min( Min, Max ); MaxTF64 = (TF64)max( Min, Max ); break;
  case WBVI_UINT64: MinTU64 = (TU64)min( Min, Max ); MaxTU64 = (TU64)max( Min, Max ); break;
  }
}

void CWBValueInput::SetConstraints( TU64 Min, TU64 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: MinTS32 = (TS32)min( Min, Max ); MaxTS32 = (TS32)max( Min, Max ); break;
  case WBVI_FLOAT: MinTF32 = (TF32)min( Min, Max ); MaxTF32 = (TF32)max( Min, Max ); break;
  case WBVI_UINT: MinTU32 = (TU32)min( Min, Max ); MaxTU32 = (TU32)max( Min, Max ); break;
  case WBVI_INT64: MinTS64 = (TS64)min( Min, Max ); MaxTS64 = (TS64)max( Min, Max ); break;
  case WBVI_DOUBLE: MinTF64 = (TF64)min( Min, Max ); MaxTF64 = (TF64)max( Min, Max ); break;
  case WBVI_UINT64: MinTU64 = (TU64)min( Min, Max ); MaxTU64 = (TU64)max( Min, Max ); break;
  }
}

void CWBValueInput::ClearConstraints()
{
  MinTS32 = INT_MIN;
  MinTF32 = FLT_MIN;
  MinTU32 = 0;
  MinTS64 = LLONG_MIN;
  MinTF64 = DBL_MIN;
  MinTU64 = 0;

  MaxTS32 = INT_MAX;
  MaxTF32 = FLT_MAX;
  MaxTU32 = UINT_MAX;
  MaxTS64 = LLONG_MAX;
  MaxTF64 = DBL_MAX;
  MaxTU64 = ULLONG_MAX;
}

TBOOL CWBValueInput::GetValue( TS32 &Value )
{
  if ( GetValueType() != WBVI_INT ) return false;
  Value = ValueTS32;
  return true;
}

TBOOL CWBValueInput::GetValue( TF32 &Value )
{
  if ( GetValueType() != WBVI_FLOAT ) return false;
  Value = ValueTF32;
  return true;
}

TBOOL CWBValueInput::GetValue( TU32 &Value )
{
  if ( GetValueType() != WBVI_UINT ) return false;
  Value = ValueTU32;
  return true;
}

TBOOL CWBValueInput::GetValue( TS64 &Value )
{
  if ( GetValueType() != WBVI_INT64 ) return false;
  Value = ValueTS64;
  return true;
}

TBOOL CWBValueInput::GetValue( TF64 &Value )
{
  if ( GetValueType() != WBVI_DOUBLE ) return false;
  Value = ValueTF64;
  return true;
}

TBOOL CWBValueInput::GetValue( TU64 &Value )
{
  if ( GetValueType() != WBVI_UINT64 ) return false;
  Value = ValueTU64;
  return true;
}

TBOOL CWBValueInput::GetValueSafe( TS32 &Value, TS32 Min, TS32 Max )
{
  if ( GetValueType() != WBVI_INT ) return false;
  Value = max( Min, min( Max, ValueTS32 ) );
  return true;
}

TBOOL CWBValueInput::GetValueSafe( TF32 &Value, TF32 Min, TF32 Max )
{
  if ( GetValueType() != WBVI_FLOAT ) return false;
  Value = max( Min, min( Max, ValueTF32 ) );
  return true;
}

TBOOL CWBValueInput::GetValueSafe( TU32 &Value, TU32 Min, TU32 Max )
{
  if ( GetValueType() != WBVI_UINT ) return false;
  Value = max( Min, min( Max, ValueTU32 ) );
  return true;
}

TBOOL CWBValueInput::GetValueSafe( TS64 &Value, TS64 Min, TS64 Max )
{
  if ( GetValueType() != WBVI_INT64 ) return false;
  Value = max( Min, min( Max, ValueTS64 ) );
  return true;
}

TBOOL CWBValueInput::GetValueSafe( TF64 &Value, TF64 Min, TF64 Max )
{
  if ( GetValueType() != WBVI_DOUBLE ) return false;
  Value = max( Min, min( Max, ValueTF64 ) );
  return true;
}

TBOOL CWBValueInput::GetValueSafe( TU64 &Value, TU64 Min, TU64 Max )
{
  if ( GetValueType() != WBVI_UINT64 ) return false;
  Value = max( Min, min( Max, ValueTU64 ) );
  return true;
}

TS32 CWBValueInput::GetValueAsInt()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TS32)ValueTS32;
  case WBVI_FLOAT: return (TS32)ValueTF32;
  case WBVI_UINT: return (TS32)ValueTU32;
  case WBVI_INT64: return (TS32)ValueTS64;
  case WBVI_DOUBLE: return (TS32)ValueTF64;
  case WBVI_UINT64: return (TS32)ValueTU64;
  }
  return 0;
}

TF32 CWBValueInput::GetValueAsFloat()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TF32)ValueTS32;
  case WBVI_FLOAT: return (TF32)ValueTF32;
  case WBVI_UINT: return (TF32)ValueTU32;
  case WBVI_INT64: return (TF32)ValueTS64;
  case WBVI_DOUBLE: return (TF32)ValueTF64;
  case WBVI_UINT64: return (TF32)ValueTU64;
  }
  return 0;
}

TU32 CWBValueInput::GetValueAsUnsignedInt()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TU32)ValueTS32;
  case WBVI_FLOAT: return (TU32)ValueTF32;
  case WBVI_UINT: return (TU32)ValueTU32;
  case WBVI_INT64: return (TU32)ValueTS64;
  case WBVI_DOUBLE: return (TU32)ValueTF64;
  case WBVI_UINT64: return (TU32)ValueTU64;
  }
  return 0;
}

TS64 CWBValueInput::GetValueAsInt64()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TS64)ValueTS32;
  case WBVI_FLOAT: return (TS64)ValueTF32;
  case WBVI_UINT: return (TS64)ValueTU32;
  case WBVI_INT64: return (TS64)ValueTS64;
  case WBVI_DOUBLE: return (TS64)ValueTF64;
  case WBVI_UINT64: return (TS64)ValueTU64;
  }
  return 0;
}

TF64 CWBValueInput::GetValueAsDouble()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TF64)ValueTS32;
  case WBVI_FLOAT: return (TF64)ValueTF32;
  case WBVI_UINT: return (TF64)ValueTU32;
  case WBVI_INT64: return (TF64)ValueTS64;
  case WBVI_DOUBLE: return (TF64)ValueTF64;
  case WBVI_UINT64: return (TF64)ValueTU64;
  }
  return 0;
}

TU64 CWBValueInput::GetValueAsUnsignedInt64()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TU64)ValueTS32;
  case WBVI_FLOAT: return (TU64)ValueTF32;
  case WBVI_UINT: return (TU64)ValueTU32;
  case WBVI_INT64: return (TU64)ValueTS64;
  case WBVI_DOUBLE: return (TU64)ValueTF64;
  case WBVI_UINT64: return (TU64)ValueTU64;
  }
  return 0;
}

TS32 CWBValueInput::GetValueAsIntSafe( TS32 Min, TS32 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return max( Min, min( Max, (TS32)ValueTS32 ) );
  case WBVI_FLOAT: return max( Min, min( Max, (TS32)ValueTF32 ) );
  case WBVI_UINT: return max( Min, min( Max, (TS32)ValueTU32 ) );
  case WBVI_INT64: return max( Min, min( Max, (TS32)ValueTS64 ) );
  case WBVI_DOUBLE: return max( Min, min( Max, (TS32)ValueTF64 ) );
  case WBVI_UINT64: return max( Min, min( Max, (TS32)ValueTU64 ) );
  }
  return 0;
}

TF32 CWBValueInput::GetValueAsFloatSafe( TF32 Min, TF32 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return max( Min, min( Max, (TF32)ValueTS32 ) );
  case WBVI_FLOAT: return max( Min, min( Max, (TF32)ValueTF32 ) );
  case WBVI_UINT: return max( Min, min( Max, (TF32)ValueTU32 ) );
  case WBVI_INT64: return max( Min, min( Max, (TF32)ValueTS64 ) );
  case WBVI_DOUBLE: return max( Min, min( Max, (TF32)ValueTF64 ) );
  case WBVI_UINT64: return max( Min, min( Max, (TF32)ValueTU64 ) );
  }
  return 0;
}

TU32 CWBValueInput::GetValueAsUnsignedIntSafe( TU32 Min, TU32 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return max( Min, min( Max, (TU32)ValueTS32 ) );
  case WBVI_FLOAT: return max( Min, min( Max, (TU32)ValueTF32 ) );
  case WBVI_UINT: return max( Min, min( Max, (TU32)ValueTU32 ) );
  case WBVI_INT64: return max( Min, min( Max, (TU32)ValueTS64 ) );
  case WBVI_DOUBLE: return max( Min, min( Max, (TU32)ValueTF64 ) );
  case WBVI_UINT64: return max( Min, min( Max, (TU32)ValueTU64 ) );
  }
  return 0;
}

TS64 CWBValueInput::GetValueAsInt64Safe( TS64 Min, TS64 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return max( Min, min( Max, (TS64)ValueTS32 ) );
  case WBVI_FLOAT: return max( Min, min( Max, (TS64)ValueTF32 ) );
  case WBVI_UINT: return max( Min, min( Max, (TS64)ValueTU32 ) );
  case WBVI_INT64: return max( Min, min( Max, (TS64)ValueTS64 ) );
  case WBVI_DOUBLE: return max( Min, min( Max, (TS64)ValueTF64 ) );
  case WBVI_UINT64: return max( Min, min( Max, (TS64)ValueTU64 ) );
  }
  return 0;
}

TF64 CWBValueInput::GetValueAsDoubleSafe( TF64 Min, TF64 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return max( Min, min( Max, (TF64)ValueTS32 ) );
  case WBVI_FLOAT: return max( Min, min( Max, (TF64)ValueTF32 ) );
  case WBVI_UINT: return max( Min, min( Max, (TF64)ValueTU32 ) );
  case WBVI_INT64: return max( Min, min( Max, (TF64)ValueTS64 ) );
  case WBVI_DOUBLE: return max( Min, min( Max, (TF64)ValueTF64 ) );
  case WBVI_UINT64: return max( Min, min( Max, (TF64)ValueTU64 ) );
  }
  return 0;
}

TU64 CWBValueInput::GetValueAsUnsignedInt64Safe( TU64 Min, TU64 Max )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return max( Min, min( Max, (TU64)ValueTS32 ) );
  case WBVI_FLOAT: return max( Min, min( Max, (TU64)ValueTF32 ) );
  case WBVI_UINT: return max( Min, min( Max, (TU64)ValueTU32 ) );
  case WBVI_INT64: return max( Min, min( Max, (TU64)ValueTS64 ) );
  case WBVI_DOUBLE: return max( Min, min( Max, (TU64)ValueTF64 ) );
  case WBVI_UINT64: return max( Min, min( Max, (TU64)ValueTU64 ) );
  }
  return 0;
}

TF32 CWBValueInput::GetValuePositionRelativeToConstraints()
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return (TF32)( ( ValueTS32 - MinTS32 ) / (TF64)( MaxTS32 - MinTS32 ) );
  case WBVI_FLOAT: return (TF32)( ( ValueTF32 - MinTF32 ) / (TF64)( MaxTF32 - MinTF32 ) );
  case WBVI_UINT: return (TF32)( ( ValueTU32 - MinTU32 ) / (TF64)( MaxTU32 - MinTU32 ) );
  case WBVI_INT64: return (TF32)( ( ValueTS64 - MinTS64 ) / (TF64)( MaxTS64 - MinTS64 ) );
  case WBVI_DOUBLE: return (TF32)( ( ValueTF64 - MinTF64 ) / (TF64)( MaxTF64 - MinTF64 ) );
  case WBVI_UINT64: return (TF32)( ( ValueTU64 - MinTU64 ) / (TF64)( MaxTU64 - MinTU64 ) );
  }
  return 0;
}

void CWBValueInput::SetInterpolatedValue( TF32 t )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return SetValue( Lerp( MinTS32, MaxTS32, t ) );
  case WBVI_FLOAT: return SetValue( Lerp( MinTF32, MaxTF32, t ) );
  case WBVI_UINT: return SetValue( Lerp( MinTU32, MaxTU32, t ) );
  case WBVI_INT64: return SetValue( Lerp( MinTS64, MaxTS64, t ) );
  case WBVI_DOUBLE: return SetValue( Lerp( MinTF64, MaxTF64, t ) );
  case WBVI_UINT64: return SetValue( Lerp( MinTU64, MaxTU64, t ) );
  }
}

void CWBValueInput::ApplyStep( TS32 StepCount )
{
  switch ( GetValueType() )
  {
  case WBVI_INT: return SetValue( ValueTS32 + StepCount*StepTS32 );
  case WBVI_FLOAT: return SetValue( ValueTF32 + StepCount*StepTF32 );
  case WBVI_UINT: return SetValue( ValueTU32 + StepCount*StepTU32 );
  case WBVI_INT64: return SetValue( ValueTS64 + StepCount*StepTS64 );
  case WBVI_DOUBLE: return SetValue( ValueTF64 + StepCount*StepTF64 );
  case WBVI_UINT64: return SetValue( ValueTU64 + StepCount*StepTU64 );
  }
}
