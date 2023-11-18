#include "BasePCH.h"
#include "TexGenMain.h"
#include "../Phoenix_Tool/apxProject.h"
#include "WorkBench.h"
#include "TexGenPreview.h"
#include "TexGenParameters.h"
#define WINDOWNAME _T("Texgen Main")
#define WINDOWXML _T("TexgenMain")

static const int defaultGridSize = 17;

CapexTexGenNewOpWindow::CapexTexGenNewOpWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WorkBench, CapexTexGenMainWindow *owner ) : CapexWindow( Parent, Pos, WorkBench, _T( "" ), _T( "NewOp" ), 0 )
{
  App->ApplyStyle( this );

  Owner = owner;
  for ( TS32 x = 0; x < 10; x++ ) FilterCounters[ x ] = 0;

  NopButton = AddFilter( 0, NULL, _T( "NOP" ) );
  LoadButton = AddFilter( 0, NULL, _T( "Load" ) );
  SaveButton = AddFilter( 0, NULL, _T( "Save" ) );
  SubRoutineButton = AddFilter( 0, NULL, _T( "Subroutine" ) );
  SubRoutineButton->SetDisplayProperty( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xffbe8534 ) );
}

CapexTexGenNewOpWindow::~CapexTexGenNewOpWindow()
{
}

TBOOL CapexTexGenNewOpWindow::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_FOCUSLOST:
    App->SendMessage( CWBMessage( App, WBM_CLOSE, GetGuid() ) );
    break;
  case WBM_LEFTBUTTONDOWN:
    if ( App->GetMouseItem() )
    {
      if ( App->GetMouseItem() == NopButton )
      {
        Owner->CreateNOPOperator();
        Owner->SetCapture();
        return true;
      }

      if ( App->GetMouseItem() == SaveButton )
      {
        Owner->CreateSaveOperator();
        Owner->SetCapture();
        return true;
      }

      if ( App->GetMouseItem() == LoadButton )
      {
        Owner->CreateLoadOperator();
        Owner->SetCapture();
        return true;
      }

      if ( App->GetMouseItem() == SubRoutineButton )
      {
        Owner->CreateSubroutineOperator();
        Owner->SetCapture();
        return true;
      }

      if ( FilterPairing.HasKey( App->GetMouseItem()->GetGuid() ) )
      {
        CphxTextureFilter_Tool *ID = FilterPairing[ App->GetMouseItem()->GetGuid() ];
        Owner->CreateNewOperator( ID );
        Owner->SetCapture();
        return true;
      }

      if ( SubroutinePairing.HasKey( App->GetMouseItem()->GetGuid() ) )
      {
        CphxTextureOperator_Subroutine *sub = SubroutinePairing[ App->GetMouseItem()->GetGuid() ];
        Owner->CreateNewSubroutineCall( sub );
        Owner->SetCapture();
        return true;
      }

    }
    break;
  case WBM_CHAR:
    if ( InFocus() )
    {
      if ( Message.Key == ' ' || Message.Key == VK_ESCAPE )
      {
        App->SendMessage( CWBMessage( App, WBM_CLOSE, GetGuid() ) );
        Owner->SetFocus();
        return true;
      }

      if ( Message.Key == VK_BACK )
      {
        searchfilter.DeleteChar( searchfilter.Length() - 1 );
      }
      else
      {
        if ( !( ( Message.Key >= 'a' && Message.Key <= 'z' ) ||
          ( Message.Key >= 'A' && Message.Key <= 'Z' ) ) )
          return true;
        searchfilter = searchfilter + CString::Format( "%c", tolower( Message.Key ) );
      }

      for ( TS32 x = 0; x < FilterPairing.NumItems(); x++ )
      {
        auto kdp = FilterPairing.GetKDPair( x );
        CWBItem *it = App->FindItemByGuid( kdp->Key );
        CString name = kdp->Data->Name;
        name.ToLower();

        if ( it )
          it->Hide( name.Find( searchfilter ) < 0 );
      }

      return true;
    }
    break;
  }

  return CapexWindow::MessageProc( Message );
}

CWBButton *CapexTexGenNewOpWindow::AddFilter( TS32 Type, CphxTextureFilter_Tool *ID, const CString &Name )
{
  CRect Button = CRect( 0, 0, defaultGridSize * 5 + 1, defaultGridSize );

  CWBButton *b = new CWBButton( this, Button + CPoint( 5, 5 ) + CPoint( Type*( defaultGridSize * 5 + 1 + 5 ), FilterCounters[ Type ] * ( defaultGridSize + 2 ) ), Name.GetPointer() );
  b->ApplyStyleDeclarations( _T( "text-transform:uppercase;" ) );
  if ( ID )
    FilterPairing[ b->GetGuid() ] = ID;

  if ( ID && !ID->External )
    b->SetDisplayProperty( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff094167 ) );

  if ( ID && ID->IsRequired() )
    b->SetDisplayProperty( WB_STATE_NORMAL, WB_ITEM_FONTCOLOR, CColor::FromARGB( 0xffffff00 ) );

  FilterCounters[ Type ]++;

  //reposition window
  TS32 maxx = 0;
  TS32 maxy = 0;
  for ( TS32 x = 0; x < 10; x++ )
  {
    if ( FilterCounters[ x ] ) maxx = x;
    maxy = max( FilterCounters[ x ], maxy );
  }

  CRect pos = CRect( 0, 0, 16 + ( maxx + 1 )*( defaultGridSize * 5 + 1 + 5 ), 16 + maxy * ( defaultGridSize + 2 ) );
  SetPosition( pos - pos.Size() / 2 + GetPosition().Center() );

  return b;
}

