#include <windows.h>
//#include <malloc.h>
//#include <tchar.h>

#pragma warning( disable : 4725 )

extern "C" typedef void( __cdecl *_PVFV )( );

//#pragma intrinsic(memset,memcpy)
#pragma section(".CRT$XCA", read, write)
#pragma data_seg(".CRT$XCA")		// start of ctor section
_PVFV __xc_a[] = { 0 };

#pragma section(".CRT$XCZ", read, write)
#pragma data_seg(".CRT$XCZ")		// end of ctor section
_PVFV __xc_z[] = { 0 };

#pragma data_seg()
#pragma comment(linker, "/merge:.CRT=.rdata")

//__int64 allocatedMem = 0;

void *operator new[]( unsigned int s )
{
  //allocatedMem += s;
  return HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, s );
}

void operator delete( void *p )
{
  HeapFree( GetProcessHeap(), 0, p );
}

//void operator delete[](void *p)
//{
//	delete p;
//}

void *operator new( unsigned int s )
{
  return new unsigned char[ s ];
}

#ifndef _DEBUG
void *malloc( unsigned int s )
{
  return new unsigned char[ s ];
}

void free( void *p )
{
  delete p;
}
#endif

extern "C"
{
  __forceinline void _initterm( _PVFV *pfbegin, _PVFV *pfend )
  {
    // walk the table of function pointers from the bottom up, until
    // the end is encountered.  Do not skip the first entry.  The initial
    // value of pfbegin points to the first valid entry.  Do not try to
    // execute what pfend points to.  Only entries before pfend are valid.
    while ( pfbegin < pfend )
    {
      if ( *pfbegin != 0 )
        ( **pfbegin )( );
      ++pfbegin;
    }
  }

/*
  int _purecall()
  {
    return 0;
  }
*/

  int _fltused = 0;

/*
  __declspec( naked ) void _allmul()
  {
    #define arg_0 4
    #define arg_4 8
    #define arg_8 0x0C
    #define arg_C 0x10

    _asm
    {    
                    mov     eax, [esp+arg_4]
                    mov     ecx, [esp+arg_C]
                    or      ecx, eax
                    mov     ecx, [esp+arg_8]
                    jnz     short hard
                    mov     eax, [esp+arg_0]
                    mul     ecx
                    retn    10h
    ; ---------------------------------------------------------------------------
    
    hard:
                    push    ebx
                    mul     ecx
                    mov     ebx, eax
                    mov     eax, [esp+4+arg_0]
                    mul     [esp+4+arg_C]
                    add     ebx, eax
                    mov     eax, [esp+4+arg_0]
                    mul     ecx
                    add     edx, ebx
                    pop     ebx
                    retn    10h
    }

    #undef arg_0
    #undef arg_4
    #undef arg_8
    #undef arg_C
  }

  __declspec( naked ) void _alldiv()
  {
    #define arg_0 4
    #define arg_4 8
    #define arg_8 0x0c
    #define arg_C 0x10

    __asm
    {    
                    push    edi
                    push    esi
                    push    ebx
                    xor     edi, edi
                    mov     eax, [esp+0Ch+arg_4]
                    or      eax, eax
                    jge     short L1_0
                    inc     edi
                    mov     edx, [esp+0Ch+arg_0]
                    neg     eax
                    neg     edx
                    sbb     eax, 0
                    mov     [esp+0Ch+arg_4], eax
                    mov     [esp+0Ch+arg_0], edx
    
    L1_0:                                   
                    mov     eax, [esp+0Ch+arg_C]
                    or      eax, eax
                    jge     short L2_0
                    inc     edi
                    mov     edx, [esp+0Ch+arg_8]
                    neg     eax
                    neg     edx
                    sbb     eax, 0
                    mov     [esp+0Ch+arg_C], eax
                    mov     [esp+0Ch+arg_8], edx
    
    L2_0:                                   
                    or      eax, eax
                    jnz     short L3_0
                    mov     ecx, [esp+0Ch+arg_8]
                    mov     eax, [esp+0Ch+arg_4]
                    xor     edx, edx
                    div     ecx
                    mov     ebx, eax
                    mov     eax, [esp+0Ch+arg_0]
                    div     ecx
                    mov     edx, ebx
                    jmp     short L4_0
    
    L3_0:                                   
                    mov     ebx, eax
                    mov     ecx, [esp+0Ch+arg_8]
                    mov     edx, [esp+0Ch+arg_4]
                    mov     eax, [esp+0Ch+arg_0]
    
    L5_0:                                   
                    shr     ebx, 1
                    rcr     ecx, 1
                    shr     edx, 1
                    rcr     eax, 1
                    or      ebx, ebx
                    jnz     short L5_0
                    div     ecx
                    mov     esi, eax
                    mul     [esp+0Ch+arg_C]
                    mov     ecx, eax
                    mov     eax, [esp+0Ch+arg_8]
                    mul     esi
                    add     edx, ecx
                    jb      short L6
                    cmp     edx, [esp+0Ch+arg_4]
                    ja      short L6
                    jb      short L7
                    cmp     eax, [esp+0Ch+arg_0]
                    jbe     short L7
    
    L6:                                     
                    dec     esi
    
    L7:                                     
                    xor     edx, edx
                    mov     eax, esi
    
    L4_0:
                    dec     edi
                    jnz     short L8
                    neg     edx
                    neg     eax
                    sbb     edx, 0
    
    L8:
                    pop     ebx
                    pop     esi
                    pop     edi
                    retn    10h

    }

    #undef arg_0
    #undef arg_4
    #undef arg_8
    #undef arg_C
  }

  __declspec( naked ) void _allshl()
  {
    __asm
    {
      cmp     cl, 64
      jae     short RETZERO
      cmp     cl, 32
      jae     short MORE32
      shld    edx, eax, cl
      shl     eax, cl
      ret
    MORE32:
      mov     edx, eax
      xor     eax, eax
      and     cl, 31
      shl     edx, cl
      ret
    RETZERO :
      xor     eax, eax
      xor     edx, edx
      ret
    }
  }
*/

  __declspec( naked ) void acos()
  {
    __asm
    {
      fld qword ptr[ esp + 4 ]
      fld		st( 0 )
      fld		st( 0 )
      fmul
      fld1
      fsubr
      fsqrt
      fxch
      fpatan
      ret
    }
  }

  //	__declspec(naked) int rounding_common()
  //	{
  //		__asm
  //		{
  //			fnstcw[ESP - 8]
  //				fldcw[ESP]
  //				fistp dword ptr[ESP]
  //				pop EAX
  //				fldcw[ESP - 12]
  //				ret
  //		}
  //	}
  //
  //#ifndef _DEBUG
  //	//int __declspec(naked) floor(float x)
  //	//{
  //	//	__asm
  //	//	{
  //	//		fld dword ptr[ESP + 4]
  //	//			push 0x73f
  //	//			jmp rounding_common
  //	//	}
  //	//}
  //#endif


#ifndef _DEBUG
  double floor( double f )
  {
    const float half = -0.5f;
    int i;
    __asm {
      fld f
      fadd st, st( 0 )
      fadd half
      fistp i
      sar i, 1
    };

    return float( i );
  }
#endif

  float __declspec( naked ) cos()
  {
    _asm {
      fld qword ptr[ esp + 4 ]
      fcos
      ret
    };
  };

  float __declspec( naked ) sin()
  {
    _asm {
      fld qword ptr[ esp + 4 ]
      fsin
      ret
    };
  };

  float __declspec( naked ) sqrt()
  {
    _asm {
      fld qword ptr[ esp + 4 ]
      fsqrt
      ret
    };
  };

  float __declspec( naked ) atan2()
  {
    _asm {
      fld qword ptr[ esp + 4 ]
      fld qword ptr[ esp + 12 ]
      fpatan
      ret
    };
  };

  float __declspec( naked ) atan()
  {
    _asm {
      fld qword ptr[ esp + 4 ]
      fld1
      fpatan
      ret
    };
  };

  float __declspec( naked ) pow()
  {
    _asm {
      fld qword ptr[ esp + 4 ]
      fld qword ptr[ esp + 12 ]
      push eax
      fxch    st( 1 )
      ftst             // <--- CHECK FOR MOTHERFUCKING (0 ^ x)!!!!
      fstsw ax
      sahf
      jnz notzero
      fstp st( 1 )
      pop eax
      ret
      notzero :
      fyl2x
        fld st( 0 )
        frndint
        fxch
        fsub st( 0 ), st( 1 )
        f2xm1
        fld1
        faddp st( 1 ), st( 0 )
        fscale
        fstp st( 1 )
        pop eax
        ret
    };
  };

  float __declspec ( naked ) fmod()
  {
    __asm {
      fld qword ptr[ esp + 4 ]
      fld qword ptr[ esp + 12 ]
      fxch    st( 1 )
      __CIfmod1:
      fprem
        fstsw   ax
        test    ax, 0400h
        jnz     __CIfmod1
        fstp    st( 1 )
        ret
    }
  }

  double fabs( double f )
  {
    return f < 0 ? -f : f;
  }

  int abs( int f )
  {
    return f < 0 ? -f : f;
  }

  __declspec( naked ) int _ftol2_sse()
  {
    __asm
    {
      push				ebp;
      mov					ebp, esp;
      sub					esp, 20h;
      and					esp, -16;
      fld					st;
      fst					dword ptr[ esp + 18h ];
      fistp				qword ptr[ esp + 10h ];
      fild				qword ptr[ esp + 10h ];
      mov					edx, dword ptr[ esp + 18h ];
      mov					eax, dword ptr[ esp + 10h ];
      test				eax, eax;
      je					int_qnan_or_zero;

    not_int_qnan:
      fsubp				st( 1 ), st;
      test				edx, edx;
      jns					positive;
      fstp				dword ptr[ esp ];
      mov					ecx, [ esp ];
      xor					ecx, 80000000h;
      add					ecx, 7fffffffh;
      adc					eax, 0;
      mov					edx, dword ptr[ esp + 14h ];
      adc					edx, 0;
      jmp					short exi;

    positive:
      fstp				dword ptr[ esp ];
      mov					ecx, dword ptr[ esp ];
      add					ecx, 7fffffffh;
      sbb					eax, 0;
      mov					edx, dword ptr[ esp + 14h ];
      sbb					edx, 0;
      jmp					short exi;

    int_qnan_or_zero:
      mov					edx, dword ptr[ esp + 14h ];
      test				edx, 7fffffffh;
      jne					not_int_qnan;
      fstp				dword ptr[ esp + 18h ];
      fstp				dword ptr[ esp + 18h ];

    exi:
      leave;
      ret;
    }
  }

  __declspec( naked ) int _ftol2()
  {
    _ftol2_sse();
    __asm
    {
      ret;
    }
  }/**/

  long holdrand = 1L;

#ifndef _DEBUG
  void __cdecl srand( unsigned int seed )
  {
    holdrand = seed;
  }

  int __cdecl rand()
  {
    return( ( ( holdrand = holdrand * 214013L + 2531011L ) >> 16 ) & 0x7fff );
  }
#endif

/*
  static short control_word;
  static short control_word2;
*/

  //inline void SetFloatingPointRoundingToTruncate()
  //{
  //	__asm
  //	{
  //		fstcw   control_word                // store fpu control word
  //			mov     dx, word ptr[control_word]
  //			or      dx, 0x0C00                  // rounding: truncate
  //			mov     control_word2, dx
  //			fldcw   control_word2               // load modfied control word
  //	}
  //}


  void WinMainCRTStartup()
  {
    _initterm( __xc_a, __xc_z );
    //SetFloatingPointRoundingToTruncate();
    WinMain( GetModuleHandle( 0 ), 0, 0, 0 );
    ExitProcess( 0 );
  }

  /*void * __cdecl memset( void *dst, int val, size_t count )
  {
    void *start = dst;
    while ( count-- ) {
      *(char *)dst = (char)val;
      dst = (char *)dst + 1;
    }
    return( start );
  }

  void * __cdecl memcpy( void * dst, const void * src, size_t count )
  {
    void * ret = dst;

    while ( count-- ) {
      *(char *)dst = *(char *)src;
      dst = (char *)dst + 1;
      src = (char *)src + 1;
    }

    return( ret );
  }*/

  //float _CIsin(float f) { return sin(f); }

  //void __fastcall mmemcpy(void * to, void * from, int count) 
  //{
  //	memcpy(to,from,count);
  //}

  //void __fastcall mmemset(void * to, unsigned char value, int count) 
  //{
  //	memset(to,value,count);
  //}

}

