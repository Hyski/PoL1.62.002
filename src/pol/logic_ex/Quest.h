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
	/// Локализованные ресурсы
	std::string m_locname;			///< Локализованное имя квеста
	std::string m_journal;			///< Сообщение в журнал
	phrase_s m_onStart;				///< Сообщение при старте квеста
	phrase_s m_onFailed;			///< Сообщение при провале квеста
	phrase_s m_onRemind;			///< Сообщение при напоминании квеста
	phrase_s m_onCompleted;			///< Сообщение при успешном завершении квеста
	phrase_s m_onCompletedOnStart;	///< Сообщение выдающееся когда квест выполнен уже при старте
	phrase_s m_onFailedOnStart;		///< Сообщение выдающееся когда квест выполнен уже при старте
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

	HActTemplate m_onStartAct;	///< Акт, проигрывающийся при старте квеста
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
	static StateChange m_stateChanges[MaxStateChanges];		///< Уведомления о переходах

	StateFn_t getStateChangeFn(QuestState s, QuestState e);

public:
	Quest(Id_t id, const QuestParams &);
	virtual ~Quest();

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stQuest; }

	void setStartTask(HTask task) {}

	/// Разрешен ли квест
	bool isEnabled() const { return m_enabled; }
	/// Разрешить квест
	void enable(bool v);

	/// Уведомить квест о произошедшем событии
	void sendMessage(QuestMessage qm);

	/// Возвращает текущее состояние квеста
	QuestState getState() const { return m_state; }
	///// Возвращает состояние условия старта
	//TaskState getStartTaskState() const { return m_

	const std::string &getHolder() const { return m_holder; }

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	/// Сбросить состояние на начальное
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
	/// Сбросить состояние квеста на начальное (как при старте новой игры)
	void resetState();
	/// Изменить состояние квеста исходя из состояний задач
	void promoteState();
	/// Изменить состояние квеста
	void changeState(QuestState);

	/// Проверка состояния квеста на валидность
	bool validState(QuestState s) const;

	/// Возвращает true, если задача не выполнена, но и не провалена
	bool notCompleted(TaskType t) const { return m_taskStates[t] == tsNotCompleted; }
	/// Возвращает true, если задача выполнена
	bool completed(TaskType t) const { return m_taskStates[t] == tsCompleted; }
	/// Возвращает true, если задача провалена
	bool failed(TaskType t) const { return m_taskStates[t] == tsFailed; }

	/// Возвращает указатель на сущность с системным именем m_holder
	BaseEntity *getHolderPtr() const;

	/// Квест начат
	void onStartQuest();
	/// Квест был выполнен до того, как подошли к холдеру
	void onCompletedOnStart();
	/// Квест завершен
	void onQuestCompleted();
	/// Напомнить квест
	void onRemindQuest();
	/// Квест провален
	void onQuestFailed();
	/// Квест был провален до того, как подошли к холдеру
	void onFailedOnStart();
	/// Задание выполнено, но игрок еще не дал свое согласие на отдачу предметов
	void onPreSuccess();

	/// Обработать сообщение
	void acceptMessage(QuestMessage qm);

	/// Включить/выключить задачи в зависимости от состояния квеста
	void adjustTasks();

private:
	/// Вызывается при изменении условия зависимости квеста
	void onDependencyChanged(bool);
	/// Создать контекст для квеста
	HAct makeContext(HAct act);
	/// Вызывается при возникновении условия напоминания квеста
	void onRemindChanged(bool);
	/// Вызывается при изменении состояния стартовой задачи
	void onStartTaskChanged(TaskState);
	/// Вызывается при изменении состояния задачи для окончания
	void onEndTaskChanged(TaskState);
	/// Вызывается при изменении состояния квестовой задачи
	void onQuestTaskChanged(TaskState);
	/// Вызывается при изменении состояния предусловия
	void onPreCondition(bool);
};

}
