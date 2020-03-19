#include "precomp.h"
#include "BringTask.h"
#include <logic2/Thing.h>
#include <logic2/AIUtils.h>
#include <logic2/ThingFactory.h>

namespace PoL
{

namespace BringTaskDetails
{

//=====================================================================================//
//                   class QuestThingFactory : public ::ThingFactory                   //
//=====================================================================================//
class QuestThingFactory : public ::ThingFactory
{
public:
	BaseThing* CreateKey(const rid_t& rid)
	{
		CreateThing(TT_KEY, rid);
		return 0;
	}

	BaseThing* CreateMoney(int sum)
	{
		CreateThing(TT_MONEY, "money");
		return 0;
	}

	BaseThing* CreateArmor(const rid_t& rid)
	{
		CreateThing(TT_ARMOUR, rid);
		return 0;
	}

	BaseThing* CreateShield(const rid_t& rid)
	{
		CreateThing(TT_SHIELD, rid);
		return 0;
	}

	BaseThing* CreateCamera(const rid_t& rid)
	{
		CreateThing(TT_CAMERA, rid);
		return 0;
	}

	BaseThing* CreateGrenade(const rid_t& rid)
	{
		CreateThing(TT_GRENADE, rid);
		return 0;
	}

	BaseThing* CreateScanner(const rid_t& rid)
	{
		CreateThing(TT_SCANNER, rid);
		return 0;
	}

	BaseThing* CreateImplant(const rid_t& rid)
	{
		CreateThing(TT_IMPLANT, rid);
		return 0;
	}

	BaseThing* CreateAmmo(const rid_t& rid, int cout)
	{
		CreateThing(TT_AMMO, rid);
		return 0;
	}

	BaseThing* CreateMedikit(const rid_t& rid, int charge)
	{
		CreateThing(TT_MEDIKIT, rid);
		return 0;
	}

	BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count)
	{
		CreateThing(TT_WEAPON, rid);
		return 0;
	}

protected:
	virtual void CreateThing(thing_type type, const rid_t& rid) = 0;
};

//=====================================================================================//
//                    class ThingsFinder : public QuestThingFactory                    //
//=====================================================================================//
class ThingsFinder : public QuestThingFactory
{
	player_type m_player;
	int m_money;
	bool m_fall_found;

public:
	ThingsFinder(player_type player = PT_PLAYER) : m_fall_found(true), m_player(player), m_money(0) {}     

	bool allFound() const { return m_fall_found; }

private:
	BaseThing* CreateMoney(int sum)
	{
		if(m_fall_found)
		{
			m_money += sum;
			m_fall_found = m_money <= MoneyMgr::GetInst()->GetMoney();
		}

		return 0;
	}

	void CreateThing(thing_type type, const rid_t& rid)
	{
		if(m_fall_found)
		{
			m_fall_found = HaveThing(HPK_HEAD, type, rid)
					   ||  HaveThing(HPK_BODY, type, rid)
					   ||  HaveThing(HPK_HANDS, type, rid)
					   ||  HaveThing(HPK_LKNEE, type, rid)
					   ||  HaveThing(HPK_RKNEE, type, rid)
					   ||  HaveThing(HPK_BACKPACK, type, rid)
					   ||  (type != TT_IMPLANT && HaveThing(HPK_IMPLANTS, type, rid));
		}
	}        

	bool HaveThing(human_pack_type pack, thing_type thing, const rid_t& rid)
	{
		EntityPool *epool = EntityPool::GetInst();

		for(EntityPool::iterator eit = epool->begin(ET_HUMAN, m_player); eit != epool->end(); ++eit)
		{
			HumanContext* context = eit->Cast2Human()->GetEntityContext();

			for(HumanContext::iterator tit = context->begin(pack, thing); tit != context->end(); ++tit)
			{
				if(tit->GetInfo()->GetRID() == rid)
				{
					return true;
				}
			}
		}

		return false;
	}
};

//=====================================================================================//
//                    class ThingsTaker : public QuestThingFactory                     //
//=====================================================================================//
class ThingsTaker : public QuestThingFactory
{
	player_type m_player;

public:
	ThingsTaker(player_type player = PT_PLAYER) : m_player(player) {}

private:
	BaseThing* CreateMoney(int sum)
	{
		MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() - sum);
		return 0;
	}

	void CreateThing(thing_type type, const rid_t& rid)
	{
		if(     RemoveThing(HPK_HEAD, type, rid)
			||  RemoveThing(HPK_BODY, type, rid)
			||  RemoveThing(HPK_HANDS, type, rid)
			||  RemoveThing(HPK_LKNEE, type, rid)
			||  RemoveThing(HPK_RKNEE, type, rid)
			||  RemoveThing(HPK_BACKPACK, type, rid)
			||  (type != TT_IMPLANT && RemoveThing(HPK_IMPLANTS, type, rid)))
			return;
	}

