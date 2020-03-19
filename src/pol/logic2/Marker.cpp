#include "logicdefs.h"

#include "Marker.h"
#include "VisMap.h"

//=====================================================================================//
//                            static int PlayerType2Index()                            //
//=====================================================================================//
static int PlayerType2Index(player_type pt)
{
	switch(pt)
	{
		case PT_PLAYER1: return 1;
		case PT_PLAYER2: return 2;
		case PT_PLAYER3: return 3;
		case PT_PLAYER4: return 4;
		case PT_PLAYER5: return 5;
	}
	return 0;
}

//=====================================================================================//
//                                  Marker::Marker()                                   //
//=====================================================================================//
Marker::Marker()
:	m_curPlayer(PlayerType2Index(PT_PLAYER1)-1),
	m_invisible(0)
{
	std::fill_n(m_counters, (int)MAX_HUMAN_PLAYERS, 0);
	std::fill_n(m_visible, (int)MAX_HUMAN_PLAYERS, false);
}

//=====================================================================================//
//                   int Marker::GetPlayerVisibilityCounter() const                    //
//=====================================================================================//
int Marker::GetPlayerVisibilityCounter(player_type pt) const
{
	if(int index = PlayerType2Index(pt))
	{
		return m_counters[index-1];
	}
	else
	{
		return 0;
	}
}

//=====================================================================================//
//                       bool Marker::IsVisibleForPlayer() const                       //
//=====================================================================================//
bool Marker::IsVisibleForPlayer(player_type pt) const
{
	if(int index = PlayerType2Index(pt))
	{
		return IsVisibleForPlayerImp(index-1);
	}
	else
	{
		return false;
	}
}

//=====================================================================================//
//                     bool Marker::IsVisibleForPlayerImp() const                      //
//=====================================================================================//
bool Marker::IsVisibleForPlayerImp(int i) const
{
	return m_visible[i] && !m_invisible;
}

//=====================================================================================//
//                           void Marker::ResetVisibility()                            //
//=====================================================================================//
void Marker::ResetVisibility(player_type pt)
{
	if(!PlayerType2Index(pt)) return;

	int pidx = PlayerType2Index(pt)-1;
	const bool new_vis = (m_counters[pidx] != 0) && !m_invisible;

	if(IsVisibleForPlayerImp(pidx) != new_vis)
	{
		m_visible[pidx] = m_counters[pidx] != 0;
		if(pidx == m_curPlayer)
		{
			Highlight(new_vis);
		}
	}
}

//=====================================================================================//
//                             void Marker::SetCurPlayer()                             //
//=====================================================================================//
void Marker::SetCurPlayer(player_type p)
{
	assert( PlayerType2Index(p) );
	int newPlayer = PlayerType2Index(p)-1;

	if(m_curPlayer != newPlayer)
	{
		std::swap(m_curPlayer,newPlayer);
		Rehighlight(newPlayer);
	}
}

//=====================================================================================//
//                             void Marker::Rehighlight()                              //
//=====================================================================================//
void Marker::Rehighlight(int op)
{
	const bool hlto = m_visible[op];
	if(m_visible[op] != m_visible[m_curPlayer])
	{
		Highlight(m_visible[m_curPlayer]);
	}
}

//=====================================================================================//
//                             void Marker::MakeSaveLoad()                             //
//=====================================================================================//
void Marker::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		MakeSave(slot);
	}
	else
	{
		std::fill_n(m_counters, (int)MAX_HUMAN_PLAYERS, 0);
		std::fill_n(m_visible, (int)MAX_HUMAN_PLAYERS, false);
		if(slot.GetStore()->GetVersion() >= 6)
		{
			MakeLoad6(slot);
		}
	}
}

//=====================================================================================//
//                               void Marker::MakeSave()                               //
//=====================================================================================//
void Marker::MakeSave(SavSlot &slot)
{
	slot << m_curPlayer << m_invisible;
	//slot.Save(m_counters,sizeof(m_counters[0])*MAX_HUMAN_PLAYERS);
	slot.Save(m_visible,sizeof(m_visible[0])*MAX_HUMAN_PLAYERS);
}

//=====================================================================================//
//                              void Marker::MakeLoad6()                               //
//=====================================================================================//
void Marker::MakeLoad6(SavSlot &slot)
{
	slot >> m_curPlayer >> m_invisible;
	//slot.Load(m_counters,sizeof(m_counters[0])*MAX_HUMAN_PLAYERS);
	slot.Load(m_visible,sizeof(m_visible[0])*MAX_HUMAN_PLAYERS);
}

