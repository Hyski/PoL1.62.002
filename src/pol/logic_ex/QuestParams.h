#pragma once

namespace PoL
{

//=====================================================================================//
//                                 struct QuestParams                                  //
//=====================================================================================//
struct QuestParams
{
	bool m_enabled;
	HTask m_start, m_end, m_quest;
	HCondition m_remind, m_dependency;
	std::string m_holder;
	HActTemplate m_onStart, m_onSucceed, m_onFail,
				 m_onRemind, m_onCompletedOnStart, m_onFailedOnStart,
				 m_onPreSucceed;
};

}