void CapexTexGenNewOpWindow::UpdateData()
{

}

CWBButton * CapexTexGenNewOpWindow::AddSubroutine( TS32 Type, CphxTextureOperator_Subroutine *Sub )
{
  CRect Button = CRect( 0, 0, defaultGridSize * 5 + 1, defaultGridSize );

  CWBButton *b = new CWBButton( this, Button + CPoint( 5, 5 ) + CPoint( Type*( defaultGridSize * 5 + 1 + 5 ), FilterCounters[ Type ] * ( defaultGridSize + 2 ) ), Sub->GetName().GetPointer() );

  b->SetDisplayProperty( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xffbe8534 ) );

  //color=CColor::FromARGB(0xff094167);
  //if (BeingGenerated) color=CColor::FromARGB(0xff414141);
  //if (Op->SubroutineRoot) color=CColor::FromARGB(0xffbe8534);

  if ( Sub )
    SubroutinePairing[ b->GetGuid() ] = Sub;

  FilterCounters[ Type ]++;

  //reposition window
  TS32 maxx = 0;
  TS32 maxy = 0;
  for ( TS32 x = 0; x < 10; x++ )
  {
    if ( FilterCounters[ x ] ) maxx = x;
    maxy = max( FilterCounters[ x ], maxy );
  }

  CRect pos = CRect( 0, 0, 16 + ( maxx + 1 )*( defaultGridSize * 5 + 1 + 5 ), 16 + maxy * ( defaultGridSize + 2 ) );
  SetPosition( pos - pos.Size() / 2 + GetPosition().Center() );

  return b;
}

CapexTexGenMainWindow::CapexTexGenMainWindow( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML )
{
  GridSize = 17;
  Panning = false;
  DragMode = dragmode_none;

  ResizedOperator = NULL;
  JustCreatedOperator = NULL;
  EditedOperator = NULL;
  EditBox = NULL;
}

CapexTexGenMainWindow::~CapexTexGenMainWindow()
{

}

TBOOL CapexTexGenMainWindow::MouseOverOp( CphxTextureOperator_Tool *Op )
{
  if ( !Op ) return false;

  CPoint p = ScreenToClient( App->GetMousePos() );
  if ( !GetClientRect().Contains( p ) ) return false;

  CRect dr = GetOpPosition( Op );

  return dr.Contains( p );
}

TS32 SubroutineNameSorter( CphxTextureOperator_Subroutine **a, CphxTextureOperator_Subroutine **b )
{
  return CString::CompareNoCase( ( *a )->Name, ( *b )->Name );
}


void MarkRequiredTextures();

void CapexTexGenMainWindow::OpenNewOpWindow()
{
  // count filter uses
  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool* t = Project.GetTextureFilterByIndex( x );
    t->useCount = 0;
  }

  ClearRequiredFlagForAllResources();

/*
  if (Project.Timeline )
    for ( TS32 x = 0; x < Project.Timeline->Events.NumItems(); x++ )
      Project.Timeline->Events[x]->MarkAsRequired();

  MarkRequiredTextures();

  for (TS32 x=0; x<Project.GetTexgenPageCount(); x++ )
    for ( int y = 0; y < Project.GetTexgenPageByIndex( x )->GetOpCount(); y++ )
    {
      auto* op = Project.GetTexgenPageByIndex( x )->GetOp( y );
      if ( !op->IsRequired() )
        continue;
      if ( op->Filter )
        op->Filter->useCount++;
    }
*/

  //filter uses counted

  CRect r = CRect( App->GetMousePos() - CPoint( 100, 100 ), App->GetMousePos() + CPoint( 100, 100 ) );
  CapexTexGenNewOpWindow *NewOpWindow = new CapexTexGenNewOpWindow( (CWBItem*)WorkBench, r, WorkBench, this );
  NewOpWindow->SetFocus();

  for ( TS32 x = 0; x < Project.GetTextureFilterCount(); x++ )
  {
    CphxTextureFilter_Tool *t = Project.GetTextureFilterByIndex( x );
    NewOpWindow->AddFilter( t->Type + 1, t, !t->IsRequired() ? t->Name : ( t->Name + CString::Format( " (%d)", t->useCount ) ) );
  }

  TS32 type = 0;
  for ( TS32 x = 0; x < 10; x++ )
    if ( NewOpWindow->FilterCounters[ x ] ) type++;

  CArray<CphxTextureOperator_Subroutine*> subs;

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
    for ( TS32 y = 0; y < Project.GetTexgenPageByIndex( x )->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *t = Project.GetTexgenPageByIndex( x )->GetOp( y );
      if ( t->GetOpType() == TEXGEN_OP_SUBROUTINE )
      {
        CphxTextureOperator_Subroutine *s = (CphxTextureOperator_Subroutine*)t;
        subs += s;
      }
    }

  subs.Sort( SubroutineNameSorter );
  for ( TS32 x = 0; x < subs.NumItems(); x++ )
    NewOpWindow->AddSubroutine( type, subs[ x ] );
}

