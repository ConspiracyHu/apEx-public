#include "BasePCH.h"
#include "WorkBench.h"
#include "MaterialEditor.h"
#include "MaterialList.h"
#include "MaterialShaderEditor.h"
#include "ModelGraph.h"
#include "ModelList.h"
#include "ModelMaterial.h"
#include "ModelParameters.h"
#include "ModelPrimitives.h"
#include "ModelView.h"
#include "ModelMatrix.h"
#include "ProjectSettings.h"
#include "SceneClips.h"
#include "SceneGraph.h"
#include "SceneList.h"
#include "SceneObjectParameters.h"
#include "ScenePrimitives.h"
#include "SceneSplineEditor.h"
#include "SceneView.h"
#include "TexGenFilterEditor.h"
#include "TexGenMain.h"
#include "TexGenPages.h"
#include "TexGenParameters.h"
#include "TexGenPreview.h"
#include "TimelineEditor.h"
#include "TimelineEventParameters.h"
#include "TimelineEventSplines.h"
#include "TimelinePreview.h"
#include "TreeSpeciesEditor.h"
#include "apExHelp.h"
#include "Console.h"
#include "UIXMLEditor.h"
#include "UICSSEditor.h"
#include "RenderTargetEditor.h"
#include "KKPViewer.h"

CapexWorkBench::CapexWorkBench( CWBItem *Parent, const CRect &Position, const TCHAR *name ) : CWBItem( Parent, Position )
{
  SnapDistance = 7;
  Name = name;
  EditedTexturePage = -1;
  ModelPreviewScene = new CphxScene_Tool();
  EditedModel = NULL;
  EditedScene = NULL;
  EditedObj = NULL;
  EditedFilter = NULL;
  PreviewObject = NULL;
  PreviewLight = NULL;
  DockRoot = nullptr;// new CapexDocker( this, GetClientRect() );
}

CapexWorkBench::~CapexWorkBench()
{
  SAFEDELETE( DockRoot );

  if ( PreviewObject )
  {
    SAFEDELETE( PreviewObject->Clips[ 0 ]->MaterialSplines );
    SAFEDELETE( PreviewObject->Clips[ 0 ] );
    SAFEDELETEA( PreviewObject->Clips );
  }
  if ( PreviewLight )
  {
    SAFEDELETE( PreviewLight->Clips[ 0 ]->MaterialSplines );
    SAFEDELETE( PreviewLight->Clips[ 0 ] );
    SAFEDELETEA( PreviewLight->Clips );
  }

  SimulateFreeArray( ModelPreviewScene->Scene.Objects, ModelPreviewScene->Scene.ObjectCount );
  //ModelPreviewScene->Scene.Objects.FreeArray();
  SAFEDELETE( ModelPreviewScene );
}

void CapexWorkBench::SnapRect( CRect &o, CRect ref, TBOOL Move, TBOOL Sides[ 4 ] )
{
  if ( ( Move || Sides[ 0 ] ) && abs( ref.x1 - o.x1 ) < SnapDistance ) { if ( Move ) o.x2 = ref.x1 + o.Width(); o.x1 = ref.x1; }
  if ( ( Move || Sides[ 0 ] ) && abs( ref.x2 - o.x1 ) < SnapDistance ) { if ( Move ) o.x2 = ref.x2 + o.Width(); o.x1 = ref.x2; }
  if ( ( Move || Sides[ 1 ] ) && abs( ref.x1 - o.x2 ) < SnapDistance ) { if ( Move ) o.x1 = ref.x1 - o.Width(); o.x2 = ref.x1; }
  if ( ( Move || Sides[ 1 ] ) && abs( ref.x2 - o.x2 ) < SnapDistance ) { if ( Move ) o.x1 = ref.x2 - o.Width(); o.x2 = ref.x2; }
  if ( ( Move || Sides[ 2 ] ) && abs( ref.y1 - o.y1 ) < SnapDistance ) { if ( Move ) o.y2 = ref.y1 + o.Height(); o.y1 = ref.y1; }
  if ( ( Move || Sides[ 2 ] ) && abs( ref.y2 - o.y1 ) < SnapDistance ) { if ( Move ) o.y2 = ref.y2 + o.Height(); o.y1 = ref.y2; }
  if ( ( Move || Sides[ 3 ] ) && abs( ref.y1 - o.y2 ) < SnapDistance ) { if ( Move ) o.y1 = ref.y1 - o.Height(); o.y2 = ref.y1; }
  if ( ( Move || Sides[ 3 ] ) && abs( ref.y2 - o.y2 ) < SnapDistance ) { if ( Move ) o.y1 = ref.y2 - o.Height(); o.y2 = ref.y2; }
}

