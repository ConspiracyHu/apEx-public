#include "BasePCH.h"
#include "../Phoenix_Tool/phxSplineExt.h"
#include "SplineEditor_Phx.h"
#include "apExRoot.h"

CArray<CphxSplineKey_Tool> CapexSplineEditor_phx::CopiedSplineKeys;

enum SplineMessages
{
  SPLINEMESSAGE_LOOP = 100,
  SPLINEMESSAGE_COPY,
  SPLINEMESSAGE_PASTE,

  SPLINEMESSAGE_CONSTANT = INTERPOLATION_CONSTANT + 150,
  SPLINEMESSAGE_LINEAR = INTERPOLATION_LINEAR + 150,
  SPLINEMESSAGE_CUBIC = INTERPOLATION_CUBIC + 150,
  SPLINEMESSAGE_BEZIER = INTERPOLATION_BEZIER + 150,
};

CapexSplineEditor_phx::CapexSplineEditor_phx( CWBItem *Parent, const CRect &Pos ) : CWBItem( Parent, Pos )
{
  KeySize = 5;
  SplineClickSensitivity = 3;
  KeyColor = CColor::FromARGB( 0xff686868 );
  KeyColorHighlight = CColor::FromARGB( 0xff9e9e9e );
  KeyColorSelectNoFocus = CColor::FromARGB( 0xffefebef );
  KeyColorSelectFocus = CColor::FromARGB( 0xff007acc );
  DragMode = DRAG_NONE;
  Limited = true;
  TimePos = 0;
  ZoomX = ZoomY = 1;
  OffsetX = OffsetY = 0;
  HighlightBackground = false;
  ContextMenuViable = false;
  SnapToGrid = false;
  DraggedKey = NULL;
  SetGridDensity( 4 );
}

CapexSplineEditor_phx::~CapexSplineEditor_phx()
{

}

