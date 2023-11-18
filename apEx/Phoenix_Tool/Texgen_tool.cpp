#include "BasePCH.h"
#include "phxSplineExt.h"
#include "Texgen_tool.h"
#include "../Phoenix_Tool/apxProject.h"

TS32 TexgenMemPoolSize = 512;

//////////////////////////////////////////////////////////////////////////
// filter

CphxTextureFilter_Tool::CphxTextureFilter_Tool()
{
  Name = "New Texture Filter";
  Type = 0;
  Code = "cbuffer b { float4 a,b; };\n\n"
    "float4 p(float4 p:TEXCOORD0) : SV_TARGET0\n{\n return float4(fmod(p.x*256+p.y*256-1,2.0f),p.x,p.y,1);\n}";
  Filter.DataDescriptor.InputCount = 0;
  Filter.DataDescriptor.NeedsRandSeed = 0;
  Filter.DataDescriptor.ParameterCount = 0;
  Filter.DataDescriptor.PassCount = 1;
  Filter.DataDescriptor.LookupType = 0;
  Filter.PixelShader = NULL;
  Shader = NULL;
  External = false;
}

CphxTextureFilter_Tool::~CphxTextureFilter_Tool()
{
  Parameters.FreeArray();
  //if (Filter.PixelShader) Filter.PixelShader->Release();
  Filter.PixelShader = NULL;
  SAFEDELETE( Shader );
}

void CphxTextureFilter_Tool::ExportData( CXMLNode *Node )
{
  Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "Code" ), false ).SetText( Code.GetPointer() );
  Node->AddChild( _T( "Type" ), false ).SetInt( Type );
  Node->AddChild( _T( "PassCount" ), false ).SetInt( Filter.DataDescriptor.PassCount );
  Node->AddChild( _T( "InputCount" ), false ).SetInt( Filter.DataDescriptor.InputCount );
  Node->AddChild( _T( "NeedsRandSeed" ) ).SetInt( Filter.DataDescriptor.NeedsRandSeed );

  if ( Filter.DataDescriptor.LookupType ) Node->AddChild( _T( "LookupType" ) ).SetInt( Filter.DataDescriptor.LookupType );

  Node->AddChild( _T( "Minifiable" ), false ).SetInt( Minifiable );

  for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
  {
    CphxTextureFilterParameter *p = Parameters[ x ];
    CXMLNode n = Node->AddChild( _T( "Parameter" ) );
    n.AddChild( _T( "Name" ), false ).SetText( p->Name.GetPointer() );
    n.AddChild( _T( "Type" ), false ).SetInt( p->Type );
    n.AddChild( _T( "Min" ), false ).SetInt( p->Minimum );
    n.AddChild( _T( "Max" ), false ).SetInt( p->Maximum );

    if ( !( p->Type == 1 || p->Type == 3 || p->Type == 4 ) )
      n.AddChild( _T( "Default" ), false ).SetInt( p->Default );

    for ( TS32 y = 0; y < p->ListItems.NumItems(); y++ )
      n.AddChild( _T( "ListItem" ), y == p->ListItems.NumItems() - 1 ).SetText( p->ListItems[ y ].GetPointer() );
  }
}

void CphxTextureFilter_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();
  if ( Node->GetChildCount( _T( "Code" ) ) ) Code = Node->GetChild( _T( "Code" ) ).GetText();
  if ( Node->GetChildCount( _T( "Type" ) ) ) Node->GetChild( _T( "Type" ) ).GetValue( Type );
  TS32 x = Filter.DataDescriptor.PassCount;
  if ( Node->GetChildCount( _T( "PassCount" ) ) ) Node->GetChild( _T( "PassCount" ) ).GetValue( x );
  Filter.DataDescriptor.PassCount = x;
  x = Filter.DataDescriptor.InputCount;
  if ( Node->GetChildCount( _T( "InputCount" ) ) ) Node->GetChild( _T( "InputCount" ) ).GetValue( x ); Filter.DataDescriptor.InputCount = x;
  x = Filter.DataDescriptor.NeedsRandSeed;
  if ( Node->GetChildCount( _T( "NeedsRandSeed" ) ) ) Node->GetChild( _T( "NeedsRandSeed" ) ).GetValue( x ); Filter.DataDescriptor.NeedsRandSeed = x;
  x = Filter.DataDescriptor.LookupType;
  if ( Node->GetChildCount( _T( "LookupType" ) ) ) Node->GetChild( _T( "LookupType" ) ).GetValue( x ); Filter.DataDescriptor.LookupType = x;

  if ( Node->GetChildCount( _T( "Minifiable" ) ) )
  {
    TBOOL i = 0;
    if ( Node->GetChild( _T( "Minifiable" ) ).GetValue( i ) )
      Minifiable = i;
  }

  TS32 ParamCount = Node->GetChildCount( _T( "Parameter" ) );
  for ( x = 0; x < ParamCount; x++ )
  {
    CphxTextureFilterParameter *p = new CphxTextureFilterParameter();
    CXMLNode n = Node->GetChild( _T( "Parameter" ), x );
    if ( n.GetChildCount( _T( "Name" ) ) ) p->Name = n.GetChild( _T( "Name" ) ).GetText();
    if ( n.GetChildCount( _T( "Type" ) ) ) n.GetChild( _T( "Type" ) ).GetValue( p->Type );

    if ( p->Type == 1 || p->Type == 3 || p->Type == 4 )
      p->Default = 0;

    if ( n.GetChildCount( _T( "Min" ) ) ) n.GetChild( _T( "Min" ) ).GetValue( p->Minimum );
    if ( n.GetChildCount( _T( "Max" ) ) ) n.GetChild( _T( "Max" ) ).GetValue( p->Maximum );
    if ( n.GetChildCount( _T( "Default" ) ) ) n.GetChild( _T( "Default" ) ).GetValue( p->Default );
    for ( TS32 y = 0; y < n.GetChildCount( _T( "ListItem" ) ); y++ )
      p->ListItems += n.GetChild( _T( "ListItem" ), y ).GetText();
    Parameters += p;
  }
}

TS32 CphxTextureFilter_Tool::GetParamStart( TS32 ParamID )
{
  TS32 v = 0;
  for ( TS32 x = 0; x < ParamID; x++ )
    v += GetParamSize( x );
  return v;
}

TS32 CphxTextureFilter_Tool::GetParamSize( TS32 ParamID )
{
  if ( ParamID < 0 || ParamID >= Parameters.NumItems() ) return 0;

  switch ( Parameters[ ParamID ]->Type )
  {
  case 0:
  case 1:
  case 3:
  case 4: return 1;
  case 2: return 4;
  }
  return 0;
}

TBOOL CphxTextureFilter_Tool::GenerateResource( CCoreDevice *Dev )
{
  //FixFloatingPoint();
  Touch();
  //LOG_DBG("[texgen] Compiled filter %s",Name.GetPointer());
  SAFEDELETE( Shader );
  Filter.PixelShader = NULL;
  Shader = Dev->CreatePixelShader( Code.GetPointer(), Code.Length(), "p", "ps_5_0" );
  if ( Shader )
    Filter.PixelShader = (ID3D11PixelShader*)Shader->GetHandle();
  return Shader != NULL;
}

