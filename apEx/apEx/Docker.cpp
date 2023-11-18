#include "BasePCH.h"
#include "Docker.h"
#include "WorkBench.h"
#include "apExWindow.h"

CRect CapexDocker::GetDragRect()
{
  if ( NumChildren() != 2 )
    return CRect( 0, 0, -1, -1 );

  CRect cl = GetClientRect();
  CPoint p = ScreenToClient( App->GetMousePos() );

  CWBItem *i1 = GetChild( 0 );
  CWBItem *i2 = GetChild( 1 );

  if ( Arrangement == WB_ARRANGE_HORIZONTAL )
    return CRect( i1->GetPosition().x2, cl.y1, i2->GetPosition().x1, cl.y2 );

  if ( Arrangement == WB_ARRANGE_VERTICAL )
    return CRect( cl.x1, i1->GetPosition().y2, cl.x2, i2->GetPosition().y1 );

  return CRect( 0, 0, -1, -1 );
}

void CapexDocker::OnDraw( CWBDrawAPI *API )
{
  CRect dr = GetDragRect();
  CPoint p = ScreenToClient( App->GetMousePos() );

  if ( Arrangement == WB_ARRANGE_HORIZONTAL )
    if ( dr.Contains( p ) || dragging )
    {
      App->SelectMouseCursor( CM_SIZEWE );
      API->DrawRect( dr, 0x80ffffff );
    }

  if ( Arrangement == WB_ARRANGE_VERTICAL )
    if ( dr.Contains( p ) || dragging )
    {
      App->SelectMouseCursor( CM_SIZENS );
      API->DrawRect( dr, 0x80ffffff );
    }

  CWBBox::OnDraw( API );
}

CapexDocker::CapexDocker() : CWBBox()
{
  Arrangement = WB_ARRANGE_HORIZONTAL;
}

CapexDocker::CapexDocker( CWBItem *Parent, const CRect &Pos ) : CWBBox( Parent, Pos )
{
  Arrangement = WB_ARRANGE_HORIZONTAL;
}

CapexDocker::~CapexDocker()
{

}

TBOOL CapexDocker::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
  {
    CRect dr = GetDragRect();
    CPoint p = ScreenToClient( App->GetMousePos() );
    if ( dr.Contains( p ) )
    {
      dragging = true;
      dragStartPos = p;
      dragStartSize = GetChild( 0 )->GetPosition().Size();
    }
  }
  break;
  case WBM_MOUSEMOVE:
    if ( dragging )
    {
      CPoint p = ScreenToClient( App->GetMousePos() );
      CPoint delta = p - dragStartPos;
      CSize clientSize = GetClientRect().Size() - CSize( Spacing, Spacing );

      if ( Arrangement == WB_ARRANGE_HORIZONTAL )
      {
        TS32 width = min( clientSize.x - 50, max( 50, dragStartSize.x + delta.x ) );
        contentSize = width;
        areaSize = clientSize.x;

        GetChild( 0 )->ApplyStyleDeclarations( CString::Format( "width:%f%%;", contentSize / (float)areaSize * 100.0f ) );
        CWBMessage m;
        GetChild( 0 )->BuildPositionMessage( GetChild( 0 )->GetPosition(), m );
        App->SendMessage( m );

        GetChild( 1 )->ApplyStyleDeclarations( "width:none;" );
        GetChild( 1 )->BuildPositionMessage( GetChild( 1 )->GetPosition(), m );
        App->SendMessage( m );
      }

      if ( Arrangement == WB_ARRANGE_VERTICAL )
      {
        TS32 height = min( clientSize.y - 50, max( 50, dragStartSize.y + delta.y ) );
        contentSize = height;
        areaSize = clientSize.y;

        GetChild( 0 )->ApplyStyleDeclarations( CString::Format( "height:%f%%;", contentSize / (float)areaSize * 100.0f ) );
        CWBMessage m;
        GetChild( 0 )->BuildPositionMessage( GetChild( 0 )->GetPosition(), m );
        App->SendMessage( m );

        GetChild( 1 )->ApplyStyleDeclarations( "height:none;" );
        GetChild( 1 )->BuildPositionMessage( GetChild( 1 )->GetPosition(), m );
        App->SendMessage( m );
      }

    }
    break;
  case WBM_LEFTBUTTONUP:
  {
    dragging = false;
  }
  break;
  }

  return CWBBox::MessageProc( Message );
}

