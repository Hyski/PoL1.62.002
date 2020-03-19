#pragma once

#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                               class UnlimitedMovesMgr                               //
//=====================================================================================//
class UnlimitedMovesMgr
{
	TaskDatabase::Connection m_spawnConn, m_teamChangeConn, m_destroyConn, m_relChange;

	bool m_hasEnemies;
	bool m_enabled;
	bool m_isGameLost;

public:
	UnlimitedMovesMgr();
	~UnlimitedMovesMgr();

	void reset();
	void update(bool applyEffects = true);

	bool areMovesUnlimited() const { return !m_hasEnemies; }
	bool isGameLost() const { return m_isGameLost; }

	void looseGame() { m_isGameLost = true; }

	static UnlimitedMovesMgr *instance();

	void enable(bool);

private:
	bool hasEnemies() const;

	void onSpawn();
	void onDestroy();
	void onTeamChange();
	void onRelationChanged();

	void restoreAll();
	void maximizeMovepoints();
};

}
