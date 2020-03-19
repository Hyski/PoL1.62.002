#include "precomp.h"

#include "UnlimitedMovesMgr.h"

#include <logic2/damager.h>
#include <logic2/enemydetection.h>

namespace PoL
{

namespace UnlimitedMovesDetails
{

//=====================================================================================//
//                            void DisplayHealthIncrease()                             //
//=====================================================================================//
void DisplayHealthIncrease(HumanEntity* human, int delta)
{
    DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("aiu_got_health").c_str(), human->GetInfo()->GetName().c_str(), delta));
    EffectMgr::GetInst()->MakeHumanTreatment(human->GetGraph()->Cast2Human());
}

//=====================================================================================//
//                              void ApplyHealthChange()                               //
//=====================================================================================//
void ApplyHealthChange(HumanContext* human, int delta)
{   
    HumanContext::Traits* traits = human->GetTraits();
    HumanContext::Limits* limits = human->GetLimits();
    
    traits->AddStrength(limits->GetStrength());
    traits->AddReaction(limits->GetReaction());
    traits->AddAccuracy(limits->GetAccuracy());
    traits->AddDexterity(limits->GetDexterity());
    traits->AddMechanics(limits->GetMechanics());
    traits->AddFrontRadius(limits->GetFrontRadius());
}

}

using namespace UnlimitedMovesDetails;

static UnlimitedMovesMgr *g_unlimoves = 0;

//=====================================================================================//
//                       UnlimitedMovesMgr::UnlimitedMovesMgr()                        //
//=====================================================================================//
UnlimitedMovesMgr::UnlimitedMovesMgr()
:	m_hasEnemies(true),
	m_enabled(true),
	m_isGameLost(false)
{
	assert( g_unlimoves == 0 );
	g_unlimoves = this;

	TaskDatabase *tdb = TaskDatabase::instance();

	m_spawnConn = tdb->attach(boost::bind(onSpawn,this),TaskDatabase::mtEntitySpawned);
	m_destroyConn = tdb->attach(boost::bind(onDestroy,this),TaskDatabase::mtEntityDestroyed);
	m_relChange = tdb->attach(boost::bind(onTeamChange,this),TaskDatabase::mtRelationChanged);
	m_teamChangeConn = tdb->attach(boost::bind(onTeamChange,this),TaskDatabase::mtHeroTeamChanged);
}

//=====================================================================================//
//                       UnlimitedMovesMgr::~UnlimitedMovesMgr()                       //
//=====================================================================================//
UnlimitedMovesMgr::~UnlimitedMovesMgr()
{
	assert( g_unlimoves == this );
	g_unlimoves = 0;
}

//=====================================================================================//
//                  UnlimitedMovesMgr *UnlimitedMovesMgr::instance()                   //
//=====================================================================================//
UnlimitedMovesMgr *UnlimitedMovesMgr::instance()
{
	return g_unlimoves;
}

//=====================================================================================//
//                           void UnlimitedMovesMgr::reset()                           //
//=====================================================================================//
void UnlimitedMovesMgr::reset()
{
	m_isGameLost = false;
	update(false);
}

//=====================================================================================//
//                          void UnlimitedMovesMgr::update()                           //
//=====================================================================================//
void UnlimitedMovesMgr::update(bool ae)
{
	bool newHasEnemies = hasEnemies();

	if(m_hasEnemies != newHasEnemies)
	{
		m_hasEnemies = newHasEnemies;
		if(ae)
		{
			if(!m_hasEnemies)
			{
				restoreAll();
			}
			else
			{
				maximizeMovepoints();
				if(m_enabled)
				{
					::GameEvMessenger::GetInst()->Notify(GameObserver::EV_UNLIMITED_MOVES_ENDED);
				}
			}
		}
	}
}

//=====================================================================================//
//                     bool UnlimitedMovesMgr::hasEnemies() const                      //
//=====================================================================================//
bool UnlimitedMovesMgr::hasEnemies() const
{
	EntityPool *ep = ::EntityPool::GetInst();

	for(::EntityPool::iterator i = ep->begin(); i != ep->end(); ++i)
	{
		if(::EnemyDetector::getInst()->isHeEnemy(&*i)) return true;
	}
	return false;
}

//=====================================================================================//
//                          void UnlimitedMovesMgr::onSpawn()                          //
//=====================================================================================//
void UnlimitedMovesMgr::onSpawn()
{
	update();
}

