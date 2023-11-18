// Written by Fabian "ryg" Giesen.
// I hereby place this code in the public domain.

#ifndef __DIS_HPP__
#define __DIS_HPP__

#include "_types.hpp"

namespace rekkrunchy
{

  /****************************************************************************/

#define NBUFFERS  20

  struct DataBuffer
  {
    sInt Size, Max;
    sU8 *Data;

    DataBuffer();
    ~DataBuffer();
    void Clear();
    void Append( sU8 *data, sInt size );
    void PutValue( sU32 value, sInt size );
  };

  class DisFilter
  {
    DataBuffer Buffer[ NBUFFERS ];
    DataBuffer SourceBuffer[ NBUFFERS ];
    sInt FuncTablePos;
    sU32 FuncTable[ 255 ];
    sBool NextFunc;
    sU32 LastJump;
    sU32 JumpTable;
    class DebugInfo *Info;

    sU32 CodeSize;

    sInt CountInstr( sU8 *instr );
    sInt ProcessInstr( sU8 *instr, const sU8* codeStart, sU32 memory, sU32 VA );

  public:
    DataBuffer Output;
    DataBuffer SourceDataOutput;
    sU8 Table[ 256 ];

    void Filter( sU8 *code, sInt size, sU32 VA, DebugInfo *info = 0 );
  };

  class ReorderBuffer;

  void DisUnFilter( sU8 *packed, sU8 *dest, sU32 oldAddr, sU32 newAddr, ReorderBuffer &reord );

  /****************************************************************************/

}
#endif
