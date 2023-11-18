#include "BasePCH.h"
#include "MaterialEditor.h"
#define WINDOWNAME _T("Material Editor")
#define WINDOWXML _T("MaterialEditor")
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"
#include "apExRoot.h"
extern CapexRoot *Root;

CString MaterialTypeNames[] =
{
  _T( "Float" ),
  _T( "Color" ),
  _T( "Z Mode" ),
  _T( "Z Function" ),
  _T( "Fill Mode" ),
  _T( "Cull Mode" ),
  _T( "Render Priority" ),
  _T( "Texture 0" ),
  _T( "Texture 1" ),
  _T( "Texture 2" ),
  _T( "Texture 3" ),
  _T( "Texture 4" ),
  _T( "Texture 5" ),
  _T( "Texture 6" ),
  _T( "Texture 7" ),
  _T( "Blend Mode 0" ),
  _T( "Blend Mode 1" ),
  _T( "Blend Mode 2" ),
  _T( "Blend Mode 3" ),
  _T( "Blend Mode 4" ),
  _T( "Blend Mode 5" ),
  _T( "Blend Mode 6" ),
  _T( "Blend Mode 7" ),
  _T( "Post RenderTarget" ),
  _T( "Particle Float" ),
  _T( "Depth Texture 7" ),
  _T( "3D texture 6" ),
  _T( "Mesh Data 0" ),
  _T( "Mesh Data 1" ),
  _T( "Mesh Data 2" ),
  _T( "Mesh Data 3" ),
  _T( "Mesh Data 4" ),
  _T( "Mesh Data 5" ),
  _T( "Mesh Data 6" ),
  _T( "Mesh Data 7" ),
  _T( "Particle Life" ),
  _T( "LTCData1 Texture 4"),
  _T( "LTCData2 Texture 5")
};

void CapexMaterialEditor::LoadCSS()
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

CapexMaterialEditor::CapexMaterialEditor() : CapexWindow()
{
  LoadCSS();
}

CapexMaterialEditor::CapexMaterialEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  LoadCSS();
}

void CapexMaterialEditor::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}

CapexMaterialEditor::~CapexMaterialEditor()
{

}

void CapexMaterialEditor::UpdateData()
{
  TS32 selectedtech = 0;
  TS32 selectedpass = 0;

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techlist" ), _T( "itemselector" ) );
  if ( l )
  {
    selectedtech = l->GetCursorPosition();
    l->Flush();
    for ( TS32 x = 0; x < Project.GetTechCount(); x++ )
      l->AddItem( Project.GetTechByIndex( x )->Name );
    l->SelectItemByIndex( selectedtech );
  }

  l = (CWBItemSelector*)FindChildByID( _T( "passlist" ), _T( "itemselector" ) );
  if ( l )
  {
    selectedpass = l->GetCursorPosition();
    l->Flush();
  }

  if ( Project.GetTechByIndex( selectedtech ) )
  {
    CphxMaterialTechnique_Tool *t = Project.GetTechByIndex( selectedtech );
    if ( l )
    {
      for ( TS32 x = 0; x < t->Passes.NumItems(); x++ )
        l->AddItem( t->Passes[ x ]->Name );
      l->SelectItemByIndex( selectedpass );
    }
  }

  l = (CWBItemSelector*)FindChildByID( _T( "materiallist" ), _T( "itemselector" ) );
  if ( l )
  {
    TS32 selectedmat = l->GetCursorPosition();
    l->Flush();
    for ( TS32 x = 0; x < Project.GetMaterialCount(); x++ )
      l->AddItem( Project.GetMaterialByIndex( x )->Name );
    l->SelectItemByIndex( selectedmat );
  }

  l = (CWBItemSelector*)FindChildByID( _T( "layerlist" ), _T( "itemselector" ) );
  if ( l )
  {
    selectedtech = l->GetCursorPosition();
    l->Flush();
    for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
      l->AddItem( Project.GetRenderLayerByIndex( x )->Name );
    l->SelectItemByIndex( selectedtech );
  }

}

