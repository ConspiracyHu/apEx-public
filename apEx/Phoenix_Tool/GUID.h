#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"

class CphxGUID
{
  friend class CphxGUIDServer;

  char GUID[ 33 ];
public:
  CphxGUID();
  bool operator==( const CphxGUID &other ) const;
  bool operator!=( const CphxGUID &other ) const;
  CphxGUID& operator=( const CphxGUID &rhs );

  char *GetString();
  void SetString( char *String );
};

class CphxGUIDServer
{
#define MERS_N   624
#define MERS_M   397
#define MERS_R   31
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   18
#define MERS_A   0x9908B0DF
#define MERS_B   0x9D2C5680
#define MERS_C   0xEFC60000

  unsigned int mt[ MERS_N ];
  int mti;

  void srand( unsigned int seed );
  unsigned char getnum();
private:


public:
  CphxGUIDServer( unsigned int Seed );
  CphxGUID RequestGUID();
};

extern CphxGUIDServer GUIDServer;