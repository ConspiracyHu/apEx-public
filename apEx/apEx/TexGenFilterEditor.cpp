#include "BasePCH.h"
#include "TexGenFilterEditor.h"
#include "TextBox_HLSL.h"
#include "ExternalTools.h"

#define WINDOWNAME _T("Texgen Filter Editor")
#define WINDOWXML _T("TextureFilterEditor")

CapexTexGenFilterEditor::CapexTexGenFilterEditor() : CapexWindow()
{
}

CapexTexGenFilterEditor::CapexTexGenFilterEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML )
{
  EditedFilter = NULL;
}

CapexTexGenFilterEditor::~CapexTexGenFilterEditor()
{

}

TBOOL CapexTexGenFilterEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
    if ( Message.IsTargetID( _T( "filterrandom" ) ) )
    {
      CWBButton *l = (CWBButton*)FindChildByID( Message.GetTargetID(), _T( "button" ) );
      if ( l )
      {
        l->Push( !l->IsPushed() );
        if ( EditedFilter ) EditedFilter->Filter.DataDescriptor.NeedsRandSeed = l->IsPushed();
      }
      return true;
    }
    if ( Message.IsTargetID( _T( "minifiable" ) ) )
    {
      CWBButton *l = (CWBButton*)FindChildByID( Message.GetTargetID(), _T( "button" ) );
      if ( l )
      {
        l->Push( !l->IsPushed() );
        if ( EditedFilter ) EditedFilter->Minifiable = l->IsPushed();
      }
      return true;
    }
    if ( Message.IsTargetID( _T( "newfilter" ) ) )
    {
      AddFilter( Project.CreateTextureFilter() );
      return true;
    }
    if ( Message.IsTargetID( _T( "newparam" ) ) )
    {
      if ( !EditedFilter ) return true;

      CphxTextureFilterParameter *p = new CphxTextureFilterParameter();
      EditedFilter->Parameters += p;
      SetEditedFilter( EditedFilter );
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
      if ( g ) g->SelectItemByIndex( EditedFilter->Parameters.NumItems() - 1 );

      return true;
    }
    if ( Message.IsTargetID( _T( "deleteparam" ) ) )
    {
      if ( !EditedFilter ) return true;
      if ( !EditedFilter->Parameters.NumItems() ) return true;
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
      if ( !g ) return true;
      if ( g->GetCursorPosition() < 0 || g->GetCursorPosition() >= EditedFilter->Parameters.NumItems() ) return true;
      EditedFilter->Parameters.DeleteByIndex( g->GetCursorPosition() );
      RebuildParameterList();
      g->SelectItemByIndex( max( 0, min( EditedFilter->Parameters.NumItems() - 1, g->GetCursorPosition() ) ) );
      return true;
    }
    if ( Message.IsTargetID( _T( "deletefilter" ) ) )
    {
      if ( !EditedFilter ) return true;
      Project.DeleteTextureFilter( EditedFilter->GetGUID() );
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "filterlist" ), _T( "itemselector" ) );
      if ( g )
      {
        TS32 cpos = g->GetCursorPosition();
        g->Flush();
        for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
        {
          CphxTextureFilter_Tool*f = Project.GetTextureFilterByIndex( x );
          g->AddItem( f->Name );
        }

        g->SelectItemByIndex( max( 0, min( Project.GetTextureFilterCount() - 1, cpos ) ) );
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "addparamlistelement" ) ) )
    {
      if ( !EditedFilter ) return true;
      if ( !EditedFilter->Parameters.NumItems() ) return true;
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
      if ( !g ) return true;
      if ( g->GetCursorPosition() < 0 || g->GetCursorPosition() >= EditedFilter->Parameters.NumItems() ) return true;

      CphxTextureFilterParameter *p = EditedFilter->Parameters[ g->GetCursorPosition() ];
      p->ListItems += _T( "New Entry" );

      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "listelements" ), _T( "itemselector" ) );
      if ( !l ) return true;
      l->AddItem( p->ListItems.Last() );

      return true;
    }

    if ( Message.IsTargetID( _T( "deleteparamlistelement" ) ) )
    {
      if ( !EditedFilter ) return true;
      if ( !EditedFilter->Parameters.NumItems() ) return true;
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
      if ( !g ) return true;
      if ( g->GetCursorPosition() < 0 || g->GetCursorPosition() >= EditedFilter->Parameters.NumItems() ) return true;

      CphxTextureFilterParameter *p = EditedFilter->Parameters[ g->GetCursorPosition() ];
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "listelements" ), _T( "itemselector" ) );
      if ( !l ) return true;

      if ( l->GetCursorPosition() < 0 || l->GetCursorPosition() >= p->ListItems.NumItems() ) return true;
      p->ListItems.DeleteByIndex( l->GetCursorPosition() );
      l->DeleteItem( l->GetCursorItem()->GetID() );
      return true;
    }

    break;
  case WBM_KEYDOWN:
  {
    if ( Message.Key == VK_F5 )
    {
      CompileShader();
      return true;
    }
    if ( Message.Key == VK_F6 )
    {
      MinimizeShader();
      return true;
    }
    if ( Message.Key == VK_F7 )
    {
      TestHLSLLexer();
      return true;
    }
  }
  break;
  case WBM_FOCUSLOST:
  {
    CWBTextBox *b = (CWBTextBox*)FindChildByID( _T( "filtercode" ), _T( "textbox" ) );
    if ( b && Message.GetTarget() == b->GetGuid() )
    {
      CompileShader();
      return true;
    }
  }
  break;
  case WBM_TEXTCHANGED:
  {
    CWBTextBox *l = (CWBTextBox*)FindChildByID( Message.GetTargetID(), _T( "textbox" ) );
    if ( Message.IsTargetID( _T( "filtercode" ) ) )
    {
      if ( EditedFilter )
        EditedFilter->Code = l->GetText();
      return true;
    }
  }
  break;
  case WBM_ITEMRENAMED:
  {
    CWBItemSelector *l = (CWBItemSelector*)FindChildByID( Message.GetTargetID(), _T( "itemselector" ) );
    if ( Message.IsTargetID( _T( "filterlist" ) ) )
    {
      CWBSelectableItem *i = l->GetItem( Message.Data );
      CphxTextureFilter_Tool *f = Project.GetTextureFilterByIndex( l->GetCursorPosition() );
      if ( !f ) return true;
      f->Name = i->GetText();
      return true;
    }

    if ( Message.IsTargetID( _T( "paramlist" ) ) )
    {
      if ( !EditedFilter ) return true;
      TS32 id = l->GetCursorPosition();
      if ( !( id >= 0 && id < EditedFilter->Parameters.NumItems() ) ) return true;
      EditedFilter->Parameters[ id ]->Name = l->GetItem( Message.Data )->GetText();
      return true;
    }

    if ( Message.IsTargetID( _T( "listelements" ) ) )
    {
      if ( !EditedFilter ) return true;
      if ( !EditedFilter->Parameters.NumItems() ) return true;
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
      if ( !g ) return true;
      if ( g->GetCursorPosition() < 0 || g->GetCursorPosition() >= EditedFilter->Parameters.NumItems() ) return true;

      CphxTextureFilterParameter *p = EditedFilter->Parameters[ g->GetCursorPosition() ];
      if ( l->GetCursorPosition() < 0 || l->GetCursorPosition() >= p->ListItems.NumItems() ) return true;
      p->ListItems[ l->GetCursorPosition() ] = l->GetItem( Message.Data )->GetText();
    }

  }
  break;
  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *l = (CWBItemSelector*)FindChildByID( Message.GetTargetID(), _T( "itemselector" ) );
    if ( Message.IsTargetID( _T( "filterlist" ) ) )
    {
      SetEditedFilter( Project.GetTextureFilterByIndex( l->GetCursorPosition() ) );
      return true;
    }
    if ( Message.IsTargetID( _T( "filtertype" ) ) )
    {
      if ( EditedFilter )
      {
        EditedFilter->Type = l->GetCursorPosition();
      }
      return true;
    }
    if ( Message.IsTargetID( _T( "paramlist" ) ) )
    {
      SetEditedParam( l->GetCursorPosition() );
      return true;
    }

    if ( Message.IsTargetID( _T( "paramtype" ) ) )
    {
      CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
      if ( EditedFilter && g && g->GetCursorPosition() >= 0 && g->GetCursorPosition() < EditedFilter->Parameters.NumItems() )
        EditedFilter->Parameters[ g->GetCursorPosition() ]->Type = l->GetCursorPosition();

      TS32 x = l->GetCursorPosition();

      CWBBox *b = (CWBBox*)FindChildByID( _T( "paramdatabox" ), _T( "box" ) );
      if ( !b ) return true;

      for ( TU32 y = 0; y < b->NumChildren(); y++ )
        b->GetChild( y )->Hide( true );

      if ( x >= 0 && x < (TS32)b->NumChildren() )
        b->GetChild( x )->Hide( false );

      return true;
    }
  }
  break;
  case WBM_VALUECHANGED:
  {
    CWBTrackBar *l = (CWBTrackBar*)FindChildByIDs( Message.GetTargetID().GetPointer(), _T( "trackbar" ), _T( "numpad" ) );
    CWBItemSelector *g = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );

    if ( Message.IsTargetID( _T( "filterinputcount" ) ) )
    {
      if ( l ) l->SetText( CString::Format( _T( "Inputs: %d" ), Message.Data ) );
      if ( EditedFilter ) EditedFilter->Filter.DataDescriptor.InputCount = Message.Data;
      return true;
    }
    if ( Message.IsTargetID( _T( "filterpasscount" ) ) )
    {
      if ( l ) l->SetText( CString::Format( _T( "Passes: %d" ), Message.Data ) );
      if ( EditedFilter ) EditedFilter->Filter.DataDescriptor.PassCount = Message.Data;
      return true;
    }
    if ( Message.IsTargetID( _T( "parammin" ) ) )
    {
      if ( l ) l->SetText( CString::Format( _T( "Minimum: %d" ), Message.Data ) );
      if ( EditedFilter && g && g->GetCursorPosition() >= 0 && g->GetCursorPosition() < EditedFilter->Parameters.NumItems() )
        EditedFilter->Parameters[ g->GetCursorPosition() ]->Minimum = Message.Data;
      return true;
    }
    if ( Message.IsTargetID( _T( "parammax" ) ) )
    {
      if ( l ) l->SetText( CString::Format( _T( "Maximum: %d" ), Message.Data ) );
      if ( EditedFilter && g && g->GetCursorPosition() >= 0 && g->GetCursorPosition() < EditedFilter->Parameters.NumItems() )
        EditedFilter->Parameters[ g->GetCursorPosition() ]->Maximum = Message.Data;
      return true;
    }
    if ( Message.IsTargetID( _T( "paramdef" ) ) )
    {
      if ( l ) l->SetText( CString::Format( _T( "Default: %d" ), Message.Data ) );
      if ( EditedFilter && g && g->GetCursorPosition() >= 0 && g->GetCursorPosition() < EditedFilter->Parameters.NumItems() )
        EditedFilter->Parameters[ g->GetCursorPosition() ]->Default = Message.Data;
      return true;
    }
  }
  break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTexGenFilterEditor::SetEditedFilter( CphxTextureFilter_Tool *f )
{
  CWBTextBox *ou = (CWBTextBox*)FindChildByID( _T( "compileroutput" ), _T( "textbox" ) );
  if ( ou ) ou->SetText( _T( "" ) );


  EditedFilter = f;

  if ( !f )
  {
    return;
  }

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "filtertype" ), _T( "itemselector" ) );
  if ( l )
  {
    if ( f )
      l->SelectItemByIndex( f->Type );
  }

  CWBTextBox *b = (CWBTextBox*)FindChildByID( _T( "filtercode" ), _T( "textbox" ) );
  if ( b ) b->SetText( f->Code );
  if ( b ) b->SetCursorPos( 0, false );

  CWBTrackBar *v = (CWBTrackBar*)FindChildByIDs( _T( "filterinputcount" ), _T( "trackbar" ), _T( "numpad" ) );
  if ( v ) v->SetValue( (TS32)f->Filter.DataDescriptor.InputCount );
  v = (CWBTrackBar*)FindChildByIDs( _T( "filterpasscount" ), _T( "trackbar" ), _T( "numpad" ) );
  if ( v ) v->SetValue( (TS32)f->Filter.DataDescriptor.PassCount );

  CWBButton *but = (CWBButton*)FindChildByID( _T( "filterrandom" ), _T( "button" ) );
  if ( but ) but->Push( f->Filter.DataDescriptor.NeedsRandSeed );

  but = (CWBButton*)FindChildByID( _T( "minifiable" ), _T( "button" ) );
  if ( but ) but->Push( f->Minifiable );

  RebuildParameterList();
  l = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
  if ( f->Parameters.NumItems() )
    l->SelectItemByIndex( 0 );

  //f->Filter.DataDescriptor.ParameterCount;
}

