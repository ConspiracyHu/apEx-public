#include "BasePCH.h"
#include "phxSplineExt.h"

void CphxSpline_Tool::UpdateSplineKeys()
{
  for ( TS32 x = 0; x < Spline->KeyCount; x++ )
    SAFEDELETE( Spline->Keys[ x ] );
  SAFEDELETEA( Spline->Keys );
  Spline->KeyCount = Keys.NumItems();

  if ( !Keys.NumItems() ) return;

  TS32 KeyCount = Keys.NumItems();
  Spline->Keys = new CphxSplineKey*[ KeyCount ];

  for ( TS32 x = 0; x < KeyCount; x++ )
  {
    Spline->Keys[ x ] = new CphxSplineKey();
    Spline->Keys[ x ]->controlpositions[ 0 ] = Keys[ x ]->Key.controlpositions[ 0 ];
    Spline->Keys[ x ]->controlpositions[ 1 ] = Keys[ x ]->Key.controlpositions[ 1 ];

    Spline->Keys[ x ]->t = Keys[ x ]->Key.t;

    for ( TS32 y = 0; y < 6; y++ )
      Spline->Keys[ x ]->controlvalues[ y ] = Keys[ x ]->Key.controlvalues[ y ];

    for ( TS32 y = 0; y < 4; y++ )
      Spline->Keys[ x ]->Value[ y ] = Keys[ x ]->Key.Value[ y ];
  }
}

CphxSpline_Tool::CphxSpline_Tool()
{
}

CphxSpline_Tool::~CphxSpline_Tool()
{
  Keys.FreeArray();
  if ( Spline->Keys )
  {
    for ( TS32 x = 0; x < Spline->KeyCount; x++ )
      SAFEDELETE( Spline->Keys[ x ] );
    SAFEDELETEA( Spline->Keys );
  }
  SAFEDELETE( Spline );
}

void CphxSpline_Tool::GetValue( float t, float Value[ 4 ] )
{
  Spline->CalculateValue( t );
  for ( TS32 x = 0; x < 4; x++ )
    Value[ x ] = Spline->Value[ x ];
}

TS32 SplineKeySorter( CphxSplineKey_Tool **a, CphxSplineKey_Tool **b )
{
  TS32 v = (TS32)( *a )->Key.t - (TS32)( *b )->Key.t;
  return v;
  //return *a-*b;
}

void CphxSpline_Tool::Sort()
{
  Keys.Sort( SplineKeySorter );
  UpdateSplineKeys();
}

void CphxSpline_Tool::ExportKeys( CXMLNode &node )
{
  if ( !Keys.NumItems() )
  {
    for ( TS32 x = 0; x < 4; x++ )
    {
      CXMLNode n = node.AddChild( _T( "value" ) );
      D3DXFLOAT16 f = D3DXFLOAT16( Spline->Value[ x ] );
      n.SetAttributeFromInteger( _T( "id" ), x );
      n.SetInt( *( (WORD*)&f ) );
    }
  }

  for ( TS32 x = 0; x < Keys.NumItems(); x++ )
  {
    CXMLNode n = node.AddChild( _T( "key" ) );
    n.AddChild( _T( "time" ), false ).SetInt( Keys[ x ]->Key.t );

    for ( TS32 y = 0; y < 4; y++ )
    {
      CXMLNode v = n.AddChild( _T( "value" ), false );
      v.SetInt( *( (WORD*)&Keys[ x ]->Key.Value[ y ] ) );
      v.SetAttributeFromInteger( _T( "id" ), y );
    }

    for ( TS32 y = 0; y < 2; y++ )
    {
      CXMLNode v = n.AddChild( _T( "controlpos" ), false );
      v.SetInt( Keys[ x ]->Key.controlpositions[ y ] );
      v.SetAttributeFromInteger( _T( "id" ), y );
    }

    for ( TS32 y = 0; y < 6; y++ )
    {
      CXMLNode v = n.AddChild( _T( "controlvalue" ), y == 5 );
      v.SetInt( *( (WORD*)&Keys[ x ]->Key.controlvalues[ y ] ) );
      v.SetAttributeFromInteger( _T( "id" ), y );
    }
  }
}

