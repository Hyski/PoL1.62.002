#include "precomp.h"
#include "GiveExpAct.h"
#include "QuestReporter.h"
#include <logic2/AIUtils.h>

namespace PoL
{

//=====================================================================================//
//                              GiveExpAct::GiveExpAct()                               //
//=====================================================================================//
GiveExpAct::GiveExpAct(int exp)
:	m_started(false),
	m_ended(false),
	m_expAmount(exp)
{
}

//=====================================================================================//
//                              void GiveExpAct::start()                               //
//=====================================================================================//
void GiveExpAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                              void GiveExpAct::update()                              //
//=====================================================================================//
void GiveExpAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	::AIUtils::AddExp4Quest(m_expAmount, PT_PLAYER);
	QuestReporter::experienceAdded(m_expAmount);

	m_ended = true;
}


}