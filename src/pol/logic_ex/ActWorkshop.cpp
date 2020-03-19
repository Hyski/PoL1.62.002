#include "precomp.h"

#include "Task.h"
#include "RefAct.h"
#include "SayAct.h"
#include "HeroAct.h"
#include "WaitAct.h"
#include "DoorAct.h"
#include "PrintAct.h"
#include "PhaseAct.h"
#include "MovieAct.h"
#include "PulseAct.h"
#include "SpawnAct.h"
#include "AddNewsAct.h"
#include "JournalAct.h"
#include "GiveExpAct.h"
#include "WinGameAct.h"
#include "AddRankAct.h"
#include "DestructAct.h"
#include "ShootDieAct.h"
#include "ActWorkshop.h"
#include "ParallelAct.h"
#include "GiveMoneyAct.h"
#include "AlignmentAct.h"
#include "CamFollowAct.h"
#include "CamSplineAct.h"
#include "GiveItemsAct.h"
#include "LooseGameAct.h"
#include "DisappearAct.h"
#include "MovePersonAct.h"
#include "SequentialAct.h"
#include "EnableLevelAct.h"
#include "QuestReportAct.h"
#include "CameraFocusAct.h"
#include "ChangeLevelAct.h"
#include "ScriptSceneAct.h"
#include "ResetCounterAct.h"
#include "AddToCounterAct.h"
#include "SendQuestMessageAct.h"