TBOOL CapexSplineEditor_phx::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_MOUSEWHEEL:
  {
    if ( !Limited )
    {
      CRect r = GetClientRect();
      CPoint mx = ScreenToClient( App->GetMousePos() );
      TF32 CursorValue1 = YToValue( mx.y );
      ZoomY *= 1 - Message.Data / 7.0f;
      TF32 CursorValue2 = YToValue( mx.y );
      OffsetY += ( CursorValue2 - CursorValue1 ) / ZoomY; //aDDict3 code FOR THE WIN
    }

    return true;
    break;
  }
  case WBM_LEFTBUTTONDOWN:
  {
    if ( !MouseOver() ) break;

    CphxSpline_Tool *s;
    TF32 t;
    TS32 k;
    TS32 h;

    App->SetCapture( this );

    if ( MouseOverKeyHelper( s, k, h ) )
    {
      if ( !App->GetCtrlState() && !s->Keys[ k ]->Selected )
        DeselectAllKeys();
      s->Keys[ k ]->Selected = true;
      if ( s->GetSplineType() != Spline_Quaternion )
        DraggedKey = s->Keys[ k ];
      DraggedHelper = h;
      DraggedSpline = s;
      DragMode = DRAG_KEYHELPER;
      StoreKeyPositions();
      return true;
    }
    if ( MouseOverKey( s, k ) )
    {
      if ( !App->GetCtrlState() && !s->Keys[ k ]->Selected )
        DeselectAllKeys();
      s->Keys[ k ]->Selected = true;
      if ( s->GetSplineType() != Spline_Quaternion )
        DraggedKey = s->Keys[ k ];
      DragMode = DRAG_KEY;
      StoreKeyPositions();
      return true;
    }
    if ( MouseOverSpline( s, t ) )
    {
      DeselectAllKeys();
      //create new key here

      CphxSplineKey_Tool *nt = new CphxSplineKey_Tool();
      nt->Selected = true;
      nt->Key.t = (TS32)( t * 255 );
      TF32 v[ 4 ];
      s->GetValue( t, v );
      for ( TS32 x = 0; x < 4; x++ )
      {
        nt->Key.Value[ x ] = v[ x ];
        nt->Stored.Value[ x ] = v[ x ];
      }
      s->Keys += nt;
      DragMode = DRAG_KEY;

      if ( s->GetSplineType() != Spline_Quaternion )
        DraggedKey = nt;

      StoreKeyPositions();
      s->Sort();
      return true;
    }

    DragMode = DRAG_SELECTION;
    return true;
  }
  break;
  case WBM_MOUSEMOVE:
  {
    ContextMenuViable = false;
    if ( DragMode == DRAG_NONE ) break;

    if ( ApplySplineDragUpdate() ) return true;

    if ( DragMode == DRAG_TIMEMOVE )
    {
      CRect r = GetClientRect();
      CPoint mp = ScreenToClient( Message.GetPosition() );
      SetTimePosition( max( 0, min( 1, ( (TS32)( ( mp.x / (TF32)r.Width() )*256.0f ) ) / 256.0f ) ) ); //set to 256 steps
      App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_TIMEPOSITIONCHANGED, GetGuid() ) );
      return true;
    }

    if ( DragMode == DRAG_DRAGVIEW )
    {
      CRect r = GetClientRect();
      OffsetY = OffsetY_ - ( Message.GetPosition().y - App->GetMidDownPos().y ) / (TF32)r.Height();
      return true;
    }

  }
  break;
  case WBM_LEFTBUTTONUP:
  {
    if ( DragMode == DRAG_SELECTION )
    {
      CRect MultiBox = ScreenToClient( CRect( App->GetLeftDownPos(), App->GetMousePos() ) );
      MultiBox.Normalize();

      CRect r = GetClientRect();

      if ( !App->GetCtrlState() )
        DeselectAllKeys();

      for ( TS32 i = 0; i < Splines.NumItems(); i++ )
        for ( TS32 x = 0; x < Splines[ i ].Spline->Keys.NumItems(); x++ )
        {
          CphxSplineKey *k = &Splines[ i ].Spline->Keys[ x ]->Key;
          //TF32 t=(x-r.x1)/(TF32)r.Width();
          TS32 xp = (TS32)( k->GetTime()*r.Width() + r.x1 );
          TS32 yp = (TS32)( ValueToY( k->Value[ 0 ] ) );

          CRect Key = CRect( xp - KeySize, yp - KeySize, xp + KeySize, yp + KeySize );
          Splines[ i ].Spline->Keys[ x ]->Selected |= MultiBox.Intersects( Key ) && DragMode == DRAG_SELECTION;

        }

    }

    App->ReleaseCapture();
    DragMode = DRAG_NONE;
    DraggedKey = NULL;
  }
  break;
  case WBM_MIDDLEBUTTONDOWN:
  {
    if ( !MouseOver() ) break;
    App->SetCapture( this );
    OffsetX_ = OffsetX;
    OffsetY_ = OffsetY;
    if ( !Limited )
      DragMode = DRAG_DRAGVIEW;
    return true;
  }
  break;
  case WBM_MIDDLEBUTTONUP:

    App->ReleaseCapture();
    DragMode = DRAG_NONE;
    return true;
    break;

  case WBM_RIGHTBUTTONDOWN:
  {
    if ( DragMode == DRAG_KEY || DragMode == DRAG_KEYHELPER )
    {
      //apply undo
      if ( ApplySplineDragUpdate() ) return true;
      break;
    }

    if ( !MouseOver() ) break;

    CphxSpline_Tool *s;
    TF32 t;
    TS32 k;

    App->SetCapture( this );

    if ( MouseOverKey( s, k ) )
    {
      DragMode = DRAG_NONE;
      ContextMenuViable = true;
      return true;
    }
    if ( MouseOverSpline( s, t ) )
    {
      DragMode = DRAG_NONE;
      ContextMenuViable = true;
      return true;
    }

    DragMode = DRAG_TIMEMOVE;
    CRect r = GetClientRect();
    CPoint mp = ScreenToClient( Message.GetPosition() );
    SetTimePosition( max( 0, min( 1, ( (TS32)( ( mp.x / (TF32)r.Width() )*256.0f ) ) / 256.0f ) ) ); //set to 256 steps
    App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_TIMEPOSITIONCHANGED, GetGuid() ) );
    return true;
  }
  break;

  case WBM_RIGHTBUTTONDBLCLK:
  case WBM_MIDDLEBUTTONDBLCLK:

    if ( DragMode == DRAG_KEY || DragMode == DRAG_KEYHELPER ) break;

    if ( !Limited )
    {
      ZoomX = 1;
      ZoomY = 5;
      OffsetX = 0;
      OffsetY = 0.5f;
    }
    break;

  case WBM_RIGHTBUTTONUP:

    if ( DragMode == DRAG_KEY || DragMode == DRAG_KEYHELPER )
    {
      //undo the undo
      if ( ApplySplineDragUpdate() ) return true;
      break;
    }

    App->ReleaseCapture();

    if ( ContextMenuViable )
    {
      CphxSpline_Tool *s;
      TF32 t;
      if ( MouseOverSpline( s, t ) )
      {
        ContextSpline = s;
        CWBContextMenu *ctx = OpenContextMenu( App->GetMousePos() );
        ctx->AddItem( _T( "Constant" ), SPLINEMESSAGE_CONSTANT );
        ctx->AddItem( _T( "Linear" ), SPLINEMESSAGE_LINEAR );
        ctx->AddItem( _T( "Catmull-Rom" ), SPLINEMESSAGE_CUBIC );
        if ( s->GetSplineType() != Spline_Quaternion )
          ctx->AddItem( _T( "Bezier" ), SPLINEMESSAGE_BEZIER );
        ctx->AddSeparator();

        if ( s->Spline->Loop )
          ctx->AddItem( _T( "*Loop" ), SPLINEMESSAGE_LOOP );
        else
          ctx->AddItem( _T( "Loop" ), SPLINEMESSAGE_LOOP );

        ctx->AddSeparator();
        if ( s->Keys.NumItems() ) ctx->AddItem( _T( "Copy Spline Keys" ), SPLINEMESSAGE_COPY );
        if ( CopiedSplineKeys.NumItems() ) ctx->AddItem( _T( "Paste Spline Keys" ), SPLINEMESSAGE_PASTE );
      }

    }

    DragMode = DRAG_NONE;
    return true;
    break;
  case WBM_CONTEXTMESSAGE:
    switch ( Message.Data )
    {
    case SPLINEMESSAGE_CONSTANT:
    case SPLINEMESSAGE_LINEAR:
    case SPLINEMESSAGE_CUBIC:
    case SPLINEMESSAGE_BEZIER:
    {
      for ( TS32 x = 0; x < Splines.NumItems(); x++ )
      {
        Splines[ x ].Spline->Spline->Interpolation = (SPLINEINTERPOLATION)( Message.Data - 150 );
      }
      App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
    }
    break;
    case SPLINEMESSAGE_LOOP:
    {
      for ( TS32 x = 0; x < Splines.NumItems(); x++ )
        Splines[ x ].Spline->Spline->Loop = !Splines[ x ].Spline->Spline->Loop;
      App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
    }
    break;

    case SPLINEMESSAGE_COPY:
      if ( ContextSpline )
      {
        CopiedSplineKeys.Flush();
        for ( TS32 x = 0; x < ContextSpline->Keys.NumItems(); x++ )
          CopiedSplineKeys += *ContextSpline->Keys[ x ];
      }
      break;

    case SPLINEMESSAGE_PASTE:
      if ( ContextSpline )
      {
        for ( TS32 x = 0; x < CopiedSplineKeys.NumItems(); x++ )
        {
          CphxSplineKey_Tool *k = new CphxSplineKey_Tool();
          *k = CopiedSplineKeys[ x ];
          ContextSpline->Keys += k;
        }
        ContextSpline->Sort();
      }
      break;
    }
    ContextSpline = NULL;
    break;
  case WBM_KEYDOWN:
    if ( !InFocus() ) break;
    if ( Message.KeyboardState&WB_KBSTATE_ALT && Message.KeyboardState&WB_KBSTATE_CTRL )
      break; //altgr
    switch ( Message.Key )
    {
    case VK_DELETE:
      DeleteSelectedKeys();
      App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
      return true;
      break;
    default:
      break;
    }

    break;
  }

  return CWBItem::MessageProc( Message );
}

