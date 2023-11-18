#include "BasePCH.h"
#include "RenderTargetEditor.h"
#define WINDOWNAME _T("Render Target Editor")
#define WINDOWXML _T("RenderTargetEditor")
#include "apExRoot.h"

CString pixelFormatNames[] =
{
  "RGBA 16 Float",
  "R 32 Float",
};

CapexRenderTargetEditor::CapexRenderTargetEditor() : CapexWindow()
{
}

CapexRenderTargetEditor::CapexRenderTargetEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexRenderTargetEditor::~CapexRenderTargetEditor()
{

}

void CapexRenderTargetEditor::UpdateData()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "rtlist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->Flush();
    for ( TS32 x = 0; x < Project.GetRenderTargetCount(); x++ )
      List->AddItem( Project.GetRenderTargetByIndex( x )->Name );
  }
  List = (CWBItemSelector*)FindChildByID( _T( "rllist" ), _T( "itemselector" ) );
  if ( List )
  {
    List->Flush();
    for ( TS32 x = 0; x < Project.GetRenderLayerCount(); x++ )
      List->AddItem( Project.GetRenderLayerByIndex( x )->Name );
  }

  List = (CWBItemSelector*)FindChildByID( _T( "pixelformat" ), _T( "itemselector" ) );
  if ( List )
  {
    List->Flush();
    for ( TS32 x = 0; x < sizeof( pixelFormatNames ) / sizeof( CString ); x++ )
      List->AddItem( pixelFormatNames[ x ], x );
  }
}


CphxRenderTarget_Tool * CapexRenderTargetEditor::GetEditedRT()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "rtlist" ), _T( "itemselector" ) );
  if ( !List ) return NULL;
  return Project.GetRenderTargetByIndex( List->GetCursorPosition() );
}