TBOOL CapexWorkBench::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_CONTEXTMESSAGE:
    if ( Message.GetTarget() == GetGuid() )
    {
      OpenWindow( (APEXWINDOW)Message.Data );
      return true;
    }
    break;
  case WBM_LEFTBUTTONDOWN:
  case WBM_RIGHTBUTTONDOWN:
  case WBM_MIDDLEBUTTONDOWN:

    if ( !App->GetMouseItem() || App->GetMouseItem() == this ) break;

    //a window has been clicked, position it on top
    for ( TU32 x = 0; x < NumChildren(); x++ )
      if ( GetChild( x )->InstanceOf( "window" ) && !GetChild( x )->IsHidden() && App->GetMouseItem()->FindItemInParentTree( GetChild( x ) ) )
      {
        SetChildAsTopmost( x );
        break;
      }

    break;

  case WBM_WINDOWDRAGSTOPPED:
    if ( activeItem.isSet )
    {
      if ( !activeItem.targetDockItem )
        if ( !activeItem.targetWindowItem )
          DockWindow( activeItem.windowToDock, activeItem.dockTarget );
        else
        {
          CWBItem *dock = activeItem.targetWindowItem->GetParent();
          if ( dock->InstanceOf( "apexdocker" ) )
          {
            CapexDocker* docker = (CapexDocker*)dock;
            docker->DockWindowToChild( activeItem.targetWindowItem, activeItem.windowToDock, activeItem.dockTarget );
          }

        }
      else
        activeItem.targetDockItem->DockWindow( activeItem.windowToDock, activeItem.dockTarget );
      activeItem.windowToDock = nullptr;
      activeItem.targetDockItem = nullptr;
      activeItem.targetWindowItem = nullptr;
    }
    break;

  case WBM_CHAR:
  {
    if ( Message.Key == 'r' || Message.Key == 'R' )
    {
      if ( EditedScene )
        EditedScene->ResetParticles();
      return true;
    }
  }
  break;

  case WBM_REPOSITION:
  {
    CWBItem *t = App->FindItemByGuid( Message.GetTarget() );
    if ( !( t && t->GetParent() == this ) ) break;

    if ( !App->GetMouseCaptureItem() ) break;

    CRect r = t->GetPosition();
    CRect c = GetClientRect();

    CRect p = Message.Rectangle;

    TBOOL SidesMoved[ 4 ];
    SidesMoved[ 0 ] = r.x1 != Message.Rectangle.x1;
    SidesMoved[ 1 ] = r.x2 != Message.Rectangle.x2;
    SidesMoved[ 2 ] = r.y1 != Message.Rectangle.y1;
    SidesMoved[ 3 ] = r.y2 != Message.Rectangle.y2;

    if ( !Message.Resized ) //window is only moved so focus on that
    {
      //perform snapping
      for ( TU32 x = 0; x < NumChildren(); x++ )
        if ( !GetChild( x )->IsHidden() && GetChild( x ) != t )
          SnapRect( p, GetChild( x )->GetPosition(), true, SidesMoved );

      //snap to client rect
      SnapRect( p, GetClientRect(), true, SidesMoved );

      //avoid moving outside of client rect
      if ( p.x1 < 0 ) p -= CPoint( p.x1, 0 );
      if ( p.y1 < 0 ) p -= CPoint( 0, p.y1 );
      if ( p.x2 > c.x2 ) p -= CPoint( p.x2 - c.x2, 0 );
      if ( p.y2 > c.y2 ) p -= CPoint( 0, p.y2 - c.y2 );

      Message.Rectangle = p;

      break;
    }

    //perform snapping
    for ( TU32 x = 0; x < NumChildren(); x++ )
      if ( !GetChild( x )->IsHidden() && GetChild( x ) != t )
        SnapRect( p, GetChild( x )->GetPosition(), false, SidesMoved );

    //snap to client rect
    SnapRect( p, GetClientRect(), false, SidesMoved );

    //Crop the result to the client rectangle
    Message.Rectangle = p | c;

    break;
  }
  break;

  default:
    return CWBItem::MessageProc( Message );
  }

  return CWBItem::MessageProc( Message );
}

void CapexWorkBench::WindowBeingDeleted( CapexWindow *w )
{
  //LOG(LOG_INFO,_T("%s Window closed in %s"),w->GetTitle().GetPointer(),Name.GetPointer());
  Windows.Delete( w );
}

CapexWindow * GetWindowByIndex( CWBItem *r, APEXWINDOW w, TS32 c, TS32& cnt )
{
  if ( r && r->InstanceOf( "apexwindow" ) )
  {
    CapexWindow* wnd = (CapexWindow*)r;
    if ( wnd->GetWindowType() == w )
    {
      if ( cnt == c ) return wnd;
      cnt++;
      return nullptr;
    }
  }

  if ( !r )
    return nullptr;

  for ( TU32 x = 0; x < r->NumChildren(); x++ )
  {
    auto res = GetWindowByIndex( r->GetChild( x ), w, c, cnt );
    if ( res )
      return res;
  }

  return nullptr;
}

