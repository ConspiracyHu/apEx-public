#include "BasePCH.h"
#include "KasparovImport.h"
#include "..\Phoenix_Tool\apxProject.h"

int __cdecl decompressPakFile( unsigned __int8 *compressedData, unsigned __int8 *output, int compressedSize )
{
  unsigned __int8 *endPtr; // ecx
  unsigned __int8 *i; // edx
  unsigned __int8 *outputPtr; // esi
  bool v6; // cf
  int v7; // eax
  unsigned __int8 *v8; // esi
  unsigned __int8 *v9; // eax
  int v10; // edi
  int v11; // ebx
  unsigned __int8 *v12; // edx
  int v13; // ebx
  unsigned __int8 *v14; // edx
  int v15; // ebp
  unsigned __int8 *v16; // eax
  unsigned int v17; // ebx
  bool v18; // zf
  bool v19; // sf
  unsigned __int8 *v20; // edi
  bool v21; // zf
  bool v22; // sf
  int v23; // eax
  int v24; // eax
  unsigned __int8 *v25; // edx
  unsigned __int8 *v27; // [esp+14h] [ebp+4h]

  endPtr = &compressedData[ compressedSize ];
  i = compressedData + 8;
  outputPtr = output;
  v6 = compressedData + 8 < &compressedData[ compressedSize ];
  v27 = &compressedData[ compressedSize ];
  if ( !v6 )
    return -1;
  while ( 1 )
  {
    v7 = *i++;
    if ( v7 < 0x80 )
      break;
    if ( v7 & 0x40 )
    {
      *outputPtr = *i;
      v8 = outputPtr + 1;
      *v8 = i[ 1 ];
      outputPtr = v8 + 1;
      i += 2;
    }
    v9 = &outputPtr[ -2 * ( v7 & 0x3F ) - 2 ];
    outputPtr += 2;
    *( outputPtr - 2 ) = *v9;
    *( outputPtr - 1 ) = v9[ 1 ];
  LABEL_27:
    if ( i >= endPtr )
      return -1;
  }
  if ( v7 >= 64 )
  {
    v10 = ( v7 & 0xF ) + 2;
    if ( v10 == 2 )
    {
      v11 = *i;
      v12 = i + 1;
      v10 = ( *v12 << 8 ) + v11;
      i = v12 + 1;
    }
    v13 = *i;
    v14 = i + 1;
    v15 = ( *v14 << 8 ) + v13;
    for ( i = v14 + 1; v7 & 0x30; ++i )
    {
      v7 -= 16;
      *outputPtr++ = *i;
    }
    v16 = &outputPtr[ -v10 - v15 + 1 ];
    if ( v10 > 0 )
    {
      do
      {
        *outputPtr++ = *v16++;
        --v10;
      } while ( v10 );
    }
    goto LABEL_27;
  }
  if ( v7 >= 32 )
  {
    v17 = v7 - 32;
    v18 = v7 == 32;
    v19 = v7 - 32 < 0;
    if ( v7 == 32 )
    {
      *( (unsigned char*)&v17 ) = *i;
      //LOBYTE( v17 ) = *i;
      v17 += 32;
      ++i;
      v18 = v17 == 0;
      v19 = ( v17 & 0x80000000 ) != 0;
    }
    if ( !v19 && !v18 )
    {
      memset( outputPtr, 0, 4 * ( v17 >> 2 ) );
      v20 = &outputPtr[ 4 * ( v17 >> 2 ) ];
      outputPtr += v17;
      memset( v20, 0, v17 & 3 );
      endPtr = v27;
    }
    goto LABEL_27;
  }
  v21 = v7 == 0;
  v22 = v7 < 0;
  if ( v7 )
    goto LABEL_25;
  v23 = *i++;
  if ( v23 )
  {
    v7 = v23 + 31;
    goto LABEL_24;
  }
  v24 = *i;
  v25 = i + 1;
  v7 = ( *v25 << 8 ) + v24;
  i = v25 + 1;
  if ( v7 )
  {
  LABEL_24:
    v21 = v7 == 0;
    v22 = v7 < 0;
  LABEL_25:
    if ( !v22 && !v21 )
    {
      do
      {
        *outputPtr++ = *i++;
        --v7;
      } while ( v7 );
    }
    goto LABEL_27;
  }
  return outputPtr - output;
}

struct DataEntry
{
  int compressedSize;
  int fileOffsetSHR11;
  int storedBlockSize;
  int uncompressedSize;
  char fileName[ 32 ];
};

