#pragma once

#include "QuestRef.h"
#include "Persistent.h"
#include "ActTemplate.h"

namespace PoL
{

class ActWorkshop;

//=====================================================================================//
//                                class ActAttachQueue                                 //
//=====================================================================================//
class ActAttachQueue
{
	struct ActAttach
	{
		HActTemplate m_at;
		std::string m_name;
		ActTemplate::Iterator_t m_itor;
	};

	friend class ActFactory;

	typedef std::list<ActAttach> ActAttachQueue_t;
	ActAttachQueue_t m_actAttachQueue;

public:
	ActAttachQueue();
	~ActAttachQueue();

	void queueActAttach(HActTemplate at, const std::string &n, ActTemplate::Iterator_t it);

	static ActAttachQueue *instance();
};

//=====================================================================================//
//                                  class ActFactory                                   //
//=====================================================================================//
class ActFactory
{
	typedef std::hash_map<std::string,Persistent::Id_t> NameMap_t;
	typedef std::vector<std::string> IdMap_t;
	typedef std::vector<HPersistent> Persists_t;
	typedef std::map<std::string,HQuestRef> QuestRefs_t;
	typedef std::hash_map<std::string,int> HeroList_t;

	Persistent::Id_t m_uniqueCtr;
	NameMap_t m_questMap, m_conditionMap, m_triggerMap, m_taskMap, m_counterMap;
	IdMap_t m_idMap;

	Persists_t m_persists;
	QuestRefs_t m_questRefs;

	HeroList_t m_availHeroes;

public:
	/// ������� ���������� ����� �� �����
	HAct create(const std::string &);
	HActTemplate getActTemplate(const std::string &);

	/// �������� ������ ����
	void registerActTemplate(const std::string &, HActTemplate);
	/// �������� �����
	void registerQuest(HQuest);
	/// �������� ������
	void registerTask(HTask);
	/// �������� �������
	void registerCounter(HCounter);
	/// �������� �������
	void registerCondition(HCondition);
	/// �������� �������
	void registerTrigger(HTrigger);

	/// �������� ��������� ������������� ��� �����
	Persistent::Id_t reserveQuestId(const std::string &name);
	Persistent::Id_t reserveTaskId(const std::string &name);
	Persistent::Id_t reserveConditionId(const std::string &name);
	Persistent::Id_t reserveTriggerId(const std::string &name);
	Persistent::Id_t reserveCounterId(const std::string &name);

	bool isQuestNameReserved(const std::string &name) const { return m_questMap.find(name) != m_questMap.end(); }
	bool isTaskNameReserved(const std::string &name) const { return m_taskMap.find(name) != m_taskMap.end(); }
	bool isConditionNameReserved(const std::string &name) const { return m_conditionMap.find(name) != m_conditionMap.end(); }
	bool isTriggerNameReserved(const std::string &name) const { return m_triggerMap.find(name) != m_triggerMap.end(); }
	bool isCounterNameReserved(const std::string &name) const { return m_counterMap.find(name) != m_counterMap.end(); }

	bool hasActiveQuests(const std::string &) const;

	/// �������� ����� � ������ ���������
	void addAvailableHero(const std::string &);
	/// ������ ����� �� ������ ���������
	void removeAvailableHero(const std::string &);

	/// ����������, �������� �� �����
	bool isHeroAvailable(const std::string &) const;

	/// ���������� ��� ������� �� ��� ��������������
	const std::string &getNameFromId(Persistent::Id_t id) const;

	/// ���������� ����� �� ��� �����
	HQuest getQuestByName(const std::string &) const;
	/// ���������� ������ �� ����� �� ��� �����
	HQuestRef getQuestRefByName(const std::string &);
	/// ���������� ����� �� ��� �����
	HCounter getCounterByName(const std::string &) const;
	/// ���������� ������� �� ��� �����
	HCondition getConditionByName(const std::string &) const;
	/// ���������� ������� �� ��� �����
	HTrigger getTriggerByName(const std::string &) const;
	/// ���������� ���� �� ��� �����
	HTask getTaskByName(const std::string &) const;

	void store(mll::io::obinstream &) const;
	void restore(mll::io::ibinstream &, unsigned int);
	void reset();

	void dumpState();
	void dumpScriptParameters();

	/// ���������� ��������� ������� ���������� ����
	static ActFactory *instance();

	ActFactory();
	~ActFactory();
};

}