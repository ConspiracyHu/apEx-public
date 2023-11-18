#include "BasePCH.h"
#include "SceneSplineEditor.h"
#define WINDOWNAME _T("Scene Spline Editor")
#define WINDOWXML _T("SceneSplineEditor")
#include "WorkBench.h"
#include "SplineEditor_Phx.h"

CString SplineNames[] =
{
  "Material Parameter",
  "Scale X",
  "Scale Y",
  "Scale Z",
  "Rotation",

  "SubScene Clip",
  "SubScene Time",
  "",

  "Position X",
  "Position Y",
  "Position Z",
  "",

  "Light Ambient R",
  "Light Ambient G",
  "Light Ambient B",
  "",

  "Light Diffuse R",
  "Light Diffuse G",
  "Light Diffuse B",
  "",

  "Light Specular R",
  "Light Specular G",
  "Light Specular B",
  "",

  "",
  "",
  "",
  "",

  "Light Exponent",
  "Light Cutoff",
  "Light Att. Linear",
  "Light Att. Quadratic",
  "",

  "Camera FOV",
  "Camera Roll",
  "",
  "",

  "Particle Offset X",
  "Particle Offset Y",
  "Particle Offset Z",
  "Emissions Per Second",
  "Particle Emission Trigger",
  "Emission Velocity",
  "Particle Life (sec)",
  "Rotation Speed",
  "Velocity Chaos",
  "Rotation Chaos",
  "Life Chaos",

  "Shadow Volume Width",
  "Shadow Volume Height",

  "Affector Power",
  "Particle Scale",
  "Particle Scale Chaos",
  "Particle Stretch X",
  "Particle Stretch Y",

  "Replicate Count",
  "Replicate Time Offset",
};

CColor SplineColors[] =
{
  CColor::FromARGB( 0xffffff00 ), // "Material Parameter",
  CColor::FromARGB( 0xffff8080 ), // "Scale X",
  CColor::FromARGB( 0xff80ff80 ), // "Scale Y",
  CColor::FromARGB( 0xff8080ff ), // "Scale Z",
  CColor::FromARGB( 0xffff00ff ), // "Rotation",

  CColor::FromARGB( 0xff00ffff ), // "SubScene Clip",
  CColor::FromARGB( 0xff00ffff ), // "SubScene Time",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffff8080 ), // "Position X",
  CColor::FromARGB( 0xff80ff80 ), // "Position Y",
  CColor::FromARGB( 0xff8080ff ), // "Position Z",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffff0000 ), // "Light Ambient R",
  CColor::FromARGB( 0xff00ff00 ), // "Light Ambient G",
  CColor::FromARGB( 0xff0000ff ), // "Light Ambient B",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffff0000 ), // "Light Diffuse R",
  CColor::FromARGB( 0xff00ff00 ), // "Light Diffuse G",
  CColor::FromARGB( 0xff0000ff ), // "Light Diffuse B",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffff0000 ), // "Light Specular R",
  CColor::FromARGB( 0xff00ff00 ), // "Light Specular G",
  CColor::FromARGB( 0xff0000ff ), // "Light Specular B",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffffffff ), // "",
  CColor::FromARGB( 0xffffffff ), // "",
  CColor::FromARGB( 0xffffffff ), // "",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffffffff ), // "Light Exponent",
  CColor::FromARGB( 0xffffffff ), // "Light Cutoff",
  CColor::FromARGB( 0xffffffff ), // "Light Att. Linear",
  CColor::FromARGB( 0xffffffff ), // "Light Att. Quadratic",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffffffff ), // "Camera FOV",
  CColor::FromARGB( 0xffffffff ), // "Camera Roll",
  CColor::FromARGB( 0xffffffff ), // "",
  CColor::FromARGB( 0xffffffff ), // "",

  CColor::FromARGB( 0xffff8080 ), // "Particle Offset X",
  CColor::FromARGB( 0xff80ff80 ), // "Particle Offset Y",
  CColor::FromARGB( 0xff8080ff ), // "Particle Offset Z",
  CColor::FromARGB( 0xffffffff ), // "Particle PerFrameEmission",
  CColor::FromARGB( 0xffffffff ), // "Particle EmissionTrigger",
  CColor::FromARGB( 0xff00ffff ), // "Particle EmissionVelocity",
  CColor::FromARGB( 0xff00ffff ), // "Particle Life",
  CColor::FromARGB( 0xff00ffff ), // "Rotation Speed",
  CColor::FromARGB( 0xff00ffff ), // "Velocity Chaos",
  CColor::FromARGB( 0xff00ffff ), // "Rotation Chaos",
  CColor::FromARGB( 0xff00ffff ), // "Life Chaos",

  CColor::FromARGB( 0xff808080 ), // "Shadow Volume Width",
  CColor::FromARGB( 0xff808080 ), // "Shadow Volume Height",

  CColor::FromARGB( 0xff00ff80 ), // "Affector Power",
  CColor::FromARGB( 0xff00ff80 ), // "Particle Scale",
  CColor::FromARGB( 0xff00ff80 ), // "Particle Scale Chaos",
  CColor::FromARGB( 0xff00ff80 ), // "Particle Stretch X",
  CColor::FromARGB( 0xff00ff80 ), // "Particle Stretch Y",

  CColor::FromARGB( 0xffffff80 ), // "Subscene repeat count",
  CColor::FromARGB( 0xffffff80 ), // "Subscene repeat time offset",
};


