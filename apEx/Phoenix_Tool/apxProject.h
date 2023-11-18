#pragma once
#include "RenderTargets.h"
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "TexgenPage.h"
#include "Texgen_tool.h"
#include "Material_Tool.h"
#include "Audio/phxSound.h"
#include "Timeline_tool.h"
#include "Model_Tool.h"
#include "Material_Tool.h"
#include "RenderLayer.h"
#include "Scene_tool.h"
#include "TreeSpecies.h"

class CapexRoot;

struct MUSICDATA
{
  CArray<TU32> TickData;

  HWND AppHandle; // = NULL;
  TF32 MVXRenderStatus; // = 0;

  TBOOL Playing; // = false;
  TS32 PlaybackStart; // = 0;

  TS32 CurrentFrame; // = 0;
  TBOOL TimelineDragged; // = false;

  TS32 OfflineLength; // = 0;

};

class CCoreDX11Texture2D;

class CapexRenderTargetCollection
{
  friend class CapexProject;

  TS32 XRes, YRes;
  TBOOL LastAspectCorrect;

  CCoreDX11Texture2D *DepthBuffer;

  CCoreDevice *Dev;

public:
  CCoreDX11Texture2D *Internal = NULL;
  CArray<CCoreDX11Texture2D*> RenderTargets;

  CapexRenderTargetCollection( CCoreDevice *Device );
  virtual ~CapexRenderTargetCollection();

  void UpdateCollection( TS32 xres, TS32 yres, TBOOL AspectCorrect );
  void Apply();
  void ReallocateCollection();

  CCoreDX11Texture2D *GetRenderTarget( CphxRenderTarget *rt );
};

class CapexProject
{
  CArray<CapexTexGenPage*> TexgenPages;
  CArray<CphxTextureFilter_Tool*> Filters;
  CArray<CphxMaterialTechnique_Tool*> Techniques;
  CphxRenderTarget_Tool InternalRenderTarget;
  CArray<CphxRenderTarget_Tool*> RenderTargets;
  CArray<CphxMaterial_Tool*> Materials;
  CArray<CphxModel_Tool*> Models;
  CArray<CphxRenderLayerDescriptor_Tool*> RenderLayers;
  CArray<CphxScene_Tool*> Scenes;
  CArray<CphxTreeSpecies*> TreeSpecies;

  void MakeTextureOperatorGUIDConnections();
  TS32 LastSaveTime;
  TS32 LastClickedTime = 0;

  CphxSound *Song;

  CArray<CapexRenderTargetCollection*> SpawnedCollections;

  void ImportMain( CXMLDocument &Document, bool reset = true );
  void ImportMaterialMain( CXMLDocument &Document );

public:
  void UpdateRTCollections();

  MUSICDATA MusicData;

  TS32 DemoResolutionX;
  TS32 DemoResolutionY;

  CString LoadedFileName;

  CString Group;
  CString Title;
  CString Urls[ 5 ];

  CapexProject();
  virtual ~CapexProject();

  void Reset();
  void Import( CString &FileName, HWND hwnd );
  void Export( CString &FileName, TBOOL BackupExisting, TBOOL saveClean );
  CString ExportToString( TBOOL saveClean );
  void ImportLibraries();

  void ImportMaterial( CString &FileName, HWND hwnd );
  void MergeProject( CString &FileName, HWND hwnd );

  //////////////////////////////////////////////////////////////////////////
  // texture filters

  CphxTextureFilter_Tool *CreateTextureFilter();
  TBOOL DeleteTextureFilter( CphxGUID &GUID );
  CphxTextureFilter_Tool *GetTextureFilter( CphxGUID &GUID );
  TS32 GetTextureFilterIndex( CphxGUID &GUID );
  TS32 GetTextureFilterCount() { return Filters.NumItems(); }
  CphxTextureFilter_Tool *GetTextureFilterByIndex( TS32 x ) { if ( x < 0 || x >= Filters.NumItems() ) return NULL; return Filters[ x ]; }