void CphxSpline_Tool::ImportKeys( CXMLNode &node )
{
  for ( TS32 x = 0; x < node.GetChildCount( _T( "value" ) ); x++ )
  {
    CXMLNode n = node.GetChild( _T( "value" ), x );
    TS32 v = 0;
    TS32 val;
    if ( n.HasAttribute( _T( "id" ) ) ) n.GetAttributeAsInteger( _T( "id" ), &v );
    if ( n.GetValue( val ) ) Spline->Value[ v ] = *( (D3DXFLOAT16*)&val );
  }

  TS32 KeyCount = node.GetChildCount( _T( "key" ) );
  for ( TS32 x = 0; x < KeyCount; x++ )
  {
    CXMLNode n = node.GetChild( _T( "key" ), x );

    CphxSplineKey_Tool *k = new CphxSplineKey_Tool();
    Keys += k;

    TS32 val;
    TS32 id;
    if ( n.GetChildCount( _T( "time" ) ) ) if ( n.GetChild( _T( "time" ) ).GetValue( val ) ) k->Key.t = val;

    for ( TS32 y = 0; y < n.GetChildCount( _T( "value" ) ); y++ )
    {
      CXMLNode v = n.GetChild( _T( "value" ), y );
      id = y;
      v.GetAttributeAsInteger( _T( "id" ), &id );
      if ( v.GetValue( val ) ) k->Key.Value[ id ] = *( (D3DXFLOAT16*)&val );
    }

    for ( TS32 y = 0; y < n.GetChildCount( _T( "controlpos" ) ); y++ )
    {
      CXMLNode v = n.GetChild( _T( "controlpos" ), y );
      id = y;
      v.GetAttributeAsInteger( _T( "id" ), &id );
      if ( v.GetValue( val ) ) k->Key.controlpositions[ id ] = val;
    }

    for ( TS32 y = 0; y < n.GetChildCount( _T( "controlvalue" ) ); y++ )
    {
      CXMLNode v = n.GetChild( _T( "controlvalue" ), y );
      id = y;
      v.GetAttributeAsInteger( _T( "id" ), &id );
      if ( v.GetValue( val ) ) k->Key.controlvalues[ id ] = *( (D3DXFLOAT16*)&val );
    }
  }
}

void CphxSpline_Tool::Backup()
{
  for ( TS32 x = 0; x < 4; x++ )
    ValueBackup[ x ] = Spline->Value[ x ];

  StoreKeyData();
}

void CphxSpline_Tool::StoreKeyData()
{
  for ( TS32 y = 0; y < Keys.NumItems(); y++ )
    Keys[ y ]->Stored = Keys[ y ]->Key;
}

void CphxSpline_Tool::ApplySplineTransformation( TF32 Timestamp, TF32 Value, TBOOL AutoKey )
{
  TF32 delta = Value - ValueBackup[ 0 ];

  Spline->Value[ 0 ] = D3DXFLOAT16( ValueBackup[ 0 ] + delta );

  if ( !AutoKey )
    for ( TS32 x = 0; x < Keys.NumItems(); x++ )
      Keys[ x ]->Key.Value[ 0 ] = Keys[ x ]->Stored.Value[ 0 ] + delta;
  else
  {
    TBOOL Found = false;
    for ( TS32 x = 0; x < Keys.NumItems(); x++ )
    {
      if ( Keys[ x ]->Key.GetTime() == Timestamp )
      {
        Keys[ x ]->Key.Value[ 0 ] = Keys[ x ]->Stored.Value[ 0 ] + delta;
        Found = true;
      }
    }

    if ( !Found )
    {
      CphxSplineKey_Tool* newkey = new CphxSplineKey_Tool();
      newkey->Key.t = (TU8)( max( 0, min( 1, Timestamp ) * 256 - 1 )  );
      newkey->Key.Value[ 0 ] = Value;
      newkey->Stored.Value[ 0 ] = ValueBackup[ 0 ];
      Keys.Add( newkey );
    }

    Sort();
  }

  UpdateSplineKeys();
}