void CapexSplineEditor_phx::OnDraw( CWBDrawAPI *API )
{
  DrawBackground( API );
  API->SetCropToClient( this );
  CRect r = GetClientRect();

  //grid x

  for ( TS32 x = 0; x < GridDensityX; x++ )
  {
    CColor color = CSSProperties.DisplayDescriptor.GetColor( GetState(), WB_ITEM_BORDERCOLOR );
    TS32 xv = (TS32)Lerp( (TF32)r.x1, (TF32)r.x2, x / (float)GridDensityX );
    TS32 yv = (TS32)Lerp( (TF32)r.y1, (TF32)r.y2, x / (float)GridDensityX );
    if ( Limited ) API->DrawLine( CPoint( r.x1, yv ), CPoint( r.x2, yv ), color );
    API->DrawLine( CPoint( xv, r.y1 ), CPoint( xv, r.y2 ), color );
  }

  //key start position x
  if ( DraggedKey && SnapToGrid && DragMode == DRAG_KEY )
  {
    int p = (TS32)TimeToX( DraggedKey->Stored.t );
    API->DrawRect( CRect( CPoint( (TS32)p - 1, r.y1 ), CPoint( (TS32)p, r.y2 ) ), CColor::FromARGB( 0x8000cc7a ) );
  }

  // grid y
  if ( !Limited ) DrawGrid( API );

  //time position
  if ( !Limited )
  {
    TS32 x = (TS32)Lerp( (TF32)r.x1, (TF32)r.x2, TimePos );
    API->DrawRect( CRect( x, r.y1, x + 1, r.y2 ), CColor::FromARGB( 0xff598557 ) );
  }

  //splines

  for ( TS32 i = 0; i < Splines.NumItems(); i++ )
  {
    TF32 oldvalue[ 4 ];
    TF32 value[ 4 ];

    TS32 vc = 1;
    if ( Splines[ i ].Spline->GetSplineType() == Spline_Quaternion ) vc = 4;

    for ( TS32 z = 0; z < vc; z++ )
    {
      for ( TS32 x = r.x1; x < r.x2; x++ )
      {
        TF32 t = ( x - r.x1 ) / (TF32)r.Width();

        Splines[ i ].Spline->GetValue( t, value );
        if ( x == r.x1 ) oldvalue[ z ] = value[ z ];

        CPoint p1, p2;
        p1 = CPoint( x - 1, (TS32)( ValueToY( oldvalue[ z ] ) ) );
        p2 = CPoint( x, (TS32)( ValueToY( value[ z ] ) ) );

        oldvalue[ z ] = value[ z ];

        API->DrawLine( p1, p2, Splines[ i ].SplineColor );
      }
    }

    for ( TS32 z = 1; z < Splines[ i ].Spline->Keys.NumItems(); z++ )
    {
      if ( Splines[ i ].Spline->Keys[ z - 1 ]->Key.t == Splines[ i ].Spline->Keys[ z ]->Key.t )
      {
        TS32 x = (TS32)( Splines[ i ].Spline->Keys[ z - 1 ]->Key.GetTime()*r.Width() + r.x1 );
        CPoint p1 = CPoint( x - 2, r.y1 );
        CPoint p2 = CPoint( x + 2, r.y2 );

        API->DrawRect( CRect( p1, p2 ), CColor::FromARGB( 0xffff0000 ) );
      }
    }
  }

  //bezier ear lines

  for ( TS32 i = 0; i < Splines.NumItems(); i++ )
  {
    for ( TS32 x = 0; x < Splines[ i ].Spline->Keys.NumItems(); x++ )
    {
      CphxSplineKey *k = &Splines[ i ].Spline->Keys[ x ]->Key;
      TS32 kxp = (TS32)( k->GetTime()*r.Width() + r.x1 );
      TS32 kyp = (TS32)( ValueToY( k->Value[ 0 ] ) );

      if ( Splines[ i ].Spline->Spline->Interpolation == INTERPOLATION_BEZIER )
      {
        TF32 v1, v2;
        TS32 t1, t2;
        v1 = Splines[ i ].Spline->Keys[ x ]->Key.Value[ 0 ] + Splines[ i ].Spline->Keys[ x ]->Key.controlvalues[ 0 ];
        v2 = Splines[ i ].Spline->Keys[ x ]->Key.Value[ 0 ] - Splines[ i ].Spline->Keys[ x ]->Key.controlvalues[ 1 ];
        t1 = Splines[ i ].Spline->Keys[ x ]->Key.t + Splines[ i ].Spline->Keys[ x ]->Key.controlpositions[ 0 ];
        t2 = Splines[ i ].Spline->Keys[ x ]->Key.t - Splines[ i ].Spline->Keys[ x ]->Key.controlpositions[ 1 ];

        TS32 xp = (TS32)( SPLINEPOSITIONCALC( t1 )*r.Width() + r.x1 );
        TS32 yp = (TS32)( ValueToY( v1 ) );

        API->DrawLine( CPoint( kxp, kyp ), CPoint( xp, yp ), CColor::FromARGB( 0xff00cc7a ) );

        xp = (TS32)( SPLINEPOSITIONCALC( t2 )*r.Width() + r.x1 );
        yp = (TS32)( ValueToY( v2 ) );

        API->DrawLine( CPoint( kxp, kyp ), CPoint( xp, yp ), CColor::FromARGB( 0xff00cc7a ) );
      }

    }
  }

  TBOOL MouseOverKey = false;
  CRect MultiBox = ScreenToClient( CRect( App->GetLeftDownPos(), App->GetMousePos() ) );
  MultiBox.Normalize();
  CSplineEditorLink &MouseKeySpline = CSplineEditorLink();
  CphxSplineKey_Tool *MouseKey = NULL;

  for ( TS32 i = 0; i < Splines.NumItems(); i++ )
  {
    for ( TS32 x = 0; x < Splines[ i ].Spline->Keys.NumItems(); x++ )
    {
      CphxSplineKey *k = &Splines[ i ].Spline->Keys[ x ]->Key;
      //TF32 t=(x-r.x1)/(TF32)r.Width();
      TS32 xp = (TS32)( ( k->GetTime() )*r.Width() + r.x1 );
      TS32 yp = (TS32)( ValueToY( k->Value[ 0 ] ) );

      CRect Key = CRect( xp - KeySize, yp - KeySize, xp + KeySize, yp + KeySize );
      CColor Col = KeyColor;

      TBOOL Selected = Splines[ i ].Spline->Keys[ x ]->Selected || ( MultiBox.Intersects( Key ) && DragMode == DRAG_SELECTION );


      if ( MouseOver() && ClientToScreen( Key ).Contains( App->GetMousePos() ) )
      {
        Col = KeyColorHighlight;
        MouseOverKey = true;
        MouseKey = Splines[ i ].Spline->Keys[ x ];
        if ( !MouseKeySpline.Spline ) MouseKeySpline = Splines[ i ];
      }

      if ( Selected )
      {
        if ( InFocus() )
          Col = KeyColorSelectFocus;
        else
          Col = KeyColorSelectNoFocus;
      }


      API->DrawRect( Key, Col );

      if ( Splines[ i ].Spline->Spline->Interpolation == INTERPOLATION_BEZIER )
      {
        TF32 v1, v2;
        TS32 t1, t2;
        v1 = Splines[ i ].Spline->Keys[ x ]->Key.Value[ 0 ] + Splines[ i ].Spline->Keys[ x ]->Key.controlvalues[ 0 ];
        v2 = Splines[ i ].Spline->Keys[ x ]->Key.Value[ 0 ] - Splines[ i ].Spline->Keys[ x ]->Key.controlvalues[ 1 ];
        t1 = Splines[ i ].Spline->Keys[ x ]->Key.t + Splines[ i ].Spline->Keys[ x ]->Key.controlpositions[ 0 ];
        t2 = Splines[ i ].Spline->Keys[ x ]->Key.t - Splines[ i ].Spline->Keys[ x ]->Key.controlpositions[ 1 ];

        xp = (TS32)( ( SPLINEPOSITIONCALC( t1 ) )*r.Width() + r.x1 );
        yp = (TS32)( ValueToY( v1 ) );

        Key = CRect( xp - ( KeySize - 2 ), yp - ( KeySize - 2 ), xp + KeySize - 2, yp + KeySize - 2 );

        Col = KeyColor;
        Selected = Splines[ i ].Spline->Keys[ x ]->Selected || ( MultiBox.Intersects( Key ) && DragMode == DRAG_SELECTION );

        if ( MouseOver() && ClientToScreen( Key ).Contains( App->GetMousePos() ) )
        {
          Col = KeyColorHighlight;
          MouseOverKey = true;
          if ( !MouseKeySpline.Spline ) MouseKeySpline = Splines[ i ];
        }

        if ( Selected )
        {
          if ( InFocus() )
            Col = KeyColorSelectFocus;
          else
            Col = KeyColorSelectNoFocus;
        }

        API->DrawRect( Key, Col );

        xp = (TS32)( ( SPLINEPOSITIONCALC( t2 ) )*r.Width() + r.x1 );
        yp = (TS32)( ValueToY( v2 ) );

        Key = CRect( xp - ( KeySize - 2 ), yp - ( KeySize - 2 ), xp + KeySize - 2, yp + KeySize - 2 );

        Col = KeyColor;
        Selected = Splines[ i ].Spline->Keys[ x ]->Selected || ( MultiBox.Intersects( Key ) && DragMode == DRAG_SELECTION );

        if ( MouseOver() && ClientToScreen( Key ).Contains( App->GetMousePos() ) )
        {
          Col = KeyColorHighlight;
          MouseOverKey = true;
          if ( !MouseKeySpline.Spline ) MouseKeySpline = Splines[ i ];
        }

        if ( Selected )
        {
          if ( InFocus() )
            Col = KeyColorSelectFocus;
          else
            Col = KeyColorSelectNoFocus;
        }

        API->DrawRect( Key, Col );
      }
    }
  }

  if ( DragMode == DRAG_SELECTION )
  {
    API->DrawRect( MultiBox, CColor::FromARGB( 0x300956a2 ) );
    API->DrawRectBorder( MultiBox, CColor::FromARGB( 0xff0956a2 ) );
  }

  if ( !Limited ) DrawGridNumbers( API );

  if ( MouseOverKey && DragMode == DRAG_NONE )
  {
    if ( !MouseKey )
      SetTooltip( MouseKeySpline.SplineName );
    else
      SetTooltip( CString::Format( _T( "%s\nTime: %.6g\nValue: %.6g (%d)" ), MouseKeySpline.SplineName.GetPointer(), (float)MouseKey->Key.GetTime(), (float)MouseKey->Key.Value[ 0 ], (int)( (float)MouseKey->Key.Value[ 0 ] * 255 ) ) );
  }

  if ( !MouseOverKey && DragMode == DRAG_NONE )
  {
    CphxSpline_Tool *s;
    TF32 t;
    CString Name;
    if ( MouseOverSpline( s, t, Name ) )
    {
      App->SelectMouseCursor( CM_CROSS );
      //change cursor here
      SetTooltip( Name );
    }
  }

  if ( DragMode == DRAG_KEY && DraggedKey )
  {
    float v[ 4 ];
    v[ 0 ] = v[ 1 ] = v[ 2 ] = v[ 3 ] = 0;
    DraggedKey->Key.GetValue( v );
    SetTooltip( CString::Format( _T( "Time: %.6g\nValue: %.6g (%d)" ), DraggedKey->Key.GetTime(), v[ 0 ], (int)( v[ 0 ] * 255 ) ) );
  }

  DrawBorder( API );
}

