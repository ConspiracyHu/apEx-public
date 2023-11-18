#pragma once
#include "../../Bedrock/BaseLib/BaseLib.h"
#include "../Phoenix_Tool/Texgen_tool.h"

typedef int APEXPAGEID;

class CapexTexGenFilterParameter
{
public:

  CString Name;
  TS32 Type;
  TS32 Min, Max, Def;

  CArray<CString> ValueNames;

  CapexTexGenFilterParameter()
  {
    Name = "New Parameter";
    Type = 0;
    Min = 0;
    Max = 255;
    Def = 127;
  }
};

class CapexTexGenFilter
{
public:

  PHXTEXTUREFILTER FilterData;

  CString Name;
  TS32 Type;

  CArray<CapexTexGenFilterParameter> Parameters;

  CapexTexGenFilter()
  {
    Name = "New Filter";
    Type = 0;
    FilterData.DataDescriptor.PassCount = 1;
    FilterData.DataDescriptor.InputCount = 0;
    FilterData.DataDescriptor.NeedsRandSeed = 0;
    FilterData.DataDescriptor.ParameterCount = 0;
  }

};

class CapexTexGenPage
{
  CString Name;
  TS32 BaseXRes;
  TS32 BaseYRes;
  TBOOL hdr;

  APEXPAGEID ID;

  CArray<CphxTextureOperator_Tool*> Operators;

  void ConnectOperators( CphxTextureOperator_Tool *opx, CphxTextureOperator_Tool *opy, TS32 &inputcnt );
  void UpdateResolution();

public:

  TBOOL NeedsExport;

  CapexTexGenPage( APEXPAGEID ID );
  virtual ~CapexTexGenPage();

  APEXPAGEID GetID() { return ID; }
  const CString &GetName() { return Name; }

  TS32 GetOpCount() { return Operators.NumItems(); }
  CphxTextureOperator_Tool *GetOp( TS32 x ) { if ( x < 0 || x >= GetOpCount() ) return NULL; return Operators[ x ]; }
  void StoreOpPositions() { for ( TS32 x = 0; x < Operators.NumItems(); x++ ) Operators[ x ]->OldPosition = Operators[ x ]->Position; }
  void RestoreOpPositions() { for ( TS32 x = 0; x < Operators.NumItems(); x++ ) Operators[ x ]->Position = Operators[ x ]->OldPosition; }
  void ClearOpSelection() { for ( TS32 x = 0; x < Operators.NumItems(); x++ ) Operators[ x ]->Selected = false; }
  void DeleteSelected();
  CphxTextureOperator_Tool *CreateOperator( CphxTextureFilter_Tool *Filter );
  CphxTextureOperator_Tool *CreateNOPOperator();
  CphxTextureOperator_Tool *CreateLoadOperator();
  CphxTextureOperator_Tool *CreateSaveOperator();
  CphxTextureOperator_Tool *CreateSubroutineOperator();
  CphxTextureOperator_Tool *CreateSubroutineCall( CphxTextureOperator_Subroutine *Sub );

  void BuildOperatorConnections();

  void Export( CXMLNode *Node );
  void Import( CXMLNode *Node );

  void AddOperator( CphxTextureOperator_Tool *Op ) { Operators += Op; }
  void SetName( CString &n ) { Name = n; }
  void SetBaseXRes( TS32 v );
  void SetBaseYRes( TS32 v );
  TS32 GetBaseXRes() { return BaseXRes; }
  TS32 GetBaseYRes() { return BaseYRes; }

  void SortOpsByFilter();
  TBOOL IsHDR() { return hdr; }
  void ToggleHDR();

};

void DeinitTexgen();