TBOOL CapexRenderTargetEditor::MessageProc( CWBMessage &Message )
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "rtlist" ), _T( "itemselector" ) );

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
    if ( Message.IsTargetID( _T( "newrt" ) ) )
    {
      CphxRenderTarget_Tool *t = Project.CreateRenderTarget();
      if ( List )
        List->SelectItem( List->AddItem( t->Name ) );
      return true;
    }
    if ( List && Message.IsTargetID( _T( "deletert" ) ) )
    {
      Project.DeleteRenderTarget( GetEditedRT() );
      if ( List )
      {
        CWBSelectableItem *i = List->GetCursorItem();
        if ( i )
        {
          TS32 x = List->GetCursorPosition();
          List->DeleteItem( i->GetID() );
          List->SelectItemByIndex( x );
        }
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "newrl" ) ) )
    {
      CWBItemSelector *List2 = (CWBItemSelector*)FindChildByID( _T( "rllist" ), _T( "itemselector" ) );
      CphxRenderLayerDescriptor_Tool *t = Project.CreateRenderLayer();
      if ( List2 )
        List2->SelectItem( List2->AddItem( t->Name ) );

      extern CapexRoot *Root;
      Root->UpdateWindowData( apEx_MaterialEditor );
      return true;
    }

    if ( Message.IsTargetID( _T( "addrt" ) ) )
    {
      CWBItemSelector *List2 = (CWBItemSelector*)FindChildByID( _T( "rltlist" ), _T( "itemselector" ) );

      CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();

      CphxRenderTarget_Tool *rt = GetEditedRT();
      if ( !rt || !rl ) return true;

      if ( rl->RenderTargets.Find( rt ) >= 0 ) return true;

      rl->AddRenderTarget( rt );

      if ( List2 )
        List2->AddItem( rt->Name );

      return true;
    }

    if ( Message.IsTargetID( _T( "omitdepth" ) ) )
    {
      CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();

      if ( rl )
      {
        CWBButton *b = (CWBButton*)FindChildByID( _T( "omitdepth" ), _T( "button" ) );
        if ( b ) b->Push( !b->IsPushed() );
        rl->RenderLayer.OmitDepthBuffer = !rl->RenderLayer.OmitDepthBuffer;
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "voxelizer" ) ) )
    {
      CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();
      if ( rl )
      {
        CWBButton *b = (CWBButton*)FindChildByID( _T( "voxelizer" ), _T( "button" ) );
        if ( b ) b->Push( !b->IsPushed() );
        rl->RenderLayer.VoxelizerLayer = !rl->RenderLayer.VoxelizerLayer;
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "nogrid" ) ) )
    {
      CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();
      if ( rl )
      {
        CWBButton *b = (CWBButton*)FindChildByID( _T( "nogrid" ), _T( "button" ) );
        if ( b ) b->Push( !b->IsPushed() );
        rl->RenderLayer.IgnoreHelperObjects = !rl->RenderLayer.IgnoreHelperObjects;
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "isclickable" ) ) )
    {
      CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();
      if ( rl )
      {
        CWBButton *b = (CWBButton*)FindChildByID( _T( "isclickable" ), _T( "button" ) );
        if ( b ) b->Push( !b->IsPushed() );
        rl->HasPicking = !rl->HasPicking;
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "clearrendertargets" ) ) )
    {
      CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();
      if ( rl )
      {
        CWBButton *b = (CWBButton*)FindChildByID( _T( "clearrendertargets" ), _T( "button" ) );
        if ( b ) b->Push( !b->IsPushed() );
        rl->RenderLayer.clearRenderTargets = !rl->RenderLayer.clearRenderTargets;
      }
      return true;
    }

    if ( Message.IsTargetID( _T( "hiddenfromtimeline" ) ) )
    {
      CphxRenderTarget_Tool *rt = GetEditedRT();

      CWBButton *b = (CWBButton*)FindChildByID( _T( "hiddenfromtimeline" ), _T( "button" ) );
      if ( b ) b->Push( !b->IsPushed() );
      rt->HiddenFromTimeline = !rt->HiddenFromTimeline;

      return true;
    }

    if ( Message.IsTargetID( _T( "cubemap" ) ) )
    {
      CphxRenderTarget_Tool *rt = GetEditedRT();

      CWBButton *b = (CWBButton*)FindChildByID( _T( "cubemap" ), _T( "button" ) );
      if ( b ) b->Push( !b->IsPushed() );
      rt->cubeMap = !rt->cubeMap;

      return true;
    }

    if ( Message.IsTargetID( _T( "relativeres" ) ) )
    {
      CphxRenderTarget_Tool *t = GetEditedRT();
      if ( !t ) return true;

      TBOOL Absolute = ( t->ResolutionDescriptor & 0x80 ) == 0;
      t->ResolutionDescriptor = ( t->ResolutionDescriptor&~0x80 ) | ( 0x80 * Absolute );

      CWBButton *b = (CWBButton*)FindChildByID( _T( "relativeres" ), _T( "button" ) );
      CWBTrackBar *x = (CWBTrackBar*)FindChildByID( _T( "xres" ), _T( "trackbar" ) );
      CWBTrackBar *y = (CWBTrackBar*)FindChildByID( _T( "yres" ), _T( "trackbar" ) );
      CWBTrackBar *z = (CWBTrackBar*)FindChildByID( _T( "zres" ), _T( "trackbar" ) );

      Absolute = ( t->ResolutionDescriptor & 0x80 ) == 0;
      TS32 XFact = ( t->ResolutionDescriptor & 0x38 ) >> 3;
      TS32 YFact = ( t->ResolutionDescriptor & 0x07 );

      if ( x ) x->SetValue( XFact );
      if ( y ) y->SetValue( YFact );
      if ( z ) z->SetValue( t->ZResolution );
      if ( b ) b->Push( Absolute );

      //t->Reallocate(App->GetDevice());

      return true;
    }

    break;
  case WBM_ITEMSELECTED:
    if ( List && Message.GetTarget() == List->GetGuid() )
    {
      CWBButton *b = (CWBButton*)FindChildByID( _T( "relativeres" ), _T( "button" ) );
      CWBTrackBar *x = (CWBTrackBar*)FindChildByID( _T( "xres" ), _T( "trackbar" ) );
      CWBTrackBar *y = (CWBTrackBar*)FindChildByID( _T( "yres" ), _T( "trackbar" ) );
      CWBTrackBar *z = (CWBTrackBar*)FindChildByID( _T( "zres" ), _T( "trackbar" ) );
      CWBItemSelector *pf = (CWBItemSelector*)FindChildByID( _T( "pixelformat" ), _T( "itemselector" ) );

      CphxRenderTarget_Tool *t = GetEditedRT();
      if ( !t ) return true;

      TBOOL Absolute = ( t->ResolutionDescriptor & 0x80 ) == 0;
      TS32 XFact = ( t->ResolutionDescriptor & 0x38 ) >> 3;
      TS32 YFact = ( t->ResolutionDescriptor & 0x07 );

      if ( x ) x->SetValue( XFact );
      if ( y ) y->SetValue( YFact );
      if ( z ) z->SetValue( t->ZResolution );
      if ( b ) b->Push( Absolute );
      if ( pf ) pf->SelectItemByIndex( t->pixelFormat );

      b = (CWBButton*)FindChildByID( _T( "hiddenfromtimeline" ), _T( "button" ) );
      if ( b ) b->Push( t->HiddenFromTimeline );

      b = (CWBButton*)FindChildByID( _T( "cubemap" ), _T( "button" ) );
      if ( b ) b->Push( t->cubeMap );

      return true;
    }
    {
      CWBItemSelector *List2 = (CWBItemSelector*)FindChildByID( _T( "rllist" ), _T( "itemselector" ) );
      if ( List2 && Message.GetTarget() == List2->GetGuid() )
      {
        CWBItemSelector *List3 = (CWBItemSelector*)FindChildByID( _T( "rltlist" ), _T( "itemselector" ) );
        if ( !List3 ) return true;
        List3->Flush();

        CphxRenderLayerDescriptor_Tool *rl = GetEditedRL();
        if ( !rl ) return true;

        for ( TS32 x = 0; x < rl->RenderTargets.NumItems(); x++ )
          List3->AddItem( rl->RenderTargets[ x ]->Name );

        CWBButton *b = (CWBButton*)FindChildByID( _T( "omitdepth" ), _T( "button" ) );
        if ( b ) b->Push( rl->RenderLayer.OmitDepthBuffer );

        b = (CWBButton*)FindChildByID( _T( "voxelizer" ), _T( "button" ) );
        if ( b ) b->Push( rl->RenderLayer.VoxelizerLayer );

        b = (CWBButton*)FindChildByID( _T( "nogrid" ), _T( "button" ) );
        if ( b ) b->Push( rl->RenderLayer.IgnoreHelperObjects );

        b = (CWBButton*)FindChildByID( _T( "isclickable" ), _T( "button" ) );
        if ( b ) b->Push( rl->HasPicking );

        b = (CWBButton*)FindChildByID( _T( "clearrendertargets" ), _T( "button" ) );
        if ( b ) b->Push( rl->RenderLayer.clearRenderTargets );

        return true;
      }

      CWBItemSelector *List3 = (CWBItemSelector*)FindChildByID( _T( "pixelformat" ), _T( "itemselector" ) );
      if ( List3 && Message.GetTarget() == List3->GetGuid() )
      {
        CphxRenderTarget_Tool *t = GetEditedRT();
        if ( !t ) return true;

        if ( t->pixelFormat != ( CphxRenderTarget::PixelFormat ) List3->GetCursorItemID() )
        {
          t->pixelFormat = ( CphxRenderTarget::PixelFormat ) List3->GetCursorItemID();
          t->InvalidateUptoDateFlag();
        }
        return true;
      }
    }
    break;
  case WBM_ITEMRENAMED:
    if ( List && Message.GetTarget() == List->GetGuid() )
    {
      CphxRenderTarget_Tool *t = GetEditedRT();
      if ( t && List->GetCursorItem() ) t->Name = List->GetCursorItem()->GetText();
      return true;
    }
    {
      CWBItemSelector *List2 = (CWBItemSelector*)FindChildByID( _T( "rllist" ), _T( "itemselector" ) );
      if ( List2 && Message.GetTarget() == List2->GetGuid() )
      {
        CphxRenderLayerDescriptor_Tool *t = GetEditedRL();
        if ( t && List2->GetCursorItem() ) t->Name = List2->GetCursorItem()->GetText();
        return true;
      }
    }
    break;
  case WBM_VALUECHANGED:
  {
    CWBTrackBar *x = (CWBTrackBar*)FindChildByID( _T( "xres" ), _T( "trackbar" ) );
    CWBTrackBar *y = (CWBTrackBar*)FindChildByID( _T( "yres" ), _T( "trackbar" ) );
    CWBTrackBar *z = (CWBTrackBar*)FindChildByID( _T( "zres" ), _T( "trackbar" ) );

    CphxRenderTarget_Tool *t = GetEditedRT();
    if ( !t ) return true;

    TBOOL Absolute = ( t->ResolutionDescriptor & 0x80 ) == 0;
    TS32 XFact = ( t->ResolutionDescriptor & 0x38 ) >> 3;
    TS32 YFact = ( t->ResolutionDescriptor & 0x07 );

    if ( x && Message.IsTargetID( _T( "xres" ) ) )
    {
      t->ResolutionDescriptor = ( t->ResolutionDescriptor&~0x38 ) | ( Message.Data << 3 );
      XFact = ( t->ResolutionDescriptor & 0x38 ) >> 3;

      TS32 xr, yr;
      GetRenderTargetResolution( t->ResolutionDescriptor, xr, yr, Project.DemoResolutionX, Project.DemoResolutionY );

      if ( Absolute )
        x->SetText( CString::Format( _T( "X Resolution: %d" ), xr ) );
      else
        x->SetText( CString::Format( _T( "X Resolution: ScreenX / %d" ), 1 << XFact ) );

      //t->Reallocate(App->GetDevice());

      return true;
    }

    if ( y && Message.IsTargetID( _T( "yres" ) ) )
    {
      t->ResolutionDescriptor = ( t->ResolutionDescriptor&~0x07 ) | ( Message.Data );
      YFact = ( t->ResolutionDescriptor & 0x07 );

      TS32 xr, yr;
      GetRenderTargetResolution( t->ResolutionDescriptor, xr, yr, Project.DemoResolutionX, Project.DemoResolutionY );

      if ( Absolute )
        y->SetText( CString::Format( _T( "Y Resolution: %d" ), yr ) );
      else
        y->SetText( CString::Format( _T( "Y Resolution: ScreenY / %d" ), 1 << YFact ) );

      //t->Reallocate(App->GetDevice());

      return true;
    }

    if ( z && Message.IsTargetID( _T( "zres" ) ) )
    {
      t->ZResolution = Message.Data;
      z->SetText( CString::Format( _T( "Z Resolution: %d" ), 1 << t->ZResolution ) );
      return true;
    }

    return true;
  }
  break;
  }

  return CapexWindow::MessageProc( Message );
}

CphxRenderLayerDescriptor_Tool * CapexRenderTargetEditor::GetEditedRL()
{
  CWBItemSelector *List = (CWBItemSelector*)FindChildByID( _T( "rllist" ), _T( "itemselector" ) );
  if ( !List ) return NULL;
  return Project.GetRenderLayerByIndex( List->GetCursorPosition() );
}