bool CphxTexturePoolTexture_Tool::Create( unsigned char res, bool _hdr )
{
  Resolution = res;
  Used = false;

  XRes = GETXRES( res );
  YRes = GETYRES( res );
  hdr = _hdr;

  return CreateRenderTarget( XRes, YRes, hdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R16G16B16A16_UNORM, TEXTUREFORMATSIZE, true, Texture, View, RTView ) != 0;

  //D3D11_TEXTURE2D_DESC tex;
  //memset(&tex,0,sizeof(D3D11_TEXTURE2D_DESC));
  //tex.ArraySize=1;
  //tex.Width=XRes;
  //tex.Height=YRes;
  //tex.MipLevels=1;
  //tex.Format=TEXTUREFORMAT;
  //tex.SampleDesc.Count=1;
  //tex.SampleDesc.Quality=0;
  //tex.BindFlags=D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

  //D3D11_SUBRESOURCE_DATA data;
  //data.pSysMem=new unsigned char[XRes*YRes*TEXTUREFORMATSIZE];
  //data.SysMemPitch=XRes*TEXTUREFORMATSIZE;
  //data.SysMemSlicePitch=0;

  //HRESULT result=phxDev->CreateTexture2D(&tex,&data,&Texture);
  //if (result!=S_OK)
  //{
  //	_com_error err(result);
  //	LOG(LOG_ERROR,_T("[texpool] Failed to create texture (%s)"),err.ErrorMessage());
  //	delete[] data.pSysMem;
  //	return false;
  //}
  //result=phxDev->CreateShaderResourceView(Texture,NULL,&View);
  //if (result!=S_OK)
  //{
  //	_com_error err(result);
  //	LOG(LOG_ERROR,_T("[texpool] Failed to create shader resultource view (%s)"),err.ErrorMessage());
  //	delete[] data.pSysMem;
  //	return false;
  //}

  //delete[] data.pSysMem;

  //D3D11_RENDER_TARGET_VIEW_DESC rt;
  //rt.Format=TEXTUREFORMAT;
  //rt.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
  //rt.Texture2D.MipSlice=0;
  //result=phxDev->CreateRenderTargetView(Texture,&rt,&RTView);
  //if (result!=S_OK)
  //{
  //	_com_error err(result);
  //	LOG(LOG_ERROR,_T("[texpool] Failed to rendertarget view (%s)"),err.ErrorMessage());
  //	return false;
  //}
  //return true;
}

//////////////////////////////////////////////////////////////////////////
// operator

APEXOPID GlobalID = 0;

void CphxTextureOperator_Tool::SetBuildsTextureSymbolRecursive( const CString& symbol )
{
  if ( !symbol.Length() )
    return;

  if ( BuildsTexture.Length() )
    BuildsTexture = "SHARED";
  else
    BuildsTexture = symbol;
  for ( int x = 0; x < GetParentCount( PHX_TEXTUREOPERATOR ); x++ )
  {
    CphxTextureOperator_Tool* parentOp = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    parentOp->GetContentOp()->SetBuildsTextureSymbolRecursive( symbol );
  }
}

CphxTextureOperator_Tool::CphxTextureOperator_Tool( CRect Pos, CapexTexGenPage *parentPage )
{
  for ( int x = 0; x < 4; x++ )
    UiVisibleCurveChannels[ x ] = true;
  Position = Pos;
  Selected = false;
  Name = "";
  ID = GlobalID++;
  Filter = NULL;
  Result = NULL;
  memset( OpData.Parameters, 0, TEXGEN_MAX_PARAMS );
  OpData.Resolution = 0x88;
  OpData.RandSeed = 0;
  SubroutineRoot = NULL;
  ImageData = NULL;
  ImageDataSize = 0;
  TextData.XPos = 2;
  TextData.YPos = 98;
  TextData.Size = 48;
  TextData.CharSpace = 64;
  TextData.Bold = 0;
  TextData.Italic = 0;
  TextData.Font = 1;
  //TextData.Text = NULL;
  Text = NULL;
  for ( TS32 x = 0; x < 4; x++ )
  {
    Curves[ x ] = new CphxSpline_Tool_float16();
    CphxSplineKey_Tool *k = new CphxSplineKey_Tool();
    k->Key.t = 0;
    k->Key.Value[ 0 ] = 0;
    Curves[ x ]->Keys += k;

    k = new CphxSplineKey_Tool();
    k->Key.t = 255;
    k->Key.Value[ 0 ] = 1;
    Curves[ x ]->Keys += k;
  }
  ParentPage = parentPage;
}

void CphxTextureOperator_Tool::ExportData( CXMLNode *Node )
{
  if ( Name.Length() ) Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "x1" ), false ).SetInt( Position.x1 );
  Node->AddChild( _T( "y1" ), false ).SetInt( Position.y1 );
  Node->AddChild( _T( "x2" ), false ).SetInt( Position.x2 );
  Node->AddChild( _T( "y2" ), false ).SetInt( Position.y2 );
  Node->AddChild( _T( "Filter" ), false ).SetText( FilterGuid.GetString() );

  Node->AddChild( _T( "Resolution" ), false ).SetInt( OpData.Resolution );
  Node->AddChild( _T( "Seed" ), false ).SetInt( OpData.RandSeed );
  for ( TS32 x = 0; x < TEXGEN_MAX_PARAMS; x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Parameter" ), x == TEXGEN_MAX_PARAMS - 1 );
    n.SetAttributeFromInteger( _T( "ID" ), x );
    n.SetInt( OpData.Parameters[ x ] );
  }

  if ( Filter )
  {
    switch ( Filter->Filter.DataDescriptor.LookupType )
    {
    case 1:
    { // image load
      if ( ImageData && ImageDataSize )
      {
        CString imgstr = CString::EncodeToBase64( ImageData, ImageDataSize );
        Node->AddChild( _T( "image" ) ).SetText( imgstr.GetPointer() );
      }
    }
    break;
    case 2:
    { // text input
      Node->AddChild( _T( "textposx" ) ).SetInt( TextData.XPos );
      Node->AddChild( _T( "textposy" ) ).SetInt( TextData.YPos );
      Node->AddChild( _T( "textsize" ) ).SetInt( TextData.Size );
      Node->AddChild( _T( "textcharspacing" ) ).SetInt( TextData.CharSpace );
      Node->AddChild( _T( "textitalic" ) ).SetInt( TextData.Italic );
      Node->AddChild( _T( "textbold" ) ).SetInt( TextData.Bold );
      Node->AddChild( _T( "textfont" ) ).SetInt( TextData.Font );
      //Node->AddChild(_T("text")).SetText(TextData.Text);

      //CString str = CString::EncodeToBase64((TU8*)TextData.Text, strlen(TextData.Text));

      if ( Text && strlen( Text ) )
      {
        CString str = CString::EncodeToBase64( (TU8*)Text, strlen( Text ) );
        Node->AddChild( _T( "textblob" ) ).SetText( str.GetPointer() );
      }
    }
    break;
    case 3:
    { // spline data

      for ( TS32 x = 0; x < 4; x++ )
      {
        CXMLNode n = Node->AddChild( _T( "spline" ) );
        n.SetAttributeFromInteger( _T( "splineid" ), x );
        Curves[ x ]->ExportData( n );
      }
    }
    break;
    default:
      break;
    }
  }

}