void CphxSpline_Tool::ApplySplineTransformation( TF32 Timestamp, D3DXQUATERNION Value, TBOOL AutoKey )
{
  D3DXQUATERNION backup = D3DXQUATERNION( ValueBackup );
  D3DXQUATERNION backupinv;
  D3DXQuaternionInverse( &backupinv, &backup );
  D3DXQUATERNION delta;
  D3DXQuaternionMultiply( &delta, &Value, &backupinv );

  D3DXQUATERNION result;
  D3DXQuaternionMultiply( &result, &delta, &backup );
  Spline->Value[ 0 ] = D3DXFLOAT16( result.x );
  Spline->Value[ 1 ] = D3DXFLOAT16( result.y );
  Spline->Value[ 2 ] = D3DXFLOAT16( result.z );
  Spline->Value[ 3 ] = D3DXFLOAT16( result.w );

  if ( !AutoKey )
  {
    for ( TS32 x = 0; x < Keys.NumItems(); x++ )
    {
      D3DXQuaternionMultiply( &result, &delta, &D3DXQUATERNION( Keys[ x ]->Stored.Value ) );
      Keys[ x ]->Key.Value[ 0 ] = result.x;
      Keys[ x ]->Key.Value[ 1 ] = result.y;
      Keys[ x ]->Key.Value[ 2 ] = result.z;
      Keys[ x ]->Key.Value[ 3 ] = result.w;
    }
  }
  else
  {
    TBOOL Found = false;
    for ( TS32 x = 0; x < Keys.NumItems(); x++ )
    {
      if ( Keys[ x ]->Key.GetTime() == Timestamp )
      {
        D3DXQuaternionMultiply( &result, &delta, &( D3DXQUATERNION( Keys[ x ]->Stored.Value ) ) );
        Keys[ x ]->Key.Value[ 0 ] = result.x;
        Keys[ x ]->Key.Value[ 1 ] = result.y;
        Keys[ x ]->Key.Value[ 2 ] = result.z;
        Keys[ x ]->Key.Value[ 3 ] = result.w;
        Found = true;
      }
    }

    if ( !Found )
    {
      CphxSplineKey_Tool* newkey = new CphxSplineKey_Tool();
      newkey->Key.t = (TU8)( max( 0, min( 1, Timestamp ) * 256 - 1 ) );
      newkey->Key.Value[ 0 ] = Value.x;
      newkey->Key.Value[ 1 ] = Value.y;
      newkey->Key.Value[ 2 ] = Value.z;
      newkey->Key.Value[ 3 ] = Value.w;
      newkey->Stored.Value[ 0 ] = ValueBackup[ 0 ];
      newkey->Stored.Value[ 1 ] = ValueBackup[ 1 ];
      newkey->Stored.Value[ 2 ] = ValueBackup[ 2 ];
      newkey->Stored.Value[ 3 ] = ValueBackup[ 3 ];
      Keys.Add( newkey );
    }

    Sort();
  }

  UpdateSplineKeys();
}

void CphxSpline_Tool::AddKey( TF32 TimeStamp, TF32 Value )
{
  CphxSplineKey_Tool *k = new CphxSplineKey_Tool();
  Keys += k;
  k->Key.t = (int)( max( 0, min( 1, TimeStamp ) )*255.0f );
  k->Key.Value[ 0 ] = Value;
  Sort();
}

CphxSpline_Tool_float16::CphxSpline_Tool_float16()
{
  CphxSpline_float16 *s = new CphxSpline_float16();

  s->Waveform = WAVEFORM_NONE;
  s->MultiplicativeWaveform = false;
  s->WaveformAmplitude = 1;
  s->WaveformFrequency = 10;
  s->RandSeed = 0;
  s->NoiseCalculated = false;

  Spline = s;
  Spline->Keys = NULL;
  Spline->KeyCount = 0;
  Spline->Interpolation = INTERPOLATION_LINEAR;
  Spline->Loop = false;
  for ( TS32 x = 0; x < 4; x++ )
    Spline->Value[ x ] = 0;

  UpdateSplineKeys();
}

void CphxSpline_Tool_float16::ExportData( CXMLNode &node )
{
  CphxSpline_float16 *s = (CphxSpline_float16*)Spline;
  node.AddChild( _T( "interpolation" ), false ).SetInt( s->Interpolation );
  node.AddChild( _T( "loop" ), false ).SetInt( s->Loop );
  node.AddChild( _T( "waveform" ), false ).SetInt( s->Waveform );
  node.AddChild( _T( "multiplicativewaveform" ), false ).SetInt( s->MultiplicativeWaveform );
  node.AddChild( _T( "wfamplitude" ), false ).SetInt( *( (WORD*)&s->WaveformAmplitude ) );
  node.AddChild( _T( "wffrequency" ), false ).SetInt( *( (WORD*)&s->WaveformFrequency ) );
  node.AddChild( _T( "wfrandseed" ), false ).SetInt( s->RandSeed );

  ExportKeys( node );
}

