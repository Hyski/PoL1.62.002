#pragma once

#include "Entity.h"
#include "Spectator.h"
#include "GameObserver.h"

//=====================================================================================//
//                              class PlayerTeamSpectator                              //
//=====================================================================================//
class PlayerTeamSpectator
:	public Spectator,
	private GameObserver,
	private EntityObserver
{
	bool m_turnOn;

public:
	enum
	{
		F_TURN_ON  = 1 << 0,
	};

	PlayerTeamSpectator();
	~PlayerTeamSpectator();

	DCTOR_DEF(PlayerTeamSpectator)

	void MakeSaveLoad(SavSlot& slot);

	VisParams CalcVisibility(Marker* marker);
	VisParams CalcVisibility(const point3& pnt);

	std::auto_ptr<Relation> CreateRelationFor(Marker* marker);

	player_type GetPlayer() const { return PT_PLAYER; }

private:
	void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);
	void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info);

	void AttachObserver();
	void DetachObserver();
};