void CphxTextureOperator_Tool::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  if ( Node->GetChildCount( _T( "x1" ) ) ) Node->GetChild( _T( "x1" ) ).GetValue( Position.x1 );
  if ( Node->GetChildCount( _T( "y1" ) ) ) Node->GetChild( _T( "y1" ) ).GetValue( Position.y1 );
  if ( Node->GetChildCount( _T( "x2" ) ) ) Node->GetChild( _T( "x2" ) ).GetValue( Position.x2 );
  if ( Node->GetChildCount( _T( "y2" ) ) ) Node->GetChild( _T( "y2" ) ).GetValue( Position.y2 );
  OldPosition = Position;

  if ( Node->GetChildCount( _T( "Filter" ) ) )
  {
    FilterGuid.SetString( Node->GetChild( _T( "Filter" ) ).GetText().GetPointer() );

    CphxTextureFilter_Tool *f = Project.GetTextureFilter( FilterGuid );
    if ( !f )
    {
      LOG_ERR( "[import] Texture filter '%s' not found in project!", FilterGuid.GetString() );
    }
    else
    {
      Filter = f;
      AddParent( Filter );
    }
  }

  if ( Node->GetChildCount( _T( "Resolution" ) ) ) Node->GetChild( _T( "Resolution" ) ).GetValue( OpData.Resolution );
  if ( Node->GetChildCount( _T( "Seed" ) ) )
  {
    TS32 val;
    if ( Node->GetChild( _T( "Seed" ) ).GetValue( val ) ) OpData.RandSeed = val;
  }

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Parameter" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Parameter" ), x );
    TS32 attrn = x;
    if ( n.HasAttribute( _T( "ID" ) ) ) n.GetAttributeAsInteger( _T( "ID" ), &attrn );
    TS32 val = 0;
    n.GetValue( val );
    OpData.Parameters[ attrn ] = val;
  }

  if ( Filter )
  {
    switch ( Filter->Filter.DataDescriptor.LookupType )
    {
    case 1:
    { // image load
      if ( Node->GetChildCount( _T( "image" ) ) )
      {
        CString txt = Node->GetChild( _T( "image" ) ).GetText();
        txt.DecodeBase64( ImageData, ImageDataSize );
      }
    }
    break;
    case 2:
    { // text input

      TS32 x = TextData.XPos;	if ( Node->GetChildCount( _T( "textposx" ) ) ) Node->GetChild( _T( "textposx" ) ).GetValue( x );				TextData.XPos = x;
      x = TextData.YPos;		if ( Node->GetChildCount( _T( "textposy" ) ) ) Node->GetChild( _T( "textposy" ) ).GetValue( x );				TextData.YPos = x;
      x = TextData.Size;		if ( Node->GetChildCount( _T( "textsize" ) ) ) Node->GetChild( _T( "textsize" ) ).GetValue( x );				TextData.Size = x;
      x = TextData.CharSpace;	if ( Node->GetChildCount( _T( "textcharspacing" ) ) ) Node->GetChild( _T( "textcharspacing" ) ).GetValue( x );	TextData.CharSpace = x;
      x = TextData.Italic;		if ( Node->GetChildCount( _T( "textitalic" ) ) ) Node->GetChild( _T( "textitalic" ) ).GetValue( x );			TextData.Italic = x;
      x = TextData.Bold;		if ( Node->GetChildCount( _T( "textbold" ) ) ) Node->GetChild( _T( "textbold" ) ).GetValue( x );				TextData.Bold = x;
      x = TextData.Font;		if ( Node->GetChildCount( _T( "textfont" ) ) ) Node->GetChild( _T( "textfont" ) ).GetValue( x );				TextData.Font = x;

      if ( Node->GetChildCount( _T( "text" ) ) )
      {
        CString txt = Node->GetChild( _T( "text" ) ).GetText();
        //SAFEDELETEA(TextData.Text);
        //TextData.Text = new char[txt.Length() + 1];
        //memset(TextData.Text, 0, txt.Length() + 1);
        //memcpy(TextData.Text, txt.GetPointer(), txt.Length());
        SAFEDELETEA( Text );
        Text = new char[ txt.Length() + 1 ];
        memset( Text, 0, txt.Length() + 1 );
        memcpy( Text, txt.GetPointer(), txt.Length() );
      }

      if ( Node->GetChildCount( _T( "textblob" ) ) )
      {
        TU8 *textdata = NULL;
        TS32 textdatasize;
        CString txt = Node->GetChild( _T( "textblob" ) ).GetText();
        txt.DecodeBase64( textdata, textdatasize );

        //SAFEDELETEA(TextData.Text);

        //if (textdatasize)
        //{
        //	TextData.Text = new char[textdatasize + 1];
        //	memset(TextData.Text, 0, textdatasize + 1);
        //	memcpy(TextData.Text, textdata, textdatasize);
        //}

        SAFEDELETEA( Text );

        if ( textdatasize )
        {
          Text = new char[ textdatasize + 1 ];
          memset( Text, 0, textdatasize + 1 );
          memcpy( Text, textdata, textdatasize );
        }

        SAFEDELETEA( textdata );
      }

      if ( !Text )
      {
        Text = new char[ 1 ];
        *Text = 0;
      }

    }
    break;
    case 3:
    { // spline data
      for ( TS32 x = 0; x < Node->GetChildCount( _T( "spline" ) ); x++ )
      {
        CXMLNode n = Node->GetChild( _T( "spline" ), x );
        TS32 id = x;
        n.GetAttributeAsInteger( _T( "splineid" ), &id );
        SAFEDELETE( Curves[ id ] );
        Curves[ id ] = new CphxSpline_Tool_float16();
        Curves[ id ]->ImportData( n );
      }
    }
    break;
    default:
      break;
    }
  }

}

TBOOL CphxTextureOperator_Tool::GenerateResource( CCoreDevice *Dev )
{
  Touch();
  if ( !Filter ) return NULL;

  if ( !Filter->ContentReady() )
  {
    LOG_NFO( "[texgen] Need to generate filter %s %s", Filter->Name.GetPointer(), Filter->GetGUID().GetString() );
    Filter->ForceUptodateFlag( Filter->GenerateResource( Dev ) );
  }

  CphxTextureOperator_Tool*parentops[ TEXGEN_MAX_PARENTS ];
  CphxTexturePoolTexture *Inputs[ TEXGEN_MAX_PARENTS ];

  for ( TS32 x = 0; x < TEXGEN_MAX_PARENTS; x++ )
  {
    parentops[ x ] = ( (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x ) );
    if ( parentops[ x ] ) parentops[ x ] = parentops[ x ]->GetContentOp();
    Inputs[ x ] = NULL;

    CphxTextureOperator_Tool *pr = parentops[ x ];
    if ( pr )
    {
      //pr->Lock(true);
      if ( !pr->ContentReady() || !pr->Result )// || !pr->IsUpToDate())
      {
        pr->UpToDate = pr->GenerateResource( Dev );
        if ( !pr->ContentReady() || !pr->Result )
          LOG_DBG( "[texgen] Invalid operator state: parent not up to date" );
        else
          Inputs[ x ] = pr->Result;
      }
      else
        Inputs[ x ] = pr->Result;
    }

    //if (parentops[x] && parentops[x]->Result)
    //	LOG_DBG("[texgen] Input %d dumped as %.4d.dds",x,parentops[x]->Result->DumpTexture());
  }

  //Result should already be allocated at this point externally

  if ( !Result )
    Allocate( true );

  CphxTexturePoolTexture *p2 = TexgenPool->GetTexture( OpData.Resolution, ParentPage->IsHDR() != 0 );
  CphxTexturePoolTexture *r = Result;

  CallRender( r, p2, Inputs, OpData.RandSeed, OpData.Parameters );
  Result = (CphxTexturePoolTexture_Tool*)r;
  //Result->Owner=this;

  p2->Used = false;
  //((CphxTexturePoolTexture_Tool*)p2)->Owner=NULL;

  //LOG_DBG("[texgen] pool size: %d, result: %d, t2: %d, p0: %d, p1: %d, p2: %d filter: %s",TexgenPool->PoolSize,Result,p2,Inputs[0],Inputs[1],Inputs[2],GetName().GetPointer());
  //LOG_DBG("[texgen] result dumped to %.4d.png",Result->DumpTexture());

  return true;
}

TBOOL CphxTextureOperator_Tool::InputsValid()
{
  if ( !Filter ) return false;

  TS32 opcount = GetParentCount( PHX_TEXTUREOPERATOR );
  //for (TS32 x=0; x<Parents.NumItems(); x++)
  //	if (Parents[x]->GetType()==PHX_TEXTUREOPERATOR) opcount++;

  return opcount == Filter->Filter.DataDescriptor.InputCount;
}

void CphxTextureOperator_Tool::LockMechanism( TBOOL Lock )
{
  if ( !Result ) return;
  //Result->Used=Lock!=0;
}

void CphxTextureOperator_Tool::AllocatorMechanism( TBOOL Alloc )
{
  if ( Result && !Alloc/* && Result->Owner==this*/ )
  {
    //LOG_DBG("[texgen] Deallocating %d",Result);
    //Result->Owner=NULL;
    Result->Used = false;
    Result = NULL;
  }

  if ( Alloc )
  {
    Result = (CphxTexturePoolTexture_Tool*)TexgenPool->GetTexture( OpData.Resolution, ParentPage->IsHDR() != 0 );
    Result->Used = true;
    //Result->Owner=this;
  }
}

CphxTextureOperator_Tool::~CphxTextureOperator_Tool()
{
  for ( TS32 x = 0; x < 4; x++ )
    SAFEDELETE( Curves[ x ] );
  //Lock(false);
  //SAFEDELETEA(TextData.Text);
  SAFEDELETEA( Text );
  SAFEDELETEA( ImageData );
  Allocate( false );
}

