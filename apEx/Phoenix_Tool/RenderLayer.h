#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../Phoenix/RenderLayer.h"
#include "phxResource.h"
#include "RenderTargets.h"

class CphxRenderLayerDescriptor_Tool : public CphxResource
{

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

public:

  CString Name;
  CArray<CphxRenderTarget_Tool*> RenderTargets;
  CphxRenderLayerDescriptor RenderLayer;
  TBOOL External;
  TBOOL HasPicking = false;

  CphxRenderLayerDescriptor_Tool();
  virtual ~CphxRenderLayerDescriptor_Tool();

  TBOOL GenerateResource( CCoreDevice *Dev ) { return true; }
  virtual PHXRESOURCETYPE GetType() { return PHX_RENDERLAYERDESCRIPTOR; }

  void AddRenderTarget( CphxRenderTarget_Tool *rt );

};