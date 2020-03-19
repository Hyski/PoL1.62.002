#include "precomp.h"
#include "ContextCondition.h"

namespace PoL
{

//=====================================================================================//
//                        ContextCondition::ContextCondition()                         //
//=====================================================================================//
ContextCondition::ContextCondition(Id_t id, const std::string &lev, int phase)
:	Condition(id),
	m_level(lev),
	m_phase(phase),
	m_inLevel(false)
{
	if(m_phase != -1)
	{
		m_phaseChangedConn = TaskDatabase::instance()->attach(boost::bind(onPhaseChanged,this),TaskDatabase::mtPhaseChanged);
	}

	m_enterLevelConn = TaskDatabase::instance()->attach(boost::bind(onEnterLevel,this),TaskDatabase::mtEnterLevel);
	m_leaveLevelConn = TaskDatabase::instance()->attach(boost::bind(onLeaveLevel,this),TaskDatabase::mtLeaveLevel);
	update(false);
}

//=====================================================================================//
//                        void ContextCondition::setPriority()                         //
//=====================================================================================//
void ContextCondition::setPriority(int p)
{
	if(m_phase != -1)
	{
		m_phaseChangedConn.setPriority(p);
	}
	m_enterLevelConn.setPriority(p);
	m_leaveLevelConn.setPriority(p);
}

//=====================================================================================//
//                           void ContextCondition::reset()                            //
//=====================================================================================//
void ContextCondition::reset()
{
	Condition::reset();
	m_inLevel = false;
	update(false);
}

//=====================================================================================//
//                       void ContextCondition::onPhaseChanged()                       //
//=====================================================================================//
void ContextCondition::onPhaseChanged()
{
	update();
}

//=====================================================================================//
//                       void ContextCondition::onLevelChanged()                       //
//=====================================================================================//
void ContextCondition::onEnterLevel()
{
	m_inLevel = true;
	update();
}

//=====================================================================================//
//                        void ContextCondition::onLeaveLevel()                        //
//=====================================================================================//
void ContextCondition::onLeaveLevel()
{
	m_inLevel = false;
	update();
}

//=====================================================================================//
//                           void ContextCondition::update()                           //
//=====================================================================================//
void ContextCondition::update(bool ntf)
{
	std::string level = m_inLevel ? getLevel() : std::string();
	setState(m_level==level && (m_phase == -1 || m_phase==getPhase()),ntf);
}

//=====================================================================================//
//                          void ContextCondition::restore()                           //
//=====================================================================================//
void ContextCondition::restore(mll::io::ibinstream &in)
{
	Condition::restore(in);
	m_inLevel = true;
}

}
