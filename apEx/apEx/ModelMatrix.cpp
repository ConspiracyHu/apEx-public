#include "BasePCH.h"
#include "ModelMatrix.h"
#define WINDOWNAME _T("Model Matrix")
#define WINDOWXML _T("ModelMatrix")
#include "WorkBench.h"
#include "apExRoot.h"

extern CapexRoot *Root;

void CapexModelMatrix::LoadCSS()
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

CapexModelMatrix::CapexModelMatrix() : CapexWindow()
{
  LoadCSS();
}

CapexModelMatrix::CapexModelMatrix( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  LoadCSS();
}

void CapexModelMatrix::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}


CapexModelMatrix::~CapexModelMatrix()
{

}

void CapexModelMatrix::UpdateData()
{
  if ( !WorkBench->GetEditedModel() || WorkBench->GetEditedModel()->GetObjectIndex( WorkBench->GetEditedModelObject() ) < 0 )
  {
    ClearValues();
    return;
  }

  if ( WorkBench->GetEditedModelFilter() && WorkBench->GetEditedModelFilter()->Filter == ModelFilter_Replicate )
  {
    SetMatrix( WorkBench->GetEditedModelFilter()->GetRawMatrix() );
    return;
  }

  if ( WorkBench->GetEditedModelFilter() && ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap ||
                                              WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm ||
                                              WorkBench->GetEditedModelFilter()->Filter == ModelFilter_TintMeshShape ) )
  {
    D3DXFLOAT16 srt[ 12 ];
    memcpy( srt, WorkBench->GetEditedModelFilter()->srt, sizeof( D3DXFLOAT16 ) * 12 );

    if ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap || WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm )
    {
      srt[ 0 ] = srt[ 0 ] + 1.0f;
      srt[ 1 ] = srt[ 1 ] + 1.0f;
      srt[ 2 ] = srt[ 2 ] + 1.0f;
    }

    SetSRT( srt );
    return;
  }

  if ( WorkBench->GetEditedModelObject() )
    SetMatrix( WorkBench->GetEditedModelObject()->GetMatrix() );
}

