#include "precomp.h"
#include "GiveMoneyAct.h"
#include "QuestReporter.h"
#include <logic2/AIUtils.h>

namespace PoL
{

//=====================================================================================//
//                            GiveMoneyAct::GiveMoneyAct()                             //
//=====================================================================================//
GiveMoneyAct::GiveMoneyAct(int money)
:	m_started(false),
	m_ended(false),
	m_moneyAmount(money)
{
}

//=====================================================================================//
//                             void GiveMoneyAct::start()                              //
//=====================================================================================//
void GiveMoneyAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                             void GiveMoneyAct::update()                             //
//=====================================================================================//
void GiveMoneyAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	::AIUtils::GiveMoneyToPlayer(m_moneyAmount);
	QuestReporter::moneyAdded(m_moneyAmount);

	m_ended = true;
}


}