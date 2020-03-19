#include "precomp.h"
#include "AddNewsAct.h"
#include <Logic2/AIUtils.h>

namespace PoL
{

//=====================================================================================//
//                              AddNewsAct::AddNewsAct()                               //
//=====================================================================================//
AddNewsAct::AddNewsAct(const std::string &newsid)
:	m_newsid(newsid),
	m_started(false),
	m_ended(false)
{
}

//=====================================================================================//
//                              void AddNewsAct::start()                               //
//=====================================================================================//
void AddNewsAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                              void AddNewsAct::update()                              //
//=====================================================================================//
void AddNewsAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(!m_newsid.empty())
	{
		::NewsPool::GetInst()->Push(m_newsid);
	}

	m_ended = true;
}

}