//=====================================================================================//
//                         void UnlimitedMovesMgr::onDestroy()                         //
//=====================================================================================//
void UnlimitedMovesMgr::onDestroy()
{
	update();
}

//=====================================================================================//
//                       void UnlimitedMovesMgr::onTeamChange()                        //
//=====================================================================================//
void UnlimitedMovesMgr::onTeamChange()
{
	update();
}

//=====================================================================================//
//                     void UnlimitedMovesMgr::onRelationChanged()                     //
//=====================================================================================//
void UnlimitedMovesMgr::onRelationChanged()
{
	update();
}

//=====================================================================================//
//                          void UnlimitedMovesMgr::enable()                           //
//=====================================================================================//
void UnlimitedMovesMgr::enable(bool e)
{
	m_enabled = e;
}

//=====================================================================================//
//                        void UnlimitedMovesMgr::restoreAll()                         //
//=====================================================================================//
void UnlimitedMovesMgr::restoreAll()
{
	if(!m_enabled) return;

	::GameEvMessenger::GetInst()->Notify(GameObserver::EV_UNLIMITED_MOVES_STARTED);
	::EntityPool *ep = ::EntityPool::GetInst();

	for(::EntityPool::iterator i = ep->begin(); i != ep->end(); ++i)
	{
		if(HumanEntity *human = i->Cast2Human())
		{
			HumanContext* context = human->GetEntityContext();
			if(context->GetTraits()->GetHealth() == 0) return;

			//подсчитать количество ходов
			context->GetTraits()->SetMovepnts(context->GetLimits()->GetMovepnts());

			//рост морали за ход
			if(context->GetTraits()->GetMorale() < context->GetLimits()->GetMorale())
			{
				context->GetTraits()->AddMorale(context->GetLimits()->GetMorale() - context->GetTraits()->GetMorale());
			}
		    
			//рост точности
			context->GetTraits()->AddAccuracy(context->GetLimits()->GetAccuracy() - context->GetTraits()->GetAccuracy() + context->GetTraits()->GetConstAccuracy());
			context->GetTraits()->AddDexterity(context->GetLimits()->GetDexterity() - context->GetTraits()->GetDexterity());

			context->ClearLongDamages();

			for(int i = 0; i < MAX_BODY_PARTS; ++i)
			{
				context->SetBodyPartDamage((body_parts_type)i,0);
			}

			//запомним текущий уровень здоровья
			int previous_health = context->GetTraits()->GetHealth();

			//учтем прибавок от импланта
			//if(context->GetTraits()->GetConstHealth() > 0)
			{
				context->GetTraits()->AddHealth(context->GetLimits()->GetHealth() - context->GetTraits()->GetHealth());
			}

			//увеличить/уменьшить парамтры здоровья
			if(int delta = context->GetTraits()->GetHealth() - previous_health)
			{
				DisplayHealthIncrease(human, delta);
				ApplyHealthChange(context, delta);
				//ApplyBodyPartsDamage(context, - delta);
			}

			human->GetEntityContext()->SetPanicType(HPT_NONE);

			human->GetGraph()->Relax();
		}
		else if(VehicleEntity *vehicle = i->Cast2Vehicle())
		{
            vehicle->GetEntityContext()->SetMovepnts(vehicle->GetInfo()->GetMovepnts());
		}
	}

	RiskArea::GetInst()->Reset();
}

//=====================================================================================//
//                    void UnlimitedMovesMgr::maximizeMovepoints()                     //
//=====================================================================================//
void UnlimitedMovesMgr::maximizeMovepoints()
{
	if(!m_enabled) return;

	::EntityPool *ep = ::EntityPool::GetInst();

	for(::EntityPool::iterator i = ep->begin(); i != ep->end(); ++i)
	{
		if(HumanEntity *human = i->Cast2Human())
		{
			HumanContext* context = human->GetEntityContext();
			if(context->GetTraits()->GetHealth() == 0) return;

			//подсчитать количество ходов
			context->GetTraits()->SetMovepnts(context->GetLimits()->GetMovepnts());
		}
	}
}

}

//=====================================================================================//
//                                bool UnlimitedMoves()                                //
//=====================================================================================//
bool UnlimitedMoves()
{
	return PoL::UnlimitedMovesMgr::instance()->areMovesUnlimited();
}

//=====================================================================================//
//                                  bool IsGameLost()                                  //
//=====================================================================================//
bool IsGameLost()
{
	return PoL::UnlimitedMovesMgr::instance()->isGameLost();
}

