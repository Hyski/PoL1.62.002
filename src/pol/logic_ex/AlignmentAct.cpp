#include "precomp.h"
#include "AlignmentAct.h"
#include <Logic2/EnemyDetection.h>

namespace PoL
{

//=====================================================================================//
//                            AlignmentAct::AlignmentAct()                             //
//=====================================================================================//
AlignmentAct::AlignmentAct(const std::string &team1, const std::string &team2,
						   const std::string &al)
:	m_started(false),
	m_ended(false),
	m_team1(team1),
	m_team2(team2)
{
	if(al == "friends" || al == "friend" || al == "friendly")
	{
		m_requiredRelation = ::RT_FRIEND;
	}
	else if(al == "enemy" || al == "enemies" || al == "hostile")
	{
		m_requiredRelation = ::RT_ENEMY;
	}
	else if(al == "neutral" || al == "neutrals" || al == "neutrally")
	{
		m_requiredRelation = ::RT_NEUTRAL;
	}
}

//=====================================================================================//
//                             void AlignmentAct::start()                              //
//=====================================================================================//
void AlignmentAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                             void AlignmentAct::update()                             //
//=====================================================================================//
void AlignmentAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	::EnemyDetector::getInst()->setRelation(m_team1, m_team2, m_requiredRelation);
	::EnemyDetector::getInst()->setRelation(m_team2, m_team1, m_requiredRelation);
	
	m_ended = true;
}


}