CWBItem * CapexSplineEditor_phx::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CapexSplineEditor_phx *splineeditor = new CapexSplineEditor_phx( Root, Pos );
  TS32 val = 1;
  node.GetAttributeAsInteger( _T( "limited" ), &val );
  splineeditor->Limited = val != 0;

  val = 0;
  node.GetAttributeAsInteger( _T( "snap" ), &val );
  splineeditor->SnapToGrid = val != 0;

  node.GetAttributeAsInteger( _T( "gridx" ), &splineeditor->GridDensityX );

  if ( !splineeditor->Limited )
  {
    splineeditor->ZoomY = 5;
    splineeditor->OffsetY = 0.5f;
  }

  return splineeditor;
}

void CapexSplineEditor_phx::AddSpline( CphxSpline_Tool *Spline, CColor Color, CString &Name )
{
  if ( !Spline ) return;
  CSplineEditorLink l;
  l.Spline = Spline;
  l.SplineColor = Color;
  l.SplineName = Name;
  Splines += l;
  l.Spline->UpdateSplineKeys();
}

TBOOL CapexSplineEditor_phx::MouseOverKey( CphxSpline_Tool *&Spline, TS32 &Key )
{
  if ( !MouseOver() ) return false;

  CRect r = GetClientRect();

  for ( TS32 i = 0; i < Splines.NumItems(); i++ )
  {
    for ( TS32 x = 0; x < Splines[ i ].Spline->Keys.NumItems(); x++ )
    {
      CphxSplineKey *k = &Splines[ i ].Spline->Keys[ x ]->Key;
      TS32 xp = (TS32)( k->GetTime()*r.Width() + r.x1 );
      TS32 yp = (TS32)( ValueToY( k->Value[ 0 ] ) );

      CRect KeyPos = CRect( xp - KeySize, yp - KeySize, xp + KeySize, yp + KeySize );

      if ( ClientToScreen( KeyPos ).Contains( App->GetMousePos() ) )
      {
        Spline = Splines[ i ].Spline;
        Key = x;
        return true;
      }
    }
  }

  return false;
}

