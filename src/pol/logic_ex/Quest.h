#pragma once

#include "Task.h"
#include "Condition.h"
#include "TaskState.h"
#include "QuestState.h"
#include "Persistent.h"
#include "QuestMessage.h"
#include <logic2/phrase.h>

namespace PoL
{

struct QuestParams;

//=====================================================================================//
//                                 class QuestPhrases                                  //
//=====================================================================================//
struct QuestPhrases
{
	/// �������������� �������
	std::string m_locname;			///< �������������� ��� ������
	std::string m_journal;			///< ��������� � ������
	phrase_s m_onStart;				///< ��������� ��� ������ ������
	phrase_s m_onFailed;			///< ��������� ��� ������� ������
	phrase_s m_onRemind;			///< ��������� ��� ����������� ������
	phrase_s m_onCompleted;			///< ��������� ��� �������� ���������� ������
	phrase_s m_onCompletedOnStart;	///< ��������� ���������� ����� ����� �������� ��� ��� ������
	phrase_s m_onFailedOnStart;		///< ��������� ���������� ����� ����� �������� ��� ��� ������
};

//=====================================================================================//
//                          class Quest : public MlRefCounted                          //
//=====================================================================================//
class Quest : public Persistent, private NotifyFn< boost::function<void (QuestState)> >
{
	typedef NotifyFn< boost::function<void (QuestState)> > NFn_t;

public:
	using NFn_t::Fn_t;
	using NFn_t::addFn;
	using NFn_t::Connection;

private:
	typedef void (Quest::*StateFn_t)();

	enum TaskType
	{
		ttStart,
		ttQuest,
		ttEnd,
		ttCount
	};

	bool m_enabled;
	bool m_initiallyEnabled;
	QuestState m_state;
	HTask m_tasks[ttCount];
	HCondition m_remind, m_dependency;
	TaskState m_taskStates[ttCount];
	std::string m_holder;

	HActTemplate m_onStartAct;	///< ���, ��������������� ��� ������ ������
	HActTemplate m_onFailAct;
	HActTemplate m_onSuccessAct;
	HActTemplate m_onRemindAct;
	HActTemplate m_onComplonsAct;
	HActTemplate m_onFailonsAct;
	HActTemplate m_onPreSucceedAct;
	bool m_complonsPlayed;
	int m_cannotRemind;

	QuestPhrases m_phrases;

	Task::Connection m_startConn, m_questConn, m_endConn;
	Condition::Connection m_remindConn, m_dependsConn, m_preCondConn;

	struct StateChange
	{
		QuestState m_from;
		QuestState m_to;
		StateFn_t m_fn;
	};

	static const int MaxStateChanges = 32;
	static StateChange m_stateChanges[MaxStateChanges];		///< ����������� � ���������

	StateFn_t getStateChangeFn(QuestState s, QuestState e);

public:
	Quest(Id_t id, const QuestParams &);
	virtual ~Quest();

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stQuest; }

	void setStartTask(HTask task) {}

	/// �������� �� �����
	bool isEnabled() const { return m_enabled; }
	/// ��������� �����
	void enable(bool v);

	/// ��������� ����� � ������������ �������
	void sendMessage(QuestMessage qm);

	/// ���������� ������� ��������� ������
	QuestState getState() const { return m_state; }
	///// ���������� ��������� ������� ������
	//TaskState getStartTaskState() const { return m_

	const std::string &getHolder() const { return m_holder; }

	/// ��������� ��������� �������
	virtual void store(mll::io::obinstream &) const;
	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &);

	/// �������� ��������� �� ���������
	virtual void reset();
	void resetTasks();

	bool isStarted() const
	{
		return m_state != qsNotStarted;
	}

	bool isPassed() const
	{
		return m_state == qsCompleted || m_state == qsFailed || m_state == qsPassed;
	}

	bool isFailed() const
	{
		return m_state == qsPassed || m_state == qsFailed;
	}

	virtual void dumpState(std::ostream &out) const;

private:
	/// �������� ��������� ������ �� ��������� (��� ��� ������ ����� ����)
	void resetState();
	/// �������� ��������� ������ ������ �� ��������� �����
	void promoteState();
	/// �������� ��������� ������
	void changeState(QuestState);

	/// �������� ��������� ������ �� ����������
	bool validState(QuestState s) const;

	/// ���������� true, ���� ������ �� ���������, �� � �� ���������
	bool notCompleted(TaskType t) const { return m_taskStates[t] == tsNotCompleted; }
	/// ���������� true, ���� ������ ���������
	bool completed(TaskType t) const { return m_taskStates[t] == tsCompleted; }
	/// ���������� true, ���� ������ ���������
	bool failed(TaskType t) const { return m_taskStates[t] == tsFailed; }

	/// ���������� ��������� �� �������� � ��������� ������ m_holder
	BaseEntity *getHolderPtr() const;

	/// ����� �����
	void onStartQuest();
	/// ����� ��� �������� �� ����, ��� ������� � �������
	void onCompletedOnStart();
	/// ����� ��������
	void onQuestCompleted();
	/// ��������� �����
	void onRemindQuest();
	/// ����� ��������
	void onQuestFailed();
	/// ����� ��� �������� �� ����, ��� ������� � �������
	void onFailedOnStart();
	/// ������� ���������, �� ����� ��� �� ��� ���� �������� �� ������ ���������
	void onPreSuccess();

	/// ���������� ���������
	void acceptMessage(QuestMessage qm);

	/// ��������/��������� ������ � ����������� �� ��������� ������
	void adjustTasks();

private:
	/// ���������� ��� ��������� ������� ����������� ������
	void onDependencyChanged(bool);
	/// ������� �������� ��� ������
	HAct makeContext(HAct act);
	/// ���������� ��� ������������� ������� ����������� ������
	void onRemindChanged(bool);
	/// ���������� ��� ��������� ��������� ��������� ������
	void onStartTaskChanged(TaskState);
	/// ���������� ��� ��������� ��������� ������ ��� ���������
	void onEndTaskChanged(TaskState);
	/// ���������� ��� ��������� ��������� ��������� ������
	void onQuestTaskChanged(TaskState);
	/// ���������� ��� ��������� ��������� �����������
	void onPreCondition(bool);
};

}
