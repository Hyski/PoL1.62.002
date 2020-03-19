#pragma once

#include "Condition.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                      class EpisodeCondition : public Condition                      //
//=====================================================================================//
class EpisodeCondition : public Condition
{
	int m_episode;
	TaskDatabase::Connection m_ecConn;

public:
	EpisodeCondition(Id_t id, int episode);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stEpisodeCond; }

	/// Установить приоритет
	virtual void setPriority(int);

	virtual void reset();

private:
	void updateState(bool ntf = true);
	void onEpisodeChanged();
};

}
