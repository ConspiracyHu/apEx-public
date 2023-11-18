#include "BasePCH.h"
#include "TexGenPreview.h"
#include "WorkBench.h"
#include "../../Bedrock/CoRE2/DX11Texture.h"
#include "apExRoot.h"
#define WINDOWNAME _T("Texgen Preview")
#define WINDOWXML _T("TexgenPreview")

CapexTexGenPreview::CapexTexGenPreview() : CapexWindow()
{
  Texture = NULL;
  Generating = false;
  AlphaBlend = SolidBlend = NULL;
  WrapSampler = NULL;
  CopyFromOp = false;
  EditedOp = -1;
  Maximized = false;
  JustBeenMaximized = true;
  Panning = false;
  Pan = _Pan = CVector2( 0, 0 );
  DisplayedOpID = -1;
}

CapexTexGenPreview::CapexTexGenPreview( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML )
{
  Texture = NULL;
  Generating = false;
  AlphaBlend = SolidBlend = NULL;
  WrapSampler = NULL;
  CopyFromOp = false;
  EditedOp = -1;
  Maximized = false;
  JustBeenMaximized = true;
  NormalPosition = MaximizedPosition = Pos;
  Panning = false;
  Pan = _Pan = CVector2( 0, 0 );
  DisplayedOpID = -1;
}

CapexTexGenPreview::~CapexTexGenPreview()
{
  SAFEDELETE( WrapSampler );
  SAFEDELETE( AlphaBlend );
  SAFEDELETE( SolidBlend );
  SAFEDELETE( Texture );
  //if (Texture)
  //{
  //	((CCoreDX11Texture2D*)Texture)->SetTextureHandle(NULL);
  //	((CCoreDX11Texture2D*)Texture)->SetView(NULL);
  //}
  //delete Texture;
}

