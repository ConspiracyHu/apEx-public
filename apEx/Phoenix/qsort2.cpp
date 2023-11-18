//#include "libct.h"

static void shortsort( char *lo, char *hi, size_t width, int( __fastcall *comp )( const void *, const void * ) );
static void swap( char *p, char *q, size_t width );

#define CUTOFF 8
#define STKSIZ (8*sizeof(void*) - 2)

//ALTERNATIVE, WORKING AND SMALLER BUT SLOWER SORT - saves up to 200 bytes

//int Pivot(char *Instances, int first, int last, size_t width, int(__fastcall *comp)(const void *, const void *))
//{
//	int  p = first;
//	void *pivot=Instances+first*width;
//
//	for (int i = first + 1; i <= last; i++)
//	{
//		if (comp(Instances+i*width,pivot)<0)
//		{
//			p++;
//			swap(Instances + i*width, Instances + p*width, width);
//		}
//	}
//
//	swap(Instances + p*width, Instances + first*width, width);
//	return p;
//}
//
//
//void Sorter(char *Instaces, int first, int last, size_t width, int(__fastcall *comp)(const void *, const void *))
//{
//	if (first < last)
//	{
//		int pivot = Pivot(Instaces, first, last, width, comp);
//		Sorter(Instaces, first, pivot - 1, width, comp);
//		Sorter(Instaces, pivot + 1, last, width, comp);
//	}
//}
//
//void __fastcall qsort2(void *base, size_t num, size_t width, int(__fastcall *comp)(const void *, const void *)) //hacked for size
//{
//	if (num < 2) return;
//	Sorter((char*)base, 0, num - 1, width, comp);
//}

//STOCK IMPLEMENTATION
void __fastcall qsort2( void *base, size_t num, size_t width, int( __fastcall *comp )( const void *, const void * ) )
{
  char *mid;
  char *loguy, *higuy;
  char *lostk[ STKSIZ ], *histk[ STKSIZ ];
  int stkptr = 0;

  if ( num < 2 ) return;

  char *lo = (char*)base;
  char *hi = (char*)base + width*( num - 1 );

recurse:

  size_t size = ( hi - lo ) / width + 1;

  if ( size <= CUTOFF )
  {
    shortsort( lo, hi, width, comp );
  }
  else {
    mid = lo + ( size / 2 )*width;

    if ( comp( lo, mid ) > 0 ) swap( lo, mid, width );
    if ( comp( lo, hi ) > 0 ) swap( lo, hi, width );
    if ( comp( mid, hi ) > 0 ) swap( mid, hi, width );

    loguy = lo;
    higuy = hi;

    for ( ;;)
    {
      if ( mid > loguy )
      {
        do
        {
          loguy += width;
        } while ( loguy < mid && comp( loguy, mid ) <= 0 );
      }
      if ( mid <= loguy )
      {
        do
        {
          loguy += width;
        } while ( loguy <= hi && comp( loguy, mid ) <= 0 );
      }

      do
      {
        higuy -= width;
      } while ( higuy > mid && comp( higuy, mid ) > 0 );

      if ( higuy < loguy ) break;

      swap( loguy, higuy, width );

      if ( mid == higuy ) mid = loguy;
    }

    higuy += width;
    if ( mid < higuy )
    {
      do
      {
        higuy -= width;
      } while ( higuy > mid && comp( higuy, mid ) == 0 );
    }
    if ( mid >= higuy )
    {
      do
      {
        higuy -= width;
      } while ( higuy > lo && comp( higuy, mid ) == 0 );
    }

    if ( higuy - lo >= hi - loguy )
    {
      if ( lo < higuy )
      {
        lostk[ stkptr ] = lo;
        histk[ stkptr ] = higuy;
        ++stkptr;
      }

      if ( loguy < hi )
      {
        lo = loguy;
        goto recurse;
      }
    }
    else
    {
      if ( loguy < hi )
      {
        lostk[ stkptr ] = loguy;
        histk[ stkptr ] = hi;
        ++stkptr;
      }

      if ( lo < higuy )
      {
        hi = higuy;
        goto recurse;
      }
    }
  }

  --stkptr;
  if ( stkptr >= 0 )
  {
    lo = lostk[ stkptr ];
    hi = histk[ stkptr ];
    goto recurse;
  }
  else
    return;
}

void shortsort( char *lo, char *hi, size_t width, int( __fastcall *comp )( const void *, const void * ) )
{
  char *p, *max;
  while ( hi > lo )
  {
    max = lo;
    for ( p = lo + width; p <= hi; p += width )
    {
      if ( comp( p, max ) > 0 )
        max = p;
    }
    swap( max, hi, width );
    hi -= width;
  }
}

void swap( char *a, char *b, size_t width )
{
  char tmp;
  if ( a != b )
    while ( width-- )
    {
      tmp = *a;
      *a++ = *b;
      *b++ = tmp;
    }
}