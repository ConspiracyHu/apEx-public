#include "BasePCH.h"
#include "ProjectSettings.h"
#define WINDOWNAME _T("Project Settings")
#define WINDOWXML _T("ProjectSettings")
#include "../Phoenix_Tool/apxProject.h"
#include "../Phoenix/Timeline.h"

CapexProjectSettings::CapexProjectSettings() : CapexWindow()
{
}

CapexProjectSettings::CapexProjectSettings( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexProjectSettings::~CapexProjectSettings()
{

}

void CapexProjectSettings::UpdateData()
{
  CWBTextBox *i = (CWBTextBox*)FindChildByID( _T( "framerate" ), _T( "textbox" ) );
  if ( i ) i->SetText( CString::Format( _T( "%d" ), Project.Timeline->Timeline->FrameRate ) );

  i = (CWBTextBox*)FindChildByID( _T( "aspectx" ), _T( "textbox" ) );
  if ( i ) i->SetText( CString::Format( _T( "%d" ), Project.Timeline->Timeline->AspectX ) );

  i = (CWBTextBox*)FindChildByID( _T( "aspecty" ), _T( "textbox" ) );
  if ( i ) i->SetText( CString::Format( _T( "%d" ), Project.Timeline->Timeline->AspectY ) );

  i = (CWBTextBox*)FindChildByID( _T( "title" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Title );

  i = (CWBTextBox*)FindChildByID( _T( "group" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Group );

  i = (CWBTextBox*)FindChildByID( _T( "www" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Urls[ 0 ] );

  i = (CWBTextBox*)FindChildByID( _T( "pouet" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Urls[ 1 ] );

  i = (CWBTextBox*)FindChildByID( _T( "facebook" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Urls[ 2 ] );

  i = (CWBTextBox*)FindChildByID( _T( "youtube" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Urls[ 3 ] );

  i = (CWBTextBox*)FindChildByID( _T( "share" ), _T( "textbox" ) );
  if ( i ) i->SetText( Project.Urls[ 4 ] );
}

TBOOL CapexProjectSettings::MessageProc( CWBMessage &Message )
{

  switch ( Message.GetMessage() )
  {
  case WBM_TEXTCHANGED:
  {
    CWBTextBox *b = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "framerate" ) )
    {
      TS32 v = 0;
      if ( b->GetText().Scan( _T( "%d" ), &v ) == 1 )
        if ( v != 0 ) Project.Timeline->Timeline->FrameRate = v;
      return true;
    }

    if ( b->GetID() == _T( "aspectx" ) )
    {
      TS32 v = 0;
      if ( b->GetText().Scan( _T( "%d" ), &v ) == 1 )
        if ( v != 0 ) Project.Timeline->Timeline->AspectX = v;
      return true;
    }

    if ( b->GetID() == _T( "aspecty" ) )
    {
      TS32 v = 0;
      if ( b->GetText().Scan( _T( "%d" ), &v ) == 1 )
        if ( v != 0 ) Project.Timeline->Timeline->AspectY = v;
      return true;
    }

    if ( b->GetID() == _T( "title" ) )
    {
      Project.Title = b->GetText();
      return true;
    }

    if ( b->GetID() == _T( "group" ) )
    {
      Project.Group = b->GetText();
      return true;
    }

    if ( b->GetID() == _T( "www" ) )
    {
      Project.Urls[ 0 ] = b->GetText();
      return true;
    }

    if ( b->GetID() == _T( "pouet" ) )
    {
      Project.Urls[ 1 ] = b->GetText();
      return true;
    }

    if ( b->GetID() == _T( "facebook" ) )
    {
      Project.Urls[ 2 ] = b->GetText();
      return true;
    }

    if ( b->GetID() == _T( "youtube" ) )
    {
      Project.Urls[ 3 ] = b->GetText();
      return true;
    }

    if ( b->GetID() == _T( "share" ) )
    {
      Project.Urls[ 4 ] = b->GetText();
      return true;
    }
  }
  break;
  }

  return CapexWindow::MessageProc( Message );
}

