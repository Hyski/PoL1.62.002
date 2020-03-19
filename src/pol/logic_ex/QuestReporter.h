#pragma once

namespace PoL
{

//=====================================================================================//
//                                 class QuestReporter                                 //
//=====================================================================================//
class QuestReporter
{
public:
	typedef int ReportId_t;

	/// ������ ����� �� ������
	static ReportId_t startReport();
	/// ��������� ����� �� ������
	static void endReport(ReportId_t);

	/// �������� ����
	static void experienceAdded(int exp);
	/// ��������� ������
	static void moneyAdded(int money);
	/// �������� ����
	static void itemAdded(BaseThing *);
};

}