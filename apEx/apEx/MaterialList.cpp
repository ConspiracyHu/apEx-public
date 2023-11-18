#include "BasePCH.h"
#include "MaterialList.h"
#define WINDOWNAME _T("Material List")
#define WINDOWXML _T("MaterialList")

CapexMaterialList::CapexMaterialList() : CapexWindow()
{
}

CapexMaterialList::CapexMaterialList( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexMaterialList::~CapexMaterialList()
{

}

void CapexMaterialList::UpdateData()
{

}