TBOOL CapexSplineEditor_phx::MouseOverKeyHelper( CphxSpline_Tool *&Spline, TS32 &Key, TS32 &HelperID )
{
  if ( !MouseOver() ) return false;

  CRect r = GetClientRect();

  for ( TS32 i = 0; i < Splines.NumItems(); i++ )
    if ( Splines[ i ].Spline->Spline->Interpolation == INTERPOLATION_BEZIER )
    {
      for ( TS32 x = 0; x < Splines[ i ].Spline->Keys.NumItems(); x++ )
      {
        CphxSplineKey *k = &Splines[ i ].Spline->Keys[ x ]->Key;

        TF32 v1, v2;
        TS32 t1, t2;
        v1 = k->Value[ 0 ] + k->controlvalues[ 0 ];
        v2 = k->Value[ 0 ] - k->controlvalues[ 1 ];
        t1 = k->t + k->controlpositions[ 0 ];
        t2 = k->t - k->controlpositions[ 1 ];

        TS32 xp, yp;
        xp = (TS32)( SPLINEPOSITIONCALC( t1 )*r.Width() + r.x1 );
        yp = (TS32)( ValueToY( v1 ) );

        CRect KeyPos = CRect( xp - ( KeySize - 2 ), yp - ( KeySize - 2 ), xp + KeySize - 2, yp + KeySize - 2 );

        if ( ClientToScreen( KeyPos ).Contains( App->GetMousePos() ) )
        {
          Spline = Splines[ i ].Spline;
          Key = x;
          HelperID = 0;
          return true;
        }

        xp = (TS32)( SPLINEPOSITIONCALC( t2 )*r.Width() + r.x1 );
        yp = (TS32)( ValueToY( v2 ) );

        KeyPos = CRect( xp - ( KeySize - 2 ), yp - ( KeySize - 2 ), xp + KeySize - 2, yp + KeySize - 2 );

        if ( ClientToScreen( KeyPos ).Contains( App->GetMousePos() ) )
        {
          Spline = Splines[ i ].Spline;
          Key = x;
          HelperID = 1;
          return true;
        }
      }
    }

  return false;
}

