#pragma once
#pragma push_macro("new")
#undef new
#include <D3DX10Math.h>
#pragma pop_macro("new")


//////////////////////////////////////////////////////////////////////////
// basic math

extern const float pi;

float lerp( float a, float b, float t );
float catmullrom( float a, float b, float c, float d, float t );
float getbeziert( float p0, float p1, float p2, float p3, float w );
float bezier( float p0, float p1, float p2, float p3, float t );
D3DXVECTOR3 bezier( D3DXVECTOR3 p0, D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, float t );
D3DXVECTOR3 GetRandomVertexOnSphere();

void _cdecl qsort( void *base, size_t num, size_t width, int( _cdecl *comp )( const void *, const void * ) );
