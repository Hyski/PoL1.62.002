#include "logicdefs.h"
#include "Spectator.h"

//=====================================================================================//
//                               Spectator::Spectator()                                //
//=====================================================================================//
Spectator::Spectator()
{
}

//=====================================================================================//
//                      void Spectator::AttachSpectatorObserver()                      //
//=====================================================================================//
void Spectator::AttachSpectatorObserver(SpectatorObserver *o, SpectatorObserver::event_t ev)
{
	m_observers.Attach(o,ev);
}

//=====================================================================================//
//                      void Spectator::DetachSpectatorObserver()                      //
//=====================================================================================//
void Spectator::DetachSpectatorObserver(SpectatorObserver *o)
{
	m_observers.Detach(o);
}

//=====================================================================================//
//                      void Spectator::NotifySpectatorObserver()                      //
//=====================================================================================//
void Spectator::NotifySpectatorObserver(SpectatorObserver::event_t ev, SpectatorObserver::info_t info)
{
	m_observers.Notify(this,ev,info);
}

//=====================================================================================//
//                           void Spectator::MakeSaveLoad()                            //
//=====================================================================================//
void Spectator::MakeSaveLoad(SavSlot &slot)
{
}


/*
//=====================================================================================//
//                              void Spectator::Detach()                               //
//=====================================================================================//
void Spectator::Detach(SpectatorObserver* observer)
{
    m_observers.Detach(observer);
}

//=====================================================================================//
//                              void Spectator::Attach()                               //
//=====================================================================================//
void Spectator::Attach(SpectatorObserver* observer, SpectatorObserver::event_t event)
{
    m_observers.Attach(observer, event);
}

//=====================================================================================//
//                              void Spectator::Notify()                               //
//=====================================================================================//
void Spectator::Notify(SpectatorObserver::event_t event, SpectatorObserver::info_t info)
{
    m_observers.Notify(this, event, info);
}
*/