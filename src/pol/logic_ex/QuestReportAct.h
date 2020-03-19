#pragma once

#include "Act.h"
#include "QuestReporter.h"

namespace PoL
{

//=====================================================================================//
//                          class QuestReportAct : public Act                          //
//=====================================================================================//
class QuestReportAct : public Act
{
	bool m_started;
	bool m_endable;
	bool m_ended;

	HAct m_act;
	QuestReporter::ReportId_t m_reportId;

public:
	QuestReportAct(HAct act);
	~QuestReportAct();

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