//=====================================================================================//
//                      void Marker::IncPlayerVisibilityCounter()                      //
//=====================================================================================//
void Marker::IncPlayerVisibilityCounter(player_type pt)
{
	if(int index = PlayerType2Index(pt))
	{
		const bool oldvis = IsVisibleForPlayerImp(index-1);

		++m_counters[index-1];
		m_visible[index-1] = true;

		if((m_curPlayer == index-1) && (oldvis != IsVisibleForPlayerImp(index-1)))
		{
			Highlight(!oldvis);
		}
	}
}

//=====================================================================================//
//                      void Marker::DecPlayerVisibilityCounter()                      //
//=====================================================================================//
void Marker::DecPlayerVisibilityCounter(player_type pt)
{
	if(int index = PlayerType2Index(pt))
	{
		--m_counters[index-1];
		assert( m_counters[index-1] >= 0 );
	}
}

//=====================================================================================//
//                         void Marker::AttachMarkerObserver()                         //
//=====================================================================================//
void Marker::AttachMarkerObserver(MarkerObserver *obs, MarkerObserver::event_t ev)
{
	m_observers.Attach(obs,ev);
}

//=====================================================================================//
//                         void Marker::DetachMarkerObserver()                         //
//=====================================================================================//
void Marker::DetachMarkerObserver(MarkerObserver *obs)
{
	m_observers.Detach(obs);
}

//=====================================================================================//
//                         void Marker::NotifyMarkerObserver()                         //
//=====================================================================================//
void Marker::NotifyMarkerObserver(MarkerObserver::event_t ev, MarkerObserver::info_t info)
{
	m_observers.Notify(this,ev,info);
}

//=====================================================================================//
//                             void Marker::SetInvisible()                             //
//=====================================================================================//
void Marker::SetInvisible(bool v)
{
	const bool oldvis = IsVisibleForPlayerImp(m_curPlayer);
	m_invisible += v ? 1 : -1;
	m_invisible = std::max(0,m_invisible);

	if(oldvis != IsVisibleForPlayerImp(m_curPlayer))
	{
		Highlight(!oldvis);
	}
}

//=====================================================================================//
//                             void Marker::ForceVisible()                             //
//=====================================================================================//
void Marker::ForceVisible(bool f, player_type p)
{
	if(!PlayerType2Index(p)) return;

	assert( !m_invisible );
	if(m_visible[PlayerType2Index(p)-1] != f)
	{
		m_visible[PlayerType2Index(p)-1] = f;
		Highlight(IsVisibleForPlayer(p));
	}
}

//=====================================================================================//
//                              void Marker::Highlight()                               //
//=====================================================================================//
void Marker::Highlight(bool v)
{
	DoHighlight(v);
	VisMap::GetInst()->UpdateMarker(this,v?UE_MARKER_ON:UE_MARKER_OFF);
}

/*
#include "VisUtils3.h"

//=====================================================================================//
//                                void Marker::Detach()                                //
//=====================================================================================//
void Marker::Detach(MarkerObserver* observer)
{
    m_observers.Detach(observer);
}

//=====================================================================================//
//                                void Marker::Attach()                                //
//=====================================================================================//
void Marker::Attach(MarkerObserver* observer, MarkerObserver::event_t event)
{
    m_observers.Attach(observer, event);
}

//=====================================================================================//
//                                void Marker::Notify()                                //
//=====================================================================================//
void Marker::Notify(MarkerObserver::event_t event, MarkerObserver::info_t info)
{
    m_observers.Notify(this, event, info);
}

//=====================================================================================//
//                             void Marker::ZeroRequests()                             //
//=====================================================================================//
void Marker::ZeroRequests()
{ 
    m_counter = 0;
}

//=====================================================================================//
//                          bool Marker::HaveRequests() const                          //
//=====================================================================================//
bool Marker::HaveRequests() const 
{
    return m_counter > 0;
}

//=====================================================================================//
//                              void Marker::AddRequest()                              //
//=====================================================================================//
void Marker::AddRequest(VisParams vis)
{
	const player_type CurPlayer = PT_PLAYER;

    if(IsHighlighted())
	{
        if(vis & VisParams::VT_INVISIBLE) m_counter++;
    }
	else
	{
        if(vis.VisibleForPlayer(CurPlayer)) m_counter++;
    }
}

//=====================================================================================//
//                              void Marker::Highlight()                               //
//=====================================================================================//
void Marker::Highlight(bool flag)
{
    if(m_mid)
	{
		VisMap::GetInst()->UpdateMarker(this, flag ? VisMap::UE_MARKER_ON : VisMap::UE_MARKER_OFF);
	}
	DoHighlight(flag);
}
*/