void CphxSpline_Tool_float16::ImportData( CXMLNode &node )
{
  CphxSpline_float16 *s = (CphxSpline_float16*)Spline;
  TS32 val = s->Interpolation;
  if ( node.GetChildCount( _T( "interpolation" ) ) ) node.GetChild( _T( "interpolation" ) ).GetValue( val ); s->Interpolation = (SPLINEINTERPOLATION)val;
  val = s->Loop; if ( node.GetChildCount( _T( "loop" ) ) ) node.GetChild( _T( "loop" ) ).GetValue( val ); s->Loop = val != 0;
  val = s->Waveform; if ( node.GetChildCount( _T( "waveform" ) ) ) node.GetChild( _T( "waveform" ) ).GetValue( val ); s->Waveform = (SPLINEWAVEFORM)val;
  val = s->MultiplicativeWaveform; if ( node.GetChildCount( _T( "multiplicativewaveform" ) ) ) node.GetChild( _T( "multiplicativewaveform" ) ).GetValue( val ); s->MultiplicativeWaveform = val != 0;

  val = *( (WORD*)&s->WaveformAmplitude ); if ( node.GetChildCount( _T( "wfamplitude" ) ) ) node.GetChild( _T( "wfamplitude" ) ).GetValue( val ); s->WaveformAmplitude = *( (D3DXFLOAT16*)&val );
  val = *( (WORD*)&s->WaveformFrequency ); if ( node.GetChildCount( _T( "wffrequency" ) ) ) node.GetChild( _T( "wffrequency" ) ).GetValue( val ); s->WaveformFrequency = *( (D3DXFLOAT16*)&val );

  val = s->RandSeed; if ( node.GetChildCount( _T( "wfrandseed" ) ) ) node.GetChild( _T( "wfrandseed" ) ).GetValue( val ); s->RandSeed = val;

  ImportKeys( node );
  Sort();
}

CphxSpline_Tool * CphxSpline_Tool_float16::Copy()
{
  CphxSpline_float16 *s = (CphxSpline_float16*)Spline;
  CphxSpline_Tool_float16 *dst = new CphxSpline_Tool_float16();
  CphxSpline_float16 *d = (CphxSpline_float16*)dst->Spline;
  d->Waveform = s->Waveform;
  d->Interpolation = s->Interpolation;
  d->Loop = s->Loop;
  d->MultiplicativeWaveform = s->MultiplicativeWaveform;
  d->WaveformAmplitude = s->WaveformAmplitude;
  d->WaveformFrequency = s->WaveformFrequency;
  d->RandSeed = d->RandSeed;
  for ( TS32 x = 0; x < 4; x++ )
    d->Value[ x ] = s->Value[ x ];

  for ( TS32 x = 0; x < Keys.NumItems(); x++ )
  {
    CphxSplineKey_Tool *n = new CphxSplineKey_Tool();
    n->Selected = Keys[ x ]->Selected;
    n->Key = Keys[ x ]->Key;
    n->Stored = Keys[ x ]->Stored;
    dst->Keys += n;
  }

  dst->UpdateSplineKeys();

  return dst;
}

void CphxSpline_Tool_float16::CopyTo( CphxSpline_Tool *Target )
{
  CphxSpline_float16 *s = (CphxSpline_float16*)Spline;
  CphxSpline_Tool_float16 *dst = (CphxSpline_Tool_float16 *)Target;
  CphxSpline_float16 *d = (CphxSpline_float16*)dst->Spline;
  d->Waveform = s->Waveform;
  d->Interpolation = s->Interpolation;
  d->Loop = s->Loop;
  d->MultiplicativeWaveform = s->MultiplicativeWaveform;
  d->WaveformAmplitude = s->WaveformAmplitude;
  d->WaveformFrequency = s->WaveformFrequency;
  d->RandSeed = s->RandSeed;
  for ( TS32 x = 0; x < 4; x++ )
    d->Value[ x ] = s->Value[ x ];

  Target->Keys.FreeArray();

  for ( TS32 x = 0; x < Keys.NumItems(); x++ )
  {
    CphxSplineKey_Tool *n = new CphxSplineKey_Tool();
    n->Selected = Keys[ x ]->Selected;
    n->Key = Keys[ x ]->Key;
    n->Stored = Keys[ x ]->Stored;
    dst->Keys += n;
  }

  dst->UpdateSplineKeys();
}

