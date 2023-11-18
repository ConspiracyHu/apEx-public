#include "BasePCH.h"
//#include "../Phoenix/float16.h"
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
////
////static int const infN = 0x7F800000; // flt32 infinity
////static int const maxN = 0x477FE000; // max flt16 normal as a flt32
////static int const minN = 0x38800000; // min flt16 normal as a flt32
////static int const signN = 0x80000000; // flt32 sign bit
////
////static int const infC = infN >> shift;
////static int const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
////static int const maxC = maxN >> shift;
////static int const minC = minN >> shift;
////static int const signC = signN >> shiftSign; // flt16 sign bit
////
////static int const mulN = 0x52000000; // (1 << 23) / minN
////static int const mulC = 0x33800000; // minN / (1 << (23 - shift))
////
////static int const subC = 0x003FF; // max flt32 subnormal down shifted
////static int const norC = 0x00400; // min flt32 normal down shifted
////
////static int const maxD = infC - maxC - 1;
////static int const minD = minC - subC - 1;
////
////static unsigned short compressf16(float value)
////{
////	Bits v, s;
////	v.f = value;
////	unsigned int sign = v.si & signN;
////	v.si ^= sign;
////	sign >>= shiftSign; // logical shift
////	s.si = mulN;
////	s.si = (int)(s.f * v.f); // correct subnormals
////	v.si ^= (s.si ^ v.si) & -(minN > v.si);
////	v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
////	v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
////	v.ui >>= shift; // logical shift
////	v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
////	v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
////	return v.ui | sign;
////}
//
//TF16::TF16( float f )
//{
//	value=XMConvertFloatToHalf(f);
//	//value=compressf16(f);
//}
