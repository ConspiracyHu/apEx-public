#include "BasePCH.h"
#include "ModelParameters.h"
#define WINDOWNAME _T("Model Parameters")
#define WINDOWXML _T("ModelParameters")
#include "WorkBench.h"
#include "ModelMaterial.h"
#include "apExRoot.h"

CString MeshUITemplateNames[] =
{
  _T( "cube" ),
  _T( "plane" ),
  _T( "sphere" ),
  _T( "cylinder" ),
  _T( "cone" ),
  _T( "arc" ),
  _T( "line" ),
  _T( "spline" ),
  _T( "loft" ),
  _T( "clone" ),
  _T( "bake" ),
  _T( "geosphere" ),
  _T( "scatter" ),
  _T( "stored" ),
  _T( "tree" ),
  _T( "leaves" ),
  _T( "text" ),
  _T( "marchingmesh" ),
  _T( "storedminimesh" ),
  _T( "merge" ),
};

CString FilterUITemplateNames[] =
{
  _T( "uvmap" ),
  _T( "bevel" ),
  _T( "mapxform" ),
  _T( "meshsmooth" ),
  _T( "smoothgroup" ),
  _T( "tint" ),
  _T( "tintshape" ),
  _T( "replicate" ),
  _T( "normaldeform" ),
  _T( "csg" ),
  _T( "greeble" ),
};

extern CapexRoot *Root;

void CapexModelParameters::LoadCSS()
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

CapexModelParameters::CapexModelParameters() : CapexWindow()
{
  LoadCSS();
  InitTrackBarMap();
}

CapexModelParameters::CapexModelParameters( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
  LoadCSS();
  InitTrackBarMap();
}

void CapexModelParameters::ReloadLayout()
{
  LoadCSS();
  CapexWindow::ReloadLayout();
}

CapexModelParameters::~CapexModelParameters()
{

}

void CapexModelParameters::UpdateData()
{
  DeleteChildren();

  if ( !WorkBench->GetEditedModel() || WorkBench->GetEditedModel()->GetObjectIndex( WorkBench->GetEditedModelObject() ) < 0 )
    return;

  if ( WorkBench->GetEditedModelFilter() )
  {
    UpdateObjectFilterUI();
    return;
  }

  if ( WorkBench->GetEditedModelObject() )
    UpdateObjectParamUI();
}

void CapexModelParameters::UpdateObjectParamUI()
{
  auto Obj = WorkBench->GetEditedModelObject();

  if ( !Obj ) return;

  App->GenerateGUITemplate( this, CString( WINDOWXML ), CString( _T( "modelparams" ) ) );

  CWBBox *parambox = (CWBBox*)FindChildByID( _T( "parambox" ), _T( "box" ) );
  if ( !parambox ) return;

  App->GenerateGUITemplate( parambox, CString( WINDOWXML ), MeshUITemplateNames[ Obj->GetPrimitive() ] );

  StyleManager.ApplyStyles( this );
  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );

  TU8 *Params = Obj->GetParameters();

  switch ( Obj->GetPrimitive() )
  {
  case Mesh_Cube:
    break;
  case Mesh_Plane:
    SetTrackBarValue( _T( "planex" ) );
    SetTrackBarValue( _T( "planey" ) );
    break;
  case Mesh_Sphere:
    SetTrackBarValue( _T( "spherex" ) );
    SetTrackBarValue( _T( "spherey" ) );
    SetTrackBarValue( _T( "spheretop" ) );
    SetTrackBarValue( _T( "spherebottom" ) );
    SetButtonValue( _T( "spherecap" ) );
    break;
  case Mesh_Cylinder:
    SetTrackBarValue( _T( "cylinderx" ) );
    SetTrackBarValue( _T( "cylindery" ) );
    SetButtonValue( _T( "cylindercap" ) );
    break;
  case Mesh_Cone:
    SetTrackBarValue( _T( "conex" ) );
    SetTrackBarValue( _T( "coney" ) );
    SetTrackBarValue( _T( "conetop" ) );
    SetButtonValue( _T( "conecap" ) );
    break;
  case Mesh_Arc:
    SetTrackBarValue( _T( "arcres" ) );
    SetTrackBarValue( _T( "arcbend" ) );
    SetButtonValue( _T( "arcclosed" ) );
    break;
  case Mesh_Line:
    SetTrackBarValue( _T( "lineres" ) );
    break;
  case Mesh_Spline:
    SetTrackBarValue( _T( "splineres" ) );
    SetButtonValue( _T( "splineclosed" ) );
    break;
  case Mesh_Loft:
    BuildArcList( _T( "loftpathlist" ) );
    BuildArcList( _T( "loftslicelist" ) );
    SetButtonValue( _T( "loftpathclose" ) );
    SetButtonValue( _T( "loftsliceclose" ) );
    SetTrackBarValue( _T( "loftrotation" ) );
    SetTrackBarValue( _T( "loftstartscale" ) );
    SetTrackBarValue( _T( "loftendscale" ) );
    break;
  case Mesh_Clone:
    BuildCloneSourceList( _T( "clonesourcelist" ) );
    BuildCloneContentList( _T( "clonecontentlist" ) );
    break;
  case Mesh_Copy:
    BuildCopyList( _T( "copyparentlist" ) );
    break;
  case Mesh_Merge:
    BuildMergeSourceList( _T( "mergesourcelist" ) );
    BuildMergeContentList( _T( "mergecontentlist" ) );
    break;
  case Mesh_GeoSphere:
    SetTrackBarValue( _T( "georesolution" ) );
    break;
  case Mesh_Scatter:
    BuildMeshList( _T( "scatterobj" ) );
    BuildMeshList( _T( "scattershape" ) );
    SetTrackBarValue( _T( "scatterseed" ) );
    SetTrackBarValue( _T( "scattervxamout" ) );
    SetTrackBarValue( _T( "scatteredgeamount" ) );
    SetTrackBarValue( _T( "scatterpolyamount" ) );
    SetTrackBarValue( _T( "scattermaxperpoly" ) );
    SetTrackBarValue( _T( "scatterxformchannel" ) );
    SetTrackBarValue( _T( "scatterscalethresh" ) );
    SetlistSelection( _T( "scatterorientation" ) );
    SetButtonValue( _T( "scatteronlyyscale" ) );
    SetTrackBarValue( _T( "scatteramountchannel" ) );
    SetTrackBarValue( _T( "scattermovethresh" ) );
    {
      CWBTextBox *ib = (CWBTextBox *)FindChildByID( _T( "scattermovethresh" ), _T( "textbox" ) );
      if ( ib )
        ib->SetText( CString::Format( _T( "%f" ), (float)Obj->FloatParameter ) );
    }
    break;
  case Mesh_Stored:
    break;
  case Mesh_Tree:
    SetTrackBarValue( _T( "treerandseed" ) );
    SetTrackBarValue( _T( "treedensity1" ) );
    SetTrackBarValue( _T( "treedensity2" ) );
    SetTrackBarValue( _T( "treedensity3" ) );
    SetTrackBarValue( _T( "treedensity4" ) );
    BuildTreeSpeciesList( _T( "treespecies" ) );
    break;
  case Mesh_TreeLeaves:
    SetTrackBarValue( _T( "leafrandseed" ) );
    SetTrackBarValue( _T( "treedensity1" ) );
    SetTrackBarValue( _T( "treedensity2" ) );
    SetTrackBarValue( _T( "treedensity3" ) );
    SetTrackBarValue( _T( "treedensity4" ) );
    SetTrackBarValue( _T( "treedensity5" ) );
    BuildTreeSpeciesList( _T( "treeleafspecies" ) );
    break;
  case Mesh_Text:
    SetlistSelection( _T( "fontlist" ) );
    SetTrackBarValue( _T( "textdeviation" ) );
    SetTextData( _T( "meshtext" ) );
    break;
  default:
    break;
  }
}