bool CphxSpline_Tool_float16::IsZeroToOne()
{
  if ( Keys.NumItems() != 2 )
    return false;
  if ( Spline->Waveform )
    return false;
  if ( Spline->Interpolation != INTERPOLATION_LINEAR )
    return false;
  if ( Keys[ 0 ]->Key.t != 0 )
    return false;
  if ( Keys[ 1 ]->Key.t != 1 )
    return false;
  if ( Keys[ 0 ]->Key.Value[ 0 ] != D3DXFLOAT16( 0.0 ) )
    return false;
  if ( Keys[ 1 ]->Key.Value[ 0 ] != D3DXFLOAT16( 1.0 ) )
    return false;
  return true;
}

bool CphxSpline_Tool_float16::IsConstant( D3DXFLOAT16 constValue[ 4 ] )
{
  CphxSpline_float16* sp = (CphxSpline_float16*)Spline;

  D3DXFLOAT16 value = 0;
  bool isConstant = true;

  /*
    if ( !Spline->Keys.NumItems() )
      value = sp->Value[ 0 ];
    if ( Spline->Keys.NumItems() == 1 )
      value = Spline->Keys[ 0 ]->Key.Value[ 0 ];

    if ( Spline->Keys.NumItems() > 1 ) // go through every time position on the spline and check if the value is the same
  */
  {
    sp->CalculateValue( 0 );
    value = sp->Value[ 0 ];
    for ( int x = 0; x < 512; x++ )
    {
      sp->CalculateValue( x / 512.0f );
      D3DXFLOAT16 v2 = sp->Value[ 0 ];
      if ( value != v2 )
      {
        isConstant = false;
        break;
      }
    }
  }

  if ( isConstant )
    constValue[ 0 ] = value;
  return isConstant;
}

bool CphxSpline_Tool_float16::HasIntValue( int v )
{
  CphxSpline_float16* sp = (CphxSpline_float16*)Spline;
  for ( int x = 0; x < 512; x++ )
  {
    sp->CalculateValue( x / 512.0f );
    int v2 = (int)sp->Value[ 0 ];
    if ( v == v2 )
      return true;
  }
  return false;
}

const TBOOL CphxSpline_Tool_float16::operator==( const CphxSpline_Tool_float16& in ) const
{
  if ( in.Keys.NumItems() != Keys.NumItems() )
    return false;

  if ( !Keys.NumItems() && Spline->Value[ 0 ] != in.Spline->Value[ 0 ] )
    return false;

  if ( !Spline || !in.Spline )
    return false;

  if ( Spline->Interpolation != in.Spline->Interpolation )
    return false;

  if ( Spline->Loop != in.Spline->Loop )
    return false;

  if ( Spline->Waveform != in.Spline->Waveform )
    return false;

  if ( Spline->Waveform != WAVEFORM_NONE )
  {
    if ( Spline->WaveformAmplitude != in.Spline->WaveformAmplitude )
      return false;

    if ( Spline->WaveformFrequency != in.Spline->WaveformFrequency )
      return false;

    if ( Spline->MultiplicativeWaveform != in.Spline->MultiplicativeWaveform )
      return false;

    if ( Spline->Waveform == WAVEFORM_NOISE )
    {
      if ( Spline->RandSeed != in.Spline->RandSeed )
        return false;
    }
  }

  for ( int x = 0; x < Keys.NumItems(); x++ )
  {
    auto& k1 = Keys[ x ]->Key;
    auto& k2 = in.Keys[ x ]->Key;

    if ( k1.t != k2.t )
      return false;

    if ( k1.Value[ 0 ] != k2.Value[ 0 ] )
      return false;

    if ( Spline->Interpolation == INTERPOLATION_BEZIER )
    {
      if ( k1.controlpositions[ 0 ] != k2.controlpositions[ 0 ] )
        return false;
      if ( k1.controlpositions[ 1 ] != k2.controlpositions[ 1 ] )
        return false;

      if ( k1.controlvalues[ 0 ] != k2.controlvalues[ 0 ] )
        return false;
      if ( k1.controlvalues[ 1 ] != k2.controlvalues[ 1 ] )
        return false;
    }
  }

  return true;
}