TBOOL CapexMaterialEditor::MessageProc( CWBMessage &Message )
{
  CphxMaterialTechnique_Tool *tech = GetEditedTech();
  CphxMaterialRenderPass_Tool *pass = GetEditedPass();

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "newtech" ) )
    {
      CphxMaterialTechnique_Tool *t = Project.CreateTech();
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techlist" ), _T( "itemselector" ) );
      if ( l ) l->SelectItem( l->AddItem( t->Name ) );
      return true;
    }

    if ( tech && b->GetID() == _T( "newpass" ) )
    {
      CphxMaterialRenderPass_Tool *t = tech->CreatePass();
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passlist" ), _T( "itemselector" ) );
      if ( l ) l->SelectItem( l->AddItem( t->Name ) );
      return true;
    }

    if ( tech && b->GetID() == _T( "minifiable" ) )
    {
      if ( pass )
      {
        pass->Minifiable = !pass->Minifiable;
        b->Push( pass->Minifiable );
      }
      return true;
    }

    if ( tech && b->GetID() == _T( "newtechparam" ) )
    {
      CphxMaterialParameter_Tool *t = tech->CreateParam();
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techparamlist" ), _T( "itemselector" ) );
      if ( l ) l->SelectItem( l->AddItem( t->Name ) );
      return true;
    }

    if ( tech && b->GetID() == _T( "deletetechparam" ) )
    {
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techparamlist" ), _T( "itemselector" ) );
      if ( l )
      {
        TS32 p = l->GetCursorPosition();
        tech->DeleteParam( p );
        SelectTech();
        l->SelectItemByIndex( p );
      }
      return true;
    }

    if ( tech && b->GetID() == _T( "exporttech" ) )
    {
      ExportTech( tech );
      return true;
    }

    if ( tech && b->GetID() == _T( "copytech" ) )
    {
      CopyTech( tech );
      return true;
    }

    if ( pass && b->GetID() == _T( "newpassparam" ) )
    {
      CphxMaterialParameter_Tool *t = pass->CreateParam();
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passparamlist" ), _T( "itemselector" ) );
      if ( l ) l->SelectItem( l->AddItem( t->Name ) );
      return true;
    }

    //if (tech && pass && b->GetID() == _T("deletepass"))
    //{
    //	TS32 idx=tech->Passes.Find(pass);
    //	if (idx >= 0)
    //	{
    //		tech->Passes.Free(pass);
    //		tech->RebuildDependents();
    //		SelectTech();
    //	}
    //	return true;
    //}

    if ( pass && b->GetID() == _T( "deletepassparam" ) )
    {
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passparamlist" ), _T( "itemselector" ) );
      if ( l )
      {
        TS32 p = l->GetCursorPosition();
        pass->DeleteParam( p );
        SelectPass();
        l->SelectItemByIndex( p );
        if ( tech ) tech->RebuildDependents();
      }
      return true;
    }

    if ( b->GetID() == _T( "techparamtype" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedTechParam();
      if ( !p ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->GetScreenRect().BottomLeft() );
      for ( TS32 x = 0; x < PARAM_COUNT; x++ )
        if ( x != PARAM_RENDERTARGET || GetEditedTech()->Tech->Type != TECH_MATERIAL )
          c->AddItem( MaterialTypeNames[ x ].GetPointer(), x );

      return true;
    }

    if ( b->GetID() == _T( "passparamtype" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedPassParam();
      if ( !p ) return true;

      CWBContextMenu *c = b->OpenContextMenu( b->GetScreenRect().BottomLeft() );
      for ( TS32 x = 0; x < PARAM_COUNT; x++ )
        if ( x != PARAM_RENDERTARGET || GetEditedTech()->Tech->Type != TECH_MATERIAL )
          c->AddItem( MaterialTypeNames[ x ].GetPointer(), x );

      return true;
    }

    if ( b->GetID() == _T( "newmaterial" ) )
    {
      CphxMaterial_Tool *t = Project.CreateMaterial();
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "materiallist" ), _T( "itemselector" ) );
      if ( l ) l->SelectItem( l->AddItem( t->Name ) );
      return true;
    }

    if ( b->GetID() == _T( "exportmaterial" ) )
    {
      ExportMaterial( GetEditedMaterial() );
      return true;
    }

    if ( b->GetID() == _T( "deletematerial" ) )
    {
      return true;
    }

    if ( b->GetID() == _T( "addtech" ) )
    {
      CphxMaterialTechnique_Tool *t = GetEditedTech();
      CphxMaterial_Tool *m = GetEditedMaterial();

      if ( t && m )
      {
        if ( m->Techniques.Find( t ) < 0 )
        {
          m->AddTech( t );
          CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techsinmaterial" ), _T( "itemselector" ) );
          if ( l )
            l->AddItem( t->Name );
        }
      }
      return true;
    }

    if ( b->GetID() == _T( "removetech" ) )
    {
      return true;
    }

    if ( b->GetID() == _T( "textureselector" ) )
    {
      BuildTextureSelectorUI( b );
      return true;
    }

    if ( b->GetID() == _T( "rendertargetselector" ) )
    {
      BuildTextureSelectorUI( b, true );
      return true;
    }


    CphxMaterialParameter_Tool *p = NULL;
    if ( b->FindParentByID( _T( "techparamvalues" ) ) ) p = GetEditedTechParam();
    if ( b->FindParentByID( _T( "passparamvalues" ) ) ) p = GetEditedPassParam();

    if ( p )
    {

      MATERIALVALUE *v = &p->Parameter.Value;
      if ( p->Parameter.Scope != PARAM_CONSTANT )
        v = &p->DefaultValue;

      if ( b->GetID() == _T( "wireframe" ) )
      {
        b->Push( !b->IsPushed() );
        v->Wireframe = b->IsPushed() != 0;
        return true;
      }

      if ( b->GetID() == _T( "zwrite" ) )
      {
        TU8 bm = v->ZMode;
        TBOOL DepthEnable = !( bm & 1 );
        TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
        ZWriteEnable = !ZWriteEnable;
        bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
        v->ZMode = bm;
        b->Push( ZWriteEnable );
        return true;
      }

      if ( b->GetID() == _T( "zenable" ) )
      {
        TU8 bm = v->ZMode;
        TBOOL DepthEnable = !( bm & 1 );
        TBOOL ZWriteEnable = !( ( bm >> 1 ) & 1 );
        DepthEnable = !DepthEnable;
        bm = ( ( !ZWriteEnable ) << 1 ) | ( !DepthEnable );
        v->ZMode = bm;
        b->Push( DepthEnable );
        return true;
      }

    }

  }
  break;
  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *l = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !l ) break;

    if ( l->GetID() == _T( "techlist" ) ) SelectTech();
    if ( l->GetID() == _T( "techparamlist" ) ) SelectTechParam();
    if ( l->GetID() == _T( "passlist" ) ) SelectPass();
    if ( l->GetID() == _T( "passparamlist" ) ) SelectPassParam();
    if ( l->GetID() == _T( "materiallist" ) ) SelectMaterial();

    if ( l->GetID() == _T( "layerlist" ) )
    {
      CphxMaterialTechnique_Tool *t = GetEditedTech();
      if ( !t ) return true;
      t->SetTargetLayer( Project.GetRenderLayerByIndex( l->GetCursorPosition() ) );
      return true;
    }

    if ( l->GetID() == _T( "techtype" ) )
    {
      CphxMaterialTechnique_Tool *t = GetEditedTech();
      if ( t )
      {
        t->Tech->Type = (TECHNIQUETYPE)l->GetCursorPosition();
        Root->UpdateWindowData( apEx_TimelineEditor );
      }
    }

    if ( l->GetID() == _T( "techparamscope" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedTechParam();
      if ( p )
      {
        MATERIALPARAMSCOPE s = p->Parameter.Scope;
        p->Parameter.Scope = (MATERIALPARAMSCOPE)l->GetCursorPosition();
        CphxMaterialTechnique_Tool *t = GetEditedTech();
        if ( p->Parameter.Scope != s && t ) t->InvalidateUptoDateFlag();
      }
      return true;
    }

    if ( l->GetID() == _T( "passparamscope" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedPassParam();
      if ( p )
      {
        MATERIALPARAMSCOPE s = p->Parameter.Scope;
        p->Parameter.Scope = (MATERIALPARAMSCOPE)l->GetCursorPosition();
        CphxMaterialTechnique_Tool *t = GetEditedTech();
        if ( p->Parameter.Scope != s && t ) t->InvalidateUptoDateFlag();
      }
      return true;
    }

    CphxMaterialParameter_Tool *p = NULL;
    if ( l->FindParentByID( _T( "techparamvalues" ) ) ) p = GetEditedTechParam();
    if ( l->FindParentByID( _T( "passparamvalues" ) ) ) p = GetEditedPassParam();

    if ( p )
    {
      TS32 idx = l->GetItemIndex( Message.Data );

      MATERIALVALUE *v = &p->Parameter.Value;
      if ( p->Parameter.Scope != PARAM_CONSTANT )
        v = &p->DefaultValue;

      if ( l->GetID() == _T( "srcblend" ) )
      {
        TU8 bm = v->BlendMode;
        bm = idx | ( bm & ~15 );
        v->BlendMode = bm;
        return true;
      }

      if ( l->GetID() == _T( "dstblend" ) )
      {
        TU8 bm = v->BlendMode;
        bm = ( idx << 4 ) | ( bm & 15 );
        v->BlendMode = bm;
        return true;
      }

      if ( l->GetID() == _T( "cullmode" ) )
      {
        v->CullMode = (D3D11_CULL_MODE)( idx + 1 );
        return true;
      }

      if ( l->GetID() == _T( "zfunct" ) )
      {
        v->ZFunction = (D3D11_COMPARISON_FUNC)( idx + 1 );
        return true;
      }
    }

    return true;
  }

  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *l = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !l ) break;

    if ( l->GetID() == _T( "techlist" ) )
    {
      CphxMaterialTechnique_Tool *t = GetEditedTech();
      if ( t && l->GetCursorItem() ) t->Name = l->GetCursorItem()->GetText();
      return true;
    }

    if ( l->GetID() == _T( "passlist" ) )
    {
      CphxMaterialRenderPass_Tool *t = GetEditedPass();
      if ( t && l->GetCursorItem() ) t->Name = l->GetCursorItem()->GetText();
      return true;
    }

    if ( l->GetID() == _T( "techparamlist" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedTechParam();
      if ( p&&l->GetCursorItem() ) p->Name = l->GetCursorItem()->GetText();
      return true;
    }

    if ( l->GetID() == _T( "passparamlist" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedPassParam();
      if ( p&&l->GetCursorItem() ) p->Name = l->GetCursorItem()->GetText();
      return true;
    }

    if ( l->GetID() == _T( "materiallist" ) )
    {
      CphxMaterial_Tool *p = GetEditedMaterial();
      if ( p&&l->GetCursorItem() ) p->Name = l->GetCursorItem()->GetText();
      return true;
    }
  }
  break;

  case WBM_CONTEXTMESSAGE:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "techparamtype" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedTechParam();
      if ( p )
      {
        p->Parameter.Type = (MATERIALPARAMTYPE)Message.Data;
        GetEditedTech()->Rebuild();
        SelectTechParam();
        return true;
      }
    }

    if ( b->GetID() == _T( "passparamtype" ) )
    {
      CphxMaterialParameter_Tool *p = GetEditedPassParam();
      if ( p )
      {
        p->Parameter.Type = (MATERIALPARAMTYPE)Message.Data;
        GetEditedTech()->Rebuild();
        SelectPassParam();
        return true;
      }
    }

    CphxMaterialParameter_Tool *p = NULL;
    if ( b->FindParentByID( _T( "techparamvalues" ) ) ) p = GetEditedTechParam();
    if ( b->FindParentByID( _T( "passparamvalues" ) ) ) p = GetEditedPassParam();

    if ( p )
    {

      MATERIALVALUE *v = &p->Parameter.Value;
      if ( p->Parameter.Scope != PARAM_CONSTANT )
        v = &p->DefaultValue;

      if ( b->GetID() == _T( "textureselector" ) )
      {
        if ( TextureIDMap.HasKey( Message.Data ) )
        {
          CphxGUID g = TextureIDMap[ Message.Data ];
          p->SetTextureGUID( g );
          if ( GetEditedTech() ) GetEditedTech()->InvalidateUptoDateFlag();

          if ( Project.GetTexgenOp( g ) )
            b->SetText( Project.GetTexgenOp( g )->GetName() );

          if ( Project.GetRenderTarget( g ) )
            b->SetText( Project.GetRenderTarget( g )->Name );

        }
        return true;
      }

      if ( b->GetID() == _T( "rendertargetselector" ) )
      {
        if ( TextureIDMap.HasKey( Message.Data ) )
        {
          CphxGUID g = TextureIDMap[ Message.Data ];
          p->SetTextureGUID( g );
          if ( GetEditedTech() ) GetEditedTech()->InvalidateUptoDateFlag();
          if ( Project.GetRenderTarget( g ) )
            b->SetText( Project.GetRenderTarget( g )->Name );
        }
        return true;
      }
    }

    return true;
  }
  break;
  case WBM_VALUECHANGED:
  {
    CWBTrackBar *b = (CWBTrackBar*)App->FindItemByGuids( Message.GetTarget(), _T( "trackbar" ), _T( "numpad" ) );
    if ( !b ) break;

    CphxMaterialParameter_Tool *p = NULL;
    if ( b->FindParentByID( _T( "techparamvalues" ) ) ) p = GetEditedTechParam();
    if ( b->FindParentByID( _T( "passparamvalues" ) ) ) p = GetEditedPassParam();

    if ( !p ) break;

    MATERIALVALUE *v = &p->Parameter.Value;
    if ( p->Parameter.Scope != PARAM_CONSTANT )
      v = &p->DefaultValue;

    if ( b->GetID() == _T( "redvalue" ) )
    {
      v->Color[ 0 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "Red: %d" ), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "greenvalue" ) )
    {
      v->Color[ 1 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "Green: %d" ), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "bluevalue" ) )
    {
      v->Color[ 2 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "Blue: %d" ), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "alphavalue" ) )
    {
      v->Color[ 3 ] = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "Alpha: %d" ), Message.Data ) );
      return true;
    }

    if ( b->GetID() == _T( "floatvalue" ) )
    {
      v->Float = Message.Data / 255.0f;
      b->SetText( CString::Format( _T( "Value: %f" ), Message.Data / 255.0f ) );
      return true;
    }

    if ( b->GetID() == _T( "renderpriovalue" ) )
    {
      v->RenderPriority = Message.Data;
      b->SetText( CString::Format( _T( "Render Priority: %d" ), Message.Data ) );
      return true;
    }

    break;
  }

  default:
    break;

  }

  return CapexWindow::MessageProc( Message );
}