TBOOL CapexSplineEditor_phx::MouseOverSpline( CphxSpline_Tool *&Spline, TF32 &t, CString &Name )
{
  if ( !MouseOver() ) return false;

  CRect r = GetClientRect();
  CPoint p = ScreenToClient( App->GetMousePos() );
  CRect mr = CRect( p.x - SplineClickSensitivity, p.y - SplineClickSensitivity, p.x + SplineClickSensitivity, p.y + SplineClickSensitivity );

  for ( TS32 i = 0; i < Splines.NumItems(); i++ )
  {
    for ( TS32 x = max( r.x1, p.x - SplineClickSensitivity ); x < min( r.x2, p.x + SplineClickSensitivity ); x++ )
    {
      TF32 t1 = ( x - 1 - r.x1 ) / (TF32)r.Width();
      TF32 t2 = ( x - r.x1 ) / (TF32)r.Width();

      TF32 v1[ 4 ], v2[ 4 ];
      Splines[ i ].Spline->GetValue( t1, v1 );
      Splines[ i ].Spline->GetValue( t2, v2 );

      CPoint p1, p2;
      p1 = CPoint( x - 1, (TS32)ValueToY( v1[ 0 ] ) );
      p2 = CPoint( x, (TS32)ValueToY( v2[ 0 ] ) );

      if ( IntervalIntersection( mr.y1, mr.y2, p1.y, p2.y ) )
      {
        Spline = Splines[ i ].Spline;
        t = ( p.x - r.x1 ) / (TF32)r.Width();
        Name = Splines[ i ].SplineName;
        return true;
      }
    }
  }

  return false;
}

TBOOL CapexSplineEditor_phx::MouseOverSpline( CphxSpline_Tool *&Spline, TF32 &t )
{
  CString s;
  return MouseOverSpline( Spline, t, s );
}


void CapexSplineEditor_phx::StoreKeyPositions()
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
    Splines[ x ].Spline->StoreKeyData();
}

void CapexSplineEditor_phx::DeselectAllKeys()
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
    for ( TS32 y = 0; y < Splines[ x ].Spline->Keys.NumItems(); y++ )
      Splines[ x ].Spline->Keys[ y ]->Selected = false;
}

TF32 CapexSplineEditor_phx::SnapValue( TF32 StoredValue, TF32 yv, TBOOL BezierHelper )
{
  TF32 uc = ValueToY( StoredValue + yv );

  //snap to grid

  TF32 griddensity = GridDensity;
  CRect r = GetClientRect();
  D3DXVECTOR2 o = D3DXVECTOR2( 0, 1 - OffsetY );
  TF32 StepSize = 1;

  while ( StepSize < ZoomY / griddensity ) StepSize *= 2.0f;
  while ( StepSize > ZoomY / griddensity ) StepSize /= 2.0f;

  TF32 yp1 = YToValue( r.y1 - 1 );
  TF32 yp2 = YToValue( r.y2 + 1 );

  TS32 yg1 = (TS32)( yp1 / StepSize ) + 1;
  TS32 yg2 = (TS32)( yp2 / StepSize ) - 1;

  if ( !BezierHelper )
    for ( TS32 x = yg2; x <= yg1; x++ )
    {
      TF32 y = x*StepSize;
      TS32 ssy = (TS32)ValueToY( y );
      if ( abs( uc - ssy ) < 6 ) yv = y - StoredValue;
      uc = ValueToY( StoredValue + yv );
    }

  //snap to 0, 1 and original

  TF32 y0 = ValueToY( 0 );
  TF32 y1 = ValueToY( 1 );
  TF32 yo = ValueToY( StoredValue );

  if ( abs( uc - yo ) < 6 ) yv = 0;
  uc = ValueToY( StoredValue + yv );

  if ( abs( uc - y0 ) < 6 ) yv = -StoredValue;
  uc = ValueToY( StoredValue + yv );

  if ( !BezierHelper )
    if ( abs( uc - y1 ) < 6 ) yv = 1 - StoredValue;

  return yv;
}

TS32 CapexSplineEditor_phx::SnapTime( TS32 StoredTime, TS32 t, TBOOL BezierHelper )
{
  TS32 uc = TimeToX( StoredTime + t );

  TS32 xt = TimeToX( (TS32)( TimePos * 255 ) );
  TS32 xo = TimeToX( StoredTime );

  if ( abs( uc - xo ) < 6 ) t = 0;
  if ( abs( uc - xt ) < 6 ) t = (TS32)( TimePos * 255 ) - StoredTime;

  return t;
}

