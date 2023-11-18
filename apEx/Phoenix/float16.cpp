//#include "float16.h"
//#include <D3DX10Math.h>
//
////http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
//
////union Bits
////{
////	float f;
////	int si;
////	unsigned int ui;
////};
////
////static int const shift = 13;
////static int const shiftSign = 16;
////static int const infN = 0x7F800000; // flt32 infinity
////static int const maxN = 0x477FE000; // max flt16 normal as a flt32
////static int const minN = 0x38800000; // min flt16 normal as a flt32
////static int const signN = 0x80000000; // flt32 sign bit
////static int const infC = infN >> shift;
//////static int const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
////static int const maxC = maxN >> shift;
////static int const minC = minN >> shift;
////static int const signC = signN >> shiftSign; // flt16 sign bit
//////static int const mulN = 0x52000000; // (1 << 23) / minN
////static int const mulC = 0x33800000; // minN / (1 << (23 - shift))
////static int const subC = 0x003FF; // max flt32 subnormal down shifted
////static int const norC = 0x00400; // min flt32 normal down shifted
////static int const maxD = infC - maxC - 1;
////static int const minD = minC - subC - 1;
////
////static float decompressf16(unsigned short value)
////{
////	Bits v;
////	v.ui = value;
////	int sign = v.si & signC;
////	v.si ^= sign;
////	sign <<= shiftSign;
////	v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
////	v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
////	Bits s;
////	s.si = mulC;
////	s.f *= v.si;
////	int mask = -(norC > v.si);
////	v.si <<= shift;
////	v.si ^= (s.si ^ v.si) & mask;
////	v.si |= sign;
////	return v.f;
////}
////
////TF16::operator float()
////{
////	return decompressf16(value);
////}
//
//#pragma comment(lib,"d3dx10.lib")
//
//TF16::operator float () 
//{ 
//	FLOAT f;
//	D3DXFloat16To32Array(&f, (D3DXFLOAT16*)this, 1);
//	return f;
//}