void CapexModelParameters::UpdateObjectFilterUI()
{
  auto Flt = WorkBench->GetEditedModelFilter();
  if ( !Flt ) return;

  App->GenerateGUITemplate( this, CString( WINDOWXML ), CString( _T( "filterparams" ) ) );

  CWBBox *parambox = (CWBBox*)FindChildByID( _T( "parambox" ), _T( "box" ) );
  if ( !parambox ) return;

  App->GenerateGUITemplate( parambox, CString( WINDOWXML ), FilterUITemplateNames[ Flt->Filter ] );

  CWBButton *v = (CWBButton*)FindChildByID( _T( "disablefilter" ), _T( "button" ) );
  if ( v && Flt ) v->Push( !Flt->Enabled );

  StyleManager.ApplyStyles( this );
  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );

  //TU8 *Params = Obj->GetParameters();
  switch ( Flt->Filter )
  {
  case ModelFilter_UVMap:
    SetTrackBarValue_Filter( _T( "uvchannel" ) );
    SetlistSelection_Filter( _T( "uvtype" ) );
    SetButtonValue_Filter( _T( "clipuv" ) );
    SetSrtInputValue( _T( "uvscalex" ) );
    SetSrtInputValue( _T( "uvscaley" ) );
    SetSrtInputValue( _T( "uvscalez" ) );
    SetSrtInputValue( _T( "uvoffsetx" ) );
    SetSrtInputValue( _T( "uvoffsety" ) );
    SetSrtInputValue( _T( "uvoffsetz" ) );
    break;
  case ModelFilter_Bevel:
    SetTrackBarValue_Filter( _T( "bevelpercent" ) );
    break;
  case ModelFilter_MapXForm:
    break;
  case ModelFilter_MeshSmooth:
    SetButtonValue_Filter( _T( "meshsmoothlinear" ) );
    SetTrackBarValue_Filter( _T( "meshsmoothiterations" ) );
    break;
  case ModelFilter_SmoothGroup:
    SetTrackBarValue_Filter( _T( "smoothgroupvalue" ) );
    break;
  case ModelFilter_TintMesh:
    SetTrackBarValue_Filter( _T( "tintsaturation" ) );
    SetTrackBarValue_Filter( _T( "tintchannel" ) );
    BuildTextureList( _T( "tinttexture" ) );
    SetButtonTextureValue_Filter( _T( "tinttextureselector" ) );
    break;
  case ModelFilter_TintMeshShape:
    SetlistSelection_Filter( _T( "tintshapetype" ) );
    SetlistSelection_Filter( _T( "tintshapeoperator" ) );
    SetTrackBarValue_Filter( _T( "tintshapepower" ) );
    break;
  case ModelFilter_Replicate:
    SetTrackBarValue_Filter( _T( "replicatecount" ) );
    break;
  case ModelFilter_NormalDeform:
    SetTrackBarValue_Filter( _T( "normaldeformfraction" ) );
    SetTrackBarValue_Filter( _T( "normaldeforminteger" ) );
    break;
  case ModelFilter_CSG:
    BuildCSGList( _T( "csgobjlist" ) );
    SetlistSelection_Filter( _T( "csgmode" ) );
    break;
  case ModelFilter_Greeble:
    SetTrackBarValue_Filter( _T( "greebleseed" ) );
    SetTrackBarValue_Filter( _T( "greebleextrude" ) );
    SetTrackBarValue_Filter( _T( "greebletaper" ) );
    break;
  default:
    break;
  }
}

