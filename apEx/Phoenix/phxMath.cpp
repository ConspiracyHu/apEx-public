#include "phxMath.h"
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

//////////////////////////////////////////////////////////////////////////
// basic math

static const float pi = 3.14159265f;

float lerp( float a, float b, float t )
{
  return ( b - a )*t + a;
}

#ifdef SPLINE_INTERPOLATION_CUBIC
float catmullrom( float a, float b, float c, float d, float t )
{
  float P = ( d - c ) - ( a - b );
  float Q = ( a - b ) - P;
  float R = c - a;
  return ( P*t*t*t ) + ( Q*t*t ) + ( R*t ) + b;
}
#endif

#ifdef SPLINE_INTERPOLATION_BEZIER
float getbeziert( float p0, float p1, float p2, float p3, float w )
{
  //calculate t value for a given x position of a bezier curve

  if ( p0 == p1 && w == 0 ) return 0; //special cases
  if ( p2 == p3 && w == 1 ) return 1;

  //analytical method:

  //float x=(p1-p0)/(p3-p0);
  //float y=(p2-p0)/(p3-p0);

  ////t^3 ( 3 x - 3 y + 1 )- t^2 ( 6 x - 3 y )+3 t x - w = 0

  //float a=3*(x-y)+1;
  //float b=(y-2*x)/a;
  //float c=x/a;
  //float d=-w/a/2.0f;

  //float Q = b * b - c;
  //float R = b * ( Q - c/2.0f ) + d;
  //float disc = R * R - Q * Q * Q;

  ///* One real root */
  //if (disc > 0) 
  //{
  //	float e = powf(sqrtf(disc) + fabs(R), 1.0f / 3.0f);
  //	if (R > 0)
  //		e = -e;
  //	return e + Q / e - b;
  //}
  ///* Three real roots */
  //else 
  //{
  //	float theta = acosf(R / sqrtf(Q * Q * Q))/3.0f;
  //	float sqrtQ = -2 * sqrtf(Q);

  //	float root1 = sqrtQ * cosf( theta ) - b;
  //	if (root1>=0 && root1<=1) return root1;
  //	theta+=2.0f/3.0f*PI_;

  //	float root2 = sqrtQ * cosf( theta ) - b;
  //	if (root2>=0 && root2<=1) return root2;
  //	theta+=2.0f/3.0f*PI_;

  //	return sqrtQ * cosf( theta ) - b;
  //}

  //iterative method:

  float xv = w*( p3 - p0 ); // normalize x value to the curve

  float c = 3 * ( p1 - p0 );
  float b = 3 * ( p2 - p1 ) - c;
  float a = p3 - p0 - c - b;
  float t = w;

  //first approximate with binary search for a few steps - 4 steps give a nice result, 6 are perfect
  float bnd1 = 0;
  float bnd2 = 1;

  for ( int z = 0; z < 4; z++ )
  {
    t = ( bnd2 + bnd1 ) / 2;
    if ( a*t*t*t + b*t*t + c*t - xv < 0 ) bnd1 = t;
    else bnd2 = t;
  }

  // newton's method finishes the calculation
  for ( int i = 0; i < 10; i++ )
    t = t - ( ( a*t*t + b*t + c )*t - xv ) / ( ( 3 * a*t + 2 * b )*t + c );

  if ( t >= 0 && t <= 1 ) return t;
  return 0;
}

float bezier( float p0, float p1, float p2, float p3, float t )
{
  float ti = 1 - t;
  float a = ti*ti*ti;
  float b = 3 * ti*ti*t;
  float c = 3 * ti*t*t;
  float d = t*t*t;
  return a*p0 + b*p1 + c*p2 + d*p3;
}

D3DXVECTOR3 bezier( D3DXVECTOR3 p0, D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, float t )
{
  return D3DXVECTOR3( bezier( p0.x, p1.x, p2.x, p3.x, t ),
                      bezier( p0.y, p1.y, p2.y, p3.y, t ),
                      bezier( p0.z, p1.z, p2.z, p3.z, t ) );
}
#endif

#ifdef PHX_SCATTER_HAS_ORIENTATION_FULLROTATE
D3DXVECTOR3 GetRandomVertexOnSphere()
{
  D3DXVECTOR3 v;
  do
  {
    v.x = ( ( rand() / (float)RAND_MAX ) - 0.5f )*2.0f;
    v.y = ( ( rand() / (float)RAND_MAX ) - 0.5f )*2.0f;
    v.z = ( ( rand() / (float)RAND_MAX ) - 0.5f )*2.0f;
  } while ( D3DXVec3Length( &v ) > 1 );
  D3DXVec3Normalize( &v, &v );
  return v;
}
#endif