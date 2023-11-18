#include "BasePCH.h"
#include "ColorPicker.h"
#define WINDOWNAME _T("Color Picker")
#define WINDOWXML _T("ColorPicker")

CapexColorPicker::CapexColorPicker() : CapexWindow()
{
}

CapexColorPicker::CapexColorPicker( CWBItem *Parent, const CRect &Pos, CapexWorkBench *WB ) : CapexWindow( Parent, Pos, WB, WINDOWNAME, WINDOWXML, WB_WINDOW_DEFAULT )
{
}

CapexColorPicker::~CapexColorPicker()
{

}