CapexSceneSplineEditor::CapexSceneSplineEditor() : CapexWindow()
{
  SetAutoKey( false );
  SetSnapToGrid( true );
}

CapexSceneSplineEditor::CapexSceneSplineEditor( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  SetAutoKey( false );
  SetSnapToGrid( true );
}

CapexSceneSplineEditor::~CapexSceneSplineEditor()
{

}

void CapexSceneSplineEditor::UpdateData()
{
  CphxScene_Tool *s = WorkBench->GetEditedScene();
  if ( !s ) return;

  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
  if ( !l ) return;

  CArray<SPLINEMAPDATA> SelectedSplines;
  for ( TS32 x = 0; x < l->NumItems(); x++ )
  {
    if ( l->GetItemByIndex( x ).IsSelected() )
      if ( SplineMap.HasKey( l->GetItemByIndex( x ).GetID() ) )
        SelectedSplines.Add( SplineMap[ l->GetItemByIndex( x ).GetID() ] );
  }

  SplineMap.Flush();

  l->Flush();

  for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
  {
    if ( s->GetObjectByIndex( x )->Selected )
    {
      TS32 clip = s->GetActiveClip();
      CphxObject_Tool *o = s->GetObjectByIndex( x );
      CphxObjectClip_Tool* c = o->GetClip( clip );
      CphxObjectClip_Tool* c0 = o->GetClip( 0 );

      for ( TS32 y = 0; y < c->GetSplineCount(); y++ )
      {
        CphxClipSpline_Tool *spline = c->GetSplineByIndex( y );

        CString Text = SplineNames[ spline->MinimalSpline.Type ];

        CphxGUID g;

        if ( spline->MinimalSpline.Type == Spline_MaterialParam )
        {
          Text = spline->Parameter->Name;
          g = spline->Parameter->GetGUID();
        }

        TS32 idx = l->AddItem( Text );

        SPLINEMAPDATA sd = SPLINEMAPDATA( spline->Spline, spline->MinimalSpline.Type, g, NULL );
        sd.Color = SplineColors[ spline->MinimalSpline.Type ];
        SplineMap[ idx ] = sd;

        CWBSelectableItem *i = l->GetItem( idx );
        if ( SelectedSplines.Find( sd ) >= 0 )
          i->Select( true );

      }

      if ( o->GetObjectType() == Object_Model )
      {
        for ( TS32 y = 0; y < c->GetMaterialSplineCount(); y++ )
        {
          CphxMaterialSpline_Tool *spline = c->GetMaterialSplineByIndex( y );
          CphxMaterialParameter_Tool *p = Project.GetMaterialParameter( spline->TargetParamGUID );
          CphxModelObject_Tool *obj = Project.GetModelObject( spline->TargetObjectGUID );

          if ( !obj ) continue;
          if ( !p ) continue;

          CphxGUID g = p->GetGUID();

          if ( p->Parameter.Scope != PARAM_ANIMATED ) continue;
          if ( p->Parameter.Type == PARAM_FLOAT )
          {
            TS32 idx = l->AddItem( obj->GetName() + _T( " " ) + p->Name );
            SPLINEMAPDATA sd = SPLINEMAPDATA( &spline->Splines[ 0 ], Spline_MaterialParam, g, obj );
            sd.Color = SplineColors[ Spline_MaterialParam ];
            SplineMap[ idx ] = sd;
            CWBSelectableItem *i = l->GetItem( idx );
            if ( SelectedSplines.Find( sd ) >= 0 )
              i->Select( true );
          }
          if ( p->Parameter.Type == PARAM_COLOR )
          {
            CString ColorNames[ 4 ] = { CString( " red" ), CString( " green" ), CString( " blue" ), CString( " alpha" ) };
            for ( int z = 0; z < 4; z++ )
            {
              TS32 idx = l->AddItem( obj->GetName() + _T( " " ) + p->Name + ColorNames[ z ] );

              CColor Colors[ 4 ] =
              {
                CColor::FromARGB( 0xffff0000 ),
                CColor::FromARGB( 0xff00ff00 ),
                CColor::FromARGB( 0xff0000ff ),
                CColor::FromARGB( 0x80ffffff ),
              };

              SPLINEMAPDATA sd = SPLINEMAPDATA( &spline->Splines[ z ], Spline_MaterialParam, g, z, Colors[ z ], obj );
              SplineMap[ idx ] = sd;
              CWBSelectableItem *i = l->GetItem( idx );
              if ( SelectedSplines.Find( sd ) >= 0 )
                i->Select( true );
            }
          }
        }
      }

      if ( o->GetObjectType() == Object_ParticleEmitterCPU )
      {
        for ( TS32 y = 0; y < c->GetMaterialSplineCount(); y++ )
        {
          CphxMaterialSpline_Tool *spline = c->GetMaterialSplineByIndex( y );
          CphxMaterialSpline_Tool *splineClip0 = c0->GetMaterialSplineByIndex( y );
          CphxMaterialParameter_Tool *p = Project.GetMaterialParameter( spline->TargetParamGUID );

          if ( ( spline && !splineClip0 ) || ( !spline && splineClip0 ) || ( spline->TargetParamGUID != splineClip0->TargetParamGUID ) )
          {
            LOG_ERR( "[particle] Spline Clip discrepancy between clip 0 and clip %d", clip );
            continue;
          }

          if ( !p ) continue;

          CphxGUID g = p->GetGUID();

          if ( p->Parameter.Scope != PARAM_ANIMATED ) continue;

          if ( p->Parameter.Type == PARAM_FLOAT )
          {
            TS32 idx = l->AddItem( p->Name );
            SPLINEMAPDATA sd = SPLINEMAPDATA( &spline->Splines[ 0 ], Spline_MaterialParam, g, NULL );
            sd.Color = SplineColors[ Spline_MaterialParam ];
            SplineMap[ idx ] = sd;
            CWBSelectableItem *i = l->GetItem( idx );
            if ( SelectedSplines.Find( sd ) >= 0 )
              i->Select( true );
          }

          if ( p->Parameter.Type == PARAM_COLOR )
          {
            CString ColorNames[ 4 ] = { CString( " red" ), CString( " green" ), CString( " blue" ), CString( " alpha" ) };
            for ( int z = 0; z < 4; z++ )
            {
              TS32 idx = l->AddItem( p->Name + ColorNames[ z ] );

              CColor Colors[ 4 ] =
              {
                CColor::FromARGB( 0xffff0000 ),
                CColor::FromARGB( 0xff00ff00 ),
                CColor::FromARGB( 0xff0000ff ),
                CColor::FromARGB( 0x80ffffff ),
              };

              SPLINEMAPDATA sd = SPLINEMAPDATA( &spline->Splines[ z ], Spline_MaterialParam, g, z, Colors[ z ], NULL );
              SplineMap[ idx ] = sd;
              CWBSelectableItem *i = l->GetItem( idx );
              if ( SelectedSplines.Find( sd ) >= 0 )
                i->Select( true );
            }
          }

          if ( p->Parameter.Type == PARAM_PARTICLELIFEFLOAT )
          {
            TS32 idx = l->AddItem( p->Name );
            SPLINEMAPDATA sd = SPLINEMAPDATA( &splineClip0->Splines[ 0 ], Spline_MaterialParam, g, NULL );
            sd.Color = SplineColors[ Spline_MaterialParam ];
            SplineMap[ idx ] = sd;
            CWBSelectableItem *i = l->GetItem( idx );
            if ( SelectedSplines.Find( sd ) >= 0 )
              i->Select( true );
          }
        }
      }

      UpdateSplines();
      return;
    }
  }

  UpdateSplines();
}

