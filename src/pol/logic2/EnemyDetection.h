/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ����������� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__ENEMY_DETECTION_H__)
#define __ENEMY_DETECTION_H__

#ifndef _PUNCH_GAMEOBSERVER_H_
#include "gameobserver.h"
#endif

class BaseEntity;
class SpawnObserver;
class RelationsObserver;
class RelationsObserversContainer;

//////////////////////////////////////////////////////////////////////////////
//
// �����, ����������� �� ���������� ��������� ����� ����������
//
//////////////////////////////////////////////////////////////////////////////
class RelationsObserver
{
public:
	enum event_type
	{
		ET_RELATION2PLAYER_CHANGED,
		ET_RELATIONS_CHANGED
	};
	struct EntityDesc
	{
		eid_t m_id;
		EntityDesc(eid_t id) : m_id(id) {}
	};
	struct RelationDesc
	{
		std::string m_first_subteam;
		std::string m_second_subteam;
		RelationType m_relation_type;
		bool isFirstPlayer() const { return m_first_subteam == "player"; }
	};
	virtual void changed(event_type et, void* param) = 0;
};


//////////////////////////////////////////////////////////////////////////////
//
// ����� ��� �������� ������������������ ������������ � ����������� ��
//
//////////////////////////////////////////////////////////////////////////////
class RelationsObserversContainer
{
public:
	// ���������������� �����������
	void attach(RelationsObserver* observer, RelationsObserver::event_type type);
	// ����������� �����������
	void detach(RelationsObserver* observer);
	// ����������
	~RelationsObserversContainer();
	// ����������� � ���, ��� ���� ������� ��������� � ������
	void notifyEntityChangedRelationWithPlayer(eid_t entity);
	// ����������� � ���, ��� ���������� ����� ������ ������
	void notifySubteamChangedRelationWithPlayer(const std::string& name);
	// ����������� � ���, ��� ���������� �������� ��������� � ������ ����������
	void notifySubteam1ChangedRelationWithSubteam2(BaseEntity* entity1, BaseEntity* entity2, RelationType type);
	void notifySubteam1ChangedRelationWithSubteam2(const std::string& subteam1, const std::string& subteam2, RelationType type);
	// �������� ������������
	void clear();
private:
	typedef std::set<RelationsObserver*> Observers;
	Observers m_player_observers;
	Observers m_anybody_observers;
};

//////////////////////////////////////////////////////////////////////////////
//
// �����, ��������������� ��������� ��� ����������� �����
//
//////////////////////////////////////////////////////////////////////////////
class EnemyDetector: private GameObserver
{
public:
	// �������� ��������� �� ������������ ������ ������
	static EnemyDetector * getInst();

	// ����������
	~EnemyDetector();

	// ��������� �������� �� ������ �������� ��� ������� �������� ������
	bool isHeEnemy4Me(BaseEntity * me, BaseEntity * he);
	bool isHeEnemy(BaseEntity * he);

	// ������� ��� ��������� ����� ������� � ���������
	RelationType getRelationBetweenPlayerAndHe(BaseEntity * he);

	// ������������������� ����� ����������� ������
	void init();

	// �������� ����������, �������� ������ ��� �������� ������
	void clearCurLevelInfo();

    //����������/�������������
    void MakeSaveLoad(SavSlot& st);

	// ���������������� �����������
	void attach(RelationsObserver* observer, RelationsObserver::event_type type);
	// ����������� �����������
	void detach(RelationsObserver* observer);

    //Punch: ��� ��� �������� � ������� ���������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  	// ����� ����������� � ���, ��� ���� ������� ��������� � ������
	void notifyEntityChangedRelationWithPlayer(eid_t entity)
    { m_observers_container.notifyEntityChangedRelationWithPlayer(entity); }       

	// ������������� �������� ��������� ����� ����� ������������.
	// ����� ������ ���������� 1 ����� ���������� � ���������� 2 ��� � new_relation
	void setRelation(const std::string& subteam1, const std::string& subteam2, RelationType new_relation);

	// �������� ��� ���������� �� �� ���������� �����
	std::string getName(const std::string& sys_name);

	void debugOutRelations();

private:

	RelationsObserversContainer m_observers_container;

	// ��������� �������������� �������� � ����������� ��� �� xls'��
	void loadAlignment();
	// ��������� ��������� � ������� �������� �� ������
    void Update(subject_t subj, event_t event, info_t info);

	// ������� ���������� 2 �� ��������� ������ � ������ ���������� 1
	void clearRelation(const std::string& subteam1, const std::string& subteam2);
	// ������� ���������� 2 ������ ���������� 1
	void setEnemyRelation(const std::string& subteam1, const std::string& subteam2);
	// ������� ���������� 2 ������ ���������� 1
	void setFriendRelation(const std::string& subteam1, const std::string& subteam2);

	// ����� ��������� � �� �������
	typedef std::map<std::string, int> subteam_map_t;
	// ��������� ������� ���������
	typedef std::set<int> subteam_set_t;
	// �����, ������� ������ �� ������ ���������� ��������� ���� ������� �� ������
	typedef std::map<int, subteam_set_t> relation_map_t;
	// �����, ������� ������������ ��������� ��� ������� � �������
	typedef std::map<std::string, std::string> convert_map_t;

	// ��� ����������
	subteam_map_t m_subteams;
	// ��������� ����� ������������
	relation_map_t m_relations;
	// ����� ��������� ������
	relation_map_t m_friends;
	// ������� ����� ��� ��������� ���������� � ����� ������� ���������
	int m_next_subteam_number;

	// �����: ���� - ����� ����������, �������� - ������ ������� ������ ������� (������)
	relation_map_t m_subteam_vs_civilians;
	// �����: ���� - ����� ������� ������, �������� - ������ ������� ��������� (������)
	relation_map_t m_civilian_vs_subteams;
	// �����: ���� - ����� ������� ������, �������� - ������ ������� ������ ������� (������)
	relation_map_t m_civilian_vs_civilians;

	// ����� ��� ����������� ���������� ����� � �������
	convert_map_t m_convert_map;

	// ������ ����������� ����� ����� �� ��� ������� ������ ������ ���
	EnemyDetector();

	// ��������� �� ����� ��� ��������� ��������� � �������� ������ ������
	SpawnObserver* m_loadlevel_observer;
	// ����������� ��������� ��� �������� � �����������
	// ��������, ����������� � ����������� ����������
	struct Container
	{
		// ��������� �� ������������ ������
		EnemyDetector * m_detector;

		~Container()
		{
			delete m_detector;
		}
	};

	static Container m_container;
};

#endif // __ENEMY_DETECTION_H__