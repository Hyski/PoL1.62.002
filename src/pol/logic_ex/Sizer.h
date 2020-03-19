#pragma once

namespace PoL
{

//=====================================================================================//
//                                   enum SizerTypes                                   //
//=====================================================================================//
enum SizerTypes
{
	stQuest,

	stBringTask,
	stEliminateTask,
	stKillTask,
	stStubTask,
	stTalkTask,
	stTaskList,
	
	stAliveCond,
	stAndCond,
	stCompareCond,
	stContextCond,
	stEpisodeCond,
	stHeroCond,
	stOrCond,
	stQuestCond,
	stTaskCond,

	stLeaveLevelSignal,
	stSmbUseSignal,
	stTaskDbSignal,
	stBringSignal,

	stRefCounter,
	stUsualCounter,

	stTrigger
};

//=====================================================================================//
//                                     class Sizer                                     //
//=====================================================================================//
class Sizer
{
public:
	virtual ~Sizer() {}

	/// Возвращает тип записи
	virtual SizerTypes getEntryType(mll::io::ibinstream &, const std::string &) = 0;
	/// Пропускает запись из сейва
	virtual void skipEntry(mll::io::ibinstream &in, const std::string &name) = 0;
};

}
