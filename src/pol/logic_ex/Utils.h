#pragma once

#include "LocTextMgr.h"

class ThingScriptParser;

namespace PoL
{

float getCurrentTime();
::BaseEntity *getEntityById( ::eid_t );
::BaseEntity *getEntityByName(const std::string &);
bool getDeadEntityPosByName(const std::string &name, point3 *pos);
bool parseThingScript(const std::string &rid, ::ThingScriptParser &);

bool showYesNoBox(const std::string &message);

void changePlayer(HumanEntity *e, ::player_type);
bool addHeroToTeam(BaseEntity *, bool nointeract);
void removeHeroFromTeam(BaseEntity *, bool nointeract);

void makeChangePlayer(HumanEntity *e, ::player_type, bool nointeract);
bool makeAddHeroToTeam(BaseEntity *);

int getPhase();
int getEpisode();
const std::string &getLevel();

void disappearEntity(BaseEntity *);

//=====================================================================================//
//                          inline ::HumanEntity *getPlayer()                          //
//=====================================================================================//
inline ::HumanEntity *getPlayer()
{
	::BaseEntity *katrin = getEntityByName("player");
	return katrin ? katrin->Cast2Human() : 0;
}

//=====================================================================================//
//                             inline std::string locStr()                             //
//=====================================================================================//
inline std::string locStr(const std::string &id)
{
	return LocTextMgr::instance()->getString(id);
}

//=====================================================================================//
//                             inline ::phrase_s locPhr()                              //
//=====================================================================================//
inline ::phrase_s locPhr(const std::string &id)
{
	return LocTextMgr::instance()->getPhrase(id);
}

}