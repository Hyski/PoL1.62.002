#include "precomp.h"

#include <logic2/AIBase.h>
#include "EliminateTask.h"

namespace PoL
{

//=====================================================================================//
//                           EliminateTask::EliminateTask()                            //
//=====================================================================================//
EliminateTask::EliminateTask(Id_t id, const std::string &force, const std::string &level)
:	Task(id,false),
	m_force(force),
	m_level(level),
	m_priority(0),
	m_victimCount(0),
	m_locked(true)
{
	m_spawnConn = TaskDatabase::instance()->attach(boost::bind(onEntitySpawned,this),TaskDatabase::mtEntitySpawned);
	m_enterLevelConn = TaskDatabase::instance()->attach(boost::bind(onEnterLevel,this),TaskDatabase::mtEnterLevel);
	m_leaveLevelConn = TaskDatabase::instance()->attach(boost::bind(onLeaveLevel,this),TaskDatabase::mtLeaveLevel);
	m_teamChangeConn = TaskDatabase::instance()->attach(boost::bind(onEntityTeamChanged,this),TaskDatabase::mtHeroTeamChanged);
}

//=====================================================================================//
//                      void EliminateTask::onEntityTeamChanged()                      //
//=====================================================================================//
void EliminateTask::onEntityTeamChanged()
{
	const EventData &data = TaskDatabase::instance()->getMessageInProcess();
	BaseEntity *entity = getEntityById(data.m_subject_id);
	if(entity->GetPlayer() != PT_PLAYER)
	{
		onEntitySpawned();
	}
}

//=====================================================================================//
//                          void EliminateTask::setPriority()                          //
//=====================================================================================//
void EliminateTask::setPriority(int p)
{
	m_spawnConn.setPriority(p);
	m_teamChangeConn.setPriority(p);

	for(Conns_t::iterator i = m_deadConns.begin(); i != m_deadConns.end(); ++i)
	{
		i->m_hconn->setPriority(p);
		i->m_hconn2->setPriority(p);
	}

	m_enterLevelConn.setPriority(p);
	m_leaveLevelConn.setPriority(p);

	m_priority = p;
}

//=====================================================================================//
//                           void EliminateTask::doEnable()                            //
//=====================================================================================//
void EliminateTask::doEnable(bool v)
{
	//m_spawnConn.enable(v);
	//for(Conns_t::iterator i = m_deadConns.begin(); i != m_deadConns.end(); ++i)
	//{
	//	i->m_hconn->enable(v);
	//}
}

//=====================================================================================//
//                         void EliminateTask::onEntityDead()                          //
//=====================================================================================//
void EliminateTask::onEntityDead(int id)
{
	const EventData &data = TaskDatabase::instance()->getMessageInProcess();
	if(id == data.m_subject_id)
	{
		ACT_LOG("INFO: EliminateTask[" << m_force << "] has removed entity [" << data.m_subject << "]\n");

		for(Conns_t::iterator i = m_deadConns.begin(); i != m_deadConns.end(); ++i)
		{
			if(i->m_eid == id)
			{
				m_deadConns.erase(i);
				break;
			}
		}

		if(!--m_victimCount)
		{
			setState(tsCompleted);
		}
	}
}

//=====================================================================================//
//                           void EliminateTask::addVictim()                           //
//=====================================================================================//
void EliminateTask::addVictim(const std::string &who, ::eid_t whoid)
{
	::BaseEntity *e = ::EntityPool::GetInst()->Get(whoid);
	if(e->GetPlayer() != PT_PLAYER)
	{
		Conn_t c,c2;

		//if(::VehicleEntity *ve = e->Cast2Vehicle())
		//{
		c2 = TaskDatabase::instance()->attach(boost::bind(onEntityDead,this,whoid),TaskDatabase::mtHeroTeamChanged);
		c2.setPriority(m_priority);
		//}

		c = TaskDatabase::instance()->attach(boost::bind(onEntityDead,this,whoid),TaskDatabase::mtEntityDestroyed);
		c.setPriority(m_priority);
		ConnInfo cinfo = {HConn_t(new Conn_t(c)),HConn_t(new Conn_t(c2)),who,whoid};
		m_deadConns.push_back(cinfo);

		if(!m_victimCount++)
		{
			setState(tsNotCompleted);
		}

		ACT_LOG("INFO: EliminateTask[" << m_force << "] has added entity [" << who << "]\n");
	}
}

//=====================================================================================//
//                           void EliminateTask::addVictim()                           //
//=====================================================================================//
void EliminateTask::addVictim(const EventData &data)
{
	addVictim(data.m_subject,data.m_subject_id);
}

//=====================================================================================//
//                        void EliminateTask::onEntitySpawned()                        //
//=====================================================================================//
void EliminateTask::onEntitySpawned()
{
	const EventData &data = TaskDatabase::instance()->getMessageInProcess();
	::BaseEntity *entity = getEntityById(data.m_subject_id);

	// здесь еще, наверное, технику нужно учесть (причем ее еще и взламывать можно)
	if(data.m_level == m_level)
	{
		std::string type;
		std::string force;
		::AIBase::getAINodeDesc(entity->GetEntityContext()->GetAIModel(),type,force);

		if(m_force == force)
		{
			addVictim(data);
		}
	}
}

//=====================================================================================//
//                         void EliminateTask::onEnterLevel()                          //
//=====================================================================================//
void EliminateTask::onEnterLevel()
{
	enable(getLevel() == m_level);
}

//=====================================================================================//
//                         void EliminateTask::onLeaveLevel()                          //
//=====================================================================================//
void EliminateTask::onLeaveLevel()
{
	enable(false);
}

//=====================================================================================//
//                          void EliminateTask::store() const                          //
//=====================================================================================//
void EliminateTask::store(mll::io::obinstream &out) const
{
	Task::store(out);
	out << m_priority;

	out << asInt(m_deadConns.size());

	for(Conns_t::const_iterator i = m_deadConns.begin(); i != m_deadConns.end(); ++i)
	{
		out << i->m_entity;
		out << asInt(i->m_eid);
	}
}

//=====================================================================================//
//                            void EliminateTask::restore()                            //
//=====================================================================================//
void EliminateTask::restore(mll::io::ibinstream &in)
{
	Task::restore(in);
	in >> m_priority;

	int count;
	in >> asInt(count);

	for(int i = 0; i < count; ++i)
	{
		::eid_t id;
		std::string ename;
		in >> ename;
		in >> asInt(id);
		addVictim(ename,id);
	}
}

//=====================================================================================//
//                             void EliminateTask::reset()                             //
//=====================================================================================//
void EliminateTask::reset()
{
	Task::reset();
	enable(false);
	m_victimCount = 0;
	m_deadConns.clear();
}

}
