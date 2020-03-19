#include "precomp.h"
#include "DynObjectInfo.h"
#include "DynObjectOldData.h"

//=====================================================================================//
//                           DynObjectInfo::DynObjectInfo()                            //
//=====================================================================================//
DynObjectInfo::DynObjectInfo()
:	m_name("@#absolutely_wrong_object_name#@"),
	m_type(0),
	m_physType(0)
{
}

//=====================================================================================//
//                           DynObjectInfo::DynObjectInfo()                            //
//=====================================================================================//
DynObjectInfo::DynObjectInfo(SavSlot &slot)
:	m_name("@#absolutely_wrong_object_name#@"),
	m_type(0),
	m_physType(0)
{
	Read(slot);
}

//=====================================================================================//
//                           DynObjectInfo::DynObjectInfo()                            //
//=====================================================================================//
DynObjectInfo::DynObjectInfo(const DynObjectOldData &dta)
:	m_name(dta.GetName()),
	m_type(0),
	m_physType(0)
{
	if(dta.IsAnimated()) m_type |= otAnimated;
	if(dta.IsSoundable()) m_type |= otSoundable;
	if(dta.IsDestructible()) m_type |= otDestructible;

	if(dta.IsGhost()) m_physType |= ptGhost;
	if(dta.IsTransparent()) m_physType |= ptTransparent;

	m_soundInfo = dta.GetSoundInfo();
	m_destructibleInfo = dta.GetDestructibleInfo();
}

//=====================================================================================//
//                             void DynObjectInfo::Read()                              //
//=====================================================================================//
void DynObjectInfo::Read(SavSlot &slot)
{
	*this = DynObjectInfo();
	assert( slot.GetStore()->GetVersion() > 4 );
	Read5(slot);
}

//=====================================================================================//
//                             void DynObjectInfo::Read5()                             //
//=====================================================================================//
void DynObjectInfo::Read5(SavSlot &slot)
{
}