void CapexTexGenPreview::OnDraw( CWBDrawAPI *API )
{
  //create blend states if needed
  if ( !AlphaBlend )
  {
    AlphaBlend = API->GetDevice()->CreateBlendState();
    AlphaBlend->SetBlendEnable( 0, true );
    AlphaBlend->SetSrcBlend( 0, COREBLEND_SRCALPHA );
    AlphaBlend->SetDestBlend( 0, COREBLEND_INVSRCALPHA );
  }

  if ( !SolidBlend )
  {
    SolidBlend = API->GetDevice()->CreateBlendState();
    SolidBlend->SetBlendEnable( 0, false );
  }

  if ( !WrapSampler )
  {
    WrapSampler = API->GetDevice()->CreateSamplerState();
    WrapSampler->SetAddressU( CORETEXADDRESS_WRAP );
    WrapSampler->SetAddressV( CORETEXADDRESS_WRAP );
  }

  CphxTextureOperator_Tool *op = Project.GetTexgenOp( EditedOp );
  CphxTextureOperator_Tool *cop = NULL;

  if ( op ) cop = op->GetContentOp();
  if ( !cop )
  {
    SAFEDELETE( Texture );
    CapexWindow::OnDraw( API );
    return;
  }

  if ( ( !cop->ContentReady() && cop->CanBeGenerated() ) || ( op != cop && DisplayedOpID != cop->ID ) )
  {
    Generating = true;
    cop->RequestContent();
    cop->Lock( true );
    DisplayedOpID = cop->ID;
  }

  if ( CopyFromOp || ( Generating && cop->ContentReady() ) )
  {
    //generating just finished
    Generating = false;
    CopyFromOp = false;
    SAFEDELETE( Texture );

    //copy finished texture
    CCoreDX11Texture2D *dummy = new CCoreDX11Texture2D( (CCoreDX11Device*)API->GetDevice() );
    dummy->SetTextureHandle( cop->Result->Texture );
    dummy->SetView( cop->Result->View );

    Texture = API->GetDevice()->CopyTexture( dummy );
    cop->Lock( false );
    //op->Allocate(false);

    dummy->SetTextureHandle( NULL );
    dummy->SetView( NULL );
    SAFEDELETE( dummy );
  }

  //if (!Texture)
  //{
  //	CCoreDX11Device *d=(CCoreDX11Device*)API->GetDevice();

  //	CphxTexturePoolTexture *p1=TexgenPool->GetTexture(0x88);
  //	CphxTexturePoolTexture *p2=TexgenPool->GetTexture(0x88);
  //	PHXTEXTUREFILTER *f=new PHXTEXTUREFILTER;
  //	f->DataDescriptor.InputCount=0;
  //	f->DataDescriptor.NeedsRandSeed=0;
  //	f->DataDescriptor.ParameterCount=0;
  //	f->PassCount=1;

  //	const LPSTR Code= "cbuffer b { float4 a; float4 b; };"
  //					  "float4 psmain(float2 uv : TEXCOORD0) : SV_TARGET0 { return float4(fmod(uv.x*256+uv.y*256-1,2.0f),uv.x,uv.y,1); }";

  //	CCorePixelShader *pxs=App->GetDevice()->CreatePixelShader(Code,strlen(Code),"psmain","ps_5_0");
  //	if (pxs)
  //	{
  //		f->PixelShader=(ID3D11PixelShader*)pxs->GetHandle();

  //		unsigned char params[16];
  //		params[0]=128;
  //		f->Render(p1,p2,NULL,0,params);
  //		delete pxs;
  //	}

  //	delete f;

  //	CCoreDX11Texture2D *t=new CCoreDX11Texture2D((CCoreDX11Device*)API->GetDevice());
  //	Texture=t;
  //	t->SetTextureHandle(p1->Texture);
  //	t->SetView(p1->View);

  //	//D3DX11SaveTextureToFile(d->GetDeviceContext(),p1->Texture,D3DX11_IFF_PNG,"!!!.png");

  //	d->ForceStateReset();
  //	API->SetUIRenderState();
  //}


  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_FONTCOLOR, 0xffffffff );
  if ( !InFocus() && WorkBench->GetWindow( apEx_TexGenPreview ) == this )
    CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_FONTCOLOR, CColor::FromARGB( 0xff7ac1ff ) );

  CapexWindow::OnDraw( API );

  //display texture
  if ( Texture )
  {
    API->SetCropToClient( this );
    CRect r = GetClientRect();

    CWBButton *b = (CWBButton *)FindChildByID( _T( "AlphaToggle" ), _T( "button" ) );

    TBOOL Alpha = b && b->IsPushed();

    b = (CWBButton *)FindChildByID( _T( "TileToggle" ), _T( "button" ) );
    TBOOL Tile = b&&b->IsPushed();
    b = (CWBButton *)FindChildByID( _T( "StretchToggle" ), _T( "button" ) );
    TBOOL Stretch = b&&b->IsPushed();

    CRect checkerrect = GetClientRect();

    TS32 xr = 256;
    TS32 yr = 256;
    if ( cop )
    {
      xr = GETXRES( cop->OpData.Resolution );
      yr = GETYRES( cop->OpData.Resolution );
    }

    if ( Tile )
      checkerrect = r;
    else
      if ( !Stretch )
        checkerrect = CRect( r.x1, r.y1, r.x1 + xr, r.y1 + yr );
      else
      {
        checkerrect = GetClientRect();
        if ( xr > yr )
          checkerrect.y2 = (TS32)( r.Width() / (TF32)xr*yr );
        else
          checkerrect.x2 = (TS32)( r.Height() / (TF32)yr*xr );

      }

    if ( Alpha )
      DrawCheckerboard( API, checkerrect );

    API->FlushDrawBuffer();

    if ( Alpha )
      API->GetDevice()->SetRenderState( AlphaBlend );
    else
      API->GetDevice()->SetRenderState( SolidBlend );

    API->GetDevice()->SetSamplerState( CORESMP_PS0, WrapSampler );

    API->GetDevice()->SetTexture( CORESMP_PS0, Texture );

    API->GetDevice()->SetPixelShader( Root->GammaDisplayShader ); //counteract the degamma in the display

    if ( Tile )
    {
      API->DrawRect( r, Pan.x, Pan.y, r.Width() / (TF32)xr + Pan.x, r.Height() / (TF32)yr + Pan.y );
    }
    else
    {
      if ( !Stretch )
      {
        CRect re = CRect( r.x1, r.y1, r.x1 + xr, r.y1 + yr );

        API->DrawRect( re, 0 + Pan.x, 0 + Pan.y, 1 + Pan.x, 1 + Pan.y );
        if ( App->GetCtrlState() )
        {
          re = ClientToScreen( re );
          CPoint mp = App->GetMousePos();
          if ( re.Contains( mp ) )
          {
            mp -= re.TopLeft();
            SetTooltip( CString::Format( "%f, %f", ( mp.x / (float)re.Width() ) - Pan.x, ( mp.y / (float)re.Height() ) - Pan.y ) );
          }
        }
      }
      else
      {
        CRect rec = GetClientRect();
        if ( xr > yr )
          rec.y2 = (TS32)( rec.Width() / (TF32)xr*yr );
        else
          rec.x2 = (TS32)( rec.Height() / (TF32)yr*xr );

        API->DrawRect( rec, 0 + Pan.x, 0 + Pan.y, 1 + Pan.x, 1 + Pan.y );
      }
    }

    API->FlushDrawBuffer();
    API->SetUIRenderState();
  }
}