TBOOL CapexSceneSplineEditor::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_SELECTIONCHANGE:
  {
    CWBItemSelector *b = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "splinelist" ) )
      UpdateSplines();
  }
  break;
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "autokey" ) )
    {
      SetAutoKey( !IsAutoKeySet() );
      return true;
    }

    if ( b->GetID() == _T( "snaptogrid" ) )
    {
      SetSnapToGrid( !IsSnapToGridSet() );
      return true;
    }

    if ( b->GetID() == _T( "limitsubscene" ) )
    {
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
      if ( !l ) return true;

      for ( int x = 0; x < l->NumItems(); x++ )
      {
        if ( l->GetItemByIndex( x ).IsSelected() && SplineMap.HasKey( l->GetItemByIndex( x ).GetID() ) )
        {
          auto map = SplineMap[ l->GetItemByIndex( x ).GetID() ];
          if ( map.Type == Spline_SubScene_Time )
          {
            CphxSpline_Tool_float16* sp = (CphxSpline_Tool_float16*)map.Spline;
            for ( int y = 0; y < sp->Keys.NumItems(); y++ )
            {
              sp->Keys[ y ]->Key.Value[ 0 ] = max( 0, min( 0.9999f, sp->Keys[ y ]->Key.Value[ 0 ] ) );
              sp->Spline->Keys[ y ]->Value[ 0 ] = max( 0, min( 0.9999f, sp->Spline->Keys[ y ]->Value[ 0 ] ) );
            }
            App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
          }
        }
      }

      return true;
    }

    if ( b->GetID() == _T( "multiplicative" ) )
    {
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
      if ( !l ) return true;

      if ( l->GetCursorItem() && SplineMap.HasKey( l->GetCursorItem()->GetID() ) )
      {
        auto map = SplineMap[ l->GetCursorItem()->GetID() ];
        if ( map.Type != Spline_Rotation )
        {
          CphxSpline_float16* sp = (CphxSpline_float16*)map.Spline->Spline;
          sp->MultiplicativeWaveform = !sp->MultiplicativeWaveform;
          b->Push( sp->MultiplicativeWaveform );
          App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
        }
      }
      return true;
    }

  }
  break;

  case WBM_VALUECHANGED:
  {
    CWBTrackBar *b = (CWBTrackBar*)App->FindItemByGuid( Message.GetTarget(), _T( "trackbar" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "randseed" ) )
    {
      b->SetText( CString::Format( "RandSeed: %d", Message.Data ) );

      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
      if ( !l ) return true;

      if ( l->GetCursorItem() && SplineMap.HasKey( l->GetCursorItem()->GetID() ) )
      {
        auto map = SplineMap[ l->GetCursorItem()->GetID() ];
        if ( map.Type != Spline_Rotation )
        {
          CphxSpline_float16* sp = (CphxSpline_float16*)map.Spline->Spline;
          sp->RandSeed = b->GetValueAsInt();
          sp->NoiseCalculated = false;
          App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
        }
      }
      return true;
    }

  }
  break;

  case WBM_ITEMSELECTED:
  {
    CWBDropDown *d = (CWBDropDown*)FindChildByID( "waveform", "dropdown" );
    if ( d && d->GetGuid() == Message.GetTarget() )
    {
      CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
      if ( !l ) return true;

      if ( l->GetCursorItem() && SplineMap.HasKey( l->GetCursorItem()->GetID() ) )
      {
        auto map = SplineMap[ l->GetCursorItem()->GetID() ];
        if ( map.Type != Spline_Rotation )
        {
          CphxSpline_float16* sp = (CphxSpline_float16*)map.Spline->Spline;
          sp->Waveform = (SPLINEWAVEFORM)d->GetCursorPosition();
          App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
        }
      }
      return true;
    }
  }
  break;

  case WBM_TEXTCHANGED:
  {
    CWBTextBox *b = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( !b ) break;

    CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
    if ( !l ) return true;

    if ( b->GetID() == _T( "amplitude" ) )
    {
      float value = 0;
      if ( l->GetCursorItem() && b->GetText().Scan( "%f", &value ) == 1 )
      {
        if ( SplineMap.HasKey( l->GetCursorItem()->GetID() ) )
        {
          auto map = SplineMap[ l->GetCursorItem()->GetID() ];
          if ( map.Type != Spline_Rotation )
          {
            CphxSpline_float16* sp = (CphxSpline_float16*)map.Spline->Spline;
            sp->WaveformAmplitude = value;
            App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
          }
        }
      }
      return true;
    }

    if ( b->GetID() == _T( "frequency" ) )
    {
      float value = 0;
      if ( l->GetCursorItem() && b->GetText().Scan( "%f", &value ) == 1 )
      {
        if ( SplineMap.HasKey( l->GetCursorItem()->GetID() ) )
        {
          auto map = SplineMap[ l->GetCursorItem()->GetID() ];
          if ( map.Type != Spline_Rotation )
          {
            CphxSpline_float16* sp = (CphxSpline_float16*)map.Spline->Spline;
            sp->WaveformFrequency = value;
            sp->NoiseCalculated = false;
            App->SendMessage( CWBMessage( App, (WBMESSAGE)WBM_SPLINECHANGED, GetGuid() ) );
          }
        }
      }
      return true;
    }
  }
  break;

  case WBM_SPLINECHANGED:
  {
    CphxScene_Tool *s = GetWorkBench()->GetEditedScene();
    if ( !s ) break;

    CphxObject_Tool *o = NULL;

    for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
    {
      if ( s->GetObjectByIndex( x )->Selected )
      {
        o = s->GetObjectByIndex( x );
        break;
      }
    }

    if ( !o ) break;

    if ( o->GetObjectType() == Object_SubScene )
    {
      for ( int x = 0; x < SplineMap.NumItems(); x++ )
      {
        auto& spl = SplineMap.GetByIndex( x );
        if ( spl.Type == Spline_SubScene_Clip )
        {
          spl.Spline->Spline->Interpolation = INTERPOLATION_CONSTANT;
          CphxObject_SubScene_Tool *ss = (CphxObject_SubScene_Tool*)o;
          int maxVal = ss->GetClip(s->GetActiveClip())->GetSubSceneTarget()->GetClipCount() - 1;
          for ( int y = 0; y < spl.Spline->Keys.NumItems(); y++ )
          {
            spl.Spline->Keys[ y ]->Key.Value[ 0 ] = (float)( (int)max( 0, min( maxVal, (float)spl.Spline->Keys[ y ]->Key.Value[ 0 ] ) ) );
            spl.Spline->Spline->Keys[ y ]->Value[ 0 ] = spl.Spline->Keys[ y ]->Key.Value[ 0 ];
          }
        }
      }
    }

    if ( o->GetObjectType() == Object_ParticleEmitterCPU )
    {
#ifdef MEMORY_TRACKING
      memTracker.Pause();
#endif
      ( (CphxObject_ParticleEmitter_CPU*)o->GetObject() )->UpdateSplineTexture();
#ifdef MEMORY_TRACKING
      memTracker.Resume();
#endif
    }
    break;

  }
  break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexSceneSplineEditor::UpdateSplines()
{
  CWBItemSelector *l = (CWBItemSelector*)FindChildByID( _T( "splinelist" ), _T( "itemselector" ) );
  if ( !l ) return;
  CapexSplineEditor_phx *e = (CapexSplineEditor_phx*)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( !e ) return;
  e->FlushSplines();

  CphxScene_Tool *s = GetWorkBench()->GetEditedScene();
  if ( !s ) return;

  CphxObject_Tool *o = NULL;

  for ( TS32 x = 0; x < s->GetObjectCount(); x++ )
  {
    if ( s->GetObjectByIndex( x )->Selected )
    {
      o = s->GetObjectByIndex( x );
      break;
    }
  }

  if ( !o ) return;

  for ( TS32 x = 0; x < l->NumItems(); x++ )
    if ( l->GetItemByIndex( x ).IsSelected() )
      if ( SplineMap.HasKey( l->GetItemByIndex( x ).GetID() ) )
      {
        e->AddSpline( SplineMap[ l->GetItemByIndex( x ).GetID() ].Spline, SplineMap[ l->GetItemByIndex( x ).GetID() ].Color, l->GetItemByIndex( x ).GetText() );
        //CphxObjectClip_Tool* c = o->GetClip(s->GetActiveClip());
        //e->AddSpline(c->GetSplineByIndex(x)->Spline);
      }

  SPLINEWAVEFORM waveform = WAVEFORM_NONE;
  float amplitude = 0;
  float frequency = 0;
  unsigned char randseed = 0;
  bool multiplicative = false;

  if ( l->GetCursorItem() && SplineMap.HasKey( l->GetCursorItem()->GetID() ) )
  {
    auto map = SplineMap[ l->GetCursorItem()->GetID() ];

    if ( map.Type != Spline_Rotation )
    {
      CphxSpline_float16* sp = (CphxSpline_float16*)map.Spline->Spline;
      waveform = sp->Waveform;
      amplitude = sp->WaveformAmplitude;
      frequency = sp->WaveformFrequency;
      randseed = sp->RandSeed;
      multiplicative = sp->MultiplicativeWaveform;
    }
  }

  CWBDropDown *wb = (CWBDropDown*)FindChildByID( "waveform", "dropdown" );
  if ( wb )
    wb->SelectItemByIndex( waveform );

  CWBTextBox* tb = (CWBTextBox*)FindChildByID( "amplitude", "textbox" );
  if ( tb )
    tb->SetText( CString::Format( "%f", amplitude ) );

  tb = (CWBTextBox*)FindChildByID( "frequency", "textbox" );
  if ( tb )
    tb->SetText( CString::Format( "%f", frequency ) );

  CWBTrackBar* rs = (CWBTrackBar*)FindChildByID( "randseed", "trackbar" );
  if ( rs )
    rs->SetValue( randseed );

  CWBButton* b = (CWBButton*)FindChildByID( "multiplicative", "button" );
  if ( b )
    b->Push( multiplicative );
}