TBOOL CapexTexGenMainWindow::MessageProc( CWBMessage &Message )
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );

  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
  {
    TBOOL res = CapexWindow::MessageProc( Message );
    UpdateScrollbars();
    return res;
  }
  break;

  case WBM_MOUSEWHEEL:
  {
    CPoint pan = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
    CPoint pos = ScreenToClient( App->GetMousePos() ) - pan;
    ZoomToMouseCenter( pan, GridSize, max( 5, min( 17, GridSize + Message.Data ) ), pos );
    SetHScrollbarPos( -pan.x );
    SetVScrollbarPos( -pan.y );
    if ( Project.GetTexgenPage( WorkBench->GetEditedPage() ) )
      pageOffsets[ WorkBench->GetEditedPage() ] = -pan;
    UpdateScrollbars();
    return true;
  }
  break;

  case WBM_COMMAND:
    if ( EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( true );
      return true;
    }
    break;

  case WBM_FOCUSLOST:
    if ( EditBox && Message.GetTarget() == EditBox->GetGuid() )
    {
      StopRenaming( true );
      return true;
    }
    break;

  case WBM_CONTEXTMESSAGE:
  {
    if ( ContextParamLinks.HasKey( Message.Data ) )
    {
      SUBROUTINEPARAMLINK l = ContextParamLinks[ Message.Data ];
      ContextParamLinks.Flush();

      if ( !l.Op->SubroutineRoot ) break;

      CphxSubroutineParam *sparam = l.Op->SubroutineRoot->Parameters[ l.SubParam ];

      TBOOL found = false;

      for ( TS32 x = 0; x < sparam->Targets.NumItems(); x++ )
      {
        if ( sparam->Targets[ x ]->TargetGUID == l.Op->GetGUID() )
          if ( sparam->Targets[ x ]->OpParam == l.OpParam )
          {
            if ( Message.Data < 0 ) //remove
            {
              sparam->Targets.FreeByIndex( x );
              return true;
            }
            found = true;
          }
      }

      if ( !found )
      {
        CphxSubroutineParamConnection *c = new CphxSubroutineParamConnection();
        c->OpParam = l.OpParam;
        c->TargetGUID = l.Op->GetGUID();
        c->TargetID = l.Op->ID;
        sparam->Targets += c;
      }

      return true;
    }
  }
  break;

  case WBM_LEFTBUTTONDOWN:
  {
    if ( App->GetMouseItem() != this ) break;
    if ( !GetClientRect().Contains( ScreenToClient( Message.GetPosition() ) ) ) break;
    if ( !p ) break;

    if ( EditBox )
      StopRenaming( true );

    App->SetCapture( this );
    DragMode = dragmode_multiselect;

    for ( TS32 x = p->GetOpCount() - 1; x >= 0 && DragMode == dragmode_multiselect; x-- )
    {
      CphxTextureOperator_Tool *Op = p->GetOp( x );
      if ( !MouseOverOp( Op ) ) continue;

      p->StoreOpPositions();
      //((wndTextureOperatorParams*)phxWindows[phxWindow_TextureOperatorParams])->SetOperator(Op);

      SetEditedOperator( Op );

      if ( !App->GetCtrlState() && !App->GetShiftState() )
      {
        DragMode = dragmode_operatormove;
        if ( !Op->Selected ) p->ClearOpSelection();
        Op->Selected = true;
      }
      else
      {
        DragMode = dragmode_none;

        if ( !App->GetCtrlState() )
        {
          DragMode = dragmode_operatorresize;
          p->ClearOpSelection();
          ResizedOperator = Op;
          Op->Selected = true;
        }
        else
          Op->Selected = !Op->Selected;
      }

      break;

    }

    if ( DragMode == dragmode_multiselect && !App->GetCtrlState() )
      p->ClearOpSelection();

    return true;
  }
  break;

  case WBM_RIGHTBUTTONDOWN:
  {
    if ( App->GetMouseItem() != this ) break;
    //TS32 x=10;
    //memset(&x,0,4);
    ////memset((void*)x,0,128);
    //int z=10/x;

    if ( !GetClientRect().Contains( ScreenToClient( Message.GetPosition() ) ) ) break;
    if ( !p ) break;
    App->SetCapture( this );

    for ( TS32 x = p->GetOpCount() - 1; x >= 0 && DragMode != dragmode_operatorcopy; x-- )
    {
      CphxTextureOperator_Tool *Op = p->GetOp( x );
      if ( !MouseOverOp( Op ) ) continue;
      if ( !Op->Selected ) continue;

      p->StoreOpPositions();
      DragMode = dragmode_operatorcopy;

      break;
    }

    if ( DragMode == dragmode_operatorcopy )
      CopySelectedOperators();

  }
  break;

  case WBM_LEFTBUTTONDBLCLK:
    if ( App->GetMouseItem() != this ) break;
    if ( CapexWindow::MessageProc( Message ) || !p ) return true;
    for ( TS32 x = 0; x < p->GetOpCount(); x++ )
    {
      if ( MouseOverOp( p->GetOp( x ) ) )
      {
        PreviewOperator( p->GetOp( x )->ID );
        break;
      }
    }
    return true;
    break;

  case WBM_MIDDLEBUTTONDOWN:
  {
    if ( App->GetMouseItem() != this ) break;
    if ( !Project.GetTexgenPage( WorkBench->GetEditedPage() ) ) return true;
    if ( CapexWindow::MessageProc( Message ) ) return true;
    if ( !GetClientRect().Contains( ScreenToClient( App->GetMousePos() ) ) ) break;
    App->SetCapture( this );
    PanStartPos = CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
    Panning = true;
    return true;
  }
  break;

  case WBM_MOUSEMOVE:
    if ( CapexWindow::MessageProc( Message ) ) return true;
    if ( App->GetMouseCaptureItem() == this && Panning )
    {
      CPoint Pan = PanStartPos + ( App->GetMidDownPos() - Message.GetPosition() );
      SetHScrollbarPos( Pan.x );
      SetVScrollbarPos( Pan.y );

      if ( Project.GetTexgenPage( WorkBench->GetEditedPage() ) )
        pageOffsets[ WorkBench->GetEditedPage() ] = Pan;
    }

    if ( !p ) return true;

    switch ( DragMode )
    {
    case dragmode_operatormove:
      for ( int x = 0; x < p->GetOpCount(); x++ )
        if ( p->GetOp( x )->Selected )
          p->GetOp( x )->Position = p->GetOp( x )->OldPosition + ( App->GetMousePos() - App->GetLeftDownPos() ) / GridSize;
      UpdateScrollbars();
      break;
    case dragmode_operatorresize:
      if ( ResizedOperator )
      {
        ResizedOperator->Position = CRect( ResizedOperator->OldPosition.TopLeft(), ResizedOperator->OldPosition.BottomRight() + ( App->GetMousePos() - App->GetLeftDownPos() ) / GridSize );
        ResizedOperator->Position.x2 = max( ResizedOperator->Position.x1 + 5, ResizedOperator->Position.x2 );
        ResizedOperator->Position.y2 = max( ResizedOperator->Position.y1 + 1, ResizedOperator->Position.y2 );
        UpdateScrollbars();
      }
      break;
    case dragmode_operatorcopy:
      for ( int x = 0; x < p->GetOpCount(); x++ )
        if ( p->GetOp( x )->Selected )
          p->GetOp( x )->Position = p->GetOp( x )->OldPosition + ( App->GetMousePos() - App->GetRightDownPos() ) / GridSize;
      break;
    }

    return true;

  case WBM_LEFTBUTTONUP:
    if ( CapexWindow::MessageProc( Message ) ) return true;
    App->ReleaseCapture();
    if ( !p ) return true;

    switch ( DragMode )
    {
    case dragmode_multiselect:
    {
      if ( !App->GetCtrlState() ) p->ClearOpSelection();

      CRect r = ScreenToClient( CRect( App->GetLeftDownPos(), App->GetMousePos() ) ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
      r.Normalize();
      for ( int y = 0; y < p->GetOpCount(); y++ )
        if ( r.Intersects( p->GetOp( y )->Position*GridSize ) ) p->GetOp( y )->Selected = true;
    }
    break;
    case dragmode_operatormove:
    case dragmode_operatorresize:
    { //check for intersections
      ResizedOperator = NULL;
      if ( JustCreatedOperator )
      {
        for ( int x = 0; x < p->GetOpCount(); x++ )
          if ( JustCreatedOperator != p->GetOp( x ) && p->GetOp( x )->Position.Intersects( JustCreatedOperator->Position ) )
          {
            p->DeleteSelected();
            UpdateWindowData( apEx_TextureOpParameters );
            UpdateScrollbars();
            break;
          }
        JustCreatedOperator = NULL;
      }
      else
      {
        bool IntersectDetected = false;
        for ( int x = 0; x < p->GetOpCount() && !IntersectDetected; x++ )
        {
          if ( p->GetOp( x )->Selected )
            for ( int y = 0; y < p->GetOpCount(); y++ )
              if ( x != y )
                IntersectDetected = IntersectDetected || p->GetOp( x )->Position.Intersects( p->GetOp( y )->Position );
        }
        if ( IntersectDetected ) p->RestoreOpPositions();
      }

      p->BuildOperatorConnections();
    }
    break;
    }

    DragMode = dragmode_none;
    break;

  case WBM_MIDDLEBUTTONUP:
    App->ReleaseCapture();
    Panning = false;
    DragMode = dragmode_none;
    return true;

  case WBM_RIGHTBUTTONUP:
    if ( p && DragMode == dragmode_operatorcopy )
    {
      bool IntersectDetected = false;
      for ( int x = 0; x < p->GetOpCount() && !IntersectDetected; x++ )
      {
        if ( p->GetOp( x )->Selected )
          for ( int y = 0; y < p->GetOpCount(); y++ )
            if ( x != y )
              IntersectDetected = IntersectDetected || p->GetOp( x )->Position.Intersects( p->GetOp( y )->Position );
      }
      if ( IntersectDetected )
        p->DeleteSelected();
      else
      {
        for ( int x = 0; x < p->GetOpCount() && !IntersectDetected; x++ )
        {
          if ( p->GetOp( x )->Selected )
          {
            SetEditedOperator( p->GetOp( x ) );
            break;
          }
        }
      }
      p->BuildOperatorConnections();
    }
    DragMode = dragmode_none;

    if ( App->GetRightDownPos() == App->GetMousePos() )
      if ( MouseOver() )
      {
        if ( !p ) break;
        //App->SetCapture(this);

        for ( TS32 x = 0; x < p->GetOpCount(); x++ )
        {
          CphxTextureOperator_Tool *Op = p->GetOp( x );
          if ( !MouseOverOp( Op ) ) continue;

          OpenRightClickMenu( Op );
          return true;
          break;
        }

      }

    App->ReleaseCapture();
    return true;
    break;

  case WBM_KEYDOWN:

    if ( Message.Key == VK_ESCAPE && EditBox )
    {
      StopRenaming( false );
    }

    if ( !EditBox )
    {
      switch ( Message.Key )
      {
      case VK_F2:
        StartRenaming();
        break;
      case VK_DELETE:
        if ( p )
        {
          p->DeleteSelected();
          UpdateWindowData( apEx_TextureOpParameters );
          UpdateScrollbars();
          p->BuildOperatorConnections();
        }
        break;
      default:
        break;
      }
    }
    break;
  case WBM_CHAR:
    if ( !EditBox )
    {
      switch ( Message.Key )
      {
      case ' ':
      {
        if ( p )
          OpenNewOpWindow();
        return true;
      }
      break;
      default:
        break;
      }
    }
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTexGenMainWindow::RestorePageOffset( APEXPAGEID ID )
{
  if ( pageOffsets.HasKey( ID ) )
  {
    CPoint offset = pageOffsets[ ID ];
    SetHScrollbarPos( offset.x );
    SetVScrollbarPos( offset.y );
  }
  else
  {
    SetHScrollbarPos( 0 );
    SetVScrollbarPos( 0 );
    pageOffsets[ ID ] = CPoint( 0, 0 );
  }

  UpdateScrollbars();
}

extern CArray<CphxResource*> UpdateQueue;

void CapexTexGenMainWindow::DisplayOperator( CWBDrawAPI *API, CphxTextureOperator_Tool *Op, TBOOL ForceSelection )
{
  CRect dr = GetOpPosition( Op );
  if ( !dr.Intersects( GetClientRect() ) ) return; //early exit

  CRect cr = API->GetCropRect();

  CColor color = CColor::FromARGB( 0xff686868 );
  if ( Op->Selected || ForceSelection ) color = CColor::FromARGB( 0xfffcd400 );

  //if (Op == EditedOperator) color = CColor::FromARGB(0xff00ff00);

  //draw border
  API->DrawRect( dr, color );

  //color=API->GetColor(GAPI_COLOR_ITEM);
  //if (Op->GetType()!=FLARERESOURCE_TEXTUREOPERATOR) color=0xff503030;

  //if (Op->ContentReady())
  //	color=((color&0x00ff0000)>>16)+((color&0x000000ff)<<16)+color&0xff00ff00;
  //else
  //	color=((color&0x0000ff00)>>8)+((color&0x000000ff)<<8)+color&0xffff0000;

  TBOOL BeingGenerated = !Op->ContentReady();

  color = CColor::FromARGB( 0xff094167 );
  if ( BeingGenerated )
  {
    color = CColor::FromARGB( 0xff414141 );
    if ( Op->GetOpType() == TEXGEN_OP_SAVE )
      color = CColor::FromARGB( 0xff101010 );
  }
  if ( Op->SubroutineRoot ) color = CColor::FromARGB( 0xffbe8534 );

  //check if this operator is being previewed
  //if (Op==((wndTexturePreview*)phxWindows[phxWindow_TexturePreview])->Operator) color=API->GetColor(GAPI_COLOR_SELECTION);

  if ( !Op->SubroutineRoot )
    for ( TS32 x = 0; x < WorkBench->GetWindowCount(); x++ )
      if ( WorkBench->GetWindowByIndex( x )->GetWindowType() == apEx_TexGenPreview )
      {
        CapexTexGenPreview *p = (CapexTexGenPreview *)WorkBench->GetWindowByIndex( x );
        if ( p->GetEditedOperator() == Op->ID )
        {
          color = CColor::FromARGB( 0xff007acc );
          if ( BeingGenerated ) color = CColor::FromARGB( 0xff7a7a7a );
          break;
        }
      }

  if ( Op->IsLocked() ) color = CColor::FromARGB( 0xffff0000 );

  if ( Op->GetOpType() == TEXGEN_OP_SAVE && !BeingGenerated )
  {
    int g = color.G();
    color.G() = (int)( color.B()*0.7 );
    color.B() = g;
  }

  //draw body
  API->SetCropRect( cr | ( ClientToScreen( dr ) + CRect( -1, -1, -1, -1 ) ) );
  API->DrawRect( dr + CRect( -1, -1, -1, -1 ), color );

  //draw invalid input marker
  if ( !Op->InputsValid() )
  {
    API->SetCropRect( cr );
    CColor barcol = CColor::FromARGB( 0xffff0000 );
    if ( Op->SubroutineRoot && Op->GetOpType() != TEXGEN_OP_SUBROUTINE ) barcol = CColor::FromARGB( 0xff00ff00 );
    API->DrawRect( CRect( dr.x1 + GridSize / 2, dr.y1, dr.x2 - GridSize / 2, dr.y1 + 2 ), barcol );
    API->SetCropRect( cr | ( ClientToScreen( dr ) + CRect( -1, -1, -1, -1 ) ) );
  }

  WBITEMSTATE i = GetState();
  CWBFont *OpFont = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );


  if ( OpFont->GetLineHeight() - 3 <= GridSize )
  {
    color = CColor::FromARGB( 0x80ffffff );
    if ( Op == EditedOperator ) color = CColor::FromARGB( 0xffffffff );

    CString N = Op->GetName();
    if ( Op->GetOpType() != TEXGEN_OP_SUBROUTINECALL && Op->Name.Length() )
      N = "\"" + N + "\"";
    OpFont->Write( API, N, OpFont->GetCenter( N, dr, TextTransform ), color, TextTransform );
  }

  //if (Op->GetType()!=FLARERESOURCE_TEXTUREOPERATOR_LOAD)
  //{
  //	if (!Op->Name)
  //		API->WriteTextCenterAlign(dr,Op->GetName());
  //	else
  //	{
  //		char *c=utlBuildString("\"%s\"",Op->GetName());
  //		API->WriteTextCenterAlign(dr,c);
  //		delete[] c;
  //	}
  //}
  //else
  //{
  //	flareTexgenOperatorLOAD *l=(flareTexgenOperatorLOAD*)Op;
  //	if (!l->GetLoadedOp())
  //		API->WriteTextCenterAlign(dr,Op->GetName());
  //	else
  //	{
  //		char *c=utlBuildString("*%s*",l->GetLoadedOp()->GetName());
  //		API->WriteTextCenterAlign(dr,c);
  //		delete[] c;
  //	}
  //}

  API->SetCropRect( cr );
}

