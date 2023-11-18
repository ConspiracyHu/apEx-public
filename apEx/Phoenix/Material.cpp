#include "Material.h"
#include "Scene.h"

//static const D3D11_BLEND BlendModeList[] =
//{
//	D3D11_BLEND_ZERO,
//	D3D11_BLEND_ONE,
//	D3D11_BLEND_SRC_COLOR,
//	D3D11_BLEND_INV_SRC_COLOR,
//	D3D11_BLEND_SRC_ALPHA,
//	D3D11_BLEND_INV_SRC_ALPHA,
//	D3D11_BLEND_DEST_ALPHA,
//	D3D11_BLEND_INV_DEST_ALPHA,
//	D3D11_BLEND_DEST_COLOR,
//	D3D11_BLEND_INV_DEST_COLOR,
//	//(DWORD)D3DBLEND_SRCALPHASAT, //probably not required at all
//	//(DWORD)D3DBLEND_BOTHSRCALPHA, //obsolete according to msdn
//	//(DWORD)D3DBLEND_BOTHINVSRCALPHA, //this can be achieved otherwise
//	//(DWORD)D3DBLEND_BLENDFACTOR, //we don't use blendfactor
//	//(DWORD)D3DBLEND_INVBLENDFACTOR //no we don't
//};

int CphxMaterialParameterBatch::CollectAnimatedData()
{
  int DataPos = 0;
  for ( int x = 0; x < ParamCount; x++ )
  {
    CphxMaterialParameter *p = Parameters[ x ];
    if ( p->Scope != PARAM_ANIMATED ) continue;

    switch ( p->Type )
    {
    case PARAM_FLOAT:
      CollectedData[ DataPos ] = p->Value.Float;
      break;
    case PARAM_COLOR:
      for ( int y = 0; y < 4; y++ )
        CollectedData[ DataPos + y ] = p->Value.Color[ y ];
      break;
    }

    switch ( Parameters[ x ]->Type )
    {
    case PARAM_FLOAT: DataPos += 1; break;
    case PARAM_COLOR: DataPos += 4; break;
    }
  }
  return DataPos;
}

//void CphxMaterialParameterBatch::CreateBuffer(ID3D11Buffer **Target)
//{
//	int size = GetDataSize()*sizeof(float);
//	if (!size)
//	{
//		*Target = NULL;
//		return;
//	}
//
//	D3D11_BUFFER_DESC desc;
//	desc.ByteWidth = size % 16 ? ((size >> 4) + 1) << 4 : size;
//	desc.Usage = D3D11_USAGE_DYNAMIC;
//	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	desc.MiscFlags = 0;
//	desc.StructureByteStride = 0;
//
//	phxDev->CreateBuffer(&desc, NULL, Target);
//}
//
//void CphxMaterialParameterBatch::BuildTechRS(ID3D11Buffer *Target)
//{
//	if (!Target) return;
//
//	CollectData();
//	int Size = GetDataSize();
//
//	D3D11_MAPPED_SUBRESOURCE map;
//	phxContext->Map(Target, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
//	memcpy(map.pData, CollectedData, Size*sizeof(float));
//	phxContext->Unmap(Target, 0);
//}

void CphxMaterialSplineBatch::ApplyToParameters( void *GroupingData )
{
  for ( int x = 0; x < SplineCount; x++ )
    if ( Splines[ x ]->GroupingData == GroupingData || GroupingData == NULL )
      Splines[ x ]->Target->Value = Splines[ x ]->GetValue();
}

void CphxMaterialSplineBatch::CalculateValues( float t )
{
  for ( int x = 0; x < SplineCount; x++ )
    Splines[ x ]->CalculateValue( t );
}

//void CphxMaterialTechnique::BuildTechRS(CphxTechRenderStateBatch *Target)
//{
//	Parameters.BuildTechRS(Target->TechLevelParams);
//	for (int x = 0; x < PassCount; x++)
//		RenderPasses[x]->Parameters.BuildTechRS(Target->PassParams[x]);
//}
//
//CphxTechRenderStateBatch * CphxMaterialTechnique::CreateRSBatch()
//{
//	CphxTechRenderStateBatch *b = new CphxTechRenderStateBatch();
//
//	b->PassParams = new ID3D11Buffer*[PassCount];
//
//	Parameters.CreateBuffer(&b->TechLevelParams);
//	for (int x = 0; x < PassCount; x++)
//		RenderPasses[x]->Parameters.CreateBuffer(&b->PassParams[x]);
//
//	return b;
//}

