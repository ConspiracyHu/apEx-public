#include "BasePCH.h"
#include "GUID.h"
#include <MMSystem.h>
#pragma comment(lib,"winmm.lib")

CphxGUIDServer GUIDServer( timeGetTime() );

CphxGUID::CphxGUID()
{
  memset( GUID, 0, 33 );
}

bool CphxGUID::operator ==( const CphxGUID &Other ) const
{
  for ( int x = 0; x < 32; x++ )
    if ( GUID[ x ] != Other.GUID[ x ] ) return false;
  return true;
}

bool CphxGUID::operator !=( const CphxGUID &Other ) const
{
  for ( int x = 0; x < 32; x++ )
    if ( GUID[ x ] != Other.GUID[ x ] ) return true;
  return false;
}

CphxGUID& CphxGUID::operator=( const CphxGUID &Original )
{
  for ( int x = 0; x < 32; x++ )
    GUID[ x ] = Original.GUID[ x ];
  return *this;
}

char *CphxGUID::GetString()
{
  return GUID;
}

void CphxGUID::SetString( char *String )
{
  if ( !String || strlen( String ) != 32 )
  {
    LOG_ERR( "[guid] Error: GUID '%s' isn't 32 bytes long.", String );
    SetString( "BADGUID!BADGUID!BADGUID!BADGUID!" );
    return;
  }
  memcpy( GUID, String, 32 );
}

unsigned char CphxGUIDServer::getnum()
{
  union { double f; unsigned int i[ 2 ]; } convert;

  unsigned int r;

  if ( mti >= MERS_N )
  {
    const unsigned int LOWER_MASK = ( 1LU << MERS_R ) - 1;
    const unsigned int UPPER_MASK = 0xFFFFFFFF << MERS_R;
    static const unsigned int mag01[ 2 ] = { 0, MERS_A };

    int kk;
    for ( kk = 0; kk < MERS_N - MERS_M; kk++ )
    {
      r = ( mt[ kk ] & UPPER_MASK ) | ( mt[ kk + 1 ] & LOWER_MASK );
      mt[ kk ] = mt[ kk + MERS_M ] ^ ( r >> 1 ) ^ mag01[ r & 1 ];
    }

    for ( ; kk < MERS_N - 1; kk++ )
    {
      r = ( mt[ kk ] & UPPER_MASK ) | ( mt[ kk + 1 ] & LOWER_MASK );
      mt[ kk ] = mt[ kk + ( MERS_M - MERS_N ) ] ^ ( r >> 1 ) ^ mag01[ r & 1 ];
    }

    r = ( mt[ MERS_N - 1 ] & UPPER_MASK ) | ( mt[ 0 ] & LOWER_MASK );
    mt[ MERS_N - 1 ] = mt[ MERS_M - 1 ] ^ ( r >> 1 ) ^ mag01[ r & 1 ];
    mti = 0;
  }

  r = mt[ mti++ ];

  // Tempering (May be omitted):
  r ^= r >> MERS_U;
  r ^= ( r << MERS_S ) & MERS_B;
  r ^= ( r << MERS_T ) & MERS_C;
  r ^= r >> MERS_L;

  convert.i[ 0 ] = r << 20;
  convert.i[ 1 ] = ( r >> 12 ) | 0x3FF00000;
  return (unsigned char)( ( convert.f - 1.0 ) * 16 );
}

CphxGUIDServer::CphxGUIDServer( unsigned int Seed )
{
  mt[ 0 ] = Seed;
  for ( mti = 1; mti < MERS_N; mti++ ) {
    mt[ mti ] = ( 1812433253UL * ( mt[ mti - 1 ] ^ ( mt[ mti - 1 ] >> 30 ) ) + mti );
  }
}

char ToHexa( unsigned char c )
{
  switch ( c )
  {
  case 0x0: return '0';
  case 0x1: return '1';
  case 0x2: return '2';
  case 0x3: return '3';
  case 0x4: return '4';
  case 0x5: return '5';
  case 0x6: return '6';
  case 0x7: return '7';
  case 0x8: return '8';
  case 0x9: return '9';
  case 0xa: return 'A';
  case 0xb: return 'B';
  case 0xc: return 'C';
  case 0xd: return 'D';
  case 0xe: return 'E';
  case 0xf: return 'F';
  }
  return 'x';
}

CphxGUID CphxGUIDServer::RequestGUID()
{
  CphxGUID g;
  for ( int x = 0; x < 32; x++ )
    g.GUID[ x ] = ToHexa( getnum() );
  return g;
}