void CapexTexGenMainWindow::OnDraw( CWBDrawAPI *API )
{
  CapexWindow::OnDraw( API );

  //API->DrawRect( GetClientRect(), CColor::FromARGB( 0xff1e1e1e ) );
  API->DrawRect( GetClientRect(), CColor::FromARGB( 0xff000000 ) );
  API->DrawRectBorder( GetClientRect(), CColor::FromARGB( 0xff4c656c ) );

  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;

  API->SetCropRect( ClientToScreen( GetClientRect() ) );

  //CColor gridcolor = CColor::FromARGB( 0xff333334 );
  CColor gridcolor = CColor::FromARGB( 0xff010405 );

  CRect r = GetClientRect();
  for ( int x = r.x1 - GetHScrollbarPos() % ( GridSize * 2 ); x < r.x2; x += GridSize * 2 ) API->DrawRect( CRect( x, r.y1, x + 1, r.y2 ), gridcolor );
  for ( int y = r.y1 - GetVScrollbarPos() % ( GridSize * 2 ); y < r.y2; y += GridSize * 2 ) API->DrawRect( CRect( r.x1, y, r.x2, y + 1 ), gridcolor );

  int xm = 0;
  int ym = 0;

  CColor colors[ 3 ] =
  {
    CColor::FromARGB( 0xff35616c ),
    CColor::FromARGB( 0xff02252e ),
    CColor::FromARGB( 0xff010a0c )
  };

  for ( int x = r.x1 - GetHScrollbarPos() % ( GridSize * 2 ); x < r.x2; x += GridSize )
  {
    ym = 0;
    for ( int y = r.y1 - GetVScrollbarPos() % ( GridSize * 2 ); y < r.y2; y += GridSize )
    {
      API->DrawRect( CRect( x, y, x + 1, y + 1 ), colors[ xm + ym ] );
      ym = ( ym + 1 ) % 2;
    }
    xm = ( xm + 1 ) % 2;
  }

  for ( int x = r.x1 - GetHScrollbarPos() % ( GridSize * 6 ); x < r.x2; x += GridSize * 6 )
    for ( int y = r.y1 - GetVScrollbarPos() % ( GridSize * 6 ); y < r.y2; y += GridSize * 6 )
    {
      API->DrawRect( CRect( x - 2, y, x + 3, y + 1 ), CColor::FromARGB( 0xff2f535a ) );
      API->DrawRect( CRect( x, y - 2, x + 1, y + 3 ), CColor::FromARGB( 0xff2f535a ) );
      API->DrawRect( CRect( x, y, x + 1, y + 1 ), CColor::FromARGB( 0xff7ba7b1 ) );
    }

  //multi selector box
  CRect MultiBox = ScreenToClient( CRect( App->GetLeftDownPos(), App->GetMousePos() ) );
  MultiBox.Normalize();
  CPoint offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );

  //draw not selected ops first
  for ( int x = 0; x < p->GetOpCount(); x++ )
    if ( !p->GetOp( x )->Selected )
      if ( DragMode != dragmode_multiselect || !( p->GetOp( x )->Position*GridSize + offset ).Intersects( MultiBox ) )
        DisplayOperator( API, p->GetOp( x ) );

  //selected ops drawn in second pass so dragged items float over others
  for ( int x = 0; x < p->GetOpCount(); x++ )
    if ( p->GetOp( x )->Selected || ( DragMode == dragmode_multiselect && ( p->GetOp( x )->Position*GridSize + offset ).Intersects( MultiBox ) ) )
      DisplayOperator( API, p->GetOp( x ), true );

  ////debug display of connections
  //if (FocusInTree && GUIAPP->Alt)
  //	for (int x=0; x<EditedPage->Operators.ItemCount; x++)
  //		DisplayTexgenOperator(EditedPage->Operators[x],Position.x1,Position.y1);


  if ( DragMode == dragmode_multiselect )
  {
    API->DrawRect( MultiBox, CColor::FromARGB( 0x300956a2 ) );
    API->DrawRectBorder( MultiBox, CColor::FromARGB( 0xff0956a2 ) );
  }

  API->DrawRectBorder( GetClientRect(), CColor::FromARGB( 0xff4c656c ) );

}