TF32 CapexSceneSplineEditor::GetTimePos()
{
  CapexSplineEditor_phx *e = (CapexSplineEditor_phx*)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( !e ) return 0;
  return e->GetTimePosition();
}

TBOOL CapexSceneSplineEditor::IsAutoKeySet()
{
  return AutoKey;
}

void CapexSceneSplineEditor::SetAutoKey( TBOOL ak )
{
  CWBButton *b = (CWBButton*)FindChildByID( _T( "autokey" ), _T( "button" ) );
  if ( b ) b->Push( ak );
  AutoKey = ak;
}

void CapexSceneSplineEditor::SetSnapToGrid( TBOOL ak )
{
  CWBButton *b = (CWBButton*)FindChildByID( _T( "snaptogrid" ), _T( "button" ) );
  if ( b ) b->Push( ak );
  SnapToGrid = ak;

  CapexSplineEditor_phx *e = (CapexSplineEditor_phx*)FindChildByID( _T( "splineeditor" ), _T( "splineeditorphx" ) );
  if ( !e ) return;
  e->SnapToGrid = ak;
}

TBOOL CapexSceneSplineEditor::IsSnapToGridSet()
{
  return SnapToGrid;
}

void CapexSceneSplineEditor::OnDraw( CWBDrawAPI *API )
{
  WBSKINELEMENTID Active = GetDisplayDescriptor().GetSkin( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDIMAGE );
  WBSKINELEMENTID Normal = GetDisplayDescriptor().GetSkin( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE );
  WBSKINELEMENTID Hover = GetDisplayDescriptor().GetSkin( WB_STATE_HOVER, WB_ITEM_BACKGROUNDIMAGE );

  if ( AutoKey )
  {
    WBSKINELEMENTID Red = App->GetSkin()->GetElementID( CString( _T( "window_red" ) ) );

    GetDisplayDescriptor().SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDIMAGE, Red );
    GetDisplayDescriptor().SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE, Red );
    GetDisplayDescriptor().SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDIMAGE, Red );
  }

  CapexWindow::OnDraw( API );

  GetDisplayDescriptor().SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDIMAGE, Active );
  GetDisplayDescriptor().SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE, Normal );
  GetDisplayDescriptor().SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDIMAGE, Hover );
}
