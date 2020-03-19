#include "precomp.h"

#include "Act.h"
#include "ActMgr.h"
#include "Trigger.h"
#include "Bureau2.h"
#include "Counter.h"
#include "SavedData.h"
#include "ActFactory.h"
#include "JournalMgr.h"
#include "TaskDatabase.h"
#include "UnlimitedMovesMgr.h"
#include <Logic2/GameFormImp.h>

namespace PoL
{

//=====================================================================================//
//                               namespace ActMgrDetail                                //
//=====================================================================================//
namespace ActMgrDetail
{

//=====================================================================================//
//                        static std::string getEventDataStr()                         //
//=====================================================================================//
static std::string getEventDataStr(const EventData &dta)
{
	std::ostringstream sstr;
	sstr << "\tepisode == " << dta.m_episode << "\n";
	sstr << "\tlevel == [" << dta.m_level << "]\n";
	sstr << "\tobject == [" << dta.m_object << "]\n";
	sstr << "\tphase == " << dta.m_phase << "\n";
	sstr << "\tsubject == [" << dta.m_subject << "]\n";
	return sstr.str();
}

//=====================================================================================//
//                                  struct NotStarted                                  //
//=====================================================================================//
struct NotStarted
{
	bool operator()(HAct act) const
	{
		return !act->isStarted();
	}
};

//=====================================================================================//
//                                  struct HasEndGame                                  //
//=====================================================================================//
struct HasEndGame
{
	bool operator()(HAct act) const
	{
		return act->hasEndGame();
	}
};

//=====================================================================================//
//                                  class ActMgrImpl                                   //
//=====================================================================================//
class ActMgrImpl
{
	typedef std::list<HAct> Acts_t;

	Acts_t m_acts;
	float m_lastPlayTime;

	bool m_alreadyInPlay;

	LocTextMgr m_locTextMgr;
	JournalMgr m_journalMgr;
	TaskDatabase m_database;
	ActFactory m_actFactory;
	UnlimitedMovesMgr m_unlimMoves;
	std::auto_ptr<Bureau2> m_bureau;

	struct Switcher
	{
		bool &m_ref;
		Switcher(bool &ref) : m_ref(ref) { m_ref = true; }
		~Switcher() { m_ref = false; }
	};

public:
	ActMgrImpl()
	:	m_lastPlayTime(0.0f),
		m_alreadyInPlay(false),
		m_bureau(new Bureau2)
	{
	}

	~ActMgrImpl()
	{
	}

	/// Начать проигрывание сцены
	void startPlayback(HAct act)
	{
		if(!act) return;

		assert( !act->isStarted() );
		assert( act->isEndable() );

		if(std::count_if(m_acts.begin(),m_acts.end(),HasEndGame()))
		{
			return;
		}

		if(act->hasEndGame())
		{
			m_acts.erase(std::remove_if(m_acts.begin(),m_acts.end(),NotStarted()),m_acts.end());
		}

		m_acts.push_back(act);
	}

	/// Проигрывается ли сейчас сцена
	bool isPlaying() const
	{
		return !m_acts.empty();
	}

	/// Рассчитать следующий кадр
	void play()
	{
		if(m_alreadyInPlay) return;

		assert( isPlaying() );
		Switcher switcher(m_alreadyInPlay);

		const float curtime = getCurrentTime();
		const float delta = curtime - m_lastPlayTime;
		m_lastPlayTime = curtime;

		HAct cact = m_acts.front();
		Acts_t::iterator iact = m_acts.begin();

		if(!cact->isStarted())
		{
			cact->start();

			if(cact->isEnded())
			{
				m_acts.erase(iact);
				if(m_acts.empty())
				{
					cleanupFromRunAct();
				}
			}
		}
		else
		{
			cact->update(delta);

			if(cact->isEnded())
			{
				m_acts.erase(iact);
				if(m_acts.empty())
				{
					cleanupFromRunAct();
				}
			}
		}
	}

	void onUseObject(const EventData &dta)
	{
		ACT_LOG("INFO: Received USE_OBJECT message from game:\n");
		ACT_LOG(getEventDataStr(dta));
		m_database.onUseObject(dta);
	}

	void onUseEntity(const EventData &dta)
	{
		ACT_LOG("INFO: Received USE_ENTITY message from game:\n");
		ACT_LOG(getEventDataStr(dta));
		m_database.onUseEntity(dta);
	}

	void onKillEntity(const EventData &dta)
	{
		ACT_LOG("INFO: Received KILL_ENTITY message from game:\n");
		ACT_LOG(getEventDataStr(dta));
		m_database.onKillEntity(dta);
	}