void CapexDocker::DockWindow( CWBItem* item, DockPosition dockPosition )
{
  CapexDocker* DockRoot = this;

  if ( item->InstanceOf( "apexwindow" ) )
    ( (CapexWindow*)item )->WindowHasDocked();

  if ( DockRoot->NumChildren() == 0 )
  {
    item->SetParent( DockRoot );
    DockRoot->ApplyStyleDeclarations( "margin:0px; child-layout: vertical; child-fill-x: true; child-fill-y: true; child-spacing:5px;" );
    CWBMessage m;
    DockRoot->BuildPositionMessage( DockRoot->GetClientRect(), m );
    App->SendMessage( m );
    return;
  }

  if ( DockRoot->NumChildren() >= 2 )
  {
    TS32 parentIdx = -1;

    for ( TU32 x = 0; x < GetParent()->NumChildren(); x++ )
      if ( GetParent()->GetChild( x ) == this )
      {
        parentIdx = x;
        break;
      }

    if ( parentIdx == -1 )
      LOG_ERR( "HOLY FUCK there's a huge problem with the GUI" );

    CapexDocker *d = new CapexDocker( GetParent(), GetClientRect() );
    DockRoot->SetParent( d );

    if ( parentIdx != 0 )
      d->SetTopmost();
    else
      d->SetBottommost();

    DockRoot = d;
    //DockRoot->SetBottommost();
  }

  item->SetParent( DockRoot );

  if ( dockPosition == DockPosition::Left || dockPosition == DockPosition::Top )
    item->SetBottommost();

  //App->ApplyStyle( DockRoot );

  if ( dockPosition == DockPosition::Left || dockPosition == DockPosition::Right )
    DockRoot->ApplyStyleDeclarations( "child-layout: horizontal;" );
  else
    DockRoot->ApplyStyleDeclarations( "child-layout: vertical;" );

  CWBMessage m;
  DockRoot->BuildPositionMessage( DockRoot->GetClientRect(), m );
  App->SendMessage( m );
}

void CapexDocker::DockWindowToChild( CWBItem* childItem, CWBItem* item, DockPosition dockPosition )
{
  auto DockRoot = this;

  if ( item->InstanceOf( "apexwindow" ) )
    ( (CapexWindow*)item )->WindowHasDocked();

  if ( DockRoot->NumChildren() == 0 )
  {
    LOG_ERR( "This should never happen - non root docker had 0 children" );
    item->SetParent( DockRoot );
    DockRoot->ApplyStyleDeclarations( "margin:0px; child-layout: vertical; child-fill-x: true; child-fill-y: true; child-spacing:5px;" );
    CWBMessage m;
    DockRoot->BuildPositionMessage( DockRoot->GetClientRect(), m );
    App->SendMessage( m );
    return;
  }

  if ( DockRoot->NumChildren() >= 2 )
  {
    TS32 childIdx = -1;

    for ( TU32 x = 0; x < NumChildren(); x++ )
      if ( GetChild( x ) == childItem )
      {
        childIdx = x;
        break;
      }

    if ( childIdx == -1 )
      LOG_ERR( "HOLY FUCK there's a huge problem with the GUI" );

    CapexDocker *d = new CapexDocker( this, GetClientRect() );
    childItem->SetParent( d );

    if ( childIdx != 0 )
      d->SetTopmost();
    else
      d->SetBottommost();

    DockRoot = d;
  }

  item->SetParent( DockRoot );

  if ( dockPosition == DockPosition::Left || dockPosition == DockPosition::Top )
    item->SetBottommost();

  //App->ApplyStyle( DockRoot );

  if ( dockPosition == DockPosition::Left || dockPosition == DockPosition::Right )
    DockRoot->ApplyStyleDeclarations( "child-layout: horizontal;" );
  else
    DockRoot->ApplyStyleDeclarations( "child-layout: vertical;" );

  CWBMessage m;
  DockRoot->BuildPositionMessage( DockRoot->GetClientRect(), m );
  App->SendMessage( m );

}

TBOOL HasDockerChildren( CWBItem* i )
{
  for ( unsigned int x = 0; x < i->NumChildren(); x++ )
  {
    if ( i->GetChild( x )->InstanceOf( "apexdocker" ) )
      return true;
  }

  for ( unsigned int x = 0; x < i->NumChildren(); x++ )
  {
    if ( HasDockerChildren( i->GetChild( x ) ) )
      return true;
  }

  return false;
}