namespace PoL
{

//=====================================================================================//
//                          HAct ActWorkshop::createWalkAct()                          //
//=====================================================================================//
HAct ActWorkshop::createWalkAct(ActTemplate::Acts_t &, const std::string &name, const ipnt2_t &target)
{
	return new MovePersonAct(name,target,MovePersonAct::mtWalk);
}

//=====================================================================================//
//                         HAct ActWorkshop::createWalkzAct()                          //
//=====================================================================================//
HAct ActWorkshop::createWalkzAct(ActTemplate::Acts_t &, const std::string &name, const std::string &zone)
{
	return new MovePersonAct(name,zone,MovePersonAct::mtWalk);
}

//=====================================================================================//
//                          HAct ActWorkshop::createRunAct()                           //
//=====================================================================================//
HAct ActWorkshop::createRunAct(ActTemplate::Acts_t &, const std::string &name, const ipnt2_t &target)
{
	return new MovePersonAct(name,target,MovePersonAct::mtRun);
}

//=====================================================================================//
//                          HAct ActWorkshop::createRunzAct()                          //
//=====================================================================================//
HAct ActWorkshop::createRunzAct(ActTemplate::Acts_t &, const std::string &name, const std::string &zone)
{
	return new MovePersonAct(name,zone,MovePersonAct::mtRun);
}

//=====================================================================================//
//                         HAct ActWorkshop::createCrawlAct()                          //
//=====================================================================================//
HAct ActWorkshop::createCrawlAct(ActTemplate::Acts_t &, const std::string &name, const ipnt2_t &target)
{
	return new MovePersonAct(name,target,MovePersonAct::mtCrawl);
}

//=====================================================================================//
//                         HAct ActWorkshop::createCrawlzAct()                         //
//=====================================================================================//
HAct ActWorkshop::createCrawlzAct(ActTemplate::Acts_t &, const std::string &name, const std::string &zone)
{
	return new MovePersonAct(name,zone,MovePersonAct::mtCrawl);
}

//=====================================================================================//
//                         HAct ActWorkshop::createLevelAct()                          //
//=====================================================================================//
HAct ActWorkshop::createLevelAct(ActTemplate::Acts_t &, const std::string &name)
{
	return new ChangeLevelAct(name);
}

//=====================================================================================//
//                         HAct ActWorkshop::createPrintAct()                          //
//=====================================================================================//
HAct ActWorkshop::createPrintAct(ActTemplate::Acts_t &, const std::string &strid)
{
	return new PrintAct(strid);
}

//=====================================================================================//
//                        HAct ActWorkshop::createWinGameAct()                         //
//=====================================================================================//
HAct ActWorkshop::createWinGameAct(ActTemplate::Acts_t &)
{
	return new WinGameAct;
}

//=====================================================================================//
//                       HAct ActWorkshop::createLooseGameAct()                        //
//=====================================================================================//
HAct ActWorkshop::createLooseGameAct(ActTemplate::Acts_t &)
{
	return new LooseGameAct;
}

//=====================================================================================//
//                          HAct ActWorkshop::createWaitAct()                          //
//=====================================================================================//
HAct ActWorkshop::createWaitAct(ActTemplate::Acts_t &, float d)
{
	return new WaitAct(d);
}

//=====================================================================================//
//                    HAct ActWorkshop::createSendQuestMessageAct()                    //
//=====================================================================================//
HAct ActWorkshop::createSendQuestMessageAct(ActTemplate::Acts_t &, const std::string &q,
											QuestMessage msg)
{
	return new SendQuestMessageAct(q,msg);
}

//=====================================================================================//
//                      HAct ActWorkshop::createScriptSceneAct()                       //
//=====================================================================================//
HAct ActWorkshop::createScriptSceneAct(ActTemplate::Acts_t &acts)
{
	assert( acts.size() == 1 );
	return new ScriptSceneAct(acts.front());
}

//=====================================================================================//
//                      HAct ActWorkshop::createQuestReportAct()                       //
//=====================================================================================//
HAct ActWorkshop::createQuestReportAct(ActTemplate::Acts_t &acts)
{
	assert( acts.size() == 1 );
	return new QuestReportAct(acts.front());
}

//=====================================================================================//
//                      HAct ActWorkshop::createFocusCameraAct()                       //
//=====================================================================================//
HAct ActWorkshop::createFocusCameraAct(ActTemplate::Acts_t &, const std::string &e)
{
	return new CameraFocusAct(e);
}

//=====================================================================================//
//                          HAct ActWorkshop::createRefAct()                           //
//=====================================================================================//
HAct ActWorkshop::createRefAct(ActTemplate::Acts_t &, const std::string &e)
{
	return new RefAct(e);
}

//=====================================================================================//
//                        HAct ActWorkshop::createGiveExpAct()                         //
//=====================================================================================//
HAct ActWorkshop::createGiveExpAct(ActTemplate::Acts_t &, int exp)
{
	return new GiveExpAct(exp);
}

//=====================================================================================//
//                       HAct ActWorkshop::createGiveMoneyAct()                        //
//=====================================================================================//
HAct ActWorkshop::createGiveMoneyAct(ActTemplate::Acts_t &, int exp)
{
	return new GiveMoneyAct(exp);
}

//=====================================================================================//
//                       HAct ActWorkshop::createGiveItemsAct()                        //
//=====================================================================================//
HAct ActWorkshop::createGiveItemsAct(ActTemplate::Acts_t &, const std::string &itset,
									 const std::string &target)
{
	return new GiveItemsAct(itset,target);
}

//=====================================================================================//
//                         HAct ActWorkshop::createPulseAct()                          //
//=====================================================================================//
HAct ActWorkshop::createPulseAct(ActTemplate::Acts_t &, const std::string &name,
								 int amount)
{
	return new PulseAct(name,amount);
}

//=====================================================================================//
//                        HAct ActWorkshop::createAddRankAct()                         //
//=====================================================================================//
HAct ActWorkshop::createAddRankAct(ActTemplate::Acts_t &, const std::string &name)
{
	return new AddRankAct(name);
}

//=====================================================================================//
//                          HAct ActWorkshop::createHeroAct()                          //
//=====================================================================================//
HAct ActWorkshop::createHeroAct(ActTemplate::Acts_t &, const std::string &h, bool v)
{
	return new HeroAct(h,v);
}

//=====================================================================================//
//                       HAct ActWorkshop::createForceHeroAct()                        //
//=====================================================================================//
HAct ActWorkshop::createForceHeroAct(ActTemplate::Acts_t &, const std::string &h, bool v)
{
	return new HeroAct(h,v,false,true);
}

//=====================================================================================//
//                        HAct ActWorkshop::createSequenceAct()                        //
//=====================================================================================//
HAct ActWorkshop::createSequenceAct(ActTemplate::Acts_t &acts)
{
	return new SequentialAct(acts.begin(),acts.end());
}

//=====================================================================================//
//                        HAct ActWorkshop::createParallelAct()                        //
//=====================================================================================//
HAct ActWorkshop::createParallelAct(ActTemplate::Acts_t &acts)
{
	return new ParallelAct(acts.begin(),acts.end());
}

//=====================================================================================//
//                      HAct ActWorkshop::createAddToCounterAct()                      //
//=====================================================================================//
HAct ActWorkshop::createAddToCounterAct(ActTemplate::Acts_t &, const std::string &c,
										int delta)
{
	return new AddToCounterAct(c,delta);
}

//=====================================================================================//
//                      HAct ActWorkshop::createResetCounterAct()                      //
//=====================================================================================//
HAct ActWorkshop::createResetCounterAct(ActTemplate::Acts_t &, const std::string &c)
{
	return new ResetCounterAct(c);
}

//=====================================================================================//
//                         HAct ActWorkshop::createPhaseAct()                          //
//=====================================================================================//
HAct ActWorkshop::createPhaseAct(ActTemplate::Acts_t &, int p)
{
	return new PhaseAct(p);
}

//=====================================================================================//
//                      HAct ActWorkshop::createEnableLevelAct()                       //
//=====================================================================================//
HAct ActWorkshop::createEnableLevelAct(ActTemplate::Acts_t &, const std::string &l, bool e)
{
	return new EnableLevelAct(l,e);
}

//=====================================================================================//
//                       HAct ActWorkshop::createDisappearAct()                        //
//=====================================================================================//
HAct ActWorkshop::createDisappearAct(ActTemplate::Acts_t &, const std::string &n)
{
	return new DisappearAct(n);
}

//=====================================================================================//
//                         HAct ActWorkshop::createMovieAct()                          //
//=====================================================================================//
HAct ActWorkshop::createMovieAct(ActTemplate::Acts_t &, const std::string &bik,
								 const std::string &levl)
{
	return new MovieAct(bik,levl);
}

//=====================================================================================//
//                         HAct ActWorkshop::createSpawnAct()                          //
//=====================================================================================//
HAct ActWorkshop::createSpawnAct(ActTemplate::Acts_t &, const std::string &who)
{
	return new SpawnAct(who);
}

//=====================================================================================//
//                       HAct ActWorkshop::createCamSplineAct()                        //
//=====================================================================================//
HAct ActWorkshop::createCamSplineAct(ActTemplate::Acts_t &, const std::string &cam,
									 float dur)
{
	return new CamSplineAct(cam,dur);
}

//=====================================================================================//
//                        HAct ActWorkshop::createDestructAct()                        //
//=====================================================================================//
HAct ActWorkshop::createDestructAct(ActTemplate::Acts_t &, const std::string &obj)
{
	return new DestructAct(obj);
}

//=====================================================================================//
//                        HAct ActWorkshop::createShootDieAct()                        //
//=====================================================================================//
HAct ActWorkshop::createShootDieAct(ActTemplate::Acts_t &, const std::string &shooter,
									const std::string &target, bool shoot, bool die)
{
	return new ShootDieAct(shooter,target,shoot,die);
}

//=====================================================================================//
//                        HAct ActWorkshop::createJournalAct()                         //
//=====================================================================================//
HAct ActWorkshop::createJournalAct(ActTemplate::Acts_t &, const std::string &jid,
								   const std::string &hdr, const std::string &strid, bool add)
{
	return new JournalAct(jid,hdr,strid,add);
}

//=====================================================================================//
//                          HAct ActWorkshop::createSayAct()                           //
//=====================================================================================//
HAct ActWorkshop::createSayAct(ActTemplate::Acts_t &, const std::string &actor,
							   const std::string &strid)
{
	return new SayAct(actor,strid);
}

//=====================================================================================//
//                          HAct ActWorkshop::createDoorAct()                          //
//=====================================================================================//
HAct ActWorkshop::createDoorAct(ActTemplate::Acts_t &, const std::string &door, bool open)
{
	return new DoorAct(door,open);
}

//=====================================================================================//
//                        HAct ActWorkshop::createAddNewsAct()                         //
//=====================================================================================//
HAct ActWorkshop::createAddNewsAct(ActTemplate::Acts_t &, const std::string &newsid)
{
	return new AddNewsAct(newsid);
}

//=====================================================================================//
//                       HAct ActWorkshop::createCamFollowAct()                        //
//=====================================================================================//
HAct ActWorkshop::createCamFollowAct(ActTemplate::Acts_t &, const std::string &person)
{
	return new CamFollowAct(person);
}

//=====================================================================================//
//                       HAct ActWorkshop::createAlignmentAct()                        //
//=====================================================================================//
HAct ActWorkshop::createAlignmentAct(ActTemplate::Acts_t &, const std::string &team1,
									 const std::string &team2, const std::string &al)
{
	return new AlignmentAct(team1,team2,al);
}

}