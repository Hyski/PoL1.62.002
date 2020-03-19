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

	/// ��������/����������
	virtual void MakeSaveLoad(SavSlot &);

	/// ���������� ��������� ��� �������
	virtual VisParams CalcVisibility(Marker *) = 0;
	/// ���������� ��������� ��� �����
	virtual VisParams CalcVisibility(const point3 &) = 0;

	/// ������� ��������� � ��������
	virtual std::auto_ptr<Relation> CreateRelationFor(Marker *) = 0;

	/// ���������� ��������, �������������� ������������
	virtual BaseEntity *GetEntity() const { return 0; }
	virtual eid_t GetEID() const { return 0; }

	virtual player_type GetPlayer() const { return PT_NONE; }

	/// ������ � �������������
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
// ����������� - ���������� �������� - �����������
class Spectator
{
public:

    Spectator() : m_sid(0) {}
    virtual ~Spectator(){}

    DCTOR_ABS_DEF(Spectator)

    //���������� ��� �������./������. �����������
    virtual void OnInsert() {}
    virtual void OnRemove() {}

    //��������� / ��������� �����������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //��������� ��������� ��� �������
    virtual VisParams CalcVisibility(Marker* marker) = 0;
    //��������� ��������� ��� �����
    virtual VisParams CalcVisibility(const point3& pnt) = 0;
    //������� ����� ����� �������� � ������������
    virtual MSRelation* CreateRelation(Marker* marker) = 0;

    //�������� ������ �� ��������
    virtual BaseEntity* GetEntity() { return 0; }
    //������ ��� ������ - ����������� (����� ������ ��� �����!)
    virtual player_type GetPlayer() const { return PT_NONE; }

    //��������/���������� �������������
    void  SetSID(sid_t sid) { m_sid = sid; }
    sid_t GetSID() const { return m_sid; }

    //������ � �������������
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