CphxTextureOperator_Tool * CphxTextureOperator_Tool::Copy()
{
  CphxTextureOperator_Tool *n = new CphxTextureOperator_Tool( Position, ParentPage );
  n->OpData = OpData;
  n->FilterGuid = FilterGuid;
  n->Filter = Filter;
  n->OldPosition = Position;
  n->Name = Name;
  if ( Filter )
    n->AddParent( Filter );
  n->CopyLookupData( this );
  return n;
}

void CphxTextureOperator_Tool::FreeIfNeeded()
{
  //TS32 UsedCount=0;
  //for (TS32 x=0; x<TexgenPool->PoolSize; x++)
  //	if (TexgenPool->Pool[x]->Used)
  //		UsedCount++;

  //if (UsedCount>10)
  //	Allocate(false);
}

TU32 CphxTextureOperator_Tool::GetInputCount()
{
  if ( !Filter ) return 0; return Filter->Filter.DataDescriptor.InputCount;
}

CString CphxTextureOperator_Tool::GetName()
{
  if ( Name.Length() ) return Name;
  if ( Filter ) return Filter->Name;
  return CString( _T( "" ) );
}

void CphxTextureOperator_Tool::SetSubroutine( CphxTextureOperator_Subroutine *Sub )
{
  SubroutineRoot = Sub;
  for ( TS32 x = 0; x < GetParentCount( PHX_TEXTUREOPERATOR ); x++ )
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    if ( t->GetOpType() != TEXGEN_OP_SAVE && t->GetOpType() != TEXGEN_OP_SUBROUTINE )
      t->SetSubroutine( Sub );
  }
}

void CphxTextureOperator_Tool::FreeSubroutineAllocationsandUpdateResolution( CphxTextureOperator_Subroutine *Root, TU8 Res )
{
  if ( SubroutineRoot != Root ) return;
  Allocate( false );

  for ( TS32 x = 0; x < GetParentCount( PHX_TEXTUREOPERATOR ); x++ )
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    t->OpData.Resolution = Res;
    t->FreeSubroutineAllocationsandUpdateResolution( Root, Res );
  }
}

void CphxTextureOperator_Tool::ClearSubroutineTags( CphxTextureOperator_Subroutine *Root )
{
  if ( SubroutineRoot != Root ) return;
  SubroutineTagged = false;

  for ( TS32 x = 0; x < GetParentCount( PHX_TEXTUREOPERATOR ); x++ )
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    t->ClearSubroutineTags( Root );
  }
}


void CphxTextureOperator_Tool::SetSubroutineInputOperators( CphxTextureOperator_Subroutine *Root, CphxTextureOperator_Tool **Inputs, TS32 NumInputs, TS32 &InputIdx )
{
  if ( SubroutineRoot != Root ) return;
  if ( InputIdx >= NumInputs ) return;
  if ( SubroutineTagged ) return;

  TS32 InputCnt = GetInputCount();
  TS32 ParentCnt = GetParentCount( PHX_TEXTUREOPERATOR );
  if ( GetOpType() == TEXGEN_OP_SUBROUTINECALL ) ParentCnt--;

  RemoveItemSpecificParents();

  for ( TS32 x = 0; x < ParentCnt; x++ ) //recurse through existing parents
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    if ( t->SubroutineRoot == Root )
    {
      t->SetSubroutineInputOperators( Root, Inputs, NumInputs, InputIdx );
    }
    else //remove previously added links
    {
      RemoveParent( t );
      x--;
      ParentCnt--;
    }
  }

  //add needed inputs
  for ( TS32 x = ParentCnt; x < InputCnt; x++ )
  {
    if ( InputIdx < NumInputs )
      AddParent( Inputs[ InputIdx++ ] );
  }

  ApplyItemSpecificParents();
  SubroutineTagged = true;
}

TS32 CphxTextureOperator_Tool::CountSubInputOps( CphxTextureOperator_Subroutine *Root )
{
  if ( SubroutineRoot != Root ) return 0;
  if ( SubroutineTagged ) return 0;
  TS32 InputCnt = GetInputCount();
  TS32 ParentCnt = GetParentCount( PHX_TEXTUREOPERATOR );
  if ( GetOpType() == TEXGEN_OP_SUBROUTINECALL ) ParentCnt--;

  TS32 cnt = 0;

  for ( TS32 x = 0; x < ParentCnt; x++ ) //recurse through existing parents
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    if ( t->SubroutineRoot == Root ) cnt += t->CountSubInputOps( Root );
    else
      cnt++;
  }

  cnt += InputCnt - ParentCnt;

  SubroutineTagged = true;

  return cnt;
}

void CphxTextureOperator_Tool::ClearSubInputs( CphxTextureOperator_Subroutine *Root )
{
  if ( SubroutineRoot != Root ) return;

  for ( TS32 x = 0; x < GetParentCount( PHX_TEXTUREOPERATOR ); x++ )
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );

    if ( t->GetOpType() != TEXGEN_OP_SUBROUTINE )
    {
      if ( t->SubroutineRoot != Root )
      {
        RemoveParent( t );
        x--;
      }
      else
        t->ClearSubInputs( Root );
    }
  }
}

void CphxTextureOperator_Tool::CallRender( CphxTexturePoolTexture *&Target, CphxTexturePoolTexture *&SwapBuffer, CphxTexturePoolTexture *Inputs[ TEXGEN_MAX_PARENTS ], unsigned char RandSeed, unsigned char Parameters[ TEXGEN_MAX_PARAMS ] )
{

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( true );
#endif

  switch ( Filter->Filter.DataDescriptor.LookupType )
  {
  case 1:
    Filter->Filter.Render( Target, SwapBuffer, Inputs, OpData.RandSeed, OpData.Parameters, ImageData, ImageDataSize );
    break;
  case 2:
    Filter->Filter.Render( Target, SwapBuffer, Inputs, OpData.RandSeed, OpData.Parameters, &TextData, (int)Text );
    break;
  case 3:
  {
    CphxSpline *Splines[ 4 ];
    for ( TS32 x = 0; x < 4; x++ )
      Splines[ x ] = Curves[ x ]->Spline;
    Filter->Filter.Render( Target, SwapBuffer, Inputs, OpData.RandSeed, OpData.Parameters, Splines, 0 );
    break;
  }
  default:
    Filter->Filter.Render( Target, SwapBuffer, Inputs, OpData.RandSeed, OpData.Parameters, NULL, 0 );
    break;
  }

#ifdef MEMORY_TRACKING
  memTracker.SetMissingIgnore( false );
#endif

}

void CphxTextureOperator_Tool::CopyLookupData( CphxTextureOperator_Tool *src )
{
  if ( src->ImageData )
  {
    SAFEDELETEA( ImageData );
    ImageData = new TU8[ src->ImageDataSize ];
    memcpy( ImageData, src->ImageData, src->ImageDataSize );
    ImageDataSize = src->ImageDataSize;
  }

  //SAFEDELETEA(TextData.Text);
  SAFEDELETEA( Text );
  TextData = src->TextData;

  //if (src->TextData.Text)
  //{
  //	TextData.Text = new char[strlen(src->TextData.Text) + 1];
  //	memset(TextData.Text, 0, strlen(src->TextData.Text) + 1);
  //	memcpy(TextData.Text, src->TextData.Text, strlen(src->TextData.Text));
  //}

  if ( src->Text )
  {
    Text = new char[ strlen( src->Text ) + 1 ];
    memset( Text, 0, strlen( src->Text ) + 1 );
    memcpy( Text, src->Text, strlen( src->Text ) );
  }

  for ( TS32 x = 0; x < 4; x++ )
    if ( src->Curves[ x ] )
    {
      CphxSpline_Tool_float16 *s = src->Curves[ x ];
      SAFEDELETE( Curves[ x ] );
      Curves[ x ] = (CphxSpline_Tool_float16 *)src->Curves[ x ]->Copy();
    }
}