void CphxMaterialTechnique::CreateRenderDataInstances( CphxMaterialPassConstantState **MaterialState, int &passid, CphxScene *RootScene, ID3D11Buffer *VertexBuffer, ID3D11Buffer *IndexBuffer, ID3D11Buffer *WireBuffer, int VertexCount, int IndexCount, void *ToolData, bool Indexed )
{
  for ( int x = 0; x < PassCount; x++ )
  {
    //todo: flareRenderDataInstance server to get cached instances
    CphxRenderDataInstance *ri = new CphxRenderDataInstance();

    ri->VertexBuffer = VertexBuffer;
    ri->IndexBuffer = IndexBuffer;
    ri->WireBuffer = WireBuffer;

    ri->TriIndexCount = VertexCount;
    ri->WireIndexCount = IndexCount;
    ri->ToolData = ToolData;
    ri->Indexed = Indexed;

    ri->Wireframe = MaterialState[ passid ]->Wireframe;
    ri->RenderPriority = MaterialState[ passid ]->RenderPriority;

    memcpy( &ri->VS, &RenderPasses[ x ]->VS, sizeof( void* ) * 5 );
    //ri->VS = RenderPasses[x]->VS;
    //ri->PS = RenderPasses[x]->PS;
    //ri->GS = RenderPasses[x]->GS;
    //ri->HS = RenderPasses[x]->HS;
    //ri->DS = RenderPasses[x]->DS;


    memcpy( &ri->BlendState, &MaterialState[ passid ]->BlendState, sizeof( void* ) * 11 );
    //ri->BlendState = MaterialState[passid]->BlendState;
    //ri->RasterizerState = MaterialState[passid]->RasterizerState;
    //ri->DepthStencilState = MaterialState[passid]->DepthStencilState;
    //for (int y = 0; y < 8; y++)
    //	ri->Textures[y] = MaterialState[passid]->Textures[y];

    int constdatasize = MaterialState[ passid ]->ConstantDataSize;
    int dyndatasize = MaterialState[ passid ]->AnimatedDataSize;

    memcpy( ri->MaterialData, MaterialState[ passid ]->ConstantData, constdatasize );
    memcpy( ri->MaterialData + constdatasize / sizeof( float ), MaterialState[ passid ]->AnimatedData, dyndatasize );

    ri->Matrices[ 0 ] = phxWorldMatrix;
    ri->Matrices[ 1 ] = phxITWorldMatrix;


    //add instance to render queue
    RootScene->AddRenderDataInstance( TargetLayer, ri );

    passid++;
  }
}

void CphxMaterialTechnique::CollectAnimatedData( CphxMaterialPassConstantState *State, int Pass )
{
  CphxMaterialParameterBatch &techbatch = Parameters;
  int datasize = techbatch.CollectAnimatedData();

  CphxMaterialParameterBatch &passbatch = RenderPasses[ Pass ]->Parameters;
  int passdatasize = passbatch.CollectAnimatedData();

  memcpy( State->AnimatedData, techbatch.CollectedData, datasize * sizeof( float ) );
  memcpy( State->AnimatedData + datasize, passbatch.CollectedData, passdatasize * sizeof( float ) );
  State->AnimatedDataSize = ( datasize + passdatasize ) * sizeof( float );
}

#include "phxSpline.h"

//MATERIALVALUE CphxMaterialParameterValue::GetValue(MATERIALPARAMSCOPE Scope, float t)
//{
//	if (Scope != PARAM_VARIABLE)
//		for (int x = 0; x < 4; x++)
//		{
//			Spline[x]->CalculateValue(t);
//			Value.Color[x] = Spline[x]->Value[0];
//		}
//	return Value;
//}

#include "Model.h"
#include "Scene.h"

void CphxMaterial::CreateRenderDataInstances( CphxModelObject_Mesh *Model, CphxScene *RootScene, void *ToolData )
{
  int passid = 0;
  for ( int x = 0; x < TechCount; x++ )
    Techniques[ x ]->CreateRenderDataInstances( Model->MaterialState, passid, RootScene, Model->VxBuffer, Model->IndexBuffer, Model->WireBuffer, Model->TriCount * 3, Model->EdgeCount * 2, ToolData, true );
}

void CphxMaterial::CreateRenderDataInstances( CphxMaterialPassConstantState **State, CphxScene *RootScene, ID3D11Buffer *VxBuffer, int VertexCount )
{
  int passid = 0;
  for ( int x = 0; x < TechCount; x++ )
    Techniques[ x ]->CreateRenderDataInstances( State, passid, RootScene, VxBuffer, NULL, NULL, VertexCount, 0, NULL, false );
}

MATERIALVALUE CphxMaterialSpline::GetValue()
{
  MATERIALVALUE v;
  for ( int x = 0; x < 4; x++ )
    v.Color[ x ] = Splines[ x ]->Value[ 0 ];
  return v;
}

void CphxMaterialSpline::CalculateValue( float t )
{
  for ( int x = 0; x < 4; x++ )
    Splines[ x ]->CalculateValue( t );
}
