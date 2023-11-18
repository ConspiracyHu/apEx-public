#include "phxSpline.h"
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif
//d3dx10 needed for half-math
//#pragma comment(lib,"d3dx10.lib")
#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")

#include "phxMath.h"

//void FixFloatingPoint();

//////////////////////////////////////////////////////////////////////////
// spline key class

void CphxSplineKey::GetValue( float v[ 4 ] )
{
  for ( int x = 0; x < 4; x++ )
    v[ x ] = Value[ x ];
}

//////////////////////////////////////////////////////////////////////////
// spline class

//OH PLEASE GOD FORGIVE ME, BUT IT'S SMALLER THIS WAY...
#define POSTPROCESS goto postproc

void CphxSpline::CalculateValue( float t )
{
  //FixFloatingPoint();
  if ( !KeyCount )
    return;

  if ( KeyCount == 1 || ( !Loop && t <= Keys[ 0 ]->GetTime() ) )
  {
    Keys[ 0 ]->GetValue( Value );
    POSTPROCESS;
    return;
  }

  if ( !Loop && t >= Keys[ KeyCount - 1 ]->GetTime() )
  {
    Keys[ KeyCount - 1 ]->GetValue( Value );
    POSTPROCESS;
    return;
  }

  //at this point we have at least two keys and t is between valid key values

  int pos = -1;
  while ( pos < KeyCount - 1 && Keys[ ( pos + 1 ) % KeyCount ]->GetTime() <= t ) pos++;
  pos += KeyCount - 1;

  CphxSplineKey *NeededKeys[ 4 ];
  for ( int x = 0; x < 4; x++ )	NeededKeys[ x ] = Keys[ ( pos + x ) % KeyCount ];

  //t is between neededkeys[1] and neededkeys[2] - if the difference in their time value is negative we have looped so we need to have fract(1+diff)

  float distkeys;//=KeyDistance(NeededKeys[1],NeededKeys[2],Loop);
  distkeys = NeededKeys[ 2 ]->GetTime() - NeededKeys[ 1 ]->GetTime();
  if ( Loop ) distkeys = 1 + distkeys - (int)( 1 + distkeys );

  //same fix for the t value between keys if we looped

  float partialt = t - NeededKeys[ 1 ]->GetTime() + 1; //don't change the order of the substraction here as it introduces float rounding errors in the next line (t-NeededKeys[1]->GetTime() sometimes don't align perfectly then)
  partialt = ( partialt - (int)( partialt ) ) / distkeys;

  switch ( Interpolation )
  {
#ifdef SPLINE_INTERPOLATION_CONSTANT
  case INTERPOLATION_CONSTANT:
    NeededKeys[ 1 ]->GetValue( Value );
    break;
#endif
#if defined SPLINE_INTERPOLATION_LINEAR || defined SPLINE_INTERPOLATION_SLERP
  case INTERPOLATION_LINEAR:
    Lerp( NeededKeys[ 1 ], NeededKeys[ 2 ], partialt );
    break;
#endif
#if defined SPLINE_INTERPOLATION_CUBIC || defined SPLINE_INTERPOLATION_SQUAD
  case INTERPOLATION_CUBIC:
    QuadraticInterpolation( NeededKeys[ 0 ], NeededKeys[ 1 ], NeededKeys[ 2 ], NeededKeys[ 3 ], partialt );
    break;
#endif
#ifdef SPLINE_INTERPOLATION_BEZIER
  case INTERPOLATION_BEZIER:
    ( ( CphxSpline_float16* )this )->BezierInterpolation( NeededKeys[ 0 ], NeededKeys[ 1 ], NeededKeys[ 2 ], NeededKeys[ 3 ], partialt );
    break;
#endif
  }

postproc:; //OH PLEASE GOD FORGIVE ME, BUT IT'S SMALLER THIS WAY...
#if defined SPLINE_WAVEFORM_SIN || defined SPLINE_WAVEFORM_SQUARE || defined SPLINE_WAVEFORM_TRIANGLE || defined SPLINE_WAVEFORM_SAWTOOTH || defined SPLINE_WAVEFORM_NOISE
  PostProcess( t );
#endif
}

#ifdef SPLINE_INTERPOLATION_LINEAR
void CphxSpline_float16::Lerp( CphxSplineKey *a, CphxSplineKey *b, float t )
{
  Value[ 0 ] = lerp( a->Value[ 0 ], b->Value[ 0 ], t );
}
#endif

#ifdef SPLINE_INTERPOLATION_CUBIC
void CphxSpline_float16::QuadraticInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t )
{
  Value[ 0 ] = catmullrom( a->Value[ 0 ], b->Value[ 0 ], c->Value[ 0 ], d->Value[ 0 ], t );
}
#endif

#ifdef SPLINE_INTERPOLATION_BEZIER
void CphxSpline_float16::BezierInterpolation( CphxSplineKey *k0, CphxSplineKey *k1, CphxSplineKey *k2, CphxSplineKey *k3, float t )
{
  //float d=KeyDistance(k1,k2,Loop);
  float t1 = k1->GetTime();
  float t2 = k2->GetTime();

  if ( t1 > t2 ) t2 += 1;

  t = getbeziert( t1,
                  t1 + k1->controlpositions[ 1 ] / 255.0f,
                  t2 - k2->controlpositions[ 0 ] / 255.0f,
                  t2,
                  t );
  Value[ 0 ] = bezier( k1->Value[ 0 ],
                       k1->Value[ 0 ] + k1->controlvalues[ 1 ],
                       k2->Value[ 0 ] - k2->controlvalues[ 0 ],
                       k2->Value[ 0 ],
                       t );
}
#endif