CphxMaterialTechnique_Tool * CapexMaterialEditor::GetEditedTech()
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techlist" ), _T( "itemselector" ) );

  if ( l )
  {
    TS32 cnt = Project.GetTechCount();
    TS32 p = l->GetCursorPosition();
    if ( p >= 0 && p < cnt )
      return Project.GetTechByIndex( p );
  }

  return NULL;
}

CphxMaterialRenderPass_Tool * CapexMaterialEditor::GetEditedPass()
{
  CphxMaterialTechnique_Tool *tech = GetEditedTech();
  if ( !tech ) return NULL;

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passlist" ), _T( "itemselector" ) );

  if ( l )
  {
    TS32 p = l->GetCursorPosition();
    if ( p >= 0 && p < tech->Passes.NumItems() )
      return tech->Passes[ p ];
  }

  return NULL;
}

void CapexMaterialEditor::SelectTech()
{
  CapexWindow *w = WorkBench->GetWindow( apEx_MaterialShaderEditor );
  if ( w ) w->UpdateData();

  CphxMaterialTechnique_Tool *t = GetEditedTech();

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passlist" ), _T( "itemselector" ) );
  if ( l )
  {
    l->Flush();
    if ( t )
      for ( TS32 x = 0; x < t->Passes.NumItems(); x++ )
        l->AddItem( t->Passes[ x ]->Name );
    l->SelectItemByIndex( 0 );
  }

  if ( t )
  {
    l = (CWBItemSelector*)FindChildByID( _T( "techtype" ), _T( "itemselector" ) );
    if ( l )
      l->SelectItemByIndex( t->Tech->Type );
  }

  if ( t )
  {
    l = (CWBItemSelector*)FindChildByID( _T( "layerlist" ), _T( "itemselector" ) );
    if ( l )
    {
      TBOOL Selected = false;
      for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
        if ( t->TargetLayer == Project.GetRenderLayerByIndex( x )->GetGUID() )
        {
          l->SelectItemByIndex( x );
          Selected = true;
        }
      if ( !Selected ) l->SelectItemByIndex( -1 );
    }
  }

  l = (CWBItemSelector*)FindChildByID( _T( "techparamlist" ), _T( "itemselector" ) );
  if ( l )
  {
    l->Flush();
    l->SelectItem( -1 );
    if ( t )
      for ( TS32 x = 0; x < t->TechParameters.Parameters.NumItems(); x++ )
        l->AddItem( t->TechParameters.Parameters[ x ]->Name );
  }

  SelectPass();
}