void CapexTexGenMainWindow::UpdateScrollbars()
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p || !p->GetOpCount() )
  {
    EnableHScrollbar( false, false );
    EnableVScrollbar( false, false );
    return;
  }

  EnableHScrollbar( true, true );
  EnableVScrollbar( true, true );

  CRect m = p->GetOp( 0 )->Position;
  for ( TS32 x = 1; x < p->GetOpCount(); x++ )
    m &= p->GetOp( x )->Position;

  m = m*GridSize + CRect( 0, 0, 1, 1 );
  SetHScrollbarParameters( m.x1, m.x2, GetClientRect().Width() );
  SetVScrollbarParameters( m.y1, m.y2, GetClientRect().Height() );
}

void CapexTexGenMainWindow::CreateNewOperator( CphxTextureFilter_Tool *Filter )
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;
  CphxTextureOperator_Tool*op = JustCreatedOperator = p->CreateOperator( Filter );
  SetEditedOperator( op );

  CPoint mp = ( ScreenToClient( App->GetMousePos() ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) ) / GridSize;
  op->OldPosition = op->Position = CRect( mp, mp + CPoint( 5, 1 ) );

  p->ClearOpSelection();
  op->Selected = true;
  DragMode = dragmode_operatormove;
  SetFocus();
  PreviewOperator( op->ID );
  UpdateScrollbars();
}

