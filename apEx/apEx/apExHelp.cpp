#include "BasePCH.h"
#include "apExHelp.h"
#define WINDOWNAME _T("Help")
#define WINDOWXML _T("Help")

CapexHelp::CapexHelp() : CapexWindow()
{
}

CapexHelp::CapexHelp( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexHelp::~CapexHelp()
{

}

void CapexHelp::UpdateData()
{

}

