#pragma once

#include "ActTemplate.h"
#include "QuestMessage.h"

namespace PoL
{

//=====================================================================================//
//                                  class ActWorkshop                                  //
//=====================================================================================//
class ActWorkshop
{
public:
	static HAct createWalkAct(ActTemplate::Acts_t &, const std::string &, const ::ipnt2_t &);
	static HAct createWalkzAct(ActTemplate::Acts_t &, const std::string &, const std::string &);
	static HAct createRunAct(ActTemplate::Acts_t &, const std::string &, const ::ipnt2_t &);
	static HAct createRunzAct(ActTemplate::Acts_t &, const std::string &, const std::string &);
	static HAct createCrawlAct(ActTemplate::Acts_t &, const std::string &, const ::ipnt2_t &);
	static HAct createCrawlzAct(ActTemplate::Acts_t &, const std::string &, const std::string &);
	static HAct createLevelAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createWinGameAct(ActTemplate::Acts_t &);
	static HAct createLooseGameAct(ActTemplate::Acts_t &);
	static HAct createPrintAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createWaitAct(ActTemplate::Acts_t &, float);
	static HAct createSendQuestMessageAct(ActTemplate::Acts_t &, const std::string &, QuestMessage);
	static HAct createScriptSceneAct(ActTemplate::Acts_t &);
	static HAct createQuestReportAct(ActTemplate::Acts_t &);
	static HAct createResetCounterAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createAddToCounterAct(ActTemplate::Acts_t &, const std::string &, int delta);
	static HAct createFocusCameraAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createRefAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createGiveExpAct(ActTemplate::Acts_t &, int);
	static HAct createGiveMoneyAct(ActTemplate::Acts_t &, int);
	static HAct createGiveItemsAct(ActTemplate::Acts_t &, const std::string &, const std::string &);
	static HAct createPulseAct(ActTemplate::Acts_t &, const std::string &, int);
	static HAct createAddRankAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createHeroAct(ActTemplate::Acts_t &, const std::string &, bool);
	static HAct createForceHeroAct(ActTemplate::Acts_t &, const std::string &, bool);
	static HAct createPhaseAct(ActTemplate::Acts_t &, int p);
	static HAct createEnableLevelAct(ActTemplate::Acts_t &, const std::string &, bool);
	static HAct createDisappearAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createMovieAct(ActTemplate::Acts_t &, const std::string &, const std::string &);
	static HAct createSpawnAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createCamSplineAct(ActTemplate::Acts_t &, const std::string &, float);
	static HAct createDestructAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createShootDieAct(ActTemplate::Acts_t &, const std::string &, const std::string &, bool, bool);
	static HAct createJournalAct(ActTemplate::Acts_t &, const std::string &, const std::string &, const std::string &, bool);
	static HAct createSayAct(ActTemplate::Acts_t &, const std::string &, const std::string &);
	static HAct createDoorAct(ActTemplate::Acts_t &, const std::string &, bool);
	static HAct createAddNewsAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createCamFollowAct(ActTemplate::Acts_t &, const std::string &);
	static HAct createAlignmentAct(ActTemplate::Acts_t &, const std::string &, const std::string &, const std::string &);
	static HAct createSequenceAct(ActTemplate::Acts_t &);
	static HAct createParallelAct(ActTemplate::Acts_t &);
};

}