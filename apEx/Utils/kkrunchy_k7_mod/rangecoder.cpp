// Written by Fabian "ryg" Giesen.
// I hereby place this code in the public domain.

#include "_types.hpp"
#include "rangecoder.hpp"
#include "debuginfo.hpp"
#include <cmath>

// ---- helpers

namespace rekkrunchy
{

  extern "C" void __fastcall modelInitASM();
  extern "C" sInt __fastcall modelASM( sInt bit );

  static sU32 sMulShift12( sU32 a, sU32 b )
  {
    __asm
    {
      mov   eax, [ a ];
      mul[ b ];
      shrd  eax, edx, 12;
    }
  }

  // ---- encoder

  extern "C" sU8 *bufPtr, *startBufPtr;

  sU8 *bufPtr, *startBufPtr;
  static sU8 *outPtr;
  static sInt ffNum, cache;
  static sU64 low;
  static sU32 range;
  static sBool firstByte;
  static sU32 byteCount;

  static void shiftLow()
  {
    sU32 carry = sU32( low >> 32 );
    if ( low < 0xff000000 || carry == 1 )
    {
      if ( !firstByte )
        *outPtr++ = cache + carry;
      else
        firstByte = false;

      for ( ; ffNum; ffNum-- )
        *outPtr++ = 0xff + carry;

      cache = sInt( ( low >> 24 ) & 0xff );
    }
    else
      ffNum++;

    low = ( low << 8 ) & 0xffffffff;
    byteCount++;
  }

  static void codeBit( sU32 prob, sInt bit )
  {
    // adjust bound
    sU32 newBound = sMulShift12( range, prob );
    if ( bit )
      range = newBound;
    else
    {
      low += newBound;
      range -= newBound;
    }

    // renormalize
    while ( range < 0x01000000 )
    {
      range <<= 8;
      shiftLow();
    }
  }

  sU32 RangecoderPack( const sU8 *in, double* packHeatMap, sU32 inSize, sU8 *out, PackerCallback cb, DebugInfo* info )
  {
    sU32 prob = 2048;
    sU32 zeroProb = 1;

    outPtr = out;
    ffNum = 0;
    low = 0;
    range = ~0U;
    cache = 0;
    firstByte = true;
    modelInitASM();
    bufPtr = startBufPtr = (sU8 *)in;
    sF64 lastSize = 0.0;
    sInt curSymbol = 0;

    memset( packHeatMap, 0, inSize * sizeof( double ) );

    for ( sInt pos = 0; pos < inSize; pos++ )
    {
      // write zero tag bit every 8k
      if ( ( pos & 8191 ) == 0 )
      {
        // check whether we want to execute the callback first
        if ( cb )
        {
          __asm emms;
          cb( pos, inSize, sU32( outPtr - out ) );
        }

        // >8k still left?
        sInt isZero = ( inSize - pos ) > 8192;

        // check whether it's really all zeroes
        if ( isZero )
        {
          for ( sInt i = 0; i < 8192; i++ )
          {
            if ( bufPtr[ i ] != 0 )
            {
              isZero = 0;
              break;
            }
          }
        }

        codeBit( zeroProb, isZero );
        zeroProb = ( zeroProb + ( isZero ? 4096 : 1 ) ) >> 1;

        if ( isZero )
        {
          bufPtr += 8192;
          pos += 8192 - 1;
          continue;
        }
      }

      for ( sInt i = 0; i < 8; i++ )
      {
        sInt bit = ( *bufPtr >> ( 7 - i ) ) & 1;
        codeBit( prob, bit );

        // update model
        if ( i == 7 )
          bufPtr++;

        prob = modelASM( bit );
      }

      // update sizes
      double fracSize = -log( sF64( range ) / sF64( 0xFFFFFFFF ) ) / log( 2.0 ) / 8.0;
      double curSize = byteCount + fracSize;
      double byteSize = curSize - lastSize;

      packHeatMap[ pos ] += byteSize;

/*
      if ( info )
      {
        if ( pos < ( info->Symbols[ curSymbol ].sourcePos + info->Symbols[ curSymbol ].Size ) ) // we don't count padding for symbols
          info->Symbols[ curSymbol ].PackedSize += byteSize;

        if (info->Symbols[curSymbol].FileNum >= 0)
        {
          info->Files[info->Symbols[curSymbol].FileNum].PackedSize += byteSize;
          info->Files[info->Symbols[curSymbol].FileNum].Size += 1;
        }
        if (info->Symbols[curSymbol].NameSpNum >= 0)
        {
          info->NameSps[info->Symbols[curSymbol].NameSpNum].PackedSize += byteSize;
          info->NameSps[info->Symbols[curSymbol].NameSpNum].Size += 1;
        }
      }
*/

      lastSize = curSize;

/*
      // move to next symbol if applicable
      if (info)
      {
        if (curSymbol < info->Symbols.Count - 1 && pos >= info->Symbols[curSymbol + 1].sourcePos)
        {
/ *
          do
          {
* /
            curSymbol++;
//          } while (info->Symbols[curSymbol].FileNum<0);
        }

        if (curSymbol==358)
          int z = 0;
      }
*/

    }

    for ( sInt i = 0; i < 5; i++ ) // these bytes are not counted correctly
      shiftLow();

    __asm emms;
    sU32 finalSize = sU32( outPtr - out );

    if ( cb )
      cb( inSize, inSize, finalSize );

    return finalSize;
  }

  // ---- decoder

  sU32 RangecoderDepack( sU8 *out, const sU8 *in, sInt outSize )
  {
    // initialize ari
    sU32 code = 0, range = ~0U;
    sU32 prob = 2048;

    for ( sInt i = 0; i < 4; i++ )
      code = ( code << 8 ) | *in++;

    modelInitASM();
    bufPtr = startBufPtr = out;

    while ( outSize )
    {
      sInt byte = 0;

      for ( sInt i = 0; i < 8; i++ )
      {
        sInt bit;
        sU32 bound = sMulShift12( range, prob );

        // decode bit
        if ( code < bound )
        {
          range = bound;
          bit = 1;
        }
        else
        {
          code -= bound;
          range -= bound;
          bit = 0;
        }

        byte += byte + bit;
        if ( i == 7 )
          *bufPtr++ = byte;

        // renormalize
        while ( range < 0x01000000 )
        {
          code = ( code << 8 ) | *in++;
          range <<= 8;
        }

        // update model
        prob = modelASM( bit );
      }

      outSize--;
    }

    __asm emms;
  }

}