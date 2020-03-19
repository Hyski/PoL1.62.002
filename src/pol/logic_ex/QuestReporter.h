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

	/// Начать отчет по квесту
	static ReportId_t startReport();
	/// Закончить отчет по квесту
	static void endReport(ReportId_t);

	/// Добавлен опыт
	static void experienceAdded(int exp);
	/// Добавлены деньги
	static void moneyAdded(int money);
	/// Добавлен итем
	static void itemAdded(BaseThing *);
};

}