void CapexSplineEditor_phx::MouseMoveSplineKeys()
{
  CRect r = GetClientRect();

  CPoint p = App->GetMousePos() - App->GetLeftDownPos();
  TS32 xv = (TS32)( ( p.x / (TF32)r.Width() ) * 255 );
  TF32 yv = (TF32)( ( -p.y ) / (TF32)r.Height() )*ZoomY;

  if ( App->GetRightButtonState() )
  {
    xv = 0;
    yv = 0;
  }

  if ( DraggedKey && SnapToGrid ) //do snap adjust
  {
    xv = SnapTime( DraggedKey->Stored.t, xv, false );
    yv = SnapValue( DraggedKey->Stored.Value[ 0 ], yv, false );
  }

  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
  {
    for ( TS32 y = 0; y < Splines[ x ].Spline->Keys.NumItems(); y++ )
      if ( Splines[ x ].Spline->Keys[ y ]->Selected )
      {
        Splines[ x ].Spline->Keys[ y ]->Key.t = max( 0, min( 255, Splines[ x ].Spline->Keys[ y ]->Stored.t + xv ) );
        if ( Splines[ x ].Spline->GetSplineType() != Spline_Quaternion )
        {
          if ( Limited )
            Splines[ x ].Spline->Keys[ y ]->Key.Value[ 0 ] = max( 0, min( 1, Splines[ x ].Spline->Keys[ y ]->Stored.Value[ 0 ] + yv ) );
          else
            Splines[ x ].Spline->Keys[ y ]->Key.Value[ 0 ] = Splines[ x ].Spline->Keys[ y ]->Stored.Value[ 0 ] + yv;
        }
      }
    Splines[ x ].Spline->Sort();
    Splines[ x ].Spline->Sort(); //sort twice to retain original key order
  }
}

void CapexSplineEditor_phx::MouseMoveSplineKeyHelper()
{
  if ( !DraggedKey ) return;
  CRect r = GetClientRect();

  CPoint p = App->GetMousePos() - App->GetLeftDownPos();
  TS32 xv = (TS32)( ( p.x / (TF32)r.Width() ) * 255 );
  TF32 yv = (TF32)( ( -p.y ) / (TF32)r.Height() )*ZoomY;

  if ( App->GetRightButtonState() )
  {
    xv = 0;
    yv = 0;
  }

  if ( DraggedHelper < 0 || DraggedHelper >= 2 ) return;

  if ( DraggedHelper == 0 )
  {
    if ( SnapToGrid )
      yv = SnapValue( DraggedKey->Stored.controlvalues[ 0 ], yv, true );

    DraggedKey->Key.controlvalues[ 0 ] = DraggedKey->Stored.controlvalues[ 0 ] + yv;
    DraggedKey->Key.controlvalues[ 1 ] = DraggedKey->Stored.controlvalues[ 1 ] + yv;
    DraggedKey->Key.controlpositions[ 0 ] = max( 0, DraggedKey->Stored.controlpositions[ 0 ] + xv );
    DraggedKey->Key.controlpositions[ 1 ] = max( 0, DraggedKey->Stored.controlpositions[ 1 ] + xv );
  }

  if ( DraggedHelper == 1 )
  {
    if ( SnapToGrid )
      yv = -SnapValue( DraggedKey->Stored.controlvalues[ 1 ], -yv, true );

    DraggedKey->Key.controlvalues[ 0 ] = DraggedKey->Stored.controlvalues[ 0 ] - yv;
    DraggedKey->Key.controlvalues[ 1 ] = DraggedKey->Stored.controlvalues[ 1 ] - yv;
    DraggedKey->Key.controlpositions[ 0 ] = max( 0, DraggedKey->Stored.controlpositions[ 0 ] - xv );
    DraggedKey->Key.controlpositions[ 1 ] = max( 0, DraggedKey->Stored.controlpositions[ 1 ] - xv );
  }
}


void CapexSplineEditor_phx::DeleteSelectedKeys()
{
  for ( TS32 x = 0; x < Splines.NumItems(); x++ )
  {
    for ( TS32 y = 0; y < Splines[ x ].Spline->Keys.NumItems(); y++ )
      if ( Splines[ x ].Spline->Keys[ y ]->Selected )
      {
        Splines[ x ].Spline->Keys.FreeByIndex( y );
        y--;
      }
    Splines[ x ].Spline->UpdateSplineKeys();
  }
}

void CapexSplineEditor_phx::FlushSplines()
{
  Splines.FlushFast();
}

void CapexSplineEditor_phx::SetTimePosition( TF32 t )
{
  TimePos = min( 1, max( 0, t ) );
}

TF32 CapexSplineEditor_phx::GetTimePosition()
{
  return TimePos;
}