void CapexTexGenPreview::UpdateData()
{

}

TBOOL CapexTexGenPreview::MessageProc( CWBMessage &Message )
{

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( b->GetID() == _T( "AlphaToggle" ) )
    {
      b->Push( !b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "TileToggle" ) )
    {
      b->Push( !b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "StretchToggle" ) )
    {
      b->Push( !b->IsPushed() );
      return true;
    }

    if ( b->GetID() == _T( "maximizebutton" ) )
    {
      b->Push( !b->IsPushed() );
      Maximized = b->IsPushed();
      if ( b->IsPushed() )
        SetPosition( MaximizedPosition );
      else
        SetPosition( NormalPosition );
      JustBeenMaximized = true;
      return true;
    }
  }
  break;
  case WBM_RIGHTBUTTONUP:
  {
    if ( App->GetMousePos() == App->GetRightDownPos() && Texture )
    {
      //CWBContextMenu *m=new CWBContextMenu(App->GetRoot(),CRect(App->GetMousePos(),CPoint(0,0)),GetGuid());
      CWBContextMenu *m = OpenContextMenu( App->GetMousePos() );
      m->AddItem( _T( "Export" ), 100 );
      return true;
    }
  }
  break;
  case WBM_CONTEXTMESSAGE:
    if ( Message.Data == 100 )
    {
      OpenExportMenu();
      return true;
    }
    break;
  case WBM_MIDDLEBUTTONDOWN:
    if ( !MouseOver() ) break;
    {
      Panning = true;
      _Pan = Pan;
      App->SetCapture( this );
      return true;
    }
    break;
  case WBM_MIDDLEBUTTONDBLCLK:
  {
    if ( !MouseOver() ) break;
    Panning = false;
    Pan = _Pan = CVector2( 0, 0 );
  }
  break;
  case WBM_MOUSEMOVE:
    if ( Panning )
    {
      CphxTextureOperator_Tool *op = Project.GetTexgenOp( EditedOp );
      if ( op ) op = op->GetContentOp();
      if ( !op ) break;

      TF32 xr = (TF32)GETXRES( op->OpData.Resolution );
      TF32 yr = (TF32)GETYRES( op->OpData.Resolution );

      CWBButton *b = (CWBButton *)FindChildByID( _T( "StretchToggle" ), _T( "button" ) );
      TBOOL Stretch = b&&b->IsPushed();

      b = (CWBButton *)FindChildByID( _T( "TileToggle" ), _T( "button" ) );
      TBOOL Tile = b&&b->IsPushed();

      if ( Stretch && !Tile )
      {
        xr = (TF32)GetClientRect().Width();
        yr = (TF32)GetClientRect().Height();
      }

      CPoint d = App->GetMidDownPos() - App->GetMousePos();
      Pan = _Pan + CVector2( (TF32)d.x / xr, (TF32)d.y / yr );
    }
    break;
  case WBM_MIDDLEBUTTONUP:
  {
    Panning = false;
    App->ReleaseCapture();
  }
  case WBM_FOCUSGAINED:
  {
    WorkBench->SetLastTexgenPreview( this );
  }
  break;
  case WBM_REPOSITION:
    if ( Message.GetTarget() == GetGuid() )
    {
      if ( Message.Resized )
      {
        CRect r = GetPosition();
        CRect c = GetClientRect();

        CSize s = r.Size() - c.Size();
        CSize n = Message.Rectangle.Size() - s;

        TBOOL SidesMoved[ 4 ];
        SidesMoved[ 0 ] = r.x1 != Message.Rectangle.x1;
        SidesMoved[ 1 ] = r.x2 != Message.Rectangle.x2;
        SidesMoved[ 2 ] = r.y1 != Message.Rectangle.y1;
        SidesMoved[ 3 ] = r.y2 != Message.Rectangle.y2;

        CphxTextureOperator_Tool *op = Project.GetTexgenOp( EditedOp );
        if ( op )
        {
          TS32 xr = GETXRES( op->OpData.Resolution );
          TS32 yr = GETYRES( op->OpData.Resolution );

          if ( abs( xr - n.x ) < 5 )
          {
            TS32 xd = xr - n.x;
            if ( SidesMoved[ 0 ] ) Message.Rectangle.x1 -= xd;
            if ( SidesMoved[ 1 ] ) Message.Rectangle.x2 += xd;
          }
          if ( abs( yr - n.y ) < 5 )
          {
            TS32 yd = yr - n.y;
            if ( SidesMoved[ 2 ] ) Message.Rectangle.y1 -= yd;
            if ( SidesMoved[ 3 ] ) Message.Rectangle.y2 += yd;
          }

          n = Message.Rectangle.Size() - s;

          if ( App->GetCtrlState() )
          {
            TS32 tx = n.x;
            TS32 ty = n.y;
            if ( DragMode & WB_DRAGMODE_LEFT || DragMode & WB_DRAGMODE_RIGHT )
            {
              tx = n.x;
              ty = (TS32)( n.x / (TF32)xr*yr );
            }
            else
            {
              tx = (TS32)( n.y / (TF32)yr*xr );
              ty = n.y;
            }

            TS32 xd = tx - n.x;
            TS32 yd = ty - n.y;
            if ( DragMode & WB_DRAGMODE_TOP ) Message.Rectangle.x1 -= xd;
            if ( DragMode & WB_DRAGMODE_BOTTOM ) Message.Rectangle.x2 += xd;
            if ( DragMode & WB_DRAGMODE_LEFT ) Message.Rectangle.y1 -= yd;
            if ( DragMode & WB_DRAGMODE_RIGHT ) Message.Rectangle.y2 += yd;
          }

        }

      }

      if ( JustBeenMaximized )
      {
        JustBeenMaximized = false;
        break;
      }
      if ( Maximized )
        MaximizedPosition = Message.Rectangle;
      else
        NormalPosition = Message.Rectangle;
    }
    break;
  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexTexGenPreview::SetEditedOperator( APEXOPID ID )
{
  if ( EditedOp == ID ) return;

  CphxTextureOperator_Tool *oldop = Project.GetTexgenOp( EditedOp );
  if ( oldop ) oldop->Lock( false );

  EditedOp = ID;
  CphxTextureOperator_Tool *op = Project.GetTexgenOp( EditedOp );

  if ( op )
  {
    CString t = WINDOWNAME;

    t = t + _T( " (" ) + op->GetName() + _T( ")" );

    SetTitle( t );
  }

  if ( !op ) return;
  if ( op->ContentReady() )
    CopyFromOp = true;
}