TBOOL CapexModelParameters::MessageProc( CWBMessage &Message )
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
  case WBM_VALUECHANGED:
  {
    CWBTrackBar *b = (CWBTrackBar*)App->FindItemByGuid( Message.GetTarget(), _T( "trackbar" ) );
    if ( !b ) break;

    if ( TrackBarTextMap.HasKey( b->GetID() ) )
    {
      int mask = 0xffffffff;
      if ( UIParameterFilterMap.HasKey( b->GetID() ) )
        mask = UIParameterFilterMap[ b->GetID() ];

      b->SetText( CString::Format( TrackBarTextMap[ b->GetID() ].GetPointer(), Message.Data & mask ) );
      if ( WorkBench->GetEditedModelFilter() && UIParameterMap.HasKey( b->GetID() ) )
      {
        if ( ( Message.Data & mask ) != ( WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] & mask ) )
        {
          unsigned char oldValue = WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ];
          WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] = ( oldValue & ~mask ) | ( Message.Data & mask );
          WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        }
        return true;
      }
      if ( WorkBench->GetEditedModelObject() && UIParameterMap.HasKey( b->GetID() ) )
      {
        if ( ( Message.Data & mask ) != ( WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] & mask ) )
        {
          unsigned char oldValue = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ];
          WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] = ( oldValue & ~mask ) | ( Message.Data & mask );
          WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
        }
      }
      return true;
    }

    break;
  }

  case WBM_COMMAND:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b )
    {
      CWBTextBox *i = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
      if ( !i ) break;
      if ( !WorkBench->GetEditedModelFilter() ) break;

      if ( !UIParameterMap.HasKey( i->GetID() ) ) break;

      TF32 add = 0;
      if ( UIParameterMap[ i->GetID() ] < 3 ) add = 1;

      TF32 value = 0;
      if ( i->GetText().Scan( _T( "%f" ), &value ) == 1 )
        WorkBench->GetEditedModelFilter()->srt[ UIParameterMap[ i->GetID() ] ] = value - add;

      WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();

      break;
    }

    if ( WorkBench->GetEditedModelFilter() && UIParameterMap.HasKey( b->GetID() ) )
    {
      int mask = 0x1;
      if ( UIParameterFilterMap.HasKey( b->GetID() ) )
        mask = UIParameterFilterMap[ b->GetID() ];

      unsigned char oldVal = WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ];

      WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] = ( oldVal & ( ~mask ) ) | ( ~oldVal & mask );
      b->Push( WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] & mask );
      WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
      return true;
    }

    if ( WorkBench->GetEditedModelObject() && UIParameterMap.HasKey( b->GetID() ) )
    {
      int mask = 0x1;
      if ( UIParameterFilterMap.HasKey( b->GetID() ) )
        mask = UIParameterFilterMap[ b->GetID() ];

      unsigned char oldVal = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ];
      WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] = ( oldVal & ( ~mask ) ) | ( ~oldVal & mask );
      b->Push( WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] & mask );
      WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
      return true;
    }

    if ( b->GetID() == _T( "cloneadd" ) )
    {
      CloneAdd();
      return true;
    }

    if ( b->GetID() == _T( "cloneremove" ) )
    {
      CloneRemove();
      return true;
    }

    if ( b->GetID() == _T( "mergeadd" ) )
    {
      MergeAdd();
      return true;
    }

    if ( b->GetID() == _T( "mergeremove" ) )
    {
      MergeRemove();
      return true;
    }

    if ( b->GetID() == _T( "resetpos" ) )
    {
      D3DXMATRIX i;
      D3DXMatrixIdentity( &i );
      if ( WorkBench->GetEditedModelObject() )
      {
        WorkBench->GetEditedModelObject()->SetMatrix( i );
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
      return true;
    }

    if ( b->GetID() == _T( "resettransform" ) )
    {
      D3DXMATRIX i;
      D3DXMatrixIdentity( &i );
      if ( WorkBench->GetEditedModelFilter() )
      {
        for ( TS32 x = 0; x < 12; x++ )
          WorkBench->GetEditedModelFilter()->srt[ x ] = 0;

        //rotate
        WorkBench->GetEditedModelFilter()->srt[ 6 ] = 1;
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();

        if ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_Replicate )
        {
          D3DXMATRIX m;
          D3DXMatrixIdentity( &m );
          WorkBench->GetEditedModelFilter()->SetRawMatrix( m );
          WorkBench->UpdateWindows( apEx_ModelMatrix );
        }
      }
      return true;
    }

    if ( b->GetID() == _T( "disablefilter" ) )
    {
      if ( WorkBench->GetEditedModelFilter() )
      {
        WorkBench->GetEditedModelFilter()->Enabled = !WorkBench->GetEditedModelFilter()->Enabled;
        b->Push( !WorkBench->GetEditedModelFilter()->Enabled );
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
      }

      return true;
    }

    if ( b->GetID() == _T( "deletefilter" ) )
    {
      if ( WorkBench->GetEditedModelFilter() )
      {
        CphxModelObject_Tool *eo = WorkBench->GetEditedModelFilter()->ParentObject;

        eo->RemoveParent( WorkBench->GetEditedModelFilter()->Texture );
        eo->DeleteFilter( WorkBench->GetEditedModelFilter() );

        WorkBench->SetEditedModelObject( eo );
        WorkBench->UpdateWindows( apEx_ModelGraph );
      }

      return true;
    }

    if ( b->GetID() == _T( "filterup" ) )
    {
      if ( WorkBench->GetEditedModelFilter() )
      {
        WorkBench->GetEditedModelFilter()->ParentObject->MoveFilterUp( WorkBench->GetEditedModelFilter() );
        WorkBench->UpdateWindows( apEx_ModelGraph );
      }
      return true;
    }

    if ( b->GetID() == _T( "filterdown" ) )
    {
      if ( WorkBench->GetEditedModelFilter() )
      {
        WorkBench->GetEditedModelFilter()->ParentObject->MoveFilterDown( WorkBench->GetEditedModelFilter() );
        WorkBench->UpdateWindows( apEx_ModelGraph );
      }
      return true;
    }

    if ( b->GetID() == _T( "fetchtransform" ) )
    {
      if ( WorkBench->GetEditedModelFilter() && WorkBench->GetEditedModelFilter()->Filter == ModelFilter_Replicate )
      {
        CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelFilter()->ParentObject;

        WorkBench->GetEditedModelFilter()->srt[ 0 ] = m->GetModelObject()->TransformationF16[ 0 ];
        WorkBench->GetEditedModelFilter()->srt[ 1 ] = m->GetModelObject()->TransformationF16[ 3 ];
        WorkBench->GetEditedModelFilter()->srt[ 2 ] = m->GetModelObject()->TransformationF16[ 6 ];
        WorkBench->GetEditedModelFilter()->srt[ 3 ] = m->GetModelObject()->TransformationF16[ 9 ];
        WorkBench->GetEditedModelFilter()->srt[ 4 ] = m->GetModelObject()->TransformationF16[ 1 ];
        WorkBench->GetEditedModelFilter()->srt[ 5 ] = m->GetModelObject()->TransformationF16[ 4 ];
        WorkBench->GetEditedModelFilter()->srt[ 6 ] = m->GetModelObject()->TransformationF16[ 7 ];
        WorkBench->GetEditedModelFilter()->srt[ 7 ] = m->GetModelObject()->TransformationF16[ 10 ];
        WorkBench->GetEditedModelFilter()->srt[ 8 ] = m->GetModelObject()->TransformationF16[ 2 ];
        WorkBench->GetEditedModelFilter()->srt[ 9 ] = m->GetModelObject()->TransformationF16[ 5 ];
        WorkBench->GetEditedModelFilter()->srt[ 10 ] = m->GetModelObject()->TransformationF16[ 8 ];
        WorkBench->GetEditedModelFilter()->srt[ 11 ] = m->GetModelObject()->TransformationF16[ 11 ];

        D3DXMATRIX i;
        D3DXMatrixIdentity( &i );
        m->SetMatrix( i );

        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        WorkBench->UpdateWindows( apEx_ModelMatrix );
      }
      return true;
    }

    if ( b->GetID() == _T( "bakefetchtransform" ) )
    {
      if ( WorkBench->GetEditedModelObject() && WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Copy )
      {
        CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelObject();
        CphxModelObject_Tool *t = Project.GetModelObject( WorkBench->GetEditedModelObject()->ParentGUIDS[ 0 ] );
        if ( t )
        {
          m->SetMatrix( t->GetMatrix() );
          WorkBench->UpdateWindows( apEx_ModelMatrix );
        }
      }
      return true;
    }

    if ( b->GetID() == _T( "tinttextureselector" ) )
    {
      BuildTintTextureSelectorUI( b );
      return true;
    }

    if ( b->GetID() == _T( "texturelocator" ) )
    {
      if ( WorkBench->GetEditedModelFilter() && WorkBench->GetEditedModelFilter()->Texture )
      {
        CphxTextureOperator_Tool *o = Project.GetTexgenOp( WorkBench->GetEditedModelFilter()->Texture->GetGUID() );
        if ( o )
          Root->GoToTexture( o, !App->GetCtrlState() );
      }
      return true;
    }


    break;
  }

  case WBM_CONTEXTMESSAGE:
  {
    CWBButton *b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b ) break;

    if ( WorkBench->GetEditedModelFilter() && b->GetID() == _T( "tinttextureselector" ) )
    {
      if ( TextureIDMap.HasKey( Message.Data ) )
      {
        CphxGUID tg = GUIDServer.RequestGUID();
        if ( WorkBench->GetEditedModelFilter()->Texture )
          tg = WorkBench->GetEditedModelFilter()->Texture->GetGUID();

        if ( tg != TextureIDMap[ Message.Data ] )
        {
          WorkBench->GetEditedModelFilter()->ParentObject->RemoveParent( WorkBench->GetEditedModelFilter()->Texture );
          WorkBench->GetEditedModelFilter()->Texture = Project.GetTexgenOp( TextureIDMap[ Message.Data ] );
          WorkBench->GetEditedModelFilter()->ParentObject->AddParent( Project.GetTexgenOp( TextureIDMap[ Message.Data ] ) );
          WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        }

        CphxGUID g = TextureIDMap[ Message.Data ];

        if ( Project.GetTexgenOp( g ) )
        {
          b->SetText( Project.GetTexgenOp( g )->GetName().GetPointer() );
          return true;
        }
        b->SetText( _T( "Select Texture" ) );

      }
      return true;
    }

  }
  break;

  case WBM_TEXTCHANGED:
  {
    CWBTextBox *b = (CWBTextBox*)App->FindItemByGuid( Message.GetTarget(), _T( "textbox" ) );
    if ( !b ) break;

    if ( WorkBench->GetEditedModelObject() ) //matrix editing
    {
      if ( b->GetID() == _T( "scattermovethresh" ) )
      {
        TF32 f = 0.0;
        if ( b->GetText().Scan( "%f", &f ) == 1 )
        {
          WorkBench->GetEditedModelObject()->FloatParameter = f;
          WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
        }
        return true;
      }
    }

    if ( b->GetID() == _T( "meshtext" ) )
    {
      if ( WorkBench->GetEditedModelObject() && WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Clone )
      {
        CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelObject();
        CString s = m->GetText();
        m->SetText( b->GetText() );
        if ( b->GetText() != s )
          WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
      }
      return true;
    }

    break;
  }

  case WBM_ITEMSELECTED:
  {
    CWBItemSelector *b = (CWBItemSelector*)App->FindItemByGuid( Message.GetTarget(), _T( "itemselector" ) );
    if ( !b ) break;

    if ( WorkBench->GetEditedModelFilter() && UIParameterMap.HasKey( b->GetID() ) && ( WorkBench->GetEditedModelFilter()->Filter == ModelFilter_CSG && b->GetID() == _T( "csgobjlist" ) ) )
    {
      WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] = Message.Data;
      TBOOL Changed = WorkBench->GetEditedModelFilter()->ReferencedGUID != WorkBench->GetEditedModelFilter()->ParentObject->GetParentModel()->GetObjectByIndex( Message.Data )->GetGUID();
      WorkBench->GetEditedModelFilter()->ReferencedGUID = WorkBench->GetEditedModelFilter()->ParentObject->GetParentModel()->GetObjectByIndex( Message.Data )->GetGUID();

      WorkBench->GetEditedModelFilter()->ParentObject->UpdateDependencies();
      if ( Changed )
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
      return true;
    }

    if ( WorkBench->GetEditedModelFilter() && UIParameterMap.HasKey( b->GetID() ) )
    {
      if ( b->GetCursorPosition() != WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] )
      {
        WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ b->GetID() ] ] = b->GetCursorPosition();
        WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
      }
      return true;
    }

    if ( WorkBench->GetEditedModelFilter() && b->GetID() == _T( "tinttexture" ) )
    {
      if ( TextureListMap.HasKey( Message.Data ) )
      {
        if ( WorkBench->GetEditedModelFilter()->Texture != TextureListMap[ Message.Data ] )
        {
          WorkBench->GetEditedModelFilter()->ParentObject->RemoveParent( WorkBench->GetEditedModelFilter()->Texture );
          WorkBench->GetEditedModelFilter()->Texture = TextureListMap[ Message.Data ];
          WorkBench->GetEditedModelFilter()->ParentObject->AddParent( TextureListMap[ Message.Data ] );
          WorkBench->GetEditedModelFilter()->ParentObject->InvalidateUptoDateFlag();
        }
      }
      return true;
    }

    if ( WorkBench->GetEditedModelObject() && UIParameterMap.HasKey( b->GetID() ) && ( ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Loft && b->GetID() == _T( "loftpathlist" ) || b->GetID() == _T( "loftslicelist" ) ) ||
      ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Scatter && b->GetID() == _T( "scatterobj" ) || b->GetID() == _T( "scattershape" ) ) ||
                                                               ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Copy && b->GetID() == _T( "copyparentlist" ) ) ) )
    {
      WorkBench->GetEditedModelObject()->RemoveParents( PHX_MODELOBJECT );
      TBOOL Changed = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] != Message.Data;

      WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] = Message.Data;

      WorkBench->GetEditedModelObject()->ParentGUIDS[ UIParameterMap[ b->GetID() ] ] = WorkBench->GetEditedModelObject()->GetParentModel()->GetObjectByIndex( Message.Data )->GetGUID();

      if ( WorkBench->GetEditedModelObject()->GetParameters()[ 0 ] < WorkBench->GetEditedModelObject()->GetParentModel()->GetObjectCount() )
        WorkBench->GetEditedModelObject()->AddParent( WorkBench->GetEditedModelObject()->GetParentModel()->GetObjectByIndex( WorkBench->GetEditedModelObject()->GetParameters()[ 0 ] ) );
      if ( WorkBench->GetEditedModelObject()->GetParameters()[ 1 ] < WorkBench->GetEditedModelObject()->GetParentModel()->GetObjectCount() )
        WorkBench->GetEditedModelObject()->AddParent( WorkBench->GetEditedModelObject()->GetParentModel()->GetObjectByIndex( WorkBench->GetEditedModelObject()->GetParameters()[ 1 ] ) );


      if ( Changed ) WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
      return true;
    }

    if ( WorkBench->GetEditedModelObject() && UIParameterMap.HasKey( b->GetID() ) && ( ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Tree && b->GetID() == _T( "treespecies" ) ) ||
      ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_TreeLeaves && b->GetID() == _T( "treeleafspecies" ) ) ) )
    {
      WorkBench->GetEditedModelObject()->RemoveParents( PHX_TREESPECIES );
      TBOOL Changed = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] != Message.Data;

      WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] = Message.Data;
      WorkBench->GetEditedModelObject()->ParentGUIDS[ UIParameterMap[ b->GetID() ] ] = Project.GetTreeSpeciesByIndex( Message.Data )->GetGUID();
      WorkBench->GetEditedModelObject()->AddParent( Project.GetTreeSpeciesByIndex( Message.Data ) );

      if ( Changed ) WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
      return true;
    }

    if ( WorkBench->GetEditedModelObject() && UIParameterMap.HasKey( b->GetID() ) )
    {
      if ( b->GetCursorPosition() != WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] )
      {
        WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ b->GetID() ] ] = b->GetCursorPosition();
        WorkBench->GetEditedModelObject()->InvalidateUptoDateFlag();
      }
      return true;
    }

    break;
  }

  default:
    break;
  }

  return CapexWindow::MessageProc( Message );
}

