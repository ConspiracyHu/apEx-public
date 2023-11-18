#include "BasePCH.h"
#include "ModelMaterial.h"
#define WINDOWNAME _T("Model Material")
#define WINDOWXML _T("ModelMaterial")
#include "../Phoenix_Tool/apxProject.h"
#include "apExRoot.h"

extern CapexRoot *Root;

void CapexModelMaterial::LoadCSS()
{
  CString xmlname = Root->GetCSSPath() + WINDOWXML + _T( ".css" );
  CStreamReaderMemory f;
  if ( !f.Open( xmlname.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading CSS: file '%s' not found", xmlname.GetPointer() );
    return;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );
  StyleManager.Reset();
  StyleManager.ParseStyleData( s );
}


CapexModelMaterial::CapexModelMaterial() : CapexWindow()
{
  LoadCSS();
}

CapexModelMaterial::CapexModelMaterial( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  LoadCSS();
}

void CapexModelMaterial::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}


CapexModelMaterial::~CapexModelMaterial()
{

}

void CapexModelMaterial::UpdateData()
{
  CWBButton *b = (CWBButton*)FindChildByID( _T( "selectmaterial" ), _T( "button" ) );
  if ( b )
  {
    if ( !WorkBench->GetEditedModelObject() || WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone )
    {
      b->SetText( _T( "" ) );
      b->Enable( false );
    }
    else
    {
      b->Enable( true );
      CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelObject();
      if ( m->Material ) b->SetText( m->Material->Name );
      else b->SetText( _T( "Select Material" ) );
    }
  }

  //build material param ui
  ParamUIMap.Flush();

  CWBBox *parambox = (CWBBox*)FindChildByID( _T( "materialparams" ), _T( "box" ) );
  if ( !parambox ) return;
  parambox->DeleteChildren();

  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return;
  CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh *)WorkBench->GetEditedModelObject();
  if ( !m->Material ) return;
  CphxMaterial_Tool *mat = m->Material;

  TextureLocatorMap.Flush();

  for ( TS32 x = 0; x < mat->Techniques.NumItems(); x++ )
  {
    CphxMaterialTechnique_Tool *t = mat->Techniques[ x ];
    for ( TS32 y = 0; y < t->TechParameters.Parameters.NumItems(); y++ )
      AddParameterUI( parambox, t->TechParameters.Parameters[ y ] );
    for ( TS32 y = 0; y < t->Passes.NumItems(); y++ )
      for ( TS32 z = 0; z < t->Passes[ y ]->PassParameters.Parameters.NumItems(); z++ )
        AddParameterUI( parambox, t->Passes[ y ]->PassParameters.Parameters[ z ] );
  }

  StyleManager.ApplyStyles( this );

  CWBMessage me;
  BuildPositionMessage( GetPosition(), me );
  me.Resized = true;
  MessageProc( me );
}