void CapexMaterialEditor::SelectPass()
{
  CapexWindow *w = WorkBench->GetWindow( apEx_MaterialShaderEditor );
  if ( w ) w->UpdateData();

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passparamlist" ), _T( "itemselector" ) );
  if ( l )
  {
    CphxMaterialRenderPass_Tool *t = GetEditedPass();
    l->Flush();
    l->SelectItem( -1 );

    if ( t )
      for ( TS32 x = 0; x < t->PassParameters.Parameters.NumItems(); x++ )
        l->AddItem( t->PassParameters.Parameters[ x ]->Name );

    if ( t )
    {
      auto *minibutt = (CWBButton*)FindChildByID( _T( "minifiable" ), _T( "button" ) );
      if ( minibutt ) minibutt->Push( t->Minifiable );
    }
  }

  SelectTechParam();
  SelectPassParam();
}

CphxMaterialParameter_Tool * CapexMaterialEditor::GetEditedTechParam()
{
  CphxMaterialTechnique_Tool *t = GetEditedTech();
  if ( !t ) return NULL;
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techparamlist" ), _T( "itemselector" ) );
  if ( !l ) return NULL;
  TS32 idx = l->GetCursorPosition();
  if ( idx < 0 || idx >= t->TechParameters.Parameters.NumItems() ) return NULL;
  return t->TechParameters.Parameters[ idx ];
}

