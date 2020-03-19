//
// интерфейс на действия
//

#ifndef _PUNCH_ACTIVITY_H_
#define _PUNCH_ACTIVITY_H_

class Activity;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

#include "ActivityCommand.h"
#include "ActivityFactory.h"

//
// наблюдатель за действием
//
class ActivityObserver : public Observer<Activity*>{
public:

    enum event_type{
        EV_NONE, 

        EV_NO_LOF,            //нет линнии стрельбы
        EV_MEET_ENTITY,       //существо наткнулось на кого-то  
        EV_DANGER_ZONE,       //существо зашло в опасную зону
        EV_ENEMY_REACTED,     //на существо среагировал враг 
        EV_ENEMY_SPOTTED,     //остановка замечен враг
        EV_STEPS_EXPIRIED,    //у существа нет больше шагов (м.б. перегруз человека)
        EV_MOVEPNTS_EXPIRIED, //у существа кончились ходы
    };

    //информация о замеченном враге
    struct enemy_spotted_s{
        BaseEntity* m_enemy;
        enemy_spotted_s(BaseEntity* entity) : m_enemy(entity) {}
    };

    //информация о прореаг. противнике
    struct enemy_reacted_s{
        BaseEntity* m_enemy;
        enemy_reacted_s(BaseEntity* entity) : m_enemy(entity) {}
    };

    //информация о существе на кот наткнулись
    struct meet_entity_s{
        BaseEntity* m_entity;
        meet_entity_s(BaseEntity* entity) : m_entity(entity) {}
    };
};

//=====================================================================================//
//                                   class Activity                                    //
//=====================================================================================//
// базовое действие
class Activity
{
public:
    virtual ~Activity(){}

    //проигрывание действия нотация как while(act.Run())
    virtual bool Run(activity_command cmd) = 0;

    //работа с наблюдателями за действием
    void Detach(ActivityObserver* observer);
    void Attach(ActivityObserver* observer, ActivityObserver::event_t type);
    void Notify(ActivityObserver::event_t event, ActivityObserver::info_t info = 0);

private:

    typedef ObserverManager<ActivityObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};

//=====================================================================================//
//                                class ActivityHolder                                 //
//=====================================================================================//
class ActivityHolder
{
	std::auto_ptr<Activity> m_activity;

	typedef void (ActivityHolder::*SafeBool_t)(Activity *);

public:
	ActivityHolder() {}
	ActivityHolder(Activity *a) : m_activity(a) {}

	void Reset(Activity *a = 0)
	{
		m_activity.reset(a);
	}

	bool Run(activity_command cmd, ActivityObserver *obs = 0)
	{
		if(m_activity.get())
		{
			if(!m_activity->Run(cmd))
			{
				if(obs) m_activity->Detach(obs);
				Reset();
			}
			return true;
		}
		return false;
	}

	ActivityHolder &operator=(ActivityHolder &holder) { m_activity = holder.m_activity; return *this; }

	operator SafeBool_t() const { return m_activity.get() ? Reset : 0; }
	bool operator!() const { return m_activity.get() == 0; }

	Activity *operator->() { return m_activity.get(); }
	const Activity *operator->() const { return m_activity.get(); }
};

#endif // _PUNCH_ACTIVITY_H_