  void ExportTextureFilters( CXMLNode *Root );
  void ImportTextureFilters( CXMLNode *Root, TBOOL External );

  void KillTextureLoops();

  void FreeTextureMem( TS32 TextureOpCount );

  //////////////////////////////////////////////////////////////////////////
  // materials

  void ExportMaterials( CXMLNode *Root );
  void ImportMaterials( CXMLNode *Root, TBOOL External );
  CphxMaterialTechnique_Tool *CreateTech();
  TS32 GetTechCount() { return Techniques.NumItems(); }
  CphxMaterialTechnique_Tool *GetTechByIndex( TS32 x ) { if ( x < 0 || x >= Techniques.NumItems() ) return NULL; return Techniques[ x ]; }
  //CphxMaterialTechnique_Tool *GetTech(CphxGUID &ID);
  CphxRenderTarget_Tool *CreateRenderTarget();
  TS32 GetRenderTargetCount() { return RenderTargets.NumItems(); }
  CphxRenderTarget_Tool *GetRenderTargetByIndex( TS32 x ) { if ( x < 0 || x >= RenderTargets.NumItems() ) return NULL; return RenderTargets[ x ]; }
  void DeleteRenderTarget( CphxRenderTarget_Tool *t );
  CphxRenderTarget_Tool *GetRenderTarget( CphxGUID &ID );
  TS32 GetRenderTargetIndex( CphxGUID &ID );
  CphxMaterialTechnique_Tool *GetTech( CphxGUID &ID );
  //ID3D11DepthStencilView *GetDepthBuffer();

  CphxMaterialParameter_Tool *GetMaterialParameter( CphxGUID &ID );

  CphxMaterial_Tool *CreateMaterial();
  TBOOL DeleteMaterial( CphxGUID &GUID );
  CphxMaterial_Tool *GetMaterial( CphxGUID &GUID );
  TS32 GetMaterialCount() { return Materials.NumItems(); }
  CphxMaterial_Tool *GetMaterialByIndex( TS32 x ) { if ( x < 0 || x >= Materials.NumItems() ) return NULL; return Materials[ x ]; }


  CphxRenderLayerDescriptor_Tool *CreateRenderLayer();
  TBOOL DeleteRenderLayer( CphxGUID &GUID );
  CphxRenderLayerDescriptor_Tool *GetRenderLayer( CphxGUID &GUID );
  TS32 GetRenderLayerCount() { return RenderLayers.NumItems(); }
  CphxRenderLayerDescriptor_Tool *GetRenderLayerByIndex( TS32 x ) { if ( x < 0 || x >= RenderLayers.NumItems() ) return NULL; return RenderLayers[ x ]; }

  void ApplyRenderTargets( CapexRenderTargetCollection *Collection );
  //void ChangeDemoResolution(TS32 XRes, TS32 YRes, CCoreDevice *dev);
  //void ForceDemoResolution(TS32 XRes, TS32 YRes, CCoreDevice *dev);

  CapexRenderTargetCollection *SpawnRenderTargetCollection( CCoreDevice *Device, TS32 XRes, TS32 YRes, TBOOL AspectCorrect );
  void DeleteRenderTargetCollection( CapexRenderTargetCollection *rtc );

  //////////////////////////////////////////////////////////////////////////
  // texgen pages
  CapexTexGenPage *CreateTexgenPage();
  TBOOL DeleteTexgenPage( APEXPAGEID ID );
  CapexTexGenPage *GetTexgenPage( APEXPAGEID ID );
  TS32 GetTexgenPageCount() { return TexgenPages.NumItems(); }
  CapexTexGenPage *GetTexgenPageByIndex( TS32 x ) { if ( x < 0 || x >= TexgenPages.NumItems() ) return NULL; return TexgenPages[ x ]; }
  CphxTextureOperator_Tool *GetTexgenOp( APEXOPID ID );
  CphxTextureOperator_Tool *GetTexgenOp( CphxGUID &ID );

