#pragma once

/*

HLSL binary format

4 bytes char: 'DXBC'
16 bytes: checksum
4 bytes int: 0x01
4 bytes int: complete filesize
4 bytes int: chunk count (cc)
cc*4 bytes int: chunk offset table

---

chunk format:
4 bytes char: chunk ID ('ISGN', 'OSGN', 'SHDR')
4 bytes int: chunk data size (without chunk header)

---

ISGN/OSGN data format:
4 bytes int: signature member count (mc)
4 bytes int: offset to signature array (should always be 0x08)
24*mc bytes signature struct array:
  4 bytes: offset to signature type string (chunk data relative)
  4 bytes: index
  4 bytes: sysvalue ?
  4 bytes: format ?
  4 bytes: register?
  4 bytes: memberusagemask ? (.xyzw stuff)
array of asciiz strings referenced by the signature offsets, PADDED TO 4 ALIGNMENT BY 0xABABABAB

---

4 bytes int: shader version
4 bytes int: chunk size/4

token stream follows
token header: 4 bytes, last byte contains token length/4


*/
