#pragma once
#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")

#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

//#include "float16.h"

#define SPLINEPOSITIONCALC( t ) ( t ? ( t + 1.0f ) / 256.0f : 0 )

class CphxSplineKey
{
public:
  unsigned char t;

  D3DXFLOAT16 Value[ 4 ];

  D3DXFLOAT16 controlvalues[ 6 ]; //bezier control point values
  unsigned char controlpositions[ 2 ]; //bezier control point positions

  float GetTime() { return SPLINEPOSITIONCALC( t ); }
  void GetValue( float v[ 4 ] );
};

enum SPLINEINTERPOLATION
{
  INTERPOLATION_CONSTANT = 0,
  INTERPOLATION_LINEAR = 1,
  INTERPOLATION_CUBIC = 2,
  INTERPOLATION_BEZIER = 3,
};

enum SPLINEWAVEFORM
{
  WAVEFORM_NONE = 0,
  WAVEFORM_SIN = 1,
  WAVEFORM_SQUARE = 2,
  WAVEFORM_TRIANGLE = 3,
  WAVEFORM_SAWTOOTH = 4,
  WAVEFORM_NOISE = 5,
};

enum DEFAULTSPLINETYPE
{
  //DEFAULTSPLINE_NOT_DEFAULT = 0,
  DEFAULTSPLINE_NOT_DEFAULT = 0,
  DEFAULTSPLINE_ZERO = 1,              // just a zero value, null rotation for quaternion splines
  DEFAULTSPLINE_ONE = 2,              // just a one value
  DEFAULTSPLINE_CONSTANTVALUE = 3,     // constant value, no key count needed

/*
  DEFAULTSPLINE_POOL_STORE = 0,        // build spline pool
  DEFAULTSPLINE_ZERO = 1,              // just a zero value, null rotation for quaternion splines
  DEFAULTSPLINE_ONE  = 2,              // just a one value
  DEFAULTSPLINE_CONSTANT_TWOBYTE = 3,  // d3dxfloat16
  DEFAULTSPLINE_ZEROTOONE = 4,         // linear zero to one
  DEFAULTSPLINE_CONSTANT_ONEBYTE = 5,  // unsigned char 0..255
  DEFAULTSPLINE_POOL_ONEBYTEINDEX = 6, // spline 0..255
  DEFAULTSPLINE_DEFAULTMATERIALPARAM = 7, // constant value not stored, fetch from default material value for material spline
*/

  DEFAULTSPLINE_UNKNOWN,               // used by the tool on export
};

//struct MINIMALSPLINESETUPDATA
//{
//	unsigned int Loop : 1;
//	unsigned int Interpolation : 2;
//	unsigned int Waveform : 3;
//	unsigned int DefaultSpline : 2;
//};

#define WAVEFORMSPLINENOISEBUFFERSIZE 8192

class CphxSpline
{
public:

  float Value[ 4 ]; //calculated output value

  SPLINEINTERPOLATION Interpolation;
  bool Loop;

  CphxSplineKey **Keys;
  int KeyCount;

  SPLINEWAVEFORM Waveform;
  D3DXFLOAT16 WaveformAmplitude;
  D3DXFLOAT16 WaveformFrequency;
  bool MultiplicativeWaveform;
  unsigned char RandSeed;
  bool NoiseCalculated;
  float NoiseMap[ WAVEFORMSPLINENOISEBUFFERSIZE ];

  void CalculateValue( float t );
#if defined SPLINE_WAVEFORM_SIN || defined SPLINE_WAVEFORM_SQUARE || defined SPLINE_WAVEFORM_TRIANGLE || defined SPLINE_WAVEFORM_SAWTOOTH || defined SPLINE_WAVEFORM_NOISE
  virtual void PostProcess( float t ) {};
#endif

#if defined SPLINE_INTERPOLATION_LINEAR && defined SPLINE_INTERPOLATION_SLERP
  virtual void Lerp( CphxSplineKey* a, CphxSplineKey* b, float t ) = 0;
#else
#if defined SPLINE_INTERPOLATION_LINEAR || defined SPLINE_INTERPOLATION_SLERP
  virtual void Lerp( CphxSplineKey* a, CphxSplineKey* b, float t ) {}
#endif
#endif

#if defined SPLINE_INTERPOLATION_CUBIC && defined SPLINE_INTERPOLATION_SQUAD
  virtual void QuadraticInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t ) = 0;
#else
#if defined SPLINE_INTERPOLATION_CUBIC || defined SPLINE_INTERPOLATION_SQUAD
  virtual void QuadraticInterpolation( CphxSplineKey* a, CphxSplineKey* b, CphxSplineKey* c, CphxSplineKey* d, float t ) {};
#endif
#endif
  //virtual void BezierInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t ) = 0;

  D3DXQUATERNION GetQuaternion() { return D3DXQUATERNION( Value ); }
  D3DXVECTOR3 GetVector() { return D3DXVECTOR3( Value ); }

  virtual const int GetKeyFloatCount() { return 1; }
};

class CphxSpline_float16 : public CphxSpline
{
public:

#if defined SPLINE_WAVEFORM_SIN || defined SPLINE_WAVEFORM_SQUARE || defined SPLINE_WAVEFORM_TRIANGLE || defined SPLINE_WAVEFORM_SAWTOOTH || defined SPLINE_WAVEFORM_NOISE
  virtual void PostProcess( float t );
#endif

#ifdef SPLINE_INTERPOLATION_LINEAR
  virtual void Lerp( CphxSplineKey *a, CphxSplineKey *b, float t );
#endif
#ifdef SPLINE_INTERPOLATION_CUBIC
  virtual void QuadraticInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t );
#endif
#ifdef SPLINE_INTERPOLATION_BEZIER
  virtual void BezierInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t );
#endif
};

class CphxSpline_Quaternion16 : public CphxSpline
{
public:

#ifdef SPLINE_INTERPOLATION_SLERP
  virtual void Lerp( CphxSplineKey *a, CphxSplineKey *b, float t );
#endif
#ifdef SPLINE_INTERPOLATION_SQUAD
  virtual void QuadraticInterpolation( CphxSplineKey *a, CphxSplineKey *b, CphxSplineKey *c, CphxSplineKey *d, float t );
#endif
  virtual const int GetKeyFloatCount() { return 4; }
};