#include <CommDlg.h>
#include "apExRoot.h"

void CapexTexGenPreview::OpenExportMenu()
{
  if ( !Texture ) return;

  TCHAR dir[ 1024 ];
  GetCurrentDirectory( 1024, dir );
  char Filestring[ 256 ];

  OPENFILENAME opf;
  opf.hwndOwner = 0;
  opf.lpstrFilter = "PNG Files\0*.png\0\0";
  opf.lpstrCustomFilter = 0;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[ 0 ] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = 0;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save Texture";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "png";
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = ( OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON ) & ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof( OPENFILENAME );

  opf.hInstance = GetModuleHandle( 0 );
  opf.pvReserved = NULL;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  extern CapexRoot *Root;
  opf.lpstrInitialDir = Root->GetTargetDirectory( "exportimage" );

  if ( GetSaveFileName( &opf ) )
  {
    Root->StoreCurrentDirectory( "exportimage" );
    SetCurrentDirectory( dir );
    Texture->ExportToImage( CString( opf.lpstrFile ), false, CORE_PNG, false );
  }
  SetCurrentDirectory( dir );

}

void CapexTexGenPreview::ExportWindow( CXMLNode *node )
{
  CapexWindow::ExportWindow( node );
  if ( !node ) return;
  node->SetAttributeFromInteger( _T( "maximized" ), Maximized );
  node->SetAttributeFromInteger( _T( "nx1" ), NormalPosition.x1 );
  node->SetAttributeFromInteger( _T( "ny1" ), NormalPosition.y1 );
  node->SetAttributeFromInteger( _T( "nx2" ), NormalPosition.x2 );
  node->SetAttributeFromInteger( _T( "ny2" ), NormalPosition.y2 );
  node->SetAttributeFromInteger( _T( "mx1" ), MaximizedPosition.x1 );
  node->SetAttributeFromInteger( _T( "my1" ), MaximizedPosition.y1 );
  node->SetAttributeFromInteger( _T( "mx2" ), MaximizedPosition.x2 );
  node->SetAttributeFromInteger( _T( "my2" ), MaximizedPosition.y2 );

}

