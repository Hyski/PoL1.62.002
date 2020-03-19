#include "precomp.h"
#include "PrintAct.h"

namespace PoL
{

//=====================================================================================//
//                                PrintAct::PrintAct()                                 //
//=====================================================================================//
PrintAct::PrintAct(const std::string &strid)
:	m_started(false),
	m_ended(false),
	m_strid(strid)
{
}

//=====================================================================================//
//                               void PrintAct::start()                                //
//=====================================================================================//
void PrintAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void PrintAct::update()                               //
//=====================================================================================//
void PrintAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(!m_strid.empty())
	{
		if(m_strid[0] == '$')
		{
			m_strid = m_strid.substr(1);
			DirtyLinks::Print(locStr(m_strid));
		}
		else
		{
			DirtyLinks::Print(m_strid);
		}
	}

	m_ended = true;
}

}