	void onStartGame(const EventData &dta)
	{
		ACT_LOG("INFO: Received START_GAME message from game:\n");
		ACT_LOG(getEventDataStr(dta));
		reset();
	}

	void onLevelExit(const EventData &dta)
	{
		ACT_LOG("INFO: Received LEVEL_EXIT message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onLeaveLevel(dta);
	}

	void onLevelEnter(const EventData &dta)
	{
		ACT_LOG("INFO: Received LEVEL_ENTER message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onEnterLevel(dta);
	}

	void onStartEpisode(const EventData &dta)
	{
		ACT_LOG("INFO: Received START_EPISODE message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onEpisodeChanged(dta);
	}

	void onEntitySpawned(const EventData &dta)
	{
		ACT_LOG("INFO: Received ENTITY_SPAWNED message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onEntitySpawned(dta);
	}

	void onDestroyEntity(const EventData &dta)
	{
		ACT_LOG("INFO: Received ENTITY_DESTROYED message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onEntityDestroyed(dta);
	}

	void onRelChanged(const EventData &dta)
	{
		ACT_LOG("INFO: Received RELATION_CHANGED message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onRelationChanged(dta);
	}

	void onEntityTeamChanged(const EventData &dta)
	{
		ACT_LOG("INFO: Received ENTITY_TEAM_CHANGED message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onEntityTeamChanged(dta);
	}

	void onPhaseChanged(const EventData &dta)
	{
		ACT_LOG("INFO: Received PHASE_CHANGED message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onPhaseChange(dta);
	}

	void onEndTurn(const EventData &dta)
	{
		ACT_LOG("INFO: Received END_TURN message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onEndTurn(dta);
	}

	void onStartTurn(const EventData &dta)
	{
		ACT_LOG("INFO: Received START_TURN message from game:\n");
		ACT_LOG(getEventDataStr(dta));

		m_database.onStartTurn(dta);
	}

	void reset()
	{
		ActFactory::instance()->reset();
		m_journalMgr.reset();
		m_database.reset();
		m_unlimMoves.reset();
		m_bureau.reset();
		m_bureau.reset(new Bureau2);
	}

	void store(std::ostream &out) const
	{
		mll::io::obinstream bout(out);
		m_journalMgr.store(bout);
		m_database.store(bout);
		ActFactory::instance()->store(bout);
		m_bureau->store(bout);
	}

	void restore(std::istream &in, unsigned int ver)
	{
		mll::io::ibinstream bin(in);
		m_journalMgr.restore(bin);
		m_database.restore(bin);
		ActFactory::instance()->restore(bin,ver);
		m_unlimMoves.update();

		if(ver >= 22)
		{
			m_bureau->restore(bin);
		}
	}

	void dumpState()
	{
		ActFactory::instance()->dumpState();
	}

	void dumpScriptParameters()
	{
		ActFactory::instance()->dumpScriptParameters();
	}

	bool isInPlay()
	{
		return m_alreadyInPlay;
	}

private:
	/// Подготовить уровень, интерфейс, скорость анимаций для проигрывания акта
	void prepareForRunAct()
	{
	}

	void cleanupFromRunAct()
	{
		if( ::Forms::GetInst()->HasGameForm() )
		{
			BaseEntity *curent = static_cast< ::GameFormImp * >( ::Forms::GetInst()->GetGameForm() )->GetCurEnt();

			if(curent && ::HexGrid::GetInst())
			{
				pnt_vec_t front;
				::PathUtils::CalcPassField(curent,&front);
				::GraphGrid::GetInst()->SetHexes(GraphGrid::HT_FRONT, front);
			}
		}
	}
};

static ActMgrImpl *g_actmgr = 0;

//=====================================================================================//
//                               ActMgrImpl &getActMgr()                               //
//=====================================================================================//
ActMgrImpl &getActMgr()
{
	if(!g_actmgr) g_actmgr = new ActMgrImpl;
	return *g_actmgr;
}

}

using namespace ActMgrDetail;

//=====================================================================================//
//                               void ActMgr::cleanup()                                //
//=====================================================================================//
void ActMgr::cleanup()
{
	delete g_actmgr;
}

//=====================================================================================//
//                            bool ActMgr::isPlayingScene()                            //
//=====================================================================================//
bool ActMgr::isPlayingScene()
{
	return getActMgr().isPlaying();
}

//=====================================================================================//
//                                 void ActMgr::play()                                 //
//=====================================================================================//
void ActMgr::play()
{
	getActMgr().play();
}

//=====================================================================================//
//                                void ActMgr::start()                                 //
//=====================================================================================//
void ActMgr::start(HAct act)
{
	getActMgr().startPlayback(act);
}

//=====================================================================================//
//                             void ActMgr::onUseObject()                              //
//=====================================================================================//
void ActMgr::onUseObject(const EventData &dta)
{
	getActMgr().onUseObject(dta);
}

//=====================================================================================//
//                             void ActMgr::onUseEntity()                              //
//=====================================================================================//
void ActMgr::onUseEntity(const EventData &dta)
{
	getActMgr().onUseEntity(dta);
}

//=====================================================================================//
//                             void ActMgr::onKillEntity()                             //
//=====================================================================================//
void ActMgr::onKillEntity(const EventData &dta)
{
	getActMgr().onKillEntity(dta);
}

//=====================================================================================//
//                             void ActMgr::onLevelExit()                              //
//=====================================================================================//
void ActMgr::onLevelExit(const EventData &dta)
{
	getActMgr().onLevelExit(dta);
}

//=====================================================================================//
//                             void ActMgr::onLevelEnter()                             //
//=====================================================================================//
void ActMgr::onLevelEnter(const EventData &dta)
{
	getActMgr().onLevelEnter(dta);
}

//=====================================================================================//
//                            void ActMgr::onStartEpisode()                            //
//=====================================================================================//
void ActMgr::onStartEpisode(const EventData &dta)
{
	getActMgr().onStartEpisode(dta);
}

//=====================================================================================//
//                             void ActMgr::onStartGame()                              //
//=====================================================================================//
void ActMgr::onStartGame(const EventData &dta)
{
	getActMgr().onStartGame(dta);
}

//=====================================================================================//
//                           void ActMgr::onEntitySpawned()                            //
//=====================================================================================//
void ActMgr::onEntitySpawned(const EventData &dta)
{
	getActMgr().onEntitySpawned(dta);
}

//=====================================================================================//
//                            void ActMgr::onPhaseChanged()                            //
//=====================================================================================//
void ActMgr::onPhaseChanged(const EventData &dta)
{
	getActMgr().onPhaseChanged(dta);
}

//=====================================================================================//
//                         void ActMgr::onEntityTeamChanged()                          //
//=====================================================================================//
void ActMgr::onEntityTeamChanged(const EventData &dta)
{
	getActMgr().onEntityTeamChanged(dta);
}

//=====================================================================================//
//                              void ActMgr::onEndTurn()                               //
//=====================================================================================//
void ActMgr::onEndTurn(const EventData &dta)
{
	getActMgr().onEndTurn(dta);
}

//=====================================================================================//
//                             void ActMgr::onStartTurn()                              //
//=====================================================================================//
void ActMgr::onStartTurn(const EventData &dta)
{
	getActMgr().onStartTurn(dta);
}

//=====================================================================================//
//                           void ActMgr::onDestroyEntity()                            //
//=====================================================================================//
void ActMgr::onDestroyEntity(const EventData &dta)
{
	getActMgr().onDestroyEntity(dta);
}

//=====================================================================================//
//                             void ActMgr::onRelChanged()                             //
//=====================================================================================//
void ActMgr::onRelChanged(const EventData &dta)
{
	getActMgr().onRelChanged(dta);
}


//=====================================================================================//
//                                void ActMgr::store()                                 //
//=====================================================================================//
void ActMgr::store(std::ostream &out)
{
	getActMgr().store(out);
}

//=====================================================================================//
//                               void ActMgr::restore()                                //
//=====================================================================================//
void ActMgr::restore(std::istream &in, unsigned int ver)
{
	getActMgr().reset();
	getActMgr().restore(in,ver);
}

//=====================================================================================//
//                                void ActMgr::reset()                                 //
//=====================================================================================//
void ActMgr::reset()
{
	getActMgr().reset();
}

//=====================================================================================//
//                              void ActMgr::dumpState()                               //
//=====================================================================================//
void ActMgr::dumpState()
{
	getActMgr().dumpState();
}

//=====================================================================================//
//                               bool ActMgr::isInPlay()                               //
//=====================================================================================//
bool ActMgr::isInPlay()
{
	return getActMgr().isInPlay();
}

//=====================================================================================//
//                         void ActMgr::dumpScriptParameters()                         //
//=====================================================================================//
void ActMgr::dumpScriptParameters()
{
	getActMgr().dumpScriptParameters();
}

}