  ID3D11ShaderResourceView *GetTextureView( CphxGUID &Guid );

  //////////////////////////////////////////////////////////////////////////
  // models

  CphxModel_Tool *CreateModel();
  CphxModel_Tool *GetModel( CphxGUID &ID );
  TBOOL DeleteModel( CphxGUID &ID );
  TS32 GetModelCount() { return Models.NumItems(); }
  CphxModel_Tool *GetModelByIndex( TS32 x ) { if ( x < 0 || x >= Models.NumItems() ) return NULL; return Models[ x ]; }
  CphxModel_Tool * GetModelByName( CString Name );
  CphxModelObject_Tool *GetModelObject( CphxGUID &guid );

  //////////////////////////////////////////////////////////////////////////
  // scenes

  CphxScene_Tool *CreateScene();
  CphxScene_Tool *GetScene( CphxGUID &ID );
  CphxScene_Tool *GetSceneByName( CString Name );
  TS32 GetSceneIndex( CphxGUID &ID );
  TBOOL DeleteScene( CphxGUID &ID );
  TS32 GetSceneCount() { return Scenes.NumItems(); }
  CphxScene_Tool *GetSceneByIndex( TS32 x ) { if ( x < 0 || x >= Scenes.NumItems() ) return NULL; return Scenes[ x ]; }

  //////////////////////////////////////////////////////////////////////////
  // treespecies

  void ExportTreeSpecies( CXMLNode *Root );
  void ImportTreeSpecies( CXMLNode *Root, TBOOL External );

  CphxTreeSpecies *CreateTreeSpecies();
  CphxTreeSpecies *GetTreeSpecies( CphxGUID &ID );
  CphxTreeSpecies *GetTreeSpeciesByName( CString Name );
  TS32 GetTreeSpeciesIndex( CphxGUID &ID );
  TBOOL DeleteTreeSpecies( CphxGUID &ID );
  TS32 GetTreeSpeciesCount() { return TreeSpecies.NumItems(); }
  CphxTreeSpecies *GetTreeSpeciesByIndex( TS32 x ) { if ( x < 0 || x >= TreeSpecies.NumItems() ) return NULL; return TreeSpecies[ x ]; }

  //////////////////////////////////////////////////////////////////////////
  // timeline

  class CphxTimeline_Tool *Timeline;
  CString SongFile;

  TS32 LastPassToDraw = -1;

  TS32 GetFrameToRender();
  void LoadSong( CStreamReader &r, HWND AppHandle );
  TBOOL IsSongLoaded() { return Song != NULL; }
  CString GetSongType();
  CString GetSynthConfig();
  void TogglePlaying();
  void SeekToTime( TS32 timepos );
  void SeekToFrame( TS32 timepos );
  void StopPlayback();
  TS32 GetTimepos();
  void UpdateCurrentFrameFromPlayback();
  void AcquireTickData();
  TBOOL CanLoadSong();

  TS32 GetEventCount() { return Timeline->Events.NumItems(); }
  CphxEvent_Tool *GetEventByIndex( TS32 x ) { if ( x < 0 || x >= Timeline->Events.NumItems() ) return NULL; return Timeline->Events[ x ]; }
  CphxEvent_Tool *GetEvent( CphxGUID &ID );
  float GetAspect();

  //////////////////////////////////////////////////////////////////////////
  // helpers
  void DoAutosave( TS32 seconds = 300 );
  void DoBackupSave( CString &Filename );
  void DoCrashSave();
  void ResetParticles();

  CphxResource *GetResource( CphxGUID &g );

  TBOOL EnableShaderMinifier = true;
  TBOOL EnableGlobalShaderMinifier = false;
  TBOOL EnableFarbrauschPrecalc = false;
  TBOOL EnableMinimalPrecalc = false;
  TBOOL EnableSetupHasSocial = true;
};

extern CapexProject Project;