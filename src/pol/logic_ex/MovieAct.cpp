#include "precomp.h"
#include "MovieAct.h"
#include <logic2/SndUtils.h>
#include <undercover/Interface/Interface.h>

namespace PoL
{

//=====================================================================================//
//                                MovieAct::MovieAct()                                 //
//=====================================================================================//
MovieAct::MovieAct(const std::string &bik, const std::string &levl)
:	m_bik(bik + ".bik"),
	m_levl(levl),
	m_started(false),
	m_ended(false),
	m_firstTime(true)
{
}

//=====================================================================================//
//                            void CameraFocusAct::start()                             //
//=====================================================================================//
void MovieAct::start()
{
	assert( !isStarted() );
	assert( !isEnded() );

	m_started = true;
}

//=====================================================================================//
//                            void CameraFocusAct::update()                            //
//=====================================================================================//
void MovieAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(m_firstTime)
	{
		SndUtils::EmptySoundSession empty_sound;
		std::string oldlev = ::DirtyLinks::GetLevelSysName();
		::Spawner::GetInst()->Notify( ::SpawnObserver::ET_EXIT_LEVEL, (SpawnObserver::info_t)&m_levl);
		::DirtyLinks::LoadLevel(std::string());

		Interface::Instance()->PlayBink(m_bik.c_str());

		m_firstTime = false;
		if(!m_levl.empty())
		{
			::Spawner::GetInst()->ExitLevel(m_levl,oldlev);
		}
		else
		{
			m_ended = true;
		}
	}
	else
	{
		m_ended = true;
	}
}


}
