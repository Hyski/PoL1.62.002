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

	/// �������� ������� ��������� ��� ������
	int GetPlayerVisibilityCounter(player_type) const;

	/// ����� �� ������ ��� ������
	bool IsVisibleForPlayer(player_type) const;

	/// �������� ��������� ��� �������
	void ResetVisibility(player_type);

	/// ���������� �������� ������
	void SetCurPlayer(player_type);

	/// ���������� ����������� ����������� (��� ������� �������� � �������)
	void SetInvisible(bool);

	/// ��������/����������
	virtual void MakeSaveLoad(SavSlot &);

	/// "�����" ��� ��������� ������� ������� ������
	void ForceVisible(bool f, player_type p);

	/// �������� ����������� ����� �������
	virtual void GetPoints(pnt3_vec_t &) const = 0;
	/// �������� ��������, �������������� ��������
	virtual BaseEntity *GetEntity() const { return 0; }

	/// ���������� ������
	virtual eid_t GetEID() const { return 0; }

	/// ������ � �������������.
    void DetachMarkerObserver(MarkerObserver *);
    void AttachMarkerObserver(MarkerObserver *, MarkerObserver::event_t event);

private:
	void MakeSave(SavSlot &);
	void MakeLoad6(SavSlot &);

	/// ��������� ����������� �� ��������
	void NotifyMarkerObserver(MarkerObserver::event_t, MarkerObserver::info_t = 0);
	/// ��������� ������� ��������� ��� ������
	void IncPlayerVisibilityCounter(player_type);
	/// ��������� ������� ��������� ��� ������
	void DecPlayerVisibilityCounter(player_type);

	bool IsVisibleForPlayerImp(int) const;

	void Rehighlight(int);

	/// ����������� ��������� �������
	void Highlight(bool);
	virtual void DoHighlight(bool) = 0;
};

/*
#include "MarkerObserver.h"

class BaseEntity;

//=====================================================================================//
//                                    class Marker                                     //
//=====================================================================================//
// ������ - ���������� ������� ��������
class Marker
{
public:

    Marker() : m_mid(0), m_counter(0){}
    virtual ~Marker() {}

    DCTOR_ABS_DEF(Marker)

    //��������� / ��������� �����������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //�������� ������ �� ��������
    virtual BaseEntity* GetEntity() {return 0;}

    //�������� ������ ����� ����������� ������
    virtual void GetPoints(pnt3_vec_t* points) = 0;

    //��������/�������� ������
    void Highlight(bool flag);
    //������� �� ������ 
    virtual bool IsHighlighted() const = 0;

    //�������� ��� ���������� ��������� �������
    void ZeroRequests();
    bool HaveRequests() const;
    void AddRequest(VisParams type);

    //��������/���������� �������������
    void  SetMID(mid_t mid) { m_mid = mid;  }
    mid_t GetMID() const    { return m_mid; }

	virtual eid_t GetEID() const { return 0; }

    //������ � �������������
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