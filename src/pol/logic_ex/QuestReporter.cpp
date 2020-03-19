#include "precomp.h"
#include "QuestReporter.h"

#include <boost/format.hpp>

namespace PoL
{

namespace QuestReporterDetails
{

using QuestReporter::ReportId_t;

//=====================================================================================//
//                                  class QuestReport                                  //
//=====================================================================================//
class QuestReport
{
	ReportId_t m_id;
	int m_exp, m_money, m_things;

public:
	QuestReport(ReportId_t id)
	:	m_id(id),
		m_exp(0),
		m_money(0),
		m_things(0)
	{
	}

	~QuestReport()
	{
		if(m_exp || m_money || m_things)
		{
			std::ostringstream ostr;
			ostr << ::DirtyLinks::GetStrRes("gqs_got_bonus");

			if(m_money)
			{
				ostr << ' ' << boost::format(::DirtyLinks::GetStrRes("gqs_got_money")) % m_money;
			}

			if(m_exp)
			{
				if(m_money) ostr << ',';
				ostr << ' ' << boost::format(::DirtyLinks::GetStrRes("gqs_got_experience")) % m_exp;
			}

			if(m_things)
			{
				if(m_money || m_exp) ostr << ',';
				ostr << ' ' << DirtyLinks::GetStrRes("gqs_got_equipment");
			}

			::DirtyLinks::Print(ostr.str());
		}
	}

	void experienceAdded(int e)
	{
		m_exp += e;
	}

	void moneyAdded(int e)
	{
		m_money += e;
	}

	void itemAdded(BaseThing *)
	{
		++m_things;
	}

	ReportId_t getReportId() const { return m_id; }
};

typedef boost::shared_ptr<QuestReport> HQuestReport;
typedef std::list< HQuestReport > Reports_t;

ReportId_t g_id = 0;
Reports_t g_reports;

}

using namespace QuestReporterDetails;

//=====================================================================================//
//                       ReportId_t QuestReporter::startReport()                       //
//=====================================================================================//
ReportId_t QuestReporter::startReport()
{
	HQuestReport report(new QuestReport(g_id++));
	g_reports.push_back(report);
	return report->getReportId();
}

//=====================================================================================//
//                           void QuestReporter::endReport()                           //
//=====================================================================================//
void QuestReporter::endReport(ReportId_t id)
{
	for(Reports_t::iterator i = g_reports.begin(); i != g_reports.end(); ++i)
	{
		if((*i)->getReportId() == id)
		{
			g_reports.erase(i);
			return;
		}
	}
}

//=====================================================================================//
//                        void QuestReporter::experienceAdded()                        //
//=====================================================================================//
void QuestReporter::experienceAdded(int exp)
{
	if(g_reports.empty()) return;
	g_reports.back()->experienceAdded(exp);
}

//=====================================================================================//
//                          void QuestReporter::moneyAdded()                           //
//=====================================================================================//
void QuestReporter::moneyAdded(int m)
{
	if(g_reports.empty()) return;
	g_reports.back()->moneyAdded(m);
}

//=====================================================================================//
//                           void QuestReporter::itemAdded()                           //
//=====================================================================================//
void QuestReporter::itemAdded(BaseThing *t)
{
	if(g_reports.empty()) return;
	g_reports.back()->itemAdded(t);
}

}