void CapexModelParameters::SetTrackBarValue( TCHAR *Name )
{
  CWBTrackBar *v = (CWBTrackBar*)FindChildByID( Name, _T( "trackbar" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  int mask = 0xffffffff;
  if ( UIParameterFilterMap.HasKey( Name ) )
    mask = UIParameterFilterMap[ Name ];

  v->SetValue( WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ] & mask );
}

void CapexModelParameters::SetTrackBarValue_Filter( TCHAR *Name )
{
  CWBTrackBar *v = (CWBTrackBar*)FindChildByID( Name, _T( "trackbar" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  int mask = 0xffffffff;
  if ( UIParameterFilterMap.HasKey( Name ) )
    mask = UIParameterFilterMap[ Name ];

  v->SetValue( WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ Name ] ] & mask );
}

void CapexModelParameters::SetButtonValue( TCHAR *Name )
{
  CWBButton *v = (CWBButton*)FindChildByID( Name, _T( "button" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  int mask = 0xffffffff;
  if ( UIParameterFilterMap.HasKey( Name ) )
    mask = UIParameterFilterMap[ Name ];

  v->Push( ( WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ] & mask ) != 0 );
}

void CapexModelParameters::SetTextData( TCHAR *Name )
{
  CWBTextBox *v = (CWBTextBox*)FindChildByID( Name, _T( "textbox" ) );
  if ( !v ) return;

  if ( !WorkBench->GetEditedModelObject() ) return;

  if ( WorkBench->GetEditedModelObject()->GetPrimitive() == Mesh_Clone ) return;
  CphxModelObject_Tool_Mesh *m = (CphxModelObject_Tool_Mesh*)WorkBench->GetEditedModelObject();

  v->SetText( m->GetText() );
}

void CapexModelParameters::SetButtonValue_Filter( TCHAR *Name )
{
  CWBButton *v = (CWBButton*)FindChildByID( Name, _T( "button" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  int mask = 0xffffffff;
  if ( UIParameterFilterMap.HasKey( Name ) )
    mask = UIParameterFilterMap[ Name ];

  v->Push( ( WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ Name ] ] & mask ) != 0 );
}

void CapexModelParameters::SetButtonTextureValue_Filter( TCHAR *Name )
{
  CWBButton *v = (CWBButton*)FindChildByID( Name, _T( "button" ) );
  if ( !v ) return;

  //if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  v->SetText( WorkBench->GetEditedModelFilter()->Texture ? WorkBench->GetEditedModelFilter()->Texture->GetName() : "Select Texture" );
}

void CapexModelParameters::SetlistSelection( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  v->SelectItemByIndex( WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ] );
}

void CapexModelParameters::SetlistSelection_Filter( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  v->SelectItemByIndex( WorkBench->GetEditedModelFilter()->Parameters[ UIParameterMap[ Name ] ] );
}

void CapexModelParameters::BuildArcList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  TS32 SelectedArc = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ];
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    PHXMESHPRIMITIVE p = m->GetObjectByIndex( x )->GetPrimitive();
    if ( p == Mesh_Arc || p == Mesh_Line || p == Mesh_Spline )
    {
      TS32 id = v->AddItem( m->GetObjectByIndex( x )->GetName() );
      v->GetItem( id )->SetID( x );
      if ( x == SelectedArc ) v->SelectItem( x );
    }
  }
}