CphxMaterialParameter_Tool * CapexMaterialEditor::GetEditedPassParam()
{
  CphxMaterialRenderPass_Tool *t = GetEditedPass();
  if ( !t ) return NULL;
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passparamlist" ), _T( "itemselector" ) );
  if ( !l ) return NULL;
  TS32 idx = l->GetCursorPosition();
  if ( idx < 0 || idx >= t->PassParameters.Parameters.NumItems() ) return NULL;
  return t->PassParameters.Parameters[ idx ];
}

void CapexMaterialEditor::SelectTechParam()
{
  CphxMaterialParameter_Tool *p = GetEditedTechParam();
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techparamscope" ), _T( "itemselector" ) );
  if ( l ) l->SelectItemByIndex( p ? p->Parameter.Scope : -1 );

  CWBButton *b = (CWBButton*)FindChildByID( _T( "techparamtype" ), _T( "button" ) );
  if ( b ) b->SetText( p ? MaterialTypeNames[ p->Parameter.Type ] : _T( "" ) );

  CWBFieldSet *fs = (CWBFieldSet*)FindChildByID( _T( "techparamvalues" ), _T( "fieldset" ) );
  if ( fs ) BuildParamDefaultValueGUI( fs, p );
}

void CapexMaterialEditor::SelectPassParam()
{
  CphxMaterialParameter_Tool *p = GetEditedPassParam();
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "passparamscope" ), _T( "itemselector" ) );
  if ( l ) l->SelectItemByIndex( p ? p->Parameter.Scope : -1 );

  CWBButton *b = (CWBButton*)FindChildByID( _T( "passparamtype" ), _T( "button" ) );
  if ( b ) b->SetText( p ? MaterialTypeNames[ p->Parameter.Type ] : _T( "" ) );

  CWBFieldSet *fs = (CWBFieldSet*)FindChildByID( _T( "passparamvalues" ), _T( "fieldset" ) );
  if ( fs ) BuildParamDefaultValueGUI( fs, p );
}