	bool RemoveThing(human_pack_type pack, thing_type type, const rid_t& rid)
	{
		EntityPool::iterator eit = EntityPool::GetInst()->begin(ET_HUMAN, m_player);
		while(eit != EntityPool::GetInst()->end())
		{
			HumanContext* context = eit->Cast2Human()->GetEntityContext();
			HumanContext::iterator tit = context->begin(pack, type);

			while(tit != context->end())
			{
				if(tit->GetInfo()->GetRID() == rid)
				{
					BaseThing* thing = &*tit;

					context->RemoveThing(thing);
					delete thing;

					return true;
				}
				++tit;
			}
			++eit;
		}
		return false;
	}
};

}

using namespace BringTaskDetails;

//=====================================================================================//
//                               BringTask::BringTask()                                //
//=====================================================================================//
BringTask::BringTask(Id_t id, const std::string &itset, const std::string &person,
					 HCondition sig)
:	Task(id),
	m_signal(sig),
	m_itset(itset),
	m_person(person),
	m_noFail(false)
{
	m_talkConn = TaskDatabase::instance()->attach(boost::bind(onSubjectTalk,this),m_person,TaskDatabase::mtPersonUsed);
	m_deadConn = TaskDatabase::instance()->attach(boost::bind(onSubjectDead,this),m_person,TaskDatabase::mtDead);
}

//=====================================================================================//
//                               BringTask::BringTask()                                //
//=====================================================================================//
BringTask::BringTask(Id_t id, const std::string &itset, const std::string &person,
					 HCondition sig, bool noFail)
:	Task(id),
	m_signal(sig),
	m_itset(itset),
	m_person(person),
	m_noFail(noFail)
{
	m_talkConn = TaskDatabase::instance()->attach(boost::bind(onSubjectTalk,this),m_person,TaskDatabase::mtPersonUsed);
	if(!m_noFail)
	{
		m_deadConn = TaskDatabase::instance()->attach(boost::bind(onSubjectDead,this),m_person,TaskDatabase::mtDead);
	}
}

//=====================================================================================//
//                             void BringTask::doEnable()                              //
//=====================================================================================//
void BringTask::doEnable(bool v)
{
	m_talkConn.enable(v);
	m_signal->enable(v);
}

//=====================================================================================//
//                           void BringTask::onSubjectDead()                           //
//=====================================================================================//
void BringTask::onSubjectDead()
{
	if(getState() == tsNotCompleted) setState(tsFailed);
}

//=====================================================================================//
//                            void BringTask::setPriority()                            //
//=====================================================================================//
void BringTask::setPriority(int p)
{
	m_talkConn.setPriority(p);
}

//=====================================================================================//
//                            void BringTask::store() const                            //
//=====================================================================================//
void BringTask::store(mll::io::obinstream &out) const
{
	Task::store(out);
}

//=====================================================================================//
//                              void BringTask::restore()                              //
//=====================================================================================//
void BringTask::restore(mll::io::ibinstream &in)
{
	Task::restore(in);
}

//=====================================================================================//
//                               void BringTask::reset()                               //
//=====================================================================================//
void BringTask::reset()
{
	Task::reset();
}

//=====================================================================================//
//                           void BringTask::onSubjectTalk()                           //
//=====================================================================================//
void BringTask::onSubjectTalk()
{
	if(isEnabled() && getState() == tsNotCompleted)
	{
        ThingsFinder finder;
		const bool parsedOk = parseThingScript(m_itset, ::ThingScriptParser(0, &finder));
		if(parsedOk && finder.allFound() && Forms::GetInst()->ShowMessageBox(DirtyLinks::GetStrRes("qqs_want_give_item")))
		{
			ThingsTaker taker;
			parseThingScript(m_itset, ::ThingScriptParser(0, &taker));
			setState(tsCompleted);
		}
	}
}

//=====================================================================================//
//                             BringSignal::BringSignal()                              //
//=====================================================================================//
BringSignal::BringSignal(Id_t id, const std::string &itset, const std::string &person)
:	Signal(id),
	m_itset(itset),
	m_person(person)
{
	m_talkConn = TaskDatabase::instance()->attach(boost::bind(onSignal,this),m_person,TaskDatabase::mtPersonUsed);
}

//=====================================================================================//
//                            void BringSignal::doEnable()                             //
//=====================================================================================//
void BringSignal::doEnable(bool b)
{
	m_talkConn.enable(b);
}

//=====================================================================================//
//                           void BringSignal::setPriority()                           //
//=====================================================================================//
void BringSignal::setPriority(int p)
{
	m_talkConn.setPriority(p);
}

//=====================================================================================//
//                            void BringSignal::onSignal()                             //
//=====================================================================================//
void BringSignal::onSignal()
{
	if(isEnabled())
	{
        ThingsFinder finder;
		const bool parsedOk = parseThingScript(m_itset, ::ThingScriptParser(0, &finder));
		if(parsedOk && finder.allFound())
		{
			notify();
		}
	}
}

}