void CapexModelParameters::BuildTreeSpeciesList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  TS32 SelectedSpecies = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ];

  for ( TS32 x = 0; x < Project.GetTreeSpeciesCount(); x++ )
  {
    TS32 id = v->AddItem( Project.GetTreeSpeciesByIndex( x )->GetName() );
    v->GetItem( id )->SetID( x );
    if ( Project.GetTreeSpeciesByIndex( x )->GetGUID() == WorkBench->GetEditedModelObject()->ParentGUIDS[ 0 ] )
      v->SelectItem( x );
  }
}

void CapexModelParameters::BuildMeshList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  TS32 SelectedMesh = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ];
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    PHXMESHPRIMITIVE p = m->GetObjectByIndex( x )->GetPrimitive();
    if ( m->GetObjectByIndex( x ) != WorkBench->GetEditedModelObject() && p != Mesh_Clone )
    {
      TS32 id = v->AddItem( m->GetObjectByIndex( x )->GetName() );
      v->GetItem( id )->SetID( x );
      if ( x == SelectedMesh ) v->SelectItem( x );
    }
  }
}

void CapexModelParameters::BuildCSGList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    PHXMESHPRIMITIVE p = m->GetObjectByIndex( x )->GetPrimitive();
    if ( m->GetObjectByIndex( x ) != WorkBench->GetEditedModelFilter()->ParentObject )
      if ( p != Mesh_Arc && p != Mesh_Line && p != Mesh_Spline && p != Mesh_Clone )
      {
        TS32 id = v->AddItem( m->GetObjectByIndex( x )->GetName() );
        v->GetItem( id )->SetID( x );
        if ( m->GetObjectByIndex( x )->GetGUID() == WorkBench->GetEditedModelFilter()->ReferencedGUID )
          v->SelectItem( x );
      }
  }
}