#if defined SPLINE_WAVEFORM_SIN || defined SPLINE_WAVEFORM_SQUARE || defined SPLINE_WAVEFORM_TRIANGLE || defined SPLINE_WAVEFORM_SAWTOOTH || defined SPLINE_WAVEFORM_NOISE

static float map[ WAVEFORMSPLINENOISEBUFFERSIZE ];

void CphxSpline_float16::PostProcess( float t )
{
  //	//this is for waveform boundary display purposes:
  //#ifdef COMPILED_FOR_TOOL
  //	PreWaveformValue=Value;
  //#endif

  if ( Waveform == WAVEFORM_NONE )
    return;

  float wf = 0;

  float ph = t*WaveformFrequency;

#if defined SPLINE_WAVEFORM_SIN || defined SPLINE_WAVEFORM_SQUARE
  float s = sin( ph*pi*2.0f );
#endif

  switch ( Waveform )
  {
#ifdef SPLINE_WAVEFORM_SIN
  case WAVEFORM_SIN:
    wf = s;
    break;
#endif
#ifdef SPLINE_WAVEFORM_SQUARE
  case WAVEFORM_SQUARE:
    if ( s == 0 )
      wf = 1.0;
    else
      wf = s / fabs( s );
    break;
#endif
#ifdef SPLINE_WAVEFORM_SAWTOOTH
  case WAVEFORM_SAWTOOTH:
  {
    float f = fmodf( ph, 2.0f );
    f = ( f > 1.0f ) ? 2 - f : f;
    wf = ( f - 0.5f ) * 2;
    break;
  }
#endif
#ifdef SPLINE_WAVEFORM_TRIANGLE
  case WAVEFORM_TRIANGLE:
    wf = ( fmodf( ph, 1 ) - 0.5f ) * 2;
    break;
#endif
#ifdef SPLINE_WAVEFORM_NOISE
  case WAVEFORM_NOISE:
  {
    if ( !NoiseCalculated )
    {
      srand( RandSeed );
      for ( int x = 0; x < WAVEFORMSPLINENOISEBUFFERSIZE; x++ )
        map[ x ] = rand() / (float)RAND_MAX;
      int sampleWidth = WAVEFORMSPLINENOISEBUFFERSIZE / max( 1, WaveformFrequency );

      for ( int z = 0; z < 3; z++ )
      {
        for ( int x = 0; x < WAVEFORMSPLINENOISEBUFFERSIZE; x++ )
        {
          float val = 0;
          for ( int y = 0; y < sampleWidth; y++ )
            val += map[ ( x + y ) % WAVEFORMSPLINENOISEBUFFERSIZE ];
          NoiseMap[ x ] = val / (float)sampleWidth;
        }
        for ( int x = 0; x < WAVEFORMSPLINENOISEBUFFERSIZE; x++ )
          map[ x ] = NoiseMap[ x ];
      }
      NoiseCalculated = true;
    }

    float tn = fmod( t * WAVEFORMSPLINENOISEBUFFERSIZE, 1 );
    int tp = (int)( t * WAVEFORMSPLINENOISEBUFFERSIZE );
    wf = ( NoiseMap[ tp ] + ( NoiseMap[ ( tp + 1 ) % WAVEFORMSPLINENOISEBUFFERSIZE ] - NoiseMap[ tp ] )*tn ) * 2 - 1;
  }
  break;
#endif
  }

  wf *= WaveformAmplitude;

  if ( MultiplicativeWaveform )
    Value[ 0 ] *= wf;
  else
    Value[ 0 ] += wf;
}
#endif


#ifdef SPLINE_INTERPOLATION_SLERP
void CphxSpline_Quaternion16::Lerp( CphxSplineKey *a, CphxSplineKey *b, float t )
{
  D3DXQUATERNION q1, q2, r;
  q1 = D3DXQUATERNION( a->Value );
  q2 = D3DXQUATERNION( b->Value );
  D3DXQuaternionSlerp( &r, &q1, &q2, t );
  Value[ 0 ] = r.x;
  Value[ 1 ] = r.y;
  Value[ 2 ] = r.z;
  Value[ 3 ] = r.w;
}
#endif

#ifdef SPLINE_INTERPOLATION_SQUAD
void CphxSpline_Quaternion16::QuadraticInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t )
{
  D3DXQUATERNION q0, q1, q2, q3, qa, qb, qc, r;
  q0 = D3DXQUATERNION( a->Value );
  q1 = D3DXQUATERNION( b->Value );
  q2 = D3DXQUATERNION( c->Value );
  q3 = D3DXQUATERNION( d->Value );

  D3DXQuaternionSquadSetup( &qa, &qb, &qc, &q0, &q1, &q2, &q3 );
  D3DXQuaternionSquad( &r, &q1, &qa, &qb, &qc, t );

  Value[ 0 ] = r.x;
  Value[ 1 ] = r.y;
  Value[ 2 ] = r.z;
  Value[ 3 ] = r.w;
}
#endif