CphxMaterial_Tool * CapexMaterialEditor::GetEditedMaterial()
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "materiallist" ), _T( "itemselector" ) );

  if ( l )
  {
    TS32 cnt = Project.GetMaterialCount();
    TS32 p = l->GetCursorPosition();
    if ( p >= 0 && p < cnt )
      return Project.GetMaterialByIndex( p );
  }

  return NULL;
}

void CapexMaterialEditor::SelectMaterial()
{
  CphxMaterial_Tool *m = GetEditedMaterial();
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techsinmaterial" ), _T( "itemselector" ) );

  if ( !l || !m ) return;

  TS32 sel = l->GetCursorPosition();

  l->Flush();
  for ( TS32 x = 0; x < m->Techniques.NumItems(); x++ )
    l->AddItem( m->Techniques[ x ]->Name );

  l->SelectItemByIndex( sel );
}

void CapexMaterialEditor::BuildParamDefaultValueGUI( CWBItem *RootItem, CphxMaterialParameter_Tool *Param )
{
  RootItem->DeleteChildren();
  if ( !Param ) return;

  MATERIALVALUE v = Param->Parameter.Value;
  if ( Param->Parameter.Scope != PARAM_CONSTANT ) v = Param->DefaultValue;

  switch ( Param->Parameter.Type )
  {
  case PARAM_FLOAT:
  case PARAM_PARTICLELIFEFLOAT:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "floattemplate" ) ) );
    {
      CWBTrackBar *b = (CWBTrackBar*)RootItem->FindChildByIDs( _T( "floatvalue" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( b ) b->SetValue( (TS32)max( 0, min( 255, v.Float * 255 + 0.5 ) ) );
    }
    break;
  case PARAM_COLOR:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "colortemplate" ) ) );
    {
      CWBTrackBar *b = (CWBTrackBar*)RootItem->FindChildByIDs( _T( "redvalue" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( b ) b->SetValue( (TS32)max( 0, min( 255, v.Color[ 0 ] * 255 + 0.5 ) ) );
      b = (CWBTrackBar*)RootItem->FindChildByIDs( _T( "greenvalue" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( b ) b->SetValue( (TS32)max( 0, min( 255, v.Color[ 1 ] * 255 + 0.5 ) ) );
      b = (CWBTrackBar*)RootItem->FindChildByIDs( _T( "bluevalue" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( b ) b->SetValue( (TS32)max( 0, min( 255, v.Color[ 2 ] * 255 + 0.5 ) ) );
      b = (CWBTrackBar*)RootItem->FindChildByIDs( _T( "alphavalue" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( b ) b->SetValue( (TS32)max( 0, min( 255, v.Color[ 3 ] * 255 + 0.5 ) ) );
    }
    break;
  case PARAM_TEXTURE0:
  case PARAM_TEXTURE1:
  case PARAM_TEXTURE2:
  case PARAM_TEXTURE3:
  case PARAM_TEXTURE4:
  case PARAM_TEXTURE5:
  case PARAM_TEXTURE6:
  case PARAM_TEXTURE7:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "texturetemplate" ) ) );
    {
      CWBButton *b = (CWBButton*)RootItem->FindChildByID( _T( "textureselector" ), _T( "button" ) );
      if ( !b ) break;
      CphxRenderTarget_Tool *t = Project.GetRenderTarget( Param->TextureGUID );
      if ( t )
      {
        b->SetText( t->Name );
        break;
      }
      CphxTextureOperator_Tool *o = Project.GetTexgenOp( Param->TextureGUID );
      if ( o )
      {
        b->SetText( o->GetName() );
        break;
      }
      b->SetText( _T( "Select Texture" ) );
    }
    break;
  case PARAM_RENDERTARGET:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "rendertargettemplate" ) ) );
    {
      CWBButton *b = (CWBButton*)RootItem->FindChildByID( _T( "rendertargetselector" ), _T( "button" ) );
      if ( !b ) break;
      CphxRenderTarget_Tool *t = Project.GetRenderTarget( Param->TextureGUID );
      if ( t )
      {
        b->SetText( t->Name );
        break;
      }
      b->SetText( _T( "Select Rendertarget" ) );
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
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "blendtemplate" ) ) );
    {
      CWBItemSelector *b = (CWBItemSelector*)RootItem->FindChildByID( _T( "srcblend" ), _T( "itemselector" ) );
      if ( b ) b->SelectItemByIndex( v.BlendMode & 15 );
      b = (CWBItemSelector*)RootItem->FindChildByID( _T( "dstblend" ), _T( "itemselector" ) );
      if ( b ) b->SelectItemByIndex( v.BlendMode >> 4 );
    }
    break;
  case PARAM_CULLMODE:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "culltemplate" ) ) );
    {
      CWBItemSelector *b = (CWBItemSelector*)RootItem->FindChildByID( _T( "cullmode" ), _T( "itemselector" ) );
      if ( b ) b->SelectItemByIndex( v.CullMode - 1 );
    }
    break;
  case PARAM_ZMODE:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "zmodetemplate" ) ) );
    {
      CWBButton *b = (CWBButton*)RootItem->FindChildByID( _T( "zenable" ), _T( "button" ) );
      if ( b ) b->Push( !( v.ZMode & 1 ) );
      b = (CWBButton*)RootItem->FindChildByID( _T( "zwrite" ), _T( "button" ) );
      if ( b ) b->Push( !( ( v.ZMode >> 1 ) & 1 ) );
    }
    break;
  case PARAM_ZFUNCTION:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "zfuncttemplate" ) ) );
    {
      CWBItemSelector *b = (CWBItemSelector*)RootItem->FindChildByID( _T( "zfunct" ), _T( "itemselector" ) );
      if ( b ) b->SelectItemByIndex( v.ZFunction - 1 );
    }
    break;
  case PARAM_FILLMODE:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "fillmodetemplate" ) ) );
    {
      CWBButton *b = (CWBButton*)RootItem->FindChildByID( _T( "wireframe" ), _T( "button" ) );
      if ( b ) b->Push( v.Wireframe );
    }
    break;
  case PARAM_RENDERPRIORITY:
    App->GenerateGUITemplate( RootItem, CString( WINDOWXML ), CString( _T( "renderprioritytemplate" ) ) );
    {
      CWBTrackBar *b = (CWBTrackBar*)RootItem->FindChildByIDs( _T( "renderpriovalue" ), _T( "trackbar" ), _T( "numpad" ) );
      if ( b ) b->SetValue( v.RenderPriority );
    }
    break;
  default:
    break;
  }

  StyleManager.ApplyStyles( this );
  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );

}