void CapexSplineEditor_phx::DrawGrid( CWBDrawAPI *API )
{
  TF32 griddensity = GridDensity;
  TU8 GridColor = 48;

  CRect r = GetClientRect();

  D3DXVECTOR2 o = D3DXVECTOR2( 0, 1 - OffsetY );

  //o is the position of the origin in viewport space

  TF32 StepSize = 1;

  //OrthoZoom equals the world space size of the height of the viewport
  //we want the grid to split the viewport into about 8 equal areas

  while ( StepSize < ZoomY / griddensity ) StepSize *= 2.0f;
  while ( StepSize > ZoomY / griddensity ) StepSize /= 2.0f;

  TF32 y1 = YToValue( r.y1 - 1 );
  TF32 y2 = YToValue( r.y2 + 1 );

  TS32 yg1 = (TS32)( y1 / StepSize ) + 1;
  TS32 yg2 = (TS32)( y2 / StepSize ) - 1;

  //adaptive grid
  for ( TS32 x = yg2; x <= yg1; x++ )
  {
    TF32 y = x*StepSize;
    TS32 ssy = (TS32)ValueToY( y );

    API->DrawRect( CRect( CPoint( r.x1, ssy - 1 ), CPoint( r.x2 - 1, ssy ) ), 0xff010101 * GridColor );
    CString s = CString::Format( "%.6g", y );
  }

  //key start position
  if ( DraggedKey && SnapToGrid && DragMode == DRAG_KEY )
  {
    int p = (TS32)ValueToY( DraggedKey->Stored.Value[ 0 ] );
    API->DrawRect( CRect( CPoint( r.x1, (TS32)p - 1 ), CPoint( r.x2, (TS32)p ) ), CColor::FromARGB( 0x8000cc7a ) );
  }

  //bezier helper start position
  if ( DraggedKey && SnapToGrid && DragMode == DRAG_KEYHELPER )
  {
    int p = (TS32)ValueToY( DraggedKey->Stored.Value[ 0 ] + DraggedKey->Stored.controlvalues[ 0 ] );
    if ( DraggedHelper == 1 )
      p = (TS32)ValueToY( DraggedKey->Stored.Value[ 0 ] - DraggedKey->Stored.controlvalues[ 1 ] );
    API->DrawRect( CRect( CPoint( r.x1, (TS32)p - 1 ), CPoint( r.x2, (TS32)p ) ), CColor::FromARGB( 0x8000cc7a ) );
  }

  //origin
  int zero = (TS32)ValueToY( 0.0f );
  API->DrawRect( CRect( CPoint( r.x1, (TS32)zero - 1 ), CPoint( r.x2, (TS32)zero ) ), CColor::FromARGB( 0xff007acc ) );

  //one
  int one = (TS32)ValueToY( 1.0f );
  API->DrawRect( CRect( CPoint( r.x1, (TS32)one - 1 ), CPoint( r.x2, (TS32)one ) ), CColor::FromARGB( 0xff007acc ) );
}

void CapexSplineEditor_phx::DrawGridNumbers( CWBDrawAPI *API )
{
  CWBFont *font = GetFont( GetState() );

  TF32 griddensity = GridDensity;
  CRect r = GetClientRect();

  D3DXVECTOR2 o = D3DXVECTOR2( 0, 1 - OffsetY );

  TF32 StepSize = 1;

  while ( StepSize < ZoomY / griddensity ) StepSize *= 2.0f;
  while ( StepSize > ZoomY / griddensity ) StepSize /= 2.0f;

  TF32 y1 = YToValue( r.y1 - 1 );
  TF32 y2 = YToValue( r.y2 + 1 );

  TS32 yg1 = (TS32)( y1 / StepSize ) + 1;
  TS32 yg2 = (TS32)( y2 / StepSize ) - 1;

  for ( TS32 x = yg2; x <= yg1; x++ )
  {
    TF32 y = x*StepSize;
    TS32 ssy = (TS32)ValueToY( y );

    CString s = CString::Format( "%.6g", y );
    TS32 width = font->GetWidth( s );
    WriteWithOutline( API, font, s, CPoint( GetClientRect().x1 + 2, ssy - 1 - font->GetLineHeight() ) );
    WriteWithOutline( API, font, s, CPoint( GetClientRect().x2 - 2 - width, ssy - 1 - font->GetLineHeight() ) );
  }
}

TS32 CapexSplineEditor_phx::TimeToX( TS32 Time )
{
  CRect r = GetClientRect();
  return (TS32)( SPLINEPOSITIONCALC( Time )*r.Width() + r.x1 );
}

TS32 CapexSplineEditor_phx::XToTime( TS32 X )
{
  CRect r = GetClientRect();
  return max( 0, (TS32)( ( X - r.x1 ) / (TF32)r.Width() * 256 ) - 1 );
}

TF32 CapexSplineEditor_phx::ValueToY( TF32 Value )
{
  CRect r = GetClientRect();
  return r.Height() - r.Height()*( Value / ZoomY + OffsetY );
}

TF32 CapexSplineEditor_phx::YToValue( TS32 Y )
{
  CRect r = GetClientRect();
  return ( ( r.Height() - Y ) / (TF32)r.Height() - OffsetY )*ZoomY;
}

TBOOL CapexSplineEditor_phx::ApplySplineDragUpdate()
{
  if ( DragMode == DRAG_KEY )
  {
    MouseMoveSplineKeys();
    App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
    return true;
  }

  if ( DragMode == DRAG_KEYHELPER )
  {
    MouseMoveSplineKeyHelper();
    DraggedSpline->UpdateSplineKeys();
    App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
    return true;
  }

  return false;
}

void CapexSplineEditor_phx::SetGridDensity( TF32 g )
{
  GridDensity = g;
}

void CapexSplineEditor_phx::WriteWithOutline( CWBDrawAPI *API, CWBFont *Font, CString &text, CPoint Pos )
{
  for ( TS32 x = -1; x < 2; x++ )
    for ( TS32 y = -1; y < 2; y++ )
    {
      if ( x != 0 || y != 0 )
        Font->Write( API, text, Pos + CPoint( x, y ), 0xff000000 );
    }

  Font->Write( API, text, Pos );
}

CSplineEditorLink::CSplineEditorLink()
{
  SplineColor = 0xffffffff;
  SplineName = _T( "Spline" );
  Spline = NULL;
}