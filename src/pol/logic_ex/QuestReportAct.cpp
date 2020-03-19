#include "precomp.h"
#include "QuestReportAct.h"

namespace PoL
{

//=====================================================================================//
//                          QuestReportAct::QuestReportAct()                           //
//=====================================================================================//
QuestReportAct::QuestReportAct(HAct act)
:	m_started(false),
	m_ended(false),
	m_endable(act->isEndable()),
	m_act(act),
	m_reportId(-1)
{
}

//=====================================================================================//
//                          QuestReportAct::~QuestReportAct()                          //
//=====================================================================================//
QuestReportAct::~QuestReportAct()
{
}

//=====================================================================================//
//                             void QuestReportAct::skip()                             //
//=====================================================================================//
void QuestReportAct::skip()
{
	assert( isStarted() );
	assert( !isEnded() );

	m_act->skip();

	m_started = false;
}

//=====================================================================================//
//                            void QuestReportAct::start()                             //
//=====================================================================================//
void QuestReportAct::start()
{
	assert( !isStarted() );
	m_started = true;

	m_reportId = QuestReporter::startReport();
	m_act->start();
}

//=====================================================================================//
//                            void QuestReportAct::update()                            //
//=====================================================================================//
void QuestReportAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(!m_act->isEnded()) m_act->update(tau);
	if(m_act->isEnded())
	{
		m_ended = true;
		QuestReporter::endReport(m_reportId);
	}
}

}