void CapexTexGenFilterEditor::AddFilter( CphxTextureFilter_Tool *f )
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "filterlist" ), _T( "itemselector" ) );
  if ( l )
    l->SelectItem( l->AddItem( f->Name ) );

  SetEditedFilter( f );
}

void CapexTexGenFilterEditor::SetEditedParam( TS32 Index )
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
  //if (l) 
  //	l->SelectItemByIndex(Index);


  if ( !EditedFilter ) return;
  if ( Index < 0 || EditedFilter->Parameters.NumItems() <= Index ) return;

  CphxTextureFilterParameter *p = EditedFilter->Parameters[ Index ];

  l = (CWBItemSelector*)FindChildByID( _T( "paramtype" ), _T( "itemselector" ) );
  if ( l )
    l->SelectItemByIndex( p->Type );

  CWBTrackBar *v = (CWBTrackBar*)FindChildByIDs( _T( "parammin" ), _T( "trackbar" ), _T( "numpad" ) );
  if ( v ) v->SetValue( p->Minimum );
  v = (CWBTrackBar*)FindChildByIDs( _T( "parammax" ), _T( "trackbar" ), _T( "numpad" ) );
  if ( v ) v->SetValue( p->Maximum );
  v = (CWBTrackBar*)FindChildByIDs( _T( "paramdef" ), _T( "trackbar" ), _T( "numpad" ) );
  if ( v ) v->SetValue( p->Default );

  // list param list	
  l = (CWBItemSelector*)FindChildByID( _T( "listelements" ), _T( "itemselector" ) );
  if ( !l ) return;
  l->Flush();
  for ( TS32 x = 0; x < p->ListItems.NumItems(); x++ )
    l->AddItem( p->ListItems[ x ] );
}

