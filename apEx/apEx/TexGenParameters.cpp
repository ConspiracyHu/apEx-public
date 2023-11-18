#include "BasePCH.h"
#include "apExRoot.h"
#include "TexGenParameters.h"
#define WINDOWNAME _T("Texgen Operator Parameters")
#define WINDOWXML _T("TexgenOpParameters")
#include "../Phoenix_Tool/Texgen_tool.h"
#include "SplineEditor_Phx.h"
#include "../Phoenix_Tool/apxProject.h"

void CapexTextureOpParameters::LoadCSS()
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

CapexTextureOpParameters::CapexTextureOpParameters() : CapexWindow()
{
  for ( TS32 x = 0; x < 4; x++ )
    VisibleSplines[ x ] = true;
  EditedOp = -1;
  LoadCSS();
}

CapexTextureOpParameters::CapexTextureOpParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML )
{
  for ( TS32 x = 0; x < 4; x++ )
    VisibleSplines[ x ] = true;
  EditedOp = -1;
  LoadCSS();
}

CapexTextureOpParameters::~CapexTextureOpParameters()
{

}

TBOOL CapexTextureOpParameters::HandleByteParamChange( TU8 Data, CWBTrackBar *v, unsigned char ParamArray[ TEXGEN_MAX_PARAMS ], TS32 s, TS32 ParamType, CString &Name )
{
  switch ( ParamType )
  {
  case 0: //byte param
  {
    v->SetText( CString::Format( _T( "%s: %d" ), Name.GetPointer(), Data ) );
    if ( ParamArray[ s ] != Data )
    {
      ParamArray[ s ] = Data;
      return true;
    }
  }
  break;
  case 2: //rgba param
  {
    TBOOL DataChanged = false;
    if ( v->GetID() == "rbar" )
    {
      v->SetText( CString::Format( _T( "%s red: %d" ), Name.GetPointer(), Data ) );
      if ( ParamArray[ s ] != Data ) DataChanged = true;
      ParamArray[ s ] = Data;
    }
    if ( v->GetID() == "gbar" )
    {
      v->SetText( CString::Format( _T( "%s green: %d" ), Name.GetPointer(), Data ) );
      if ( ParamArray[ s + 1 ] != Data ) DataChanged = true;
      ParamArray[ s + 1 ] = Data;
    }
    if ( v->GetID() == "bbar" )
    {
      v->SetText( CString::Format( _T( "%s blue: %d" ), Name.GetPointer(), Data ) );
      if ( ParamArray[ s + 2 ] != Data ) DataChanged = true;
      ParamArray[ s + 2 ] = Data;
    }
    if ( v->GetID() == "abar" )
    {
      v->SetText( CString::Format( _T( "%s alpha: %d" ), Name.GetPointer(), Data ) );
      if ( ParamArray[ s + 3 ] != Data ) DataChanged = true;
      ParamArray[ s + 3 ] = Data;
    }
    if ( DataChanged ) return true;
    break;
  }
  default:
    break;
  }

  return false;
}

