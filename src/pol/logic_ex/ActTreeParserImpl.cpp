#include "precomp.h"

#include "Act.h"
#include "Task.h"
#include "Quest.h"
#include "Trigger.h"
#include "StubTask.h"
#include "TaskList.h"
#include "KillTask.h"
#include "TalkTask.h"
#include "BringTask.h"
#include "RefCounter.h"
#include "ActFactory.h"
#include "ActWorkshop.h"
#include "OrCondition.h"
#include "AndCondition.h"
#include "UsualCounter.h"
#include "EliminateTask.h"
#include "HeroCondition.h"
#include "TaskCondition.h"
#include "AliveCondition.h"
#include "QuestCondition.h"
#include "OnSmbUseSignal.h"
#include "OnTaskDbSignal.h"
#include "ActTreeParser.hpp"
#include "ContextCondition.h"
#include "LeaveLevelSignal.h"
#include "EpisodeCondition.h"
#include "CompareCondition.h"

//=====================================================================================//
//                          void ActTreeParser::reportError()                          //
//=====================================================================================//
void ActTreeParser::reportError(const antlr::RecognitionException& ex)
{
	ACT_LOG("ERROR: " << ex.toString().c_str() << "\n");
}

//=====================================================================================//
//                          void ActTreeParser::reportError()                          //
//=====================================================================================//
void ActTreeParser::reportError(const std::string& s)
{
	if(getFilename().empty())
	{
		ACT_LOG("ERROR: " << s.c_str() << "\n");
	}
	else
	{
		ACT_LOG("ERROR: " << getFilename().c_str() << ": " << s.c_str() << "\n");
	}
}