#include "libcminimal.h"

#ifdef DEBUGINTOFILE

static void printchar( char **str, int c )
{
  //extern int putchar(int c);
  if ( str ) {
    **str = c;
    ++( *str );
  }
  //else (void)putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints( char **out, const char *string, int width, int pad )
{
  register int pc = 0, padchar = ' ';

  if ( width > 0 ) {
    register int len = 0;
    register const char *ptr;
    for ( ptr = string; *ptr; ++ptr ) ++len;
    if ( len >= width ) width = 0;
    else width -= len;
    if ( pad & PAD_ZERO ) padchar = '0';
  }
  if ( !( pad & PAD_RIGHT ) ) {
    for ( ; width > 0; --width ) {
      printchar( out, padchar );
      ++pc;
    }
  }
  for ( ; *string; ++string ) {
    printchar( out, *string );
    ++pc;
  }
  for ( ; width > 0; --width ) {
    printchar( out, padchar );
    ++pc;
  }

  return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi( char **out, int i, int b, int sg, int width, int pad, int letbase )
{
  char print_buf[ PRINT_BUF_LEN ];
  register char *s;
  register int t, neg = 0, pc = 0;
  register unsigned int u = i;

  if ( i == 0 ) {
    print_buf[ 0 ] = '0';
    print_buf[ 1 ] = '\0';
    return prints( out, print_buf, width, pad );
  }

  if ( sg && b == 10 && i < 0 ) {
    neg = 1;
    u = -i;
  }

  s = print_buf + PRINT_BUF_LEN - 1;
  *s = '\0';

  while ( u ) {
    t = u % b;
    if ( t >= 10 )
      t += letbase - '0' - 10;
    *--s = t + '0';
    u /= b;
  }

  if ( neg ) {
    if ( width && ( pad & PAD_ZERO ) ) {
      printchar( out, '-' );
      ++pc;
      --width;
    }
    else {
      *--s = '-';
    }
  }

  return pc + prints( out, s, width, pad );
}

static int print( char **out, int *varg )
{
  register int width, pad;
  register int pc = 0;
  register char *format = (char *)( *varg++ );
  char scr[ 2 ];

  for ( ; *format != 0; ++format ) {
    if ( *format == '%' ) {
      ++format;
      width = pad = 0;
      if ( *format == '\0' ) break;
      if ( *format == '%' ) goto out;
      if ( *format == '-' ) {
        ++format;
        pad = PAD_RIGHT;
      }
      while ( *format == '0' ) {
        ++format;
        pad |= PAD_ZERO;
      }
      for ( ; *format >= '0' && *format <= '9'; ++format ) {
        width *= 10;
        width += *format - '0';
      }
      if ( *format == 's' ) {
        register char *s = *( (char **)varg++ );
        pc += prints( out, s ? s : "(null)", width, pad );
        continue;
      }
      if ( *format == 'd' ) {
        pc += printi( out, *varg++, 10, 1, width, pad, 'a' );
        continue;
      }
      if ( *format == 'x' ) {
        pc += printi( out, *varg++, 16, 0, width, pad, 'a' );
        continue;
      }
      if ( *format == 'X' ) {
        pc += printi( out, *varg++, 16, 0, width, pad, 'A' );
        continue;
      }
      if ( *format == 'u' ) {
        pc += printi( out, *varg++, 10, 0, width, pad, 'a' );
        continue;
      }
      if ( *format == 'c' ) {
        /* char are converted to int then pushed on the stack */
        scr[ 0 ] = *varg++;
        scr[ 1 ] = '\0';
        pc += prints( out, scr, width, pad );
        continue;
      }
    }
    else {
    out:
      printchar( out, *format );
      ++pc;
    }
  }
  if ( out ) **out = '\0';
  return pc;
}

/* assuming sizeof(void *) == sizeof(int) */

int printf( const char *format, ... )
{
  register int *varg = (int *)( &format );
  return print( 0, varg );
}

int sprintf2( char *out, const char *format, ... )
{
  register int *varg = (int *)( &format );
  return print( &out, varg );
}

#endif