void CapexTexGenFilterEditor::RebuildParameterList()
{
  if ( !EditedFilter ) return;

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );

  if ( l )
  {
    l->Flush();
    for ( TS32 x = 0; x < EditedFilter->Parameters.NumItems(); x++ )
      l->AddItem( EditedFilter->Parameters[ x ]->Name );
  }
}

void CapexTexGenFilterEditor::UpdateData()
{
  TS32 selectedfilter = 0;
  TS32 selectedparam = 0;

  CWBItemSelector *pl = (CWBItemSelector*)FindChildByID( _T( "paramlist" ), _T( "itemselector" ) );
  CWBItemSelector *fl = (CWBItemSelector*)FindChildByID( _T( "filterlist" ), _T( "itemselector" ) );

  if ( fl ) selectedfilter = fl->GetCursorPosition();
  if ( pl ) selectedparam = pl->GetCursorPosition();

  if ( fl )
  {
    fl->Flush();
    for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
    {
      CphxTextureFilter_Tool*f = Project.GetTextureFilterByIndex( x );
      fl->AddItem( f->Name );
    }
  }

  RebuildParameterList();

  if ( fl ) fl->SelectItemByIndex( selectedfilter );
  if ( pl ) pl->SelectItemByIndex( selectedparam );

  SetEditedFilter( EditedFilter );
}

