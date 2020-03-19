#include "precomp.h"
#include "AddRankAct.h"

namespace PoL
{

//=====================================================================================//
//                         ResetCounterAct::ResetCounterAct()                          //
//=====================================================================================//
AddRankAct::AddRankAct(const std::string &ctr)
:	m_started(false),
	m_ended(false),
	m_entity(ctr)
{
}

//=====================================================================================//
//                              void AddRankAct::start()                               //
//=====================================================================================//
void AddRankAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                              void AddRankAct::update()                              //
//=====================================================================================//
void AddRankAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	BaseEntity *entity = getEntityByName(m_entity);
	if(entity && entity->Cast2Human()) entity->Cast2Human()->GetEntityContext()->AddRank();

	m_ended = true;
}

}
