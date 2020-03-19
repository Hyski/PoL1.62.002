#pragma once

#include "Task.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                          class EliminateTask : public Task                          //
//=====================================================================================//
class EliminateTask : public Task
{
	typedef TaskDatabase::Connection Conn_t;
	typedef boost::shared_ptr<Conn_t> HConn_t;

	struct ConnInfo
	{
		boost::shared_ptr<TaskDatabase::Connection> m_hconn;
		boost::shared_ptr<TaskDatabase::Connection> m_hconn2;
		std::string m_entity;
		::eid_t m_eid;
	};

	typedef std::list<ConnInfo> Conns_t;

	std::string m_force;
	std::string m_level;
	TaskDatabase::Connection m_spawnConn;
	TaskDatabase::Connection m_teamChangeConn;
	Conns_t m_deadConns;

	int m_priority;
	int m_victimCount;
	bool m_locked;

	TaskDatabase::Connection m_enterLevelConn, m_leaveLevelConn;

public:
	EliminateTask(Id_t, const std::string &force, const std::string &level);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stEliminateTask; }

	/// Установить приоритет задачи
	virtual void setPriority(int);

	virtual void store(mll::io::obinstream &out) const;
	virtual void restore(mll::io::ibinstream &in);
	virtual void reset();

private:
	virtual void doEnable(bool v);

	void addVictim(const EventData &);
	void addVictim(const std::string &who, ::eid_t whoid);

	void onEnterLevel();
	void onLeaveLevel();

	void onEntityDead(int);
	void onEntitySpawned();

	void onEntityTeamChanged();
};

}