void CapexMaterialEditor::BuildTextureSelectorUI( CWBItem *Button, TBOOL RTOnly )
{
  CWBContextMenu *c = Button->OpenContextMenu( Button->ClientToScreen( Button->GetClientRect().BottomLeft() ) );
  TextureIDMap.Flush();

  TS32 id = 0;

  for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
  {
    c->AddItem( Project.GetRenderTargetByIndex( x )->Name.GetPointer(), id );
    TextureIDMap[ id ] = Project.GetRenderTargetByIndex( x )->GetGUID();
    id++;
  }

  if ( RTOnly ) return;

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
}

void CapexMaterialEditor::CopyTech( CphxMaterialTechnique_Tool *Tech )
{
  if ( !Tech ) return;

  CphxMaterialTechnique_Tool *Copy = Project.CreateTech();
  Copy->Name = _T( "Copy of " ) + Tech->Name;
  Copy->SetTargetLayer( Project.GetRenderLayer( Tech->TargetLayer ) );
  Copy->Tech->Type = Tech->Tech->Type;

  for ( TS32 x = 0; x < Tech->TechParameters.Parameters.NumItems(); x++ )
  {
    CphxMaterialParameter_Tool *Param = Tech->TechParameters.Parameters[ x ];
    CphxMaterialParameter_Tool *ParamCopy = Copy->TechParameters.CreateParam();
    ParamCopy->Name = Param->Name;
    ParamCopy->Parameter = Param->Parameter;
    ParamCopy->DefaultValue = Param->DefaultValue;
    ParamCopy->TextureGUID = Param->TextureGUID;
  }

  for ( TS32 x = 0; x < Tech->Passes.NumItems(); x++ )
  {
    CphxMaterialRenderPass_Tool *Pass = Tech->Passes[ x ];
    CphxMaterialRenderPass_Tool *PassCopy = Copy->CreatePass();
    PassCopy->Name = Pass->Name;
    PassCopy->ShaderCode = Pass->ShaderCode;

    for ( TS32 y = 0; y < Pass->PassParameters.Parameters.NumItems(); y++ )
    {
      CphxMaterialParameter_Tool *Param = Pass->PassParameters.Parameters[ y ];
      CphxMaterialParameter_Tool *ParamCopy = PassCopy->PassParameters.CreateParam();
      ParamCopy->Name = Param->Name;
      ParamCopy->Parameter = Param->Parameter;
      ParamCopy->DefaultValue = Param->DefaultValue;
      ParamCopy->TextureGUID = Param->TextureGUID;
    }

    PassCopy->Rebuild();
  }

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "techlist" ), _T( "itemselector" ) );
  if ( l ) l->SelectItem( l->AddItem( Copy->Name ) );
}

