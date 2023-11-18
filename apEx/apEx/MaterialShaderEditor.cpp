#include "BasePCH.h"
#include "MaterialShaderEditor.h"
#define WINDOWNAME _T("Shader Editor")
#define WINDOWXML _T("ShaderEditor")
#include "WorkBench.h"
#include "MaterialEditor.h"
#include "TextBox_HLSL.h"
#include "ExternalTools.h"

CapexMaterialShaderEditor::CapexMaterialShaderEditor() : CapexWindow()
{
}

CapexMaterialShaderEditor::CapexMaterialShaderEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexMaterialShaderEditor::~CapexMaterialShaderEditor()
{
}

void CapexMaterialShaderEditor::UpdateData()
{
  CphxMaterialRenderPass_Tool *p = GetEditedPass();
  if ( !p )
  {
    //h=(CWBTextBox_HLSL*)FindChildByID(_T("shadercode"),_T("textbox"));
    //if (h) 
    //	h->SetText(_T(""));
    //h=(CWBTextBox_HLSL*)FindChildByID(_T("shadererror"),_T("textbox"));
    //if (h) 
    //	h->SetText(_T(""));
    return;
  }

  CWBTextBox_HLSL *h = (CWBTextBox_HLSL*)FindChildByID( _T( "shadercode" ), _T( "textbox" ) );
  if ( h )
  {
    h->SetText( p->ShaderCode );
    h->SetCursorPos( 0, false );
  }
  else
    h->SetText( _T( "" ) );
}

TBOOL CapexMaterialShaderEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_TEXTCHANGED:
  {
    CWBTextBox_HLSL *h = (CWBTextBox_HLSL*)FindChildByID( _T( "shadercode" ), _T( "textbox" ) );
    if ( h && h->GetGuid() == Message.GetTarget() )
    {
      CphxMaterialRenderPass_Tool *p = GetEditedPass();
      if ( !p ) break;
      p->ShaderCode = h->GetText();
      return true;
    }
  }
  break;
  case WBM_FOCUSLOST:
  {
    CWBTextBox_HLSL *h = (CWBTextBox_HLSL*)FindChildByID( _T( "shadercode" ), _T( "textbox" ) );
    if ( h && h->GetGuid() == Message.GetTarget() )
    {
      CompileEditedShader();
      return true;
    }
  }
  break;
  case WBM_KEYDOWN:
  {
    if ( Message.Key == VK_F5 )
    {
      CompileEditedShader();
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
  default:
    break;

  }

  return CapexWindow::MessageProc( Message );
}

CphxMaterialRenderPass_Tool * CapexMaterialShaderEditor::GetEditedPass()
{
  CapexMaterialEditor *m = (CapexMaterialEditor*)WorkBench->GetWindow( apEx_MaterialEditor );
  if ( !m ) return NULL;
  return m->GetEditedPass();
}

void CapexMaterialShaderEditor::CompileEditedShader()
{
  CWBTextBox_HLSL *h = (CWBTextBox_HLSL*)FindChildByID( _T( "shadererror" ), _T( "textbox" ) );
  if ( h ) h->SetText( _T( "" ) );

  CphxMaterialRenderPass_Tool *p = GetEditedPass();
  if ( !p ) return;

  CString Error;
  p->CompileShaders( App->GetDevice(), Error );

  if ( h ) h->SetText( Error );
}

void CapexMaterialShaderEditor::MinimizeShader()
{
  CWBTextBox_HLSL *h = (CWBTextBox_HLSL*)FindChildByID( _T( "shadercode" ), _T( "textbox" ) );
  CWBTextBox_HLSL *o = (CWBTextBox_HLSL*)FindChildByID( _T( "shaderheader" ), _T( "textbox" ) );
  if ( !h ) return;
  if ( !o ) return;

  CString mini;
  if ( !MinifyShader( h->GetText().GetPointer(), mini ) )
  {
    o->SetText( "Failed to minimize shader." );
    return;
  }

  o->SetText( mini );

}

#include "HLSLParser.h"

void CapexMaterialShaderEditor::TestHLSLLexer()
{
  CWBTextBox_HLSL *h = (CWBTextBox_HLSL*)FindChildByID( _T( "shadercode" ), _T( "textbox" ) );
  if ( !h ) return;

  CString Output;
  if ( CrossCompiler::Parser::Parse( h->GetText().GetPointer(), Output, true ) )
  {
    h->SetText( Output, true );
    CphxMaterialRenderPass_Tool *p = GetEditedPass();
    if ( !p ) return;
    p->ShaderCode = Output;
    CompileEditedShader();
  }
  //Scanner.Lex(EditedFilter->Code);
  //Scanner.Dump();
}