//=====================================================================================//
//                         void ActTreeParser::reportWarning()                         //
//=====================================================================================//
void ActTreeParser::reportWarning(const std::string& s)
{
	if (getFilename().empty())
	{
		ACT_LOG("WARNING: " << s.c_str() << "\n");
	}
	else
	{
		ACT_LOG("WARNING: " << getFilename().c_str() << ": " << s.c_str() << "\n");
	}
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::walk()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::walk(const std::string &name, int x, int y)
{
	return boost::bind(PoL::ActWorkshop::createWalkAct,_1,name,::ipnt2_t(x,y));
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::walkz()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::walkz(const std::string &name, const std::string &zone)
{
	return boost::bind(PoL::ActWorkshop::createWalkzAct,_1,name,zone);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::walk()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::run(const std::string &name, int x, int y)
{
	return boost::bind(PoL::ActWorkshop::createRunAct,_1,name,::ipnt2_t(x,y));
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::walkz()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::runz(const std::string &name, const std::string &zone)
{
	return boost::bind(PoL::ActWorkshop::createRunzAct,_1,name,zone);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::walk()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::crawl(const std::string &name, int x, int y)
{
	return boost::bind(PoL::ActWorkshop::createCrawlAct,_1,name,::ipnt2_t(x,y));
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::walkz()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::crawlz(const std::string &name, const std::string &zone)
{
	return boost::bind(PoL::ActWorkshop::createCrawlzAct,_1,name,zone);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::level()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::level(const std::string &lev)
{
	return boost::bind(PoL::ActWorkshop::createLevelAct,_1,lev);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::winGame()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::winGame()
{
	return PoL::ActWorkshop::createWinGameAct;
}

//=====================================================================================//
//                   ActTreeParser::AC_t ActTreeParser::looseGame()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::looseGame()
{
	return PoL::ActWorkshop::createLooseGameAct;
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::print()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::print(const std::string &strid)
{
	return boost::bind(PoL::ActWorkshop::createPrintAct,_1,strid);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::wait()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::wait(float tau)
{
	return boost::bind(PoL::ActWorkshop::createWaitAct,_1,tau);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::qsend()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::qsend(const std::string &q, PoL::QuestMessage msg)
{
	return boost::bind(PoL::ActWorkshop::createSendQuestMessageAct,_1,q,msg);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::sscene()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::sscene()
{
	return PoL::ActWorkshop::createScriptSceneAct;
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::report()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::report()
{
	return PoL::ActWorkshop::createQuestReportAct;
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::reset()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::reset(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createResetCounterAct,_1,n);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::add()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::add(const std::string &n, int v)
{
	return boost::bind(PoL::ActWorkshop::createAddToCounterAct,_1,n,v);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::inc()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::inc(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createAddToCounterAct,_1,n,1);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::dec()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::dec(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createAddToCounterAct,_1,n,-1);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::addRank()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::addRank(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createAddRankAct,_1,n);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::addHero()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::addHero(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createHeroAct,_1,n,true);
}

//=====================================================================================//
//                  ActTreeParser::AC_t ActTreeParser::forceAddHero()                  //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::forceAddHero(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createForceHeroAct,_1,n,true);
}

//=====================================================================================//
//                   ActTreeParser::AC_t ActTreeParser::removeHero()                   //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::removeHero(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createHeroAct,_1,n,false);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::phase()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::phase(int p)
{
	return boost::bind(PoL::ActWorkshop::createPhaseAct,_1,p);
}

//=====================================================================================//
//                  ActTreeParser::AC_t ActTreeParser::enableLevel()                   //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::enableLevel(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createEnableLevelAct,_1,n,true);
}

//=====================================================================================//
//                  ActTreeParser::AC_t ActTreeParser::disableLevel()                  //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::disableLevel(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createEnableLevelAct,_1,n,false);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::camfocus()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::camfocus(const std::string &n)
{
	return boost::bind(PoL::ActWorkshop::createFocusCameraAct,_1,n);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::ref()                       //
//=====================================================================================//
void ActTreeParser::ref(PoL::HActTemplate tmpl, const std::string &n, It_t it)
{
	PoL::ActAttachQueue::instance()->queueActAttach(tmpl,n,it);
	//return boost::bind(PoL::ActWorkshop::createRefAct,_1,n);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::exp()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::exp(int experi)
{
	return boost::bind(PoL::ActWorkshop::createGiveExpAct,_1,experi);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::money()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::money(int experi)
{
	return boost::bind(PoL::ActWorkshop::createGiveMoneyAct,_1,experi);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::give()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::give(const std::string &itset, const std::string &t)
{
	return boost::bind(PoL::ActWorkshop::createGiveItemsAct,_1,itset,t);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::pulse()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::pulse(const std::string &name, int amount)
{
	return boost::bind(PoL::ActWorkshop::createPulseAct,_1,name,amount);
}

//=====================================================================================//
//                   ActTreeParser::AC_t ActTreeParser::disappear()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::disappear(const std::string &h)
{
	return boost::bind(PoL::ActWorkshop::createDisappearAct,_1,h);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::movie()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::movie(const std::string &bik, const std::string &levl)
{
	return boost::bind(PoL::ActWorkshop::createMovieAct,_1,bik,levl);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::spawn()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::spawn(const std::string &who)
{
	return boost::bind(PoL::ActWorkshop::createSpawnAct,_1,who);
}

//=====================================================================================//
//                   ActTreeParser::AC_t ActTreeParser::camspline()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::camspline(const std::string &cam, float dur)
{
	return boost::bind(PoL::ActWorkshop::createCamSplineAct,_1,cam,dur);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::destruct()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::destruct(const std::string &obj)
{
	return boost::bind(PoL::ActWorkshop::createDestructAct,_1,obj);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::shoot()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::shoot(const std::string &shooter, const std::string &target)
{
	return boost::bind(PoL::ActWorkshop::createShootDieAct,_1,shooter,target,true,false);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::kill()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::kill(const std::string &shooter, const std::string &target)
{
	return boost::bind(PoL::ActWorkshop::createShootDieAct,_1,shooter,target,true,true);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::die()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::die(const std::string &target)
{
	return boost::bind(PoL::ActWorkshop::createShootDieAct,_1,std::string(),target,false,true);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::addrec()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::addrec(const std::string &jid, const std::string &hdr,
										  const std::string &strid)
{
	return boost::bind(PoL::ActWorkshop::createJournalAct,_1,jid,hdr,strid,true);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::eraserec()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::eraserec(const std::string &jid)
{
	return boost::bind(PoL::ActWorkshop::createJournalAct,_1,jid,std::string(),std::string(),false);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::say()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::say(const std::string &who, const std::string &strid)
{
	return boost::bind(PoL::ActWorkshop::createSayAct,_1,who,strid);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::open()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::open(const std::string &door)
{
	return boost::bind(PoL::ActWorkshop::createDoorAct,_1,door,true);
}

//=====================================================================================//
//                     ActTreeParser::AC_t ActTreeParser::close()                      //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::close(const std::string &door)
{
	return boost::bind(PoL::ActWorkshop::createDoorAct,_1,door,false);
}

//=====================================================================================//
//                    ActTreeParser::AC_t ActTreeParser::addnews()                     //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::addnews(const std::string &newsid)
{
	return boost::bind(PoL::ActWorkshop::createAddNewsAct,_1,newsid);
}

//=====================================================================================//
//                   ActTreeParser::AC_t ActTreeParser::camfollow()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::camfollow(const std::string &pers)
{
	return boost::bind(PoL::ActWorkshop::createCamFollowAct,_1,pers);
}

//=====================================================================================//
//                   ActTreeParser::AC_t ActTreeParser::alignment()                    //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::alignment(const std::string &team1,
											 const std::string &team2,
											 const std::string &al)
{
	return boost::bind(PoL::ActWorkshop::createAlignmentAct,_1,team1,team2,al);
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::seq()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::seq()
{
	return PoL::ActWorkshop::createSequenceAct;
}

//=====================================================================================//
//                      ActTreeParser::AC_t ActTreeParser::par()                       //
//=====================================================================================//
ActTreeParser::AC_t ActTreeParser::par()
{
	return PoL::ActWorkshop::createParallelAct;
}

//=====================================================================================//
//                       void ActTreeParser::registerTemplate()                        //
//=====================================================================================//
void ActTreeParser::registerTemplate(const std::string &name, PoL::HActTemplate t)
{
	PoL::ActFactory::instance()->registerActTemplate(name,t);
}

//=====================================================================================//
//                       PoL::HQuest ActTreeParser::makeQuest()                        //
//=====================================================================================//
PoL::HQuest ActTreeParser::makeQuest(const std::string &name, Id_t id,
									 const PoL::QuestParams &params)
{
	PoL::HQuest q = new PoL::Quest(id,params);
	PoL::ActFactory::instance()->registerQuest(q);
	return q;
}

//=====================================================================================//
//                 ActTreeParser::Id_t ActTreeParser::reserveQuestId()                 //
//=====================================================================================//
ActTreeParser::Id_t ActTreeParser::reserveQuestId(const std::string &n)
{
	return PoL::ActFactory::instance()->reserveQuestId(n);
}

//=====================================================================================//
//                 ActTreeParser::Id_t ActTreeParser::reserveTaskId()                  //
//=====================================================================================//
ActTreeParser::Id_t ActTreeParser::reserveTaskId(const std::string &n)
{
	return PoL::ActFactory::instance()->reserveTaskId(n);
}

//=====================================================================================//
//                ActTreeParser::Id_t ActTreeParser::reserveTriggerId()                //
//=====================================================================================//
ActTreeParser::Id_t ActTreeParser::reserveTriggerId(const std::string &n)
{
	return PoL::ActFactory::instance()->reserveTriggerId(n);
}

//=====================================================================================//
//               ActTreeParser::Id_t ActTreeParser::reserveConditionId()               //
//=====================================================================================//
ActTreeParser::Id_t ActTreeParser::reserveConditionId(const std::string &n)
{
	return PoL::ActFactory::instance()->reserveConditionId(n);
}

//=====================================================================================//
//                ActTreeParser::Id_t ActTreeParser::reserveCounterId()                //
//=====================================================================================//
ActTreeParser::Id_t ActTreeParser::reserveCounterId(const std::string &n)
{
	return PoL::ActFactory::instance()->reserveCounterId(n);
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createStubTask()                      //
//=====================================================================================//
PoL::HTask ActTreeParser::createStubTask(Id_t id, const std::string &name)
{
	PoL::HTask result = new PoL::StubTask(id,PoL::tsNotCompleted);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createTaskList()                      //
//=====================================================================================//
PoL::HTask ActTreeParser::createTaskList(Id_t id, const std::string &name,
										 const std::vector<PoL::HTask> &tasks)
{
	MlHandle<PoL::TaskList> result = new PoL::TaskList(id,tasks.begin(),tasks.end());
	PoL::ActFactory::instance()->registerTask(result.get());
	return result.get();
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createKillTask()                      //
//=====================================================================================//
PoL::HTask ActTreeParser::createKillTask(Id_t id, const std::string &name,
										 const std::string &victim)
{
	PoL::HTask result = new PoL::KillTask(id,victim,false);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createDeadTask()                      //
//=====================================================================================//
PoL::HTask ActTreeParser::createDeadTask(Id_t id, const std::string &name,
										 const std::string &victim)
{
	PoL::HTask result = new PoL::KillTask(id,victim,true);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createTalkTask()                      //
//=====================================================================================//
PoL::HTask ActTreeParser::createTalkTask(Id_t id, const std::string &name,
										 const std::string &subj)
{
	PoL::HTask result = new PoL::TalkTask(id,subj);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createTalk2Task()                     //
//=====================================================================================//
PoL::HTask ActTreeParser::createTalk2Task(Id_t id, const std::string &name,
										 const std::string &subj)
{
	PoL::HTask result = new PoL::TalkTask(id,subj,true);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                     PoL::HTask ActTreeParser::createBringTask()                     //
//=====================================================================================//
PoL::HTask ActTreeParser::createBringTask(Id_t id, const std::string &name,
										  const std::string &pers,
										  const std::string &itset,
										  PoL::HCondition cond)
{
	PoL::HTask result = new PoL::BringTask(id,itset,pers,cond);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                    PoL::HTask ActTreeParser::createBring2Task()                     //
//=====================================================================================//
PoL::HTask ActTreeParser::createBring2Task(Id_t id, const std::string &name,
										  const std::string &pers,
										  const std::string &itset,
										  PoL::HCondition cond)
{
	PoL::HTask result = new PoL::BringTask(id,itset,pers,cond,true);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}

//=====================================================================================//
//                   PoL::HTask ActTreeParser::createEliminateTask()                   //
//=====================================================================================//
PoL::HTask ActTreeParser::createEliminateTask(Id_t id, const std::string &name,
											  const std::string &force,
											  const std::string &level)
{
	PoL::HTask result = new PoL::EliminateTask(id,force,level);
	PoL::ActFactory::instance()->registerTask(result);
	return result;
}


//=====================================================================================//
//                    PoL::HCounter ActTreeParser::createCounter()                     //
//=====================================================================================//
PoL::HCounter ActTreeParser::createCounter(Id_t id, const std::string &name, int value,
										   const CounterParams &cp)
{
	PoL::HCounter ctr = new PoL::UsualCounter(id,value,cp.m_inc,cp.m_dec);
	PoL::ActFactory::instance()->registerCounter(ctr);
	return ctr;
}

//=====================================================================================//
//                   PoL::HCounter ActTreeParser::createRefCounter()                   //
//=====================================================================================//
PoL::HCounter ActTreeParser::createRefCounter(Id_t id, const std::string &name,
											  const std::string &ref)
{
	PoL::HCounter rctr = PoL::ActFactory::instance()->getCounterByName(ref);
	if(!rctr)
	{
		ACT_LOG("WARNING: counter \"" << ref << "\" not found\n");
	}

	PoL::HCounter ctr = new PoL::RefCounter(id,rctr);
	PoL::ActFactory::instance()->registerCounter(ctr);
	return ctr;
}

//=====================================================================================//
//                  PoL::HCounter ActTreeParser::createConstCounter()                  //
//=====================================================================================//
PoL::HCounter ActTreeParser::createConstCounter(Id_t id, const std::string &name, int value)
{
	PoL::HCounter ctr = new PoL::UsualCounter(id,value,0,0);
	PoL::ActFactory::instance()->registerCounter(ctr);
	return ctr;
}

//=====================================================================================//
//                  PoL::HCondition ActTreeParser::getRefCondition()                   //
//=====================================================================================//
PoL::HCondition ActTreeParser::getRefCondition(const std::string &name)
{
	return PoL::ActFactory::instance()->getConditionByName(name);
}

//=====================================================================================//
//                PoL::HCondition ActTreeParser::createAliveCondition()                //
//=====================================================================================//
PoL::HCondition ActTreeParser::createAliveCondition(ActTreeParser::Id_t id,
													const std::string &name,
													const std::string &who)
{
	PoL::HCondition cond = new PoL::AliveCondition(id,who);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                PoL::HCondition ActTreeParser::createDeadCondition()                 //
//=====================================================================================//
PoL::HCondition ActTreeParser::createDeadCondition(ActTreeParser::Id_t id,
												   const std::string &name,
												   const std::string &who)
{
	PoL::HCondition cond = new PoL::AliveCondition(id,who,false);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                 PoL:HCondition ActTreeParser::createTaskCondition()                 //
//=====================================================================================//
PoL::HCondition ActTreeParser::createTaskCondition(Id_t id, const std::string &name,
												   PoL::HTask task, PoL::TaskState state)
{
	PoL::HCondition cond = new PoL::TaskCondition(id,task,state);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//               PoL::HCondition ActTreeParser::createContextCondition()               //
//=====================================================================================//
PoL::HCondition ActTreeParser::createContextCondition(Id_t id, const std::string &name,
													  const std::string &lev, int phase)
{
	PoL::HCondition cond = new PoL::ContextCondition(id,lev,phase);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//              oL::HCondition ActTreeParser::createCompareCondition() {               //
//=====================================================================================//
PoL::HCondition ActTreeParser::createCompareCondition(Id_t id, const std::string &name,
													  PoL::LogicalCondition lc,
													  PoL::HCounter l, PoL::HCounter r)
{
	PoL::HCondition cond = new PoL::CompareCondition(id,lc,l,r);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                PoL::HCondition ActTreeParser::createQuestCondition()                //
//=====================================================================================//
PoL::HCondition ActTreeParser::createQuestCondition(Id_t id, const std::string &name,
													PoL::QuestConditionState qcs,
													const std::string &quest)
{
	PoL::HQuestRef qst = PoL::ActFactory::instance()->getQuestRefByName(quest);
	PoL::HCondition cond = new PoL::QuestCondition(id,qcs,qst);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//               PoL::HCondition ActTreeParser::createEpisodeCondition()               //
//=====================================================================================//
PoL::HCondition ActTreeParser::createEpisodeCondition(Id_t id, const std::string &name,
													  int episode)
{
	PoL::HCondition cond = new PoL::EpisodeCondition(id,episode);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//              PoL::HCondition ActTreeParser::createHaveHeroCondition()               //
//=====================================================================================//
PoL::HCondition ActTreeParser::createHaveHeroCondition(Id_t id, const std::string &name,
													   const std::string &hero)
{
	PoL::HCondition cond = new PoL::HeroCondition(id,hero,true);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//               PoL::HCondition ActTreeParser::createNoHeroCondition()                //
//=====================================================================================//
PoL::HCondition ActTreeParser::createNoHeroCondition(Id_t id, const std::string &name,
													 const std::string &hero)
{
	PoL::HCondition cond = new PoL::HeroCondition(id,hero,false);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                 PoL::HCondition ActTreeParser::createAndCondition()                 //
//=====================================================================================//
PoL::HCondition ActTreeParser::createAndCondition(Id_t id, const std::string &name,
												  PoL::HCondition l, PoL::HCondition r)
{
	PoL::HCondition cond = new PoL::AndCondition(id,l,r);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                 PoL::HCondition ActTreeParser::createOrCondition()                  //
//=====================================================================================//
PoL::HCondition ActTreeParser::createOrCondition(Id_t id, const std::string &name,
												 PoL::HCondition l, PoL::HCondition r)
{
	PoL::HCondition cond = new PoL::OrCondition(id,l,r);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                 PoL::HCondition ActTreeParser::createBringSignal()                  //
//=====================================================================================//
PoL::HCondition ActTreeParser::createBringSignal(Id_t id, const std::string &name,
												 const std::string &person,
												 const std::string &itset)
{
	PoL::HCondition cond = new PoL::BringSignal(id,itset,person);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                  PoL::HCondition ActTreeParser::createKillSignal()                  //
//=====================================================================================//
PoL::HCondition ActTreeParser::createKillSignal(Id_t id, const std::string &name,
												const std::string &subj)
{
	PoL::HCondition cond = new PoL::OnTaskDbSignal(id,PoL::TaskDatabase::mtKilled,subj);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                  PoL::HCondition ActTreeParser::createDeadSignal()                  //
//=====================================================================================//
PoL::HCondition ActTreeParser::createDeadSignal(Id_t id, const std::string &name,
												const std::string &subj)
{
	PoL::HCondition cond = new PoL::OnTaskDbSignal(id,PoL::TaskDatabase::mtDead,subj);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                  PoL::HCondition ActTreeParser::createTalkSignal()                  //
//=====================================================================================//
PoL::HCondition ActTreeParser::createTalkSignal(Id_t id, const std::string &name,
												const std::string &subj)
{
	PoL::HCondition cond = new PoL::OnTaskDbSignal(id,PoL::TaskDatabase::mtPersonUsed,subj);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                  PoL::HCondition ActTreeParser::createUseSignal()                   //
//=====================================================================================//
PoL::HCondition ActTreeParser::createUseSignal(Id_t id, const std::string &name,
											   const std::string &obj, const std::string &subj)
{
	PoL::HCondition cond;
	
	if(obj.empty())
	{
		cond = new PoL::OnTaskDbSignal(id,PoL::TaskDatabase::mtItemUsed,subj);
	}
	else
	{
		cond = new PoL::OnSmbUseSignal(id,obj,subj);
	}

	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//               PoL::HCondition ActTreeParser::createStartTurnSignal()                //
//=====================================================================================//
PoL::HCondition ActTreeParser::createStartTurnSignal(Id_t id, const std::string &name)
{
	PoL::HCondition cond = new PoL::OnTaskDbSignal(id,PoL::TaskDatabase::mtStartTurn);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                PoL::HCondition ActTreeParser::createEndTurnSignal()                 //
//=====================================================================================//
PoL::HCondition ActTreeParser::createEndTurnSignal(Id_t id, const std::string &name)
{
	PoL::HCondition cond = new PoL::OnTaskDbSignal(id,PoL::TaskDatabase::mtEndTurn);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//               PoL::HCondition ActTreeParser::createLeaveLevelSignal()               //
//=====================================================================================//
PoL::HCondition ActTreeParser::createLeaveLevelSignal(Id_t id, const std::string &name,
													  const std::string &lev,
													  const std::string &to)
{
	PoL::HCondition cond = new PoL::LeaveLevelSignal(id,lev,to);
	PoL::ActFactory::instance()->registerCondition(cond);
	return cond;
}

//=====================================================================================//
//                    PoL::HTrigger ActTreeParser::createTrigger()                     //
//=====================================================================================//
PoL::HTrigger ActTreeParser::createTrigger(Id_t id, const std::string &name,
										   PoL::HCondition cond, PoL::HActTemplate a,
										   bool onetime)
{
	PoL::HTrigger trig = new PoL::Trigger(id,cond,a,onetime);
	PoL::ActFactory::instance()->registerTrigger(trig);
	return trig;
}