void CphxTextureOperator_Tool::KillLoops()
{
  if ( TouchedByLoop )
  {
    LOG_WARN( "[texgen] Texture loop killed in operator %d (%s)", ID, GetName().GetPointer() );
    KillLoopCause();
    return;
  }

  TouchedByLoop = true;
  for ( TS32 x = 0; x < GetParentCount(); x++ )
    if ( GetParent( x )->GetType() == PHX_TEXTUREOPERATOR )
      ( (CphxTextureOperator_Tool*)GetParent( x ) )->KillLoops();
  TouchedByLoop = false;

}

void CphxTextureOperator_Tool::ClearSubRoots( CphxTextureOperator_Subroutine *Root )
{
  if ( this == Root ) return;
  if ( SubroutineRoot != Root ) return;
  SubroutineRoot = NULL;

  for ( TS32 x = 0; x < GetParentCount( PHX_TEXTUREOPERATOR ); x++ )
  {
    CphxTextureOperator_Tool *t = (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x );
    t->ClearSubRoots( Root );
  }
}

TBOOL CphxTextureOperator_Tool::UsedByMaterial()
{
  TS32 cnt = GetChildCount( PHX_TEXTUREOPERATOR );
  for ( TS32 x = 0; x < cnt; x++ )
  {
    CphxTextureOperator_Tool *r = (CphxTextureOperator_Tool*)GetChild( PHX_TEXTUREOPERATOR, x );
    if ( r->GetOpType() == TEXGEN_OP_SAVE )
    {
      //TS32 rc = r->GetChildCount();
      //TS32 rc2 = r->GetChildCount(PHX_TEXTUREOPERATOR);
      //if (rc != rc2) 
      return true;
    }
  }
  return false;
}

TBOOL CphxTextureOperator_Tool::SubTreeMatch( CphxTextureOperator_Tool *Op )
{
  if ( !ParentPage || !Op->ParentPage ) return false;
  if ( Filter != Op->Filter ) return false;
  if ( !( ParentPage->GetBaseXRes() == Op->ParentPage->GetBaseXRes() &&
          ParentPage->GetBaseYRes() == Op->ParentPage->GetBaseYRes() ) ) return false;
  for ( TU32 x = 0; x < Filter->Filter.DataDescriptor.ParameterCount; x++ )
    if ( OpData.Parameters[ x ] != Op->OpData.Parameters[ x ] ) return false;
  if ( Filter->Filter.DataDescriptor.NeedsRandSeed )
    if ( OpData.RandSeed != Op->OpData.RandSeed ) return false;

  switch ( Filter->Filter.DataDescriptor.LookupType )
  {
  case 1: //image load
    if ( ImageDataSize != Op->ImageDataSize ) return false;
    if ( memcmp( ImageData, Op->ImageData, ImageDataSize ) != 0 ) return false;
    break;

  case 2: //text load
    if ( memcmp( &TextData, &Op->TextData, 5 ) != 0 ) return false;
    if ( strlen( Text ) != strlen( Op->Text ) ) return false;
    if ( strcmp( Text, Op->Text ) != 0 ) return false;
    break;

  case 3: //spline
    return false; //TODO TODO TODO
    break;

  case 4: //noise
    break;

  default:
    break;
  }

  TBOOL ParentsMatch = true;

  for ( TU32 x = 0; x < Filter->Filter.DataDescriptor.InputCount; x++ )
  {
    CphxTextureOperator_Tool *p1 = ( (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, x ) );
    if ( p1 ) p1 = p1->GetContentOp();

    CphxTextureOperator_Tool *p2 = ( (CphxTextureOperator_Tool*)Op->GetParent( PHX_TEXTUREOPERATOR, x ) );
    if ( p2 ) p2 = p2->GetContentOp();

    if ( p1 != p2 )
      ParentsMatch &= p1->SubTreeMatch( p2 );
  }

  return ParentsMatch;
}

//////////////////////////////////////////////////////////////////////////
// misc

CphxTexturePoolTexture * CphxTexturePool_Tool::GetTexture( unsigned char Resolution, bool _hdr )
{
  TS64 GlobalSize = 0;
  for ( TS32 x = 0; x < Pool.ItemCount; x++ )
    if ( Pool[ x ]->Used )
      GlobalSize += GETXRES( Pool[ x ]->Resolution )*GETYRES( Pool[ x ]->Resolution ) * 8 * 2;

  if ( GlobalSize > 1024 * 1024 * ( (TS64)TexgenMemPoolSize ) )
    Project.FreeTextureMem( 5 );

  for ( int x = 0; x < Pool.ItemCount; x++ )
    if ( Pool[ x ]->Resolution == Resolution && Pool[ x ]->hdr == _hdr && !Pool[ x ]->Used )
    {
      Pool[ x ]->Used = true;
      return Pool[ x ];
    }


  //CphxTexturePoolTexture *p=CphxTexturePool::GetTexture(Resolution);
  //if (p) 
  //{
  //	//if (((CphxTexturePoolTexture_Tool*)p)->Owner)
  //	//{
  //	//	((CphxTexturePoolTexture_Tool*)p)->Owner->Allocate(false);
  //	//	((CphxTexturePoolTexture_Tool*)p)->Owner=NULL;
  //	//}
  //	return p;
  //}

  CphxTexturePoolTexture *p = NULL;

  CphxTexturePoolTexture_Tool *t = new CphxTexturePoolTexture_Tool;
  if ( !t->Create( Resolution, _hdr!=0 ) )
  {
    delete t;
    Project.FreeTextureMem( 25 );

    //p=CphxTexturePool::GetTexture(Resolution);
    p = NULL;
    for ( int x = 0; x < Pool.ItemCount; x++ )
      if ( Pool[ x ]->Resolution == Resolution && !Pool[ x ]->Used )
      {
        Pool[ x ]->Used = true;
        p = Pool[ x ];
        break;
      }

    if ( p ) return p;
    t = new CphxTexturePoolTexture_Tool;
    t->Create( Resolution, _hdr != 0 );
  }
  //t=new CphxTexturePoolTexture_Tool;
  //t->Create(Resolution);
  //CphxTexturePoolTexture **tp=new CphxTexturePoolTexture*[PoolSize+1];
  //memcpy(tp,Pool,sizeof(CphxTexturePoolTexture*)*PoolSize);
  //tp[PoolSize]=t;

  Pool.Add( t );
  t->Used = true;

  //delete[] Pool;
  //Pool=tp;
  //PoolSize++;

  GlobalSize = 0;
  for ( TS32 x = 0; x < Pool.ItemCount; x++ )
    GlobalSize += GETXRES( Pool[ x ]->Resolution )*GETYRES( Pool[ x ]->Resolution ) * 8 * 2;

  //LOG_DBG("[texgen] pool size: %d MB",GlobalSize/1024/1024);

  return t;
}

CphxTexturePool_Tool::CphxTexturePool_Tool()
{
  pool = NULL;
  poolSize = 0;
  Pool.Array = NULL;
  Pool.ItemCount = 0;
}

void CphxTexturePool_Tool::Clear()
{
  for ( int x = 0; x < Pool.ItemCount; x++ )
    if ( Pool[ x ] )
    {
      //if ( Pool[ x ]->View ) Pool[ x ]->View->Release();
      //if ( Pool[ x ]->RTView ) Pool[ x ]->RTView->Release();
      //if ( Pool[ x ]->Texture ) Pool[ x ]->Texture->Release();
      SAFEDELETE( Pool[ x ] );
    }

  Pool.ItemCount = 0;
  SAFEDELETEA( Pool.Array );
}

CphxTextureFilterParameter::CphxTextureFilterParameter()
{
  Name = CString( _T( "New Parameter" ) );
  Type = 0;
  Minimum = 0;
  Maximum = 255;
  Default = 127;
}

CphxTexturePoolTexture_Tool::CphxTexturePoolTexture_Tool()
{
  //Owner=NULL;
  View = NULL;
  RTView = NULL;
  Texture = NULL;
}

CphxTexturePoolTexture_Tool::~CphxTexturePoolTexture_Tool()
{
  if ( View ) View->Release();
  if ( RTView ) RTView->Release();
  if ( Texture ) Texture->Release();
  View = NULL;
  RTView = NULL;
  Texture = NULL;
}