struct KasparovDataFile
{
  unsigned char* unpackedData = nullptr;
  int unpackedDataSize = 0;

  virtual ~KasparovDataFile()
  {
    SAFEDELETEA( unpackedData );
  }
};

unsigned int textureAliasArray[] =
{
0x0E2000000, 0x0E100000C, 0x0E2000001, 0x0E1000002, 0x0E2000002,
0x0E1000005, 0x0E2000003, 0x0E1000003, 0x0E2000004, 0x0E1000004,
0x0E2000005, 0x0E1000001, 0x0E2000006, 0x0E1000000, 0x0E2000007,
0x0E1000006, 0x0E3000003, 0x0E1000002, 0x0E3000006, 0x0E1000006,
0x0E3000007, 0x0E1000003, 0x0E3000008, 0x0E1000010, 0x0E3000009,
0x0E1000011, 0x0E300000A, 0x0E1000004, 0x0E300000B, 0x0E1000005,
0x0E300000C, 0x0E100000C, 0x0E300000D, 0x0E1000001, 0x0E300000E,
0x0E1000000, 0x0E4000000, 0x0E3000000, 0x0E4000002, 0x0E1000006,
0x0E4000004, 0x0E1000011, 0x0E5000000, 0x0E1000003, 0x0E5000001,
0x0E3000000, 0x0E5000002, 0x0E4000001, 0x0E5000003, 0x0E1000011,
0x0E5000004, 0x0E1000006, 0x0E6000000, 0x0E1000011, 0x0E6000001,
0x0E3000004, 0x0E6000002, 0x0E3000000, 0x0E7000003, 0x0E3000000,
0x0E7000004, 0x0E1000006, 0x0E7000005, 0x0E1000001, 0x0E7000006,
0x0E1000011, 0x0E7000007, 0x0E1000003, 0x0E7000008, 0x0E1000010,
0x0E8000000, 0x0E1000011, 0x0E8000001, 0x0E1000010, 0x0E8000002,
0x0E3000000, 0x0E8000007, 0x0E5000005, 0x0E9000003, 0x0E3000000,
0x0E9000004, 0x0E8000003, 0x0E9000007, 0x0E8000005, 0x0E9000008,
0x0E8000006, 0x0E9000009, 0x0E8000004, 0x0E900000A, 0x0E1000011,
0x0E900000D, 0x0E1000005, 0x0E900000F, 0x0E1000006, 0x0EA000000,
0x0E1000005, 0x0EA000003, 0x0E1000011, 0x0EA000004, 0x0E100000B,
0x0EA000005, 0x0E1000004, 0x0EA000006, 0x0E1000008, 0x0EA000007,
0x0E1000009, 0x0EA000008, 0x0E1000007, 0x0EA000009, 0x0E100000A,
0x0EF000000, 0x0E3000000, 0x0EF000001, 0x0E1000001, 0x0EF000002,
0x0E100000E, 0x0EF000003, 0x0E1000011, 0x0EF000004, 0x0E4000001,
0x0EF000006, 0x0E3000004, 0x0ED00000A, 0x0E9000000, 0x0ED00000B,
0x0E9000002, 0x0ED00000C, 0x0E9000005, 0x0ED00000D, 0x0E9000006,
0x0ED00000E, 0x0E8000005, 0x0ED00000F, 0x0E8000003, 0x0ED000010,
0x0E8000006, 0x0ED000011, 0x0E8000004, 0x0, 0, 0 };

void FindItemInStoredArray( int *a1 )
{
  int *result; // eax
  int *sArray; // ecx
  int v3; // esi

  result = (int *)textureAliasArray;
  if ( textureAliasArray )
  {
    sArray = (int *)&textureAliasArray;
    result = (int *)&textureAliasArray;
    do
    {
      if ( *sArray == *a1 )
        *a1 = result[ 1 ];
      v3 = result[ 2 ];
      result += 2;
      sArray = result;
    } while ( v3 );
  }
}

