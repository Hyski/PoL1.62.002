#pragma once

#include "VisParams.h"
#include "SpectatorObserver.h"

class Marker;
class Relation;
class BaseEntity;

//=====================================================================================//
//                                   class Spectator                                   //
//=====================================================================================//
class Spectator
{
    typedef ObserverManager<SpectatorObserver> ObsMgr_t;
    ObsMgr_t m_observers;

public:
	Spectator();
	virtual ~Spectator() {}

	DCTOR_ABS_DEF(Spectator)

	virtual void OnInsert() {}
	virtual void OnRemove() {}

	/// «агрузка/сохранение
	virtual void MakeSaveLoad(SavSlot &);

	/// –ассчитать видимость дл€ маркера
	virtual VisParams CalcVisibility(Marker *) = 0;
	/// –ассчитать видимость дл€ точки
	virtual VisParams CalcVisibility(const point3 &) = 0;

	/// —оздать отношение с маркером
	virtual std::auto_ptr<Relation> CreateRelationFor(Marker *) = 0;

	/// ¬озвращает сущность, представленную наблюдателем
	virtual BaseEntity *GetEntity() const { return 0; }
	virtual eid_t GetEID() const { return 0; }

	virtual player_type GetPlayer() const { return PT_NONE; }

	/// –абота с наблюдател€ми
    void DetachSpectatorObserver(SpectatorObserver *);
    void AttachSpectatorObserver(SpectatorObserver *, SpectatorObserver::event_t);
    void NotifySpectatorObserver(SpectatorObserver::event_t, SpectatorObserver::info_t = 0);
};

/*
#error Deprecated

#include "VisDefs.h"
#include "SpectatorObserver.h"

class Marker;
class MSRelation;
class BaseEntity;

//=====================================================================================//
//                                   class Spectator                                   //
//=====================================================================================//
// наблюдатель - абстракци€ сущности - наблюдател€
class Spectator
{
public:

    Spectator() : m_sid(0) {}
    virtual ~Spectator(){}

    DCTOR_ABS_DEF(Spectator)

    //вызываетс€ при присоед./отсоед. наблюдател€
    virtual void OnInsert() {}
    virtual void OnRemove() {}

    //сохранить / загрузить наблюдател€
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //расчитать видимость дл€ маркера
    virtual VisParams CalcVisibility(Marker* marker) = 0;
    //расчитать видимость дл€ точки
    virtual VisParams CalcVisibility(const point3& pnt) = 0;
    //создать св€зь между маркером и наблюдателем
    virtual MSRelation* CreateRelation(Marker* marker) = 0;

    //получить ссылку на существо
    virtual BaseEntity* GetEntity() { return 0; }
    //узнать тип игрока - наблюдател€ (нужно только дл€ камер!)
    virtual player_type GetPlayer() const { return PT_NONE; }

    //получить/установить идентификатор
    void  SetSID(sid_t sid) { m_sid = sid; }
    sid_t GetSID() const { return m_sid; }

    //работа с наблюдател€ми
    void Detach(SpectatorObserver* observer);
    void Attach(SpectatorObserver* observer, SpectatorObserver::event_t event);
    void Notify(SpectatorObserver::event_t event, SpectatorObserver::info_t info = 0);

	virtual std::string DescribeSelf() const = 0;

private:
    sid_t m_sid;

    typedef ObserverManager<SpectatorObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};
*/