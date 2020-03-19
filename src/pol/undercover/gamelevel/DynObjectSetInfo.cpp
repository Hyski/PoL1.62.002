#include "precomp.h"
#include "DynObjectSetInfo.h"
#include "DynObjectOldData.h"

//=====================================================================================//
//                        DynObjectSetInfo::DynObjectSetInfo()                         //
//=====================================================================================//
DynObjectSetInfo::DynObjectSetInfo()
:	m_name("@#absolutely_wrong_object_name#@"),
	m_fullName("@#absolutely_wrong_object_name#@"),
	m_type(0)
{
}

////=====================================================================================//
////                        DynObjectSetInfo::DynObjectSetInfo()                         //
////=====================================================================================//
//DynObjectSetInfo::DynObjectSetInfo(SavSlot &slot)
//:	m_name("@#absolutely_wrong_object_name#@"),
//	m_fullName("@#absolutely_wrong_object_name#@"),
//	m_type(0)
//{
//	Read(slot);
//}

//=====================================================================================//
//                        DynObjectSetInfo::DynObjectSetInfo()                         //
//=====================================================================================//
DynObjectSetInfo::DynObjectSetInfo(const DynObjectOldData &dta, const std::string &levname)
:	m_name(dta.GetName()),
	m_fullName(m_name + ":" + levname),
	m_type(0)
{
	if(dta.IsDoor()) m_type |= otDoor;
	if(dta.IsTrain()) m_type |= otTrain;
	if(dta.IsUsable()) m_type |= otUsable;
	if(dta.IsStorage()) m_type |= otStorage;
	if(dta.IsElevator()) m_type |= otElevator;
	if(dta.IsDatabase()) m_type |= otDatabase;
	if(dta.IsInvisible()) m_type |= otInvisible;
	if(dta.IsDestructible()) m_type |= otDestructible;

	m_useInfo = dta.GetUseInfo();
	m_trainInfo = dta.GetTrainInfo();
	m_caseInfo = dta.GetStorageInfo();
}

////=====================================================================================//
////                            void DynObjectSetInfo::Read()                            //
////=====================================================================================//
//void DynObjectSetInfo::Read(SavSlot &slot)
//{
//	*this = DynObjectSetInfo();
//	assert( slot.GetStore()->GetVersion() > 4 );
//	Read5(slot);
//}
//
////=====================================================================================//
////                           void DynObjectSetInfo::Read5()                            //
////=====================================================================================//
//void DynObjectSetInfo::Read5(SavSlot &slot)
//{
//}