void ImportWDOObjectToModel( CphxModel_Tool* model, unsigned char* data, CphxMaterial_Tool* mat, CapexTexGenPage* page )
{
  struct Header
  {
    unsigned int size;
    int vertexDataOffset;
    int indexDataOffset;
    int uvStreamOffset;
    int transformDataOffset;
    int animOffset;
    int field18offset;
    int field1coffset;
  };

  Header* head = (Header*)data;

  CArray<CphxVertex> vertices;

  unsigned char* vx = data + head->vertexDataOffset;
  CphxVertex vert;
  vert.Normal = D3DXVECTOR3( 0, 0, 0 );

  while ( 1 )
  {
    unsigned int desc = *(unsigned int*)vx;
    vx += 4;

    if ( desc & 0x01 )
    {
      vert.Position.x = ( (short*)vx )[ 0 ] / 65536.0f*32.0f;
      vert.Position.y = ( (short*)vx )[ 1 ] / 65536.0f*32.0f;
      vert.Position.z = ( (short*)vx )[ 2 ] / 65536.0f*32.0f;
      //LOG_NFO( "vx: %f %f %f", vert.Position.x, vert.Position.y, vert.Position.z );

      vx += 8;
    }

    if ( desc & 0x02 ) // normal
    {
      int nrm = *(unsigned int*)vx;
      vert.Normal.x = float( nrm << 24 >> 23 );
      vert.Normal.y = float( nrm << 16 >> 23 );
      vert.Normal.z = float( nrm << 8 >> 23 );
      //vert.Normal = D3DXVECTOR3( 0, 1, 0 );
      D3DXVec3Normalize( &vert.Normal, &vert.Normal );
      //LOG_NFO( "Norm: %f %f %f", vert.Normal.x, vert.Normal.y, vert.Normal.z );
      vx += 4;
    }

    if ( desc & 0x04 )
    {
      vert.Color[ 0 ] = vx[ 2 ] / 255.0f;
      vert.Color[ 1 ] = vx[ 1 ] / 255.0f;
      vert.Color[ 2 ] = vx[ 0 ] / 255.0f;
      vert.Color[ 3 ] = vx[ 3 ] / 255.0f;
      vx += 4;
    }

    if ( desc & 0x08 ) // uv
    {
      vert.StoredPosition.x = ( ( *(int*)vx ) & 0xffffff ) / 65536.0f;
      vx += 4;
      vert.StoredPosition.y = ( ( *(int*)vx ) & 0xffffff ) / 65536.0f;
      vx += 4;
    }

    if ( desc & 0x10 ) // should not happen
    {
      vx += 8;
    }

    if ( desc & 0x20 )
    {
      vx += 4;
    }

    if ( desc & 0x40 )
    {
      unsigned char *anim = data + head->animOffset;
      int keyCount = ( (int*)anim )[ 1 ];
      int count2 = ( (int*)anim )[ 2 ];

      unsigned char *vxStart = data + head->animOffset + 12 + keyCount * 4;
      vert.Position.x = ( (short*)vxStart )[ vertices.NumItems() * 4 - 4 ] / 65536.0f*32.0f;
      vert.Position.y = ( (short*)vxStart )[ vertices.NumItems() * 4 + 1 - 4 ] / 65536.0f*32.0f;
      vert.Position.z = ( (short*)vxStart )[ vertices.NumItems() * 4 + 2 - 4 ] / 65536.0f*32.0f;

    }

    vertices += vert;

    if ( desc & 0x80000000 )
      break;
  }

  unsigned char* idx = data + head->indexDataOffset;

  CphxGUID texguid;

  while ( 1 )
  {
    CArray<int> indices;

    unsigned short desc = *(unsigned short*)idx; idx += 2;
    unsigned short a = *(unsigned short*)idx; idx += 2;
    unsigned short b = *(unsigned short*)idx; idx += 2;
    unsigned short c = *(unsigned short*)idx; idx += 2;
    unsigned short triCount = *(unsigned short*)idx; idx += 2;

    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
    int v4 = 0;


    if ( a != 0xffff )
    {
      v1 = ( (int*)( data + head->uvStreamOffset + a * 12 ) )[ 0 ];
      v2 = ( (int*)( data + head->uvStreamOffset + a * 12 ) )[ 1 ];

      FindItemInStoredArray( &v2 );

      CString name = CString::Format( "%.8x", v2 );
      name.ToLower();
      //LOG_NFO( "Trying to find texture %s", name.GetPointer() );
      bool found = false;

      for ( int x = 0; x < page->GetOpCount(); x++ )
        if ( page->GetOp( x )->GetName().Find( name ) >= 0 )
        {
          texguid = page->GetOp( x )->GetGUID();
          //LOG_NFO( "Texture located: %.8x vs %s", v2, page->GetOp( x )->GetName().GetPointer() );
          found = true;
        }

      if ( !found )
        LOG_NFO( "Unable to locate texture %s (%x %x)", name.GetPointer(), v1, v2 );

    }

    if ( b != 0xffff )
    {
      v3 = ( (int*)( data + head->uvStreamOffset + b * 12 ) )[ 0 ];
      v4 = ( (int*)( data + head->uvStreamOffset + b * 12 ) )[ 1 ];

      FindItemInStoredArray( &v4 );

      CString name = CString::Format( "%.8x", v4 );
      name.ToLower();
      //LOG_NFO( "Trying to find texture %s", name.GetPointer() );
      bool found = false;

      for ( int x = 0; x < page->GetOpCount(); x++ )
        if ( page->GetOp( x )->GetName().Find( name ) >= 0 )
        {
          texguid = page->GetOp( x )->GetGUID();
          //LOG_NFO( "Texture located: %.8x vs %s", v4, page->GetOp( x )->GetName().GetPointer() );
          found = true;
        }

      if ( !found )
        LOG_NFO( "Unable to locate texture #2 %s (%x %x)", name.GetPointer(), v3, v4 );

    }

    //LOG_NFO( "index data: %x %x %x %x", v1, v2, v3, v4 );

    for ( int x = 0; x < triCount * 3; x++ )
    {
      indices += *(unsigned short*)idx;
      idx += 2;
    }


    CphxModelObject_Tool_Mesh *mesh = (CphxModelObject_Tool_Mesh*)model->AddPrimitive( Mesh_Stored );

    mesh->GetModelObject()->StoredVertexCount = vertices.NumItems();
    mesh->GetModelObject()->StoredVertices = new CphxVertex[ vertices.NumItems() ];
    memcpy( mesh->GetModelObject()->StoredVertices, vertices.GetPointer( 0 ), sizeof( CphxVertex ) * vertices.NumItems() );

    mesh->GetModelObject()->StoredPolyCount = indices.NumItems() / 3;
    mesh->GetModelObject()->StoredPolygons = new CphxPolygon[ indices.NumItems() / 3 ];

    for ( int x = 0; x < indices.NumItems() / 3; x++ )
    {
      mesh->GetModelObject()->StoredPolygons[ x ].VertexCount = 3;
      mesh->GetModelObject()->StoredPolygons[ x ].VertexIDs[ 0 ] = indices[ x * 3 ] + 1;
      mesh->GetModelObject()->StoredPolygons[ x ].VertexIDs[ 2 ] = indices[ x * 3 + 1 ] + 1;
      mesh->GetModelObject()->StoredPolygons[ x ].VertexIDs[ 1 ] = indices[ x * 3 + 2 ] + 1;

      mesh->GetModelObject()->StoredPolygons[ x ].Normals[ 0 ] = vertices[ indices[ x * 3 ] + 1 ].Normal;
      mesh->GetModelObject()->StoredPolygons[ x ].Normals[ 2 ] = vertices[ indices[ x * 3 + 1 ] + 1 ].Normal;
      mesh->GetModelObject()->StoredPolygons[ x ].Normals[ 1 ] = vertices[ indices[ x * 3 + 2 ] + 1 ].Normal;

      mesh->GetModelObject()->StoredPolygons[ x ].Texcoords[ 0 ][ 0 ] = D3DXVECTOR2( vertices[ indices[ x * 3 ] + 1 ].StoredPosition );
      mesh->GetModelObject()->StoredPolygons[ x ].Texcoords[ 2 ][ 0 ] = D3DXVECTOR2( vertices[ indices[ x * 3 + 1 ] + 1 ].StoredPosition );
      mesh->GetModelObject()->StoredPolygons[ x ].Texcoords[ 1 ][ 0 ] = D3DXVECTOR2( vertices[ indices[ x * 3 + 2 ] + 1 ].StoredPosition );

      //LOG_NFO( "ply: %d %d %d", indices[ x * 3 ], indices[ x * 3 + 1 ], indices[ x * 3 + 2 ] );
    }

    //LOG_NFO( "Stored vertex count: %d poly count: %d", mesh->GetModelObject()->StoredVertexCount, mesh->GetModelObject()->StoredPolyCount );

    if ( mat )
    {
      mesh->SetMaterial( mat );
      mesh->UpdateMaterialState();

      for ( int x = 0; x < mat->Techniques.NumItems(); x++ )
        for ( int y = 0; y < mat->Techniques[ x ]->Passes.NumItems(); y++ )
          for ( int z = 0; z < mat->Techniques[ x ]->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ )
            if ( mat->Techniques[ x ]->Passes[ y ]->PassParameters.Parameters[ z ]->Parameter.Type == PARAM_TEXTURE0 )
              mesh->MaterialData.MaterialTextures[ mat->Techniques[ x ]->Passes[ y ]->PassParameters.Parameters[ z ]->GetGUID() ] = texguid;
    }

    if ( ( desc & 0x8000 ) != 0 )
      break;
  }

}

