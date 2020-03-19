#pragma once

namespace PoL
{

//=====================================================================================//
//                                   enum QuestState                                   //
//=====================================================================================//
enum QuestState
{
	qsNotStarted,
	qsStarted,
	qsReadyToPass,
	qsPassed,
	qsFailed,
	qsTaskDone,
	qsCompleted,
	qsReadyToFail,
	qsReadyToPassBad,
	qsDestroyed,
	//qsUncompletable,
	qsStateCount
};

}
