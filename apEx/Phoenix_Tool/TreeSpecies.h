#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "phxResource.h"
#include "../Phoenix/Arbaro.h"

class CphxTreeSpecies : public CphxResource
{
protected:

  virtual void ExportData( CXMLNode *Node );
  virtual void ImportData( CXMLNode *Node );

  virtual TBOOL GenerateResource( CCoreDevice *Dev );
  virtual PHXRESOURCETYPE GetType()
  {
    return PHX_TREESPECIES;
  }

public:

  CString Name;
  TBOOL External = false;

  CphxTreeSpecies();
  virtual ~CphxTreeSpecies();

  CString GetName();

  TREESPECIESDESCRIPTOR Tree;
};