void CapexDocker::OnPostDraw( CWBDrawAPI* API )
{
  if ( App->GetMousePos() == App->GetLeftDownPos() )
    return;

  if ( !App->GetMouseCaptureItem() )
    return;

  if ( !App->GetMouseCaptureItem()->InstanceOf( "window" ) )
    return;

  if ( !GetScreenRect().Contains( App->GetMousePos() ) )
    return;

  if ( HasDockerChildren( this ) )
    return;

  CWBWindow* wnd = (CWBWindow*)App->GetMouseCaptureItem();
  if ( wnd->GetDragMode() != WB_DRAGMODE_MOVE )
    return;

  if ( GetParent() == GetActiveWorkBench() )
    return;

  CRect cl = GetClientRect();

  CRect left = CRect( 0, cl.Center().y - 25, 50, cl.Center().y + 25 ) + CPoint( 50, 0 );
  CRect right = CRect( cl.x2 - 50, cl.Center().y - 25, cl.x2, cl.Center().y + 25 ) - CPoint( 50, 0 );

  CRect top = CRect( cl.Center().x - 25, 0, cl.Center().x + 25, 50 ) + CPoint( 0, 50 );
  CRect bottom = CRect( cl.Center().x - 25, cl.y2 - 50, cl.Center().x + 25, cl.y2 ) - CPoint( 0, 50 );

  CPoint p = ScreenToClient( App->GetMousePos() );

  WindowDockDisplayInfo dleft( ClientToScreen( left.Center() ), ClientToScreen( CRect( 0, 0, cl.Center().x, cl.y2 ) ), DockPosition::Left, App->GetMouseCaptureItem(), this, nullptr );
  WindowDockDisplayInfo dright( ClientToScreen( right.Center() ), ClientToScreen( CRect( cl.Center().x, 0, cl.x2, cl.y2 ) ), DockPosition::Right, App->GetMouseCaptureItem(), this, nullptr );
  WindowDockDisplayInfo dtop( ClientToScreen( top.Center() ), ClientToScreen( CRect( 0, 0, cl.x2, cl.Center().y ) ), DockPosition::Top, App->GetMouseCaptureItem(), this, nullptr );
  WindowDockDisplayInfo dbottom( ClientToScreen( bottom.Center() ), ClientToScreen( CRect( 0, cl.Center().y, cl.x2, cl.y2 ) ), DockPosition::Bottom, App->GetMouseCaptureItem(), this, nullptr );

  GetActiveWorkBench()->dockerItems += dleft;
  GetActiveWorkBench()->dockerItems += dright;
  GetActiveWorkBench()->dockerItems += dtop;
  GetActiveWorkBench()->dockerItems += dbottom;

  if ( left.Contains( p ) )
    GetActiveWorkBench()->activeItem = dleft;

  if ( right.Contains( p ) )
    GetActiveWorkBench()->activeItem = dright;

  if ( top.Contains( p ) )
    GetActiveWorkBench()->activeItem = dtop;

  if ( bottom.Contains( p ) )
    GetActiveWorkBench()->activeItem = dbottom;
}

TBOOL CapexDocker::ApplyStyle( CString & prop, CString & value, CStringArray & pseudo )
{
  return CWBBox::ApplyStyle( prop, value, pseudo );
}

void CapexDocker::ApplyAutoScaleStyles()
{
  if ( Arrangement == WB_ARRANGE_HORIZONTAL )
  {
    GetChild( 0 )->ApplyStyleDeclarations( CString::Format( "width:%f%%;", contentSize / (float)areaSize * 100.0f ) );
    //CWBMessage m;
    //GetChild( 0 )->BuildPositionMessage( GetChild( 0 )->GetPosition(), m );
    //App->SendMessage( m );

    GetChild( 1 )->ApplyStyleDeclarations( "width:none;" );
    //GetChild( 1 )->BuildPositionMessage( GetChild( 1 )->GetPosition(), m );
    //App->SendMessage( m );
  }

  if ( Arrangement == WB_ARRANGE_VERTICAL )
  {
    GetChild( 0 )->ApplyStyleDeclarations( CString::Format( "height:%f%%;", contentSize / (float)areaSize * 100.0f ) );
    //CWBMessage m;
    //GetChild( 0 )->BuildPositionMessage( GetChild( 0 )->GetPosition(), m );
    //App->SendMessage( m );

    GetChild( 1 )->ApplyStyleDeclarations( "height:none;" );
    //GetChild( 1 )->BuildPositionMessage( GetChild( 1 )->GetPosition(), m );
    //App->SendMessage( m );
  }
}