void CapexModelParameters::BuildTintTextureSelectorUI( CWBButton* Button )
{
  CWBContextMenu *c = Button->OpenContextMenu( Button->ClientToScreen( Button->GetClientRect().BottomLeft() ) );
  TextureIDMap.Flush();

  TS32 id = 0;

  c->AddItem( _T( "No Texture" ), id );
  CphxGUID g;
  g.SetString( _T( "NONENONENONENONENONENONENONENONE" ) );

  TextureIDMap[ id ] = g;
  id++;

  c->AddSeparator();

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    CapexTexGenPage *p = Project.GetTexgenPageByIndex( x );

    TS32 savecnt = 0;
    for ( TS32 y = 0; y < p->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *t = p->GetOp( y );
      if ( t->GetOpType() == TEXGEN_OP_SAVE ) savecnt++;
    }

    if ( savecnt )
    {
      CWBContextItem *it = c->AddItem( p->GetName().GetPointer(), id++ );

      for ( TS32 y = 0; y < p->GetOpCount(); y++ )
      {
        CphxTextureOperator_Tool *t = p->GetOp( y );
        if ( t->GetOpType() == TEXGEN_OP_SAVE )
        {
          it->AddItem( t->GetName().GetPointer(), id );
          TextureIDMap[ id ] = t->GetGUID();
          id++;
        }
      }
    }
  }
}

void CapexModelParameters::InitTrackBarMap()
{
  TrackBarTextMap[ _T( "planex" ) ] = _T( "X Resolution: %d" );
  TrackBarTextMap[ _T( "planey" ) ] = _T( "Y Resolution: %d" );
  TrackBarTextMap[ _T( "spherex" ) ] = _T( "X Resolution: %d" );
  TrackBarTextMap[ _T( "spherey" ) ] = _T( "Y Resolution: %d" );
  TrackBarTextMap[ _T( "spheretop" ) ] = _T( "Top Position: %d" );
  TrackBarTextMap[ _T( "spherebottom" ) ] = _T( "Bottom Position: %d" );
  TrackBarTextMap[ _T( "georesolution" ) ] = _T( "Iterations: %d" );
  TrackBarTextMap[ _T( "cylinderx" ) ] = _T( "X Resolution: %d" );
  TrackBarTextMap[ _T( "cylindery" ) ] = _T( "Y Resolution: %d" );
  TrackBarTextMap[ _T( "conex" ) ] = _T( "X Resolution: %d" );
  TrackBarTextMap[ _T( "coney" ) ] = _T( "Y Resolution: %d" );
  TrackBarTextMap[ _T( "conetop" ) ] = _T( "Top Position: %d" );
  TrackBarTextMap[ _T( "arcres" ) ] = _T( "Resolution: %d" );
  TrackBarTextMap[ _T( "arcbend" ) ] = _T( "Bend: %d" );
  TrackBarTextMap[ _T( "lineres" ) ] = _T( "Resolution: %d" );
  TrackBarTextMap[ _T( "splineres" ) ] = _T( "Resolution: %d" );

  TrackBarTextMap[ _T( "loftrotation" ) ] = _T( "Rotation: %d" );
  TrackBarTextMap[ _T( "loftstartscale" ) ] = _T( "Start Scale: %d" );
  TrackBarTextMap[ _T( "loftendscale" ) ] = _T( "End Scale: %d" );
  TrackBarTextMap[ _T( "scatterseed" ) ] = _T( "RandSeed: %d" );
  TrackBarTextMap[ _T( "scattervxamout" ) ] = _T( "Vertex Amount: %d" );
  TrackBarTextMap[ _T( "scatteredgeamount" ) ] = _T( "Edge Amount: %d" );
  TrackBarTextMap[ _T( "scatterpolyamount" ) ] = _T( "Poly Amount: %d" );
  TrackBarTextMap[ _T( "scattermaxperpoly" ) ] = _T( "Max Per Poly: %d" );
  TrackBarTextMap[ _T( "scatteramountchannel" ) ] = _T( "Amount Channel: %d" );
  TrackBarTextMap[ _T( "scatterscalethresh" ) ] = _T( "Scale Threshold: %d" );
  TrackBarTextMap[ _T( "scatterxformchannel" ) ] = _T( "XForm Tint Channel: %d" );

  TrackBarTextMap[ _T( "uvchannel" ) ] = _T( "UV Channel: %d" );
  TrackBarTextMap[ _T( "tintsaturation" ) ] = _T( "Saturation: %d" );
  TrackBarTextMap[ _T( "tintchannel" ) ] = _T( "UV Channel: %d" );
  TrackBarTextMap[ _T( "meshsmoothiterations" ) ] = _T( "Iterations: %d" );
  TrackBarTextMap[ _T( "meshsmooththreshold" ) ] = _T( "Tint Threshold: %d" );
  TrackBarTextMap[ _T( "smoothgroupvalue" ) ] = _T( "Smooth Break: %d" );
  TrackBarTextMap[ _T( "bevelpercent" ) ] = _T( "Percentage: %d" );
  TrackBarTextMap[ _T( "replicatecount" ) ] = _T( "Copies: %d" );
  TrackBarTextMap[ _T( "textdeviation" ) ] = _T( "Roughness: %d" );

  TrackBarTextMap[ _T( "treerandseed" ) ] = _T( "RandSeed: %d" );
  TrackBarTextMap[ _T( "leafrandseed" ) ] = _T( "RandSeed: %d" );

  TrackBarTextMap[ _T( "treedensity1" ) ] = _T( "Level 1 density: %d" );
  TrackBarTextMap[ _T( "treedensity2" ) ] = _T( "Level 2 density: %d" );
  TrackBarTextMap[ _T( "treedensity3" ) ] = _T( "Level 3 density: %d" );
  TrackBarTextMap[ _T( "treedensity4" ) ] = _T( "Level 4 density: %d" );
  TrackBarTextMap[ _T( "treedensity5" ) ] = _T( "Level 5 density: %d" );

  TrackBarTextMap[ _T( "normaldeformfraction" ) ] = _T( "Power Fraction: %d" );
  TrackBarTextMap[ _T( "normaldeforminteger" ) ] = _T( "Power Integer: %d" );

  TrackBarTextMap[ _T( "tintshapepower" ) ] = _T( "Power: %d" );

  TrackBarTextMap[ _T( "greebleseed" ) ] = _T( "RandSeed: %d" );
  TrackBarTextMap[ _T( "greebleextrude" ) ] = _T( "Extrusion: %d" );
  TrackBarTextMap[ _T( "greebletaper" ) ] = _T( "Taper: %d" );

  UIParameterMap[ _T( "planex" ) ] = 0;
  UIParameterMap[ _T( "planey" ) ] = 1;
  UIParameterMap[ _T( "spherex" ) ] = 0;
  UIParameterMap[ _T( "spherey" ) ] = 1;
  UIParameterMap[ _T( "spheretop" ) ] = 2;
  UIParameterMap[ _T( "spherebottom" ) ] = 3;
  UIParameterMap[ _T( "spherecap" ) ] = 4;
  UIParameterMap[ _T( "georesolution" ) ] = 0;
  UIParameterMap[ _T( "cylinderx" ) ] = 0;
  UIParameterMap[ _T( "cylindery" ) ] = 1;
  UIParameterMap[ _T( "cylindercap" ) ] = 2;
  UIParameterMap[ _T( "conex" ) ] = 0;
  UIParameterMap[ _T( "coney" ) ] = 1;
  UIParameterMap[ _T( "conetop" ) ] = 2;
  UIParameterMap[ _T( "conecap" ) ] = 3;
  UIParameterMap[ _T( "arcres" ) ] = 0;
  UIParameterMap[ _T( "arcbend" ) ] = 1;
  UIParameterMap[ _T( "arcclosed" ) ] = 2;
  UIParameterMap[ _T( "lineres" ) ] = 0;
  UIParameterMap[ _T( "splineres" ) ] = 0;
  UIParameterMap[ _T( "splineclosed" ) ] = 1;
  UIParameterMap[ _T( "loftpathlist" ) ] = 0;
  UIParameterMap[ _T( "loftslicelist" ) ] = 1;
  UIParameterMap[ _T( "loftpathclose" ) ] = 2;
  UIParameterMap[ _T( "loftsliceclose" ) ] = 3;
  UIParameterMap[ _T( "loftrotation" ) ] = 4;
  UIParameterMap[ _T( "loftstartscale" ) ] = 5;
  UIParameterMap[ _T( "loftendscale" ) ] = 6;
  UIParameterMap[ _T( "scatterobj" ) ] = 0;
  UIParameterMap[ _T( "scattershape" ) ] = 1;
  UIParameterMap[ _T( "scatterseed" ) ] = 2;
  UIParameterMap[ _T( "scattervxamout" ) ] = 3;
  UIParameterMap[ _T( "scatteredgeamount" ) ] = 4;
  UIParameterMap[ _T( "scatterpolyamount" ) ] = 5;
  UIParameterMap[ _T( "scattermaxperpoly" ) ] = 6;
  UIParameterMap[ _T( "scatteramountchannel" ) ] = 7;
  UIParameterMap[ _T( "scatterorientation" ) ] = 8;
  UIParameterMap[ _T( "scatterscalethresh" ) ] = 9;
  UIParameterMap[ _T( "scatteronlyyscale" ) ] = 10;
  UIParameterMap[ _T( "scatterxformchannel" ) ] = 11;
  UIParameterMap[ _T( "copyparentlist" ) ] = 0;
  UIParameterMap[ _T( "uvtype" ) ] = 0;
  UIParameterMap[ _T( "uvchannel" ) ] = 1;
  UIParameterFilterMap[ _T( "uvchannel" ) ] = 0x0f;
  UIParameterMap[ _T( "clipuv" ) ] = 1;
  UIParameterFilterMap[ _T( "clipuv" ) ] = 1 << 4;
  UIParameterMap[ _T( "tintsaturation" ) ] = 1;
  UIParameterMap[ _T( "tintchannel" ) ] = 0;
  UIParameterMap[ _T( "meshsmoothlinear" ) ] = 0;
  UIParameterMap[ _T( "meshsmoothiterations" ) ] = 1;
  UIParameterMap[ _T( "meshsmooththreshold" ) ] = 2;
  UIParameterMap[ _T( "smoothgroupvalue" ) ] = 0;
  UIParameterMap[ _T( "bevelpercent" ) ] = 0;
  UIParameterMap[ _T( "fontlist" ) ] = 0;
  UIParameterMap[ _T( "replicatecount" ) ] = 0;
  UIParameterMap[ _T( "textdeviation" ) ] = 1;

  UIParameterMap[ _T( "treespecies" ) ] = 0;
  UIParameterMap[ _T( "treeleafspecies" ) ] = 0;
  UIParameterMap[ _T( "treerandseed" ) ] = 1;
  UIParameterMap[ _T( "leafrandseed" ) ] = 1;
  UIParameterMap[ _T( "treedensity1" ) ] = 2;
  UIParameterMap[ _T( "treedensity2" ) ] = 3;
  UIParameterMap[ _T( "treedensity3" ) ] = 4;
  UIParameterMap[ _T( "treedensity4" ) ] = 5;
  UIParameterMap[ _T( "treedensity5" ) ] = 6;

  UIParameterMap[ _T( "uvscalex" ) ] = 0;
  UIParameterMap[ _T( "uvscaley" ) ] = 1;
  UIParameterMap[ _T( "uvscalez" ) ] = 2;
  UIParameterMap[ _T( "uvoffsetx" ) ] = 7;
  UIParameterMap[ _T( "uvoffsety" ) ] = 8;
  UIParameterMap[ _T( "uvoffsetz" ) ] = 9;

  UIParameterMap[ _T( "normaldeformfraction" ) ] = 0;
  UIParameterMap[ _T( "normaldeforminteger" ) ] = 1;

  UIParameterMap[ _T( "tintshapetype" ) ] = 0;
  UIParameterMap[ _T( "tintshapeoperator" ) ] = 1;
  UIParameterMap[ _T( "tintshapepower" ) ] = 2;

  UIParameterMap[ _T( "csgobjlist" ) ] = 0;
  UIParameterMap[ _T( "csgmode" ) ] = 1;

  UIParameterMap[ _T( "greebleseed" ) ] = 0;
  UIParameterMap[ _T( "greebleextrude" ) ] = 1;
  UIParameterMap[ _T( "greebletaper" ) ] = 2;
}