void CapexTexGenMainWindow::PreviewOperator( APEXOPID ID )
{
  CapexTexGenPreview *pv = WorkBench->lastTexgenPreview;
  if ( !pv )
    pv = (CapexTexGenPreview*)WorkBench->GetWindow( apEx_TexGenPreview );

  if ( pv ) pv->SetEditedOperator( ID );
}

void CapexTexGenMainWindow::UpdateData()
{

}

void CapexTexGenMainWindow::SetEditedOperator( CphxTextureOperator_Tool *op )
{
  CapexTextureOpParameters *pw = (CapexTextureOpParameters*)WorkBench->GetWindow( apEx_TextureOpParameters );

  if ( pw )
    pw->BackupSplineUiStateTo( EditedOperator );

  EditedOperator = op;
  if ( !pw ) return;

  pw->SetEditedOperator( op );
}

void CapexTexGenMainWindow::CopySelectedOperators()
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;

  for ( TS32 x = p->GetOpCount() - 1; x >= 0; x-- )
  {
    CphxTextureOperator_Tool *op = p->GetOp( x );

    if ( op->Selected )
    {
      CphxTextureOperator_Tool *newop = op->Copy();
      op->Selected = false;
      newop->Selected = true;
      newop->NeedsInvalidation = true;
      p->AddOperator( newop );
      newop->InvalidateUptoDateFlag();
    }
  }
}