TS32 DumpTextureID = 0;

TS32 CphxTexturePoolTexture_Tool::DumpTexture()
{
  CString s = CString::Format( _T( "%.4d.dds" ), DumpTextureID );

  HRESULT res = D3DX11SaveTextureToFile( phxContext, Texture, D3DX11_IFF_DDS, s.GetPointer() );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[texture] Failed to dump texture (%s)" ), err.ErrorMessage() );
    return DumpTextureID++;
  }

  return DumpTextureID++;
}

TBOOL CphxTextureOperator_NOP::GenerateResource( CCoreDevice *Dev )
{
  Touch();
  return UpToDate && GetContentOp() && GetContentOp()->ContentReady();
}

TBOOL CphxTextureOperator_NOP::InputsValid()
{
  TS32 opcount = GetParentCount( PHX_TEXTUREOPERATOR );
  return opcount == 1;
}

CphxTextureOperator_Tool * CphxTextureOperator_NOP::Copy()
{
  CphxTextureOperator_NOP *n = new CphxTextureOperator_NOP( Position, ParentPage );
  n->OpData = OpData;
  n->FilterGuid = FilterGuid;
  n->Filter = Filter;
  n->OldPosition = Position;
  n->Name = Name;
  if ( Filter )
    n->AddParent( Filter );
  n->CopyLookupData( this );
  return n;
}

CphxTextureOperator_NOP::CphxTextureOperator_NOP( CRect Pos, CapexTexGenPage *parentPage ) : CphxTextureOperator_Tool( Pos, parentPage )
{

}

CphxTextureOperator_NOP::~CphxTextureOperator_NOP()
{

}

CphxTextureOperator_Tool * CphxTextureOperator_NOP::GetContentOp()
{
  if ( !GetParent( PHX_TEXTUREOPERATOR, 0 ) ) return NULL;
  return ( (CphxTextureOperator_Tool*)GetParent( PHX_TEXTUREOPERATOR, 0 ) )->GetContentOp();
}

void CphxTextureOperator_NOP::LockMechanism( TBOOL Lock )
{

}

void CphxTextureOperator_NOP::AllocatorMechanism( TBOOL Alloc )
{

}

void CphxTextureOperator_NOP::ExportData( CXMLNode *Node )
{
  if ( Name.Length() ) Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "x1" ), false ).SetInt( Position.x1 );
  Node->AddChild( _T( "y1" ), false ).SetInt( Position.y1 );
  Node->AddChild( _T( "x2" ), false ).SetInt( Position.x2 );
  Node->AddChild( _T( "y2" ), false ).SetInt( Position.y2 );
}

void CphxTextureOperator_NOP::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  if ( Node->GetChildCount( _T( "x1" ) ) ) Node->GetChild( _T( "x1" ), false ).GetValue( Position.x1 );
  if ( Node->GetChildCount( _T( "y1" ) ) ) Node->GetChild( _T( "y1" ), false ).GetValue( Position.y1 );
  if ( Node->GetChildCount( _T( "x2" ) ) ) Node->GetChild( _T( "x2" ), false ).GetValue( Position.x2 );
  if ( Node->GetChildCount( _T( "y2" ) ) ) Node->GetChild( _T( "y2" ), false ).GetValue( Position.y2 );
  OldPosition = Position;
}

TU32 CphxTextureOperator_NOP::GetInputCount()
{
  return 1;
}

TBOOL CphxTextureOperator_NOP::ContentReady()
{
  if ( GetContentOp() ) return GetContentOp()->ContentReady();
  return false;
}

CString CphxTextureOperator_NOP::GetName()
{
  if ( Name.Length() ) return Name;
  return CString( _T( "NOP" ) );
}

TBOOL CphxTextureOperator_NOP::IsRequiredForGenerator()
{
  return true;
}

CString CphxTextureOperator_Save::GetName()
{
  if ( Name.Length() ) return Name;
  return CString( _T( "Save" ) );
}

CphxTextureOperator_Save::CphxTextureOperator_Save( CRect Pos, CapexTexGenPage *parentPage ) : CphxTextureOperator_NOP( Pos, parentPage )
{

}

CphxTextureOperator_Save::~CphxTextureOperator_Save()
{

}

CphxTextureOperator_Tool * CphxTextureOperator_Save::Copy()
{
  CphxTextureOperator_Save *s = new CphxTextureOperator_Save( Position, ParentPage );
  s->OpData = OpData;
  s->FilterGuid = FilterGuid;
  s->Filter = Filter;
  s->OldPosition = Position;
  s->Name = Name;
  if ( Filter )
    s->AddParent( Filter );
  s->ApplyItemSpecificParents();
  s->CopyLookupData( this );
  return s;
}

TBOOL CphxTextureOperator_Save::IsRequiredForGenerator()
{
  return true;
}


CphxTextureOperator_Load::CphxTextureOperator_Load( CRect Pos, CapexTexGenPage *parentPage ) : CphxTextureOperator_NOP( Pos, parentPage )
{
  LoadedOp = NULL;
}

CphxTextureOperator_Load::~CphxTextureOperator_Load()
{

}

CString CphxTextureOperator_Load::GetName()
{
  if ( LoadedOp ) if ( LoadedOp->GetName() != _T( "Save" ) ) return _T( "*" ) + LoadedOp->GetName() + _T( "*" );

  if ( Name.Length() ) return Name;
  return CString( _T( "Load" ) );
}

void CphxTextureOperator_Load::ApplyItemSpecificParents()
{
  RemoveParents( PHX_TEXTUREOPERATOR );
  if ( LoadedOp )
    AddParent( LoadedOp );
}

void CphxTextureOperator_Load::RemoveItemSpecificParents()
{
  RemoveParents( PHX_TEXTUREOPERATOR );
}

void CphxTextureOperator_Load::ExportData( CXMLNode *Node )
{
  if ( Name.Length() ) Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "x1" ), false ).SetInt( Position.x1 );
  Node->AddChild( _T( "y1" ), false ).SetInt( Position.y1 );
  Node->AddChild( _T( "x2" ), false ).SetInt( Position.x2 );
  Node->AddChild( _T( "y2" ), false ).SetInt( Position.y2 );
  Node->AddChild( _T( "Resolution" ), false ).SetInt( OpData.Resolution );

  if ( LoadedOp )
    Node->AddChild( _T( "LoadedOp" ), false ).SetText( LoadedOp->GetGUID().GetString() );
}

CphxTextureOperator_Tool * CphxTextureOperator_Load::Copy()
{
  CphxTextureOperator_Load *s = new CphxTextureOperator_Load( Position, ParentPage );
  s->OpData = OpData;
  s->FilterGuid = FilterGuid;
  s->Filter = Filter;
  s->OldPosition = Position;
  s->Name = Name;
  s->LoadedOp = LoadedOp;
  if ( Filter )
    s->AddParent( Filter );
  s->ApplyItemSpecificParents();
  s->CopyLookupData( this );
  return s;
}


void CphxTextureOperator_Load::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  if ( Node->GetChildCount( _T( "x1" ) ) ) Node->GetChild( _T( "x1" ), false ).GetValue( Position.x1 );
  if ( Node->GetChildCount( _T( "y1" ) ) ) Node->GetChild( _T( "y1" ), false ).GetValue( Position.y1 );
  if ( Node->GetChildCount( _T( "x2" ) ) ) Node->GetChild( _T( "x2" ), false ).GetValue( Position.x2 );
  if ( Node->GetChildCount( _T( "y2" ) ) ) Node->GetChild( _T( "y2" ), false ).GetValue( Position.y2 );
  OldPosition = Position;

  if ( Node->GetChildCount( _T( "LoadedOp" ) ) )
  {
    LoadTime_TargetOpGUID.SetString( Node->GetChild( _T( "LoadedOp" ), false ).GetText().GetPointer() );
  }
  else
    LOG_ERR( "[import] Texture load %s missing LoadedOp id field!", GetGUID().GetString() );
}

void CphxTextureOperator_Load::KillLoopCause()
{
  RemoveItemSpecificParents();
  LoadedOp = NULL;
}

