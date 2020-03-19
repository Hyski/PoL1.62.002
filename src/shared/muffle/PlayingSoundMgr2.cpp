#include "precomp.h"
#include "Work2.h"
#include "Worker2.h"
#include "PlayingSoundMgr2.h"

namespace Muffle2
{

namespace PlayingSoundMgr2Details
{

//=====================================================================================//
//                        class SoundStateSeeker : public Work                         //
//=====================================================================================//
class SoundStateSeeker : public Work
{
	WHPlayingSoundMgr m_mgr;

public:
	SoundStateSeeker(WHPlayingSoundMgr mgr) : m_mgr(mgr) {}

	/// Возвращает true, если объект уже не актуален
	virtual bool expired() const
	{
		return m_mgr.expired();
	}

	/// Выполнить действие
	virtual void execute()
	{
		if(HPlayingSoundMgr lockedMgr = m_mgr.acquire())
		{
			lockedMgr->checkSoundStates();
		}
	}
};

}

using namespace PlayingSoundMgr2Details;

static WHPlayingSoundMgr g_inst;

//=====================================================================================//
//                         PlayingSoundMgr::PlayingSoundMgr()                          //
//=====================================================================================//
PlayingSoundMgr::PlayingSoundMgr()
{
	g_inst.reset(this);
	m_soundStateSeeker = Ptr<SoundStateSeeker>::create(this);
	Worker::inst()->addWork(m_soundStateSeeker,0);
}

//=====================================================================================//
//                         PlayingSoundMgr::~PlayingSoundMgr()                         //
//=====================================================================================//
PlayingSoundMgr::~PlayingSoundMgr()
{
	g_inst.reset();

	stopAllSounds();
}

//=====================================================================================//
//                        void PlayingSoundMgr::stopAllSounds()                        //
//=====================================================================================//
void PlayingSoundMgr::stopAllSounds()
{
	MUFFLE_SYNCHRONIZED_THIS;

	for(Sounds_t::iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		(*i)->badStop();
	}

	while(!isEmpty())
	{
		checkSoundStates();
		Sleep(20);
	}
}

//=====================================================================================//
//                        bool PlayingSoundMgr::isEmpty() const                        //
//=====================================================================================//
bool PlayingSoundMgr::isEmpty() const
{
	MUFFLE_SYNCHRONIZED_THIS;
	return m_sounds.empty();
}

//=====================================================================================//
//                size_t PlayingSoundMgr::getPlayingSoundCount() const                 //
//=====================================================================================//
size_t PlayingSoundMgr::getPlayingSoundCount() const
{
	MUFFLE_SYNCHRONIZED_THIS;
	return m_sounds.size();
}

//=====================================================================================//
//                          void PlayingSoundMgr::addSound()                           //
//=====================================================================================//
void PlayingSoundMgr::addSound(HPlayingSound snd)
{
	MUFFLE_SYNCHRONIZED_THIS;
	m_sounds.push_back(snd);
}

//=====================================================================================//
//                      void PlayingSoundMgr::checkSoundStates()                       //
//=====================================================================================//
void PlayingSoundMgr::checkSoundStates()
{
	MUFFLE_SYNCHRONIZED_THIS;

	for(Sounds_t::iterator i = m_sounds.begin(); i != m_sounds.end();)
	{
		if(!(*i)->isPlaying())
		{
			i = m_sounds.erase(i);
		}
		else
		{
			(*i)->decode();
			++i;
		}
	}
}

//=====================================================================================//
//                      HPlayingSoundMgr PlayingSoundMgr::inst()                       //
//=====================================================================================//
HPlayingSoundMgr PlayingSoundMgr::inst()
{
	return g_inst.acquire();
}

}
