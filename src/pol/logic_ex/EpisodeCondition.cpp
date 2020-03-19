#include "precomp.h"
#include "EpisodeCondition.h"

namespace PoL
{

//=====================================================================================//
//                        EpisodeCondition::EpisodeCondition()                         //
//=====================================================================================//
EpisodeCondition::EpisodeCondition(Id_t id, int episode)
:	Condition(id),
	m_episode(episode)
{
	m_ecConn = TaskDatabase::instance()->attach(boost::bind(onEpisodeChanged,this),TaskDatabase::mtEpisodeChanged);
    updateState();
}

//=====================================================================================//
//                        void EpisodeCondition::setPriority()                         //
//=====================================================================================//
void EpisodeCondition::setPriority(int p)
{
	m_ecConn.setPriority(p);
}

//=====================================================================================//
//                      void EpisodeCondition::onEpisodeChanged()                      //
//=====================================================================================//
void EpisodeCondition::onEpisodeChanged()
{
	updateState();
}

//=====================================================================================//
//                        void EpisodeCondition::updateState()                         //
//=====================================================================================//
void EpisodeCondition::updateState(bool ntf)
{
	setState(getEpisode() == m_episode, ntf);
}

//=====================================================================================//
//                           void EpisodeCondition::reset()                            //
//=====================================================================================//
void EpisodeCondition::reset()
{
	Condition::reset();
	updateState(false);
}


}
