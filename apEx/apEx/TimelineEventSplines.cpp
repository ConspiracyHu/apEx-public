#define _WINSOCKAPI_
#include "BasePCH.h"
#include "../Phoenix_Tool/phxSplineExt.h"
#include "../Phoenix/Timeline.h"
#include "SplineEditor_Phx.h"
#include "TimelineEventSplines.h"
#define WINDOWNAME _T("Timeline Event Splines")
#define WINDOWXML _T("TimelineEventSplines")
#include "../Phoenix_Tool/apxProject.h"

CapexTimelineEventSplines::CapexTimelineEventSplines() : CapexWindow()
{
  EditedEvent = NULL;
}

CapexTimelineEventSplines::CapexTimelineEventSplines( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  EditedEvent = NULL;
}

CapexTimelineEventSplines::~CapexTimelineEventSplines()
{

}

void CapexTimelineEventSplines::UpdateData()
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
  if ( !l ) return;

  CapexSplineEditor_phx *e = (CapexSplineEditor_phx *)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( !e ) return;

  CArray<TS32> Selected;
  for ( TS32 x = 0; x < l->NumItems(); x++ )
  {
    if ( l->GetItemByIndex( x ).IsSelected() )
      Selected += x;
  }

  e->FlushSplines();
  l->Flush();
  SplineLinks.Flush();

  if ( !EditedEvent ) return;
  if ( Project.Timeline->Events.Find( EditedEvent ) < 0 )
  {
    EditedEvent = NULL;
    return;
  }

  for ( TS32 x = 0; x < EditedEvent->GetSplineCount(); x++ )
  {
    CString name;
    CColor color;
    CphxSpline_Tool_float16 *spline = EditedEvent->GetSpline( x, name, color );
    if ( spline )
    {
      TS32 id = l->AddItem( name );
      SplineLinks[ id ] = spline;
      if ( Selected.Find( x ) >= 0 )
        l->SelectItem( id );
    }
  }

}

TBOOL CapexTimelineEventSplines::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
    if ( l && Message.GetTarget() == l->GetGuid() )
    {
      CapexSplineEditor_phx *e = (CapexSplineEditor_phx *)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
      if ( e )
      {
        e->FlushSplines();

        for ( TS32 x = 0; x < l->NumItems(); x++ )
        {
          CWBSelectableItem &i = l->GetItemByIndex( x );
          if ( i.IsSelected() )
            if ( SplineLinks.HasKey( i.GetID() ) )
            {
              CphxSpline_Tool_float16 *spl = SplineLinks[ i.GetID() ];
              CColor col = CColor::FromARGB( 0xffffffff );
              CString name = i.GetText();
              name.ToLower();
              if ( name.Find( "red" ) >= 0 ) col = CColor::FromARGB( 0xffff0000 );
              if ( name.Find( "green" ) >= 0 ) col = CColor::FromARGB( 0xff00ff00 );
              if ( name.Find( "blue" ) >= 0 ) col = CColor::FromARGB( 0xff0000ff );

              e->AddSpline( spl, col, i.GetText() );
            }
        }
      }
    }
    return true;
  }

  case WBM_TIMEPOSITIONCHANGED:
  {
    CapexSplineEditor_phx *e = (CapexSplineEditor_phx *)App->FindItemByGuid( Message.GetTarget(), _T( "splineeditorphx" ) );
    if ( !e ) return true;
    if ( !EditedEvent ) return true;

    TF32 t = e->GetTimePosition();
    TS32 frame = (TS32)Lerp( (TF32)EditedEvent->Event->StartFrame, (TF32)EditedEvent->Event->EndFrame - 1, t );

    Project.SeekToFrame( frame );

    return true;
  }

  break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTimelineEventSplines::UpdateSplines( CphxEvent_Tool *ev )
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
  if ( !l ) return;
  CapexSplineEditor_phx *e = (CapexSplineEditor_phx *)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( !e ) return;

  if ( EditedEvent == ev )
    return;

  bool selctionChanged = EditedEvent != ev;
  EditedEvent = ev;

  l->Flush();
  SplineLinks.Flush();
  e->FlushSplines();

  if ( !ev ) return;

  for ( TS32 x = 0; x < ev->GetSplineCount(); x++ )
  {
    CString name;
    CColor color;
    CphxSpline_Tool_float16 *spline = ev->GetSpline( x, name, color );
    if ( spline )
    {
      TS32 id = l->AddItem( name );
      SplineLinks[ id ] = spline;

      if ( selctionChanged && name == _T( "Time Envelope" ) )
        l->SelectItemByIndex( l->NumItems() - 1 );
    }
  }
}

void CapexTimelineEventSplines::OnDraw( CWBDrawAPI *API )
{
  CapexSplineEditor_phx *e = (CapexSplineEditor_phx *)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( e && EditedEvent )
    e->SetTimePosition( max( 0, min( 1, ( Project.GetFrameToRender() - EditedEvent->Event->StartFrame ) / (TF32)( EditedEvent->Event->EndFrame - EditedEvent->Event->StartFrame ) ) ) );

  CapexWindow::OnDraw( API );
}