void CapexTexGenMainWindow::CreateSaveOperator()
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;
  CphxTextureOperator_Tool*op = JustCreatedOperator = p->CreateSaveOperator();
  SetEditedOperator( op );

  CPoint mp = ( ScreenToClient( App->GetMousePos() ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) ) / GridSize;
  op->OldPosition = op->Position = CRect( mp, mp + CPoint( 5, 1 ) );

  p->ClearOpSelection();
  op->Selected = true;
  DragMode = dragmode_operatormove;
  SetFocus();
  PreviewOperator( op->ID );
  UpdateScrollbars();
}

void CapexTexGenMainWindow::CreateLoadOperator()
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;
  CphxTextureOperator_Tool*op = JustCreatedOperator = p->CreateLoadOperator();
  SetEditedOperator( op );

  CPoint mp = ( ScreenToClient( App->GetMousePos() ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) ) / GridSize;
  op->OldPosition = op->Position = CRect( mp, mp + CPoint( 5, 1 ) );

  p->ClearOpSelection();
  op->Selected = true;
  DragMode = dragmode_operatormove;
  SetFocus();
  PreviewOperator( op->ID );
  UpdateScrollbars();
}

void CapexTexGenMainWindow::CreateSubroutineOperator()
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;
  CphxTextureOperator_Tool*op = JustCreatedOperator = p->CreateSubroutineOperator();
  SetEditedOperator( op );

  CPoint mp = ( ScreenToClient( App->GetMousePos() ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) ) / GridSize;
  op->OldPosition = op->Position = CRect( mp, mp + CPoint( 5, 1 ) );

  p->ClearOpSelection();
  op->Selected = true;
  DragMode = dragmode_operatormove;
  SetFocus();
  PreviewOperator( op->ID );
  UpdateScrollbars();
}

void CapexTexGenMainWindow::CreateNOPOperator()
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;
  CphxTextureOperator_Tool*op = JustCreatedOperator = p->CreateNOPOperator();
  SetEditedOperator( op );

  CPoint mp = ( ScreenToClient( App->GetMousePos() ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) ) / GridSize;
  op->OldPosition = op->Position = CRect( mp, mp + CPoint( 5, 1 ) );

  p->ClearOpSelection();
  op->Selected = true;
  DragMode = dragmode_operatormove;
  SetFocus();
  PreviewOperator( op->ID );
  UpdateScrollbars();
}

void CapexTexGenMainWindow::CreateNewSubroutineCall( CphxTextureOperator_Subroutine *Sub )
{
  CapexTexGenPage *p = Project.GetTexgenPage( WorkBench->GetEditedPage() );
  if ( !p ) return;
  CphxTextureOperator_Tool*op = JustCreatedOperator = p->CreateSubroutineCall( Sub );
  SetEditedOperator( op );

  CPoint mp = ( ScreenToClient( App->GetMousePos() ) + CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) ) / GridSize;
  op->OldPosition = op->Position = CRect( mp, mp + CPoint( 5, 1 ) );

  p->ClearOpSelection();
  op->Selected = true;
  DragMode = dragmode_operatormove;
  SetFocus();
  PreviewOperator( op->ID );
  UpdateScrollbars();
}

void CapexTexGenMainWindow::CenterOperator( CphxTextureOperator_Tool *Op )
{
  if ( !Op )
    return;
  CPoint c = ( Op->Position*GridSize ).Center();

  CPoint r = GetClientRect().Center();

  SetHScrollbarPos( c.x - r.x );
  SetVScrollbarPos( c.y - r.y );

  if ( Project.GetTexgenPage( WorkBench->GetEditedPage() ) )
    pageOffsets[ WorkBench->GetEditedPage() ] = CPoint( c.x - r.x, c.y - r.y );
}

