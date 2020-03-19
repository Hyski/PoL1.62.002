#include "precomp.h"

#include "ActMgr.h"
#include "HeroAct.h"
#include <undercover/Game.h>
#include "ActFactory.h"
#include "TaskDatabase.h"
#include <logic2/VisMap.h>
#include <logic2/AIUtils.h>
#include <common/ui/widget.h>
#include <undercover/interface/menumgr.h>
#include <common/timer/timer.h>
#include <logic2/ThingFactory.h>
#include <logic2/GameObserver.h>
#include <undercover/Interface/GameScreen.h>
#include <undercover/interface/loadingscreen.h>
#include <logic2/GameFormImp.h>

#include <boost/format.hpp>

namespace PoL
{

//=====================================================================================//
//                               float getCurrentTime()                                //
//=====================================================================================//
float getCurrentTime()
{
	return ::Timer::GetSeconds();
}

//=====================================================================================//
//                             BaseEntity *getEntityById()                             //
//=====================================================================================//
BaseEntity *getEntityById(::eid_t eid)
{
	return ::EntityPool::GetInst()->Get(eid);
}

//=====================================================================================//
//                           ::BaseEntity *getEntityByName()                           //
//=====================================================================================//
BaseEntity *getEntityByName(const std::string &name)
{
	::EntityPool *epool = ::EntityPool::GetInst();

	for(::EntityPool::iterator i = epool->begin(); i != epool->end(); ++i)
	{
		if(i->GetInfo()->GetRID() == name)
		{
			return &*i;
		}
	}

	return 0;
}

//=====================================================================================//
//                            bool getDeadEntityPosByName()                            //
//=====================================================================================//
bool getDeadEntityPosByName(const std::string &name, point3 *pos)
{
	return TaskDatabase::instance()->getDeadEntityPos(name,pos);
}

//=====================================================================================//
//                                 bool showYesNoBox()                                 //
//=====================================================================================//
bool showYesNoBox(const std::string &message)
{
	const bool mcv = ::MenuMgr::MouseCursor()->IsVisible();

	Widget *gms =::MenuMgr::Instance()->Child( ::GameScreen::m_pDialogName);
	Widget *hms = gms ? gms->Child( ::HiddenMovementScreen::m_pDialogName) : 0;

	bool hiddenvisible = true;
	bool gamelevelrender = ::Game::RenderEnabled();

	::Game::SetGameLevelRender(true);

	if(hms)
	{
		hiddenvisible = hms->IsVisible();
		hms->SetVisible(false);
		::DirtyLinks::EnableLevelAndEffectSound(true);
	}

	if(!mcv) ::MenuMgr::MouseCursor()->SetVisible(true);

	const bool result = Forms::GetInst()->ShowMessageBox(message);

	if(!mcv) ::MenuMgr::MouseCursor()->SetVisible(mcv);

	if(hms)
	{
		hms->SetVisible(hiddenvisible);
		::DirtyLinks::EnableLevelAndEffectSound(!hiddenvisible);
	}

	::Game::SetGameLevelRender(gamelevelrender);

	return result;
}

//=====================================================================================//
//                               void parseThingScript()                               //
//=====================================================================================//
bool parseThingScript(const std::string &rid, ::ThingScriptParser &parser)
{
    int  line;
    std::string script;

    //получим описание скрипта item_set.xls
	if(!::AIUtils::GetItemSetStr(rid, &script, &line))
	{
        std::ostringstream ostr;
        ostr << "parseThingScript: нет набора предметов <" << rid << "> в item_set.xls";
		::DirtyLinks::Print(ostr.str());

		return false;
    }
	else if(!parser.Parse(script))
	{
        std::ostringstream ostr;
        ostr << "parseThingScript: ошибочный набор предметов <" << rid << ">";
		::DirtyLinks::Print(ostr.str());

        ostr.str(""); ostr.clear();
        ostr << "строка в item_set.xls: " << line + 1;
		::DirtyLinks::Print(ostr.str());

        ostr.str(""); ostr.clear();
        ostr << "позиция в строке: " << parser.GetLastPos();
		::DirtyLinks::Print(ostr.str());

		return false;
    }

	return true;
}

//=====================================================================================//
//                            static bool canChangePlayer()                            //
//=====================================================================================//
static bool canChangePlayer(HumanEntity *e, player_type pt)
{
    if(e == 0) return false;

    //если присоединение к команде игрока, проверим в ней кол-во народу
    if(pt == PT_PLAYER)
	{
        int number_of_humans = 0;
		int total_number_of_humans = 0;
		const bool can_dismiss = e->GetInfo()->CanDismiss();

        //посчитаем кол-во народу
        EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
        for(; itor != EntityPool::GetInst()->end(); ++itor)
		{
			++total_number_of_humans;
			if(can_dismiss == itor->Cast2Human()->GetInfo()->CanDismiss())
			{
				++number_of_humans;
			}
        }

		const int human_limit = e->GetInfo()->CanDismiss() ? MAX_TEAMMATES : MAX_NON_DISMISSABLE_TEAMMATES + MAX_TEAMMATES;
		const int total_human_limit = MAX_NON_DISMISSABLE_TEAMMATES + MAX_TEAMMATES;

		//если кол-во народу больше, чем нужно выход
        if(number_of_humans >= human_limit || total_number_of_humans >= total_human_limit)
		{
			std::string msg;
			try
			{
				msg = (boost::format(DirtyLinks::GetStrRes("gqs_can_not_add_hero")) % e->GetInfo()->GetName()).str();
			}
			catch(boost::io::format_error &)
			{
				msg = DirtyLinks::GetStrRes("gqs_can_not_add_hero");
			}

			DirtyLinks::Print(msg);
            return false;
        }
    }

	return true;
}

//=====================================================================================//
//                                 void changePlayer()                                 //
//=====================================================================================//
void changePlayer(HumanEntity *e, player_type pt)
{
	bool needselect = false;

	if(pt == PT_PLAYER)
	{
		//посчитаем кол-во народу
		EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
		needselect = (itor == EntityPool::GetInst()->end());
	}

	::AIUtils::ChangeEntityPlayer(e, pt);

	if(needselect)
	{
		Forms::GetInst()->GetGameForm()->HandleSelectReq(e,GameFormImp::SEL_COMMAND);
	}
}

//=====================================================================================//
//                               void makeChangePlayer()                               //
//=====================================================================================//
void makeChangePlayer(HumanEntity *e, player_type pt, bool nointeract)
{
	ActMgr::start(new HeroAct(e->GetInfo()->GetRID(),pt == PT_PLAYER ? true : false, nointeract));
}

//=====================================================================================//
//                           static bool checkRankForHire()                            //
//=====================================================================================//
static bool checkRankForHire( ::HumanEntity *katrin, ::HumanEntity *he)
{
	if(katrin->GetEntityContext()->GetRank() >= he->GetEntityContext()->GetRank()) return true;
	const std::string fmt = ::DirtyLinks::GetStrRes("gqs_cannot_add_hero_no_rank");
	::DirtyLinks::Print((boost::format(fmt) % he->GetInfo()->GetName()).str());
	return false;
}

//=====================================================================================//
//                                void addHeroToTeam()                                 //
//=====================================================================================//
bool addHeroToTeam(BaseEntity *e, bool nointeract)
{
	if(::HumanEntity *he = e->Cast2Human())
	{
		HumanEntity *katrin = getPlayer();

		if(katrin
			&& ActFactory::instance()->isHeroAvailable(e->GetInfo()->GetRID())
			&& he->GetPlayer() != PT_PLAYER
			&& canChangePlayer(he,PT_PLAYER)
			&& checkRankForHire(katrin,he)
			)
		{
			std::string msg = (boost::format(DirtyLinks::GetStrRes("gqs_add_hero")) % he->GetInfo()->GetName()).str();
			if(!he->GetInfo()->CanDismiss() || (nointeract || showYesNoBox(msg)))
			{
				changePlayer(he,PT_PLAYER);
			}
			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                              bool makeAddHeroToTeam()                               //
//=====================================================================================//
bool makeAddHeroToTeam(BaseEntity *e)
{
	if(::HumanEntity *he = e->Cast2Human())
	{
		HumanEntity *katrin = getPlayer();

		if(katrin
			&& ActFactory::instance()->isHeroAvailable(e->GetInfo()->GetRID())
			&& he->GetPlayer() != PT_PLAYER
			&& canChangePlayer(he,PT_PLAYER)
			&& checkRankForHire(katrin,he)
			)
		{
			std::string msg = (boost::format(DirtyLinks::GetStrRes("gqs_add_hero")) % he->GetInfo()->GetName()).str();
			if(!he->GetInfo()->CanDismiss() || showYesNoBox(msg))
			{
				makeChangePlayer(he,PT_PLAYER,true);
			}
			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                              void removeHeroFromTeam()                              //
//=====================================================================================//
void removeHeroFromTeam(BaseEntity *e, bool nointeract)
{
	if(e)
	{
		if(e->GetPlayer() != PT_PLAYER) return;
		changePlayer(e->Cast2Human(),PT_CIVILIAN);
	}
}

//=====================================================================================//
//                                  int getEpisode()                                   //
//=====================================================================================//
int	getEpisode()
{
	return ::Spawner::GetInst()->GetEpisode();
}

//=====================================================================================//
//                                   int getPhase()                                    //
//=====================================================================================//
int getPhase()
{
	return ::Spawner::GetInst()->GetPhase();
}

//=====================================================================================//
//                            const std::string &getLevel()                            //
//=====================================================================================//
const std::string &getLevel()
{
	return ::DirtyLinks::GetLevelSysName();
}

//=====================================================================================//
//                               void disappearEntity()                                //
//=====================================================================================//
void disappearEntity(BaseEntity *entity)
{
	if(entity)
	{
		::GameObserver::kill_info info(0,entity,::GameObserver::KEV_KILL);
		::GameEvMessenger::GetInst()->Notify(::GameObserver::EV_DISAPPEAR,&info);

		PathUtils::UnlinkEntity(entity);

		EntityContext* context = entity->GetEntityContext();

		VisMap::GetInst()->Remove(context->GetMarker());
		if(context->GetSpectator())
		{
			VisMap::GetInst()->Remove(context->GetSpectator());
		}

		entity->GetGraph()->Destroy();
		EntityPool::GetInst()->Remove(entity);
	}
}

}