void CapexTexGenFilterEditor::CompileShader()
{
  CWBTextBox *b = (CWBTextBox*)FindChildByID( _T( "filtercode" ), _T( "textbox" ) );
  if ( !EditedFilter ) return;

  if ( EditedFilter->Shader ) SAFEDELETE( EditedFilter->Shader );

  CWBTextBox *ou = (CWBTextBox*)FindChildByID( _T( "compileroutput" ), _T( "textbox" ) );

  CString err;
  EditedFilter->Shader = App->GetDevice()->CreatePixelShader( b->GetText().GetPointer(), b->GetText().Length(), "p", "ps_5_0", &err );
  if ( ou )
  {
    ou->SetText( err );
    if ( !err.Length() )
    {
      ou->SetText( _T( "Shader compiled successfully." ) );
      EditedFilter->Filter.PixelShader = (ID3D11PixelShader*)EditedFilter->Shader->GetHandle();
    }
  }

  EditedFilter->InvalidateUptoDateFlag();

  //CString Mini;
  //MinifyShader(Code->GetText().GetPointer(),Mini);
  //Code->SetText(Mini);
}

void CapexTexGenFilterEditor::MinimizeShader()
{
  if ( !EditedFilter ) return;

  CWBTextBox *b = (CWBTextBox*)FindChildByID( _T( "minioutput" ), _T( "textbox" ) );
  if ( !b ) return;

  CString mini;

  MinifyShader( EditedFilter->Code.GetPointer(), mini );
  b->SetText( mini );
}

#include "HLSLParser.h"

void CapexTexGenFilterEditor::TestHLSLLexer()
{
  if ( !EditedFilter ) return;

  CWBTextBox *b = (CWBTextBox*)FindChildByID( _T( "filtercode" ), _T( "textbox" ) );
  if ( !b ) return;

  CString Output;
  if ( CrossCompiler::Parser::Parse( EditedFilter->Code, Output, true ) )
  {
    b->SetText( Output, true );
    EditedFilter->InvalidateUptoDateFlag();
  }

  //Scanner.Lex(EditedFilter->Code);
  //Scanner.Dump();
}