void CapexTexGenMainWindow::OpenRightClickMenu( CphxTextureOperator_Tool *Op )
{
  if ( !Op ) return;
  if ( !Op->SubroutineRoot ) return;
  if ( Op->SubroutineRoot->GetOpType() != TEXGEN_OP_SUBROUTINE ) return;

  CphxTextureOperator_Subroutine *sub = (CphxTextureOperator_Subroutine*)Op->SubroutineRoot;
  if ( !sub->Parameters.NumItems() ) return;

  //CWBContextMenu *Menu=new CWBContextMenu(App->GetRoot(),CRect(App->GetMousePos(),App->GetMousePos()),GetGuid());
  CWBContextMenu *Menu = OpenContextMenu( App->GetMousePos() );

  TS32 cntr = 0;

  ContextParamLinks.Flush();

  for ( TS32 x = 0; x < sub->Parameters.NumItems(); x++ )
  {
    CWBContextItem *itm = Menu->AddItem( sub->Parameters[ x ]->Name.GetPointer(), cntr++ );
    CphxSubroutineParam *sparam = sub->Parameters[ x ];

    if ( Op->Filter ) //filter
    {
      TBOOL foundrand = false;

      if ( Op->Filter->Filter.DataDescriptor.NeedsRandSeed )
      {
        for ( TS32 z = 0; z < sparam->Targets.NumItems(); z++ )
        {
          if ( sparam->Targets[ z ]->TargetGUID == Op->GetGUID() )
            if ( sparam->Targets[ z ]->OpParam == -1 )
              foundrand = true;
        }

        //if (!foundrand)
        {
          CWBContextItem *itm2;
          if ( !foundrand )
            itm2 = itm->AddItem( _T( "RandSeed" ), cntr );
          else
            itm2 = itm->AddItem( _T( "*RandSeed" ), -cntr );
          SUBROUTINEPARAMLINK l;
          l.Op = Op;
          l.OpParam = -1;
          l.SubParam = x;
          if ( !foundrand )
            ContextParamLinks[ cntr ] = l;
          else
            ContextParamLinks[ -cntr ] = l;
          cntr++;
        }
      }

      for ( TS32 y = 0; y < Op->Filter->Parameters.NumItems(); y++ )
      {

        TBOOL found = false;

        for ( TS32 z = 0; z < sparam->Targets.NumItems(); z++ )
        {
          if ( sparam->Targets[ z ]->TargetGUID == Op->GetGUID() )
            if ( sparam->Targets[ z ]->OpParam == y )
              found = true;
        }

        //if (!found)
        {
          CWBContextItem *itm2;
          if ( !found )
            itm2 = itm->AddItem( Op->Filter->Parameters[ y ]->Name.GetPointer(), cntr );
          else
            itm2 = itm->AddItem( ( _T( "*" ) + Op->Filter->Parameters[ y ]->Name ).GetPointer(), -cntr );

          SUBROUTINEPARAMLINK l;
          l.Op = Op;
          l.OpParam = y;
          l.SubParam = x;
          if ( !found )
            ContextParamLinks[ cntr ] = l;
          else
            ContextParamLinks[ -cntr ] = l;
          cntr++;
        }
      }
    }

    if ( Op->GetOpType() == TEXGEN_OP_SUBROUTINECALL )
    {
      CphxTextureOperator_SubroutineCall *s = (CphxTextureOperator_SubroutineCall*)Op;
      CphxTextureOperator_Subroutine *targetsub = s->Subroutine;
      if ( targetsub )
      {
        for ( TS32 y = 0; y < targetsub->Parameters.NumItems(); y++ )
        {
          CphxSubroutineParam *subparam = sub->Parameters[ x ];

          TBOOL found = false;

          for ( TS32 z = 0; z < subparam->Targets.NumItems(); z++ )
          {
            if ( subparam->Targets[ z ]->TargetGUID == Op->GetGUID() )
              if ( subparam->Targets[ z ]->OpParam == y )
                found = true;
          }

          if ( !found )
          {
            CWBContextItem *itm2 = itm->AddItem( targetsub->Parameters[ y ]->Name.GetPointer(), cntr );
            SUBROUTINEPARAMLINK l;
            l.Op = Op;
            l.OpParam = y;
            l.SubParam = x;
            ContextParamLinks[ cntr ] = l;
            cntr++;
          }
        }

      }
    }

  }

}

void CapexTexGenMainWindow::StartRenaming()
{
  if ( !EditedOperator ) return;
  CphxTextureOperator_Tool *Op = EditedOperator;
  CRect dr = GetOpPosition( Op );
  if ( !dr.Intersects( GetClientRect() ) ) return; //early exit
  if ( EditBox ) SAFEDELETE( EditBox );

  //TS32 ItemHeight=FixItemHeight;
  //CPoint Offset=-CPoint(GetHScrollbarPos(),GetVScrollbarPos());
  //CPoint p=GetClientRect().TopLeft()+Offset;
  //p+=CPoint(0,ItemHeight*CursorPosition);
  //CRect r=CRect(p,CPoint((GetClientRect()+Offset).x2,p.y+ItemHeight));

  CWBTextBox *b = new CWBTextBox( this, dr );
  b->ApplyStyleDeclarations( _T( "border:1px; border-color:#fffcd400; background:#ff414141;" ) );
  App->ApplyStyle( b );

  b->SetText( Op->Name );
  b->SetFocus();
  b->EnableHScrollbar( false, false );
  b->EnableVScrollbar( false, false );
  //b->SetBackgroundColor(rgbBackgroundSelectedFocus);
  b->SetSelection( 0, b->GetText().Length() );

  EditBox = b;

}

void CapexTexGenMainWindow::StopRenaming( TBOOL ApplyChanges )
{
  if ( !EditedOperator ) return;
  if ( !EditBox ) return;

  if ( ApplyChanges )
  {
    CWBTextBox *ib = (CWBTextBox*)EditBox;
    EditedOperator->Name = ib->GetText();
  }

  SAFEDELETE( EditBox );
}

CRect CapexTexGenMainWindow::GetOpPosition( CphxTextureOperator_Tool *Op )
{
  return ( Op->Position*GridSize ) - CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) + CRect( 0, 0, 1, 1 );
}

void CapexTexGenMainWindow::ResetUIData()
{
  pageOffsets.Flush();
}