TBOOL CphxTextureOperator_Load::IsRequiredForGenerator()
{
  return true;
}

CphxTextureOperator_Subroutine::CphxTextureOperator_Subroutine( CRect Pos, CapexTexGenPage *parentPage ) : CphxTextureOperator_NOP( Pos, parentPage )
{

}

CphxTextureOperator_Subroutine::~CphxTextureOperator_Subroutine()
{
  Parameters.FreeArray();
}

CString CphxTextureOperator_Subroutine::GetName()
{
  if ( Name.Length() ) return Name;
  return CString( _T( "Subroutine" ) );
}

void CphxTextureOperator_Subroutine::SetCaller( CphxTextureOperator_SubroutineCall *Caller )
{
  FreeSubroutineAllocationsandUpdateResolution( this, Caller->OpData.Resolution );
  ClearSubroutineTags( this );

  //set input textures here

  TS32 InputIdx = 0;

  TS32 NumInputs = CountSubInputOps( this );
  if ( NumInputs > 0 )
  {
    CphxResource **Inputs = new CphxResource*[ NumInputs ];
    CphxTextureOperator_Tool **InputOps = new CphxTextureOperator_Tool*[ NumInputs ];

    Caller->RemoveItemSpecificParents();
    Caller->GetParentsOfType( PHX_TEXTUREOPERATOR, Inputs, NumInputs );
    Caller->ApplyItemSpecificParents();

    for ( TS32 x = 0; x < NumInputs; x++ )
      InputOps[ x ] = ( (CphxTextureOperator_Tool*)Inputs[ x ] );

    ClearSubroutineTags( this );
    SetSubroutineInputOperators( this, InputOps, NumInputs, InputIdx );

    delete[] Inputs;
    delete[] InputOps;
  }

  //set input values here
  for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
  {
    TS32 start = GetParamStart( x );
    TS32 size = GetParamSize( x );

    for ( TS32 y = 0; y < Parameters[ x ]->Targets.NumItems(); y++ )
    {
      CphxSubroutineParamConnection *pconn = Parameters[ x ]->Targets[ y ];
      CphxTextureOperator_Tool *op = Project.GetTexgenOp( pconn->TargetID );
      if ( op )
      {
        TS32 targetstart = 0;
        if ( pconn->OpParam != -1 )
        {
          if ( op->Filter ) targetstart = op->Filter->GetParamStart( pconn->OpParam );
          if ( op->GetOpType() == TEXGEN_OP_SUBROUTINECALL ) targetstart = ( (CphxTextureOperator_SubroutineCall*)op )->Subroutine->GetParamStart( pconn->OpParam );
          op->OpData.Parameters[ targetstart ] = Caller->OpData.Parameters[ start ];
        }
        else
          op->OpData.RandSeed = Caller->OpData.Parameters[ start ];
      }
      else
        LOG_WARN( "[texgen] Subroutine %s referenced unknown operator (param %s target %d)", GetName().GetPointer(), Parameters[ x ]->Name.GetPointer(), y );
    }
  }

}


TS32 CphxTextureOperator_Subroutine::GetParamStart( TS32 ParamID )
{
  TS32 v = 0;
  for ( TS32 x = 0; x < ParamID; x++ )
    v += GetParamSize( x );
  return v;
}

TS32 CphxTextureOperator_Subroutine::GetParamSize( TS32 ParamID )
{
  if ( ParamID < 0 || ParamID >= Parameters.NumItems() ) return 0;

  switch ( Parameters[ ParamID ]->Type )
  {
  case 0:
  case 1:
  case 3:
  case 4: return 1;
  case 2: return 4;
  }
  return 0;
}

TBOOL CphxTextureOperator_Subroutine::InputsValid()
{
  return GetParentCount();
}

TBOOL CphxTextureOperator_Subroutine::CanBeGenerated()
{
  return GetParentCount();
}

void CphxTextureOperator_Subroutine::ExportData( CXMLNode *Node )
{
  if ( Name.Length() ) Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "x1" ), false ).SetInt( Position.x1 );
  Node->AddChild( _T( "y1" ), false ).SetInt( Position.y1 );
  Node->AddChild( _T( "x2" ), false ).SetInt( Position.x2 );
  Node->AddChild( _T( "y2" ), false ).SetInt( Position.y2 );

  for ( TS32 x = 0; x < Parameters.NumItems(); x++ )
  {
    CXMLNode n = Node->AddChild( _T( "SubParameter" ), x == Parameters.NumItems() - 1 );
    n.AddChild( _T( "Name" ) ).SetText( Parameters[ x ]->Name.GetPointer() );
    n.AddChild( _T( "Type" ) ).SetInt( Parameters[ x ]->Type );
    n.AddChild( _T( "Min" ) ).SetInt( Parameters[ x ]->Minimum );
    n.AddChild( _T( "Max" ) ).SetInt( Parameters[ x ]->Maximum );
    n.AddChild( _T( "Default" ) ).SetInt( Parameters[ x ]->Default );
    for ( TS32 y = 0; y < Parameters[ x ]->Targets.NumItems(); y++ )
    {
      CXMLNode t = n.AddChild( _T( "ParamTarget" ), y == Parameters[ x ]->Targets.NumItems() - 1 );
      t.AddChild( _T( "Target" ) ).SetText( Parameters[ x ]->Targets[ y ]->TargetGUID.GetString() );
      t.AddChild( _T( "TargetParameter" ) ).SetInt( Parameters[ x ]->Targets[ y ]->OpParam );
    }
  }
}

void CphxTextureOperator_Subroutine::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  if ( Node->GetChildCount( _T( "x1" ) ) ) Node->GetChild( _T( "x1" ), false ).GetValue( Position.x1 );
  if ( Node->GetChildCount( _T( "y1" ) ) ) Node->GetChild( _T( "y1" ), false ).GetValue( Position.y1 );
  if ( Node->GetChildCount( _T( "x2" ) ) ) Node->GetChild( _T( "x2" ), false ).GetValue( Position.x2 );
  if ( Node->GetChildCount( _T( "y2" ) ) ) Node->GetChild( _T( "y2" ), false ).GetValue( Position.y2 );
  OldPosition = Position;

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "SubParameter" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "SubParameter" ), x );

    CphxSubroutineParam *p = new CphxSubroutineParam();

    p->Type = 0;
    p->Minimum = 0;
    p->Maximum = 255;
    p->Default = 0;


    if ( n.GetChildCount( _T( "Name" ) ) ) p->Name = n.GetChild( _T( "Name" ) ).GetText();
    if ( n.GetChildCount( _T( "Type" ) ) ) n.GetChild( _T( "Type" ) ).GetValue( p->Type );
    if ( n.GetChildCount( _T( "Min" ) ) ) n.GetChild( _T( "Min" ) ).GetValue( p->Minimum );
    if ( n.GetChildCount( _T( "Max" ) ) ) n.GetChild( _T( "Max" ) ).GetValue( p->Maximum );
    if ( n.GetChildCount( _T( "Default" ) ) ) n.GetChild( _T( "Default" ) ).GetValue( p->Default );

    for ( TS32 y = 0; y < n.GetChildCount( _T( "ParamTarget" ) ); y++ )
    {
      CXMLNode nx = n.GetChild( _T( "ParamTarget" ), y );
      CphxSubroutineParamConnection *c = new CphxSubroutineParamConnection();
      if ( nx.GetChildCount( _T( "Target" ) ) )
        c->TargetGUID.SetString( nx.GetChild( _T( "Target" ) ).GetText().GetPointer() );
      if ( nx.GetChildCount( _T( "TargetParameter" ) ) )
        nx.GetChild( _T( "TargetParameter" ) ).GetValue( c->OpParam );
      p->Targets += c;
    }

    Parameters += p;
  }

}

CphxTextureOperator_Tool * CphxTextureOperator_Subroutine::Copy()
{
  CphxTextureOperator_Subroutine *s = new CphxTextureOperator_Subroutine( Position, ParentPage );
  s->OpData = OpData;
  s->FilterGuid = FilterGuid;
  s->Filter = Filter;
  s->OldPosition = Position;
  s->Name = Name;
  if ( Filter )
    s->AddParent( Filter );
  s->ApplyItemSpecificParents();
  s->CopyLookupData( this );
  return s;
}

