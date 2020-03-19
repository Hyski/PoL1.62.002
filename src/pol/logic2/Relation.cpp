#include "logicdefs.h"

#include "Marker.h"
#include "Relation.h"
#include "Spectator.h"

#include "Entity.h"
#include <mll/io/debug_ostream.h>

//=====================================================================================//
//                                Relation::Relation()                                 //
//=====================================================================================//
Relation::Relation(Marker *m, Spectator *s)
:	m_marker(m),
	m_spectator(s),
	m_curPlayer(s ? s->GetPlayer() : PT_NONE)
{
}

//=====================================================================================//
//                           void Relation::UpdateRelation()                           //
//=====================================================================================//
void Relation::UpdateRelation(VisUpdateEvent_t ev)
{
	VisParams old_params = m_flags;
	m_flags = DoUpdateRelation(ev);
	player_type old_player = m_curPlayer;
	m_curPlayer = m_spectator ? m_spectator->GetPlayer() : PT_NONE;

	if(old_player != m_curPlayer && m_flags != old_params)
	{
		if(m_flags.IsVisible())
		{
			m_marker->IncPlayerVisibilityCounter(m_curPlayer);
		}
		else
		{
			m_marker->DecPlayerVisibilityCounter(old_player);
		}
	}
	else if(old_player != m_curPlayer)
	{
		if(m_flags.IsVisible())
		{
			m_marker->DecPlayerVisibilityCounter(old_player);
			m_marker->IncPlayerVisibilityCounter(m_curPlayer);
		}
	}
	else if(m_flags != old_params)
	{
		if(m_flags.IsVisible())
		{
			m_marker->IncPlayerVisibilityCounter(m_curPlayer);
		}
		else
		{
			m_marker->DecPlayerVisibilityCounter(m_curPlayer);
		}
	}

	if(old_params != m_flags)
	{
        SpectatorObserver::marker_info mrk_info(m_marker);
        MarkerObserver::spectator_info spr_info(m_spectator);

		if(m_flags.IsVisible())
		{
			m_marker->NotifyMarkerObserver(MarkerObserver::EV_VISIBLE_FOR_SPECTATOR, &spr_info);
			m_spectator->NotifySpectatorObserver(SpectatorObserver::EV_VISIBLE_MARKER, &mrk_info);
		}
		else
		{
			m_marker->NotifyMarkerObserver(MarkerObserver::EV_INVISIBLE_FOR_SPECTATOR, &spr_info);
			m_spectator->NotifySpectatorObserver(SpectatorObserver::EV_INVISIBLE_MARKER, &mrk_info);
		}
	}
}

//=====================================================================================//
//                    VisParams Relation::DoUpdateRelation() const                     //
//=====================================================================================//
VisParams Relation::DoUpdateRelation(VisUpdateEvent_t ev) const
{
	if(ev == UE_DEL_MARKER)
	{
		return VisParams::vpInvisible;
	}

	return VisParams::vpVisible;
}