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

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip();
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_ended; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return m_endable; }
	/// �������� �� ��� ������� ��� ��������
	virtual bool hasEndGame() const { return m_act->hasEndGame(); }
};


}