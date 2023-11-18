#include "BasePCH.h"
#include "RenderLayer.h"
#include "apxProject.h"

CphxRenderLayerDescriptor_Tool::CphxRenderLayerDescriptor_Tool()
{
  Name = _T( "New RenderLayer" );
  RenderLayer.TargetCount = 0;
  RenderLayer.Targets = NULL;
  RenderLayer.OmitDepthBuffer = false;
  RenderLayer.VoxelizerLayer = false;
  RenderLayer.IgnoreHelperObjects = false;
  RenderLayer.clearRenderTargets = true;
  External = false;
}

CphxRenderLayerDescriptor_Tool::~CphxRenderLayerDescriptor_Tool()
{
  SAFEDELETEA( RenderLayer.Targets );
}

void CphxRenderLayerDescriptor_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "OmitDepthBuffer" ), false ).SetInt( RenderLayer.OmitDepthBuffer );
  Node->AddChild( _T( "ClearRenderTargets" ), false ).SetInt( RenderLayer.clearRenderTargets );
  Node->AddChild( _T( "Voxelizer" ), false ).SetInt( RenderLayer.VoxelizerLayer );
  Node->AddChild( _T( "IgnoreHelperObjects" ), false ).SetInt( RenderLayer.IgnoreHelperObjects );
  Node->AddChild( _T( "Pickable" ), false ).SetInt( HasPicking );
  for ( TS32 x = 0; x < RenderTargets.NumItems(); x++ )
    Node->AddChild( _T( "RenderTarget" ), false ).SetText( RenderTargets[ x ]->GetGUID().GetString() );
}

void CphxRenderLayerDescriptor_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  if ( Node->GetChildCount( _T( "OmitDepthBuffer" ) ) ) Node->GetChild( _T( "OmitDepthBuffer" ) ).GetValue( (TBOOL&)RenderLayer.OmitDepthBuffer );
  if ( Node->GetChildCount( _T( "ClearRenderTargets" ) ) ) Node->GetChild( _T( "ClearRenderTargets" ) ).GetValue( (TBOOL&)RenderLayer.clearRenderTargets );
  if ( Node->GetChildCount( _T( "Voxelizer" ) ) ) Node->GetChild( _T( "Voxelizer" ) ).GetValue( (TBOOL&)RenderLayer.VoxelizerLayer );
  if ( Node->GetChildCount( _T( "IgnoreHelperObjects" ) ) ) Node->GetChild( _T( "IgnoreHelperObjects" ) ).GetValue( (TBOOL&)RenderLayer.IgnoreHelperObjects );
  if ( Node->GetChildCount( _T( "Pickable" ) ) ) Node->GetChild( _T( "Pickable" ) ).GetValue( HasPicking );
  for ( TS32 x = 0; x < Node->GetChildCount( _T( "RenderTarget" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "RenderTarget" ), x );
    CphxGUID guid;
    guid.SetString( n.GetText().GetPointer() );
    CphxRenderTarget_Tool *t = Project.GetRenderTarget( guid );
    if ( t )
      AddRenderTarget( t );
  }
}

void CphxRenderLayerDescriptor_Tool::AddRenderTarget( CphxRenderTarget_Tool *rt )
{
  if ( RenderTargets.Find( rt ) >= 0 ) return;
  RenderTargets += rt;
  AddParent( rt );

  CphxRenderTarget **tl = RenderLayer.Targets;
  RenderLayer.TargetCount++;
  RenderLayer.Targets = new CphxRenderTarget*[ RenderLayer.TargetCount ];
  for ( TS32 x = 0; x < RenderLayer.TargetCount - 1; x++ )
    RenderLayer.Targets[ x ] = tl[ x ];
  RenderLayer.Targets[ RenderLayer.TargetCount - 1 ] = &rt->rt;
  SAFEDELETEA( tl );
}

