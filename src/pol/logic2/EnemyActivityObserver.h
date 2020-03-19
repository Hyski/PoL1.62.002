#pragma once

#include "Activity.h"

//=====================================================================================//
//                             class EnemyActivityObserver                             //
//=====================================================================================//
/// класс - наблюдатель за активностью существа
class EnemyActivityObserver : public ActivityObserver
{
public:
	EnemyActivityObserver() : m_last_event(EV_NONE) {}

	//обработать уведомление о событии
	virtual void Update(Activity* subj, event_t event, info_t info)
	{
		setLastEvent(static_cast<event_type>(event));
	}

	event_type getLastEvent() { return m_last_event; }
	void setLastEvent(event_type event) { m_last_event = event; }
	void clear() { m_last_event = EV_NONE; }

private:
	event_type m_last_event;
};
