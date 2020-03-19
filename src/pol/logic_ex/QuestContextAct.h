#pragma once

#include "Act.h"
#include "QuestContext.h"

namespace PoL
{

//=====================================================================================//
//                          class QuestReportAct : public Act                          //
//=====================================================================================//
class QuestContextAct : public Act
{
	bool m_started;
	bool m_endable;
	bool m_ended;

	HAct m_act;
	std::string m_holder, m_actor;
	QuestContext::ContextId_t m_contextId;

public:
	QuestContextAct(HAct act, const std::string &holder, const std::string &actor);
	~QuestContextAct();

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip();
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return m_endable; }
	/// Содержит ли акт выигрыщ или проигрыш
	virtual bool hasEndGame() const { return m_act->hasEndGame(); }
};


}