TBOOL CapexModelMaterial::MessageProc( CWBMessage &Message )
{
  CphxModelObject_Tool_Mesh *editedmesh = (CphxModelObject_Tool_Mesh *)WorkBench->GetEditedModelObject();

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "selectmaterial" ) )
    {
      if ( !WorkBench->GetEditedModelObject() ) return true;
      if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->ClientToScreen( b->GetClientRect().BottomLeft() ) );

      c->AddItem( _T( "Remove Material" ), -1 );
      c->AddSeparator();

      for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
      {
        TBOOL ParticleMat = false;

        CphxMaterial_Tool *m = Project.GetMaterialByIndex( x );
        for ( TS32 y = 0; y < m->Techniques.NumItems(); y++ )
          if ( m->Techniques[ y ]->Tech->Type == TECH_PARTICLE ) ParticleMat = true;

        if ( !ParticleMat ) //particle materials omitted
          c->AddItem( Project.GetMaterialByIndex( x )->Name.GetPointer(), x );
      }

      return true;
    }

    if ( b->GetID() == _T( "textureselector" ) )
    {
      if ( !WorkBench->GetEditedModelObject() ) return true;
      if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;
      BuildTextureSelectorUI( b );
      return true;
    }

    if ( b->GetID() == _T( "texturelocator" ) )
    {
      if ( TextureLocatorMap.HasKey( b->GetGuid() ) )
      {
        CphxGUID &g = editedmesh->MaterialData.MaterialTextures[ TextureLocatorMap[ b->GetGuid() ] ];
        CphxTextureOperator_Tool *o = Project.GetTexgenOp( g );
        if ( o )
          Root->GoToTexture( o, !App->GetCtrlState() );
      }
      return true;
    }

    if ( b->GetID() == _T( "wireframe" ) )
    {
      if ( !WorkBench->GetEditedModelObject() ) return true;
      if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe = !editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe;
      b->Push( editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Wireframe );
    }

    if ( b->GetID() == _T( "zenable" ) )
    {
      if ( !WorkBench->GetEditedModelObject() ) return true;
      if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode;
      TBOOL DepthEnable = !( bm & 1 );
      TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
      DepthEnable = !DepthEnable;
      bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode = bm;
      b->Push( DepthEnable );
    }

    if ( b->GetID() == _T( "zwrite" ) )
    {
      if ( !WorkBench->GetEditedModelObject() ) return true;
      if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode;
      TBOOL DepthEnable = !( bm & 1 );
      TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
      ZWriteEnable = !ZWriteEnable;
      bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].ZMode = bm;
      b->Push( ZWriteEnable );
    }

  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *b = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "srcblend" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode;
      bm = b->GetCursorPosition() | ( bm & ~15 );
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode = bm;
      return true;
    }

    if ( b->GetID() == _T( "dstblend" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      TU8 bm = editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode;
      bm = ( b->GetCursorPosition() << 4 ) | ( bm & 15 );
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].BlendMode = bm;
      return true;
    }

    if ( b->GetID() == _T( "culllist" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].CullMode = (D3D11_CULL_MODE)( b->GetCursorPosition() + 1 );
      return true;
    }

    if ( b->GetID() == _T( "zfunctlist" ) )
    {
      CphxGUID g = TextureIDMap[ Message.Data ];
      CphxMaterialParameter_Tool *p = GetTargetParam( b );
      if ( !p ) return true;
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].ZFunction = (D3D11_COMPARISON_FUNC)( b->GetCursorPosition() + 1 );
      return true;
    }

    break;
  }

  case WBM_CONTEXTMESSAGE:
  {
    if ( !WorkBench->GetEditedModelObject() ) return true;
    if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "selectmaterial" ) )
    {
      CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelObject();
      CphxMaterial_Tool *mat = Message.Data >= 0 ? Project.GetMaterialByIndex( Message.Data ) : DefaultMaterial;

      if ( mat == m->GetMaterial() ) break;

      m->SetMaterial( mat );
      if ( m->Material ) b->SetText( m->Material->Name );
      UpdateData();
      return true;
    }

    if ( b->GetID() == _T( "textureselector" ) )
    {
      if ( TextureIDMap.HasKey( Message.Data ) )
      {
        CphxGUID g = TextureIDMap[ Message.Data ];
        CphxMaterialParameter_Tool *p = GetTargetParam( b );
        if ( !p ) return true;
        editedmesh->MaterialData.MaterialTextures[ p->GetGUID() ] = g;

        if ( Project.GetTexgenOp( g ) )
        {
          b->SetText( Project.GetTexgenOp( g )->GetName().GetPointer() );
          return true;
        }
        if ( Project.GetRenderTarget( g ) )
        {
          b->SetText( Project.GetRenderTarget( g )->Name.GetPointer() );
          return true;
        }
        b->SetText( _T( "Select Texture" ) );

      }
      return true;
    }

    return true;
    break;
  }

  case WBM_VALUECHANGED:
  {
    if ( !WorkBench->GetEditedModelObject() ) return true;
    if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return true;
    CWBTrackBar *b = (CWBTrackBar *)App->FindItemByGuid( Message.GetTarget(), _T( "trackbar" ) );
    if ( !b ) break;

    CphxMaterialParameter_Tool *p = GetTargetParam( b );
    if ( !p ) return true;

    if ( b->GetID() == _T( "red" ) )
    {
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 0 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Red: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "green" ) )
    {
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 1 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Green: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "blue" ) )
    {
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 2 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Blue: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "alpha" ) )
    {
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Color[ 3 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s Alpha: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "float" ) )
    {
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].Float = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "%s: %d" ), p->Name.GetPointer(), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "renderpriority" ) )
    {
      editedmesh->MaterialData.MaterialParams[ p->GetGUID() ].RenderPriority = Message.Data;
      b->SetText( CString::Format( _T( "Render Priority: %d" ), Message.Data ) );
      return true;
    }

  }

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexModelMaterial::AddParameterUI( CWBItem *Root, CphxMaterialParameter_Tool *Param )
{
  if ( Param->Parameter.Scope != PARAM_VARIABLE ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return;
  CphxModelObject_Tool_Mesh *editedmesh = (CphxModelObject_Tool_Mesh *)WorkBench->GetEditedModelObject();

  CWBBox *nb = new CWBBox( Root, CRect( 10, 10, 50, 50 ) );
  ParamUIMap[ Param->GetGUID() ] = nb;

  CString templatename;

  switch ( Param->Parameter.Type )
  {
  case PARAM_TEXTURE0:
  case PARAM_TEXTURE1:
  case PARAM_TEXTURE2:
  case PARAM_TEXTURE3:
  case PARAM_TEXTURE4:
  case PARAM_TEXTURE5:
  case PARAM_TEXTURE6:
  case PARAM_TEXTURE7:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "texturetemplate" ) ) );
    CWBButton *b = (CWBButton *)nb->FindChildByID( _T( "textureselector" ), _T( "button" ) );
    if ( b )
    {
      if ( editedmesh->MaterialData.MaterialTextures.HasKey( Param->GetGUID() ) )
      {
        CphxGUID &g = editedmesh->MaterialData.MaterialTextures[ Param->GetGUID() ];
        CphxRenderTarget_Tool *t = Project.GetRenderTarget( g );
        if ( t ) b->SetText( t->Name );
        else
        {
          CphxTextureOperator_Tool *o = Project.GetTexgenOp( g );
          if ( o ) b->SetText( o->GetName() );
          else b->SetText( _T( "Select Texture" ) );
        }
      }
      else b->SetText( _T( "Select Texture" ) );
    }

    b = (CWBButton *)nb->FindChildByID( _T( "texturelocator" ), _T( "button" ) );
    if ( b )
      TextureLocatorMap[ b->GetGuid() ] = Param->GetGUID();
  }
  break;
  case PARAM_BLENDMODE0:
  case PARAM_BLENDMODE1:
  case PARAM_BLENDMODE2:
  case PARAM_BLENDMODE3:
  case PARAM_BLENDMODE4:
  case PARAM_BLENDMODE5:
  case PARAM_BLENDMODE6:
  case PARAM_BLENDMODE7:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "blendtemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].BlendMode = phxSrcBlend_ONE | phxDstBlend_ZERO;

    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *src = (CWBItemSelector*)nb->FindChildByID( _T( "srcblend" ), _T( "itemselector" ) );
    CWBItemSelector *dst = (CWBItemSelector*)nb->FindChildByID( _T( "dstblend" ), _T( "itemselector" ) );

    if ( src )
      src->SelectItemByIndex( v.BlendMode & 15 );
    if ( dst )
      dst->SelectItemByIndex( v.BlendMode >> 4 );
  }
  break;

  case PARAM_COLOR:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "colortemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      for ( TS32 c = 0; c < 4; c++ )
        editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].Color[ c ] = 0.25;

    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByID( _T( "red" ), _T( "trackbar" ) );
    CWBTrackBar *g = (CWBTrackBar*)nb->FindChildByID( _T( "green" ), _T( "trackbar" ) );
    CWBTrackBar *b = (CWBTrackBar*)nb->FindChildByID( _T( "blue" ), _T( "trackbar" ) );
    CWBTrackBar *a = (CWBTrackBar*)nb->FindChildByID( _T( "alpha" ), _T( "trackbar" ) );

    if ( r ) r->SetValue( (TS32)( v.Color[ 0 ] * 255 + 0.5f ) );
    if ( g ) g->SetValue( (TS32)( v.Color[ 1 ] * 255 + 0.5f ) );
    if ( b ) b->SetValue( (TS32)( v.Color[ 2 ] * 255 + 0.5f ) );
    if ( a ) a->SetValue( (TS32)( v.Color[ 3 ] * 255 + 0.5f ) );

    break;
  }

  case PARAM_FLOAT:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "floattemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].Float = 0;

    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByID( _T( "float" ), _T( "trackbar" ) );

    if ( r ) r->SetValue( (TS32)( v.Float * 255 + 0.5f ) );
    break;
  }

  case PARAM_RENDERPRIORITY:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "prioritytemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].RenderPriority = 127;

    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByID( _T( "renderpriority" ), _T( "trackbar" ) );

    if ( r )
      r->SetValue( v.RenderPriority );
    break;
  }

  case PARAM_CULLMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "culltemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].CullMode = D3D11_CULL_NONE;
    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *cull = (CWBItemSelector*)nb->FindChildByID( _T( "culllist" ), _T( "itemselector" ) );

    if ( cull )
      cull->SelectItemByIndex( v.CullMode - 1 );
    break;
  }

  case PARAM_FILLMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "filltemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].Wireframe = false;
    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBButton *wf = (CWBButton*)nb->FindChildByID( _T( "wireframe" ), _T( "button" ) );
    if ( wf )
      wf->Push( v.Wireframe );
    break;
  }

  case PARAM_ZFUNCTION:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "zfuncttemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].ZFunction = D3D11_COMPARISON_LESS;
    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];
    CWBItemSelector *zf = (CWBItemSelector*)nb->FindChildByID( _T( "zfunctlist" ), _T( "itemselector" ) );

    if ( zf )
      zf->SelectItemByIndex( v.ZFunction - 1 );
    break;
  }

  case PARAM_ZMODE:
  {
    App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString( _T( "zmodetemplate" ) ) );
    if ( !editedmesh->MaterialData.MaterialParams.HasKey( Param->GetGUID() ) )
      editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ].ZMode = 0;
    MATERIALVALUE v = editedmesh->MaterialData.MaterialParams[ Param->GetGUID() ];

    CWBButton *zenable = (CWBButton*)nb->FindChildByID( _T( "zenable" ), _T( "button" ) );
    CWBButton *zwrite = (CWBButton*)nb->FindChildByID( _T( "zwrite" ), _T( "button" ) );

    if ( zenable ) zenable->Push( !( v.ZMode & 1 ) );
    if ( zwrite ) zwrite->Push( !( ( v.ZMode >> 1 ) & 1 ) );

    break;
  }

  default:
    break;
  }

  CWBLabel *pn = (CWBLabel*)nb->FindChildByID( _T( "paramname" ), _T( "label" ) );
  if ( pn ) pn->SetText( Param->Name );
}