CphxSpline_Tool_Quaternion16::CphxSpline_Tool_Quaternion16()
{
  Spline = new CphxSpline_Quaternion16();
  Spline->Keys = NULL;
  Spline->KeyCount = 0;
  Spline->Interpolation = INTERPOLATION_LINEAR;
  Spline->Loop = false;
  for ( TS32 x = 0; x < 4; x++ )
    Spline->Value[ x ] = 0;
}

void CphxSpline_Tool_Quaternion16::ExportData( CXMLNode &node )
{
  CphxSpline_Quaternion16 *s = (CphxSpline_Quaternion16*)Spline;
  node.AddChild( _T( "interpolation" ), false ).SetInt( s->Interpolation );
  node.AddChild( _T( "loop" ), false ).SetInt( s->Loop );

  ExportKeys( node );
}

void CphxSpline_Tool_Quaternion16::ImportData( CXMLNode &node )
{
  CphxSpline_Quaternion16 *s = (CphxSpline_Quaternion16*)Spline;
  TS32 val = s->Interpolation;
  if ( node.GetChildCount( _T( "interpolation" ) ) ) node.GetChild( _T( "interpolation" ) ).GetValue( val ); s->Interpolation = (SPLINEINTERPOLATION)val;
  val = s->Loop; if ( node.GetChildCount( _T( "loop" ) ) ) node.GetChild( _T( "loop" ) ).GetValue( val ); s->Loop = val != 0;

  ImportKeys( node );
  Sort();
}

CphxSpline_Tool * CphxSpline_Tool_Quaternion16::Copy()
{
  CphxSpline_Quaternion16 *s = (CphxSpline_Quaternion16*)Spline;
  CphxSpline_Tool_Quaternion16 *dst = new CphxSpline_Tool_Quaternion16();
  CphxSpline_Quaternion16 *d = (CphxSpline_Quaternion16*)dst->Spline;
  d->Interpolation = s->Interpolation;
  d->Loop = s->Loop;
  for ( TS32 x = 0; x < 4; x++ )
    d->Value[ x ] = s->Value[ x ];

  for ( TS32 x = 0; x < Keys.NumItems(); x++ )
  {
    CphxSplineKey_Tool *n = new CphxSplineKey_Tool();
    n->Selected = Keys[ x ]->Selected;
    n->Key = Keys[ x ]->Key;
    n->Stored = Keys[ x ]->Stored;
    dst->Keys += n;
  }

  dst->UpdateSplineKeys();

  return dst;
}

void CphxSpline_Tool_Quaternion16::CopyTo( CphxSpline_Tool *Target )
{
  CphxSpline_Quaternion16 *s = (CphxSpline_Quaternion16*)Spline;
  CphxSpline_Tool_Quaternion16 *dst = (CphxSpline_Tool_Quaternion16 *)Target;
  CphxSpline_Quaternion16 *d = (CphxSpline_Quaternion16*)dst->Spline;
  d->Interpolation = s->Interpolation;
  d->Loop = s->Loop;
  for ( TS32 x = 0; x < 4; x++ )
    d->Value[ x ] = s->Value[ x ];

  Target->Keys.FreeArray();

  for ( TS32 x = 0; x < Keys.NumItems(); x++ )
  {
    CphxSplineKey_Tool *n = new CphxSplineKey_Tool();
    n->Selected = Keys[ x ]->Selected;
    n->Key = Keys[ x ]->Key;
    n->Stored = Keys[ x ]->Stored;
    dst->Keys += n;
  }

  dst->UpdateSplineKeys();
}