TBOOL CphxTextureOperator_Subroutine::IsRequiredForGenerator()
{
  return CphxResource::IsRequiredForGenerator();
}


CphxTextureOperator_SubroutineCall::CphxTextureOperator_SubroutineCall( CRect Pos, CapexTexGenPage *parentPage ) : CphxTextureOperator_Tool( Pos, parentPage )
{
  Subroutine = NULL;
}

CphxTextureOperator_SubroutineCall::~CphxTextureOperator_SubroutineCall()
{

}

CString CphxTextureOperator_SubroutineCall::GetName()
{
  if ( !Subroutine ) return CString( _T( "SubCall" ) );
  return _T( "'" ) + Subroutine->GetName() + _T( "' Call" );
}

extern CArray<CphxResource*> UpdateQueue;

void CphxTextureOperator_SubroutineCall::RequestContent()
{
  if ( ContentReady() ) return;
  if ( !CanBeGenerated() ) return;

  for ( TS32 x = 0; x < GetParentCount(); x++ )
    if ( GetParent( x ) != Subroutine )
      GetParent( x )->RequestContent();

  CphxResource *ct = GetContentResource();

  if ( UpdateQueue.Find( ct ) >= 0 ) return;
  UpdateQueue += ct;
}

TBOOL CphxTextureOperator_SubroutineCall::GenerateResource( CCoreDevice *Dev )
{
  Touch();
  if ( !Subroutine || !Subroutine->GetContentOp() ) return false;
  //Dev->CaptureCurrentFrame();

  Subroutine->SetCaller( this );
  CphxTextureOperator_Tool *ContentOp = Subroutine->GetContentOp();
  UpToDate = ContentOp->GenerateResource( Dev );

  //hijack result of content operator
  Allocate( true );
  CphxTexturePoolTexture_Tool *t = Result;
  Result = ContentOp->Result;
  ContentOp->Result = t;
  ContentOp->Allocate( false );

  //if (!ContentOp->IsAllocated()) 
  //	ContentOp->Result->Used=false;

  //LOG_DBG("[texgen] Subroutine call result: %d %d allocated: %d",Result,Result->Used,IsAllocated());
  //LOG_DBG("[texgen] result dumped to %.4d.png",Result->DumpTexture());

  //CaptureCurrentFrame

  //clear sub inputs
  Subroutine->ClearSubInputs( Subroutine );

  return true;
}

TBOOL CphxTextureOperator_SubroutineCall::InputsValid()
{
  if ( !Subroutine ) return false;

  TS32 opcount = GetParentCount( PHX_TEXTUREOPERATOR );
  //for (TS32 x=0; x<Parents.NumItems(); x++)
  //	if (Parents[x]->GetType()==PHX_TEXTUREOPERATOR) opcount++;

  return opcount == GetInputCount() + 1;
}

CphxTextureOperator_Tool * CphxTextureOperator_SubroutineCall::Copy()
{
  CphxTextureOperator_SubroutineCall *s = new CphxTextureOperator_SubroutineCall( Position, ParentPage );
  s->OpData = OpData;
  s->FilterGuid = FilterGuid;
  s->Filter = Filter;
  s->OldPosition = Position;
  s->Name = Name;
  if ( Filter )
    s->AddParent( Filter );
  s->Subroutine = Subroutine;
  s->ApplyItemSpecificParents();
  s->CopyLookupData( this );
  return s;
}

TU32 CphxTextureOperator_SubroutineCall::GetInputCount()
{
  if ( !Subroutine ) return 0;
  Subroutine->ClearSubroutineTags( Subroutine );
  return Subroutine->CountSubInputOps( Subroutine );
}

void CphxTextureOperator_SubroutineCall::ApplyItemSpecificParents()
{
  if ( Subroutine )
    AddParent( Subroutine );
}

void CphxTextureOperator_SubroutineCall::RemoveItemSpecificParents()
{
  if ( Subroutine )
    RemoveParent( Subroutine );
}

void CphxTextureOperator_SubroutineCall::ExportData( CXMLNode *Node )
{
  if ( Name.Length() ) Node->AddChild( _T( "Name" ), false ).SetText( Name.GetPointer() );
  Node->AddChild( _T( "x1" ), false ).SetInt( Position.x1 );
  Node->AddChild( _T( "y1" ), false ).SetInt( Position.y1 );
  Node->AddChild( _T( "x2" ), false ).SetInt( Position.x2 );
  Node->AddChild( _T( "y2" ), false ).SetInt( Position.y2 );

  if ( Subroutine )
    Node->AddChild( _T( "Subroutine" ), false ).SetText( Subroutine->GetGUID().GetString() );

  Node->AddChild( _T( "Resolution" ), false ).SetInt( OpData.Resolution );
  for ( TS32 x = 0; x < TEXGEN_MAX_PARAMS; x++ )
  {
    CXMLNode n = Node->AddChild( _T( "Parameter" ), x == TEXGEN_MAX_PARAMS - 1 );
    n.SetAttributeFromInteger( _T( "ID" ), x );
    n.SetInt( OpData.Parameters[ x ] );
  }
}

void CphxTextureOperator_SubroutineCall::ImportData( CXMLNode *Node )
{
  if ( Node->GetChildCount( _T( "Name" ) ) ) Name = Node->GetChild( _T( "Name" ) ).GetText();

  if ( Node->GetChildCount( _T( "x1" ) ) ) Node->GetChild( _T( "x1" ), false ).GetValue( Position.x1 );
  if ( Node->GetChildCount( _T( "y1" ) ) ) Node->GetChild( _T( "y1" ), false ).GetValue( Position.y1 );
  if ( Node->GetChildCount( _T( "x2" ) ) ) Node->GetChild( _T( "x2" ), false ).GetValue( Position.x2 );
  if ( Node->GetChildCount( _T( "y2" ) ) ) Node->GetChild( _T( "y2" ), false ).GetValue( Position.y2 );
  OldPosition = Position;

  if ( Node->GetChildCount( _T( "Subroutine" ) ) )
  {
    Loadtime_SubroutineGUID.SetString( Node->GetChild( _T( "Subroutine" ), false ).GetText().GetPointer() );
  }
  else
    LOG_ERR( "[import] Texture subroutine call %s missing subroutine id field!", GetGUID().GetString() );

  if ( Node->GetChildCount( _T( "Resolution" ) ) ) Node->GetChild( _T( "Resolution" ), false ).GetValue( OpData.Resolution );

  for ( TS32 x = 0; x < Node->GetChildCount( _T( "Parameter" ) ); x++ )
  {
    CXMLNode n = Node->GetChild( _T( "Parameter" ), x );
    TS32 attrn = x;
    if ( n.HasAttribute( _T( "ID" ) ) ) n.GetAttributeAsInteger( _T( "ID" ), &attrn );
    TS32 val = 0;
    n.GetValue( val );
    OpData.Parameters[ attrn ] = val;
  }

}

void CphxTextureOperator_SubroutineCall::KillLoopCause()
{
  for ( TS32 x = 0; x < GetChildCount(); x++ )
  {
    if ( GetChild( x )->GetType() == PHX_TEXTUREOPERATOR )
    {
      GetChild( x )->RemoveParent( this );
      x--;
    }
  }
  if ( SubroutineRoot )
    ClearSubRoots( SubroutineRoot );

}

TBOOL CphxTextureOperator_SubroutineCall::ParentContentReady()
{
  TBOOL r = true;
  for ( int x = 0; x < GetParentCount(); x++ )
    if ( GetParent( x ) != Subroutine )
      r = r&&GetParent( x )->ContentReady();
  return r;
}

CphxSubroutineParam::~CphxSubroutineParam()
{
  Targets.FreeArray();
}

CphxSubroutineParam::CphxSubroutineParam()
{
  Minimum = 0;
  Maximum = 255;
  Default = 127;
  Type = 0;
}

CphxSubroutineParamConnection::CphxSubroutineParamConnection()
{
  TargetID = 0;
  OpParam = 0;
}