void ImportKasparovDat( CString &File )
{

  FILE *f = fopen( File.GetPointer(), "rb" );

  if ( !f )
  {
    printf( "Kasparov.dat not found!\n" );
    return;
  }

  DataEntry fileList[ 114 ];
  fread( fileList, 48, 114, f );

  KasparovDataFile files[ 114 ];

  for ( int x = 0; x < 114; x++ )
  {
    LOG_NFO( "File %d - compressed: %d - datasize: %d - offset: %d - FileName: %s\n", x, fileList[ x ].compressedSize != fileList[ x ].uncompressedSize, fileList[ x ].storedBlockSize, fileList[ x ].fileOffsetSHR11 << 11, fileList[ x ].fileName );

    fseek( f, fileList[ x ].fileOffsetSHR11 << 11, SEEK_SET );
    unsigned char *data = new unsigned char[ fileList[ x ].storedBlockSize ];
    fread( data, 1, fileList[ x ].storedBlockSize, f );

    if ( fileList[ x ].compressedSize != fileList[ x ].uncompressedSize )
    {
      unsigned char *unpackedData = new unsigned char[ fileList[ x ].uncompressedSize ];

      if ( decompressPakFile( data, unpackedData, fileList[ x ].compressedSize ) != fileList[ x ].uncompressedSize )
      {
        printf( "DECOMPRESS ERROR!\n" );
      }

      delete[] data;
      data = unpackedData;
    }

    files[ x ].unpackedData = data;
    files[ x ].unpackedDataSize = fileList[ x ].uncompressedSize;
  }

  fclose( f );

  // import textures

  int textureCount = 0;
  CapexTexGenPage *pge = Project.CreateTexgenPage();

  CphxTextureFilter_Tool* image = nullptr;

  for ( int x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    if ( Project.GetTextureFilterByIndex( x )->Name == "Image" )
    {
      image = Project.GetTextureFilterByIndex( x );
      break;
    }
  }

  CphxGUID basetexguid;

  for ( int x = 0; x < 114; x++ )
  {
    if ( strstr( fileList[ x ].fileName, ".BMP" ) ||
         strstr( fileList[ x ].fileName, ".BMA" ) ||
         strstr( fileList[ x ].fileName, ".BMC" ) ||
         strstr( fileList[ x ].fileName, ".JPG" ) )
    {
      int xp = ( textureCount % 10 ) * 6;
      int yp = ( textureCount / 10 ) * 3;

      auto* op = pge->CreateOperator( image );
      op->ImageData = new unsigned char[ files[ x ].unpackedDataSize ];
      memcpy( op->ImageData, files[ x ].unpackedData, files[ x ].unpackedDataSize );
      op->ImageDataSize = files[ x ].unpackedDataSize;
      op->Position = CRect( xp, yp, xp + 5, yp + 1 );

      op = pge->CreateSaveOperator();
      op->Position = CRect( xp, yp + 1, xp + 5, yp + 2 );
      op->Name = fileList[ x ].fileName;
      op->Name.ToLower();
      basetexguid = op->GetGUID();

      textureCount++;
    }
  }

  pge->BuildOperatorConnections();

  // import models

  CphxMaterial_Tool* mat = nullptr;
  for ( int x = 0; x < Project.GetMaterialCount(); x++ )
  {
    if ( Project.GetMaterialByIndex( x )->Name.Find( "textured" ) >= 0 )
    {
      mat = Project.GetMaterialByIndex( x );
      break;
    }
  }

  int cnt = 0;

  for ( int x = 0; x < 114; x++ )
  {
    if ( !strstr( fileList[ x ].fileName, ".WDO" ) )
      continue;

    auto* model = Project.CreateModel();
    model->SetName( CString( fileList[ x ].fileName ) );

    unsigned char* data = files[ x ].unpackedData;

    while ( 1 )
    {
      unsigned int objSize = *(unsigned int*)data;

      ImportWDOObjectToModel( model, data, mat, pge );

      if ( objSize & 0x80000000 )
        break;
      data += objSize;

    }
  }


}