bool CphxSpline_Tool_Quaternion16::IsConstant( D3DXFLOAT16 constValue[ 4 ] )
{
  CphxSpline_float16* sp = (CphxSpline_float16*)Spline;

  D3DXFLOAT16 value[ 4 ]{};
  bool isConstant = true;

  /*
    if ( !Spline->Keys.NumItems() )
      value = sp->Value[ 0 ];
    if ( Spline->Keys.NumItems() == 1 )
      value = Spline->Keys[ 0 ]->Key.Value[ 0 ];

    if ( Spline->Keys.NumItems() > 1 ) // go through every time position on the spline and check if the value is the same
  */
  {
    sp->CalculateValue( 0 );
    value[ 0 ] = sp->Value[ 0 ];
    value[ 1 ] = sp->Value[ 1 ];
    value[ 2 ] = sp->Value[ 2 ];
    value[ 3 ] = sp->Value[ 3 ];
    for ( int x = 0; x < 512; x++ )
    {
      sp->CalculateValue( x / 512.0f );
      if ( value[ 0 ] != (D3DXFLOAT16)( sp->Value[ 0 ] ) ||
           value[ 1 ] != (D3DXFLOAT16)( sp->Value[ 1 ] ) ||
           value[ 2 ] != (D3DXFLOAT16)( sp->Value[ 2 ] ) ||
           value[ 3 ] != (D3DXFLOAT16)( sp->Value[ 3 ] ) )
      {
        isConstant = false;
        break;
      }
    }
  }

  if ( isConstant )
  {
    constValue[ 0 ] = value[ 0 ];
    constValue[ 1 ] = value[ 1 ];
    constValue[ 2 ] = value[ 2 ];
    constValue[ 3 ] = value[ 3 ];
  }
  return isConstant;
}

const TBOOL CphxSpline_Tool_Quaternion16::operator==( const CphxSpline_Tool_Quaternion16& in ) const
{
  if ( in.Keys.NumItems() != Keys.NumItems() )
    return false;

  if ( !Keys.NumItems() && Spline->Value[ 0 ] != in.Spline->Value[ 0 ] )
    return false;

  if ( !Keys.NumItems() && Spline->Value[ 1 ] != in.Spline->Value[ 1 ] )
    return false;

  if ( !Keys.NumItems() && Spline->Value[ 2 ] != in.Spline->Value[ 2 ] )
    return false;

  if ( !Keys.NumItems() && Spline->Value[ 3 ] != in.Spline->Value[ 3 ] )
    return false;

  if ( !Spline || !in.Spline )
    return false;

  if ( Spline->Interpolation != in.Spline->Interpolation )
    return false;

  if ( Spline->Loop != in.Spline->Loop )
    return false;

/*
  if ( Spline->Waveform != in.Spline->Waveform )
    return false;

  if ( Spline->Waveform != WAVEFORM_NONE )
  {
    if ( Spline->WaveformAmplitude != in.Spline->WaveformAmplitude )
      return false;

    if ( Spline->WaveformFrequency != in.Spline->WaveformFrequency )
      return false;

    if ( Spline->MultiplicativeWaveform != in.Spline->MultiplicativeWaveform )
      return false;

    if ( Spline->Waveform == WAVEFORM_NOISE )
    {
      if ( Spline->RandSeed != in.Spline->RandSeed )
        return false;
    }
  }
*/

  for ( int x = 0; x < Keys.NumItems(); x++ )
  {
    auto& k1 = Keys[ x ]->Key;
    auto& k2 = in.Keys[ x ]->Key;

    if ( k1.t != k2.t )
      return false;

    for ( int y = 0; y < 4; y++ )
      if ( k1.Value[ y ] != k2.Value[ y ] )
        return false;

/*
    if ( Spline->Interpolation == INTERPOLATION_BEZIER )
    {
      if ( k1.controlpositions[ 0 ] != k2.controlpositions[ 0 ] )
        return false;
      if ( k1.controlpositions[ 1 ] != k2.controlpositions[ 1 ] )
        return false;

      if ( k1.controlvalues[ 0 ] != k2.controlvalues[ 0 ] )
        return false;
      if ( k1.controlvalues[ 1 ] != k2.controlvalues[ 1 ] )
        return false;
    }
*/
  }

  return true;
}

CphxSplineKey_Tool::CphxSplineKey_Tool()
{
  Key.Value[ 0 ] = Key.Value[ 1 ] = Key.Value[ 2 ] = Key.Value[ 3 ] = 0;
  Key.controlvalues[ 0 ] = Key.controlvalues[ 1 ] = Key.controlvalues[ 2 ] = Key.controlvalues[ 3 ] = Key.controlvalues[ 4 ] = Key.controlvalues[ 5 ] = 0;
  Key.controlpositions[ 0 ] = Key.controlpositions[ 1 ] = 15;
  Key.t = 0;
  Selected = false;
}