#include "BuildInfo.h"
#include <CommDlg.h>

void CapexMaterialEditor::ExportTech( CphxMaterialTechnique_Tool *Tech )
{
  if ( !Tech ) return;

  CXMLDocument d;
  CXMLNode n = d.GetDocumentNode();
  n = n.AddChild( "apExMaterial" );
  n.SetAttribute( _T( "build" ), apexBuild.GetPointer() );

  CXMLNode t = n.AddChild( _T( "rendertechnique" ) );
  Tech->Export( &t );

  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Material Files\0*.apxmat\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save Material";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "apxmat";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  extern CapexRoot *Root;
  opf.lpstrInitialDir = Root->GetTargetDirectory( "exporttech" );

  if ( GetSaveFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "exporttech" );
    App->SelectMouseCursor( CM_WAIT );
    App->FinalizeMouseCursor();
    SetCurrentDirectory( dir );

    d.SaveToFile( CString( opf.lpstrFile ).GetPointer() );
  }
  SetCurrentDirectory( dir );
}

void CapexMaterialEditor::ExportMaterial( CphxMaterial_Tool *Mat )
{
  if ( !Mat ) return;

  CXMLDocument d;
  CXMLNode n = d.GetDocumentNode();
  n = n.AddChild( "apExMaterial" );
  n.SetAttribute( _T( "build" ), apexBuild.GetPointer() );

  for ( TS32 x = 0; x < Mat->Techniques.NumItems(); x++ )
  {
    CXMLNode t = n.AddChild( _T( "rendertechnique" ) );
    Mat->Techniques[ x ]->Export( &t );
  }

  CXMLNode t = n.AddChild( _T( "material" ) );
  Mat->Export( &t );

  TCHAR dir[ 1024 ];
  if ( !GetCurrentDirectory( 1024, dir ) )
    memset( dir, 0, sizeof( TCHAR ) * 1024 );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "apEx Material Files\0*.apxmat\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save Material";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "apxmat";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );
  opf.lpTemplateName = NULL;

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  extern CapexRoot *Root;
  opf.lpstrInitialDir = Root->GetTargetDirectory( "exportmaterial" );

  if ( GetSaveFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "exportmaterial" );
    App->SelectMouseCursor( CM_WAIT );
    App->FinalizeMouseCursor();
    SetCurrentDirectory( dir );

    d.SaveToFile( CString( opf.lpstrFile ).GetPointer() );
  }
  SetCurrentDirectory( dir );
}