TBOOL CapexModelMatrix::MessageProc( CWBMessage &Message )
{

  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
  {
    switch ( Message.Key )
    {
    case VK_DELETE:
    {
      if ( WorkBench->GetEditedModel() )
      {
        WorkBench->GetEditedModel()->DeleteSelected();
        WorkBench->UpdateWindows( apEx_ModelGraph );
        WorkBench->UpdateWindows( apEx_ModelParameters );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
    }
    return true;
    break;
    case 'C':
    {
      if ( WorkBench->GetEditedModel() )
      {
        WorkBench->GetEditedModel()->CopySelected();
        WorkBench->UpdateWindows( apEx_ModelGraph );
        WorkBench->UpdateWindows( apEx_ModelParameters );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
    }
    return true;
    break;
    }
  }
  break;
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b )
      break;

    if ( b->GetID() == _T( "applymatrix" ) )
    {
      D3DXMATRIX mx = GetMatrixFromSRT();

      if ( WorkBench->GetEditedModelFilter() && WorkBench->GetEditedModelFilter()->Filter == ModelFilter_Replicate )
      {
        WorkBench->GetEditedModelFilter()->SetRawMatrix( mx );
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        WorkBench->UpdateWindows( apEx_ModelMatrix );
        return true;
      }

      if ( WorkBench->GetEditedModelFilter() && ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap ||
                                                  WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm ||
                                                  WorkBench->GetEditedModelFilter()->Filter == ModelFilter_TintMeshShape ) )
      {
        D3DXFLOAT16 srt[ 10 ];
        GetSRT( srt );

        if ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap || WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm )
        {
          srt[ 0 ] = srt[ 0 ] - 1.0f;
          srt[ 1 ] = srt[ 1 ] - 1.0f;
          srt[ 2 ] = srt[ 2 ] - 1.0f;
        }

        memcpy( WorkBench->GetEditedModelFilter()->srt, srt, sizeof( D3DXFLOAT16 ) * 10 );
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        WorkBench->UpdateWindows( apEx_ModelMatrix );
        return true;
      }

      if ( Message.Data != 0 && WorkBench->GetEditedModelObject() )
      {
        WorkBench->GetEditedModelObject()->SetMatrix( mx );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }

      return true;
    }

    if ( b->GetID() == _T( "resetpos" ) )
    {
      D3DXMATRIX i;
      D3DXMatrixIdentity( &i );

      if ( WorkBench->GetEditedModelFilter() && WorkBench->GetEditedModelFilter()->Filter == ModelFilter_Replicate )
      {
        WorkBench->GetEditedModelFilter()->SetRawMatrix( i );
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        WorkBench->UpdateWindows( apEx_ModelMatrix );
        return true;
      }

      if ( Message.HumanEdit != 0 && WorkBench->GetEditedModelFilter() && ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap ||
                                                                            WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm ||
                                                                            WorkBench->GetEditedModelFilter()->Filter == ModelFilter_TintMeshShape ) )
      {

        D3DXFLOAT16 srt[ 10 ];
        memset( srt, 0, sizeof( srt ) );

        if ( !( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap || WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm ) )
        {
          srt[ 0 ] = 1.0f;
          srt[ 1 ] = 1.0f;
          srt[ 2 ] = 1.0f;
        }

        memcpy( WorkBench->GetEditedModelFilter()->srt, srt, sizeof( D3DXFLOAT16 ) * 10 );
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        WorkBench->UpdateWindows( apEx_ModelMatrix );
        return true;
      }

      if ( WorkBench->GetEditedModelObject() )
      {
        WorkBench->GetEditedModelObject()->SetMatrix( i );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
      return true;
    }

    break;
  }

  case WBM_TEXTCHANGED:
  {
    CWBTextBox *b = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( !b ) break;

    if ( Message.HumanEdit != 0 && !WorkBench->GetEditedModel() )
    {
      WorkBench->UpdateWindows( apEx_ModelMatrix );
      return true;
    }

    if ( Message.HumanEdit != 0 && WorkBench->GetEditedModelFilter() && WorkBench->GetEditedModelFilter()->Filter == ModelFilter_Replicate )
    {
      for ( TS32 x = 0; x < 4; x++ )
        for ( TS32 y = 0; y < 4; y++ )
        {
          CString s = CString::Format( _T( "tr%d%d" ), x, y );
          if ( b->GetID() == s )
          {
            D3DXMATRIX mx = WorkBench->GetEditedModelFilter()->GetRawMatrix();
            TF32 f = mx.m[ x ][ y ];
            b->GetText().Scan( "%f", &f );
            mx.m[ x ][ y ] = f;
            WorkBench->GetEditedModelFilter()->SetRawMatrix( mx );
            WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
            WorkBench->UpdateWindows( apEx_ModelMatrix );
            return true;
          }
        }
    }

    if ( Message.HumanEdit != 0 && WorkBench->GetEditedModelFilter() && ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap ||
                                                                     WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm ||
                                                                     WorkBench->GetEditedModelFilter()->Filter == ModelFilter_TintMeshShape ) )
    {
      if ( b->GetID() == _T( "tx" ) || b->GetID() == _T( "ty" ) || b->GetID() == _T( "tz" ) ||
           b->GetID() == _T( "sx" ) || b->GetID() == _T( "sy" ) || b->GetID() == _T( "sz" ) ||
           b->GetID() == _T( "rx" ) || b->GetID() == _T( "ry" ) || b->GetID() == _T( "rz" ) )
      {
        D3DXFLOAT16 srt[ 10 ];
        GetSRT( srt );

        if ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_UVMap || WorkBench->GetEditedModelFilter()->Filter == ModelFilter_MapXForm )
        {
          srt[ 0 ] = srt[ 0 ] - 1.0f;
          srt[ 1 ] = srt[ 1 ] - 1.0f;
          srt[ 2 ] = srt[ 2 ] - 1.0f;
        }

        memcpy( WorkBench->GetEditedModelFilter()->srt, srt, sizeof( D3DXFLOAT16 ) * 10 );
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        WorkBench->UpdateWindows( apEx_ModelMatrix );
        return true;
      }

      WorkBench->UpdateWindows( apEx_ModelMatrix );
      return true;
    }

    if ( Message.HumanEdit != 0 && WorkBench->GetEditedModelObject() ) //matrix editing
    {
      for ( TS32 x = 0; x < 4; x++ )
        for ( TS32 y = 0; y < 4; y++ )
        {
          CString s = CString::Format( _T( "tr%d%d" ), x, y );
          if ( b->GetID() == s )
          {
            D3DXMATRIX mx = WorkBench->GetEditedModelObject()->GetMatrix();
            TF32 f = mx.m[ x ][ y ];
            b->GetText().Scan( "%f", &f );
            mx.m[ x ][ y ] = f;
            WorkBench->GetEditedModelObject()->SetMatrix( mx );
            WorkBench->UpdateWindows( apEx_ModelMatrix );
            return true;
          }
        }
    }

    break;
  }

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

