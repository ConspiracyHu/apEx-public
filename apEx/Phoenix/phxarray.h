#pragma once
#ifdef PHX_MINIMAL_BUILD
#include "PhoenixConfig.h"
#else
#include "PhoenixConfig_Full.h"
#endif

template <typename ItemType> class CphxArray
{
protected:

  int Capacity;

  void Expand( int AddedItemCount )
  {
    //if (!Array)
    //{
    //	Array = new ItemType[AddedItemCount];
    //	Capacity = AddedItemCount;
    //	ItemCount = 0;
    //	return;
    //}

    ItemType *NewArray = new ItemType[ Capacity + AddedItemCount ];
    for ( int x = 0; x < ItemCount; x++ )
      NewArray[ x ] = Array[ x ];

    if ( Array )
      delete[] Array;

    Array = NewArray;
    Capacity += AddedItemCount;
  }

#ifndef PHX_MINIMAL_BUILD
  /*__forceinline*/ int GetCapacity() const
  {
    return Capacity;
  }
#endif

public:
  ItemType *Array;
  int ItemCount;

  typedef int( __cdecl *ARRAYSORTCALLBACK )( ItemType *a, ItemType *b );

  CphxArray()
  {
    Capacity = 0;
    ItemCount = 0;
    Array = 0;
  }

#ifndef PHX_MINIMAL_BUILD
  CphxArray( int Size )
  {
    Array = new ItemType[ Size ];
    Capacity = Size;
    ItemCount = 0;
  }
#endif

  //#ifndef PHX_MINIMAL_BUILD
  CphxArray( const CphxArray<ItemType> &original )
  {
    Capacity = original.GetCapacity();
    ItemCount = original.NumItems();
    Array = new ItemType[ Capacity ];
    for ( int x = 0; x < ItemCount; x++ )
      Array[ x ] = original[ x ];
  }
  //#endif

//#ifndef PHX_MINIMAL_BUILD
  ~CphxArray()
  {
    if ( Array ) delete[] Array;
    Capacity = ItemCount = 0;
    Array = 0;
  }
//#endif

  /*__forceinline */int NumItems() const
  {
    return ItemCount;
  }

  void Flush()
  {
//#ifndef PHX_MINIMAL_BUILD
    if ( Array ) delete[] Array;
    Array = new ItemType[ Capacity ];
//#endif
    ItemCount = 0;
  }

  void FlushFast()
  {
    ItemCount = 0;
  }

  void Add( const ItemType &Item )
  {
    if ( ItemCount == Capacity || !Array ) Expand( Capacity + 1 );
    Array[ ItemCount ] = Item;
    ItemCount++;
  }

  CphxArray<ItemType> &operator= ( const CphxArray<ItemType> &a )
  {
    if ( &a == this ) return *this;
    Flush();
    for ( int i = 0; i < a.NumItems(); i++ )
      Add( a[ i ] );
    return *this;
  }

  CphxArray<ItemType> &operator+= ( const ItemType &i )
  {
    Add( i );
    return *this;
  }

  CphxArray<ItemType> &operator+= ( const CphxArray<ItemType> &i )
  {
    for ( int x = 0; x < i.NumItems(); x++ )
      Add( i[ x ] );
    return *this;
  }

  CphxArray<ItemType> &operator-= ( const ItemType &i )
  {
    Delete( i );
    return *this;
  }

  void AddUnique( const ItemType &Item )
  {
    if ( Find( Item ) != -1 ) return;
    Add( Item );
  }

  ItemType &operator[]( const int idx )
  {
    return Array[ idx ];
  }

  const ItemType &operator[]( const int idx ) const
  {
    return Array[ idx ];
  }

  ItemType &Last()
  {
    return Array[ ItemCount - 1 ];
  }

  int const Find( const ItemType &i ) const
  {
    for ( int x = 0; x < ItemCount; x++ )
      if ( Array[ x ] == i ) return x;
    return -1;
  }

  void DeleteByIndex( const int idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    ItemCount--;
    for ( int x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void Delete( const ItemType &i )
  {
    DeleteByIndex( Find( i ) );
  }

  void FreeByIndex( const int idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    delete Array[ idx ];
    ItemCount--;
    for ( int x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void Free( const ItemType &i )
  {
    FreeByIndex( Find( i ) );
  }

  void FreeAByIndex( const int idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    delete[] Array[ idx ];
    ItemCount--;
    for ( int x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void FreeA( const ItemType &i )
  {
    FreeAByIndex( Find( i ) );
  }

  void Swap( const int a, const int b )
  {
    ItemType temp = Array[ a ];
    Array[ a ] = Array[ b ];
    Array[ b ] = temp;
  }

  void Sort( ARRAYSORTCALLBACK SortFunct )
  {
    qsort( Array, ItemCount, sizeof( ItemType ), ( int( _cdecl* )( const void*, const void* ) )SortFunct );
  }

  ItemType *GetPointer( const int idx )
  {
    return &Array[ idx ];
  }

  void FreeArray()
  {
    //#ifndef PHX_MINIMAL_BUILD
    for ( int x = NumItems() - 1; x >= 0; x-- )
      if ( Array[ x ] )
        delete Array[ x ];
    //#endif
    FlushFast();
  }

  //#ifndef PHX_MINIMAL_BUILD
  void FreeArrayA()
  {
    for ( int x = NumItems() - 1; x >= 0; x-- )
      if ( Array[ x ] )
        delete[] Array[ x ];
    FlushFast();
  }

  void TrimHead( int count )
  {
    if ( count < 0 ) return;

    if ( count >= ItemCount )
    {
      FlushFast();
      return;
    }

    for ( int x = 0; x < ItemCount - count; x++ )
      Array[ x ] = Array[ x + count ];
    ItemCount -= count;
  }
  //#endif
};