void CapexModelParameters::BuildCloneSourceList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Clone ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  v->Flush();
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    CphxModelObject_Tool *t = m->GetObjectByIndex( x );
    if ( WorkBench->GetEditedModelObject()->FindInCloneTree( t ) ) continue;
    TS32 i = v->AddItem( t->GetName() );
    v->GetItem( i )->SetID( x );
  }
}

void CapexModelParameters::BuildCloneContentList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Clone ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  CphxModelObject_Tool_Clone *c = (CphxModelObject_Tool_Clone *)WorkBench->GetEditedModelObject();

  v->Flush();
  for ( TS32 x = 0; x < c->ClonedObjects.NumItems(); x++ )
  {
    CphxModelObject_Tool *t = c->ClonedObjects[ x ];
    TS32 i = v->AddItem( t->GetName() );
    v->GetItem( i )->SetID( x );
  }
}

void CapexModelParameters::BuildMergeSourceList( TCHAR* Name )
{
  CWBItemSelector* v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Merge ) return;

  CphxModel_Tool* m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  v->Flush();
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
  {
    CphxModelObject_Tool* t = m->GetObjectByIndex( x );
    if ( t->GetPrimitive() == Mesh_Clone || t->GetPrimitive() == Mesh_Merge )
      continue;
    if ( WorkBench->GetEditedModelObject()->FindInCloneTree( t ) ) continue;
    TS32 i = v->AddItem( t->GetName() );
    v->GetItem( i )->SetID( x );
  }
}

