#pragma once

#include "Act.h"
#include "Quest.h"
#include "QuestMessage.h"

namespace PoL
{

//=====================================================================================//
//                         class SendQuestMessage : public Act                         //
//=====================================================================================//
class SendQuestMessageAct : public Act
{
	bool m_started;
	bool m_ended;

	HQuest m_quest;
	QuestMessage m_msg;

public:
	SendQuestMessageAct(const std::string &, QuestMessage);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() { m_ended = true; }
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_ended; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return true; }
};

}