CapexWindow *CapexWorkBench::GetWindow( APEXWINDOW w, TS32 id )
{
  TS32 cnt = 0;

  for ( TS32 x = NumChildren() - 1; x >= 0; x-- )
  {
    CWBItem *i = GetChild( x );
    if ( i->InstanceOf( _T( "apexwindow" ) ) )
    {
      CapexWindow *win = (CapexWindow*)i;
      if ( win->GetWindowType() == w )
      {
        if ( cnt == id ) return win;
        cnt++;
      }
    }
  }

  return ::GetWindowByIndex( DockRoot, w, id, cnt );
}

TBOOL CapexWorkBench::IsUniqueWindow( APEXWINDOW w )
{
  if ( w == apEx_TexGenPreview || w == apEx_ModelView || w == apEx_SceneView ) return false;
  return true;
}

CapexWindow *CapexWorkBench::OpenWindow( APEXWINDOW w )
{
  CapexWindow *win = GetWindow( w );
  if ( !win || !IsUniqueWindow( w ) )
  {
    switch ( w )
    {
    case apEx_MaterialEditor:			      win = new CapexMaterialEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_MaterialList:				      win = new CapexMaterialList( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_MaterialShaderEditor:		  win = new CapexMaterialShaderEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelGraph:				        win = new CapexModelGraph( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelList:				        win = new CapexModelList( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelMaterial:			      win = new CapexModelMaterial( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelParameters:			    win = new CapexModelParameters( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelPrimitives:			    win = new CapexModelPrimitives( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelView:				        win = new CapexModelView( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ModelMatrix:		          win = new CapexModelMatrix( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ProjectSettings:			    win = new CapexProjectSettings( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_SceneClips:				        win = new CapexSceneClips( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_SceneGraph:				        win = new CapexSceneGraph( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_SceneList:				        win = new CapexSceneList( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_SceneObjectParameters:	  win = new CapexSceneObjectParameters( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_ScenePrimitives:			    win = new CapexScenePrimitives( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_SceneSplineEditor:		    win = new CapexSceneSplineEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_SceneView:				        win = new CapexSceneView( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TexGenFilterEditor:		    win = new CapexTexGenFilterEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TexGenMain:				        win = new CapexTexGenMainWindow( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TexGenPages:				      win = new CapexTexGenPages( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TextureOpParameters:		  win = new CapexTextureOpParameters( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TexGenPreview:			      win = new CapexTexGenPreview( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TimelineEditor:			      win = new CapexTimelineEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TimelineEventParameters:	win = new CapexTimelineEventParameters( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TimelineEventSplines:		  win = new CapexTimelineEventSplines( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TimelinePreview:			    win = new CapexTimelinePreview( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_TreeSpeciesEditor:		    win = new CapexTreeSpeciesEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_Help:						          win = new CapexHelp( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_Console:					        win = new CapexConsoleWindow( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_XMLEditor:				        win = new CapexUIXMLEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_CSSEditor:				        win = new CapexUICSSEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_RenderTargetEditor:		    win = new CapexRenderTargetEditor( this, CRect( 10, 10, 200, 200 ), this ); break;
    case apEx_KKPViewer:		            win = new CKKPViewer( this, CRect( 10, 10, 200, 200 ), this ); break;
    default:
      break;
    }

    if ( win ) Windows += win;
    if ( win ) win->UpdateData();
    //if (win) LOG(LOG_INFO,_T("[apex] %s Window opened in %s"),win->GetTitle().GetPointer(),Name.GetPointer());

  }

  TS32 idx = GetChildIndex( win );
  if ( idx >= 0 ) SetChildAsTopmost( idx );
  if ( win )
  {
    win->SetPosition( win->GetPosition() + CRect( 1, 1, 1, 1 ) );
    win->SetFocus();
  }

  return win;
}

CapexWindow * CapexWorkBench::OpenWindow( CXMLNode *w )
{
  if ( !w->HasAttribute( _T( "type" ) ) ) return NULL;
  TS32 type = 0;
  w->GetAttributeAsInteger( _T( "type" ), &type );
  CRect p;
  if ( w->HasAttribute( _T( "x1" ) ) ) w->GetAttributeAsInteger( _T( "x1" ), &p.x1 );
  if ( w->HasAttribute( _T( "y1" ) ) ) w->GetAttributeAsInteger( _T( "y1" ), &p.y1 );
  if ( w->HasAttribute( _T( "x2" ) ) ) w->GetAttributeAsInteger( _T( "x2" ), &p.x2 );
  if ( w->HasAttribute( _T( "y2" ) ) ) w->GetAttributeAsInteger( _T( "y2" ), &p.y2 );

  CapexWindow *win = OpenWindow( (APEXWINDOW)type );
  if ( !win ) return NULL;
  win->SetPosition( p );
  win->ImportConfig( w, p );
  return win;
}

void CapexWorkBench::SetEditedPage( APEXPAGEID ID )
{
  EditedTexturePage = ID;
  CapexTexGenMainWindow* txMain = (CapexTexGenMainWindow*)GetWindow( apEx_TexGenMain );
  if ( txMain )
    txMain->RestorePageOffset( ID );
}

void CapexWorkBench::OnDraw( CWBDrawAPI *API )
{
  CWBItem::OnDraw( API );
  activeItem.windowToDock = nullptr;
  activeItem.targetDockItem = nullptr;
  activeItem.targetWindowItem = nullptr;
  activeItem.isSet = false;
  dockerItems.FlushFast();
}

void CapexWorkBench::OnPostDraw( CWBDrawAPI* API )
{
  if ( App->GetMousePos() == App->GetLeftDownPos() )
    return;

  if ( !App->GetMouseCaptureItem() )
    return;

  if ( !App->GetMouseCaptureItem()->InstanceOf( "window" ) )
    return;

  CWBWindow* wnd = (CWBWindow*)App->GetMouseCaptureItem();
  if ( wnd->GetDragMode() != WB_DRAGMODE_MOVE )
    return;

  CRect cl = GetClientRect();

  CRect left = CRect( 0, cl.Center().y - 25, 50, cl.Center().y + 25 );
  CRect right = CRect( cl.x2 - 50, cl.Center().y - 25, cl.x2, cl.Center().y + 25 );

  CRect top = CRect( cl.Center().x - 25, 0, cl.Center().x + 25, 50 );
  CRect bottom = CRect( cl.Center().x - 25, cl.y2 - 50, cl.Center().x + 25, cl.y2 );

  CPoint p = ScreenToClient( App->GetMousePos() );

  WindowDockDisplayInfo dleft( ClientToScreen( left.Center() ), ClientToScreen( CRect( 0, 0, cl.Center().x, cl.y2 ) ), DockPosition::Left, App->GetMouseCaptureItem(), nullptr, nullptr );
  WindowDockDisplayInfo dright( ClientToScreen( right.Center() ), ClientToScreen( CRect( cl.Center().x, 0, cl.x2, cl.y2 ) ), DockPosition::Right, App->GetMouseCaptureItem(), nullptr, nullptr );
  WindowDockDisplayInfo dtop( ClientToScreen( top.Center() ), ClientToScreen( CRect( 0, 0, cl.x2, cl.Center().y ) ), DockPosition::Top, App->GetMouseCaptureItem(), nullptr, nullptr );
  WindowDockDisplayInfo dbottom( ClientToScreen( bottom.Center() ), ClientToScreen( CRect( 0, cl.Center().y, cl.x2, cl.y2 ) ), DockPosition::Bottom, App->GetMouseCaptureItem(), nullptr, nullptr );

  GetActiveWorkBench()->dockerItems += dleft;
  GetActiveWorkBench()->dockerItems += dright;
  GetActiveWorkBench()->dockerItems += dtop;
  GetActiveWorkBench()->dockerItems += dbottom;

  if ( left.Contains( p ) )
    activeItem = dleft;

  if ( right.Contains( p ) )
    activeItem = dright;

  if ( top.Contains( p ) )
    activeItem = dtop;

  if ( bottom.Contains( p ) )
    activeItem = dbottom;

  if ( activeItem.isSet )
  {
    API->DrawRect( ScreenToClient( activeItem.highlightArea ), 0x20ffffff );
    CPoint cp = ScreenToClient( activeItem.displayCenter );
    CRect r = CRect( cp - CPoint( 25, 25 ), cp + CPoint( 25, 25 ) );
    API->DrawRect( r, 0xffffffff );
  }

  for ( TS32 x = 0; x < dockerItems.NumItems(); x++ )
  {
    if ( dockerItems[ x ] == activeItem )
      continue;
    CPoint cp = ScreenToClient( dockerItems[ x ].displayCenter );
    CRect r = CRect( cp - CPoint( 25, 25 ), cp + CPoint( 25, 25 ) );

    API->DrawRect( r, 0x80ffffff );
  }
}

void CapexWorkBench::DockWindow( CWBItem* item, DockPosition dockPosition )
{
  if ( item->InstanceOf( "apexwindow" ) )
    ( (CapexWindow*)item )->WindowHasDocked();

  if ( !DockRoot || DockRoot->NumChildren() == 0 )
  {
    if ( !DockRoot )
    {
      DockRoot = new CapexDocker( this, GetClientRect() );
      DockRoot->SetBottommost();
    }

    item->SetParent( DockRoot );
    DockRoot->ApplyStyleDeclarations( "margin:0px; child-layout: vertical;" );
    CWBMessage m;
    DockRoot->BuildPositionMessage( DockRoot->GetClientRect(), m );
    App->SendMessage( m );
    return;
  }

  if ( DockRoot->NumChildren() >= 2 )
  {
    CapexDocker *d = new CapexDocker( this, GetClientRect() );
    DockRoot->SetParent( d );
    DockRoot = d;
    DockRoot->SetBottommost();
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

void CapexWorkBench::UndockWindow( CapexWindow* wnd )
{
  if ( !wnd->GetParent()->InstanceOf( "apexdocker" ) )
    return;

  CWBItem* dock = wnd->GetParent();

  if ( dock->NumChildren() != 2 && dock != DockRoot )
    LOG_ERR( "Docker with not 2 children found!" );

  CPoint osave = wnd->GetSavedPosition().TopLeft();
  CPoint diff = wnd->GetParent()->ClientToScreen( osave ) - ClientToScreen( osave );
  wnd->SetSavedPosition( wnd->GetSavedPosition() + diff );

  wnd->SetParent( this );
  CWBMessage m;
  wnd->BuildPositionMessage( wnd->GetPosition() + diff, m );
  App->SendMessage( m );

  dock->BuildPositionMessage( dock->GetPosition(), m );
  App->SendMessage( m );

  if ( dock != DockRoot )
  {
    CWBItem *child = dock->GetChild( 0 );
    CWBItem *parent = dock->GetParent();

    bool needsMove = parent->GetChild( 0 ) == dock;

    dock->MarkForDeletion( true );
    child->SetParent( parent );

    if ( needsMove )
      child->SetBottommost();

    CWBMessage m;
    parent->BuildPositionMessage( parent->GetPosition(), m );
    App->SendMessage( m );
  }

  wnd->ApplyStyleDeclarations( "width:none;height:none;" );
  wnd->BuildPositionMessage( wnd->GetPosition(), m );
  App->SendMessage( m );
}

void CapexWorkBench::ExportLayout( CXMLNode *node )
{
  if ( !node ) return;
  node->SetAttribute( _T( "Name" ), Name.GetPointer() );

  if ( DockRoot && DockRoot->NumChildren() )
    ExportDocker( node, DockRoot );

  for ( TS32 x = 0; x < Windows.NumItems(); x++ )
  {
    if ( Windows[ x ]->GetParent() == this )
    {
      CXMLNode w = node->AddChild( _T( "window" ), x == Windows.NumItems() - 1 );
      Windows[ x ]->ExportWindow( &w );
    }
  }
}

void CapexWorkBench::ExportDocker( CXMLNode *node, CapexDocker* docker )
{
  if ( !docker->NumChildren() )
    return;

  auto dockNode = node->AddChild( _T( "docker" ) );
  if ( docker->GetArrangement() != WB_ARRANGE_NONE )
  {
    dockNode.SetAttributeFromInteger( _T( "Vertical" ), docker->GetArrangement() == WB_ARRANGE_VERTICAL );
    if ( docker->NumChildren() == 2 )
    {
      dockNode.SetAttributeFromInteger( _T( "contentSize" ), docker->contentSize );
      dockNode.SetAttributeFromInteger( _T( "areaSize" ), docker->areaSize );
    }
  }

  for ( TU32 x = 0; x < docker->NumChildren(); x++ )
  {
    if ( docker->GetChild( x )->InstanceOf( _T( "apexdocker" ) ) )
      ExportDocker( &dockNode, (CapexDocker*)docker->GetChild( x ) );
    if ( docker->GetChild( x )->InstanceOf( _T( "apexwindow" ) ) )
    {
      auto wndnode = dockNode.AddChild( _T( "window" ) );
      CapexWindow* wnd = (CapexWindow*)docker->GetChild( x );
      wnd->ExportWindow( &wndnode );
    }
  }
}

void CapexWorkBench::ImportLayout( CXMLNode *node, bool resize )
{
  if ( !node ) return;
  if ( node->HasAttribute( _T( "Name" ) ) ) Name = node->GetAttributeAsString( _T( "Name" ) );

  if ( node->GetChildCount( _T( "docker" ) ) )
  {
    CXMLNode n = node->GetChild( _T( "docker" ) );
    if ( DockRoot )
      DockRoot->MarkForDeletion( true );
    DockRoot = new CapexDocker( this, GetClientRect() );
    ImportDocker( &n, DockRoot );
  }

  CRect bb;

  for ( TS32 x = 0; x < node->GetChildCount( _T( "window" ) ); x++ )
  {
    CXMLNode n = node->GetChild( _T( "window" ), x );
    OpenWindow( &n );

    CRect bb2;

    if ( n.HasAttribute( _T( "x1" ) ) ) n.GetAttributeAsInteger( _T( "x1" ), &bb2.x1 );
    if ( n.HasAttribute( _T( "y1" ) ) ) n.GetAttributeAsInteger( _T( "y1" ), &bb2.y1 );
    if ( n.HasAttribute( _T( "x2" ) ) ) n.GetAttributeAsInteger( _T( "x2" ), &bb2.x2 );
    if ( n.HasAttribute( _T( "y2" ) ) ) n.GetAttributeAsInteger( _T( "y2" ), &bb2.y2 );

    if ( x == 0 )
    {
      bb = bb2;
    }
    else
    {
      bb.x1 = min( bb.x1, bb2.x1 );
      bb.y1 = min( bb.y1, bb2.y1 );
      bb.x2 = max( bb.x2, bb2.x2 );
      bb.y2 = max( bb.y2, bb2.y2 );
    }

  }

  if ( resize && Windows.NumItems() )
  {
    CPoint offset = bb.TopLeft();
    TF32 xs = GetClientRect().Width() / (TF32)bb.Width();
    TF32 ys = GetClientRect().Height() / (TF32)bb.Height();

    int cnt = 0;

    for ( TS32 x = 0; x < Windows.NumItems(); x++ )
    {
      if ( Windows[ x ]->GetParent() == this )
      {
        CXMLNode n = node->GetChild( _T( "window" ), cnt );

        CRect r;

        if ( n.HasAttribute( _T( "x1" ) ) ) n.GetAttributeAsInteger( _T( "x1" ), &r.x1 );
        if ( n.HasAttribute( _T( "y1" ) ) ) n.GetAttributeAsInteger( _T( "y1" ), &r.y1 );
        if ( n.HasAttribute( _T( "x2" ) ) ) n.GetAttributeAsInteger( _T( "x2" ), &r.x2 );
        if ( n.HasAttribute( _T( "y2" ) ) ) n.GetAttributeAsInteger( _T( "y2" ), &r.y2 );

        r.x1 = TS32( ( r.x1 - offset.x )*xs );
        r.y1 = TS32( ( r.y1 - offset.y )*ys );
        r.x2 = TS32( ( r.x2 - offset.x )*xs );
        r.y2 = TS32( ( r.y2 - offset.y )*ys );
        Windows[ x ]->SetPosition( r );

        cnt++;
      }
    }
  }

}

void CapexWorkBench::ImportDocker( CXMLNode *node, CapexDocker* docker )
{
  for ( TS32 x = 0; x < node->GetChildCount(); x++ )
  {
    CXMLNode n = node->GetChild( x );

    if ( n.GetNodeName() == _T( "docker" ) )
    {
      CapexDocker *d = new CapexDocker( docker, docker->GetClientRect() );
      ImportDocker( &n, d );
    }
    if ( n.GetNodeName() == _T( "window" ) )
    {
      auto* wnd = OpenWindow( &n );
      if ( wnd )
        wnd->SetParent( docker );
    }
  }

  if ( node->HasAttribute( _T( "Vertical" ) ) )
  {
    TS32 v = 0;
    node->GetAttributeAsInteger( _T( "Vertical" ), &v );

    //App->ApplyStyle( docker );

    if ( !v )
      docker->ApplyStyleDeclarations( "child-layout: horizontal;" );
    else
      docker->ApplyStyleDeclarations( "child-layout: vertical;" );
  }

  if ( docker->NumChildren() == 2 )
  {
    if ( node->HasAttribute( _T( "contentSize" ) ) )
      node->GetAttributeAsInteger( _T( "contentSize" ), &docker->contentSize );
    if ( node->HasAttribute( _T( "areaSize" ) ) )
      node->GetAttributeAsInteger( _T( "areaSize" ), &docker->areaSize );

    docker->ApplyAutoScaleStyles();
  }

  App->ApplyStyle( docker );
  CWBMessage m;
  docker->BuildPositionMessage( GetChild( 0 )->GetPosition(), m );
  App->SendMessage( m );
}

void CapexWorkBench::SetEditedModel( CphxModel_Tool *Model )
{
  if ( PreviewObject )
  {
    SAFEDELETE( PreviewObject->Clips[ 0 ]->MaterialSplines );
    SAFEDELETE( PreviewObject->Clips[ 0 ] );
    SAFEDELETEA( PreviewObject->Clips );
  }

  if ( PreviewLight )
  {
    SAFEDELETE( PreviewLight->Clips[ 0 ]->MaterialSplines );
    SAFEDELETE( PreviewLight->Clips[ 0 ] );
    SAFEDELETEA( PreviewLight->Clips );
  }

  SimulateFreeArray( ModelPreviewScene->Scene.Objects, ModelPreviewScene->Scene.ObjectCount );
  //ModelPreviewScene->Scene.Objects.FreeArray();
  PreviewObject = NULL;
  PreviewLight = NULL;
  SAFEDELETE( ModelPreviewScene );

  ModelPreviewScene = new CphxScene_Tool();
  EditedModel = Model;

  if ( !Model )
  {
    UpdateWindows( apEx_ModelGraph );
    UpdateWindows( apEx_ModelParameters );
    UpdateWindows( apEx_ModelMaterial );
    UpdateWindows( apEx_ModelMatrix );
    return;
  }

  PreviewObject = new CphxObject_Model;
  PreviewObject->Clips = NULL;
  PreviewObject->Target = NULL;
  PreviewObject->Parent = NULL;
  PreviewObject->Children = NULL;
  PreviewObject->ChildCount = 0;

  SimulateAddItem<CphxObject*>( ModelPreviewScene->Scene.Objects, ModelPreviewScene->Scene.ObjectCount, PreviewObject );
  //ModelPreviewScene->Scene.Objects += PreviewObject;
  PreviewObject->Model = Model->GetModel();
  PreviewObject->SplineResults[ Spline_Position_x ] = 0;
  PreviewObject->SplineResults[ Spline_Position_y ] = 0;
  PreviewObject->SplineResults[ Spline_Position_z ] = 0;
  PreviewObject->SplineResults[ Spline_Scale_x ] = 1;
  PreviewObject->SplineResults[ Spline_Scale_y ] = 1;
  PreviewObject->SplineResults[ Spline_Scale_z ] = 1;
  PreviewObject->ToolData = NULL;
  PreviewObject->Clips = new CphxObjectClip*[ 1 ];
  PreviewObject->Clips[ 0 ] = new CphxObjectClip;
  PreviewObject->Clips[ 0 ]->SplineCount = 0;
  PreviewObject->Clips[ 0 ]->Splines = NULL;
  PreviewObject->Clips[ 0 ]->MaterialSplines = new CphxMaterialSplineBatch;
  PreviewObject->Clips[ 0 ]->MaterialSplines->SplineCount = 0;
  D3DXQuaternionIdentity( &PreviewObject->RotationResult );
  
  PreviewLight = new CphxObject;
  PreviewLight->ObjectType = Object_Light;
  PreviewLight->Clips = NULL;
  PreviewLight->Target = NULL;
  PreviewLight->Parent = NULL;
  PreviewLight->Children = NULL;
  PreviewLight->ChildCount = 0;

  SimulateAddItem( ModelPreviewScene->Scene.Objects, ModelPreviewScene->Scene.ObjectCount, PreviewLight );
  //ModelPreviewScene->Scene.Objects += PreviewLight;
  memset( PreviewLight->SplineResults, 0, sizeof( PreviewLight->SplineResults ) );
  PreviewLight->SplineResults[ Spline_Position_x ] = 1;
  PreviewLight->SplineResults[ Spline_Position_y ] = 1;
  PreviewLight->SplineResults[ Spline_Position_z ] = 1;
  PreviewLight->SplineResults[ Spline_Position_w ] = 0;
  PreviewLight->SplineResults[ Spline_Light_AmbientR ] = 0.2f;
  PreviewLight->SplineResults[ Spline_Light_AmbientG ] = 0.2f;
  PreviewLight->SplineResults[ Spline_Light_AmbientB ] = 0.2f;
  PreviewLight->SplineResults[ Spline_Light_DiffuseR ] = 1;
  PreviewLight->SplineResults[ Spline_Light_DiffuseG ] = 1;
  PreviewLight->SplineResults[ Spline_Light_DiffuseB ] = 1;
  PreviewLight->SplineResults[ Spline_Light_SpecularR ] = 1;
  PreviewLight->SplineResults[ Spline_Light_SpecularG ] = 1;
  PreviewLight->SplineResults[ Spline_Light_SpecularB ] = 1;
  PreviewLight->ToolData = NULL;
  PreviewLight->Clips = new CphxObjectClip*[ 1 ];
  PreviewLight->Clips[ 0 ] = new CphxObjectClip;
  PreviewLight->Clips[ 0 ]->SplineCount = 0;
  PreviewLight->Clips[ 0 ]->Splines = NULL;
  PreviewLight->Clips[ 0 ]->MaterialSplines = new CphxMaterialSplineBatch;
  PreviewLight->Clips[ 0 ]->MaterialSplines->SplineCount = 0;
  D3DXQuaternionIdentity( &PreviewLight->RotationResult );


  ModelPreviewScene->AddClip();
  ModelPreviewScene->RebuildMinimalData();

  UpdateWindows( apEx_ModelGraph );
  UpdateWindows( apEx_ModelParameters );
  UpdateWindows( apEx_ModelMaterial );
  UpdateWindows( apEx_ModelMatrix );
}

CphxModel_Tool *CapexWorkBench::GetEditedModel()
{
  return EditedModel;
}

void CapexWorkBench::SetEditedScene( CphxScene_Tool *Scene )
{
  if ( EditedScene == Scene )
    return;

  EditedScene = Scene;
  UpdateWindows( apEx_SceneGraph );
  UpdateWindows( apEx_SceneClips );
  UpdateWindows( apEx_SceneSplineEditor );
  UpdateWindows( apEx_SceneObjectParameters );
}

CphxScene_Tool *CapexWorkBench::GetEditedScene()
{
  return EditedScene;
}

void CapexWorkBench::SetEditedModelObject( CphxModelObject_Tool *_EditedObj )
{
  if ( EditedObj == _EditedObj )
    return;

  EditedObj = _EditedObj;
  EditedFilter = NULL;
  UpdateWindows( apEx_ModelGraph );
  UpdateWindows( apEx_ModelMaterial );
  UpdateWindows( apEx_ModelParameters );
  UpdateWindows( apEx_ModelMatrix );
}

void CapexWorkBench::SetEditedModelFilter( CphxMeshFilter_Tool *_EditedFilter )
{
  if ( _EditedFilter )
  {
    if ( EditedObj == _EditedFilter->ParentObject && EditedFilter == _EditedFilter )
      return;
  }
  else
    if ( EditedFilter == _EditedFilter )
      return;

  EditedFilter = _EditedFilter;
  if ( EditedFilter )
    EditedObj = EditedFilter->ParentObject;
  else
    EditedObj = NULL;
  UpdateWindows( apEx_ModelGraph );
  UpdateWindows( apEx_ModelMaterial );
  UpdateWindows( apEx_ModelParameters );
  UpdateWindows( apEx_ModelMatrix );
}

PHXMESHFILTER CapexWorkBench::GetEditedModelFilterType()
{
  if ( !GetEditedModelFilter() )
    return ModelFilter_NONE;
  return GetEditedModelFilter()->Filter;
}

CphxScene_Tool * CapexWorkBench::GetModelPreviewScene()
{
  return ModelPreviewScene;
}

CphxModelObject_Tool * CapexWorkBench::GetEditedModelObject()
{
  return EditedObj;
}

CphxMeshFilter_Tool * CapexWorkBench::GetEditedModelFilter()
{
  return EditedFilter;
}

void CapexWorkBench::ResetUIData()
{
  for ( int x = 0; x < Windows.NumItems(); x++ )
    Windows[ x ]->ResetUIData();
}

#include "../Phoenix/Timeline.h"

void CapexWorkBench::UpdateWindows()
{
  UpdateWindows( apEx_TexGenPages );
  UpdateWindows( apEx_ModelList );
  UpdateWindows( apEx_ScenePrimitives );
  UpdateWindows( apEx_TimelineEventParameters );
  for ( TS32 x = 0; x < Project.GetEventCount(); x++ )
    Project.GetEventByIndex( x )->Event->OnScreenLastFrame = false;
}

void CapexWorkBench::UpdateWindows( APEXWINDOW w )
{
  TS32 cnt = GetWindowCount( w );
  for ( TS32 x = 0; x < cnt; x++ )
    GetWindowByIndex( w, x )->UpdateData();
}

void CapexWorkBench::GoToTexture( CphxTextureOperator_Tool *Save )
{
  CapexTexGenMainWindow *w = (CapexTexGenMainWindow*)GetWindow( apEx_TexGenMain );

  if ( !w )
    return;

  CapexTexGenPages *p = (CapexTexGenPages*)GetWindow( apEx_TexGenPages );

  if ( !p )
    return;

  CapexTextureOpParameters *pw = (CapexTextureOpParameters*)GetWindow( apEx_TextureOpParameters );

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    auto page = Project.GetTexgenPageByIndex( x );
    for ( TS32 y = 0; y < page->GetOpCount(); y++ )
    {
      if ( page->GetOp( y ) == Save )
      {
        //p->SetEditedPage( page->GetID() );
        SetEditedPage( page->GetID() );
        w->SetEditedOperator( Save );
        w->CenterOperator( Save );
        if ( pw )
          pw->SetEditedOperator( Save );

        UpdateWindows( apEx_TexGenPages );

        return;
      }
    }
  }
}

void CapexWorkBench::SetLastTexgenPreview( CapexTexGenPreview* preview )
{
  lastTexgenPreview = preview;
}

TS32 CapexWorkBench::GetWindowCount( APEXWINDOW w )
{
  TS32 cnt = 0;
  for ( TS32 x = 0; x < Windows.NumItems(); x++ )
    if ( Windows[ x ]->GetWindowType() == w ) cnt++;
  return cnt;
}

CapexWindow * CapexWorkBench::GetWindowByIndex( APEXWINDOW w, TS32 c )
{
  TS32 cnt = 0;
  for ( TS32 x = 0; x < Windows.NumItems(); x++ )
    if ( Windows[ x ]->GetWindowType() == w )
    {
      if ( cnt == c ) return Windows[ x ];
      cnt++;
    }
  return NULL;
}

#include "apExRoot.h"

CapexWorkBench * GetActiveWorkBench()
{
  extern CapexRoot *Root;
  if ( !Root ) return NULL;
  return Root->GetWorkBench( Root->GetSelectedWorkBench() );
}