void CapexModelParameters::BuildMergeContentList( TCHAR* Name )
{
  CWBItemSelector* v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Merge ) return;

  CphxModel_Tool* m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  CphxModelObject_Tool_Clone* c = (CphxModelObject_Tool_Clone*)WorkBench->GetEditedModelObject();

  v->Flush();
  for ( TS32 x = 0; x < c->ClonedObjects.NumItems(); x++ )
  {
    CphxModelObject_Tool* t = c->ClonedObjects[ x ];
    TS32 i = v->AddItem( t->GetName() );
    v->GetItem( i )->SetID( x );
  }
}

void CapexModelParameters::CloneAdd()
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( _T( "clonesourcelist" ), _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Clone ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  CWBSelectableItem *i = v->GetCursorItem();
  if ( !i ) return;

  CphxModelObject_Tool_Clone *c = (CphxModelObject_Tool_Clone *)WorkBench->GetEditedModelObject();
  c->AddClonedObject( m->GetObjectByIndex( i->GetID() ) );

  TS32 cursor = v->GetCursorPosition();

  BuildCloneSourceList( _T( "clonesourcelist" ) );
  BuildCloneContentList( _T( "clonecontentlist" ) );

  v = (CWBItemSelector*)FindChildByID( _T( "clonesourcelist" ), _T( "itemselector" ) );
  if ( v ) v->SelectItemByIndex( cursor );
}

void CapexModelParameters::CloneRemove()
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( _T( "clonecontentlist" ), _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Clone ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  CWBSelectableItem *i = v->GetCursorItem();
  if ( !i ) return;

  CphxModelObject_Tool_Clone *c = (CphxModelObject_Tool_Clone *)WorkBench->GetEditedModelObject();
  c->RemoveClonedObject( c->ClonedObjects[ i->GetID() ] );

  TS32 cursor = v->GetCursorPosition();
  BuildCloneSourceList( _T( "clonesourcelist" ) );
  BuildCloneContentList( _T( "clonecontentlist" ) );

  v = (CWBItemSelector*)FindChildByID( _T( "clonecontentlist" ), _T( "itemselector" ) );
  if ( v ) v->SelectItemByIndex( cursor );
}

void CapexModelParameters::MergeAdd()
{
  CWBItemSelector* v = (CWBItemSelector*)FindChildByID( _T( "mergesourcelist" ), _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Merge ) return;

  CphxModel_Tool* m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  CWBSelectableItem* i = v->GetCursorItem();
  if ( !i ) return;

  CphxModelObject_Tool* c = (CphxModelObject_Tool*)WorkBench->GetEditedModelObject();
  c->AddClonedObject( m->GetObjectByIndex( i->GetID() ) );

  TS32 cursor = v->GetCursorPosition();

  BuildMergeSourceList( _T( "mergesourcelist" ) );
  BuildMergeContentList( _T( "mergecontentlist" ) );

  v = (CWBItemSelector*)FindChildByID( _T( "mergesourcelist" ), _T( "itemselector" ) );
  if ( v ) v->SelectItemByIndex( cursor );
  c->InvalidateUptoDateFlag();
}

void CapexModelParameters::MergeRemove()
{
  CWBItemSelector* v = (CWBItemSelector*)FindChildByID( _T( "mergecontentlist" ), _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;
  if ( WorkBench->GetEditedModelObject()->GetPrimitive() != Mesh_Merge ) return;

  CphxModel_Tool* m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  CWBSelectableItem* i = v->GetCursorItem();
  if ( !i ) return;

  CphxModelObject_Tool* c = (CphxModelObject_Tool*)WorkBench->GetEditedModelObject();
  c->RemoveClonedObject( c->ClonedObjects[ i->GetID() ] );

  TS32 cursor = v->GetCursorPosition();
  BuildMergeSourceList( _T( "mergesourcelist" ) );
  BuildMergeContentList( _T( "mergecontentlist" ) );

  v = (CWBItemSelector*)FindChildByID( _T( "mergecontentlist" ), _T( "itemselector" ) );
  if ( v ) v->SelectItemByIndex( cursor );
  c->InvalidateUptoDateFlag();
}

void CapexModelParameters::BuildTextureList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  v->Flush();
  TextureListMap.Flush();

  for ( TS32 x = 0; x < Project.GetTexgenPageCount(); x++ )
  {
    CapexTexGenPage *p = Project.GetTexgenPageByIndex( x );
    for ( TS32 y = 0; y < p->GetOpCount(); y++ )
    {
      CphxTextureOperator_Tool *t = p->GetOp( y );
      if ( t->GetOpType() == TEXGEN_OP_SAVE )
      {
        TS32 i = v->AddItem( t->GetName() );
        if ( WorkBench->GetEditedModelFilter()->Texture == t ) v->SelectItem( i );
        TextureListMap[ i ] = t;
      }
    }
  }
}

void CapexModelParameters::BuildCopyList( TCHAR *Name )
{
  CWBItemSelector *v = (CWBItemSelector*)FindChildByID( Name, _T( "itemselector" ) );
  if ( !v ) return;
  if ( !WorkBench->GetEditedModelObject() ) return;

  v->Flush();
  CphxModel_Tool *m = GetWorkBench()->GetEditedModel();
  if ( !m ) return;

  TS32 Selected = WorkBench->GetEditedModelObject()->GetParameters()[ UIParameterMap[ Name ] ];
  for ( TS32 x = 0; x < m->GetObjectCount(); x++ )
    if ( m->GetObjectByIndex( x ) != WorkBench->GetEditedModelObject() && m->GetObjectByIndex( x )->GetPrimitive() != Mesh_Clone )
    {
      TS32 i = v->AddItem( m->GetObjectByIndex( x )->GetName() );
      v->GetItem( i )->SetID( x );
      if ( x == Selected ) v->SelectItem( x );
    }

}

void CapexModelParameters::SetSrtInputValue( TCHAR *Name )
{
  CWBTextBox *v = (CWBTextBox*)FindChildByID( Name, _T( "textbox" ) );
  if ( !v ) return;

  if ( !UIParameterMap.HasKey( Name ) ) return;
  if ( !WorkBench->GetEditedModelFilter() ) return;

  TF32 add = 0;
  if ( UIParameterMap[ Name ] < 3 ) add = 1;

  v->SetText( CString::Format( _T( "%f" ), (TF32)WorkBench->GetEditedModelFilter()->srt[ UIParameterMap[ Name ] ] + add ) );

  //v->SelectItemByIndex(EditedFilter->Parameters[UIParameterMap[Name]]);

}

