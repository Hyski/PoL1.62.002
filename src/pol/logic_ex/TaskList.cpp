#include "precomp.h"
#include "TaskList.h"

namespace PoL
{

//=====================================================================================//
//                                TaskList::~TaskList()                                //
//=====================================================================================//
TaskList::~TaskList()
{
}

//=====================================================================================//
//                             void TaskList::initState()                              //
//=====================================================================================//
void TaskList::initState()
{
	m_taskConns.resize(m_tasks.size());
	m_taskStates.resize(m_tasks.size());
	m_counts[tsNotCompleted] = m_counts[tsCompleted] = m_counts[tsFailed] = 0;

	for(Tasks_t::iterator i = m_tasks.begin(); i != m_tasks.end(); ++i)
	{
		const int index = i-m_tasks.begin();
		++m_counts[m_taskStates[index] = (*i ? (*i)->getState() : tsNotCompleted)];
		assert( i->valid() );
		if(*i) m_taskConns[index].reset(new Task::Connection((*i)->addFn(boost::bind(onTaskStateChanged,this,index,_1))));
	}

	applyState();
}

//=====================================================================================//
//                             void TaskList::applyState()                             //
//=====================================================================================//
void TaskList::applyState(bool notify)
{
	if(m_counts[tsFailed])
	{
		setState(tsFailed,notify);
	}
	else if(m_counts[tsCompleted] == m_tasks.size())
	{
		setState(tsCompleted,notify);
	}
	else
	{
		setState(tsNotCompleted,notify);
	}
}

//=====================================================================================//
//                    HCondition TaskList::getPreCondition() const                     //
//=====================================================================================//
HCondition TaskList::getPreCondition() const
{
	for(Tasks_t::const_iterator i = m_tasks.begin(); i != m_tasks.end(); ++i)
	{
		if((*i)->getPreCondition()) return (*i)->getPreCondition();
	}

	return 0;
}

//=====================================================================================//
//                         void TaskList::onTaskStateChanged()                         //
//=====================================================================================//
void TaskList::onTaskStateChanged(int i, TaskState ts)
{
	--m_counts[m_taskStates[i]];
	++m_counts[m_taskStates[i] = ts];
	applyState();
}

//=====================================================================================//
//                              void TaskList::doEnable()                              //
//=====================================================================================//
void TaskList::doEnable(bool v)
{
	for(int i = 0; i < m_tasks.size(); ++i)
	{
		m_tasks[i]->enable(v);
	}
}

//=====================================================================================//
//                            void TaskList::setPriority()                             //
//=====================================================================================//
void TaskList::setPriority(int v)
{
	for(int i = 0; i < m_tasks.size(); ++i)
	{
		m_tasks[i]->setPriority(v);
	}
}

//=====================================================================================//
//                            void TaskList::store() const                             //
//=====================================================================================//
void TaskList::store(mll::io::obinstream &out) const
{
	Task::store(out);

	for(int i = 0; i < m_taskStates.size(); ++i)
	{
		out << asInt(m_taskStates[i]);
	}

	for(int i = 0; i < tsCount; ++i)
	{
		out << asInt(m_counts[i]);
	}
}

//=====================================================================================//
//                              void TaskList::restore()                               //
//=====================================================================================//
void TaskList::restore(mll::io::ibinstream &in)
{
	Task::restore(in);

	for(int i = 0; i < m_taskStates.size(); ++i)
	{
		in >> asInt(m_taskStates[i]);
	}

	for(int i = 0; i < tsCount; ++i)
	{
		in >> asInt(m_counts[i]);
	}
}

//=====================================================================================//
//                               void TaskList::reset()                                //
//=====================================================================================//
void TaskList::reset()
{
	Task::reset();
	initState();
	applyState(false);
}

//=====================================================================================//
//                             void TaskList::taskReset()                              //
//=====================================================================================//
void TaskList::taskReset()
{
	for(int i = 0; i != m_tasks.size(); ++i)
	{
		m_tasks[i]->taskReset();
	}

	Task::taskReset();
}

}