CphxMaterialParameter_Tool * CapexModelMaterial::GetTargetParam( CWBItem *i )
{
  if ( !WorkBench->GetEditedModelObject() || WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return NULL;
  CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh *)WorkBench->GetEditedModelObject();
  if ( !m->Material ) return NULL;
  CphxMaterial_Tool *mat = m->Material;

  for ( TS32 x = 0; x < ParamUIMap.NumItems(); x++ )
  {
    CDictionary<CphxGUID, CWBItem *>::KDPair *kdp = ParamUIMap.GetKDPair( x );
    if ( i->FindItemInParentTree( kdp->Data ) )
      return mat->GetParameter( kdp->Key );
  }

  return NULL;
}

void CapexModelMaterial::BuildTextureSelectorUI( CWBItem *Button )
{
  CWBContextMenu *c = Button->OpenContextMenu( Button->ClientToScreen( Button->GetClientRect().BottomLeft() ) );
  TextureIDMap.Flush();

  TS32 id = 0;

  c->AddItem( _T( "No Texture" ), id );
  CphxGUID g;
  g.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  TextureIDMap[ id ] = g;
  id++;

  c->AddSeparator();

  if ( Project.GetRenderTargetCount() )
  {
    CWBContextItem *it = c->AddItem( _T( "Rendertargets" ), id++ );

    for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
    {
      it->AddItem( Project.GetRenderTargetByIndex( x )->Name.GetPointer(), id );
      TextureIDMap[ id ] = Project.GetRenderTargetByIndex( x )->GetGUID();
      id++;
    }

    c->AddSeparator();
  }

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    CapexTexGenPage *p = Project.GetTexgenPageByIndex( x );

    TS32 savecnt = 0;
    for ( TS32 y = 0; y < p->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *t = p->GetOp( y );
      if ( t->GetOpType() == TEXGEN_OP_SAVE ) savecnt++;
    }

    if ( savecnt )
    {
      CWBContextItem *it = c->AddItem( p->GetName().GetPointer(), id++ );

      for ( TS32 y = 0; y < p->GetOpCount(); y++ )
      {
        CphxTextureOperator_Tool *t = p->GetOp( y );
        if ( t->GetOpType() == TEXGEN_OP_SAVE )
        {
          it->AddItem( t->GetName().GetPointer(), id );
          TextureIDMap[ id ] = t->GetGUID();
          id++;
        }
      }
    }
  }

  //for (TS32 x = 0; x < Project.GetTexgenPageCount(); x++)
  //{
  //	CapexTexGenPage *p = Project.GetTexgenPageByIndex(x);
  //	for (TS32 y = 0; y < p->GetOpCount(); y++)
  //	{
  //		CphxTextureOperator_Tool *t = p->GetOp(y);
  //		if (t->GetOpType() == TEXGEN_OP_SAVE)
  //		{
  //			c->AddItem(t->GetName().GetPointer(), id);
  //			TextureIDMap[id] = t->GetGUID();
  //			id++;
  //		}
  //	}
  //}
}

