#pragma once

#include "VisParams.h"
#include "MarkerObserver.h"

class Relation;
class BaseEntity;

//=====================================================================================//
//                                    class Marker                                     //
//=====================================================================================//
class Marker
{
	friend class Relation;

	int m_counters[MAX_HUMAN_PLAYERS];
	bool m_visible[MAX_HUMAN_PLAYERS];

	int m_curPlayer;
	int m_invisible;

	typedef ObserverManager<MarkerObserver> ObsMgr_t;
	ObsMgr_t m_observers;

public:
	typedef std::vector<point3> pnt3_vec_t;

	Marker();
	virtual ~Marker() {}

	DCTOR_ABS_DEF(Marker)

	/// Получить счетчик видимости для игрока
	int GetPlayerVisibilityCounter(player_type) const;

	/// Видим ли маркер для игрока
	bool IsVisibleForPlayer(player_type) const;

	/// Обновить видимость для игроков
	void ResetVisibility(player_type);

	/// Установить текущего игрока
	void SetCurPlayer(player_type);

	/// Установить безусловную невидимость (при посадке сущности в технику)
	void SetInvisible(bool);

	/// Загрузка/сохранение
	virtual void MakeSaveLoad(SavSlot &);

	/// "Дырка" для поддержки старого формата сейвов
	void ForceVisible(bool f, player_type p);

	/// Получить характерные точки маркера
	virtual void GetPoints(pnt3_vec_t &) const = 0;
	/// Получить сущность, представленную маркером
	virtual BaseEntity *GetEntity() const { return 0; }

	/// Отладочные методы
	virtual eid_t GetEID() const { return 0; }

	/// Работа с наблюдателями.
    void DetachMarkerObserver(MarkerObserver *);
    void AttachMarkerObserver(MarkerObserver *, MarkerObserver::event_t event);

private:
	void MakeSave(SavSlot &);
	void MakeLoad6(SavSlot &);

	/// Уведомить наблюдателя за маркером
	void NotifyMarkerObserver(MarkerObserver::event_t, MarkerObserver::info_t = 0);
	/// Увеличить счетчик видимости для игрока
	void IncPlayerVisibilityCounter(player_type);
	/// Уменьшить счетчик видимости для игрока
	void DecPlayerVisibilityCounter(player_type);

	bool IsVisibleForPlayerImp(int) const;

	void Rehighlight(int);

	/// Переключить видимость маркера
	void Highlight(bool);
	virtual void DoHighlight(bool) = 0;
};

/*
#include "MarkerObserver.h"

class BaseEntity;

//=====================================================================================//
//                                    class Marker                                     //
//=====================================================================================//
// маркер - абстракция видимой сущности
class Marker
{
public:

    Marker() : m_mid(0), m_counter(0){}
    virtual ~Marker() {}

    DCTOR_ABS_DEF(Marker)

    //сохранить / загрузить наблюдателя
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //получить ссылку на существо
    virtual BaseEntity* GetEntity() {return 0;}

    //получить массив точек описывающих маркер
    virtual void GetPoints(pnt3_vec_t* points) = 0;

    //показать/погасить маркер
    void Highlight(bool flag);
    //освещен ли маркер 
    virtual bool IsHighlighted() const = 0;

    //операции для обновления состояния маркера
    void ZeroRequests();
    bool HaveRequests() const;
    void AddRequest(VisParams type);

    //получить/установить идентификатор
    void  SetMID(mid_t mid) { m_mid = mid;  }
    mid_t GetMID() const    { return m_mid; }

	virtual eid_t GetEID() const { return 0; }

    //работа с наблюдателями
    void Detach(MarkerObserver* observer);
    void Attach(MarkerObserver* observer, MarkerObserver::event_t event);
    void Notify(MarkerObserver::event_t event, MarkerObserver::info_t info = 0);

	virtual std::string DescribeSelf() const = 0;

private:
	virtual void DoHighlight(bool) = 0;

    mid_t m_mid;
    int m_counter;

    typedef ObserverManager<MarkerObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};
*/