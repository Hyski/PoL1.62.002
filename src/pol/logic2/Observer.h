//
//  шаблонный класс на наблюдателей
//

#ifndef _PUNCH_OBSERVER_H_
#define _PUNCH_OBSERVER_H_

#pragma warning(disable : 4503)

typedef void* void_ptr;

//
// базовый класс самого наблюдателя
//
template<class _Subject, class _Event = int, class _Info = void_ptr>
class Observer{
public:

    virtual ~Observer(){}

    //тип информации
    typedef _Info info_t;
    //тип события
    typedef _Event event_t;
    //тип наблюдаемого объекта
    typedef _Subject subject_t;

    //функция для уведомления наблюдателя
    virtual void Update(subject_t subj, event_t event, info_t info) = 0;
};

//=====================================================================================//
//                                class ObserverManager                                //
//=====================================================================================//
// сервер для подписки и рассылки событий
template<class _Observer>
class ObserverManager
{
public:
    //тип наблюдателя
    typedef _Observer observer_t;

    //тип пересылаемой информации о событии
    typedef typename observer_t::info_t info_t;
    //тип события
    typedef typename observer_t::event_t event_t;
    //тип наблюдаемого объекта
    typedef typename observer_t::subject_t subject_t;

public:
	ObserverManager() : m_inNotify(0) {}

    //сняться с подписки
    void Detach(observer_t* observer)
    {
		if(m_inNotify > 0)
		{
			std::transform(m_observers.begin(),m_observers.end(),m_observers.begin(), ZeroObs(observer));
		}
		else
		{
			m_observers.erase(std::remove_if(m_observers.begin(),m_observers.end(),ObsEq(observer)),m_observers.end());
		}
    }

    //подписаться на событие
    void Attach(observer_t* observer, event_t event)
    {
		obs_vec_t::iterator i = std::upper_bound(m_observers.begin(),m_observers.end(),event,EventLess());
		m_observers.insert(i,obs_s(observer,event));
    }

    //отправить уведомление
    void Notify(subject_t subj, event_t event, info_t info)
    {
		std::pair<obs_vec_t::iterator,obs_vec_t::iterator> range;
		range = std::equal_range(m_observers.begin(),m_observers.end(),event,EventLess());

		{
			KeepInNotify keeper(m_inNotify);

			for(size_t count = std::distance(range.first,range.second); count; --count, ++range.first)
			{
				if(range.first->m_observer)
				{
					range.first->m_observer->Update(subj,event,info);
				}
			}
		}

		Cleanup();
    }

    bool isAttached(observer_t *subj)
    {
		return std::find_if(m_observers.begin(),m_observers.end(),ObsEq(subj)) != m_observers.end();
    }

private:
	void Cleanup()
	{
		if(m_inNotify == 0)
		{
			Detach(0);
		}
	}

	friend struct ObsEq;
	friend struct EventLess;

    struct obs_s
	{
        event_t     m_event;
        observer_t* m_observer;

        obs_s(observer_t* obs, event_t event) : m_observer(obs), m_event(event) {} 
    };

	struct EventLess
	{
		bool operator()(const obs_s &l, const obs_s &r) const
		{
			return l.m_event < r.m_event;
		}
		bool operator()(event_t l, const obs_s &r) const
		{
			return l < r.m_event;
		}
		bool operator()(const obs_s &l, event_t r) const
		{
			return l.m_event < r;
		}
	};

	struct ObsEq
	{
		observer_t *m_obs;
		ObsEq(observer_t *obs) : m_obs(obs) {}
		bool operator()(const obs_s &s) const { return s.m_observer == m_obs; }
	};

	struct ZeroObs
	{
		observer_t *m_obs;
		ZeroObs(observer_t *o) : m_obs(o) {}
		obs_s operator()(const obs_s &o) const
		{
			obs_s result(o.m_observer==m_obs?0:o.m_observer,o.m_event);
			return result;
		}
	};

	int m_inNotify;

	struct KeepInNotify
	{
		int &m_p;

		KeepInNotify(int &p) : m_p(p) { ++m_p; }
		~KeepInNotify() { --m_p; }
	};

    typedef std::list<obs_s> obs_vec_t;
    obs_vec_t m_observers;
};

#endif // _PUNCH_OBSERVER_H_
