// Written by Fabian "ryg" Giesen.
// I hereby place this code in the public domain.

#ifndef __RANGECODER_HPP__
#define __RANGECODER_HPP__

#include "_types.hpp"

namespace rekkrunchy
{

  class DebugInfo;

  /****************************************************************************/

  typedef void( *PackerCallback )( sU32 srcPos, sU32 srcSize, sU32 dstPos );

  sU32 RangecoderPack( const sU8 *in, double* packHeatMap, sU32 inSize, sU8 *out, PackerCallback cb, DebugInfo* info );
  sU32 RangecoderDepack( sU8 *out, const sU8 *in, sInt outSize );

  /****************************************************************************/

}
#endif