void CapexTexGenPreview::ImportConfig( CXMLNode *node, CRect &Pos )
{
  MaximizedPosition = NormalPosition = Pos;
  TS32 i = Maximized;
  if ( node->HasAttribute( _T( "maximized" ) ) ) node->GetAttributeAsInteger( _T( "maximized" ), &i );
  Maximized = 0;
  if ( node->HasAttribute( _T( "nx1" ) ) ) node->GetAttributeAsInteger( _T( "nx1" ), &NormalPosition.x1 );
  if ( node->HasAttribute( _T( "ny1" ) ) ) node->GetAttributeAsInteger( _T( "ny1" ), &NormalPosition.y1 );
  if ( node->HasAttribute( _T( "nx2" ) ) ) node->GetAttributeAsInteger( _T( "nx2" ), &NormalPosition.x2 );
  if ( node->HasAttribute( _T( "ny2" ) ) ) node->GetAttributeAsInteger( _T( "ny2" ), &NormalPosition.y2 );
  if ( node->HasAttribute( _T( "mx1" ) ) ) node->GetAttributeAsInteger( _T( "mx1" ), &MaximizedPosition.x1 );
  if ( node->HasAttribute( _T( "my1" ) ) ) node->GetAttributeAsInteger( _T( "my1" ), &MaximizedPosition.y1 );
  if ( node->HasAttribute( _T( "mx2" ) ) ) node->GetAttributeAsInteger( _T( "mx2" ), &MaximizedPosition.x2 );
  if ( node->HasAttribute( _T( "my2" ) ) ) node->GetAttributeAsInteger( _T( "my2" ), &MaximizedPosition.y2 );
  if ( Maximized ) SetPosition( MaximizedPosition );
  else SetPosition( NormalPosition );

  CWBButton *b = (CWBButton*)FindChildByID( _T( "maximizebutton" ), _T( "button" ) );
  if ( b ) b->Push( Maximized );
}

void CapexTexGenPreview::DrawCheckerboard( CWBDrawAPI *API, CRect r )
{
  CRect cr = API->GetCropRect();
  API->SetCropRect( cr | ClientToScreen( r ) );

  for ( TS32 x = 0; x < r.Width(); x += 8 )
    for ( TS32 y = 0; y < r.Height(); y += 8 )
      API->DrawRect( CRect( x, y, x + 8, y + 8 ), ( ( x / 8 + y / 8 ) % 2 ) ? CColor::FromABGR( 0xff404040 ) : CColor::FromABGR( 0xff303030 ) );

  API->SetCropRect( cr );
}