double rad2deg( double rad )
{
  return rad * 180.0 / PI;
}

static void toEulerAngle( const D3DXQUATERNION& q, double& roll, double& pitch, double& yaw )
{
  // roll (x-axis rotation)
  double sinr = +2.0 * ( q.w * q.x + q.y * q.z );
  double cosr = +1.0 - 2.0 * ( q.x * q.x + q.y * q.y );
  roll = atan2( sinr, cosr );

  // pitch (y-axis rotation)
  double sinp = +2.0 * ( q.w * q.y - q.z * q.x );
  if ( fabs( sinp ) >= 1 )
    pitch = copysign( PI / 2, sinp ); // use 90 degrees if out of range
  else
    pitch = asin( sinp );

  // yaw (z-axis rotation)
  double siny = +2.0 * ( q.w * q.z + q.x * q.y );
  double cosy = +1.0 - 2.0 * ( q.y * q.y + q.z * q.z );
  yaw = atan2( siny, cosy );
}

void CapexModelMatrix::SetMatrix( const D3DXMATRIX& mx )
{
  for ( TS32 x = 0; x < 4; x++ )
    for ( TS32 y = 0; y < 4; y++ )
    {
      CString s = CString::Format( _T( "tr%d%d" ), x, y );
      CWBTextBox *ib = (CWBTextBox *)FindChildByID( s, _T( "textbox" ) );
      if ( ib && !ib->InFocus() )
        ib->SetText( CString::Format( _T( "%f" ), mx.m[ x ][ y ] ) );
    }

  D3DXVECTOR3 scale = D3DXVECTOR3( 1, 1, 1 ), pos = D3DXVECTOR3( 0, 0, 0 );
  D3DXQUATERNION rot = D3DXQUATERNION( 1, 0, 0, 0 );

  if ( D3DXMatrixDecompose( &scale, &rot, &pos, &mx ) == S_OK )
  {
    D3DXVECTOR3 axis;
    FLOAT angle;

    D3DXQuaternionToAxisAngle( &rot, &axis, &angle );

    CWBTextBox* b = FindChildByID<CWBTextBox>( _T( "tx" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", pos.x ) );
    b = FindChildByID<CWBTextBox>( _T( "ty" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", pos.y ) );
    b = FindChildByID<CWBTextBox>( _T( "tz" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", pos.z ) );

    b = FindChildByID<CWBTextBox>( _T( "sx" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", scale.x ) );
    b = FindChildByID<CWBTextBox>( _T( "sy" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", scale.y ) );
    b = FindChildByID<CWBTextBox>( _T( "sz" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", scale.z ) );

    double y, p, r;
    toEulerAngle( rot, y, p, r );
    y = rad2deg( y );
    p = rad2deg( p );
    r = rad2deg( r );

    b = FindChildByID<CWBTextBox>( _T( "rx" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", y ) );
    b = FindChildByID<CWBTextBox>( _T( "ry" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", p ) );
    b = FindChildByID<CWBTextBox>( _T( "rz" ) );
    if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", r ) );
  }

}

void CapexModelMatrix::SetSRT( D3DXFLOAT16 srt[ 12 ] )
{
  for ( TS32 x = 0; x < 4; x++ )
    for ( TS32 y = 0; y < 4; y++ )
    {
      CString s = CString::Format( _T( "tr%d%d" ), x, y );
      CWBTextBox *ib = (CWBTextBox *)FindChildByID( s, _T( "textbox" ) );
      if ( ib )
        ib->SetText( _T( "-" ) );
    }

  D3DXVECTOR3 scale( srt[ 0 ], srt[ 1 ], srt[ 2 ] );
  D3DXQUATERNION rot( srt[ 3 ], srt[ 4 ], srt[ 5 ], srt[ 6 ] );
  D3DXVECTOR3 pos( srt[ 7 ], srt[ 8 ], srt[ 9 ] );

  double y, p, r;
  toEulerAngle( rot, y, p, r );
  y = rad2deg( y );
  p = rad2deg( p );
  r = rad2deg( r );

  CWBTextBox* b = FindChildByID<CWBTextBox>( _T( "tx" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", pos.x ) );
  b = FindChildByID<CWBTextBox>( _T( "ty" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", pos.y ) );
  b = FindChildByID<CWBTextBox>( _T( "tz" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", pos.z ) );

  b = FindChildByID<CWBTextBox>( _T( "sx" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", scale.x ) );
  b = FindChildByID<CWBTextBox>( _T( "sy" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", scale.y ) );
  b = FindChildByID<CWBTextBox>( _T( "sz" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", scale.z ) );

  b = FindChildByID<CWBTextBox>( _T( "rx" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", y ) );
  b = FindChildByID<CWBTextBox>( _T( "ry" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", p ) );
  b = FindChildByID<CWBTextBox>( _T( "rz" ) );
  if ( b && !b->InFocus() ) b->SetText( CString::Format( "%f", r ) );
}

void CapexModelMatrix::GetSRT( D3DXFLOAT16 srt[ 10 ] )
{
  D3DXVECTOR3 s = D3DXVECTOR3( 1, 1, 1 ), r = D3DXVECTOR3( 0, 0, 0 ), t = D3DXVECTOR3( 0, 0, 0 );

  CWBTextBox* b = FindChildByID<CWBTextBox>( _T( "tx" ) );
  if ( b ) b->GetText().Scan( "%f", &t.x );
  b = FindChildByID<CWBTextBox>( _T( "ty" ) );
  if ( b ) b->GetText().Scan( "%f", &t.y );
  b = FindChildByID<CWBTextBox>( _T( "tz" ) );
  if ( b ) b->GetText().Scan( "%f", &t.z );

  b = FindChildByID<CWBTextBox>( _T( "sx" ) );
  if ( b ) b->GetText().Scan( "%f", &s.x );
  b = FindChildByID<CWBTextBox>( _T( "sy" ) );
  if ( b ) b->GetText().Scan( "%f", &s.y );
  b = FindChildByID<CWBTextBox>( _T( "sz" ) );
  if ( b ) b->GetText().Scan( "%f", &s.z );

  b = FindChildByID<CWBTextBox>( _T( "rx" ) );
  if ( b ) b->GetText().Scan( "%f", &r.x );
  b = FindChildByID<CWBTextBox>( _T( "ry" ) );
  if ( b ) b->GetText().Scan( "%f", &r.y );
  b = FindChildByID<CWBTextBox>( _T( "rz" ) );
  if ( b ) b->GetText().Scan( "%f", &r.z );

  r = r * PI / 180.0f;

  D3DXQUATERNION rot;
  D3DXQuaternionRotationYawPitchRoll( &rot, r.y, r.x, r.z );

  srt[ 0 ] = s.x;
  srt[ 1 ] = s.y;
  srt[ 2 ] = s.z;

  srt[ 3 ] = rot.x;
  srt[ 4 ] = rot.y;
  srt[ 5 ] = rot.z;
  srt[ 6 ] = rot.w;

  srt[ 7 ] = t.x;
  srt[ 8 ] = t.y;
  srt[ 9 ] = t.z;
}

void CapexModelMatrix::ClearValues()
{
  for ( TS32 x = 0; x < 4; x++ )
    for ( TS32 y = 0; y < 4; y++ )
    {
      CString s = CString::Format( _T( "tr%d%d" ), x, y );
      CWBTextBox *ib = (CWBTextBox *)FindChildByID( s, _T( "textbox" ) );
      if ( ib )
        ib->SetText( _T( "-" ) );
    }

  CWBTextBox* b = FindChildByID<CWBTextBox>( _T( "tx" ) );
  if ( b ) b->SetText( _T( "-" ) );
  b = FindChildByID<CWBTextBox>( _T( "ty" ) );
  if ( b ) b->SetText( _T( "-" ) );
  b = FindChildByID<CWBTextBox>( _T( "tz" ) );
  if ( b ) b->SetText( _T( "-" ) );

  b = FindChildByID<CWBTextBox>( _T( "sx" ) );
  if ( b ) b->SetText( _T( "-" ) );
  b = FindChildByID<CWBTextBox>( _T( "sy" ) );
  if ( b ) b->SetText( _T( "-" ) );
  b = FindChildByID<CWBTextBox>( _T( "sz" ) );
  if ( b ) b->SetText( _T( "-" ) );

  b = FindChildByID<CWBTextBox>( _T( "rx" ) );
  if ( b ) b->SetText( _T( "-" ) );
  b = FindChildByID<CWBTextBox>( _T( "ry" ) );
  if ( b ) b->SetText( _T( "-" ) );
  b = FindChildByID<CWBTextBox>( _T( "rz" ) );
  if ( b ) b->SetText( _T( "-" ) );
}

D3DXMATRIX CapexModelMatrix::GetMatrixFromSRT()
{
  D3DXVECTOR3 s = D3DXVECTOR3( 1, 1, 1 ), r = D3DXVECTOR3( 0, 0, 0 ), t = D3DXVECTOR3( 0, 0, 0 );

  CWBTextBox* b = FindChildByID<CWBTextBox>( _T( "tx" ) );
  if ( b ) b->GetText().Scan( "%f", &t.x );
  b = FindChildByID<CWBTextBox>( _T( "ty" ) );
  if ( b ) b->GetText().Scan( "%f", &t.y );
  b = FindChildByID<CWBTextBox>( _T( "tz" ) );
  if ( b ) b->GetText().Scan( "%f", &t.z );

  b = FindChildByID<CWBTextBox>( _T( "sx" ) );
  if ( b ) b->GetText().Scan( "%f", &s.x );
  b = FindChildByID<CWBTextBox>( _T( "sy" ) );
  if ( b ) b->GetText().Scan( "%f", &s.y );
  b = FindChildByID<CWBTextBox>( _T( "sz" ) );
  if ( b ) b->GetText().Scan( "%f", &s.z );

  b = FindChildByID<CWBTextBox>( _T( "rx" ) );
  if ( b ) b->GetText().Scan( "%f", &r.x );
  b = FindChildByID<CWBTextBox>( _T( "ry" ) );
  if ( b ) b->GetText().Scan( "%f", &r.y );
  b = FindChildByID<CWBTextBox>( _T( "rz" ) );
  if ( b ) b->GetText().Scan( "%f", &r.z );

  r = r * PI / 180.0f;

  D3DXQUATERNION rot;
  D3DXQuaternionRotationYawPitchRoll( &rot, r.y, r.x, r.z );

  D3DXMATRIX result;
  D3DXMatrixTransformation( &result, &D3DXVECTOR3( 0, 0, 0 ), &D3DXQUATERNION( 1, 0, 0, 0 ), &s, &D3DXVECTOR3( 0, 0, 0 ), &rot, &t );

  return result;
}