TBOOL CapexTextureOpParameters::MessageProc( CWBMessage &Message )
{
  CWBItem *target = App->FindItemByGuid( Message.GetTarget() );

  CphxTextureOperator_Tool *EditedOperator = Project.GetTexgenOp( EditedOp );

  CphxTextureFilter_Tool *filter = NULL;
  if ( EditedOperator && EditedOperator->Filter )
    filter = EditedOperator->Filter;

  switch ( Message.GetMessage() )
  {
  case WBM_TEXTCHANGED:
  {
    CWBTextBox *v = (CWBTextBox*)target;
    if ( !v ) break;
    if ( !EditedOperator ) break;

    if ( v->GetID() == _T( "textinput" ) )
      if ( EditedOperator->Filter && EditedOperator->Filter->Filter.DataDescriptor.LookupType == 2 )
      {
        //SAFEDELETEA(EditedOperator->TextData.Text);
        //CString &s = v->GetText();
        //EditedOperator->TextData.Text = new char[s.Length() + 1];
        //memset(EditedOperator->TextData.Text, 0, s.Length() + 1);
        //memcpy(EditedOperator->TextData.Text, s.GetPointer(), s.Length());
        SAFEDELETEA( EditedOperator->Text );
        CString &s = v->GetText();
        EditedOperator->Text = new char[ s.Length() + 1 ];
        memset( EditedOperator->Text, 0, s.Length() + 1 );
        memcpy( EditedOperator->Text, s.GetPointer(), s.Length() );
        EditedOperator->InvalidateUptoDateFlag();
        return true;
      }
  }
  break;
  case WBM_VALUECHANGED:
  {
    CWBTrackBar *v = (CWBTrackBar*)target;
    if ( !v ) break;
    if ( !EditedOperator ) break;

    if ( EditedOperator->GetOpType() == TEXGEN_OP_SUBROUTINE )
    {
      CphxTextureOperator_Subroutine *sub = (CphxTextureOperator_Subroutine*)EditedOperator;
      CWBTrackBar *submin = (CWBTrackBar*)FindChildByIDs( _T( "submin" ), _T( "trackbar" ), _T( "numpad" ) );
      CWBTrackBar *submax = (CWBTrackBar*)FindChildByIDs( _T( "submax" ), _T( "trackbar" ), _T( "numpad" ) );
      CWBTrackBar *subdef = (CWBTrackBar*)FindChildByIDs( _T( "subdef" ), _T( "trackbar" ), _T( "numpad" ) );

      CWBItemSelector *subparamlist = (CWBItemSelector*)FindChildByID( _T( "subparamlist" ), _T( "itemselector" ) );

      if ( target == submin )
      {
        submin->SetText( CString::Format( _T( "Minimum: %d" ), Message.Data ) );
        if ( subparamlist )
        {
          TS32 p = subparamlist->GetCursorPosition();
          if ( p >= 0 && p < sub->Parameters.NumItems() )
            sub->Parameters[ p ]->Minimum = Message.Data;
        }
      }

      if ( target == submax )
      {
        submax->SetText( CString::Format( _T( "Maximum: %d" ), Message.Data ) );
        if ( subparamlist )
        {
          TS32 p = subparamlist->GetCursorPosition();
          if ( p >= 0 && p < sub->Parameters.NumItems() )
            sub->Parameters[ p ]->Maximum = Message.Data;
        }
      }

      if ( target == subdef )
      {
        subdef->SetText( CString::Format( _T( "Default: %d" ), Message.Data ) );
        if ( subparamlist )
        {
          TS32 p = subparamlist->GetCursorPosition();
          if ( p >= 0 && p < sub->Parameters.NumItems() )
            sub->Parameters[ p ]->Default = Message.Data;
        }
      }

      return true;
    }

    if ( EditedOperator->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
    {
      CphxTextureOperator_Subroutine *sub = ( (CphxTextureOperator_SubroutineCall*)EditedOperator )->Subroutine;
      for ( TS32 x = 0; x < GeneratedParamRoots.NumItems(); x++ )
        if ( target->FindItemInParentTree( GeneratedParamRoots[ x ] ) )
          if ( x >= 0 && x < sub->Parameters.NumItems() )
          {
            if ( HandleByteParamChange( Message.Data, v, EditedOperator->OpData.Parameters, sub->GetParamStart( x ), sub->Parameters[ x ]->Type, sub->Parameters[ x ]->Name ) )
              EditedOperator->InvalidateUptoDateFlag();
            return true;
          }
    }

    if ( filter )
    {
      if ( filter->Filter.DataDescriptor.LookupType == 2 )
      {
        //text input
        CWBTrackBar *textxoff = (CWBTrackBar*)FindChildByIDs( _T( "textxoffset" ), _T( "trackbar" ), _T( "numpad" ) );
        CWBTrackBar *textyoff = (CWBTrackBar*)FindChildByIDs( _T( "textyoffset" ), _T( "trackbar" ), _T( "numpad" ) );
        CWBTrackBar *textsize = (CWBTrackBar*)FindChildByIDs( _T( "textsize" ), _T( "trackbar" ), _T( "numpad" ) );
        CWBTrackBar *textcharspace = (CWBTrackBar*)FindChildByIDs( _T( "textcharspacing" ), _T( "trackbar" ), _T( "numpad" ) );
        if ( target == textxoff )
        {
          textxoff->SetText( CString::Format( _T( "X Position: %d" ), Message.Data ) );
          EditedOperator->TextData.XPos = Message.Data;
          EditedOperator->InvalidateUptoDateFlag();
          return true;
        }
        if ( target == textyoff )
        {
          textyoff->SetText( CString::Format( _T( "Y Position: %d" ), Message.Data ) );
          EditedOperator->TextData.YPos = Message.Data;
          EditedOperator->InvalidateUptoDateFlag();
          return true;
        }
        if ( target == textsize )
        {
          textsize->SetText( CString::Format( _T( "Text Size: %d" ), Message.Data ) );
          EditedOperator->TextData.Size = Message.Data;
          EditedOperator->InvalidateUptoDateFlag();
          return true;
        }
        if ( target == textcharspace )
        {
          textcharspace->SetText( CString::Format( _T( "Character Spacing: %d" ), Message.Data ) );
          EditedOperator->TextData.CharSpace = Message.Data;
          EditedOperator->InvalidateUptoDateFlag();
          return true;
        }
      }

      for ( TS32 x = 0; x < GeneratedParamRoots.NumItems(); x++ )
        if ( target->FindItemInParentTree( GeneratedParamRoots[ x ] ) )
          if ( x >= 0 && x < filter->Parameters.NumItems() )
          {
            if ( HandleByteParamChange( Message.Data, v, EditedOperator->OpData.Parameters, filter->GetParamStart( x ), filter->Parameters[ x ]->Type, filter->Parameters[ x ]->Name ) )
              EditedOperator->InvalidateUptoDateFlag();
            return true;
          }

      if ( filter->Filter.DataDescriptor.NeedsRandSeed )
      {
        CWBBox *rbox = (CWBBox*)FindChildByID( _T( "randseed" ), _T( "box" ) );
        if ( rbox )
        {
          if ( v->GetID() == "bytebar" )
          {
            v->SetText( CString::Format( _T( "Random Seed: %d" ), Message.Data ) );
            if ( EditedOperator->OpData.RandSeed != Message.Data )
            {
              EditedOperator->OpData.RandSeed = Message.Data;
              EditedOperator->InvalidateUptoDateFlag();
            }
            return true;
          }
        }
      }
    }

  }
  break;
  case WBM_ITEMRENAMED:
  {
    if ( !EditedOperator ) break;
    if ( EditedOperator->GetOpType() != TEXGEN_OP_SUBROUTINE ) break;

    CphxTextureOperator_Subroutine *Sub = (CphxTextureOperator_Subroutine*)EditedOperator;

    CWBItemSelector *l = (CWBItemSelector*)FindChildByID( Message.GetTargetID(), _T( "itemselector" ) );
    if ( Message.IsTargetID( _T( "subparamlist" ) ) )
    {
      CWBSelectableItem *i = l->GetItem( Message.Data );
      TS32 p = l->GetCursorPosition();
      if ( i && p >= 0 && p < Sub->Parameters.NumItems() )
        Sub->Parameters[ p ]->Name = i->GetText();
      return true;
    }
  }
  break;
  case WBM_ITEMSELECTED:
    if ( !target ) break;
    if ( !EditedOperator ) break;

    if ( EditedOperator->Filter && EditedOperator->Filter->Filter.DataDescriptor.LookupType == 2 )
    {
      //text input
      CWBItemSelector *fontlist = (CWBItemSelector*)FindChildByID( _T( "fontlist" ), _T( "itemselector" ) );
      if ( target == fontlist )
      {
        EditedOperator->TextData.Font = fontlist->GetCursorPosition();
        EditedOperator->InvalidateUptoDateFlag();
        return true;
      }
    }

    if ( EditedOperator->GetOpType() == TEXGEN_OP_SUBROUTINE )
    {
      CphxTextureOperator_Subroutine *sub = (CphxTextureOperator_Subroutine*)EditedOperator;
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( Message.GetTargetID(), _T( "itemselector" ) );

      CWBItemSelector *subparamlist = (CWBItemSelector*)FindChildByID( _T( "subparamlist" ), _T( "itemselector" ) );
      CWBItemSelector *subparamtargets = (CWBItemSelector*)FindChildByID( _T( "subparamtargets" ), _T( "itemselector" ) );
      CWBItemSelector *subparamtype = (CWBItemSelector*)FindChildByID( _T( "subparamtype" ), _T( "itemselector" ) );

      CWBTrackBar *submin = (CWBTrackBar*)FindChildByIDs( _T( "submin" ), _T( "trackbar" ), _T( "numpad" ) );
      CWBTrackBar *submax = (CWBTrackBar*)FindChildByIDs( _T( "submax" ), _T( "trackbar" ), _T( "numpad" ) );
      CWBTrackBar *subdef = (CWBTrackBar*)FindChildByIDs( _T( "subdef" ), _T( "trackbar" ), _T( "numpad" ) );

      if ( Message.IsTargetID( _T( "subparamlist" ) ) )
      {
        subparamtargets->Flush();
        TS32 p = l->GetCursorPosition();

        if ( p >= 0 && p < sub->Parameters.NumItems() )
        {
          if ( subparamtype )
            subparamtype->SelectItemByIndex( sub->Parameters[ p ]->Type );
          if ( submin )
          {
            submin->SetConstraints( 0, 255 );
            submin->SetValue( sub->Parameters[ p ]->Minimum );
          }
          if ( submax )
          {
            submax->SetConstraints( 0, 255 );
            submax->SetValue( sub->Parameters[ p ]->Maximum );
          }
          if ( subdef )
          {
            subdef->SetConstraints( 0, 255 );
            subdef->SetValue( sub->Parameters[ p ]->Default );
          }
          for ( TS32 x = 0; x < sub->Parameters[ p ]->Targets.NumItems(); x++ )
          {
            CphxTextureOperator_Tool *trg = Project.GetTexgenOp( sub->Parameters[ p ]->Targets[ x ]->TargetID );

            CString st = CString::Format( _T( "Error - target %d not found" ), sub->Parameters[ p ]->Targets[ x ]->TargetID );
            if ( trg )
            {
              CString fname;
              if ( trg->Filter )
              {
                if ( sub->Parameters[ p ]->Targets[ x ]->OpParam != -1 )
                  fname = trg->Filter->Parameters[ sub->Parameters[ p ]->Targets[ x ]->OpParam ]->Name;
                else fname = _T( "RandSeed" );
              }
              if ( trg->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
              {
                CphxTextureOperator_SubroutineCall *scall = (CphxTextureOperator_SubroutineCall*)trg;
                if ( scall->Subroutine )
                {
                  if ( sub->Parameters[ p ]->Targets[ x ]->OpParam != -1 )
                    fname = scall->Subroutine->Parameters[ sub->Parameters[ p ]->Targets[ x ]->OpParam ]->Name;
                  else fname = _T( "RandSeed" );
                }
              }

              st = CString::Format( _T( "%s (%d) - %s" ), trg->GetName().GetPointer(), trg->ID, fname.GetPointer() );
            }
            subparamtargets->AddItem( st );
          }
        }
        return true;
      }

      if ( Message.IsTargetID( _T( "subparamtype" ) ) )
      {
        if ( subparamlist )
        {
          TS32 p = subparamlist->GetCursorPosition();
          if ( p >= 0 && p < sub->Parameters.NumItems() )
            sub->Parameters[ p ]->Type = subparamtype->GetCursorPosition();
        }
        return true;
      }

      if ( Message.IsTargetID( _T( "subparamtargets" ) ) )
      {
        return true;
      }

      break;
    }

    if ( !filter ) break;
    {
      CWBItemSelector *l = (CWBItemSelector*)target;
      for ( TS32 x = 0; x < GeneratedParamRoots.NumItems(); x++ )
        if ( target->FindItemInParentTree( GeneratedParamRoots[ x ] ) )
        {
          TS32 s = filter->GetParamStart( x );
          if ( filter->Parameters[ x ]->Type == 4 )
          {
            if ( EditedOperator->OpData.Parameters[ s ] != l->GetCursorPosition() )
            {
              EditedOperator->OpData.Parameters[ s ] = l->GetCursorPosition();
              EditedOperator->InvalidateUptoDateFlag();
            }
          }
          return true;
        }
    }
    break;
  case WBM_CONTEXTMESSAGE:
  {
    if ( !target ) break;
    if ( !EditedOperator ) break;

    if ( EditedOperator->GetOpType() == TEXGEN_OP_LOAD )
    {
      CWBButton *loadop = (CWBButton*)FindChildByID( _T( "loadoperator" ), _T( "button" ) );
      if ( target == loadop )
      {
        CphxTextureOperator_Load *load = (CphxTextureOperator_Load*)EditedOperator;
        load->LoadedOp = (CphxTextureOperator_Tool*)Message.Data;
        load->ApplyItemSpecificParents();
        Project.KillTextureLoops();
        load->InvalidateUptoDateFlag();
      }

      return true;
    }

  }
  break;
  case WBM_COMMAND:
  {
    if ( !target ) break;
    if ( !EditedOperator ) break;

    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );

    if ( b && ( b->GetID() == _T( "splinered" ) || b->GetID() == _T( "splinegreen" ) || b->GetID() == _T( "splineblue" ) || b->GetID() == _T( "splinealpha" ) ) )
    {
      if ( b->GetID() == _T( "splinered" ) ) VisibleSplines[ 0 ] = !VisibleSplines[ 0 ];
      if ( b->GetID() == _T( "splinegreen" ) ) VisibleSplines[ 1 ] = !VisibleSplines[ 1 ];
      if ( b->GetID() == _T( "splineblue" ) ) VisibleSplines[ 2 ] = !VisibleSplines[ 2 ];
      if ( b->GetID() == _T( "splinealpha" ) ) VisibleSplines[ 3 ] = !VisibleSplines[ 3 ];

      b->Push( !b->IsPushed() );

      CapexSplineEditor_phx *s = (CapexSplineEditor_phx*)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
      if ( s )
      {
        s->FlushSplines();
        if ( VisibleSplines[ 0 ] ) s->AddSpline( (CphxSpline_Tool*)EditedOperator->Curves[ 0 ], CColor::FromARGB( 0xffff0000 ), CString( _T( "Red" ) ) );
        if ( VisibleSplines[ 1 ] ) s->AddSpline( (CphxSpline_Tool*)EditedOperator->Curves[ 1 ], CColor::FromARGB( 0xff00ff00 ), CString( _T( "Green" ) ) );
        if ( VisibleSplines[ 2 ] ) s->AddSpline( (CphxSpline_Tool*)EditedOperator->Curves[ 2 ], CColor::FromARGB( 0xff0000ff ), CString( _T( "Blue" ) ) );
        if ( VisibleSplines[ 3 ] ) s->AddSpline( (CphxSpline_Tool*)EditedOperator->Curves[ 3 ], CColor::FromARGB( 0xffffffff ), CString( _T( "Alpha" ) ) );
      }

      return true;
    }

    if ( b && ( b->GetID() == _T( "applygamma" ) ) )
    {
      for ( TS32 x = 0; x < GeneratedParamRoots.NumItems(); x++ )
        if ( b->FindItemInParentTree( GeneratedParamRoots[ x ] ) )
          if ( x >= 0 && x < filter->Parameters.NumItems() )
          {
            TS32 red = (TS32)( DeGamma( EditedOperator->OpData.Parameters[ filter->GetParamStart( x ) + 0 ] / 255.0f )*255.0f );
            TS32 gre = (TS32)( DeGamma( EditedOperator->OpData.Parameters[ filter->GetParamStart( x ) + 1 ] / 255.0f )*255.0f );
            TS32 blu = (TS32)( DeGamma( EditedOperator->OpData.Parameters[ filter->GetParamStart( x ) + 2 ] / 255.0f )*255.0f );
            CWBTrackBar *i1 = (CWBTrackBar*)GeneratedParamRoots[ x ]->FindChildByID( "rbar", "trackbar" );
            CWBTrackBar *i2 = (CWBTrackBar*)GeneratedParamRoots[ x ]->FindChildByID( "gbar", "trackbar" );
            CWBTrackBar *i3 = (CWBTrackBar*)GeneratedParamRoots[ x ]->FindChildByID( "bbar", "trackbar" );
            if ( i1 ) i1->SetValue( red );
            if ( i2 ) i2->SetValue( gre );
            if ( i3 ) i3->SetValue( blu );
            return true;
          }
    }

    if ( EditedOperator->Filter && EditedOperator->Filter->Filter.DataDescriptor.LookupType == 2 )
    {
      CWBButton *bold = (CWBButton*)FindChildByID( _T( "boldbutton" ), _T( "button" ) );
      CWBButton *italic = (CWBButton*)FindChildByID( _T( "italicbutton" ), _T( "button" ) );
      if ( target == bold )
      {
        EditedOperator->TextData.Bold = !EditedOperator->TextData.Bold;
        bold->Push( EditedOperator->TextData.Bold );
        EditedOperator->InvalidateUptoDateFlag();
        return true;
      }
      if ( target == italic )
      {
        EditedOperator->TextData.Italic = !EditedOperator->TextData.Italic;
        italic->Push( EditedOperator->TextData.Italic );
        EditedOperator->InvalidateUptoDateFlag();
        return true;
      }
    }

    if ( EditedOperator->GetOpType() == TEXGEN_OP_SUBROUTINE )
    {
      CphxTextureOperator_Subroutine *sub = (CphxTextureOperator_Subroutine*)EditedOperator;
      CWBButton *addparam = (CWBButton*)FindChildByID( _T( "newparam" ), _T( "button" ) );
      CWBButton *delparam = (CWBButton*)FindChildByID( _T( "delparam" ), _T( "button" ) );
      CWBButton *removetarget = (CWBButton*)FindChildByID( _T( "removetarget" ), _T( "button" ) );

      CWBItemSelector *subparamlist = (CWBItemSelector*)FindChildByID( _T( "subparamlist" ), _T( "itemselector" ) );
      CWBItemSelector *subparamtargets = (CWBItemSelector*)FindChildByID( _T( "subparamtargets" ), _T( "itemselector" ) );

      if ( target == addparam )
      {
        CphxSubroutineParam *p = new CphxSubroutineParam();
        p->Name = _T( "New Parameter" );
        p->Type = 0;
        p->Minimum = 0;
        p->Maximum = 255;
        p->Default = 127;
        sub->Parameters += p;
        if ( subparamlist )
        {
          TS32 v = subparamlist->AddItem( p->Name );
          subparamlist->SelectItem( v );
        }
      }

      if ( target == delparam )
      {
        if ( subparamlist )
        {
          TS32 v = subparamlist->GetCursorPosition();
          if ( v >= 0 && v < sub->Parameters.NumItems() )
          {
            sub->Parameters.DeleteByIndex( v );
            subparamlist->DeleteItem( subparamlist->GetCursorItem()->GetID() );
            subparamlist->SelectItem( v );
          }
        }
      }

      if ( target == removetarget )
      {
        if ( subparamtargets && subparamlist )
        {
          TS32 p = subparamlist->GetCursorPosition();
          if ( p >= 0 && p < sub->Parameters.NumItems() )
          {
            TS32 l = subparamtargets->GetCursorPosition();
            if ( l >= 0 && l < sub->Parameters[ p ]->Targets.NumItems() )
            {
              sub->Parameters[ p ]->Targets.FreeByIndex( l );
              subparamtargets->DeleteItem( subparamtargets->GetCursorItem()->GetID() );
            }
          }
        }
      }

    }
    else
      if ( EditedOperator->GetOpType() == TEXGEN_OP_LOAD )
      {
        CphxTextureOperator_Load *sub = (CphxTextureOperator_Load*)EditedOperator;
        CWBButton *loadop = (CWBButton*)FindChildByID( _T( "loadoperator" ), _T( "button" ) );

        if ( target == loadop )
        {
          //CWBContextMenu *context=new CWBContextMenu(App->GetRoot(),CRect(App->GetMousePos(),App->GetMousePos()),target->GetGuid());
          CWBContextMenu *context = target->OpenContextMenu( App->GetMousePos() );
          for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
          {
            CapexTexGenPage *p = Project.GetTexgenPageByIndex( x );
            CWBContextItem *it = context->AddItem( p->GetName().GetPointer(), 0 );
            for ( TS32 y = 0; y < p->GetOpCount(); y++ )
              if ( p->GetOp( y )->GetOpType() == TEXGEN_OP_SAVE )
                it->AddItem( p->GetOp( y )->GetName().GetPointer(), (TS32)p->GetOp( y ) );
          }
        }

        return true;
      }

    {
      CWBButton *loadimage = (CWBButton*)FindChildByID( _T( "loadimage" ), _T( "button" ) );
      if ( target == loadimage && loadimage )
      {
        OpenImageImport();
        return true;
      }
    }

    //if (!filter) break;
    {
      CWBButton *l = (CWBButton*)target;
      for ( TS32 x = 0; x < GeneratedParamRoots.NumItems(); x++ )
        if ( target->FindItemInParentTree( GeneratedParamRoots[ x ] ) )
        {
          TS32 s = 0;
          TS32 type = 0;
          if ( EditedOperator->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
          {
            CphxTextureOperator_Subroutine *sub = ( (CphxTextureOperator_SubroutineCall*)EditedOperator )->Subroutine;
            if ( sub )
            {
              s = sub->GetParamStart( x );
              type = sub->Parameters[ x ]->Type;
            }
          }
          if ( filter )
          {
            s = filter->GetParamStart( x );
            type = filter->Parameters[ x ]->Type;
          }
          switch ( type )
          {
          case 1: //button
            EditedOperator->OpData.Parameters[ s ] = !EditedOperator->OpData.Parameters[ s ];
            l->Push( EditedOperator->OpData.Parameters[ s ] );
            EditedOperator->InvalidateUptoDateFlag();
            break;
          case 3: //channel
          {
            CWBButton *br = (CWBButton*)GeneratedParamRoots[ x ]->FindChildByID( _T( "ch1button" ), _T( "button" ) );
            CWBButton *bg = (CWBButton*)GeneratedParamRoots[ x ]->FindChildByID( _T( "ch2button" ), _T( "button" ) );
            CWBButton *bb = (CWBButton*)GeneratedParamRoots[ x ]->FindChildByID( _T( "ch3button" ), _T( "button" ) );
            CWBButton *ba = (CWBButton*)GeneratedParamRoots[ x ]->FindChildByID( _T( "ch4button" ), _T( "button" ) );
            if ( br == l ) EditedOperator->OpData.Parameters[ s ] = 0;
            if ( bg == l ) EditedOperator->OpData.Parameters[ s ] = 1;
            if ( bb == l ) EditedOperator->OpData.Parameters[ s ] = 2;
            if ( ba == l ) EditedOperator->OpData.Parameters[ s ] = 3;
            if ( br ) br->Push( br == l );
            if ( bg ) bg->Push( bg == l );
            if ( bb ) bb->Push( bb == l );
            if ( ba ) ba->Push( ba == l );
            EditedOperator->InvalidateUptoDateFlag();
          }
          break;
          default:
            break;
          }

          return true;
        }
    }
    break;
  }
  case WBM_SPLINECHANGED:
    if ( !target ) break;
    if ( !EditedOperator ) break;
    EditedOperator->InvalidateUptoDateFlag();
    return true;
    break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}


void CapexTextureOpParameters::UpdateData()
{
  CphxTextureOperator_Tool *EditedOperator = Project.GetTexgenOp( EditedOp );
  SetEditedOperator( EditedOperator );
}

void CapexTextureOpParameters::FlushParameters()
{
  GeneratedParamRoots.FlushFast();
  DeleteChildren();

  //CString xmlname=CString(_T("Data/UI/"))+WINDOWXML;
  //App->LoadCSSFromFile(xmlname+".css");

  App->GenerateGUI( this, WINDOWXML );
  StyleManager.ApplyStyles( this );

  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );
}

void CapexTextureOpParameters::AddParameter( TS32 ParamID )
{
  CphxTextureOperator_Tool *EditedOperator = Project.GetTexgenOp( EditedOp );
  CWBBox *bb = (CWBBox*)FindChildByID( _T( "parameterlist" ), _T( "box" ) );
  if ( !bb || !EditedOperator ) return;

  CString Name = _T( "" );
  TU8 *Params = NULL;
  TS32 start = 0;
  TS32 type = 0;
  TS32 parammin = 0;
  TS32 parammax = 0;

  CphxTextureFilterParameter *Param = NULL;

  if ( EditedOperator->GetOpType() != TEXGEN_OP_SUBROUTINECALL )
  {
    if ( !EditedOperator->Filter ) return;
    Param = EditedOperator->Filter->Parameters[ ParamID ];
    Name = EditedOperator->Filter->Parameters[ ParamID ]->Name;
    Params = EditedOperator->OpData.Parameters;
    start = EditedOperator->Filter->GetParamStart( ParamID );
    type = Param->Type;
    parammin = Param->Minimum;
    parammax = Param->Maximum;
  }

  if ( EditedOperator->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
  {
    CphxTextureOperator_Subroutine *Sub = ( (CphxTextureOperator_SubroutineCall*)EditedOperator )->Subroutine;
    if ( !Sub ) return;

    Name = Sub->Parameters[ ParamID ]->Name;
    Params = EditedOperator->OpData.Parameters;
    start = Sub->GetParamStart( ParamID );
    type = Sub->Parameters[ ParamID ]->Type;
    parammin = Sub->Parameters[ ParamID ]->Minimum;
    parammax = Sub->Parameters[ ParamID ]->Maximum;
  }

  CWBBox *nb = new CWBBox( bb, CRect( 10, 10, 50, 50 ) );
  nb->AddClass( _T( "generatedgui" ) );
  nb->SetID( Name );

  GeneratedParamRoots += nb;

  App->GenerateGUITemplate( nb, CString( WINDOWXML ), CString::Format( "%d", type ) );

  CWBLabel *l = (CWBLabel*)nb->FindChildByID( _T( "label" ), _T( "label" ) );
  if ( l )
    l->SetText( Name );

  switch ( type )
  {
  case 0: //byte param
  {
    CWBTrackBar *b = (CWBTrackBar*)nb->FindChildByIDs( _T( "bytebar" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( b )
    {
      b->SetConstraints( parammin, parammax );
      if ( Params )
        b->SetValue( Params[ start ] );
    }
  }
  case 1: //bool param
  {
    CWBButton *b = (CWBButton*)nb->FindChildByID( _T( "boolbutton" ), _T( "button" ) );
    if ( b )
    {
      b->SetText( Name );
      if ( Params ) b->Push( Params[ start ] );
    }
  }
  break;
  case 2: //rgba param
  {
    CWBTrackBar *r = (CWBTrackBar*)nb->FindChildByIDs( _T( "rbar" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( r )
    {
      r->SetConstraints( parammin, parammax );
      r->SetValue( Params[ start ] );
    }
    CWBTrackBar *g = (CWBTrackBar*)nb->FindChildByIDs( _T( "gbar" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( g )
    {
      g->SetConstraints( parammin, parammax );
      g->SetValue( Params[ start + 1 ] );
    }
    CWBTrackBar *b = (CWBTrackBar*)nb->FindChildByIDs( _T( "bbar" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( b )
    {
      b->SetConstraints( parammin, parammax );
      b->SetValue( Params[ start + 2 ] );
    }
    CWBTrackBar *a = (CWBTrackBar*)nb->FindChildByIDs( _T( "abar" ), _T( "trackbar" ), _T( "numpad" ) );
    if ( a )
    {
      a->SetConstraints( parammin, parammax );
      a->SetValue( Params[ start + 3 ] );
    }
  }
  break;
  case 3: //channel param
  {
    CWBButton *b1 = (CWBButton*)nb->FindChildByID( _T( "ch1button" ), _T( "button" ) );
    CWBButton *b2 = (CWBButton*)nb->FindChildByID( _T( "ch2button" ), _T( "button" ) );
    CWBButton *b3 = (CWBButton*)nb->FindChildByID( _T( "ch3button" ), _T( "button" ) );
    CWBButton *b4 = (CWBButton*)nb->FindChildByID( _T( "ch4button" ), _T( "button" ) );
    TS32 v = Params[ start ];
    if ( b1 ) b1->Push( v == 0 );
    if ( b2 ) b2->Push( v == 1 );
    if ( b3 ) b3->Push( v == 2 );
    if ( b4 ) b4->Push( v == 3 );
  }
  break;
  case 4: //list param
  {
    if ( !Param ) return;

    CWBItemSelector *b = (CWBItemSelector*)nb->FindChildByID( _T( "listlist" ), _T( "itemselector" ) );
    if ( b )
      for ( TS32 x = 0; x < Param->ListItems.NumItems(); x++ )
        b->AddItem( Param->ListItems[ x ] );
    if ( b )
      b->SelectItemByIndex( Params[ start ] );
  }
  break;

  default:
    break;
  }
}

void CapexTextureOpParameters::SetEditedOperator( CphxTextureOperator_Tool *op )
{
  CphxTextureOperator_Tool *EditedOperator = op;
  if ( op )
    EditedOp = op->ID;
  else EditedOp = -1;
  //EditedOperator=op;
  FlushParameters();
  if ( !op ) return;

  if ( op->Filter )
  {
    if ( op->Filter->Filter.DataDescriptor.LookupType )
    {
      //need to add lookup texture parameters
      CWBBox *b = (CWBBox*)FindChildByID( _T( "parameterlist" ), _T( "box" ) );
      if ( b )
      {
        CWBBox *nb = new CWBBox( b, CRect( 10, 10, 50, 50 ) );
        nb->AddClass( _T( "generatedgui" ) );
        nb->SetID( _T( "lookup" ) );
        App->GenerateGUITemplate( nb, WINDOWXML, CString::Format( _T( "lookup%d" ), op->Filter->Filter.DataDescriptor.LookupType ).GetPointer() );

        switch ( op->Filter->Filter.DataDescriptor.LookupType )
        {
        case 1:
          break;
        case 2:
        {	// text input
          CWBTextBox *ib = (CWBTextBox*)FindChildByID( _T( "textinput" ), _T( "textbox" ) );
          if ( ib )
          {
            //if (!op->TextData.Text) ib->SetText(_T("Enter Text Here!"));
            //ib->SetText(_T(op->TextData.Text));
            if ( !op->Text ) ib->SetText( _T( "Enter Text Here!" ) );
            ib->SetText( _T( op->Text ) );
          }
          CWBTrackBar *vb = (CWBTrackBar*)FindChildByIDs( _T( "textxoffset" ), _T( "trackbar" ), _T( "numpad" ) );
          if ( vb ) vb->SetValue( op->TextData.XPos );
          vb = (CWBTrackBar*)FindChildByIDs( _T( "textyoffset" ), _T( "trackbar" ), _T( "numpad" ) );
          if ( vb ) vb->SetValue( op->TextData.YPos );
          vb = (CWBTrackBar*)FindChildByIDs( _T( "textsize" ), _T( "trackbar" ), _T( "numpad" ) );
          if ( vb ) vb->SetValue( op->TextData.Size );
          vb = (CWBTrackBar*)FindChildByIDs( _T( "textcharspacing" ), _T( "trackbar" ), _T( "numpad" ) );
          if ( vb ) vb->SetValue( op->TextData.CharSpace );
          CWBButton *bb = (CWBButton*)FindChildByID( _T( "boldbutton" ), _T( "button" ) );
          if ( bb ) bb->Push( op->TextData.Bold );
          bb = (CWBButton*)FindChildByID( _T( "italicbutton" ), _T( "button" ) );
          if ( bb ) bb->Push( op->TextData.Italic );
          CWBItemSelector *fl = (CWBItemSelector*)FindChildByID( _T( "fontlist" ), _T( "itemselector" ) );
          if ( fl ) fl->SelectItemByIndex( op->TextData.Font );
        }
        break;
        case 3:
        {	// curves input
          CapexSplineEditor_phx *s = (CapexSplineEditor_phx*)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
          if ( s )
          {
            for ( int x = 0; x < 4; x++ )
              VisibleSplines[ x ] = op->UiVisibleCurveChannels[ x ];

            CWBButton *b = (CWBButton*)FindChildByID( _T( "splinered" ), _T( "button" ) );
            if ( b ) b->Push( VisibleSplines[ 0 ] );
            b = (CWBButton*)FindChildByID( _T( "splinegreen" ), _T( "button" ) );
            if ( b ) b->Push( VisibleSplines[ 1 ] );
            b = (CWBButton*)FindChildByID( _T( "splineblue" ), _T( "button" ) );
            if ( b ) b->Push( VisibleSplines[ 2 ] );
            b = (CWBButton*)FindChildByID( _T( "splinealpha" ), _T( "button" ) );
            if ( b ) b->Push( VisibleSplines[ 3 ] );

            if ( VisibleSplines[ 0 ] ) s->AddSpline( (CphxSpline_Tool*)op->Curves[ 0 ], CColor::FromARGB( 0xffff0000 ), CString( _T( "Red" ) ) );
            if ( VisibleSplines[ 1 ] ) s->AddSpline( (CphxSpline_Tool*)op->Curves[ 1 ], CColor::FromARGB( 0xff00ff00 ), CString( _T( "Green" ) ) );
            if ( VisibleSplines[ 2 ] ) s->AddSpline( (CphxSpline_Tool*)op->Curves[ 2 ], CColor::FromARGB( 0xff0000ff ), CString( _T( "Blue" ) ) );
            if ( VisibleSplines[ 3 ] ) s->AddSpline( (CphxSpline_Tool*)op->Curves[ 3 ], CColor::FromARGB( 0xffffffff ), CString( _T( "Alpha" ) ) );

            s->ZoomX = op->UiCurveZoomX;
            s->ZoomY = op->UiCurveZoomY;
            s->OffsetX = op->UiCurveOffsetX;
            s->OffsetY = op->UiCurveOffsetY;
          }
        }
        break;
        default:
          break;
        }

      }
    }

    if ( op->Filter->Filter.DataDescriptor.NeedsRandSeed ) //add randseed bar
    {
      CWBBox *b = (CWBBox*)FindChildByID( _T( "parameterlist" ), _T( "box" ) );
      if ( b )
      {
        CWBBox *nb = new CWBBox( b, CRect( 10, 10, 50, 50 ) );
        nb->AddClass( _T( "generatedgui" ) );
        nb->SetID( _T( "randseed" ) );
        App->GenerateGUITemplate( nb, WINDOWXML, _T( "0" ) );
        CWBTrackBar *vb = (CWBTrackBar*)nb->FindChildByIDs( _T( "bytebar" ), _T( "trackbar" ), _T( "numpad" ) );
        if ( vb )
        {
          vb->SetConstraints( 0, 255 );
          vb->SetValue( EditedOperator->OpData.RandSeed );
        }
      }
    }

    for ( TS32 x = 0; x < op->Filter->Parameters.NumItems(); x++ )
      AddParameter( x );
  }

  if ( op->GetOpType() == TEXGEN_OP_SUBROUTINE )
  {
    //build subroutine ui
    CphxTextureOperator_Subroutine *Sub = (CphxTextureOperator_Subroutine*)op;
    App->GenerateGUITemplate( this, CString( WINDOWXML ), CString( _T( "subroutine" ) ) );
    CWBItemSelector *subparamlist = (CWBItemSelector*)FindChildByID( _T( "subparamlist" ), _T( "itemselector" ) );

    if ( subparamlist )
      for ( TS32 x = 0; x < Sub->Parameters.NumItems(); x++ )
        subparamlist->AddItem( Sub->Parameters[ x ]->Name );
  }

  if ( op->GetOpType() == TEXGEN_OP_LOAD )
  {
    //build load ui
    CphxTextureOperator_Load *Sub = (CphxTextureOperator_Load*)op;
    App->GenerateGUITemplate( this, CString( WINDOWXML ), CString( _T( "load" ) ) );

    CWBLabel *opname = (CWBLabel*)FindChildByID( _T( "loadedop" ), _T( "label" ) );

    if ( opname )
    {
      CphxTextureOperator_Tool *cop = Sub->LoadedOp;

      if ( cop )
        opname->SetText( cop->GetName() );
    }
  }

  if ( op->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
  {
    //build subroutine call ui
    CphxTextureOperator_SubroutineCall *SubCall = (CphxTextureOperator_SubroutineCall*)op;
    CphxTextureOperator_Subroutine *Sub = SubCall->Subroutine;

    if ( Sub )
      for ( TS32 x = 0; x < Sub->Parameters.NumItems(); x++ )
        AddParameter( x );
  }

  StyleManager.ApplyStyles( this );

  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );
}

void CapexTextureOpParameters::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}

void CapexTextureOpParameters::BackupSplineUiStateTo( CphxTextureOperator_Tool* op )
{
  if ( !op )
    return;

  CapexSplineEditor_phx *s = (CapexSplineEditor_phx*)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( !s )
    return;

  for ( int x = 0; x < 4; x++ )
    op->UiVisibleCurveChannels[ x ] = VisibleSplines[ x ];

  op->UiCurveZoomX = s->ZoomX;
  op->UiCurveZoomY = s->ZoomY;
  op->UiCurveOffsetX = s->OffsetX;
  op->UiCurveOffsetY = s->OffsetY;
}

#include <CommDlg.h>
#include "apExRoot.h"

void CapexTextureOpParameters::OpenImageImport()
{
  CphxTextureOperator_Tool *EditedOperator = Project.GetTexgenOp( EditedOp );
  if ( !EditedOperator ) return;

  TCHAR dir[ 1024 ];
  GetCurrentDirectory( 1024, dir );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "Image Files\0*.jpg;*.png;*.bmp;*.gif;*.emf;*.wmf\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Open Image";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = NULL;
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  extern CapexRoot *Root;
  opf.lpstrInitialDir = Root->GetTargetDirectory( "openimage" );

  if ( GetOpenFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "openimage" );
    SetCurrentDirectory( dir );
    CStreamReaderFile f;
    if ( f.Open( opf.lpstrFile ) )
    {
      if ( f.GetLength() )
      {
        SAFEDELETEA( EditedOperator->ImageData );
        EditedOperator->ImageData = new TU8[ (TS32)f.GetLength() ];
        f.Read( EditedOperator->ImageData, (TS32)f.GetLength() );
        EditedOperator->ImageDataSize = (TS32)f.GetLength();
        EditedOperator->InvalidateUptoDateFlag();
      }
    }
  }

  SetCurrentDirectory( dir );

}
