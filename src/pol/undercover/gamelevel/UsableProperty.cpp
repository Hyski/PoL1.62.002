#include "precomp.h"
#include "UsableProperty.h"
#include <logic2/HexGrid.h>

//=====================================================================================//
//                          UsableProperty::UsableProperty()                           //
//=====================================================================================//
UsableProperty::UsableProperty(DynObjectSet *set)
:	ObjectProperty(set)
{
}

//=====================================================================================//
//                           bool UsableProperty::CanPass()                            //
//=====================================================================================//
bool UsableProperty::CanPass(const ipnt2_t &np)
{
	const HexGrid::cell &cell = HexGrid::GetInst()->Get(np);

	for(int p=0; p<6; p++)
	{
		if(cell.GetWeight(p)<50) return true;
	}

	return false;
}
